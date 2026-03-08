#include "GameLogic/TurnBasedGameMode.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ConfigWidget.h"
#include "Grid/GridManager.h"
#include "Units/Sniper.h"
#include "Units/Brawler.h"
#include "UI/PlacementWidget.h"
#include "UI/GameHUDWidget.h"
#include "EngineUtils.h"

ATurnBasedGameMode::ATurnBasedGameMode()
{
    GameStateClass = ATurnBasedGameState::StaticClass();
    DefaultPawnClass = nullptr;
    HumanUnitsPlaced = 0;
    AIUnitsPlaced = 0;
    PlacementTurn = ETurnOwner::Human;
    GridManagerRef = nullptr;
    SelectedUnit = nullptr;
    SelectedCell = nullptr;
    TowerControlSystem = CreateDefaultSubobject<UTowerControlSystem>(TEXT("TowerControlSystem"));
    Pathfinding = CreateDefaultSubobject<UPathfindingComponent>(TEXT("Pathfinding"));
    MovingUnit = nullptr;
    CurrentPathStep = 0;
    bIsAIMoving = false;
    SelectedTower = nullptr;
}

void ATurnBasedGameMode::BeginPlay()
{
    Super::BeginPlay();

    //fallback per il controllo delle torri
    if (!TowerControlSystem)
    {
        TowerControlSystem = NewObject<UTowerControlSystem>(this);
        UE_LOG(LogTemp, Warning, TEXT("TowerControlSystem recreated in BeginPlay"));
    }
    if (!Pathfinding)
    {
        Pathfinding = NewObject<UPathfindingComponent>(this);
        UE_LOG(LogTemp, Warning, TEXT("Pathfinding recreated in BeginPlay"));
    }

    GetWorldTimerManager().SetTimerForNextTick([this]()
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (!PC) return;

            PC->bAutoManageActiveCameraTarget = false;

            for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
            {
                FViewTargetTransitionParams Params;
                Params.BlendTime = 0.f;
                PC->SetViewTarget(*It, Params);
                break;
            }

            for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
            {
                GridManagerRef = *It;
                break;
            }

            if (!ConfigWidgetClass || !GridManagerRef) return;

            UConfigWidget* Widget = CreateWidget<UConfigWidget>(PC, ConfigWidgetClass);
            if (Widget)
            {
                Widget->GridManager = GridManagerRef;
                Widget->AddToViewport();
                PC->bShowMouseCursor = true;
                PC->bEnableClickEvents = true;
                PC->SetInputMode(FInputModeUIOnly());
            }
        });
}

void ATurnBasedGameMode::PerformCoinFlip()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    bool bHumanWins = FMath::RandBool();
    GS->CoinFlipWinner = bHumanWins ? ETurnOwner::Human : ETurnOwner::AI;
    GS->CurrentTurn = GS->CoinFlipWinner;
    GS->CurrentPhase = EGamePhase::Placement;
    PlacementTurn = GS->CoinFlipWinner;

    FString Winner = bHumanWins ? TEXT("Human") : TEXT("AI");
    UE_LOG(LogTemp, Warning, TEXT("Coin flip: %s wins!"), *Winner);

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !CoinFlipWidgetClass) return;

    UCoinFlipWidget* Widget = CreateWidget<UCoinFlipWidget>(PC, CoinFlipWidgetClass);
    if (Widget)
    {
        Widget->AddToViewport();
        Widget->ShowResult(Winner);
    }

    FTimerHandle PlacementTimer;
    GetWorldTimerManager().SetTimer(PlacementTimer, [this]()
        {
            if (PlacementTurn == ETurnOwner::Human)
            {
                HighlightHumanPlacementZone();
                ShowPlacementWidget();
            }
            else
                PerformAIPlacement();
        }, 3.f, false);
}

void ATurnBasedGameMode::HighlightHumanPlacementZone()
{
    if (!GridManagerRef) return;
    ClearHighlight();

    //evidenzia celle valide in Y=0,1,2 non occupate e non acqua
    for (int32 Y = 0; Y <= 2; Y++)
    {
        for (int32 X = 0; X < 25; X++)
        {
            AGridCell* Cell = GridManagerRef->GetCell(X, Y);
            if (Cell && Cell->ElevationLevel > 0 && !Cell->bIsOccupied)
            {
                UMaterialInstanceDynamic* DynMat =
                    Cell->GetCellMesh()->CreateAndSetMaterialInstanceDynamic(0);
                if (DynMat)
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.f, 1.f, 1.f));
                HighlightedCells.Add(Cell);
            }
        }
    }
}

void ATurnBasedGameMode::OnHumanPlacementCellClicked(int32 X, int32 Y)
{
    if (!GridManagerRef) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS || GS->CurrentPhase != EGamePhase::Placement) return;
    if (GS->CurrentTurn != ETurnOwner::Human) return;

    if (Y < 0 || Y > 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid zone (Y must be 0-2)"));
        return;
    }

    AGridCell* Cell = GridManagerRef->GetCell(X, Y);
    if (!Cell || Cell->ElevationLevel == 0 || Cell->bIsOccupied) return;

    SpawnUnitAtCell(Cell);
}

void ATurnBasedGameMode::SpawnUnitAtCell(AGridCell* Cell)
{
    if (!Cell || !GridManagerRef) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    TSubclassOf<ABaseUnit> ClassToSpawn = (HumanUnitsPlaced == 0)
        ? TSubclassOf<ABaseUnit>(SniperClass)
        : TSubclassOf<ABaseUnit>(BrawlerClass);

    if (!ClassToSpawn) return;

    FVector WorldPos = Cell->GetActorLocation();
    WorldPos.Z += 60.f;

    FActorSpawnParameters Params;
    Params.Owner = this;
    ABaseUnit* Unit = GetWorld()->SpawnActor<ABaseUnit>(ClassToSpawn, WorldPos, FRotator::ZeroRotator, Params);
    if (!Unit) return;

    Unit->GridX = Cell->GridX;
    Unit->GridY = Cell->GridY;
    Unit->SpawnGridX = Cell->GridX;
    Unit->SpawnGridY = Cell->GridY;
    Unit->UnitOwner = EOwner::Human;
    Unit->OwnerColor = HumanUnitColor;
    Unit->SetOwnerColor();
    Cell->bIsOccupied = true;

    GS->HumanUnits.Add(Unit);
    HumanUnitsPlaced++;

    UE_LOG(LogTemp, Warning, TEXT("Human spawned unit %d at (%d,%d)"), HumanUnitsPlaced, Cell->GridX, Cell->GridY);

    ClearHighlight();
    AdvancePlacementStep();
}

void ATurnBasedGameMode::ClearHighlight()
{
    //ripristina il colore originale di ogni cella evidenziata
    for (AGridCell* Cell : HighlightedCells)
        if (Cell) Cell->UpdateVisualColor();
    HighlightedCells.Empty();
}

void ATurnBasedGameMode::ShowPlacementWidget()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PlacementWidgetClass) return;

    if (!PlacementWidgetRef)
    {
        PlacementWidgetRef = CreateWidget<UPlacementWidget>(PC, PlacementWidgetClass);
        if (PlacementWidgetRef) PlacementWidgetRef->AddToViewport();
    }

    FString UnitName = (HumanUnitsPlaced == 0) ? TEXT("Sniper") : TEXT("Brawler");
    if (PlacementWidgetRef) PlacementWidgetRef->ShowPlacementPrompt(UnitName);

    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
}

void ATurnBasedGameMode::PerformAIPlacement()
{
    if (!GridManagerRef) return;

    TArray<AGridCell*> ValidCells;
    for (int32 Y = 22; Y <= 24; Y++)
        for (int32 X = 0; X < 25; X++)
        {
            AGridCell* Cell = GridManagerRef->GetCell(X, Y);
            if (Cell && Cell->ElevationLevel > 0 && !Cell->bIsOccupied)
                ValidCells.Add(Cell);
        }

    if (ValidCells.IsEmpty()) return;

    AGridCell* Chosen = ValidCells[FMath::RandRange(0, ValidCells.Num() - 1)];
    SpawnAIUnitAtCell(Chosen);
}

void ATurnBasedGameMode::SpawnAIUnitAtCell(AGridCell* Cell)
{
    if (!Cell || !GridManagerRef) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    TSubclassOf<ABaseUnit> ClassToSpawn = (AIUnitsPlaced == 0)
        ? TSubclassOf<ABaseUnit>(SniperClass)
        : TSubclassOf<ABaseUnit>(BrawlerClass);

    if (!ClassToSpawn) return;

    FVector WorldPos = Cell->GetActorLocation();
    WorldPos.Z += 60.f;

    FActorSpawnParameters Params;
    Params.Owner = this;
    ABaseUnit* Unit = GetWorld()->SpawnActor<ABaseUnit>(ClassToSpawn, WorldPos, FRotator::ZeroRotator, Params);
    if (!Unit) return;

    Unit->GridX = Cell->GridX;
    Unit->GridY = Cell->GridY;
    Unit->SpawnGridX = Cell->GridX;
    Unit->SpawnGridY = Cell->GridY;
    Unit->UnitOwner = EOwner::AI;
    Unit->OwnerColor = AIUnitColor;
    Unit->SetOwnerColor();
    Cell->bIsOccupied = true;

    GS->AIUnits.Add(Unit);
    AIUnitsPlaced++;

    UE_LOG(LogTemp, Warning, TEXT("AI spawned unit %d at (%d,%d)"), AIUnitsPlaced, Cell->GridX, Cell->GridY);

    AdvancePlacementStep();
}

void ATurnBasedGameMode::AdvancePlacementStep()
{
    if ((HumanUnitsPlaced + AIUnitsPlaced) >= 4)
    {
        if (PlacementWidgetRef) PlacementWidgetRef->HidePlacementPrompt();
        StartGamePhase();
        return;
    }

    PlacementTurn = (PlacementTurn == ETurnOwner::Human) ? ETurnOwner::AI : ETurnOwner::Human;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (GS) GS->CurrentTurn = PlacementTurn;

    if (PlacementTurn == ETurnOwner::AI)
        PerformAIPlacement();
    else
    {
        HighlightHumanPlacementZone();
        ShowPlacementWidget();
    }
}

void ATurnBasedGameMode::StartGamePhase()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    GS->CurrentPhase = EGamePhase::Playing;
    GS->CurrentTurn = GS->CoinFlipWinner;

    UE_LOG(LogTemp, Warning, TEXT("Game phase started! First turn: %s"),
        GS->CurrentTurn == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"));

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->SetInputMode(FInputModeGameAndUI());
    }

    //se l'AI ha vinto il coin flip, avvia subito il suo turno
    if (GS->CurrentTurn == ETurnOwner::AI)
    {
        FTimerHandle DelayTimer;
        GetWorldTimerManager().SetTimer(DelayTimer, this, &ATurnBasedGameMode::StartAITurn, 1.f, false);
    }

    //crea il widget HUD con i bottoni
    APlayerController* HUDController = GetWorld()->GetFirstPlayerController();
    if (HUDController && GameHUDWidgetClass)
    {
        GameHUDWidgetRef = CreateWidget<UGameHUDWidget>(HUDController, GameHUDWidgetClass);
        if (GameHUDWidgetRef) GameHUDWidgetRef->AddToViewport();
    }
    
}

void ATurnBasedGameMode::OnTowerClicked(ATower* Tower)
{
    if (SelectedTower == Tower)
        ClearInfoHighlight();
    else
        ShowTowerCaptureZone(Tower);
}

void ATurnBasedGameMode::ShowTowerCaptureZone(ATower* Tower)
{
    ClearInfoHighlight();
    if (!Tower || !GridManagerRef) return;
    SelectedTower = Tower;

    //evidenzia in giallo tutte le celle nel raggio Chebyshev <= CaptureRadius
    for (int32 DX = -Tower->CaptureRadius; DX <= Tower->CaptureRadius; DX++)
    {
        for (int32 DY = -Tower->CaptureRadius; DY <= Tower->CaptureRadius; DY++)
        {
            int32 CX = Tower->GridX + DX;
            int32 CY = Tower->GridY + DY;
            if (CX < 0 || CX >= 25 || CY < 0 || CY >= 25) continue;
            AGridCell* Cell = GridManagerRef->GetCell(CX, CY);
            if (!Cell) continue;
            UMaterialInstanceDynamic* DynMat = Cell->GetCellDynMat();
            if (DynMat)
                DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.9f, 0.f));
            InfoHighlightedCells.Add(Cell);
        }
    }
}

void ATurnBasedGameMode::ClearInfoHighlight()
{
    for (AGridCell* Cell : InfoHighlightedCells)
        if (Cell) Cell->UpdateVisualColor();
    InfoHighlightedCells.Empty();
    SelectedTower = nullptr;
}

bool ATurnBasedGameMode::IsPlacementComplete() const
{
    return (HumanUnitsPlaced + AIUnitsPlaced) >= 4;
}

void ATurnBasedGameMode::OnHumanGameCellClicked(int32 X, int32 Y)
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS || GS->CurrentPhase != EGamePhase::Playing) return;
    if (GS->CurrentTurn != ETurnOwner::Human) return;

    //controlla se si è cliccato un nemico attaccabile
    if (SelectedUnit && !SelectedUnit->bHasAttacked)
    {
        for (ABaseUnit* Enemy : AttackableTargets)
        {
            if (Enemy && Enemy->GridX == X && Enemy->GridY == Y)
            {
                ExecuteAttack(SelectedUnit, Enemy);
                return;
            }
        }
    }

    //controlla se cella nel range di movimento
    if (SelectedUnit && !SelectedUnit->bHasMoved && ReachableCellSet.Contains(FIntPoint(X, Y)))
    {
        MoveUnitToCell(SelectedUnit, X, Y);
        return;
    }

    //controlla se si è cliccata un'unità Human
    for (ABaseUnit* Unit : GS->HumanUnits)
    {
        if (Unit && Unit->GridX == X && Unit->GridY == Y)
        {
            if (SelectedUnit == Unit)
            {
                //deseleziona: se ha già mosso e nessun nemico in range, conta l'azione
                if (Unit->bHasMoved && AttackableTargets.IsEmpty())
                {
                    DeselectUnit();
                    HumanUnitsActedSet.Add(Unit);
                    ATurnBasedGameState* GS2 = GetTurnGameState();
                    if (GS2 && HumanUnitsActedSet.Num() >= GS2->HumanUnits.Num())
                        EndTurn();
                }
                else
                    DeselectUnit();
            }
            else
                SelectUnit(Unit);
            return;
        }
    }

    if (SelectedUnit)
        DeselectUnit();
}

void ATurnBasedGameMode::EndTurn()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    DeselectUnit();
    HumanUnitsActedSet.Empty(); //reset set unità che hanno agito

    TArray<ABaseUnit*>& CurrentUnits = (GS->CurrentTurn == ETurnOwner::Human)
        ? GS->HumanUnits : GS->AIUnits;
    for (ABaseUnit* Unit : CurrentUnits)
        if (Unit) Unit->ResetTurnState();

    //valuta torri a fine turno
    if (TowerControlSystem)
        TowerControlSystem->EvaluateTowers(this, GS);
    else
        UE_LOG(LogTemp, Warning, TEXT("TowerControlSystem is NULL!"));

    CheckGameOver();
    if (GS->CurrentPhase == EGamePhase::GameOver) return;

    GS->SwitchTurn();

    UE_LOG(LogTemp, Warning, TEXT("EndTurn called: CurrentTurn=%s, HumanActedSet=%d, Turn=%d"),
        GS->CurrentTurn == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"),
        HumanUnitsActedSet.Num(),
        GS->TurnNumber);

    if (GS->CurrentTurn == ETurnOwner::AI)
        StartAITurn();
}

void ATurnBasedGameMode::SelectUnit(ABaseUnit* Unit)
{
    DeselectUnit();
    SelectedUnit = Unit;

    AGridCell* Cell = GridManagerRef->GetCell(Unit->GridX, Unit->GridY);
    if (Cell)
    {
        SelectedCell = Cell;
        UMaterialInstanceDynamic* DynMat = Cell->GetCellDynMat();
        if (DynMat)
            DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.4f, 0.f, 0.6f));
    }

    //prima attacco (arancione), poi movimento (cyan) sopra — movimento ha priorità visiva
    if (!Unit->bHasAttacked)
        ShowAttackRange(Unit);

    if (!Unit->bHasMoved)
        ShowMovementRange(Unit);
}
void ATurnBasedGameMode::DeselectUnit()
{
    ClearMovementRange();
    ClearAttackRange();

    if (SelectedCell)
    {
        SelectedCell->UpdateVisualColor();
        SelectedCell = nullptr;
    }
    SelectedUnit = nullptr;
}

void ATurnBasedGameMode::ShowMovementRange(ABaseUnit* Unit)
{
    ClearMovementRange();
    if (!Unit || !GridManagerRef) return;

    TMap<FIntPoint, int32> Reachable = Pathfinding->GetReachableCells(
        GridManagerRef, FIntPoint(Unit->GridX, Unit->GridY), Unit->MaxMovement);

    FIntPoint Start(Unit->GridX, Unit->GridY);
    for (auto& Pair : Reachable)
    {
        if (Pair.Key == Start) continue;
        AGridCell* Cell = GridManagerRef->GetCell(Pair.Key.X, Pair.Key.Y);
        if (!Cell) continue;

        UMaterialInstanceDynamic* DynMat = Cell->GetCellDynMat();
        if (DynMat)
            DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.f, 0.8f, 1.f));

        MovementHighlightedCells.Add(Cell);
        ReachableCellSet.Add(Pair.Key);
    }
}

void ATurnBasedGameMode::ClearMovementRange()
{
    for (AGridCell* Cell : MovementHighlightedCells)
        if (Cell) Cell->UpdateVisualColor();
    MovementHighlightedCells.Empty();
    ReachableCellSet.Empty();
}

void ATurnBasedGameMode::MoveUnitToCell(ABaseUnit* Unit, int32 DestX, int32 DestY)
{
    if (!Unit || !GridManagerRef) return;

    TArray<FIntPoint> Path = Pathfinding->FindPath(
        GridManagerRef,
        FIntPoint(Unit->GridX, Unit->GridY),
        FIntPoint(DestX, DestY),
        Unit->MaxMovement);

    if (Path.IsEmpty()) return;

    DeselectUnit();
    StartStepMovement(Unit, Path);
}

void ATurnBasedGameMode::StartStepMovement(ABaseUnit* Unit, TArray<FIntPoint> Path)
{
    MovingUnit = Unit;
    MovementPath = Path;
    CurrentPathStep = 0;

    //libera la cella di partenza
    AGridCell* OldCell = GridManagerRef->GetCell(Unit->GridX, Unit->GridY);
    if (OldCell) OldCell->bIsOccupied = false;

    //timer: uno step ogni 0.2 secondi
    GetWorldTimerManager().SetTimer(StepTimer, this,
        &ATurnBasedGameMode::DoMovementStep, 0.2f, true);
}

void ATurnBasedGameMode::DoMovementStep()
{
    if (!MovingUnit || CurrentPathStep >= MovementPath.Num())
    {
        GetWorldTimerManager().ClearTimer(StepTimer);

        if (MovingUnit)
        {
            AGridCell* NewCell = GridManagerRef->GetCell(MovingUnit->GridX, MovingUnit->GridY);
            if (NewCell) NewCell->bIsOccupied = true;
            MovingUnit->bHasMoved = true;

            if (bIsAIMoving)
            {
                //AI: attacca se possibile poi processa la prossima unità
                ABaseUnit* JustMoved = MovingUnit;
                MovingUnit = nullptr;
                MovementPath.Empty();
                CurrentPathStep = 0;
                bIsAIMoving = false;
                AIAttackIfPossible(JustMoved);
                return;
            }
            else
            {
                //Human: mostra nemici attaccabili
                ShowAttackRange(MovingUnit);

                if (AttackableTargets.IsEmpty())
                {
                    ClearMovementRange(); //nasconde il range arancione se nessun nemico attaccabile
                    MovingUnit->bHasAttacked = true;
                    HumanUnitsActedSet.Add(MovingUnit);
                    MovingUnit = nullptr;
                    MovementPath.Empty();
                    CurrentPathStep = 0;
                    ATurnBasedGameState* GS = GetTurnGameState();
                    if (GS && HumanUnitsActedSet.Num() >= GS->HumanUnits.Num())
                        EndTurn();
                    return;
                }
                else
                {
                    //ci sono nemici in range: evidenzia e aspetta click del player
                    SelectUnit(MovingUnit);
                }
            }
        }

        MovingUnit = nullptr;
        MovementPath.Empty();
        CurrentPathStep = 0;
        return;
    }

    //sposta l'unità alla cella successiva nel path
    FIntPoint NextPos = MovementPath[CurrentPathStep++];
    AGridCell* NextCell = GridManagerRef->GetCell(NextPos.X, NextPos.Y);
    if (!NextCell) return;

    FVector NewWorldPos = NextCell->GetActorLocation();
    NewWorldPos.Z += 60.f;
    MovingUnit->SetActorLocation(NewWorldPos);
    MovingUnit->GridX = NextPos.X;
    MovingUnit->GridY = NextPos.Y;

    UE_LOG(LogTemp, Warning, TEXT("Unit step -> (%d,%d)"), NextPos.X, NextPos.Y);
}

void ATurnBasedGameMode::HumanEndTurn()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS || GS->CurrentTurn != ETurnOwner::Human) return;

    //forza la fine del turno human indipendentemente dalle azioni fatte
    DeselectUnit();
    for (ABaseUnit* Unit : GS->HumanUnits)
        if (Unit) Unit->ResetTurnState();
    HumanUnitsActedSet.Empty();
    EndTurn();
}

void ATurnBasedGameMode::HumanConfirmPosition()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS || GS->CurrentTurn != ETurnOwner::Human) return;
    if (!SelectedUnit) return;

    SelectedUnit->bHasMoved = true;
    ClearMovementRange();

    ShowAttackRange(SelectedUnit);

    if (AttackableTargets.IsEmpty())
    {
        //nessun nemico attaccabile: unità ha finito le sue azioni
        ClearAttackRange();
        SelectedUnit->bHasAttacked = true;
        HumanUnitsActedSet.Add(SelectedUnit);
        ABaseUnit* Done = SelectedUnit;
        DeselectUnit();
        if (HumanUnitsActedSet.Num() >= GS->HumanUnits.Num())
            EndTurn();
    }
    //altrimenti rimane selezionata e mostra il range arancione
}

void ATurnBasedGameMode::StartAITurn()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    AIUnitQueue.Empty();
    for (ABaseUnit* Unit : GS->AIUnits)
        if (Unit && Unit->IsAlive()) AIUnitQueue.Add(Unit);

    UE_LOG(LogTemp, Warning, TEXT("AI turn started - %d units"), AIUnitQueue.Num());

    FTimerHandle DelayTimer;
    GetWorldTimerManager().SetTimer(DelayTimer, this, &ATurnBasedGameMode::ProcessNextAIUnit, 0.8f, false);
}

void ATurnBasedGameMode::ProcessNextAIUnit()
{
    UE_LOG(LogTemp, Warning, TEXT("ProcessNextAIUnit: queue size = %d"), AIUnitQueue.Num());
    if (AIUnitQueue.IsEmpty())
    {
        EndTurn();
        return;
    }

    ABaseUnit* Unit = AIUnitQueue[0];
    AIUnitQueue.RemoveAt(0);

    if (!Unit || !Unit->IsAlive())
    {
        ProcessNextAIUnit();
        return;
    }

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    TMap<FIntPoint, int32> Reachable = Pathfinding->GetReachableCells(
        GridManagerRef, FIntPoint(Unit->GridX, Unit->GridY), Unit->MaxMovement);

    FIntPoint BestTarget(-1, -1);
    int32 BestScore = INT_MAX;

    //priorità 1: torre neutrale — cerca la cella raggiungibile più vicina a una torre libera
    for (ATower* Tower : GridManagerRef->GetTowers())
    {
        if (!Tower || Tower->OwnerPlayer != ETowerOwner::None) continue;
        FIntPoint TowerPos(Tower->GridX, Tower->GridY);

        for (auto& Pair : Reachable)
        {
            int32 Dist = FMath::Abs(Pair.Key.X - TowerPos.X) + FMath::Abs(Pair.Key.Y - TowerPos.Y);
            if (Dist < BestScore) { BestScore = Dist; BestTarget = Pair.Key; }
        }
    }

    //priorità 2: se nessuna torre libera, muoviti verso il nemico più vicino
    if (BestTarget == FIntPoint(-1, -1))
    {
        ABaseUnit* NearestEnemy = nullptr;
        int32 MinDist = INT_MAX;
        for (ABaseUnit* Enemy : GS->HumanUnits)
        {
            if (!Enemy || !Enemy->IsAlive()) continue;
            int32 Dist = FMath::Abs(Unit->GridX - Enemy->GridX) + FMath::Abs(Unit->GridY - Enemy->GridY);
            if (Dist < MinDist) { MinDist = Dist; NearestEnemy = Enemy; }
        }

        if (NearestEnemy)
        {
            FIntPoint EnemyPos(NearestEnemy->GridX, NearestEnemy->GridY);
            for (auto& Pair : Reachable)
            {
                int32 Dist = FMath::Abs(Pair.Key.X - EnemyPos.X) + FMath::Abs(Pair.Key.Y - EnemyPos.Y);
                if (Dist < BestScore) { BestScore = Dist; BestTarget = Pair.Key; }
            }
        }
    }

    bIsAIMoving = true;
    FIntPoint CurrentPos(Unit->GridX, Unit->GridY);
    if (BestTarget != FIntPoint(-1, -1) && BestTarget != CurrentPos)
    {
        TArray<FIntPoint> Path = Pathfinding->FindPath(
            GridManagerRef, CurrentPos, BestTarget, Unit->MaxMovement);

        if (!Path.IsEmpty())
        {
            StartStepMovement(Unit, Path);
            return;
        }
    }

    //nessun movimento possibile: prova solo ad attaccare
    bIsAIMoving = false;
    AIAttackIfPossible(Unit);
}

void ATurnBasedGameMode::AIAttackIfPossible(ABaseUnit* Unit)
{
    if (!Unit) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    for (ABaseUnit* Enemy : GS->HumanUnits)
    {
        if (!Enemy || !Enemy->IsAlive()) continue;
        if (IAttackable::Execute_IsTargetInRange(Unit, Enemy))
        {
            IAttackable::Execute_PerformAttack(Unit, Enemy);
            UE_LOG(LogTemp, Warning, TEXT("AI unit attacked enemy at (%d,%d)"), Enemy->GridX, Enemy->GridY);
            break;
        }
    }

    FTimerHandle DelayTimer;
    GetWorldTimerManager().SetTimer(DelayTimer, this, &ATurnBasedGameMode::ProcessNextAIUnit, 0.5f, false);
}

void ATurnBasedGameMode::ShowAttackRange(ABaseUnit* Unit)
{
    ClearAttackRange();
    if (!Unit || !GridManagerRef) return;

    AGridCell* MyCell = GridManagerRef->GetCell(Unit->GridX, Unit->GridY);
    if (!MyCell) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    for (int32 X = 0; X < 25; X++)
    {
        for (int32 Y = 0; Y < 25; Y++)
        {
            AGridCell* Cell = GridManagerRef->GetCell(X, Y);
            if (!Cell) continue;
            if (X == Unit->GridX && Y == Unit->GridY) continue;
            if (Cell->ElevationLevel > MyCell->ElevationLevel) continue;

            int32 Dist = FMath::Abs(X - Unit->GridX) + FMath::Abs(Y - Unit->GridY);

            if (Unit->AttackType == EAttackType::Melee && Dist != 1) continue;
            if (Unit->AttackType == EAttackType::Ranged && Dist > Unit->AttackRange) continue;

            UMaterialInstanceDynamic* DynMat = Cell->GetCellDynMat();
            if (DynMat)
                DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.3f, 0.f));

            MovementHighlightedCells.Add(Cell);
        }
    }

    //popola AttackableTargets per sapere quali nemici sono cliccabili
    TArray<ABaseUnit*>& Enemies = (Unit->UnitOwner == EOwner::Human) ? GS->AIUnits : GS->HumanUnits;
    for (ABaseUnit* Enemy : Enemies)
    {
        if (!Enemy || !Enemy->IsAlive()) continue;
        if (!IAttackable::Execute_IsTargetInRange(Unit, Enemy)) continue;
        AttackableTargets.Add(Enemy);
    }
}

void ATurnBasedGameMode::ClearAttackRange()
{
    for (ABaseUnit* Enemy : AttackableTargets)
    {
        if (!Enemy) continue;
        AGridCell* Cell = GridManagerRef->GetCell(Enemy->GridX, Enemy->GridY);
        if (Cell) Cell->UpdateVisualColor();
    }
    AttackableTargets.Empty();
}

void ATurnBasedGameMode::ExecuteAttack(ABaseUnit* Attacker, ABaseUnit* Target)
{
    if (!Attacker || !Target) return;

    IAttackable::Execute_PerformAttack(Attacker, Target);
    Attacker->bHasAttacked = true;

    ClearMovementRange(); //rimuove highlight arancione e cyan
    ClearAttackRange();   //svuota AttackableTargets

    HumanUnitsActedSet.Add(Attacker);

    if (HumanUnitsActedSet.Num() >= GetTurnGameState()->HumanUnits.Num())
        EndTurn();
}

void ATurnBasedGameMode::CheckGameOver()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    //controlla se tutte le unità di un giocatore sono morte
    bool bAllHumanDead = GS->HumanUnits.Num() > 0 && GS->HumanUnits.FilterByPredicate(
        [](ABaseUnit* U) { return U && U->IsAlive(); }).Num() == 0;

    bool bAllAIDead = GS->AIUnits.Num() > 0 && GS->AIUnits.FilterByPredicate(
        [](ABaseUnit* U) { return U && U->IsAlive(); }).Num() == 0;

    if (bAllHumanDead)
    {
        GS->CurrentPhase = EGamePhase::GameOver;
        UE_LOG(LogTemp, Warning, TEXT("GAME OVER - Winner: AI (all human units eliminated)"));
        return;
    }

    if (bAllAIDead)
    {
        GS->CurrentPhase = EGamePhase::GameOver;
        UE_LOG(LogTemp, Warning, TEXT("GAME OVER - Winner: Human (all AI units eliminated)"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("CheckGameOver: HumanConsec=%d AIConsec=%d"),
        GS->HumanConsecutiveTowerTurns, GS->AIConsecutiveTowerTurns);

    ETurnOwner Winner;
    if (GS->CheckWinCondition(Winner))
    {
        GS->CurrentPhase = EGamePhase::GameOver;
        UE_LOG(LogTemp, Warning, TEXT("GAME OVER - Winner: %s"),
            Winner == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"));
    }
}

ATurnBasedGameState* ATurnBasedGameMode::GetTurnGameState() const
{
    return GetGameState<ATurnBasedGameState>();
}
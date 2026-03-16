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
    FString Winner = bHumanWins ? TEXT("HP") : TEXT("AI");
    UE_LOG(LogTemp, Warning, TEXT("Coin flip: %s wins!"), *Winner);

    //mostra risultato nel widget già in viewport
    if (CoinFlipWidgetRef)
        CoinFlipWidgetRef->ShowResult(Winner);

    //avvia placement dopo 3 secondi (stesso delay di HideWidget)
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

void ATurnBasedGameMode::LogMove(const FString& Entry)
{
    UE_LOG(LogTemp, Warning, TEXT("Move: %s"), *Entry);
    if (GameHUDWidgetRef)
        GameHUDWidgetRef->AddMoveEntry(Entry);
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
    WorldPos.Z += 20.f;

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
    if (HumanUnitsPlaced == 0)
    {
        PlacementWidgetRef->ShowUnitSelection();
    }
    else
    {
        EAttackType Remaining = (SelectedUnitType == EAttackType::Sniper)
            ? EAttackType::Brawler : EAttackType::Sniper;
        FString UnitName = (Remaining == EAttackType::Sniper) ? TEXT("Sniper") : TEXT("Brawler");
        SelectedUnitType = Remaining;
        PlacementWidgetRef->ShowPlacementPrompt(UnitName);
    }
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
    WorldPos.Z += 20.f;

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

    RefreshHUD();
    
}

void ATurnBasedGameMode::OnHumanUnitTypeSelected(EAttackType SelectedType)
{
    FString UnitName = (SelectedType == EAttackType::Sniper) ? TEXT("Sniper") : TEXT("Brawler");
    SelectedUnitType = SelectedType;
    if (PlacementWidgetRef)
        PlacementWidgetRef->ShowPlacementPrompt(UnitName);
    HighlightHumanPlacementZone();
}

void ATurnBasedGameMode::RefreshHUD()
{
    if (!GameHUDWidgetRef) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    FString TurnText = (GS->CurrentTurn == ETurnOwner::Human)
        ? TEXT("Turn: HP") : TEXT("Turn: AI");

    //costruisci stringa HP unità Human
    FString HumanText = TEXT("HP:\n");
    for (ABaseUnit* Unit : GS->HumanUnits)
    {
        if (!Unit) continue;
        FString UnitName = Unit->IsA<ASniper>() ? TEXT("Sniper") : TEXT("Brawler");
        HumanText += FString::Printf(TEXT("%s HP: %d/%d\n"),
            *UnitName, Unit->CurrentHP, Unit->MaxHP);
    }

    //costruisci stringa HP unità AI
    FString AIText = TEXT("AI:\n");
    for (ABaseUnit* Unit : GS->AIUnits)
    {
        if (!Unit) continue;
        FString UnitName = Unit->IsA<ASniper>() ? TEXT("Sniper") : TEXT("Brawler");
        AIText += FString::Printf(TEXT("%s HP: %d/%d\n"),
            *UnitName, Unit->CurrentHP, Unit->MaxHP);
    }

    GameHUDWidgetRef->UpdateHUD(TurnText, HumanText, AIText,
        GS->HumanTowersControlled, GS->AITowersControlled);
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
                if (Unit->bHasMoved && AttackableTargets.IsEmpty())
                {
                    DeselectUnit();
                    HumanUnitsActedSet.Add(Unit);
                    ATurnBasedGameState* GS2 = GetTurnGameState();
                    int32 AliveHuman = 0;
                    for (ABaseUnit* U : GS2->HumanUnits)
                        if (U && U->IsAlive()) AliveHuman++;
                    if (HumanUnitsActedSet.Num() >= AliveHuman)
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
    HumanUnitsActedSet.Empty();
    TArray<ABaseUnit*>& CurrentUnits = (GS->CurrentTurn == ETurnOwner::Human)
        ? GS->HumanUnits : GS->AIUnits;
    for (ABaseUnit* Unit : CurrentUnits)
        if (Unit) Unit->ResetTurnState();
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
    RefreshHUD();
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
    //formato: HP/AI: S/B CellOrig -> CellDest
    ATurnBasedGameState* GS = GetTurnGameState();
    FString Player = (Unit->UnitOwner == EOwner::Human) ? TEXT("HP") : TEXT("AI");
    FString UnitType = Unit->IsA<ASniper>() ? TEXT("S") : TEXT("B");
    FString From = FString::Printf(TEXT("%c%d"), (TCHAR)('A' + Unit->GridX), Unit->GridY);
    FString To = FString::Printf(TEXT("%c%d"), (TCHAR)('A' + DestX), DestY);
    LogMove(FString::Printf(TEXT("%s: %s %s -> %s"), *Player, *UnitType, *From, *To));
    StartStepMovement(Unit, Path);
}

void ATurnBasedGameMode::StartStepMovement(ABaseUnit* Unit, TArray<FIntPoint> Path)
{
    MovingUnit = Unit;
    MovementPath = Path;
    CurrentPathStep = 0;
    StartGridX = Unit->GridX;
    StartGridY = Unit->GridY;

    //libera la cella di partenza
    AGridCell* OldCell = GridManagerRef->GetCell(Unit->GridX, Unit->GridY);
    if (OldCell) OldCell->bIsOccupied = false;

    //se è l'AI, mostra il range di movimento in cyan
    if (bIsAIMoving)
        ShowMovementRange(Unit);

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
                //logga il movimento AI
                FString From = FString::Printf(TEXT("%c%d"), 'A' + StartGridX, StartGridY);
                FString To = FString::Printf(TEXT("%c%d"), 'A' + MovingUnit->GridX, MovingUnit->GridY);
                FString UnitType = MovingUnit->IsA<ASniper>() ? TEXT("S") : TEXT("B");
                LogMove(FString::Printf(TEXT("AI: %s %s -> %s"), *UnitType, *From, *To));
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
                    ClearMovementRange();
                    MovingUnit->bHasAttacked = true;
                    HumanUnitsActedSet.Add(MovingUnit);
                    MovingUnit = nullptr;
                    MovementPath.Empty();
                    CurrentPathStep = 0;
                    ATurnBasedGameState* GS = GetTurnGameState();
                    int32 AliveHuman = 0;
                    for (ABaseUnit* U : GS->HumanUnits)
                        if (U && U->IsAlive()) AliveHuman++;
                    if (HumanUnitsActedSet.Num() >= AliveHuman)
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
    NewWorldPos.Z += 20.f;
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
    if (SelectedUnit->bHasMoved && SelectedUnit->bHasAttacked) return; // unità ha già esaurito entrambe le azioni
    SelectedUnit->bHasMoved = true;
    ClearMovementRange();
    ShowAttackRange(SelectedUnit);
    if (AttackableTargets.IsEmpty())
    {
        ClearAttackRange();
        SelectedUnit->bHasAttacked = true;
        HumanUnitsActedSet.Add(SelectedUnit);
        ABaseUnit* Done = SelectedUnit;
        DeselectUnit();
        int32 AliveHuman = 0;
        for (ABaseUnit* U : GS->HumanUnits)
            if (U && U->IsAlive()) AliveHuman++;
        if (HumanUnitsActedSet.Num() >= AliveHuman)
            EndTurn();
    }
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
    GetWorldTimerManager().SetTimer(DelayTimer, this, &ATurnBasedGameMode::ProcessNextAIUnitHeuristic, 0.8f, false);
}

void ATurnBasedGameMode::ProcessNextAIUnitHeuristic()
{
    if (AIUnitQueue.IsEmpty()) { EndTurn(); return; }

    ABaseUnit* Unit = AIUnitQueue[0];
    AIUnitQueue.RemoveAt(0);

    if (!Unit || !Unit->IsAlive()) { ProcessNextAIUnitHeuristic(); return; }

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    TMap<FIntPoint, int32> Reachable = Pathfinding->GetReachableCells(
        GridManagerRef, FIntPoint(Unit->GridX, Unit->GridY), Unit->MaxMovement);

    bool bEnemyHasTwoTowers = (GS->HumanTowersControlled >= 2);
    bool bLowHP = (Unit->CurrentHP < Unit->MaxHP * 0.5f);
    bool bIsSniper = (Unit->AttackType == EAttackType::Sniper);

    // controlla se l'unità è già nella zona di cattura di una torre contestata
    bool bAlreadyInContestedZone = false;
    if (bLowHP && !bIsSniper)
    {
        for (ATower* Tower : GridManagerRef->GetTowers())
        {
            if (!Tower) continue;
            int32 DX = FMath::Abs(Unit->GridX - Tower->GridX);
            int32 DY = FMath::Abs(Unit->GridY - Tower->GridY);
            if (FMath::Max(DX, DY) <= Tower->CaptureRadius)
            {
                bAlreadyInContestedZone = true;
                break;
            }
        }
    }

    // se Brawler con HP basse è già nella zona: rimane fermo e attacca
    if (bAlreadyInContestedZone)
    {
        bIsAIMoving = false;
        AIAttackIfPossible(Unit);
        return;
    }

    // trova il nemico più debole (HP più basse)
    ABaseUnit* WeakestEnemy = nullptr;
    int32 MinHP = INT_MAX;
    for (ABaseUnit* Enemy : GS->HumanUnits)
    {
        if (!Enemy || !Enemy->IsAlive()) continue;
        if (Enemy->CurrentHP < MinHP) { MinHP = Enemy->CurrentHP; WeakestEnemy = Enemy; }
    }

    // controlla se lo Sniper nemico è nella zona di una torre da contestare
    bool bSniperInTowerZone = false;
    ATower* TargetTowerWithSniper = nullptr;
    if (bLowHP && !bIsSniper && WeakestEnemy && WeakestEnemy->AttackType == EAttackType::Sniper)
    {
        for (ATower* Tower : GridManagerRef->GetTowers())
        {
            if (!Tower) continue;
            int32 DX = FMath::Abs(WeakestEnemy->GridX - Tower->GridX);
            int32 DY = FMath::Abs(WeakestEnemy->GridY - Tower->GridY);
            if (FMath::Max(DX, DY) <= Tower->CaptureRadius)
            {
                bSniperInTowerZone = true;
                TargetTowerWithSniper = Tower;
                break;
            }
        }
    }

    FIntPoint BestTarget(-1, -1);
    float BestScore = FLT_MAX;

    for (auto& Pair : Reachable)
    {
        if (Pair.Key == FIntPoint(Unit->GridX, Unit->GridY)) continue;
        float Score = 0.f;

        // distanza reale dal nemico più debole
        float WeakEnemyDist = FLT_MAX;
        if (WeakestEnemy)
        {
            int32 D = Pathfinding->GetActualDistance(GridManagerRef, Pair.Key, FIntPoint(WeakestEnemy->GridX, WeakestEnemy->GridY));
            WeakEnemyDist = (D >= 0) ? (float)D : 9999.f;
        }

        // distanza reale dal nemico più vicino
        float MinEnemyDist = FLT_MAX;
        for (ABaseUnit* Enemy : GS->HumanUnits)
        {
            if (!Enemy || !Enemy->IsAlive()) continue;
            int32 D = Pathfinding->GetActualDistance(GridManagerRef, Pair.Key, FIntPoint(Enemy->GridX, Enemy->GridY));
            float RealD = (D >= 0) ? (float)D : 9999.f;
            if (RealD < MinEnemyDist) MinEnemyDist = RealD;
        }

        if (bEnemyHasTwoTowers)
        {
            float MinTowerDist = FLT_MAX;
            for (ATower* Tower : GridManagerRef->GetTowers())
            {
                if (!Tower || Tower->OwnerPlayer != ETowerOwner::Human) continue;
                int32 D = Pathfinding->GetActualDistance(GridManagerRef, Pair.Key, FIntPoint(Tower->GridX, Tower->GridY));
                float RealD = (D >= 0) ? (float)D : 9999.f;
                if (RealD < MinTowerDist) MinTowerDist = RealD;
            }
            Score = MinTowerDist * 3.f;
        }
        else if (bLowHP && !bIsSniper && bSniperInTowerZone && TargetTowerWithSniper)
        {
            int32 D = Pathfinding->GetActualDistance(GridManagerRef, Pair.Key, FIntPoint(TargetTowerWithSniper->GridX, TargetTowerWithSniper->GridY));
            float TowerDist = (D >= 0) ? (float)D : 9999.f;
            Score = TowerDist * 2.f + WeakEnemyDist * 1.f;
        }
        else if (bLowHP && !bIsSniper)
        {
            Score = WeakEnemyDist * 2.f + Pair.Value * 1.f;
        }
        else
        {
            float MinNeutralTowerDist = FLT_MAX;
            for (ATower* Tower : GridManagerRef->GetTowers())
            {
                if (!Tower || Tower->OwnerPlayer != ETowerOwner::None) continue;
                int32 D = Pathfinding->GetActualDistance(GridManagerRef, Pair.Key, FIntPoint(Tower->GridX, Tower->GridY));
                float RealD = (D >= 0) ? (float)D : 9999.f;
                if (RealD < MinNeutralTowerDist) MinNeutralTowerDist = RealD;
            }
            Score += MinNeutralTowerDist * 2.f;
            if (bIsSniper)
            {
                float TargetDist = (float)Unit->AttackRange;
                Score += FMath::Abs(MinEnemyDist - TargetDist) * 1.5f;
            }
            else
            {
                Score += WeakEnemyDist * 1.f;
            }
            Score += Pair.Value * 1.f;
        }

        // bonus forte se da questa cella posso attaccare direttamente un nemico (Manhattan ok: è range d'attacco)
        for (ABaseUnit* Enemy : GS->HumanUnits)
        {
            if (!Enemy || !Enemy->IsAlive()) continue;
            int32 AttackDist = FMath::Abs(Pair.Key.X - Enemy->GridX) + FMath::Abs(Pair.Key.Y - Enemy->GridY);
            if (Unit->AttackType == EAttackType::Brawler && AttackDist == 1)
            {
                Score -= 10.f; break;
            }
            if (Unit->AttackType == EAttackType::Sniper && AttackDist <= Unit->AttackRange)
            {
                Score -= 10.f; break;
            }
        }

        if (Score < BestScore) { BestScore = Score; BestTarget = Pair.Key; }
    }

    bIsAIMoving = true;
    FIntPoint CurrentPos(Unit->GridX, Unit->GridY);
    if (BestTarget != FIntPoint(-1, -1) && BestTarget != CurrentPos)
    {
        TArray<FIntPoint> Path = Pathfinding->FindPath(
            GridManagerRef, CurrentPos, BestTarget, Unit->MaxMovement);
        if (!Path.IsEmpty()) { StartStepMovement(Unit, Path); return; }
    }

    bIsAIMoving = false;
    AIAttackIfPossible(Unit);
}

void ATurnBasedGameMode::AIAttackIfPossible(ABaseUnit* Unit)
{
    if (!Unit) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    ClearMovementRange();
    ShowAttackRange(Unit);

    for (ABaseUnit* Enemy : GS->HumanUnits)
    {
        if (!Enemy || !Enemy->IsAlive()) continue;
        if (IAttackable::Execute_IsTargetInRange(Unit, Enemy))
        {
            FString UnitType = Unit->IsA<ASniper>() ? TEXT("S") : TEXT("B");
            FString TargetCell = FString::Printf(TEXT("%c%d"), (TCHAR)('A' + Enemy->GridX), Enemy->GridY);
            int32 HPBefore = Unit->CurrentHP;
            int32 Dmg = IAttackable::Execute_PerformAttack(Unit, Enemy);
            LogMove(FString::Printf(TEXT("AI: %s %s %d"), *UnitType, *TargetCell, Dmg));
            if (Unit->CurrentHP < HPBefore)
            {
                FString AttackerCell = FString::Printf(TEXT("%c%d"), (TCHAR)('A' + Unit->GridX), Unit->GridY);
                LogMove(FString::Printf(TEXT("Counter: %s %s"), *UnitType, *AttackerCell));
            }
            RefreshHUD();
            UE_LOG(LogTemp, Warning, TEXT("AI unit attacked enemy at (%d,%d)"), Enemy->GridX, Enemy->GridY);
            break;
        }
    }

    FTimerHandle DelayTimer;
    GetWorldTimerManager().SetTimer(DelayTimer, [this]()
        {
            ClearMovementRange();
            ClearAttackRange();
            ProcessNextAIUnitHeuristic();
        }, 0.8f, false);
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
            if (Unit->AttackType == EAttackType::Brawler && Dist != 1) continue;
            if (Unit->AttackType == EAttackType::Sniper && Dist > Unit->AttackRange) continue;

            //line-of-sight per Sniper
            if (Unit->AttackType == EAttackType::Sniper)
            {
                int32 DX = X - Unit->GridX;
                int32 DY = Y - Unit->GridY;
                int32 Steps = FMath::Max(FMath::Abs(DX), FMath::Abs(DY));
                bool bBlocked = false;

                for (int32 i = 1; i < Steps; i++)
                {
                    int32 CX = Unit->GridX + FMath::RoundToInt((float)DX * i / Steps);
                    int32 CY = Unit->GridY + FMath::RoundToInt((float)DY * i / Steps);
                    AGridCell* Mid = GridManagerRef->GetCell(CX, CY);
                    if (Mid && Mid->ElevationLevel > MyCell->ElevationLevel)
                    {
                        bBlocked = true;
                        break;
                    }
                }
                if (bBlocked) continue;
            }

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
    FString Player = (Attacker->UnitOwner == EOwner::Human) ? TEXT("HP") : TEXT("AI");
    FString UnitType = Attacker->IsA<ASniper>() ? TEXT("S") : TEXT("B");
    FString TargetCell = FString::Printf(TEXT("%c%d"), (TCHAR)('A' + Target->GridX), Target->GridY);
    int32 HPBefore = Attacker->CurrentHP;
    int32 Dmg = IAttackable::Execute_PerformAttack(Attacker, Target);
    LogMove(FString::Printf(TEXT("%s: %s %s %d"), *Player, *UnitType, *TargetCell, Dmg));
    if (Attacker->CurrentHP < HPBefore)
    {
        FString AttackerCell = FString::Printf(TEXT("%c%d"), (TCHAR)('A' + Attacker->GridX), Attacker->GridY);
        LogMove(FString::Printf(TEXT("Counter: %s %s"), *UnitType, *AttackerCell));
    }
    RefreshHUD();
    Attacker->bHasAttacked = true;
    ClearMovementRange();
    ClearAttackRange();
    HumanUnitsActedSet.Add(Attacker);
    int32 AliveHuman = 0;
    for (ABaseUnit* U : GetTurnGameState()->HumanUnits)
        if (U && U->IsAlive()) AliveHuman++;
    if (HumanUnitsActedSet.Num() >= AliveHuman)
        EndTurn();
}

void ATurnBasedGameMode::CheckGameOver()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

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


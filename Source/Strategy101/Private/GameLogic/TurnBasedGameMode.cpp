#include "GameLogic/TurnBasedGameMode.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MapConfigWidget.h"
#include "Grid/GridManager.h"
#include "Units/Sniper.h"
#include "Units/Brawler.h"
#include "UI/PlacementWidget.h"
#include "EngineUtils.h"

ATurnBasedGameMode::ATurnBasedGameMode()
{
    GameStateClass = ATurnBasedGameState::StaticClass();
    DefaultPawnClass = nullptr;
    HumanUnitsPlaced = 0;
    AIUnitsPlaced = 0;
    PlacementTurn = ETurnOwner::Human;
    GridManagerRef = nullptr;
}

void ATurnBasedGameMode::BeginPlay()
{
    Super::BeginPlay();

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

            if (!MapConfigWidgetClass || !GridManagerRef) return;

            UMapConfigWidget* Widget = CreateWidget<UMapConfigWidget>(PC, MapConfigWidgetClass);
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

    // Prima unità AI = Sniper, seconda = Brawler
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

void ATurnBasedGameMode::HighlightHumanPlacementZone()
{
    if (!GridManagerRef) return;
    ClearHighlight();

    // Evidenzia celle valide in Y=0,1,2 non occupate e non acqua
    for (int32 Y = 0; Y <= 2; Y++)
    {
        for (int32 X = 0; X < 25; X++)
        {
            AGridCell* Cell = GridManagerRef->GetCell(X, Y);
            if (Cell && Cell->ElevationLevel > 0 && !Cell->bIsOccupied)
            {
                // Colore ciano per indicare cella disponibile
                UMaterialInstanceDynamic* DynMat =
                    Cell->GetCellMesh()->CreateAndSetMaterialInstanceDynamic(0);
                if (DynMat)
                    DynMat->SetVectorParameterValue(TEXT("BaseColor"),
                        FLinearColor(0.f, 1.f, 1.f));
                HighlightedCells.Add(Cell);
            }
        }
    }
}

void ATurnBasedGameMode::ClearHighlight()
{
    // Ripristina il colore originale di ogni cella evidenziata
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

    // Prima unità = Sniper, seconda = Brawler
    FString UnitName = (HumanUnitsPlaced == 0) ? TEXT("Sniper") : TEXT("Brawler");
    if (PlacementWidgetRef) PlacementWidgetRef->ShowPlacementPrompt(UnitName);

    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
}

void ATurnBasedGameMode::SpawnUnitAtCell(AGridCell* Cell)
{
    if (!Cell || !GridManagerRef) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    // Determina quale classe spawnare
    TSubclassOf<ABaseUnit> ClassToSpawn = (HumanUnitsPlaced == 0)
        ? TSubclassOf<ABaseUnit>(SniperClass)
        : TSubclassOf<ABaseUnit>(BrawlerClass);

    if (!ClassToSpawn) return;

    FVector WorldPos = Cell->GetActorLocation();
    WorldPos.Z += 60.f; // solleva l'unità sopra la cella

    FActorSpawnParameters Params;
    Params.Owner = this;
    ABaseUnit* Unit = GetWorld()->SpawnActor<ABaseUnit>(ClassToSpawn, WorldPos, FRotator::ZeroRotator, Params);
    if (!Unit) return;

    // Inizializza l'unità
    Unit->GridX = Cell->GridX;
    Unit->GridY = Cell->GridY;
    Unit->SpawnGridX = Cell->GridX;
    Unit->SpawnGridY = Cell->GridY;
    Unit->UnitOwner = EOwner::Human;
    Cell->bIsOccupied = true;

    GS->HumanUnits.Add(Unit);
    HumanUnitsPlaced++;

    UE_LOG(LogTemp, Warning, TEXT("Human spawned unit %d at (%d,%d)"), HumanUnitsPlaced, Cell->GridX, Cell->GridY);

    ClearHighlight();
    AdvancePlacementStep();
}

void ATurnBasedGameMode::StartGamePhase()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    GS->CurrentPhase = EGamePhase::Playing;
    GS->CurrentTurn = GS->CoinFlipWinner;

    UE_LOG(LogTemp, Warning, TEXT("Game phase started! First turn: %s"),
        GS->CurrentTurn == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"));
}

bool ATurnBasedGameMode::IsPlacementComplete() const
{
    return (HumanUnitsPlaced + AIUnitsPlaced) >= 4;
}

void ATurnBasedGameMode::EndTurn()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    TArray<ABaseUnit*>& CurrentUnits = (GS->CurrentTurn == ETurnOwner::Human)
        ? GS->HumanUnits : GS->AIUnits;

    for (ABaseUnit* Unit : CurrentUnits)
        if (Unit) Unit->ResetTurnState();

    CheckGameOver();
    GS->SwitchTurn();

    UE_LOG(LogTemp, Warning, TEXT("Turn %d — Now: %s"), GS->TurnNumber,
        GS->CurrentTurn == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"));

    if (GS->CurrentTurn == ETurnOwner::AI)
        StartAITurn();
}

void ATurnBasedGameMode::StartAITurn()
{
    // Placeholder — implementato al Giorno 19
    UE_LOG(LogTemp, Warning, TEXT("AI turn started"));
}

void ATurnBasedGameMode::CheckGameOver()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    ETurnOwner Winner;
    if (GS->CheckWinCondition(Winner))
    {
        GS->CurrentPhase = EGamePhase::GameOver;
        UE_LOG(LogTemp, Warning, TEXT("GAME OVER — Winner: %s"),
            Winner == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"));
    }
}

ATurnBasedGameState* ATurnBasedGameMode::GetTurnGameState() const
{
    return GetGameState<ATurnBasedGameState>();
}


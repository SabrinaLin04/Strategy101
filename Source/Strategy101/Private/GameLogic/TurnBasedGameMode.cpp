#include "GameLogic/TurnBasedGameMode.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MapConfigWidget.h"
#include "Grid/GridManager.h"
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

    UE_LOG(LogTemp, Warning, TEXT("Coin flip: %s wins!"),
        bHumanWins ? TEXT("Human") : TEXT("AI"));

    if (PlacementTurn == ETurnOwner::AI)
        PerformAIPlacement();
}

void ATurnBasedGameMode::OnHumanPlacementCellClicked(int32 X, int32 Y)
{
    if (!GridManagerRef) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS || GS->CurrentPhase != EGamePhase::Placement) return;
    if (GS->CurrentTurn != ETurnOwner::Human) return;

    // Zone valide per Human: Y = 0, 1, 2
    if (Y < 0 || Y > 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid placement zone (Y must be 0-2)"));
        return;
    }

    AGridCell* Cell = GridManagerRef->GetCell(X, Y);
    if (!Cell || Cell->ElevationLevel == 0 || Cell->bIsOccupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell (%d,%d) not available"), X, Y);
        return;
    }

    // Lo spawn effettivo dell'unità sarà al Giorno 12
    Cell->bIsOccupied = true;
    HumanUnitsPlaced++;

    UE_LOG(LogTemp, Warning, TEXT("Human placed unit %d at (%d,%d)"), HumanUnitsPlaced, X, Y);

    AdvancePlacementStep();
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
    Chosen->bIsOccupied = true;
    AIUnitsPlaced++;

    UE_LOG(LogTemp, Warning, TEXT("AI placed unit %d at (%d,%d)"), AIUnitsPlaced, Chosen->GridX, Chosen->GridY);

    AdvancePlacementStep();
}

void ATurnBasedGameMode::AdvancePlacementStep()
{
    if ((HumanUnitsPlaced + AIUnitsPlaced) >= 4)
    {
        StartGamePhase();
        return;
    }

    PlacementTurn = (PlacementTurn == ETurnOwner::Human) ? ETurnOwner::AI : ETurnOwner::Human;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (GS) GS->CurrentTurn = PlacementTurn;

    if (PlacementTurn == ETurnOwner::AI)
        PerformAIPlacement();
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
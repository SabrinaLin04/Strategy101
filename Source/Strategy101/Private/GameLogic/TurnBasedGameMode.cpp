#include "GameLogic/TurnBasedGameMode.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ATurnBasedGameMode::ATurnBasedGameMode()
{
    // Imposta il GameState personalizzato
    GameStateClass = ATurnBasedGameState::StaticClass();

    UnitsPlacedCount = 0;
    PlacementTurn = ETurnOwner::Human;
}

void ATurnBasedGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Avvia la partita con il coin flip
    PerformCoinFlip();
}

void ATurnBasedGameMode::PerformCoinFlip()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    // 50/50 tra Human e AI
    bool bHumanWins = FMath::RandBool();
    GS->CoinFlipWinner = bHumanWins ? ETurnOwner::Human : ETurnOwner::AI;
    GS->CurrentTurn = GS->CoinFlipWinner;
    PlacementTurn = GS->CoinFlipWinner;

    UE_LOG(LogTemp, Warning, TEXT("Coin flip: %s wins!"),
        bHumanWins ? TEXT("Human") : TEXT("AI"));
}

void ATurnBasedGameMode::OnUnitPlaced(ABaseUnit* PlacedUnit, int32 GridX, int32 GridY)
{
    if (!PlacedUnit) return;

    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    // Registra l'unità nel GameState
    if (PlacementTurn == ETurnOwner::Human)
    {
        GS->HumanUnits.Add(PlacedUnit);
        PlacedUnit->UnitOwner = EOwner::Human;
    }
    else
    {
        GS->AIUnits.Add(PlacedUnit);
        PlacedUnit->UnitOwner = EOwner::AI;
    }

    // Salva posizione di spawn per il respawn futuro
    PlacedUnit->SpawnGridX = GridX;
    PlacedUnit->SpawnGridY = GridY;
    PlacedUnit->GridX = GridX;
    PlacedUnit->GridY = GridY;

    UnitsPlacedCount++;

    // Alterna il turno di placement
    PlacementTurn = (PlacementTurn == ETurnOwner::Human)
        ? ETurnOwner::AI : ETurnOwner::Human;

    // Se tutte e 4 le unità sono piazzate, inizia la partita
    if (IsPlacementComplete())
    {
        GS->CurrentPhase = EGamePhase::Playing;
        GS->CurrentTurn = GS->CoinFlipWinner;
        UE_LOG(LogTemp, Warning, TEXT("Placement complete. Game starts!"));
    }
}

bool ATurnBasedGameMode::IsPlacementComplete() const
{
    return UnitsPlacedCount >= 4; // 2 Human + 2 AI
}

void ATurnBasedGameMode::EndTurn()
{
    ATurnBasedGameState* GS = GetTurnGameState();
    if (!GS) return;

    // Resetta lo stato azione di tutte le unità del giocatore che ha appena giocato
    TArray<ABaseUnit*>& CurrentUnits = (GS->CurrentTurn == ETurnOwner::Human)
        ? GS->HumanUnits : GS->AIUnits;

    for (ABaseUnit* Unit : CurrentUnits)
    {
        if (Unit) Unit->ResetTurnState();
    }

    // Controlla vittoria prima di passare il turno
    CheckGameOver();

    // Passa il turno
    GS->SwitchTurn();

    UE_LOG(LogTemp, Warning, TEXT("Turn %d — Now playing: %s"),
        GS->TurnNumber,
        GS->CurrentTurn == ETurnOwner::Human ? TEXT("Human") : TEXT("AI"));

    // Se tocca all'AI, avvia il suo turno automaticamente
    if (GS->CurrentTurn == ETurnOwner::AI)
    {
        StartAITurn();
    }
}

void ATurnBasedGameMode::StartAITurn()
{
    // Placeholder — verrà implementata nei giorni dedicati all'AI (Giorno 16+)
    UE_LOG(LogTemp, Warning, TEXT("AI turn started — AI logic not yet implemented"));
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
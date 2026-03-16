#include "GameLogic/TurnBasedGameState.h"

ATurnBasedGameState::ATurnBasedGameState()
{
    CurrentPhase = EGamePhase::Placement;
    CurrentTurn = ETurnOwner::Human;
    TurnNumber = 1;
    HumanTowersControlled = 0;
    AITowersControlled = 0;
    HumanConsecutiveTowerTurns = 0;
    AIConsecutiveTowerTurns = 0;
}

bool ATurnBasedGameState::CheckWinCondition(ETurnOwner& OutWinner) const
{
    // Vince chi controlla 2 torri su 3 per 2 turni consecutivi
    if (HumanConsecutiveTowerTurns >= 2)
    {
        OutWinner = ETurnOwner::Human;
        return true;
    }
    if (AIConsecutiveTowerTurns >= 2)
    {
        OutWinner = ETurnOwner::AI;
        return true;
    }
    return false;
}

void ATurnBasedGameState::SwitchTurn()
{
    if (CurrentTurn == ETurnOwner::Human)
    {
        CurrentTurn = ETurnOwner::AI;
    }
    else
    {
        CurrentTurn = ETurnOwner::Human;
        TurnNumber++;
    }
}

void ATurnBasedGameState::UpdateTowerCounts(int32 HumanTowers, int32 AITowers, ETurnOwner TurnJustEnded)
{
    HumanTowersControlled = HumanTowers;
    AITowersControlled = AITowers;

    // Incrementa il counter dell'avversario di chi ha appena giocato
    if (TurnJustEnded == ETurnOwner::AI)
    {
        // E' appena finito il turno dell'AI: se l'Human tiene 2+ torri, il suo counter avanza
        if (HumanTowers >= 2) HumanConsecutiveTowerTurns++;
        else HumanConsecutiveTowerTurns = 0;
    }
    else
    {
        // E' appena finito il turno dell'Human: se l'AI tiene 2+ torri, il suo counter avanza
        if (AITowers >= 2) AIConsecutiveTowerTurns++;
        else AIConsecutiveTowerTurns = 0;
    }
}
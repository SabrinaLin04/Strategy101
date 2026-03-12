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
        bHumanActedThisCycle = true;
        CurrentTurn = ETurnOwner::AI;
    }
    else
    {
        bAIActedThisCycle = true;
        CurrentTurn = ETurnOwner::Human;
        TurnNumber++;
    }
}

void ATurnBasedGameState::UpdateTowerCounts(int32 HumanTowers, int32 AITowers, ETurnOwner TurnJustEnded)
{
    HumanTowersControlled = HumanTowers;
    AITowersControlled = AITowers;

    if (TurnJustEnded == ETurnOwner::Human) bHumanActedThisCycle = true;
    if (TurnJustEnded == ETurnOwner::AI) bAIActedThisCycle = true;

    //ciclo completo: entrambi hanno agito
    if (bHumanActedThisCycle && bAIActedThisCycle)
    {
        if (HumanTowers >= 2) HumanConsecutiveTowerTurns++;
        else HumanConsecutiveTowerTurns = 0;

        if (AITowers >= 2) AIConsecutiveTowerTurns++;
        else AIConsecutiveTowerTurns = 0;

        bHumanActedThisCycle = false;
        bAIActedThisCycle = false;
    }
}
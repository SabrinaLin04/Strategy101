#include "GameLogic/TowerControlSystem.h"
#include "Grid/Tower.h"
#include "Units/BaseUnit.h"
#include "GameLogic/TurnBasedGameState.h"
#include "GameLogic/TurnBasedGameMode.h"
#include "Grid/GridManager.h"

void UTowerControlSystem::EvaluateTowers(ATurnBasedGameMode* GM, ATurnBasedGameState* GS)
{
    UE_LOG(LogTemp, Warning, TEXT("EvaluateTowers: %d towers, %d human units, %d AI units"),
        GM->GetGridManager()->GetTowers().Num(),
        GS->HumanUnits.Num(),
        GS->AIUnits.Num());
    if (!GM || !GS) return;

    int32 HumanControlled = 0;
    int32 AIControlled = 0;

    for (ATower* Tower : GM->GetGridManager()->GetTowers())
    {
        if (!Tower) continue;

        bool bHumanInZone = false;
        bool bAIInZone = false;

        for (ABaseUnit* Unit : GS->HumanUnits)
            if (Unit && IsUnitInCaptureZone(Unit, Tower))
            {
                UE_LOG(LogTemp, Warning, TEXT("Human unit at (%d,%d) in zone of tower at (%d,%d)"),
                    Unit->GridX, Unit->GridY, Tower->GridX, Tower->GridY);
                bHumanInZone = true;
            }

        for (ABaseUnit* Unit : GS->AIUnits)
            if (Unit && IsUnitInCaptureZone(Unit, Tower))
            {
                UE_LOG(LogTemp, Warning, TEXT("AI unit at (%d,%d) in zone of tower at (%d,%d)"),
                    Unit->GridX, Unit->GridY, Tower->GridX, Tower->GridY);
                bAIInZone = true;
            }

        // Aggiorna i colori dinamici della torre
        Tower->HumanColor = GM->HumanUnitColor;
        Tower->AIColor = GM->AIUnitColor;
        Tower->EvaluateState(bHumanInZone, bAIInZone);

        if (Tower->OwnerPlayer == ETowerOwner::Human)      HumanControlled++;
        else if (Tower->OwnerPlayer == ETowerOwner::AI)    AIControlled++;
    }

    GS->UpdateTowerCounts(HumanControlled, AIControlled);

    UE_LOG(LogTemp, Warning, TEXT("Towers - Human: %d | AI: %d"), HumanControlled, AIControlled);
}

bool UTowerControlSystem::IsUnitInCaptureZone(ABaseUnit* Unit, ATower* Tower) const
{
    if (!Unit || !Tower) return false;
    int32 DX = FMath::Abs(Unit->GridX - Tower->GridX);
    int32 DY = FMath::Abs(Unit->GridY - Tower->GridY);
    return FMath::Max(DX, DY) <= Tower->CaptureRadius; // distanza Chebyshev include diagonali
}
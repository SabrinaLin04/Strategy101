#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TowerControlSystem.generated.h"

class ATower;
class ABaseUnit;
class ATurnBasedGameState;
class ATurnBasedGameMode;

UCLASS()
class STRATEGY101_API UTowerControlSystem : public UObject
{
    GENERATED_BODY()

public:
    //valuta lo stato di tutte le torri a fine turno e aggiorna i contatori
    void EvaluateTowers(ATurnBasedGameMode* GM, ATurnBasedGameState* GS);

    //controlla se un'unità è nella zona di cattura (distanza Chebyshev <= CaptureRadius)
    bool IsUnitInCaptureZone(ABaseUnit* Unit, ATower* Tower) const;
};
#pragma once

#include "CoreMinimal.h"
#include "BaseUnit.h"
#include "Sniper.generated.h"

UCLASS()
class STRATEGY101_API ASniper : public ABaseUnit
{
    GENERATED_BODY()

public:
    ASniper();

    /**
     * Override del calcolo danno: Sniper usa range 4-8.
     * Gestisce anche il danno da contrattacco se necessario.
     */
    virtual int32 RollDamage() const override;
};
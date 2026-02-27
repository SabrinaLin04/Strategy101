#pragma once

#include "CoreMinimal.h"
#include "BaseUnit.h"
#include "Brawler.generated.h"

UCLASS()
class STRATEGY101_API ABrawler : public ABaseUnit
{
    GENERATED_BODY()

public:
    ABrawler();

    /** Override danno: Brawler usa range 1-6 */
    virtual int32 RollDamage() const override;
};
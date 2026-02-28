#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Attackable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAttackable : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interfaccia per tutte le entità che possono ricevere ed effettuare attacchi.
 * Implementata da ABaseUnit e sue sottoclassi.
 */
class STRATEGY101_API IAttackable
{
    GENERATED_BODY()

public:
    /**
     * Effettua un attacco su un bersaglio.
     * @param Target - unità bersaglio dell'attacco
     * @return danno inflitto
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
    int32 PerformAttack(AActor* Target);

    /**
     * Riceve danno da un attacco.
     * @param DamageAmount - quantità di danno ricevuto
     * @return true se l'unità muore
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
    bool ReceiveDamage(int32 DamageAmount);

    /**
     * Controlla se il bersaglio è nel range di attacco.
     * @param Target - bersaglio da controllare
     * @return true se attaccabile
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
    bool IsTargetInRange(AActor* Target);

    /**
     * Calcola il danno da contrattacco (usato dallo Sniper).
     * @return danno da contrattacco (0 se non applicabile)
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
    int32 GetCounterAttackDamage();
};
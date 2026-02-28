#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Movable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UMovable : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interfaccia per tutte le entità che possono muoversi sulla griglia.
 * Implementata da ABaseUnit e sue sottoclassi.
 */
class STRATEGY101_API IMovable
{
    GENERATED_BODY()

public:
    /**
     * Muove l'unità verso una cella di destinazione.
     * @param DestX - coordinata X destinazione
     * @param DestY - coordinata Y destinazione
     * @return true se il movimento è andato a buon fine
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement")
    bool MoveTo(int32 DestX, int32 DestY);

    /**
     * Ritorna tutte le celle raggiungibili dall'unità nel turno corrente.
     * @return array di coordinate raggiungibili
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement")
    TArray<FVector2D> GetReachableCells();

    /**
     * Calcola il costo di movimento tra due celle adiacenti.
     * Piano/stesso livello = 1, salita = 2, discesa = 1.
     * @param FromElevation - livello elevazione cella di partenza
     * @param ToElevation - livello elevazione cella di arrivo
     * @return costo del movimento
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement")
    int32 GetMovementCost(int32 FromElevation, int32 ToElevation);

    /**
     * Controlla se una cella è percorribile (non acqua, non torre, non occupata).
     * @param GridX - coordinata X cella
     * @param GridY - coordinata Y cella
     * @return true se calpestabile
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement")
    bool IsCellWalkable(int32 GridX, int32 GridY);
};
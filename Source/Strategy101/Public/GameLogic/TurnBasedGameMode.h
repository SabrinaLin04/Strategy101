#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Units/BaseUnit.h"
#include "TurnBasedGameMode.generated.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"

UCLASS()
class STRATEGY101_API ATurnBasedGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATurnBasedGameMode();

    virtual void BeginPlay() override;

    // --- Coin Flip ---

    /** Esegue il coin flip, imposta CoinFlipWinner nel GameState */
    UFUNCTION(BlueprintCallable, Category = "GameMode|Setup")
    void PerformCoinFlip();

    // --- Fase di Posizionamento ---

    /**
     * Chiamata quando il player posiziona un'unità durante la fase di placement.
     * Alterna tra Human e AI fino all'esaurimento delle unità.
     */
    UFUNCTION(BlueprintCallable, Category = "GameMode|Placement")
    void OnUnitPlaced(ABaseUnit* PlacedUnit, int32 GridX, int32 GridY);

    /** Controlla se il posizionamento è completo (4 unità totali piazzate) */
    bool IsPlacementComplete() const;

    // --- Gestione Turni ---

    /** Chiamata al termine delle azioni di un giocatore, passa il turno */
    UFUNCTION(BlueprintCallable, Category = "GameMode|Turn")
    void EndTurn();

    /** Chiamata quando l'AI deve eseguire il suo turno */
    UFUNCTION(BlueprintCallable, Category = "GameMode|Turn")
    void StartAITurn();

    /** Controlla la condizione di vittoria e termina la partita se necessario */
    void CheckGameOver();

    // --- Utility ---

    /** Ritorna il GameState castato al tipo corretto */
    ATurnBasedGameState* GetTurnGameState() const;

protected:
    /** Numero di unità piazzate finora (max 4: 2 human + 2 AI) */
    int32 UnitsPlacedCount;

    /** A chi tocca piazzare nella fase placement */
    ETurnOwner PlacementTurn;
};
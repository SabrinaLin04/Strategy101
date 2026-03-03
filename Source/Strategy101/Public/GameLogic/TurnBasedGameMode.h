#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Units/BaseUnit.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "Grid/GridManager.h"
#include "TurnBasedGameMode.generated.h"

UCLASS()
class STRATEGY101_API ATurnBasedGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATurnBasedGameMode();
    virtual void BeginPlay() override;

    // Esegue il coin flip e imposta CoinFlipWinner nel GameState
    UFUNCTION(BlueprintCallable, Category = "GameMode|Setup")
    void PerformCoinFlip();

    // Chiamata quando il player umano clicca una cella per piazzare un'unità
    UFUNCTION(BlueprintCallable, Category = "GameMode|Placement")
    void OnHumanPlacementCellClicked(int32 X, int32 Y);

    // Fa piazzare all'AI la sua prossima unità automaticamente
    void PerformAIPlacement();

    // Avanza al prossimo step del piazzamento (alterna Human/AI)
    void AdvancePlacementStep();

    // Transizione alla fase di gioco vera e propria
    void StartGamePhase();

    // Controlla se il posizionamento è completo (4 unità totali piazzate)
    bool IsPlacementComplete() const;

    // Chiamata al termine delle azioni di un giocatore, passa il turno
    UFUNCTION(BlueprintCallable, Category = "GameMode|Turn")
    void EndTurn();

    // Chiamata quando l'AI deve eseguire il suo turno
    UFUNCTION(BlueprintCallable, Category = "GameMode|Turn")
    void StartAITurn();

    // Controlla la condizione di vittoria e termina la partita se necessario
    void CheckGameOver();

    // Ritorna il GameState castato al tipo corretto
    ATurnBasedGameState* GetTurnGameState() const;

    // Classe del widget di avvio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> MapConfigWidgetClass;

protected:
    // Numero unità piazzate per ogni giocatore
    int32 HumanUnitsPlaced;
    int32 AIUnitsPlaced;

    // A chi tocca piazzare nella fase placement
    ETurnOwner PlacementTurn;

    // Riferimento al GridManager nella scena
    AGridManager* GridManagerRef;
};
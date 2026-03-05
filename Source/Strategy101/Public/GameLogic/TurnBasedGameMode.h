#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Units/BaseUnit.h"
#include "Units/Sniper.h"
#include "Units/Brawler.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "Grid/GridManager.h"
#include "UI/CoinFlipWidget.h"
#include "UI/PlacementWidget.h"
#include "Grid/GridCell.h"
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

    void SpawnAIUnitAtCell(AGridCell* Cell);

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

    void ShowPlacementWidget();

    // Ritorna il GameState castato al tipo corretto
    ATurnBasedGameState* GetTurnGameState() const;

    // Classe del widget di avvio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> MapConfigWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UCoinFlipWidget> CoinFlipWidgetClass;

    // Classe del widget di piazzamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UPlacementWidget> PlacementWidgetClass;

    // Classe Blueprint dello Sniper da spawnare
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Units")
    TSubclassOf<ASniper> SniperClass;

    // Classe Blueprint del Brawler da spawnare
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Units")
    TSubclassOf<ABrawler> BrawlerClass;

protected:
    // Numero unità piazzate per ogni giocatore
    int32 HumanUnitsPlaced;
    int32 AIUnitsPlaced;

    // A chi tocca piazzare nella fase placement
    ETurnOwner PlacementTurn;

    // Riferimento al GridManager nella scena
    AGridManager* GridManagerRef;

    // Riferimento al widget di piazzamento attivo
    UPlacementWidget* PlacementWidgetRef;

    // Celle evidenziate nella zona di schieramento
    TArray<AGridCell*> HighlightedCells;

    // Evidenzia le celle valide per il piazzamento Human (Y=0,1,2)
    void HighlightHumanPlacementZone();

    // Rimuove l'evidenziazione
    void ClearHighlight();

    // Spawna l'unità corretta nella cella cliccata
    void SpawnUnitAtCell(AGridCell* Cell);
};
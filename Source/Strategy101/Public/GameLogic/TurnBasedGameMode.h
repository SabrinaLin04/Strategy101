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
#include "GameLogic/TowerControlSystem.h"
#include "Grid/Tower.h"
#include "GameLogic/PathfindingComponent.h"
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

    // Chiamata quando il player clicca durante la fase Playing
    void OnHumanGameCellClicked(int32 X, int32 Y);

    // Seleziona un'unità e evidenzia la sua cella
    void SelectUnit(ABaseUnit* Unit);

    // Deseleziona l'unità corrente
    void DeselectUnit();

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
    TSubclassOf<UUserWidget> ConfigWidgetClass;

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

    // Colori scelti dal player per le unità
    UPROPERTY(BlueprintReadWrite, Category = "Units")
    FLinearColor HumanUnitColor = FLinearColor(0.5f, 0.f, 1.f);

    UPROPERTY(BlueprintReadWrite, Category = "Units")
    FLinearColor AIUnitColor = FLinearColor(1.f, 1.f, 1.f);

    //getter per il GridManager (usato dal TowerControlSystem)
    AGridManager* GetGridManager() const { return GridManagerRef; }

    void OnTowerClicked(ATower* Tower);

protected:
    // Numero unità piazzate per ogni giocatore
    int32 HumanUnitsPlaced;
    int32 AIUnitsPlaced;

    // Unità attualmente selezionata dal player
    ABaseUnit* SelectedUnit;

    // Cella attualmente evidenziata per la selezione
    AGridCell* SelectedCell;

    // Numero di unità Human che hanno completato le azioni in questo turno
    TSet<ABaseUnit*> HumanUnitsActedSet;

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

    //mostra il range di movimento dell'unità selezionata colorando le celle raggiungibili
    void ShowMovementRange(ABaseUnit* Unit);

    //rimuove l'evidenziazione del range di movimento
    void ClearMovementRange();

    //sposta l'unità alla destinazione e aggiorna griglia e posizione world
    void MoveUnitToCell(ABaseUnit* Unit, int32 DestX, int32 DestY);

    //celle attualmente evidenziate per il movimento
    TArray<AGridCell*> MovementHighlightedCells;

    //set di posizioni raggiungibili per lookup rapido durante il click
    TSet<FIntPoint> ReachableCellSet;

    UPROPERTY()
    UTowerControlSystem* TowerControlSystem;

    UPROPERTY()
    UPathfindingComponent* Pathfinding;

    //muove l'unità step-by-step lungo il path con timer
    void StartStepMovement(ABaseUnit* Unit, TArray<FIntPoint> Path);

    //eseguito ad ogni step dell'animazione di movimento
    void DoMovementStep();

    //mostra i nemici attaccabili dall'unità selezionata (highlight rosso)
    void ShowAttackRange(ABaseUnit* Unit);

    //rimuove l'highlight dell'attacco
    void ClearAttackRange();

    //esegue l'attacco dell'unità selezionata sul target
    void ExecuteAttack(ABaseUnit* Attacker, ABaseUnit* Target);

    //nemici attualmente evidenziati come attaccabili
    TArray<ABaseUnit*> AttackableTargets;

    //processa il turno della prossima unità AI nella coda
    void ProcessNextAIUnit();

    //attacca un nemico in range se disponibile, poi processa la prossima unità
    void AIAttackIfPossible(ABaseUnit* Unit);

    //coda delle unità AI da processare nel turno corrente
    TArray<ABaseUnit*> AIUnitQueue;

    //true durante il movimento step-by-step dell'AI (differenzia da movimento Human)
    bool bIsAIMoving;

    void ShowTowerCaptureZone(ATower* Tower);
    void ClearInfoHighlight();
    TArray<AGridCell*> InfoHighlightedCells;
    ATower* SelectedTower;

    //stato interno animazione movimento
    ABaseUnit* MovingUnit;
    TArray<FIntPoint> MovementPath;
    int32 CurrentPathStep;
    FTimerHandle StepTimer;
};
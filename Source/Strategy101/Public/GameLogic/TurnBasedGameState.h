#pragma once                          // DEVE essere la prima riga assoluta

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Units/BaseUnit.h"
#include "TurnBasedGameState.generated.h"   // DEVE essere l'ultimo include

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Placement   UMETA(DisplayName = "Placement"),   // fase posizionamento unità
    Playing     UMETA(DisplayName = "Playing"),     // partita in corso
    GameOver    UMETA(DisplayName = "GameOver")     // partita terminata
};

UENUM(BlueprintType)
enum class ETurnOwner : uint8
{
    Human   UMETA(DisplayName = "Human"),
    AI      UMETA(DisplayName = "AI")
};

UCLASS()
class STRATEGY101_API ATurnBasedGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ATurnBasedGameState();

    /** Fase corrente della partita */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    EGamePhase CurrentPhase;

    /** A chi tocca il turno */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    ETurnOwner CurrentTurn;

    /** Numero turno corrente (incrementa ogni volta che entrambi hanno giocato) */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    int32 TurnNumber;

    /** Torri controllate dal player umano */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    int32 HumanTowersControlled;

    /** Torri controllate dall'AI */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    int32 AITowersControlled;

    /** Turni consecutivi in cui il player umano controlla 2+ torri */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    int32 HumanConsecutiveTowerTurns;

    /** Turni consecutivi in cui l'AI controlla 2+ torri */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    int32 AIConsecutiveTowerTurns;

    /** Unità vive del player umano */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    TArray<ABaseUnit*> HumanUnits;

    /** Unità vive dell'AI */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    TArray<ABaseUnit*> AIUnits;

    /** Chi ha vinto il coin flip (inizia il posizionamento) */
    UPROPERTY(BlueprintReadWrite, Category = "GameState")
    ETurnOwner CoinFlipWinner;

    /** Controlla se un giocatore ha vinto (2 torri per 2 turni consecutivi) */
    bool CheckWinCondition(ETurnOwner& OutWinner) const;

    /** Passa il turno all'altro giocatore */
    void SwitchTurn();

    /** Incrementa il contatore torri e aggiorna i turni consecutivi */
    void UpdateTowerCounts(int32 HumanTowers, int32 AITowers);
};
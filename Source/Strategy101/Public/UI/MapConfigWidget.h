#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Grid/GridManager.h"
#include "GameLogic/TurnBasedGameMode.h"
#include "MapConfigWidget.generated.h"

// Widget di avvio partita
UCLASS()
class STRATEGY101_API UMapConfigWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Riferimento al GridManager, impostato dal GameMode
    UPROPERTY(BlueprintReadWrite, Category = "Config")
    AGridManager* GridManager;

    // Chiamata dal Blueprint al click di Start Game
    UFUNCTION(BlueprintCallable, Category = "Config")
    void OnStartGame();
};
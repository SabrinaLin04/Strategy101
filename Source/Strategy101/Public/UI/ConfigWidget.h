#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameLogic/TurnBasedGameMode.h"
#include "Grid/GridManager.h"
#include "ConfigWidget.generated.h"

UCLASS()
class STRATEGY101_API UConfigWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Riferimento al GridManager passato dal GameMode
    UPROPERTY(BlueprintReadWrite, Category = "Config")
    AGridManager* GridManager;

    // Colori scelti per Human e AI (default: viola e bianco)
    UPROPERTY(BlueprintReadWrite, Category = "Config")
    FLinearColor HumanColor = FLinearColor(0.5f, 0.f, 1.f);

    UPROPERTY(BlueprintReadWrite, Category = "Config")
    FLinearColor AIColor = FLinearColor(1.f, 1.f, 1.f);

    // Chiamata dal bottone Start Game
    UFUNCTION(BlueprintCallable, Category = "Config")
    void OnStartGame();

    // Seleziona colore Human (chiamata dai bottoni colore)
    UFUNCTION(BlueprintCallable, Category = "Config")
    void SetHumanColor(FLinearColor Color);

    // Seleziona colore AI (chiamata dai bottoni colore)
    UFUNCTION(BlueprintCallable, Category = "Config")
    void SetAIColor(FLinearColor Color);
};
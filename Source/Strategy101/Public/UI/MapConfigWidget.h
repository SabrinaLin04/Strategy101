#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Grid/GridManager.h"
#include "MapConfigWidget.generated.h"

// Classe C++ base del widget di configurazione mappa
UCLASS()
class STRATEGY101_API UMapConfigWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Riferimento al GridManager nella scena
    UPROPERTY(BlueprintReadWrite, Category = "Config")
    AGridManager* GridManager;

    // Chiamata dal Blueprint quando il player preme "Start Game"
    UFUNCTION(BlueprintCallable, Category = "Config")
    void OnStartGame(int32 Seed, float WaterThresh, float NoiseScaleVal);

    // Rimuove il widget e avvia la generazione della mappa
    UFUNCTION(BlueprintCallable, Category = "Config")
    void ApplyAndClose();

protected:
    // Valori inseriti dal player nel widget
    UPROPERTY(BlueprintReadWrite, Category = "Config")
    int32 SelectedSeed;

    UPROPERTY(BlueprintReadWrite, Category = "Config")
    float SelectedWaterThreshold;

    UPROPERTY(BlueprintReadWrite, Category = "Config")
    float SelectedNoiseScale;
};
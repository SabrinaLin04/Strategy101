#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlacementWidget.generated.h"

// Widget per la selezione del tipo di unità da piazzare
UCLASS()
class STRATEGY101_API UPlacementWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Mostra il widget con indicazione di quale unità piazzare
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowPlacementPrompt(const FString& UnitName);

    // Nasconde il widget
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HidePlacementPrompt();
};
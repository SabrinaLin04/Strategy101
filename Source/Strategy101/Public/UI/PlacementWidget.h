#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlacementWidget.generated.h"

UCLASS()
class STRATEGY101_API UPlacementWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowUnitSelection(); //mostra i bottoni per scegliere l'unità

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowPlacementPrompt(const FString& UnitName); //mostra prompt dopo la scelta

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HidePlacementPrompt();

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnSniperClicked();

    UFUNCTION()
    void OnBrawlerClicked();
};
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinFlipWidget.generated.h"

UCLASS()
class STRATEGY101_API UCoinFlipWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowResult(const FString& WinnerName);

protected:
    virtual void NativeConstruct() override;
    FTimerHandle HideTimer;

    UFUNCTION()
    void HideWidget();
};
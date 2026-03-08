#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

UCLASS()
class STRATEGY101_API UGameHUDWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    void OnEndTurnClicked();

    UFUNCTION(BlueprintCallable)
    void OnConfirmPositionClicked();
};
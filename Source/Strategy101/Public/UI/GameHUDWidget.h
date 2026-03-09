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

    //aggiorna tutti i valori visualizzati nell'HUD
    UFUNCTION(BlueprintCallable)
    void UpdateHUD(const FString& TurnText,
        const FString& HumanUnitsText,
        const FString& AIUnitsText,
        int32 HumanTowers,
        int32 AITowers);

    //riferimenti ai TextBlock da collegare nel Blueprint
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* TurnLabel;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* HumanUnitsLabel;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* AIUnitsLabel;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* HumanTowersLabel;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* AITowersLabel;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UScrollBox* MoveScrollBox;

    UFUNCTION(BlueprintCallable)
    void AddMoveEntry(const FString& Entry);
};
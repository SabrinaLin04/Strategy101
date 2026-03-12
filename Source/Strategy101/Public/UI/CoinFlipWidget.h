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

    //texture delle tre facce della moneta — assegnate nel Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
    class UTexture2D* TextureHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
    class UTexture2D* TextureAI;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
    class UTexture2D* TextureEdge;

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnFlipClicked();

    UFUNCTION()
    void OnSpinTick();

    UFUNCTION()
    void HideWidget();

private:
    FString PendingWinner;           //risultato del flip, mostrato a fine animazione
    int32 SpinStep = 0;              //step corrente dell'animazione
    int32 TotalSpinSteps = 20;       //numero totale di step
    float CurrentInterval = 0.05f;  //intervallo iniziale tra step (veloce)
    bool bShowingEdge = false;       //alterna tra faccia e bordo

    FTimerHandle SpinTimer;
    FTimerHandle HideTimer;

    //aggiorna la texture della moneta
    void SetCoinTexture(class UTexture2D* Texture);
};
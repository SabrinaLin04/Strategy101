#include "UI/CoinFlipWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "GameLogic/TurnBasedGameMode.h"

void UCoinFlipWidget::NativeConstruct()
{
    Super::NativeConstruct();

    //nascondi il testo risultato all'inizio
    UTextBlock* ResultText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultText")));
    if (ResultText) ResultText->SetVisibility(ESlateVisibility::Hidden);

    //mostra subito la faccia HP
    SetCoinTexture(TextureHP);

    UButton* FlipButton = Cast<UButton>(GetWidgetFromName(TEXT("FlipButton")));
    if (FlipButton)
        FlipButton->OnClicked.AddDynamic(this, &UCoinFlipWidget::OnFlipClicked);
}

void UCoinFlipWidget::OnFlipClicked()
{
    UButton* FlipButton = Cast<UButton>(GetWidgetFromName(TEXT("FlipButton")));
    if (FlipButton) FlipButton->SetIsEnabled(false);

    //avvia animazione — il GameMode viene chiamato solo a fine spin
    SpinStep = 0;
    CurrentInterval = 0.05f;
    bShowingEdge = false;
    GetWorld()->GetTimerManager().SetTimer(SpinTimer, this,
        &UCoinFlipWidget::OnSpinTick, CurrentInterval, false);
}

void UCoinFlipWidget::OnSpinTick()
{
    SpinStep++;
    bShowingEdge = !bShowingEdge;

    if (bShowingEdge)
        SetCoinTexture(TextureEdge);
    else
    {
        bool bShowHP = (SpinStep % 4 < 2);
        SetCoinTexture(bShowHP ? TextureHP : TextureAI);
    }

    if (SpinStep >= TotalSpinSteps)
    {
        //animazione finita: chiama il GameMode che decide il vincitore e mostra il risultato
        ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        if (GM) GM->PerformCoinFlip();
        return;
    }

    //rallenta progressivamente nella seconda metà
    if (SpinStep > TotalSpinSteps / 2)
        CurrentInterval = FMath::Lerp(0.05f, 0.3f,
            (float)(SpinStep - TotalSpinSteps / 2) / (TotalSpinSteps / 2));

    GetWorld()->GetTimerManager().SetTimer(SpinTimer, this,
        &UCoinFlipWidget::OnSpinTick, CurrentInterval, false);
}

void UCoinFlipWidget::SetCoinTexture(UTexture2D* Texture)
{
    UImage* CoinImage = Cast<UImage>(GetWidgetFromName(TEXT("CoinImage")));
    if (CoinImage && Texture)
        CoinImage->SetBrushFromTexture(Texture);
}

void UCoinFlipWidget::ShowResult(const FString& WinnerName)
{
    SetCoinTexture(WinnerName == TEXT("HP") ? TextureHP : TextureAI);

    UTextBlock* ResultText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultText")));
    if (ResultText)
    {
        ResultText->SetVisibility(ESlateVisibility::Visible);
        ResultText->SetText(FText::FromString(
            FString::Printf(TEXT("%s goes first!"), *WinnerName)));
    }

    GetWorld()->GetTimerManager().SetTimer(HideTimer, this,
        &UCoinFlipWidget::HideWidget, 3.f, false);
}

void UCoinFlipWidget::HideWidget()
{
    RemoveFromParent();
}
#include "UI/CoinFlipWidget.h"
#include "Components/TextBlock.h"

void UCoinFlipWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCoinFlipWidget::ShowResult(const FString& WinnerName)
{
    if (ResultText)
        ResultText->SetText(FText::FromString(FString::Printf(TEXT("%s goes first!"), *WinnerName)));

    // Nasconde il widget dopo 3 secondi
    GetWorld()->GetTimerManager().SetTimer(HideTimer, this, &UCoinFlipWidget::HideWidget, 3.f, false);
}

void UCoinFlipWidget::HideWidget()
{
    RemoveFromParent();
}
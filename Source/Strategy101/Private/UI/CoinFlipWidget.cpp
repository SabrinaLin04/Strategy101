#include "UI/CoinFlipWidget.h"
#include "Components/TextBlock.h"

void UCoinFlipWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCoinFlipWidget::ShowResult(const FString& WinnerName)
{
    // Cerca il TextBlock per nome invece di BindWidget
    UTextBlock* ResultText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ResultText")));
    if (ResultText)
        ResultText->SetText(FText::FromString(FString::Printf(TEXT("%s goes first!"), *WinnerName)));

    GetWorld()->GetTimerManager().SetTimer(HideTimer, this, &UCoinFlipWidget::HideWidget, 3.f, false);
}

void UCoinFlipWidget::HideWidget()
{
    RemoveFromParent();
}
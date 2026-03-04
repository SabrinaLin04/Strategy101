#include "UI/PlacementWidget.h"
#include "Components/TextBlock.h"

void UPlacementWidget::ShowPlacementPrompt(const FString& UnitName)
{
    // Cerca il TextBlock per nome a runtime
    UTextBlock* PromptText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PromptText")));
    if (PromptText)
        PromptText->SetText(FText::FromString(FString::Printf(TEXT("Place your %s\n(click a green cell)"), *UnitName)));

    SetVisibility(ESlateVisibility::Visible);
}

void UPlacementWidget::HidePlacementPrompt()
{
    SetVisibility(ESlateVisibility::Hidden);
}
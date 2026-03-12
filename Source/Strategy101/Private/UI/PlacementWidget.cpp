#include "UI/PlacementWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Units/BaseUnit.h"
#include "GameLogic/TurnBasedGameMode.h"

void UPlacementWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UButton* SniperButton = Cast<UButton>(GetWidgetFromName(TEXT("SniperButton")));
    if (SniperButton)
        SniperButton->OnClicked.AddDynamic(this, &UPlacementWidget::OnSniperClicked);

    UButton* BrawlerButton = Cast<UButton>(GetWidgetFromName(TEXT("BrawlerButton")));
    if (BrawlerButton)
        BrawlerButton->OnClicked.AddDynamic(this, &UPlacementWidget::OnBrawlerClicked);
}

void UPlacementWidget::OnSniperClicked()
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    //disabilita bottoni per evitare doppio click
    UButton* SniperButton = Cast<UButton>(GetWidgetFromName(TEXT("SniperButton")));
    UButton* BrawlerButton = Cast<UButton>(GetWidgetFromName(TEXT("BrawlerButton")));
    if (SniperButton) SniperButton->SetIsEnabled(false);
    if (BrawlerButton) BrawlerButton->SetIsEnabled(false);

    GM->OnHumanUnitTypeSelected(EAttackType::Sniper);
}

void UPlacementWidget::OnBrawlerClicked()
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    UButton* SniperButton = Cast<UButton>(GetWidgetFromName(TEXT("SniperButton")));
    UButton* BrawlerButton = Cast<UButton>(GetWidgetFromName(TEXT("BrawlerButton")));
    if (SniperButton) SniperButton->SetIsEnabled(false);
    if (BrawlerButton) BrawlerButton->SetIsEnabled(false);

    GM->OnHumanUnitTypeSelected(EAttackType::Brawler);
}

void UPlacementWidget::ShowUnitSelection()
{
    //mostra bottoni, nascondi prompt
    UButton* SniperButton = Cast<UButton>(GetWidgetFromName(TEXT("SniperButton")));
    UButton* BrawlerButton = Cast<UButton>(GetWidgetFromName(TEXT("BrawlerButton")));
    UTextBlock* PromptText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PromptText")));

    if (SniperButton) { SniperButton->SetIsEnabled(true); SniperButton->SetVisibility(ESlateVisibility::Visible); }
    if (BrawlerButton) { BrawlerButton->SetIsEnabled(true); BrawlerButton->SetVisibility(ESlateVisibility::Visible); }
    if (PromptText) PromptText->SetVisibility(ESlateVisibility::Hidden);

    SetVisibility(ESlateVisibility::Visible);
}

void UPlacementWidget::ShowPlacementPrompt(const FString& UnitName)
{
    //nascondi bottoni, mostra prompt
    UButton* SniperButton = Cast<UButton>(GetWidgetFromName(TEXT("SniperButton")));
    UButton* BrawlerButton = Cast<UButton>(GetWidgetFromName(TEXT("BrawlerButton")));
    UTextBlock* PromptText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PromptText")));
    UTextBlock* ChooseText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ChooseText")));

    if (ChooseText) ChooseText->SetVisibility(ESlateVisibility::Hidden);
    if (SniperButton) SniperButton->SetVisibility(ESlateVisibility::Hidden);
    if (BrawlerButton) BrawlerButton->SetVisibility(ESlateVisibility::Hidden);
    if (PromptText)
    {
        PromptText->SetVisibility(ESlateVisibility::Visible);
        PromptText->SetText(FText::FromString(
            FString::Printf(TEXT("Place your %s\n(click a cyan cell)"), *UnitName)));
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UPlacementWidget::HidePlacementPrompt()
{
    SetVisibility(ESlateVisibility::Hidden);
}
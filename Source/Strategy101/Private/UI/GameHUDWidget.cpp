#include "UI/GameHUDWidget.h"
#include "GameLogic/TurnBasedGameMode.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

void UGameHUDWidget::OnEndTurnClicked()
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(
        UGameplayStatics::GetGameMode(GetWorld()));
    if (GM) GM->HumanEndTurn();
}

void UGameHUDWidget::OnConfirmPositionClicked()
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(
        UGameplayStatics::GetGameMode(GetWorld()));
    if (GM) GM->HumanConfirmPosition();
}

void UGameHUDWidget::UpdateHUD(const FString& TurnText,
    const FString& HumanUnitsText,
    const FString& AIUnitsText,
    int32 HumanTowers,
    int32 AITowers)
{
    if (TurnLabel)       TurnLabel->SetText(FText::FromString(TurnText));
    if (HumanUnitsLabel) HumanUnitsLabel->SetText(FText::FromString(HumanUnitsText));
    if (AIUnitsLabel)    AIUnitsLabel->SetText(FText::FromString(AIUnitsText));
    if (HumanTowersLabel) HumanTowersLabel->SetText(
        FText::FromString(FString::Printf(TEXT("HP Towers: %d"), HumanTowers)));
    if (AITowersLabel)   AITowersLabel->SetText(
        FText::FromString(FString::Printf(TEXT("AI Towers: %d"), AITowers)));
}

void UGameHUDWidget::AddMoveEntry(const FString& Entry)
{
    if (!MoveScrollBox) return;
    UTextBlock* Text = NewObject<UTextBlock>(this);
    Text->SetText(FText::FromString(Entry));
    Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    MoveScrollBox->AddChild(Text);
    MoveScrollBox->ScrollToEnd();
}
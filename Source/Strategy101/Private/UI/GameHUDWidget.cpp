#include "UI/GameHUDWidget.h"
#include "GameLogic/TurnBasedGameMode.h"
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
#include "UI/ConfigWidget.h"
#include "Grid/GridManager.h"
#include "GameLogic/TurnBasedGameMode.h"

void UConfigWidget::OnStartGame()
{
    if (!GridManager) return;
    GridManager->NoiseSeed = 0;
    GridManager->GenerateGrid();
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    PC->SetInputMode(FInputModeGameOnly());

    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        // Passa i colori scelti al GameMode
        GM->HumanUnitColor = HumanColor;
        GM->AIUnitColor = AIColor;
        GM->PerformCoinFlip();
    }
}

void UConfigWidget::SetHumanColor(FLinearColor Color)
{
    HumanColor = Color;
}

void UConfigWidget::SetAIColor(FLinearColor Color)
{
    AIColor = Color;
}
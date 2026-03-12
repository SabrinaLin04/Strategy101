#include "UI/ConfigWidget.h"
#include "Grid/GridManager.h"
#include "GameLogic/TurnBasedGameMode.h"

void UConfigWidget::OnStartGame()
{
    if (!GridManager) return;
    GridManager->NoiseSeed = 0;
    GridManager->MaxElevationLevel = MaxElevation;
    GridManager->GenerateGrid();
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeUIOnly());

    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->HumanUnitColor = HumanColor;
        GM->AIUnitColor = AIColor;

        //mostra widget con bottone — PerformCoinFlip viene chiamato al click
        if(GM->CoinFlipWidgetClass)
        {
            GM->CoinFlipWidgetRef = CreateWidget<UCoinFlipWidget>(PC, GM->CoinFlipWidgetClass);
            if (GM->CoinFlipWidgetRef)
                GM->CoinFlipWidgetRef->AddToViewport();
        }
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

void UConfigWidget::SetMaxElevation(int32 Value)
{
    MaxElevation = FMath::Clamp(Value, 1, 4);
}
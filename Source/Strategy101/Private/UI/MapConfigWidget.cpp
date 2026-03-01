#include "UI/MapConfigWidget.h"

void UMapConfigWidget::OnStartGame()
{
    if (!GridManager) return;

    // Genera la mappa con seed random e rimuove il widget
    GridManager->NoiseSeed = 0;
    GridManager->GenerateGrid();
    RemoveFromParent();

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    PC->SetInputMode(FInputModeGameOnly());

    ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode) GameMode->PerformCoinFlip();
}
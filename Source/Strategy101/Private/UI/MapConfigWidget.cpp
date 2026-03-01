#include "MapConfigWidget.h"
#include "Grid/GridManager.h"

void UMapConfigWidget::OnStartGame(int32 Seed, float WaterThresh, float NoiseScaleVal)
{
    SelectedSeed = Seed;
    SelectedWaterThreshold = WaterThresh;
    SelectedNoiseScale = NoiseScaleVal;
    ApplyAndClose();
}

void UMapConfigWidget::ApplyAndClose()
{
    if (!GridManager) return;

    // Applica i parametri scelti dal player al GridManager
    GridManager->NoiseSeed = SelectedSeed;
    GridManager->WaterThreshold = SelectedWaterThreshold;
    GridManager->NoiseScale = SelectedNoiseScale;

    // Genera la mappa con i nuovi parametri
    GridManager->GenerateGrid();

    // Rimuove il widget dallo schermo
    RemoveFromParent();
}
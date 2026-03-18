#include "HPBarOverlay.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Engine/GameViewportClient.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Units/BaseUnit.h"

void UHPBarOverlay::NativeConstruct()
{
    Super::NativeConstruct();
}

UProgressBar* UHPBarOverlay::GetOrCreateBar(ABaseUnit* Unit)
{
    if (BarMap.Contains(Unit)) return BarMap[Unit];

    UProgressBar* Bar = NewObject<UProgressBar>(this);
    if (!Bar) return nullptr;

    UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Bar);
    if (!PanelSlot) return nullptr;

    PanelSlot->SetSize(FVector2D(50.f, 6.f));
    PanelSlot->SetAnchors(FAnchors(0.f, 0.f));
    PanelSlot->SetAlignment(FVector2D(0.5f, 1.f)); // pivot centrato X, base in basso Y

    BarMap.Add(Unit, Bar);
    return Bar;
}

void UHPBarOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    ATurnBasedGameState* GS = Cast<ATurnBasedGameState>(GetWorld()->GetGameState());
    if (!GS) return;

    TArray<ABaseUnit*> AllUnits;
    AllUnits.Append(GS->HumanUnits);
    AllUnits.Append(GS->AIUnits);

    TArray<ABaseUnit*> ToRemove;
    for (auto& Pair : BarMap)
        if (!AllUnits.Contains(Pair.Key)) ToRemove.Add(Pair.Key);
    for (ABaseUnit* Gone : ToRemove)
    {
        if (BarMap[Gone]) BarMap[Gone]->RemoveFromParent();
        BarMap.Remove(Gone);
    }

    for (ABaseUnit* Unit : AllUnits)
    {
        if (!Unit) continue;

        UProgressBar* Bar = GetOrCreateBar(Unit);
        if (!Bar) continue;

        if (Unit->IsHidden())
        {
            Bar->SetVisibility(ESlateVisibility::Hidden);
            continue;
        }
        Bar->SetVisibility(ESlateVisibility::Visible);

        float Pct = (Unit->MaxHP > 0) ? FMath::Clamp((float)Unit->CurrentHP / (float)Unit->MaxHP, 0.f, 1.f) : 0.f;
        Bar->SetPercent(Pct);

        FLinearColor BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Green, Pct);
        Bar->SetFillColorAndOpacity(BarColor);

        // Proietta posizione world -> screen, poi sposta in pixel verso l'alto (Y negativo = su)
        FVector2D ScreenPos;
        if (PC->ProjectWorldLocationToScreen(Unit->GetActorLocation(), ScreenPos))
        {
            float Scale = UWidgetLayoutLibrary::GetViewportScale(this); // DPI scale corretto per UMG
            ScreenPos /= Scale;
            ScreenPos.Y -= 30.f;
            UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Bar->Slot);
            if (PanelSlot) PanelSlot->SetPosition(ScreenPos);
        }
    }
}
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPBarOverlay.generated.h"

class ABaseUnit;
class UCanvasPanel;
class UProgressBar;

UCLASS()
class STRATEGY101_API UHPBarOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    virtual void NativeConstruct() override;

private:
    // Canvas radice del widget — il nome deve essere ESATTAMENTE "RootCanvas" nel Blueprint
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* RootCanvas;

    // Mappa unit� -> progress bar corrispondente
    TMap<ABaseUnit*, UProgressBar*> BarMap;

    // Restituisce la bar esistente o ne crea una nuova per l'unit� data
    UProgressBar* GetOrCreateBar(ABaseUnit* Unit);
};

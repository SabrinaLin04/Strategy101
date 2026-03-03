#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinFlipWidget.generated.h"

// Widget che mostra il risultato del coin flip a schermo
UCLASS()
class STRATEGY101_API UCoinFlipWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Mostra il messaggio e lo nasconde dopo N secondi
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowResult(const FString& WinnerName);

protected:
    virtual void NativeConstruct() override;

    // Testo del risultato — bindato nel Blueprint
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* ResultText;

    // Timer per nascondere il widget automaticamente
    FTimerHandle HideTimer;

    // Nasconde e rimuove il widget
    UFUNCTION()
    void HideWidget();
};
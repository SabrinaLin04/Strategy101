#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower.generated.h"

//stato della torre
UENUM(BlueprintType)
enum class ETowerState : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Controlled  UMETA(DisplayName = "Controlled"),
    Contested   UMETA(DisplayName = "Contested")
};

UENUM(BlueprintType)
enum class ETowerOwner : uint8
{
    None    UMETA(DisplayName = "None"),
    Human   UMETA(DisplayName = "Human"),
    AI      UMETA(DisplayName = "AI")
};

//posizione e stato della torre
UCLASS()
class STRATEGY101_API ATower : public AActor
{
    GENERATED_BODY()

public:
    ATower();

    UPROPERTY(BlueprintReadWrite, Category = "Tower")
    int32 GridX;

    UPROPERTY(BlueprintReadWrite, Category = "Tower")
    int32 GridY;

    UPROPERTY(BlueprintReadWrite, Category = "Tower")
    ETowerState TowerState;

    UPROPERTY(BlueprintReadWrite, Category = "Tower")
    ETowerOwner OwnerPlayer;

    //raggio di cattura
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
    int32 CaptureRadius;

    //cambio colore della torre in base alla sua appartenenza
    UFUNCTION(BlueprintCallable, Category = "Tower")
    void UpdateVisualState();

    UFUNCTION(BlueprintCallable, Category = "Tower")
    void EvaluateState(bool bHumanInZone, bool bAIInZone);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* TowerMesh;
};
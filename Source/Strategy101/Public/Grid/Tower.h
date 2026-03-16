#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower.generated.h"

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
    int32 CaptureRadius;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* TowerMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* RightMesh;

    UPROPERTY()
    UMaterialInstanceDynamic* TowerDynMat;

    UPROPERTY()
    UMaterialInstanceDynamic* RightDynMat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor HumanColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor AIColor;

    UFUNCTION(BlueprintCallable, Category = "Tower")
    void UpdateVisualState();

    UFUNCTION(BlueprintCallable, Category = "Tower")
    void EvaluateState(bool bHumanInZone, bool bAIInZone);

    UFUNCTION()
    void OnTowerClicked(AActor* TouchedActor, FKey ButtonPressed);

protected:
    virtual void BeginPlay() override;
};
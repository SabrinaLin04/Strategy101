#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.generated.h"

UENUM(BlueprintType)
enum class ECellType : uint8
{
    Water       UMETA(DisplayName = "Water"),       // livello 0 - non calpestabile
    Plain       UMETA(DisplayName = "Plain"),       // livello 1
    Mountain2   UMETA(DisplayName = "Mountain2"),  // livello 2
    Mountain3   UMETA(DisplayName = "Mountain3"),  // livello 3
    Mountain4   UMETA(DisplayName = "Mountain4")   // livello 4
};

UCLASS()
class STRATEGY101_API AGridCell : public AActor
{
    GENERATED_BODY()

public:
    AGridCell();

    /** Coordinate nella griglia */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridY;

    /** Livello di elevazione (0-4) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 ElevationLevel;

    /** Tipo cella derivato dall'elevazione */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    ECellType CellType;

    /** True se la cella è occupata da un'unità o torre */
    UPROPERTY(BlueprintReadWrite, Category = "Grid")
    bool bIsOccupied;

    /** Imposta il colore della mesh in base al livello */
    void UpdateVisualColor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CellMesh;
};
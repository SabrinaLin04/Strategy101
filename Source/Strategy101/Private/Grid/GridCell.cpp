#include "GridCell.h"
#include "UObject/ConstructorHelpers.h"

AGridCell::AGridCell()
{
    PrimaryActorTick.bCanEverTick = false;

    // Crea la mesh della cella
    CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
    RootComponent = CellMesh;

    // Valori default
    GridX = 0;
    GridY = 0;
    ElevationLevel = 1;
    CellType = ECellType::Plain;
    bIsOccupied = false;
}

void AGridCell::BeginPlay()
{
    Super::BeginPlay();
    UpdateVisualColor();
}

void AGridCell::UpdateVisualColor()
{
    if (!CellMesh) return;
    if (CellMesh->GetNumMaterials() == 0) return; // nessun materiale assegnato

    // Crea un MaterialInstanceDynamic per cambiare colore a runtime
    UMaterialInstanceDynamic* DynMat = CellMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (!DynMat) return;

    FLinearColor Color;
    switch (ElevationLevel)
    {
    case 0: Color = FLinearColor(0.0f, 0.3f, 1.0f); break;  // Blu - acqua
    case 1: Color = FLinearColor(0.1f, 0.8f, 0.1f); break;  // Verde - piano
    case 2: Color = FLinearColor(1.0f, 1.0f, 0.0f); break;  // Giallo
    case 3: Color = FLinearColor(1.0f, 0.5f, 0.0f); break;  // Arancio
    case 4: Color = FLinearColor(1.0f, 0.0f, 0.0f); break;  // Rosso
    default: Color = FLinearColor::White;
    }

    DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
}
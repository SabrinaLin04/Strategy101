#include "GridCell.h"
#include "UObject/ConstructorHelpers.h"
#include "GameLogic/TurnBasedGameMode.h"

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

    // Abilita i click sulla mesh
    if (CellMesh)
    {
        CellMesh->SetGenerateOverlapEvents(false);
        EnableInput(GetWorld()->GetFirstPlayerController());
    }
    OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);
}

void AGridCell::OnCellClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    ATurnBasedGameState* GS = Cast<ATurnBasedGameState>(GetWorld()->GetGameState());
    if (!GS) return;

    // Routing in base alla fase di gioco
    if (GS->CurrentPhase == EGamePhase::Placement)
        GM->OnHumanPlacementCellClicked(GridX, GridY);
    else if (GS->CurrentPhase == EGamePhase::Playing)
        GM->OnHumanGameCellClicked(GridX, GridY);
}

void AGridCell::UpdateVisualColor()
{
    if (!CellMesh) return;
    if (CellMesh->GetNumMaterials() == 0) return; // nessun materiale assegnato

    // Crea un MaterialInstanceDynamic per cambiare colore a runtime
    if (!CellDynMat)
        CellDynMat = CellMesh->CreateAndSetMaterialInstanceDynamic(0);
    UMaterialInstanceDynamic* DynMat = CellDynMat;
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
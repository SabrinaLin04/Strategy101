#include "Tower.h"

ATower::ATower()
{
    PrimaryActorTick.bCanEverTick = false;

    TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
    RootComponent = TowerMesh;

    GridX = 0;
    GridY = 0;
    TowerState = ETowerState::Neutral;
    OwnerPlayer = ETowerOwner::None;
    CaptureRadius = 2;
}

void ATower::BeginPlay()
{
    Super::BeginPlay();
    UpdateVisualState();
}

void ATower::UpdateVisualState()
{
    if (!TowerMesh) return;
    if (TowerMesh->GetNumMaterials() == 0) return;

    UMaterialInstanceDynamic* DynMat = TowerMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (!DynMat) return;

    FLinearColor Color;
    switch (TowerState)
    {
    case ETowerState::Neutral:
        Color = FLinearColor(0.7f, 0.7f, 0.7f);
        break;
    case ETowerState::Controlled:
        Color = (OwnerPlayer == ETowerOwner::Human)
            ? FLinearColor(0.0f, 0.0f, 1.0f)
            : FLinearColor(1.0f, 0.0f, 0.0f);
        break;
    case ETowerState::Contested:
        Color = FLinearColor(0.5f, 0.0f, 0.5f);
        break;
    }

    DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

void ATower::EvaluateState(bool bHumanInZone, bool bAIInZone)
{
    if (bHumanInZone && bAIInZone)
    {
        TowerState = ETowerState::Contested;
        OwnerPlayer = ETowerOwner::None;
    }
    else if (bHumanInZone)
    {
        TowerState = ETowerState::Controlled;
        OwnerPlayer = ETowerOwner::Human;
    }
    else if (bAIInZone)
    {
        TowerState = ETowerState::Controlled;
        OwnerPlayer = ETowerOwner::AI;
    }
    else
    {
        TowerState = ETowerState::Neutral;
        OwnerPlayer = ETowerOwner::None;
    }

    UpdateVisualState();
}
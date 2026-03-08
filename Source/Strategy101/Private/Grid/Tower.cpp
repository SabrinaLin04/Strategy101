#include "Tower.h"
#include "GameLogic/TurnBasedGameMode.h"

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
    EnableInput(GetWorld()->GetFirstPlayerController());
    OnClicked.AddDynamic(this, &ATower::OnTowerClicked);
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
        Color = (OwnerPlayer == ETowerOwner::Human) ? HumanColor : AIColor;
        break;
    case ETowerState::Contested:
        Color = FLinearColor(0.5f, 0.0f, 0.5f);
        break;
    default:
        Color = FLinearColor(0.7f, 0.7f, 0.7f);
        break;
    }

    DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

void ATower::OnTowerClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (GM) GM->OnTowerClicked(this);
}

void ATower::EvaluateState(bool bHumanInZone, bool bAIInZone)
{
    if (bHumanInZone && bAIInZone)
    {
        //contesa: rimuove il punto al proprietario precedente
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
        //nessuna unità in zona: mantieni il proprietario precedente se la torre era Controlled
        //torna Neutral solo se era Contested o non aveva mai avuto proprietario
        if (TowerState == ETowerState::Contested)
        {
            TowerState = ETowerState::Neutral;
            OwnerPlayer = ETowerOwner::None;
        }
        //se era Controlled rimane Controlled con lo stesso OwnerPlayer
    }

    UpdateVisualState();
}
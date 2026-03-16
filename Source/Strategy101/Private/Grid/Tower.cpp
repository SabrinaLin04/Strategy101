#include "Tower.h"
#include "GameLogic/TurnBasedGameMode.h"

ATower::ATower()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
    TowerMesh->SetupAttachment(Root);

    RightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMesh"));
    RightMesh->SetupAttachment(Root);
    RightMesh->SetVisibility(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> TowerMat(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MITower.MITower'"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> TowerMatHalf(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MI_TowerHalf.MI_TowerHalf'"));

    if (PlaneMesh.Succeeded())
    {
        TowerMesh->SetStaticMesh(PlaneMesh.Object);
        RightMesh->SetStaticMesh(PlaneMesh.Object);
    }
    if (TowerMat.Succeeded())
        TowerMesh->SetMaterial(0, TowerMat.Object);

    if (TowerMatHalf.Succeeded())
        RightMesh->SetMaterial(0, TowerMatHalf.Object);

    GridX = 0;
    GridY = 0;
    TowerState = ETowerState::Neutral;
    OwnerPlayer = ETowerOwner::None;
    CaptureRadius = 2;
}

void ATower::BeginPlay()
{
    Super::BeginPlay();

    RightMesh->SetVisibility(true);
    TowerDynMat = TowerMesh->CreateAndSetMaterialInstanceDynamic(0);
    RightDynMat = RightMesh->CreateAndSetMaterialInstanceDynamic(0);
    RightMesh->SetVisibility(false);

    UpdateVisualState();

    EnableInput(GetWorld()->GetFirstPlayerController());
    OnClicked.AddDynamic(this, &ATower::OnTowerClicked);
}

void ATower::UpdateVisualState()
{
    if (!TowerMesh || !RightMesh) return;

    if (!TowerDynMat) TowerDynMat = TowerMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (!RightDynMat) RightDynMat = RightMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (!TowerDynMat || !RightDynMat) return;

    FLinearColor TowerColor;
    switch (TowerState)
    {
    case ETowerState::Neutral:
        TowerColor = FLinearColor(0.7f, 0.7f, 0.7f);
        RightMesh->SetVisibility(false);
        break;
    case ETowerState::Controlled:
        TowerColor = (OwnerPlayer == ETowerOwner::Human) ? HumanColor : AIColor;
        RightMesh->SetVisibility(false);
        break;
    case ETowerState::Contested:
        TowerColor = (OwnerPlayer == ETowerOwner::Human) ? HumanColor : AIColor;
        RightMesh->SetVisibility(true);
        RightDynMat->SetScalarParameterValue(TEXT("UScale"), 0.5f);
        RightDynMat->SetScalarParameterValue(TEXT("UOffset"), 0.5f);
        RightDynMat->SetVectorParameterValue(TEXT("BaseColor"),
            (OwnerPlayer == ETowerOwner::Human) ? AIColor : HumanColor);
        break;
    default:
        TowerColor = FLinearColor(0.7f, 0.7f, 0.7f);
        RightMesh->SetVisibility(false);
        break;
    }

    TowerDynMat->SetVectorParameterValue(TEXT("BaseColor"), TowerColor);
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
        if (TowerState == ETowerState::Contested)
        {
            TowerState = ETowerState::Neutral;
            OwnerPlayer = ETowerOwner::None;
        }
    }

    UpdateVisualState();
}
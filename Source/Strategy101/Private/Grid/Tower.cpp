#include "Tower.h"
#include "GameLogic/TurnBasedGameMode.h"

ATower::ATower()
{
    PrimaryActorTick.bCanEverTick = false;

    TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
    RootComponent = TowerMesh; // deve essere PRIMA degli attach

    LeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMesh"));
    LeftMesh->SetupAttachment(RootComponent);
    LeftMesh->SetRelativeLocation(FVector(-25.f, 0.f, 1.f));
    LeftMesh->SetRelativeScale3D(FVector(0.5f, 1.f, 1.f));

    RightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMesh"));
    RightMesh->SetupAttachment(RootComponent);
    RightMesh->SetRelativeLocation(FVector(25.f, 0.f, 1.f));
    RightMesh->SetRelativeScale3D(FVector(0.5f, 1.f, 1.f));

    // carica il Plane di default di UE direttamente dal codice
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> TowerMat(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Materials/MITower.MITower'"));

    if (PlaneMesh.Succeeded())
    {
        LeftMesh->SetStaticMesh(PlaneMesh.Object);
        RightMesh->SetStaticMesh(PlaneMesh.Object);
    }
    if (TowerMat.Succeeded())
    {
        LeftMesh->SetMaterial(0, TowerMat.Object);
        RightMesh->SetMaterial(0, TowerMat.Object);
    }

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
    if (!TowerMesh || TowerMesh->GetNumMaterials() == 0) return;

    if (!TowerDynMat)
        TowerDynMat = TowerMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (!TowerDynMat) return;

    if (TowerTexture)
        TowerDynMat->SetTextureParameterValue(TEXT("TowerTexture"), TowerTexture);

    if (TowerState == ETowerState::Contested)
    {
        // nasconde la torre intera e mostra le due metà
        TowerMesh->SetVisibility(false);
        LeftMesh->SetVisibility(true);
        RightMesh->SetVisibility(true);

        if (!LeftDynMat)  LeftDynMat = LeftMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (!RightDynMat) RightDynMat = RightMesh->CreateAndSetMaterialInstanceDynamic(0);

        if (TowerTexture)
        {
            LeftDynMat->SetTextureParameterValue(TEXT("TowerTexture"), TowerTexture);
            RightDynMat->SetTextureParameterValue(TEXT("TowerTexture"), TowerTexture);
        }

        LeftDynMat->SetVectorParameterValue(TEXT("BaseColor"), HumanColor);
        RightDynMat->SetVectorParameterValue(TEXT("BaseColor"), AIColor);
    }
    else
    {
        // mostra la torre intera e nasconde le due metà
        TowerMesh->SetVisibility(true);
        LeftMesh->SetVisibility(false);
        RightMesh->SetVisibility(false);

        FLinearColor Color;
        switch (TowerState)
        {
        case ETowerState::Neutral:    Color = FLinearColor(0.7f, 0.7f, 0.7f); break;
        case ETowerState::Controlled: Color = (OwnerPlayer == ETowerOwner::Human) ? HumanColor : AIColor; break;
        default:                      Color = FLinearColor(0.7f, 0.7f, 0.7f); break;
        }
        TowerDynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
    }
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
#include "BaseUnit.h"
#include "GameLogic/TurnBasedGameMode.h"
#include "GameLogic/TurnBasedGameState.h"
#include "Math/UnrealMathUtility.h"

ABaseUnit::ABaseUnit()
{
    PrimaryActorTick.bCanEverTick = false;

    UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = UnitMesh;

    // Default stats (overrideati nelle sottoclassi)
    MaxMovement = 1;
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 1;
    MaxHP = 10;
    CurrentHP = MaxHP;
    bHasMoved = false;
    bHasAttacked = false;
    GridX = 0;
    GridY = 0;
}

void ABaseUnit::SetOwnerColor()
{
    if (!UnitMesh) return;
    UMaterialInstanceDynamic* DynMat = UnitMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynMat)
        DynMat->SetVectorParameterValue(TEXT("BaseColor"), OwnerColor);
    UE_LOG(LogTemp, Warning, TEXT("SetOwnerColor called R:%f G:%f B:%f"),
        OwnerColor.R, OwnerColor.G, OwnerColor.B);
}

void ABaseUnit::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
    EnableInput(GetWorld()->GetFirstPlayerController());
    OnClicked.AddDynamic(this, &ABaseUnit::OnUnitClicked);
}

void ABaseUnit::OnUnitClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return;

    ATurnBasedGameState* GS = Cast<ATurnBasedGameState>(GetWorld()->GetGameState());
    if (!GS) return;

    if (GS->CurrentPhase == EGamePhase::Playing)
        GM->OnHumanGameCellClicked(GridX, GridY);
}

int32 ABaseUnit::RollDamage() const
{
    // Estrae danno random nel range [MinDamage, MaxDamage]
    return FMath::RandRange(MinDamage, MaxDamage);
}

bool ABaseUnit::TakeDamage_Unit(int32 DamageAmount)
{
    CurrentHP -= DamageAmount;
    if (CurrentHP <= 0)
    {
        CurrentHP = 0;
        return true; // unità morta
    }
    return false;
}

// --- IAttackable ---

int32 ABaseUnit::PerformAttack_Implementation(AActor* Target)
{
    // Placeholder — logica completa al Giorno 18
    return RollDamage();
}

bool ABaseUnit::ReceiveDamage_Implementation(int32 DamageAmount)
{
    return TakeDamage_Unit(DamageAmount);
}

bool ABaseUnit::IsTargetInRange_Implementation(AActor* Target)
{
    // Placeholder — logica completa al Giorno 18
    return false;
}

int32 ABaseUnit::GetCounterAttackDamage_Implementation()
{
    // Placeholder — logica contrattacco al Giorno 24
    return 0;
}

// --- IMovable ---

bool ABaseUnit::MoveTo_Implementation(int32 DestX, int32 DestY)
{
    // Placeholder — logica completa al Giorno 16
    GridX = DestX;
    GridY = DestY;
    bHasMoved = true;
    return true;
}

TArray<FVector2D> ABaseUnit::GetReachableCells_Implementation()
{
    // Placeholder — logica completa al Giorno 16
    return TArray<FVector2D>();
}

int32 ABaseUnit::GetMovementCost_Implementation(int32 FromElevation, int32 ToElevation)
{
    // Piano o discesa = 1, salita = 2
    if (ToElevation > FromElevation) return 2;
    return 1;
}

bool ABaseUnit::IsCellWalkable_Implementation(int32 InGridX, int32 InGridY)
{
    // Placeholder — logica completa al Giorno 16
    return true;
}

void ABaseUnit::Respawn()
{
    CurrentHP = MaxHP;
    GridX = SpawnGridX;
    GridY = SpawnGridY;
    bHasMoved = false;
    bHasAttacked = false;
    SetActorHiddenInGame(false);
}

void ABaseUnit::ResetTurnState()
{
    bHasMoved = false;
    bHasAttacked = false;
}

bool ABaseUnit::IsAlive() const
{
    return CurrentHP > 0;
}
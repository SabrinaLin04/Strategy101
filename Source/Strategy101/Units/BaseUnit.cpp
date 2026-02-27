#include "BaseUnit.h"
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

void ABaseUnit::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
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
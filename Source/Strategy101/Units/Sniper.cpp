#include "Sniper.h"

ASniper::ASniper()
{
    // Statistiche Sniper come da specifiche
    MaxMovement = 4;
    AttackType = EAttackType::Ranged;
    AttackRange = 10;
    MinDamage = 4;
    MaxDamage = 8;
    MaxHP = 20;
    CurrentHP = MaxHP;
}

int32 ASniper::RollDamage() const
{
    return FMath::RandRange(MinDamage, MaxDamage);
}
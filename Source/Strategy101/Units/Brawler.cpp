#include "Brawler.h"

ABrawler::ABrawler()
{
    // Statistiche Brawler come da specifiche
    MaxMovement = 6;
    AttackType = EAttackType::Melee;
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 6;
    MaxHP = 40;
    CurrentHP = MaxHP;
}

int32 ABrawler::RollDamage() const
{
    return FMath::RandRange(MinDamage, MaxDamage);
}
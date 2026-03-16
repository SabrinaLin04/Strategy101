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

bool ABaseUnit::ReceiveDamage_Implementation(int32 DamageAmount)
{
    return TakeDamage_Unit(DamageAmount);
}

bool ABaseUnit::IsTargetInRange_Implementation(AActor* Target)
{
    ABaseUnit* TargetUnit = Cast<ABaseUnit>(Target);
    if (!TargetUnit) return false;

    ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
    if (!GM) return false;
    AGridManager* Grid = GM->GetGridManager();
    if (!Grid) return false;

    AGridCell* MyCell = Grid->GetCell(GridX, GridY);
    AGridCell* TargetCell = Grid->GetCell(TargetUnit->GridX, TargetUnit->GridY);
    if (!MyCell || !TargetCell) return false;

    //vincolo elevazione: posso attaccare solo unità al mio stesso livello o inferiore
    if (TargetCell->ElevationLevel > MyCell->ElevationLevel) return false;

    //per il range usiamo Manhattan senza costo doppio in salita
    if (AttackType == EAttackType::Sniper)
    {
        int32 Dist = FMath::Abs(GridX - TargetUnit->GridX) + FMath::Abs(GridY - TargetUnit->GridY);
        if (Dist > AttackRange) return false;

        int32 DX = TargetUnit->GridX - GridX;
        int32 DY = TargetUnit->GridY - GridY;
        int32 Steps = FMath::Max(FMath::Abs(DX), FMath::Abs(DY));

        for (int32 i = 1; i < Steps; i++)
        {
            int32 CX = GridX + FMath::RoundToInt((float)DX * i / Steps);
            int32 CY = GridY + FMath::RoundToInt((float)DY * i / Steps);
            AGridCell* Mid = Grid->GetCell(CX, CY);
            if (Mid && Mid->ElevationLevel > MyCell->ElevationLevel)
                return false;
        }

        return true;
    }
    else
    {
        //Brawler: range 1, solo celle adiacenti (no diagonale)
        int32 DX = FMath::Abs(GridX - TargetUnit->GridX);
        int32 DY = FMath::Abs(GridY - TargetUnit->GridY);
        return (DX + DY) == 1;
    }
}

int32 ABaseUnit::PerformAttack_Implementation(AActor* Target)
{
    ABaseUnit* TargetUnit = Cast<ABaseUnit>(Target);
    if (!TargetUnit) return 0;

    int32 Damage = RollDamage();
    bool bDied = TargetUnit->TakeDamage_Unit(Damage);

    UE_LOG(LogTemp, Warning, TEXT("%s attacked %s for %d damage (HP left: %d)"),
        *GetName(), *TargetUnit->GetName(), Damage, TargetUnit->CurrentHP);

    if (bDied)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s was eliminated!"), *TargetUnit->GetName());
        TargetUnit->SetActorHiddenInGame(true);

        ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            AGridCell* Cell = GM->GetGridManager()->GetCell(TargetUnit->GridX, TargetUnit->GridY);
            if (Cell) Cell->bIsOccupied = false;
        }

        ABaseUnit* UnitToRespawn = TargetUnit;
        ATurnBasedGameMode* GMRef = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        if (GMRef)
        {
            FTimerHandle RespawnTimer;
            GMRef->GetWorldTimerManager().SetTimer(RespawnTimer, [UnitToRespawn, GMRef]()
                {
                    if (!UnitToRespawn) return;
                    UnitToRespawn->Respawn();

                    AGridCell* SpawnCell = GMRef->GetGridManager()->GetCell(
                        UnitToRespawn->SpawnGridX, UnitToRespawn->SpawnGridY);
                    if (SpawnCell)
                    {
                        FVector WorldPos = SpawnCell->GetActorLocation();
                        WorldPos.Z += 20.f;
                        UnitToRespawn->SetActorLocation(WorldPos);
                    }

                    UE_LOG(LogTemp, Warning, TEXT("%s respawned at (%d,%d)"),
                        *UnitToRespawn->GetName(),
                        UnitToRespawn->SpawnGridX, UnitToRespawn->SpawnGridY);
                }, 2.f, false);
        }
    }

    //contrattacco: solo se l'attaccante è uno Sniper
    if (AttackType == EAttackType::Sniper)
    {
        int32 Dist = FMath::Abs(GridX - TargetUnit->GridX) + FMath::Abs(GridY - TargetUnit->GridY);
        ATurnBasedGameMode* GMCounter = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        AGridManager* Grid = GMCounter ? GMCounter->GetGridManager() : nullptr;
        AGridCell* MyCell = Grid ? Grid->GetCell(GridX, GridY) : nullptr;
        AGridCell* TargetCell = Grid ? Grid->GetCell(TargetUnit->GridX, TargetUnit->GridY) : nullptr;

        bool bSameElevation = MyCell && TargetCell && (MyCell->ElevationLevel == TargetCell->ElevationLevel);
        bool bTargetIsSniper = (TargetUnit->AttackType == EAttackType::Sniper) && bSameElevation;
        bool bTargetIsBrawlerAdjacent = (TargetUnit->AttackType == EAttackType::Brawler && Dist == 1);

        if (bTargetIsSniper || bTargetIsBrawlerAdjacent)
        {
            int32 CounterDamage = FMath::RandRange(1, 3);
            bool bAttackerDied = TakeDamage_Unit(CounterDamage);

            UE_LOG(LogTemp, Warning, TEXT("%s received counter-attack damage: %d (HP left: %d)"),
                *GetName(), CounterDamage, CurrentHP);

            if (bAttackerDied)
            {
                UE_LOG(LogTemp, Warning, TEXT("%s was eliminated by counter-attack!"), *GetName());
                SetActorHiddenInGame(true);

                ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
                if (GM)
                {
                    AGridCell* Cell = GM->GetGridManager()->GetCell(GridX, GridY);
                    if (Cell) Cell->bIsOccupied = false;
                }
            }
        }
    }

    bHasAttacked = true;
    return Damage;
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
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
    if (AttackType == EAttackType::Ranged)
    {
        //Sniper: distanza Manhattan pura, oltrepassa acqua
        int32 Dist = FMath::Abs(GridX - TargetUnit->GridX) + FMath::Abs(GridY - TargetUnit->GridY);
        return Dist <= AttackRange;
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

        //libera la cella occupata
        ATurnBasedGameMode* GM = Cast<ATurnBasedGameMode>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            AGridCell* Cell = GM->GetGridManager()->GetCell(TargetUnit->GridX, TargetUnit->GridY);
            if (Cell) Cell->bIsOccupied = false;
        }
    }

    bHasAttacked = true;
    return Damage;
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
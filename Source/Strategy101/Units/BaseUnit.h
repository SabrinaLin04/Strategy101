#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseUnit.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Melee   UMETA(DisplayName = "Melee"),   // Brawler - corpo a corpo
    Ranged  UMETA(DisplayName = "Ranged")   // Sniper - distanza
};

UENUM(BlueprintType)
enum class EOwner : uint8
{
    Human   UMETA(DisplayName = "Human"),
    AI      UMETA(DisplayName = "AI")
};

UCLASS(Abstract)  // Abstract: non può essere istanziata direttamente
class STRATEGY101_API ABaseUnit : public AActor
{
    GENERATED_BODY()

public:
    ABaseUnit();

    // --- Statistiche unità ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    int32 MaxMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    int32 AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    int32 MinDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    int32 MaxDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    int32 MaxHP;

    UPROPERTY(BlueprintReadWrite, Category = "Unit|Stats")
    int32 CurrentHP;

    // --- Stato turno ---

    UPROPERTY(BlueprintReadWrite, Category = "Unit|State")
    bool bHasMoved;

    UPROPERTY(BlueprintReadWrite, Category = "Unit|State")
    bool bHasAttacked;

    // --- Posizione sulla griglia ---

    UPROPERTY(BlueprintReadWrite, Category = "Unit|Grid")
    int32 GridX;

    UPROPERTY(BlueprintReadWrite, Category = "Unit|Grid")
    int32 GridY;

    /** Posizione di spawn originale (per il respawn) */
    UPROPERTY(BlueprintReadWrite, Category = "Unit|Grid")
    int32 SpawnGridX;

    UPROPERTY(BlueprintReadWrite, Category = "Unit|Grid")
    int32 SpawnGridY;

    /** Proprietario dell'unità */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|State")
    EOwner Owner;

    // --- Metodi virtuali ---

    /** Calcola danno random nel range dell'unità */
    virtual int32 RollDamage() const;

    /** Applica danno all'unità, ritorna true se muore */
    virtual bool TakeDamage_Unit(int32 DamageAmount);

    /** Resetta HP e stato per il respawn */
    virtual void Respawn();

    /** Resetta i flag di azione a inizio turno */
    void ResetTurnState();

    /** Ritorna true se l'unità è ancora viva */
    bool IsAlive() const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* UnitMesh;
};
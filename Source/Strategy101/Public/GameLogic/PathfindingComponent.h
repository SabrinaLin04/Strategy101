#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PathfindingComponent.generated.h"

class AGridCell;
class AGridManager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STRATEGY101_API UPathfindingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPathfindingComponent();

    //A* da Start a Goal, ritorna percorso ordinato di celle (Start escluso, Goal incluso)
    TArray<FIntPoint> FindPath(AGridManager* Grid, FIntPoint Start, FIntPoint Goal, int32 MaxMoveCost);

    //Dijkstra: ritorna tutte le celle raggiungibili entro MaxMoveCost (usato per highlight)
    TMap<FIntPoint, int32> GetReachableCells(AGridManager* Grid, FIntPoint Start, int32 MaxMoveCost);

   // Ritorna il costo reale del percorso più breve tra due punti (-1 se irraggiungibile)
    int32 GetActualDistance(AGridManager* Grid, FIntPoint Start, FIntPoint Goal) const;

private:
    //costo di movimento tra due celle adiacenti (salita=2, piano/discesa=1)
    int32 MoveCost(AGridCell* From, AGridCell* To) const;

    //heuristica Manhattan distance
    int32 Heuristic(FIntPoint A, FIntPoint B) const;

    //controlla se la cella è calpestabile durante il pathfinding
    bool IsWalkable(AGridCell* Cell) const;
};
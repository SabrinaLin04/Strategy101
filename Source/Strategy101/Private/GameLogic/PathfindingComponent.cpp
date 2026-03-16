#include "GameLogic/PathfindingComponent.h"
#include "Grid/GridManager.h"
#include "Grid/GridCell.h"

UPathfindingComponent::UPathfindingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

int32 UPathfindingComponent::Heuristic(FIntPoint A, FIntPoint B) const
{
    return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

int32 UPathfindingComponent::MoveCost(AGridCell* From, AGridCell* To) const
{
    if (!From || !To) return 999;
    return (To->ElevationLevel > From->ElevationLevel) ? 2 : 1;
}

bool UPathfindingComponent::IsWalkable(AGridCell* Cell) const
{
    if (!Cell) return false;
    if (Cell->ElevationLevel == 0) return false; //acqua
    if (Cell->bIsOccupied) return false;          //torre o unità
    return true;
}

TArray<FIntPoint> UPathfindingComponent::FindPath(AGridManager* Grid, FIntPoint Start, FIntPoint Goal, int32 MaxMoveCost)
{
    if (!Grid) return {};

    //GScore: costo reale da Start; FScore: GScore + euristica
    TMap<FIntPoint, int32> GScore, FScore;
    TMap<FIntPoint, FIntPoint> CameFrom;
    TArray<FIntPoint> OpenSet;

    GScore.Add(Start, 0);
    FScore.Add(Start, Heuristic(Start, Goal));
    OpenSet.Add(Start);

    const TArray<FIntPoint> Dirs = { {1,0},{-1,0},{0,1},{0,-1} };

    while (OpenSet.Num() > 0)
    {
        //nodo con FScore minore nell'open set
        OpenSet.Sort([&](const FIntPoint& A, const FIntPoint& B) {
            return FScore.FindRef(A) < FScore.FindRef(B);
            });
        FIntPoint Current = OpenSet[0];
        OpenSet.RemoveAt(0);

        if (Current == Goal)
        {
            //ricostruisce il percorso a ritroso
            TArray<FIntPoint> Path;
            FIntPoint Step = Goal;
            while (Step != Start)
            {
                Path.Insert(Step, 0);
                Step = CameFrom[Step];
            }
            return Path;
        }

        for (const FIntPoint& Dir : Dirs)
        {
            FIntPoint Neighbor(Current.X + Dir.X, Current.Y + Dir.Y);
            if (Neighbor.X < 0 || Neighbor.X >= 25 || Neighbor.Y < 0 || Neighbor.Y >= 25) continue;

            AGridCell* NeighborCell = Grid->GetCell(Neighbor.X, Neighbor.Y);
            AGridCell* CurrentCell = Grid->GetCell(Current.X, Current.Y);

            //la cella Goal può essere non walkable solo se è la destinazione (es. attacco)
            if (Neighbor != Goal && !IsWalkable(NeighborCell)) continue;
            if (Neighbor == Goal && !NeighborCell) continue;

            int32 TentativeG = GScore.FindRef(Current) + MoveCost(CurrentCell, NeighborCell);
            if (TentativeG > MaxMoveCost) continue;

            if (!GScore.Contains(Neighbor) || TentativeG < GScore[Neighbor])
            {
                CameFrom.Add(Neighbor, Current);
                GScore.Add(Neighbor, TentativeG);
                FScore.Add(Neighbor, TentativeG + Heuristic(Neighbor, Goal));
                if (!OpenSet.Contains(Neighbor))
                    OpenSet.Add(Neighbor);
            }
        }
    }

    return {}; //nessun percorso trovato
}

TMap<FIntPoint, int32> UPathfindingComponent::GetReachableCells(AGridManager* Grid, FIntPoint Start, int32 MaxMoveCost)
{
    TMap<FIntPoint, int32> CostMap;
    TArray<TPair<int32, FIntPoint>> Queue;

    CostMap.Add(Start, 0);
    Queue.Add({ 0, Start });

    const TArray<FIntPoint> Dirs = { {1,0},{-1,0},{0,1},{0,-1} };

    while (Queue.Num() > 0)
    {
        Queue.Sort([](const TPair<int32, FIntPoint>& A, const TPair<int32, FIntPoint>& B) { return A.Key < B.Key; });
        auto [CurrentCost, Current] = Queue[0];
        Queue.RemoveAt(0);

        if (CostMap.Contains(Current) && CurrentCost > CostMap[Current]) continue;

        for (const FIntPoint& Dir : Dirs)
        {
            FIntPoint Neighbor(Current.X + Dir.X, Current.Y + Dir.Y);
            if (Neighbor.X < 0 || Neighbor.X >= 25 || Neighbor.Y < 0 || Neighbor.Y >= 25) continue;

            AGridCell* NeighborCell = Grid->GetCell(Neighbor.X, Neighbor.Y);
            AGridCell* CurrentCell = Grid->GetCell(Current.X, Current.Y);
            if (!IsWalkable(NeighborCell)) continue;

            int32 NewCost = CurrentCost + MoveCost(CurrentCell, NeighborCell);
            if (NewCost > MaxMoveCost) continue;

            if (!CostMap.Contains(Neighbor) || NewCost < CostMap[Neighbor])
            {
                CostMap.Add(Neighbor, NewCost);
                Queue.Add({ NewCost, Neighbor });
            }
        }
    }

    return CostMap;
}

int32 UPathfindingComponent::GetActualDistance(AGridManager* Grid, FIntPoint Start, FIntPoint Goal) const
{
    if (!Grid) return -1;

    TMap<FIntPoint, int32> GScore;
    TArray<TPair<int32, FIntPoint>> Queue;
    GScore.Add(Start, 0);
    Queue.Add({ 0, Start });
    const TArray<FIntPoint> Dirs = { {1,0},{-1,0},{0,1},{0,-1} };

    while (Queue.Num() > 0)
    {
        Queue.Sort([](const TPair<int32, FIntPoint>& A, const TPair<int32, FIntPoint>& B) { return A.Key < B.Key; });
        auto [Cost, Current] = Queue[0];
        Queue.RemoveAt(0);

        if (Current == Goal) return Cost;
        if (Cost > GScore.FindRef(Current)) continue;

        for (const FIntPoint& Dir : Dirs)
        {
            FIntPoint Neighbor(Current.X + Dir.X, Current.Y + Dir.Y);
            if (Neighbor.X < 0 || Neighbor.X >= 25 || Neighbor.Y < 0 || Neighbor.Y >= 25) continue;

            AGridCell* NeighborCell = Grid->GetCell(Neighbor.X, Neighbor.Y);
            AGridCell* CurrentCell = Grid->GetCell(Current.X, Current.Y);

            // permette di raggiungere il Goal anche se occupato (per calcolo distanza attacco)
            if (Neighbor != Goal && !IsWalkable(NeighborCell)) continue;
            if (!NeighborCell) continue;

            int32 NewCost = GScore.FindRef(Current) + MoveCost(CurrentCell, NeighborCell);
            if (!GScore.Contains(Neighbor) || NewCost < GScore[Neighbor])
            {
                GScore.Add(Neighbor, NewCost);
                Queue.Add({ NewCost, Neighbor });
            }
        }
    }
    return -1; // irraggiungibile
}
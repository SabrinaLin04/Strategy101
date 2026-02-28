#include "GridManager.h"
#include "Math/UnrealMathUtility.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;

    //valori default 
    GridWidth = 25;
    GridHeight = 25;
    CellSize = 100.f;
    NoiseSeed = 0;
    NoiseScale = 0.1f;
    WaterThreshold = 0.3f;
    MaxHeightOffset = 50.f;
    NoiseOffsetX = 0.f;
    NoiseOffsetY = 0.f;
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    GenerateGrid();
}

void AGridManager::GenerateGrid()
{
    ClearGrid();

    //inizializzazione seed
    if (NoiseSeed == 0)
    {
        FMath::RandInit(FDateTime::Now().GetTicks());
    }
    else
    {
        FMath::RandInit(NoiseSeed);
    }

    NoiseOffsetX = FMath::RandRange(0.f, 10000.f);
    NoiseOffsetY = FMath::RandRange(0.f, 10000.f);

    //prealloco array
    Cells.SetNum(GridWidth * GridHeight);

    //classe di fallback se non impostata in blueprint
    TSubclassOf<AGridCell> ClassToSpawn = GridCellClass
        ? GridCellClass
        : TSubclassOf<AGridCell>(AGridCell::StaticClass());

    //creo le celle
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Elevation = CalculateElevation(X, Y);

            //posizione mondo cella
            FVector WorldPos = GridToWorld(X, Y, Elevation);
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;

            AGridCell* NewCell = GetWorld()->SpawnActor<AGridCell>(
                ClassToSpawn, WorldPos, FRotator::ZeroRotator, SpawnParams);

            if (NewCell)
            {
                NewCell->GridX = X;
                NewCell->GridY = Y;
                NewCell->ElevationLevel = Elevation;
                NewCell->bIsOccupied = false;

                //definisco il tipo di cella in base al livello
                switch (Elevation)
                {
                case 0:  NewCell->CellType = ECellType::Water;      break;
                case 1:  NewCell->CellType = ECellType::Plain;      break;
                case 2:  NewCell->CellType = ECellType::Mountain2;  break;
                case 3:  NewCell->CellType = ECellType::Mountain3;  break;
                default: NewCell->CellType = ECellType::Mountain4;  break;
                }

                NewCell->UpdateVisualColor();
                Cells[X + Y * GridWidth] = NewCell;
            }
        }
    }

    //verifica connettività e corregge eventuali isole
    EnsureConnectivity();

    UE_LOG(LogTemp, Warning, TEXT("Grid generated: %d x %d cells"), GridWidth, GridHeight);
}

int32 AGridManager::CalculateElevation(int32 X, int32 Y) const
{
    float NX = (X + NoiseOffsetX) * NoiseScale;
    float NY = (Y + NoiseOffsetY) * NoiseScale;

    float NoiseValue = FMath::PerlinNoise2D(FVector2D(NX, NY));
    float Normalized = (NoiseValue + 1.f) * 0.5f;

    // Applica soglia acqua
    if (Normalized < WaterThreshold)
    {
        return 0;
    }

    //rimappa il range [WaterThreshold, 1] in [1, 4]
    float Remapped = (Normalized - WaterThreshold) / (1.f - WaterThreshold);
    int32 Level = FMath::Clamp(FMath::FloorToInt(Remapped * 4.f) + 1, 1, 4);

    return Level;
}

AGridCell* AGridManager::GetCell(int32 X, int32 Y) const
{
    if (!IsValidCoord(X, Y)) return nullptr;
    return Cells[X + Y * GridWidth];
}

bool AGridManager::IsValidCoord(int32 X, int32 Y) const
{
    return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

FVector AGridManager::GridToWorld(int32 X, int32 Y, int32 ElevationLevel) const
{
    //z proporzionale al livello di elevazione
    float ZOffset = ElevationLevel * MaxHeightOffset;
    return FVector(X * CellSize, Y * CellSize, ZOffset);
}

TArray<AGridCell*> AGridManager::GetNeighbors(int32 X, int32 Y) const
{
    TArray<AGridCell*> Neighbors;

    //solo movimenti ortogonali
    const int32 DX[] = { 0,  0, -1, 1 };
    const int32 DY[] = { -1, 1,  0, 0 };

    for (int32 i = 0; i < 4; i++)
    {
        int32 NX = X + DX[i];
        int32 NY = Y + DY[i];
        AGridCell* Neighbor = GetCell(NX, NY);
        if (Neighbor) Neighbors.Add(Neighbor);
    }

    return Neighbors;
}

void AGridManager::ClearGrid()
{
    for (AGridCell* Cell : Cells)
    {
        if (Cell) Cell->Destroy();
    }
    Cells.Empty();
}

bool AGridManager::IsCellWalkable(int32 X, int32 Y) const
{
    AGridCell* Cell = GetCell(X, Y);
    if (!Cell) return false;
    return Cell->ElevationLevel > 0;
}

void AGridManager::EnsureConnectivity()
{
    //trova la prima cella calpestabile come punto di partenza del BFS
    int32 StartX = -1, StartY = -1;
    for (int32 Y = 0; Y < GridHeight && StartX == -1; Y++)
        for (int32 X = 0; X < GridWidth && StartX == -1; X++)
            if (IsCellWalkable(X, Y)) { StartX = X; StartY = Y; }

    if (StartX == -1) return; // tutta acqua — caso estremo

    // BFS per visitare tutte le celle raggiungibili
    TArray<bool> Visited;
    Visited.SetNumZeroed(GridWidth * GridHeight);

    TQueue<FIntPoint> Queue;
    Queue.Enqueue(FIntPoint(StartX, StartY));
    Visited[StartX + StartY * GridWidth] = true;

    const int32 DX[] = { 0,  0, -1, 1 };
    const int32 DY[] = { -1, 1,  0, 0 };

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        for (int32 i = 0; i < 4; i++)
        {
            int32 NX = Current.X + DX[i];
            int32 NY = Current.Y + DY[i];

            if (!IsValidCoord(NX, NY)) continue;
            if (Visited[NX + NY * GridWidth]) continue;
            if (!IsCellWalkable(NX, NY)) continue;

            Visited[NX + NY * GridWidth] = true;
            Queue.Enqueue(FIntPoint(NX, NY));
        }
    }

    //celle calpestabili non visitate = isole isolate — le riempiamo a livello 1
    int32 FixedCells = 0;
    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            if (!IsCellWalkable(X, Y)) continue;
            if (Visited[X + Y * GridWidth]) continue;

            // Cella isolata — la abbassiamo a piano (livello 1) e la colleghiamo
            AGridCell* Cell = GetCell(X, Y);
            if (!Cell) continue;

            // Cerca il vicino calpestabile già connesso più vicino e allinealo
            for (int32 i = 0; i < 4; i++)
            {
                int32 NX = X + DX[i];
                int32 NY = Y + DY[i];
                if (!IsValidCoord(NX, NY)) continue;

                AGridCell* Neighbor = GetCell(NX, NY);
                if (Neighbor && Neighbor->ElevationLevel == 0)
                {
                    // Il vicino è acqua — lo trasformiamo in piano per creare un ponte
                    Neighbor->ElevationLevel = 1;
                    Neighbor->CellType = ECellType::Plain;
                    Neighbor->UpdateVisualColor();
                    Visited[NX + NY * GridWidth] = true;
                    FixedCells++;
                    break;
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Connectivity check: %d cells fixed"), FixedCells);
}
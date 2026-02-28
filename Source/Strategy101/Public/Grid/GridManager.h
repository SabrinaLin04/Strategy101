#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/GridCell.h"
#include "GridManager.generated.h"

UCLASS()
class STRATEGY101_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

    virtual void BeginPlay() override;

    //lista di parametri configurabili 

    //numero di colonne della griglia
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    int32 GridWidth;

    //numero di righe della griglia
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    int32 GridHeight;

    //dimensione in unità UE di ogni cella quadrata 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    float CellSize;

    //seed per la generazione casuale della mappa (0 = random ad ogni run)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    int32 NoiseSeed;

    //scala del Perlin Noise — valori bassi = terreno più uniforme 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    float NoiseScale;

    //soglia sotto la quale una cella diventa acqua (livello 0) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    float WaterThreshold;

    //altezza massima in unità UE per il livello 4 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    float MaxHeightOffset;

    //classe Blueprint della cella da spawnare
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Config")
    TSubclassOf<AGridCell> GridCellClass;

    //API pubblica

    //genero la griglia completa creo le celle e assegno i livelli
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void GenerateGrid();

    
    //ritorna la cella alle coordinate (X, Y), nullptr se fuori bounds.
    UFUNCTION(BlueprintCallable, Category = "Grid")
    AGridCell* GetCell(int32 X, int32 Y) const;

    //verifica se le coordinate sono dentro la griglia
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsValidCoord(int32 X, int32 Y) const;

    /**
     * conversione griglia per UE
     * @param ElevationLevel - livello elevazione per l'offset Z
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GridToWorld(int32 X, int32 Y, int32 ElevationLevel) const;

    /**
     * ritorna tutte le celle adiacenti (su, giu, sinistra, destra) a (X, Y)
     * esclude celle fuori bounds.
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    TArray<AGridCell*> GetNeighbors(int32 X, int32 Y) const;

    /**
     * distrugge tutte le celle esistenti e pulisce il array
     * usata prima di rigenerare la mappa
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void ClearGrid();

protected:
    //calcola il livello di elevazione per la cella (X, Y) usando Perlin Noise.
    int32 CalculateElevation(int32 X, int32 Y) const;

    //array bidimensionale delle celle con accesso tramite Cells[X + Y * GridWidth]
    UPROPERTY()
    TArray<AGridCell*> Cells;

    //offset X casuale per variare il Perlin Noise ad ogni run
    float NoiseOffsetX;

    //offset Y casuale
    float NoiseOffsetY;
};
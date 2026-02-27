// Fill out your copyright notice in the Description page of Project Settings.


#include "Units/Brawler.h"

// Sets default values
ABrawler::ABrawler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABrawler::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABrawler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


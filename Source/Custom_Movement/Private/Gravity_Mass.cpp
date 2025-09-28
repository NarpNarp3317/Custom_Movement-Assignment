// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity_Mass.h"

// Sets default values
AGravity_Mass::AGravity_Mass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGravity_Mass::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGravity_Mass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


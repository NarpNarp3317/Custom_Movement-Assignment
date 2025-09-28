// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyPlayerController.h"
#include "TargetLockOnComponent.generated.h"

/*
this is the target lock on component that will enable lock on the target on a screen and switch the target based on
the directional input from the mouse or gamepad. this will only trace the custom collision chanel "Lock on target trace"
and can be used for any perspective. Fuck yeah
*/

// Use delegate to update locked on and off setting to other class
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetLockedOn, bool);

DECLARE_MULTICAST_DELEGATE_OneParam(FDelegateUpdateTarget, AActor*);


UCLASS( ClassGroup=(LockOnTarget), meta=(BlueprintSpawnableComponent) )
class CUSTOM_MOVEMENT_API UTargetLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	//Declare!!!
	FOnTargetLockedOn OnLockedChanged;
	FDelegateUpdateTarget DelegateUpdateTarget;
	
	// Sets default values for this component's properties
	UTargetLockOnComponent();
private:

	float ToleranceValue=0.01;// could it be just shared globally?

	UPROPERTY(EditAnywhere)
	float ScoreRatio=0.5;// alpha value(0~1), score ratio for finding most suitable target when switching the starget by input direction
	
	UPROPERTY(EditAnywhere)
	float ScreenTraceWidth;
	
	UPROPERTY()
	AMyPlayerController* PlayerController;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bIsDebugPrint;// to check debug draw or no draw

	bool bIsLockedOnTarget;

	
	FVector2D CurrentTargetCoord;// this is the coord of current locked target
	
	FVector2D ScreenCenterCoord;
	int32 ScreenX_Width;
	int32 ScreenY_Height;
	
	// controller has information of screen size and coord. so no need to connect with hud class for this
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="LockonTarget")
	TEnumAsByte<ECollisionChannel> CollisionChanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="LockonTarget")
	float MaxLockOnDistance;// this will prevent lock on to far object

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="LockonTarget")
	TArray<AActor*> DetectedTargets;// in these targets, find the closest one and set it as locked target
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="LockonTarget")
	AActor* CurrentTarget;//locked on target

//===== Functions ====================================================================================================//
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void UpdateTargetStatus(float DeltaTime);
	// this is for detecting the change and update(trace target and update infos, also for debug sphere drawing)
	
	// update the current target.
	void UpdateLockedTarget(AActor* Target);

	UFUNCTION(BlueprintPure)
	AActor* GetLockedTarget() const;
	
	//this will be used to find the closest target to the screen coord.
	AActor* FindClosestTargetInScreenCoord(const FVector2D& Coord);
	
	// this will find the closest actor near the center of the screen
	AActor* FindClosestTargetInScreenCenter();
	
	//target switching. find the nearest actor on input's direction
	AActor* FindTargetByInputDirection(const FVector2D& InputDirection);


	//====== Binded Action =====// ---> this will be binded to world interaction IMC
	UFUNCTION()
	void ToggleTargetting();
	// switch for lock on, lock off
	
	UFUNCTION()
	void SwitchTarget(const FVector2D& InputDirection);

	//===== Confirmation ======//

	bool IsStillValid() const;
	bool IsStillVisible();
	bool IsTooFar() const;
	UFUNCTION(BlueprintPure)
	bool IsLockedOnTarget() const;

	bool ScanTargetsByScreenTrace(float Scale);// put traced targets in the range for later comparison.
	
	bool ScanTargetsByLastRenderTime();//version 2, find actors in the screen by finding rendered actors from previous frame!!!!
	
	AActor*FindMostAcceptableTarget(const FVector2D& ReferenceCoord, const FVector2D& InputDirection, bool bUseInputDirection);// this will be used as the helper function for finding suitable target
	
	bool UpdateTargetScreenCoord(AActor* Target);
	bool UpdateTargetWorldLocation(AActor* Target, FVector& WorldLocation);// return false if the target is invalid
	//return true if the target is valid, and change the Location vector directly, remotely to use it as ref
	
	AActor* FindNextNearestTarget();
	// this will happen when the target actor is no longer visible on a screen (ex. out of screen frame or target died)

	void LockOn();
	void LockOff();
	// set camera roation to normal--> when ther is no target found on screen / or / simply toggled py input

	void DrawDebugSphereOnTarget();


};

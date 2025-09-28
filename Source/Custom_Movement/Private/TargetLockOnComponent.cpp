// Fill out your copyright notice in the Description page of Project Settings.


#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"// for drawing debug
#include "EnhancedInputComponent.h"
#include "Custom_LogCategory.h"
#include "TargetLockOnComponent.h"

#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UTargetLockOnComponent::UTargetLockOnComponent():
	CurrentTarget(nullptr),
	PlayerController(nullptr),
	bIsLockedOnTarget(false),
	bIsDebugPrint(false),
	MaxLockOnDistance(20000.0f),// default distance
	ScreenTraceWidth(100)//default screen trace width

	//CollisionChanel(ECC_GameTraceChannel1)//custom trace chanel for lock on target

{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTargetLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<AMyPlayerController>(GetOwner()->GetInstigatorController());
	if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("PlayerController is nullptr in beginplay"));
		return;
	}
	// get the size from the controller and store it in membervariable
	PlayerController->GetViewportSize(ScreenX_Width, ScreenY_Height);
	// Set Center coord
	ScreenCenterCoord.X = ScreenX_Width / 2.0f;
	ScreenCenterCoord.Y = ScreenY_Height / 2.0f;
	
	UE_LOG(LockOnTarget_Log, Log, TEXT("BeginPlay OK. Viewport: %d x %d  Center: (%.1f, %.1f)"),
	   ScreenX_Width, ScreenY_Height, ScreenCenterCoord.X, ScreenCenterCoord.Y);
}

void UTargetLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("PlayerController is null"));
		return;
	}
	
	if (bIsLockedOnTarget)// only update when it is locked on
	{
		UpdateTargetStatus(DeltaTime);
	}

	//debug draw
	DrawDebugSphereOnTarget();

}

void UTargetLockOnComponent::UpdateTargetStatus(float DeltaTime)// this is only done when target lockon is enabled
{
	if (!PlayerController||!bIsLockedOnTarget) return;
	
	if (!IsStillValid()||!IsStillVisible()||IsTooFar())// conditions for invalid targets
	{
		AActor* NewTarget=FindClosestTargetInScreenCoord(CurrentTargetCoord);

		if (NewTarget)
		{
			UpdateLockedTarget(NewTarget);
			UE_LOG(LockOnTarget_Log,Log,TEXT("Switched Target : %s"), *NewTarget->GetName());
		}
		else
		{
			LockOff();
			
			UE_LOG(LockOnTarget_Log,Log,TEXT("No Valid target Detected"));
			return;
		}
	}
	else
	{
		UpdateTargetScreenCoord(CurrentTarget);// update the current targets coord
	}

	if (bIsDebugPrint&&CurrentTarget)// if debug draw is on and current target is valid
	{
		DrawDebugSphere(GetWorld(),CurrentTarget->GetActorLocation(),30.0f,20,FColor::Green);
	}
	
}

void UTargetLockOnComponent::UpdateLockedTarget(AActor* Target)
{
	if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("PlayerController is null"));
		LockOff();
		return;
	}
	if (!Target)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("Target is null"));
		LockOff();
		return;
	}
	
	CurrentTarget = Target;
	if (!bIsLockedOnTarget)
	{
		LockOn();
	}
}

AActor* UTargetLockOnComponent::GetLockedTarget() const
{
	return CurrentTarget;
}

AActor* UTargetLockOnComponent::FindClosestTargetInScreenCoord(const FVector2D& Coord)
{
	/*if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("PlayerController is null"));
		return nullptr;
	}

	AActor* ClosestTarget = nullptr;//make empty holder for most acceptable target

	//float MinDist=3.402823466 E + 38;
	float MinDist= MAX_FLT;// macro for max value of float!!!!

	for (AActor* Target: DetectedTargets)
	{
		if (!Target) continue;

		FVector2D ScreenCoord;
		bool bIsTargetVisible = PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(),ScreenCoord);
		//https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/GameFramework/APlayerController/ProjectWorldLocationToScreen?application_version=5.6
		// fuck yeah!!! no need for manually rasterizing the screen to put line trace on each section of screen
		//--> basically convert the 3d position into 2d position based on the screen
		// returns bool( no detection== false, found == true, set balue in screencoord)
		if (!bIsTargetVisible) continue;// not visible on screen

		float Distance =FVector2D::Distance(Coord, ScreenCoord);
		if (Distance > MinDist) continue;

		float DistanceFromPawn=FVector::Distance(Target->GetActorLocation(),PlayerController->GetPawn()->GetActorLocation());
		if (DistanceFromPawn >MaxLockOnDistance) continue; // if its out of range

		MinDist = Distance;// set new shortest distance for loop
		ClosestTarget = Target;// and set it as most suitable target

	}
	//loop is done
	return ClosestTarget;*/// helper for removing unnecessary repetition!!!!!! fuck yeah

	return FindMostAcceptableTarget(Coord, {0,0},false);// no direction needed
}

AActor* UTargetLockOnComponent::FindClosestTargetInScreenCenter()
{
	return FindClosestTargetInScreenCoord(ScreenCenterCoord);
}

AActor* UTargetLockOnComponent::FindTargetByInputDirection(const FVector2D& InputDirection)
{
	/*if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("PlayerController is null"));
		return nullptr;
	}
	
	if (!CurrentTarget)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("Target is null"));
		return nullptr;
	}


	/*
	there is 2 things to consider for finding most suitable target
	1. resemblance of direction --> use dot
	2. closest target --> min distance

	how?
	set scores based on those 2 factors and compare in the last-->
	#1#
	
	AActor* HighestScoreTarget = nullptr;
	float MinDist= MAX_FLT;
	float HighestScore=0;// from 0
	FVector2D NormalizedInputDirection=InputDirection.GetSafeNormal();//normalize it for direction(input can be (1,1,0)--> not normalized
	
	for (AActor* Target: DetectedTargets)
	{
		/*if (!Target)
		{
			UE_LOG(LockOnTarget_Log,Error,TEXT("Target is null"));
			continue;
		}
		if (Target=CurrentTarget)
		{
			UE_LOG(LockOnTarget_Log,Error,TEXT("Target is already target"));
			continue;
		}#1#//--> no need for log in here

		if (!Target||Target==CurrentTarget) continue;// if the target is invalid or it is same as the current target, pass
		FVector2D TracedTargetCoord;
		if (!PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(),TracedTargetCoord)) continue;
		// no trace--> false// detected--> set coord to ScreenCoord
		//=== Invalid Filtered ====//

		FVector2D Direction=(TracedTargetCoord-CurrentTargetCoord).GetSafeNormal();// get local direction by subtracting
		
		//1. distance compare
		float Distance=FVector2D::Distance(CurrentTargetCoord,TracedTargetCoord);
		if (Distance < MinDist)// new shortest distance found
		{
			MinDist=Distance;//update
		}

		float Resemblance=FVector2D::DotProduct(Direction,NormalizedInputDirection);// get resemblance value by the dot product
		if (Resemblance<0) continue;// ignore the current target if the direction is opposite


		//float Score=((1/Distance)*(ScoreRatio))/*Distance Score#1#+ Resemblance*(1-ScoreRatio)/*Angle Resemblance score#1#;
		// ((1/Distance)*(ScoreRatio)) the score goes abnormally gets bigger when distance is less than 1(ex. 0.001)

		float Score=(FMath::Max((1/Distance),1)*(ScoreRatio))/*Distance Score#1#+ Resemblance*(1-ScoreRatio);
		//clamp the max score of distance to be 1--> 1*score ratio== max score for distance
		if (Score<HighestScore) continue;

		HighestScore=Score;
		HighestScoreTarget = Target;
	}
	// loop done
	return HighestScoreTarget;*///---> use helper to make it simpler!!!!

	return FindMostAcceptableTarget(CurrentTargetCoord,InputDirection,true);// fuck, how elegant is this!!!!
}

void UTargetLockOnComponent::ToggleTargetting()// initializer of lock on--> find from center
{
	// just toggle the lock on setting
	if (bIsLockedOnTarget)
	{
		LockOff();
	}
	else
	{
		LockOn();
	}
}

void UTargetLockOnComponent::SwitchTarget(const FVector2D& InputDirection)
{
	
	if (!CurrentTarget)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("CurrentTarget is null"));
		return;
	}
	
	if (InputDirection.IsNearlyZero(ToleranceValue))
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("MovementValue is 0 or too small"));
		return;
	}

	AActor* NextTarget=FindTargetByInputDirection(InputDirection);
	if (!NextTarget)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("No Target to Switch"));
		return;
	}
	
	UpdateLockedTarget(NextTarget);
	DelegateUpdateTarget.Broadcast(CurrentTarget);// update in delegate too
	UE_LOG(LockOnTarget_Log,Log,TEXT("Switched Target : %s"), *CurrentTarget->GetName());
	
}

bool UTargetLockOnComponent::IsStillValid() const
{
	/*bool result=true;
	
	if (!CurrentTarget)
	{
		result=false;
	}

	if (!DetectedTargets.Contains(CurrentTarget))
	{
		result=false;
	}
	
	return result;*/
	if (!CurrentTarget)return false;
	if (DetectedTargets.IsEmpty())return false;

	return true;
}

bool UTargetLockOnComponent::IsStillVisible()
{
	if (!CurrentTarget) return false;

	FVector2D TargetCoord;
	bool bIsVisible=PlayerController->ProjectWorldLocationToScreen(CurrentTarget->GetActorLocation(),TargetCoord);
	if (bIsVisible)
	{
		CurrentTargetCoord=TargetCoord;// update the current current target's coord
	}
	
	return bIsVisible;
}

bool UTargetLockOnComponent::IsTooFar() const
{
	if (!CurrentTarget) return true;
	float Distance=FVector::Distance(CurrentTarget->GetActorLocation(),PlayerController->GetPawn()->GetActorLocation());
	return Distance>MaxLockOnDistance;//if distance is longer than limit, return true/ else false
}

bool UTargetLockOnComponent::IsLockedOnTarget() const
{
	return bIsLockedOnTarget;
}

bool UTargetLockOnComponent::ScanTargetsByScreenTrace(float Scale)
{
    DetectedTargets.Empty(); // reset previous candidates

    if (!PlayerController || !GetWorld()) return false;

    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // Get screen size
    

    // basically, raseterize the screen and set X,Y coord for line trace
    int32 ScreenX_Coord = FMath::Max(1, FMath::CeilToInt(ScreenX_Width / Scale));
    int32 ScreenY_Coord = FMath::Max(1, FMath::CeilToInt(ScreenY_Height / Scale));
	
    for (int32 x = 0; x <= ScreenX_Coord; ++x)// hello again! you fucking simple loop
    {
        for (int32 y = 0; y <= ScreenY_Coord; ++y)// both for 2d vector of the screen
        {
            // Compute screen coordinates for this grid point
            float ScreenX = ScreenX_Width * x / ScreenX_Coord;
            float ScreenY = ScreenY_Height * y / ScreenY_Coord;

            FVector LineStartLocation, LineDirection;//empty storage
        	
            if (PlayerController->DeprojectScreenPositionToWorld(ScreenX, ScreenY, LineStartLocation, LineDirection))
            	//https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/GameFramework/APlayerController/DeprojectScreenPositionToWorld?application_version=5.6
            	//Deproject--> uplike project screen--> convert 2d space to 3d space
            {
                FVector LineEnd = LineStartLocation + LineDirection * MaxLockOnDistance;// get end point for each linetrace

                FHitResult HitResult;//empty storage
            	FCollisionObjectQueryParams ObjectQueryParams;
                FCollisionQueryParams Params;
            	
            	ObjectQueryParams.AddObjectTypesToQuery(CollisionChanel);// put collision type here
                Params.AddIgnoredActor(GetOwner());//set actors to ignore in here

                //bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, LineStartLocation, LineEnd, CollisionChanel, Params);
            	GetWorld()->LineTraceSingleByObjectType(HitResult, LineStartLocation,LineEnd, ObjectQueryParams,Params);
				bool bHit=HitResult.bBlockingHit;
            	
            	if (bIsDebugPrint)//only draw when debug draw is enabled
            	{
            		FColor Detectioncolor=bHit ? FColor::Green : FColor::Red;
            		DrawDebugLine(GetWorld(),LineStartLocation,LineEnd,Detectioncolor,false,1.0f,0,1.0f);
            	}// draw line trace indicator
               

                if (bHit)// hit Detected!!!!
                {
                    if (AActor* HitActor = HitResult.GetActor())
                    {
                        DetectedTargets.AddUnique(HitActor);
                    	UE_LOG(LockOnTarget_Log, Log, TEXT("%s Traced from the screen"), *HitActor->GetName());

                        if (bIsDebugPrint)// draw debug line at detected actor
                        {
                            DrawDebugSphere(GetWorld(), HitActor->GetActorLocation(), 50.f, 12, FColor::Red, false, 2.0f);
							// Draw debug sphere on every detected targets.--> indication for targetable trace is done or not
                        }
                    }
                }
            }
        }
    }
    if (DetectedTargets.IsEmpty()) return false;
	return true;
}

bool UTargetLockOnComponent::ScanTargetsByLastRenderTime()//well.. not so sure how to use it... fuck
{
	DetectedTargets.Empty();//reset

	if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log, Error, TEXT("PlayerController is null"));
		return false;
	}
	if (!GetWorld())
	{
		UE_LOG(LockOnTarget_Log, Error, TEXT("GetWorld() is null"));
		return false;
	}

	float CurrentTime=GetWorld()->GetTimeSeconds();

	for (AActor* RenderedActors:DetectedTargets)
	{
		
	}
	return true;
}

AActor* UTargetLockOnComponent::FindMostAcceptableTarget(const FVector2D& ReferenceCoord,const FVector2D& InputDirection, bool bUseInputDirection)
{
	if (!PlayerController) return nullptr;

	AActor* MostAcceptableTarget=nullptr;//empty
	float BestScore=0;
	float MinDist=MAX_FLT;

	FVector2D NormalizedInputDirection=InputDirection.GetSafeNormal();

	for (AActor* TargetActor : DetectedTargets)
	{
		FVector2D ScreenCoord;
		if (!PlayerController->ProjectWorldLocationToScreen(TargetActor->GetActorLocation(),ScreenCoord)) continue;
		
		float Distance=FVector2D::Distance(ScreenCoord,ReferenceCoord);
		float DistanceFromPawn=FVector::Distance(TargetActor->GetActorLocation(),PlayerController->GetPawn()->GetActorLocation());

		if (DistanceFromPawn>MaxLockOnDistance) continue;

		if (!bUseInputDirection)// pick closeset
		{
			if (Distance<MinDist)
			{
				MinDist=Distance;
				MostAcceptableTarget=TargetActor;
			}
		}
		else // based on the score
		{
			FVector2D Direction=(ScreenCoord-ReferenceCoord).GetSafeNormal();
			float Resemblance=FVector2D::DotProduct(Direction,NormalizedInputDirection);
			if (Resemblance<0) continue;

			float Score=FMath::Max((1/Distance),1)*(ScoreRatio)+Resemblance*(1-ScoreRatio);
			if (Score>BestScore)
			{
				BestScore=Score;
				MostAcceptableTarget=TargetActor;
			}
		}
	}
	// loop done!!!
	return MostAcceptableTarget;
}


bool UTargetLockOnComponent::UpdateTargetScreenCoord(AActor* Target)
{
	if (!Target||!PlayerController) return false;

	FVector2D ScreenCoord;
	if (PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(),ScreenCoord))
	{
		CurrentTargetCoord=ScreenCoord;
		return true;
	}
	return false;
}

bool UTargetLockOnComponent::UpdateTargetWorldLocation(AActor* Target, FVector& WorldLocation)
{
	if (!Target) return false;// no target to update

	WorldLocation=Target->GetActorLocation();
	return true;
}

AActor* UTargetLockOnComponent::FindNextNearestTarget()
{
	CurrentTarget=FindClosestTargetInScreenCoord(CurrentTargetCoord);
	return CurrentTarget;
}

void UTargetLockOnComponent::LockOn()//initiator of the targetting. then switch the lock on target next
{
	if (!PlayerController)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("PlayerController is null"));
		return;
	}
	
	if (!CurrentTarget)// only scan when target is not set already
	{
		ScanTargetsByScreenTrace(ScreenTraceWidth);// trace all targets for comparison
		//test for better scanning
		//ScanTargetsByFrustum();
		CurrentTarget = FindClosestTargetInScreenCenter();
	}

	if (!CurrentTarget)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("Target is null"));
		return;
	}
	
	if (!bIsLockedOnTarget)
	{
		bIsLockedOnTarget=true;
		UE_LOG(LockOnTarget_Log,Log,TEXT("Locked Target : %s"), *CurrentTarget->GetName());
		
		OnLockedChanged.Broadcast(true);// tell the world that target is locked!!!!
		// send signal to hud class to draw lock on indicator on a target coord


		DelegateUpdateTarget.Broadcast(CurrentTarget);
		
		//TODO--> Make HUD Class to print the indicator
	}
}

void UTargetLockOnComponent::LockOff()
{
	bIsLockedOnTarget = false;
	CurrentTarget = nullptr;

	//use delegate to update
	OnLockedChanged.Broadcast(false);// spread the word that target is locked off!!!!
	DelegateUpdateTarget.Broadcast(nullptr);// wont be necessary // 

	UE_LOG(LockOnTarget_Log,Log,TEXT("Lockoff"));
}

void UTargetLockOnComponent::DrawDebugSphereOnTarget()
{
	if (!bIsDebugPrint||!bIsLockedOnTarget||!CurrentTarget) return;
	
	DrawDebugSphere(GetWorld(), CurrentTarget->GetActorLocation(), 60.f, 12, FColor::Green, false, 0.0f);
	
}




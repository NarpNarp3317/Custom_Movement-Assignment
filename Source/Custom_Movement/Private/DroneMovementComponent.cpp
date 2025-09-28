// Fill out your copyright notice in the Description page of Project Settings.

#include "DroneMovementComponent.h"

#include "MaterialHLSLTree.h"
#include "RewindData.h"
#include "VectorTypes.h"
#include "Kismet/GameplayStatics.h"// to get camera 
#include "Kismet/KismetMathLibrary.h"// rotation to forward, right, up vector
#include "Custom_LogCategory.h"
#include "TargetLockOnComponent.h"

// Sets default values for this component's properties



UDroneMovementComponent::UDroneMovementComponent():
	Owner(nullptr),Updated_Base_Comp(nullptr),TargetedActor(nullptr),LiftForce(1000)//temp default value
{
	UE_LOG(MoveComp_Log,Log,TEXT("Constructor Activated"));
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}


// Called when the game starts
void UDroneMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(MoveComp_Log,Log,TEXT("BeginPlay Activated"));
	//Acceleration=500;//temp--> this should be done in character
	//Deceleration=500,// same as accel
	Current_MaxSpeed=MaxFlySpeed;// temp
	GravityDirection={0,0,-1};// temp, default value for no gravity mass class
	bIsinAir=true;//temp
	bIsMoving=false;//temp
	bIsLockedOn=false;
	bIsGravityOn=true;//temp default for drone flight
	
	// ...

	//Bind Delegator to activate the function

	AActor*MyOwner=GetOwner();
	if (!MyOwner)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("GetOwner NULL"));
		return;
	}
	// owner valid

	
	if (UTargetLockOnComponent* LockComp=MyOwner->FindComponentByClass<UTargetLockOnComponent>())
	{
		LockComp->OnLockedChanged.AddLambda([this](bool bIsLockedOn)
		{
			this->bIsLockedOn=bIsLockedOn;
			UE_LOG(LockOnTarget_Log,Log,TEXT("bIsLockedOn=%d"),bIsLockedOn);
		});

		LockComp->DelegateUpdateTarget.AddLambda([this](AActor* Target)
		{
			this->TargetedActor=Target;
		});

	}
}


// Called every frame
void UDroneMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)// where does this function get activated?
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Updated_Base_Comp) return;

	Set_Forward_Right_Up_Vector_BasedOnCamera();// update it per tick

	//====Gravity ====//
	if (bIsGravityOn)
	{
		//only apply this when gravity is on
		ApplyGravity(DeltaTime);//Apply gravity
	}

	
	//ApplyGravity();// apply gravity
	UpdateMovementInput(DeltaTime);// update movement
	UpdateMovementBoolValues();
	//UE_LOG(LogTemp,Error,TEXT("Velocity: X=%f, Y=%f, Z=%f"),Velocity.X,Velocity.Y,Velocity.Z);--> not for every tick in log

	//to ease out the boost deceleration
	float Speed= bIsBoosting? MaxFlySpeed*Boost_Multiplier:MaxFlySpeed;
	Current_MaxSpeed=FMath::FInterpTo(Current_MaxSpeed,Speed,DeltaTime,5);


	if (!bIsLockedOn)
	{
		FVector InputDirection=Velocity.GetSafeNormal();// not velocity!!! rotate to controller input direction
		if (!DidRotateTowardsDirection( InputDirection))// rotate till the rotation matches the direction
		{
			RotateTowardMovementDirection(InputDirection, DeltaTime);//--> temp
			//RotateTowardMovementDirection_DroneMovement(InputDirection, DeltaTime);// change it into drone rotation type
			// not so sure how to work with this, so, pass(Should it only tilt the skeletal or static mesh or the full root comp?)
		}
	}
	else// locked on rotaion update
	{
		LockedWorldLocation= TargetedActor->GetActorLocation();

		UE_LOG(LockOnTarget_Log,Log,TEXT("%s"),*LockedWorldLocation.ToString());
		RotateTowardWorldLocation(LockedWorldLocation, DeltaTime);
	}

	MovementInput={0,0,0};// reset here, so that rotation can be done

}

void UDroneMovementComponent::SetOwner_in_Move_Comp(APawn* newOwner)
{
	if (!newOwner)
	{
		UE_LOG(MoveComp_Log,Error,TEXT("Invalid OwnerPtr"));
		return;
	}
	UE_LOG(MoveComp_Log,Log,TEXT("Constructor Activated"));
	//add log for owner setting failed
	Owner = newOwner;
}

void UDroneMovementComponent::SetUpdatedComp(UCapsuleComponent* Updated_Comp)
{
	if (!Updated_Comp)
	{
		UE_LOG(MoveComp_Log,Error,TEXT("Invalid Updated Comp"));
		return;
	}
	Updated_Base_Comp=Updated_Comp;
	UE_LOG(MoveComp_Log,Log,TEXT("Updated Comp Settled successfully"));
}

ECustomMovementMode UDroneMovementComponent::GetMovementMode() const
{
	return MovementMode;
}

bool UDroneMovementComponent::IsInAir() const
{
	return bIsinAir;
}

bool UDroneMovementComponent::IsMoving() const
{
	return bIsMoving;
}

bool UDroneMovementComponent::IsBoosting() const
{
	return bIsBoosting;
}

bool UDroneMovementComponent::IsLockedOn() const
{
	return bIsLockedOn;
}

void UDroneMovementComponent::SetbIsLockedOn(bool newIsLockedOn)
{
	bIsLockedOn=newIsLockedOn;
}

ECustomMovementMode UDroneMovementComponent::GetMovementType() const
{
	return MovementMode;
}

void UDroneMovementComponent::UpdateMovementBoolValues()
{
	//temp checking
	if (Velocity.IsNearlyZero(ToleranceValue))
	{
		bIsMoving=false;
		//UE_LOG(LogTemp,Warning,TEXT("character is not moving"));
	}
	else
	{
		bIsMoving=true;
		//UE_LOG(LogTemp,Warning,TEXT("character is moving"));
	}
}

void UDroneMovementComponent::ApplyGravityOrNot(bool YesOrNo)
{
	bIsGravityOn=YesOrNo;
}

void UDroneMovementComponent::Set_Forward_Right_Up_Vector_BasedOnCamera()// should it be updated by tick?
{
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);
	if (!PlayerCameraManager)
	{
		UE_LOG(LogTemp,Error,TEXT("PlayerCameraManager is null"));
		return;
	}
	
	FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
	
	// if the character is on the ground, rotate camera rotation till the up vector of camera's up vector matches with gravity direction
	if (!bIsinAir)// temp --> make it back to !bisinAir
	{
		FVector CameraUp=UKismetMathLibrary::GetUpVector(CameraRotation);
		FQuat Alignment_Quat= UKismetMathLibrary::Quat_FindBetweenNormals(CameraUp, GravityDirection);

		CameraRotation=(CameraRotation.Quaternion()*=Alignment_Quat).Rotator();
		// make new camera rotator for movement alined with surface normal
	}
	
	/*
	//https://forums.unrealengine.com/t/trying-to-get-the-forward-and-right-vector-from-a-rotation-c/320420
	ForwardVector=FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::X);
	RightVector=FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);
	UpVector=FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Z);
	*/
	//--> or use KismetMathLibrary!!! fuck yeah
	
	ForwardVector=UKismetMathLibrary::GetForwardVector(CameraRotation);
	RightVector=UKismetMathLibrary::GetRightVector(CameraRotation);
	UpVector=UKismetMathLibrary::GetUpVector(CameraRotation);
}

FVector UDroneMovementComponent::GetVelocity()
{
	return Velocity;
}

FVector UDroneMovementComponent::Get_Decel_Velocity()
{
	return DecelerationVelocity;
}

FVector UDroneMovementComponent::GetForwardVector()
{
	return ForwardVector;
}

FVector UDroneMovementComponent::GetRightVector()
{
	return RightVector;
}

FVector UDroneMovementComponent::GetUpVector()
{
	return UpVector;
}


void UDroneMovementComponent::MoveForward(float Value, float DeltaTime)// this only adds value to movement input --> tick decides when to move
{
	if (!Updated_Base_Comp) return;// no capsule, return
	if (FMath::IsNearlyZero(Value,ToleranceValue)) return;// no movement, return
	 MovementInput.X=Value;

	//--> this method disables movement immediatley when stop recieving input. for smooth deceleration, 
	
	//float DeltaTime = GetWorld()->GetDeltaSeconds();// get delta second and use it to calculate location after movement per tick
	//--> Get Delta Time from the argument, that would work better with other movement functions
	
	/*
	//get current velocity and add forward vector to addup 
	Velocity+=ForwardVector*Value*Acceleration*DeltaTime;

	//Velocity=UKismetMathLibrary::Vector_ClampSizeMax(Velocity,Current_MaxSpeed); --> for blueprint
	Velocity=Velocity.GetClampedToMaxSize(Current_MaxSpeed);// directly clamp in c++

	/*
	FVector Newlocation=Owner->GetActorLocation()+Velocity*DeltaTime;
	// find new location per delta tick
	Owner->SetActorLocation(Newlocation);// and set Owning Actor to new Location
	#1#
	//This wont work with collision hit, it will simply go through the colliding mesh and clip in.

	FVector Movement=Velocity*DeltaTime;
	FHitResult Hit;
	Updated_Base_Comp->MoveComponent(Movement, Updated_Base_Comp->GetComponentRotation(),true,&Hit);
	// move Scene Component(root of capsule component)

	if (Hit.IsValidBlockingHit())
	{
		//SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit);//--> this is movement component's function. this is an empty actor component
	}*/
	
}

void UDroneMovementComponent::MoveRight(float Value, float DeltaTime)
{
	if (!Updated_Base_Comp) return;// no capsule, return
	if (FMath::IsNearlyZero(Value,ToleranceValue)) return;// no movement, return

	 MovementInput.Y=Value;
	
}

void UDroneMovementComponent::MoveUp(float Value, float DeltaTime)
{
	if (!Updated_Base_Comp) return;// no capsule, return
	if (FMath::IsNearlyZero(Value,ToleranceValue)) return;// no movement, return

	 MovementInput.Z=Value;
}

void UDroneMovementComponent::RotateYaw(float Value, float DeltaTime)
{
	if (!Updated_Base_Comp) return;// no capsule, return
	//if (FMath::IsNearlyZero(Value,ToleranceValue)) return;// no input, return

	if (bIsLockedOn)
	{
		// do not add rotation by controller when it is locked on to target. the view and forward vector is looking at the target
		return;
	}
	CurrentRotation=Updated_Base_Comp->GetComponentRotation();
	FRotator DeltaRotation = (RightVector*RotationSpeed*DeltaTime).Rotation();

	FRotator NewWorldRotation=CurrentRotation+DeltaRotation;// make world rotation
	Updated_Base_Comp->SetWorldRotation(NewWorldRotation);
}

void UDroneMovementComponent::LiftBody(float DeltaTime)
{
	if (!Updated_Base_Comp) return;
	
	Velocity+=-GravityDirection*LiftForce*DeltaTime;// add the up vector for the movement change.
	Velocity=Velocity.GetClampedToMaxSize(Current_MaxSpeed);// limit the velocity to its max speed
}


// rotate the body after boosting is done? --> tilt back to normal(roll, pitch to local 0)
void UDroneMovementComponent::BalanceMovement(float DeltaTime)
{
if (!Updated_Base_Comp) return;
	
	// Keep forward direction based on velocity if moving, else use current forward
	FVector ForwardDir = Velocity.IsNearlyZero(ToleranceValue) ?  CurrentRotation.Vector() : Velocity.GetSafeNormal();

	// Align up with gravity
	FVector DesiredUp = -GravityDirection.GetSafeNormal();

	// Build rotation matrix from forward and up
	FQuat TargetQuat = FRotationMatrix::MakeFromXZ(ForwardDir, DesiredUp).ToQuat();
	FRotator TargetRot = TargetQuat.Rotator();

	// Interpolate toward target rotation
	FRotator NewRot = FMath::RInterpTo( CurrentRotation, TargetRot, DeltaTime, RotationSpeed);
	
	NewRot.Yaw =  CurrentRotation.Yaw;

	// Apply rotation
	Updated_Base_Comp->SetWorldRotation(NewRot);

	// Update stored current rotation
	CurrentRotation = NewRot;
}


void UDroneMovementComponent::StartBoost(bool value)
{
	if (!Updated_Base_Comp) return;// no capsule, return
	//if (!value) return;
	UE_LOG(MoveComp_Log,Log,TEXT("StartBoost Activated"));
	Current_MaxSpeed=MaxFlySpeed*Boost_Multiplier;
	bIsBoosting=true;
}

void UDroneMovementComponent::StopBoost(bool value)
{
	if (!Updated_Base_Comp) return;// no capsule, return
	//if (value)return;
	UE_LOG(MoveComp_Log,Log,TEXT("StopBoost Activated"));
	Current_MaxSpeed=MaxFlySpeed;
	bIsBoosting=false;
}

void UDroneMovementComponent::UpdateMovementInput(float DeltaTime)
{
	FVector WantedMovementDirection=
		(ForwardVector*MovementInput.X)+(RightVector*MovementInput.Y)+(UpVector*MovementInput.Z);

	if (!WantedMovementDirection.IsNearlyZero(ToleranceValue))// if input has been added
	{
		//UKismetMathLibrary::Normalize();
		//---> in c++, normalize returns 
		WantedMovementDirection=WantedMovementDirection.GetSafeNormal();
		Velocity+=WantedMovementDirection*Acceleration*DeltaTime;
	}
	else// if no input is added
	{
		/*DecelerationVelocity=-Velocity.GetSafeNormal()*Deceleration*DeltaTime;// add counter velocity(decel_velocity to make it zero vector)
		
		if (!Velocity.IsNearlyZero())//if there is no input but it's still moving
		{
			Velocity+=DecelerationVelocity;
		}
		else// no movement, no input
		{
			Velocity={0,0,0};//or zero vector
			bIsMoving=false;//temp
			//Velocity=FVector::ZeroVector;
			//return; --> no need
		}*/

		/*Velocity=FMath::VInterpTo(Velocity,{0,0,0},DeltaTime,Deceleration);// this will prevent over subtraction
		if (Velocity.IsNearlyZero())
		{
			Velocity=FVector(0,0,0);
		}*/
		//-> this method does not use deceleration value but just use interp speed value


		//--->version 3, keep the range and save deceleration velocity
		if (!Velocity.IsNearlyZero(ToleranceValue))
		{
			float Speed=Velocity.Length();// get current speed
			float DecelerationAmount=Deceleration*DeltaTime;// get deceleration amount per delta time

			DecelerationVelocity=Velocity.GetSafeNormal()*DecelerationAmount;//calculate the Deceleration velocity here

			if (Speed<=DecelerationAmount)// decleration is equal or higher than the current speed--> over subttract
			{
				Velocity={0,0,0};
				DecelerationVelocity={0,0,0};
			}
			else
			{
				Velocity-=DecelerationVelocity;
			}
		}
		else
		{
			Velocity={0,0,0};// if its nearly zero--> zero!
		}
	}
	//=== Clamp Final Velocity ====//
	Velocity=Velocity.GetClampedToMaxSize(Current_MaxSpeed);// this would be also because of the air resistance.... change it?

	//=== Apply Movement to Base component ===//
	FVector Movement_per_Delta= Velocity*DeltaTime;

	UpdateCollision(Movement_per_Delta);
	//Move component per tick
	//Updated_Base_Comp->MoveComponent(Movement_per_Delta, Updated_Base_Comp->GetComponentRotation(),true,&HitResult);
	//sweep will get detect hit and hit result will store the information

	bIsMoving=!Velocity.IsNearlyZero(ToleranceValue);// update the movement status
}

void UDroneMovementComponent::StopMovementImmediately()
{
	Velocity={0.0f,0.0f,0.0f};
}

bool UDroneMovementComponent::DidRotateTowardsDirection(FVector& Direction)
{
	if (!Updated_Base_Comp) return false;// no comp to rotate
	if (Direction.IsNearlyZero(ToleranceValue))return true;// already facing

	float Dot=FVector::DotProduct(Updated_Base_Comp->GetForwardVector(), Direction);
	//return FMath::IsNearlyZero(Dot, ToleranceValue);
	return Dot>=1.0-ToleranceValue;
	
	//eturn FMath::IsNearlyZero((UKismetMathLibrary::Dot_VectorVector(Velocity.GetSafeNormal(), Direction)),ToleranceValue);
	//get resemblance by dot product(in c++ its Dot_VectorVector), and check if rotation is done or not
}

void UDroneMovementComponent::RotateTowardMovementDirection(FVector& Direction, float DeltaTime)
{
	if (!Updated_Base_Comp) return;
	if (Direction.IsNearlyZero(ToleranceValue)) return;
	
	CurrentRotation=Updated_Base_Comp->GetComponentRotation();
	
	FRotator TargetRotation=Direction.Rotation();//convert to rotator
	FRotator NewRotation=FMath::RInterpTo(CurrentRotation,  TargetRotation, DeltaTime, RotationSpeed);

	Updated_Base_Comp->SetWorldRotation(NewRotation);
}

void UDroneMovementComponent::RotateTowardMovementDirection_DroneMovement(FVector& Direction, float DeltaTime)
{
	/*if (!Updated_Base_Comp) return;
	if (Direction.IsNearlyZero(ToleranceValue)) return;

	// Current rotation
	FRotator CurrentRot = Updated_Base_Comp->GetComponentRotation();
    
	// flatten the movement based on the gravity direction(like how i did with the collision with surface normal)
	FVector HorizontalDir = FVector::VectorPlaneProject(Direction, -GravityDirection);
	if (HorizontalDir.IsNearlyZero(ToleranceValue))
	{
		HorizontalDir = ForwardVector; // fallback
	}
	HorizontalDir.Normalize();

	// == Local pitch tilt ==//
	float ForwardSpeed = FVector::DotProduct(HorizontalDir, ForwardVector);
	float TargetPitch = FMath::Clamp(-ForwardSpeed * MaxTiltRotation, -MaxTiltRotation, MaxTiltRotation);// set min and max

	//== local roll tilt ==//
	float RightSpeed = FVector::DotProduct(HorizontalDir, RightVector);
	float TargetRoll = FMath::Clamp(RightSpeed * MaxTiltRotation, -MaxTiltRotation,MaxTiltRotation);

	//  get horizontal vector based on the gravity direction ==//
	FQuat GravityAlignedQuat = FRotationMatrix::MakeFromXZ(HorizontalDir, -GravityDirection).ToQuat();
	FRotator TargetRot = GravityAlignedQuat.Rotator();
	TargetRot.Pitch = TargetPitch;
	TargetRot.Roll  = TargetRoll;

	// Interpolate smoothly
	FRotator NewRotation = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);

	Updated_Base_Comp->SetWorldRotation(NewRotation);*/// version 1
	
        if (!Updated_Base_Comp) return;
        if (Direction.IsNearlyZero(ToleranceValue)) return;
    
        // Current rotation
        FRotator CurrentRot = Updated_Base_Comp->GetComponentRotation();
    
        // Project movement on plane perpendicular to gravity
        FVector HorizontalDir = FVector::VectorPlaneProject(Direction, -GravityDirection);
        if (HorizontalDir.IsNearlyZero(ToleranceValue))
        {
            HorizontalDir = Updated_Base_Comp->GetForwardVector(); // fallback
        }
        HorizontalDir.Normalize();
    
        // Build rotation aligned with horizontal movement and gravity
        FQuat GravityAlignedQuat = FRotationMatrix::MakeFromXZ(HorizontalDir, -GravityDirection).ToQuat();
        FRotator TargetRot = GravityAlignedQuat.Rotator();
    
        // Smoothly interpolate rotation
        FRotator NewRotation = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);
    
        Updated_Base_Comp->SetWorldRotation(NewRotation);
}



void UDroneMovementComponent::RotateTowardWorldLocation(FVector& W_Location, float DeltaTime)
{
	/*if (!Updated_Base_Comp) return;
	if (!Owner) return;

	FVector DirectionToTarget=(W_Location-(Owner->GetActorLocation())).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero(ToleranceValue)) return;

	CurrentRotation=Updated_Base_Comp->GetComponentRotation();
	FRotator TargetRotation=DirectionToTarget.Rotation();

	FRotator DeltaRotation=(TargetRotation-CurrentRotation).GetNormalized();
	
	float MaxStep=RotationSpeed*DeltaTime;
	DeltaRotation.Yaw=FMath::Clamp(DeltaRotation.Yaw,-MaxStep,MaxStep);
	DeltaRotation.Pitch=FMath::Clamp(DeltaRotation.Pitch,-MaxStep,MaxStep);
	DeltaRotation.Roll=0.0f;// no for roll

	FRotator NewRotation=CurrentRotation+DeltaRotation;
	Updated_Base_Comp->SetWorldRotation(NewRotation);*/

	if (!Updated_Base_Comp)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("Invalid Updated_Base_Comp"));
		return;
	}
	AActor*MyOwner=GetOwner();
	if (!MyOwner)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("Invalid Owner"));
		return;
	}

	// Direction to target
	FVector DirectionToTarget = (W_Location - MyOwner->GetActorLocation()).GetSafeNormal();
	if (DirectionToTarget.IsNearlyZero(ToleranceValue))
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("Rotation value too small"));
		return;
	}

	// Current & target rotation
	FRotator CurrentRot = Updated_Base_Comp->GetComponentRotation();
	FRotator TargetRot  = DirectionToTarget.Rotation();

	// Smooth interpolation
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);

	// Kill roll (keep level)
	NewRot.Roll = 0.0f;

	Updated_Base_Comp->SetWorldRotation(NewRot);
	UE_LOG(LockOnTarget_Log,Warning,TEXT("RotateTowardWorldLocation"));

	
}

void UDroneMovementComponent::UpdateCollision(FVector& DeltaMovement)
{
	if (!Updated_Base_Comp)
	{
		UE_LOG(MoveComp_Log, Error, TEXT("Error, Invalid Updated Comp detected"))
		return;
	}

	Updated_Base_Comp->MoveComponent(DeltaMovement,Updated_Base_Comp->GetComponentRotation(),true,&HitResult);

	if (HitResult.IsValidBlockingHit())//hit detected
	{
		FVector MovementDirection=DeltaMovement.GetSafeNormal();// get direction to compare with the surface normal
		FVector HitSurfaceNormal=HitResult.Normal;

		float Dot=FVector::DotProduct(MovementDirection,HitSurfaceNormal);

		float TempAcceptalbeDotRange=0.2;//TODO--> replace this temp range
		if (Dot<TempAcceptalbeDotRange)// move towards the
		{
			Velocity={0,0,0};
			DecelerationVelocity={0,0,0};

			UE_LOG(MoveComp_Log, Error, TEXT("Drone Collided with object"));
		}
		else
		{
			FVector SlideVector=FVector::VectorPlaneProject(DeltaMovement,HitSurfaceNormal);//flatten the movement to surface
			Updated_Base_Comp->MoveComponent(SlideVector,Updated_Base_Comp->GetComponentRotation(),true,&HitResult);//move along the new slide direction
			Velocity=FVector::VectorPlaneProject(Velocity,HitSurfaceNormal);//set velocity based on surface
		}

		//====== Update Ground Status after hit interaction =====//
		
		if (FVector::DotProduct(HitResult.Normal,GravityDirection)>0.07)
		{
			bIsinAir=false;
			Velocity.Z=0;
		}
		else
		{
			bIsinAir=true;
		}
	}
	else
	{
		bIsinAir=true;
	}
		/*// the updated component got throw the hit surface. dive back 
		//How can i calculate the deflection direciton? ---> hit hit surface normal--> compare with the movement direction--> calculate counter impact

		//--> Slide on a surface?
		FVector Deflection=FVector::VectorPlaneProject(DeltaMovement,HitResult.Normal);
		//https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Math/TVector/VectorPlaneProject
		
		// basically it projects the vector on a plane(normal) and make vector parallel to plane
		//simply saying---> it flattens the vector on a plane(normal)
		Updated_Base_Comp->MoveComponent(Deflection,Updated_Base_Comp->GetComponentRotation(),true,&HitResult);
		Velocity=FVector::VectorPlaneProject(DeltaMovement,HitResult.Normal);// slide on a surface
		
		//StopMovementImmediately();// this is stopping the movement immedietly. how can i change it?
		// bounde curve base on the movement direciton, surface normal,

		if (FVector::DotProduct(HitResult.Normal,GravityDirection)>0.7f)
		{
			bIsinAir=false;
			Velocity.Z=0.f;// change this to alighned with gravity direction later
		}
		else
		{
			bIsinAir=true;
		}
	}
	else
	{
		bIsinAir=true;
	}*///--> organise this mess
	
}

void UDroneMovementComponent::ApplyGravity(float DeltaTime)
{
	if (!bIsinAir) return;

	// Apply acceleration due to gravity
	Velocity += GravityDirection * GravitalForce * DeltaTime;

	// Apply air resistance
	if (!Velocity.IsNearlyZero())
	{
		FVector DragMovement = Velocity.GetSafeNormal() * AirResistance * DeltaTime;
		if (DragMovement.SizeSquared() > Velocity.SizeSquared())
		{
			Velocity = FVector::ZeroVector;
		}
		else
		{
			Velocity -= DragMovement;
		}
	}
}


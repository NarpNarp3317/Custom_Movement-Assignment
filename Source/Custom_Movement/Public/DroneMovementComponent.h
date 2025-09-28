// Fill out your copyright notice in the Description page of Project Settings.
/*
 Making Movement Component in my likening
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "CustomMovementMode.h"
//#include "IMC_Bundle_Str.h"
#include "DroneMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOM_MOVEMENT_API UDroneMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDroneMovementComponent();

protected:
	// the mode of this movement component is Drone movement
	ECustomMovementMode MovementMode= ECustomMovementMode::Drone_Move;
	
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	//=== Define not working, so put it in here.
	float ToleranceValue=0.01;

	//==== IMC for Movement ===// ---> lets seperate them and make a wrapper that gathers movecomp and movement imc
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IMC")
	//FIMC_Bundle Movement_IMC_B;

	//==== Owner ====//
	UPROPERTY()// for garbage collector
	APawn* Owner;// the owner of this component //-->cast if it's needed
	UPROPERTY()
	//UCapsuleComponent * Updated_CapsuleComp;// the owner of the capsule is actor, but in here, it borrows it and manipulate it to do movement functions
    UPrimitiveComponent* Updated_Base_Comp;// the base of components with(transform, rendering, collision)
	
	//==== Input ====//
	//FVector2D MovementInput;//XY input
	UPROPERTY(BlueprintReadOnly, Category="Movement / input")
	FVector MovementInput;//XYZ at once

	UPROPERTY(BlueprintReadOnly, Category="Rotator / input")
	FVector RotatorInput;//XYZ at once
	
	bool bHasMovementInput;//check if movement has been inputed//for checkin movement without input(unintentional)

	
	//==== Debug Indicator ====//
	bool bIsDebugOn;

	//=== Lock on movement ====//
	bool bIsLockedOn;//to check if looking direction(forward direction) is locked or not.
	AActor* TargetedActor;
	FVector LockedWorldLocation;// the world location of the targeted actor or anything(not as the actor but the location)
	//---> should this be in the movement compoenent? looking--> is this movement or world interaction?

	//==== Confirmations ====//
	bool bIsBoosting;//sprint input
	bool bIsMoving;// checking if the movement velocity is less than acceotable value?
	bool bIsinAir;// is main collision component contatcting with surface?
	bool bIsGravityOn;// for checking if the component is influenced by gravity or not
	
	//=== Gravity / Mid Air ===//

	FVector GravityDirection;
	UPROPERTY(EditAnywhere, Category="Movement / Gravity")
	float GravitalForce=9.8;// default of 9.8// basically the gravity accelertation value
	float Mass;
	FVector MassCenter;// for balancing


	
	//Todo--> which method should?????
	float AirResistance;// to clamp the falling speed--> air_resist low--> fall speed high
	//required factors --> speed, surface, shape, air density
	
	float MaxFallSpeed;// or could clamp the fall speed instead of calculating the fall deceleration
	// could imitate the fall deceleration using params
	
	//== Engine Related ==//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement / Engine")
	float LiftForce;// this is for lifting up the body while starting the engine
	
	
	

	//==== Collision ====//
	bool bDidHit;
	FHitResult HitResult;



	
	//=== Basic Movement ===//

	FVector CurrentLocation;
	FRotator CurrentRotation;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Basic Movement")
	float MaxTiltRotation;// this is for movement tilting


	
	// set defualt value in here?
	UPROPERTY(EditAnywhere, Category="Basic Movement")
	float MaxBoostSpeed;
	UPROPERTY(EditAnywhere, Category="Basic Movement")
	float MaxFlySpeed;

	UPROPERTY(EditAnywhere, Category="Basic Movement")
	float Boost_Multiplier;// this will increase the current maxspeed

//--> the speed for movement mode will be stored in character, but in here, it only store the max Speed

	float Current_MaxSpeed;// set max speed in here and switch it if the movement type changes
	
	//----> These will be defined by the owner of the component

	
	FVector Velocity;// for basic movement?
	FVector DecelerationVelocity;// for dragg --> use it for animation
	
	//=== Accel / Decel ===//
	UPROPERTY(EditAnywhere, Category="Basic Movement")
	float Acceleration;
	UPROPERTY(EditAnywhere, Category="Basic Movement")
	float Deceleration;
	UPROPERTY(EditAnywhere, Category="Basic Movement")
	float RotationSpeed = 5.0f;// for base rotation  inpterp speed
	
	float MovementOrientedRotationSpeed;// rotation interp
	bool bIsMovementOrientedOrienting;// checking if the character movement is based on look or input

	

	FVector ForwardVector;// these will be calculated based on the current camera's rotation value
	FVector RightVector;
	FVector UpVector;

	//==== Ground interaction ===//
	
	bool bIsInAir;//checking if the character is on ground or in mid air
	FVector Contacting_SurfaceNormal;// compare it with gravity direction and calculate the leaning amount


	

public:	
	
	void SetOwner_in_Move_Comp(APawn* newOwner);
	void SetUpdatedComp(UCapsuleComponent * Updated_Comp);

	//UFUNCTION(BlueprintPure`, Category="IMC")
	//FIMC_Bundle GetMovement_IMC_B();
	
	UFUNCTION(BlueprintPure, Category="Movement Mode")
	ECustomMovementMode GetMovementMode()const;

	
	//===== Confirmation =====//

	UFUNCTION(BlueprintPure, Category="Confirmation")
	bool IsInAir() const;
	UFUNCTION(BlueprintPure, Category="Confirmation")
	bool IsMoving() const;
	UFUNCTION(BlueprintPure, Category="Confirmation")
	bool IsBoosting() const;
	UFUNCTION(BlueprintPure, Category="Confirmation")
	bool IsLockedOn() const;// to manage the rotation setting

	void SetbIsLockedOn(bool newIsLockedOn);
	
	UFUNCTION(BlueprintPure, Category="Confirmation")
	ECustomMovementMode GetMovementType() const;

	

	void UpdateMovementBoolValues();//temp

	


	

	//=== Gravity ===//
	void SetGravityDirection(FVector& Direction);
	FVector GetGravityDirection();
	void SetGravitalForce(float G_Force);
	float GetGravitalForce();

	void ApplyGravityOrNot(bool YesOrNo);


	//=== Surface Interaction ===//

	float GroundFriction;// how slippery the surface is---> this will could be the deceleration on the ground
	
	
	void Set_Forward_Right_Up_Vector_BasedOnCamera();// get current camera and set forward, right, up vector

	UFUNCTION(BlueprintPure, Category="Movement")
	FVector GetVelocity();
	UFUNCTION(BlueprintPure, Category="Movement")
	FVector Get_Decel_Velocity();
	UFUNCTION(BlueprintPure, Category="Movement")
	FVector GetForwardVector();
	UFUNCTION(BlueprintPure, Category="Movement")
	FVector GetRightVector();
	UFUNCTION(BlueprintPure, Category="Movement")
	FVector GetUpVector();// by calculating with forward and right or by just saving the up vector?

	//=== Movement ===//
	void MoveForward(float Value, float DeltaTime);
	void MoveRight(float Value, float DeltaTime);
	void MoveUp(float Value, float DeltaTime);
	void RotateYaw(float Value, float DeltaTime);

	// this is for lifting up the updated comp when the engine is on
	void LiftBody(float DeltaTime);
	
	void BalanceMovement(float DeltaTime);// this function will recover its rotation after the directional rotation or movement tilt

	void StartBoost(bool value);
	void StopBoost(bool value);

	void UpdateMovementInput(float DeltaTime);
	
	void StopMovementImmediately();//sets velocity to {0,0,0}
	bool DidRotateTowardsDirection(FVector& Direction);// it will rotate till the rotator matches(unlike character movement)
	
	//it will return true if the rotation is completed
	void RotateTowardMovementDirection(FVector& Direction, float DeltaTime);
	//--> using this will make drone to face up when moving up. that is not how drone moves like
	
	void RotateTowardMovementDirection_DroneMovement(FVector& Direction,float DeltaTime);// this will tilt and rotate when movement is forward, and right but not for up

	void RotateTowardWorldLocation(FVector& W_Location, float DeltaTime);

	//==== Collision =====//

	void UpdateCollision(FVector& DeltaMovement);// this will update the hit result while updating the movment of the updated comp


	

	void ApplyGravity(float DeltaTime);// this will add delta movement value to velicty  per tick towards the gravity direction
};

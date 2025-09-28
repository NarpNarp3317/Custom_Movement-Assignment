// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DroneMovementComponent.h"
//#include "ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CustomMovementMode.h"
#include "CustomCharacter.generated.h"

struct FInputActionValue;
class AMyPlayerController;
class UTargetLockOnComponent;


UCLASS()
class CUSTOM_MOVEMENT_API ACustomCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACustomCharacter();
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",meta = (AllowPrivateAccess = "true"))
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")// to use movement comp functions in editor
	// is BleprintReadWrite hides the detail?
	UDroneMovementComponent* Current_Movement_Comp;
	
	// this is for lock on component!!!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Components")
	UTargetLockOnComponent* LockOn_Comp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* Skeletal_Comp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCapsuleComponent* Main_Capsule_Comp;// this is the root component

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	AMyPlayerController* PlayerController_Comp;
	
	//==== Maybe Replace with Gameplay Camera System ====//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm_Comp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* Camera_Comp;
	
	// this is for making long bodied character with more than 2 limbs. 
	/*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TArray<UCapsuleComponent*> ExtraCapsule_Comps;// store extra capsule components
	*///-->make seperate movement component for quadraped character. for now, not necessary 
	
	// this is only for storing names for action binding, preventing typo.
	//TODO	
	//******************************************************************************************************************
	//				  !!!!!!!!!!!!!!!! MUST PUT BINDABLE FUNCTION NAME IN HERE!!!!!!!!!!!!!!!!
	// or not
	//******************************************************************************************************************
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Function Names")
	TArray<FName> Function_Names;
	
protected:

	//=== Components ===//

	float CapsuleHeight;// 2*capsule half height,
	float CapsuleRadius;// Radius of the Capsule

	UPROPERTY(BlueprintReadWrite, Category="Camera")
	float CameraRotationSpeed;

	//===== Gravity Enable/disable ====//

	bool bIsEngineOn;//on off for engine
	bool bIsProgressionRising;// basically, it is for is progression rising or falling


	float ProgressionDecomposeSpeed;// this is the speed of how fast the progress will go down after release befor it reaches the time
	float ProgressionSpeed;
	float EngineProgression;// 
	UPROPERTY(EditAnywhere, Category="Movement / Gravity")
	float EngineStartTimer;//--> this is the threshold to start the engine
	//--> this will slowly lift the drone body and when it reaches the time, it will turn of the gravity and move freely without falling
	//engine will turned on and off---> off --> just one press, when it collide with the ground--> start over
	

	//==== Member Function ====//
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	void CustomCharacter_SetOwner();// set the owner in the movement component

	UFUNCTION(BlueprintPure)
	UDroneMovementComponent* GetCustomMovementComponent() const;

	UFUNCTION(BlueprintPure)
	ECustomMovementMode GetCurrentMovementMode() const;

	void GetPossessedBy(AController* NewController); //this is done by the game mode

	//void Set_Forward_Right_Up_Vector_BasedOnCamera();// get current camera and set forward, right, up vector
	//--> this is for the movement component i guess

	UFUNCTION()// expose function to reflection system to bind with enhanced input system?
	void MoveForward(const FInputActionValue& value);
	UFUNCTION()
	void MoveRight(const FInputActionValue& value) const;
	UFUNCTION()
	void MoveUp(const FInputActionValue& value);
	/*UFUNCTION()
	void RotateYaw(const FInputActionValue& value);*///--> rotation is based on movment and lock on condition
	UFUNCTION()
	void LockOnOff(const FInputActionValue& value);

	UFUNCTION()
	void SwitchTarget(const FInputActionValue& value) ;
	
	UFUNCTION()
	void ToggleEngine(const FInputActionValue& value) ;// bind this with input action

	bool UpdateEngineProgression(float DeltaTime);// this will be setted in the tick
	void StopEngine();// this will enable the gravity and make drone fall
	void StartEngine();
	
	UFUNCTION()
	void StartBoost(const FInputActionValue& value) const;
	UFUNCTION()
	void StopBoost(const FInputActionValue& value) const;
	UFUNCTION()
	void Look(const FInputActionValue& value);

	//===Camera Related =====//---> temp//TODO:  make a camera manager system to controll camera so that pawn class can be just a puppet

	UFUNCTION()
	void AddCameraYaw(float Value, float Deltatime);
	UFUNCTION()
	void AddCameraPitch(float Value, float Deltatime);
	
	/*UFUNCTION()
	void AskController_To_BindActions();//Send signal to controller *///---> no need!, controller has the function for that
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacter.h"
#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TargetLockOnComponent.h"
#include "Custom_LogCategory.h"



// Sets default values
ACustomCharacter::ACustomCharacter():
	bIsEngineOn(false)

{
	UE_LOG(CustomCharacter_Log,Display, TEXT("CustomCharacter Contruction started"));
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//===== create comps ====//

	Main_Capsule_Comp=CreateDefaultSubobject<UCapsuleComponent>(TEXT("Main_Capsule"));
	SetRootComponent(Main_Capsule_Comp);// set main comp as root comp
	
	Skeletal_Comp=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal_Mesh"));
	Skeletal_Comp->SetupAttachment(Main_Capsule_Comp);

	SpringArm_Comp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArm_Comp->SetupAttachment(RootComponent);
	SpringArm_Comp->TargetArmLength = 300.0f; // distance to character
	SpringArm_Comp->bUsePawnControlRotation = false; // rotate boom based on controller, not based on pawn rotation
	SpringArm_Comp->SetUsingAbsoluteRotation(true);
	Camera_Comp = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera_Comp->SetupAttachment(SpringArm_Comp, USpringArmComponent::SocketName);
	Camera_Comp->bUsePawnControlRotation = false; // camera does not rotate relative to boom


	//Set Movement Component
	Current_Movement_Comp=CreateDefaultSubobject<UDroneMovementComponent>(TEXT("Movement_Comp"));
	Current_Movement_Comp->SetUpdatedComp(Main_Capsule_Comp);// tell what component will be used for the updatedComp

	//Set LockOnTarget Component
	LockOn_Comp=CreateDefaultSubobject<UTargetLockOnComponent>(TEXT("TargetLockOn"));
	
	//==== Setting Binding Function Names =====//
	Function_Names={"MoveForward","MoveRight","MoveUp","RotateYaw","StartJump","StopJump","StartSprint","StopSprint"};
	// .... Is this Necessary?


	CameraRotationSpeed=50.0f;//temp
}

void ACustomCharacter::CustomCharacter_SetOwner()
{
	if (!Current_Movement_Comp)
	{
		UE_LOG(CustomCharacter_Log,Error, TEXT("Invalid Movement Comp"))
		return;
	}

	Current_Movement_Comp->SetOwner_in_Move_Comp(this);// set self as a owner of the movement comp

}

UDroneMovementComponent* ACustomCharacter::GetCustomMovementComponent() const
{
	return Current_Movement_Comp;
}

ECustomMovementMode ACustomCharacter::GetCurrentMovementMode() const// return current movement mode
{
	return Current_Movement_Comp->GetMovementMode();
}

// Called when the game starts or when spawned
void ACustomCharacter::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(CustomCharacter_Log,Display,TEXT("BeginPlay"));

	//Delegate update Binding
	
	/*if (LockOn_Comp)//if lock on comp is valid
	{
		LockOn_Comp->OnLockedChanged.AddLambda([this](bool bIsLockedOn)
		{
			//this->bIsLockedOn=bIsLockedOn;
			UE_LOG(LockOnTarget_Log,Log,TEXT("Delegate update in Character, bIsLockedOn=%d"),bIsLockedOn);
		});
	}*///--> this should be in the movement component, not in the character
    		
	
}

// Called every frame
void ACustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(!Current_Movement_Comp)
	{
		return;
	}
	
	if (UpdateEngineProgression(DeltaTime)) // engine update)
	{
		Current_Movement_Comp->LiftBody(DeltaTime);
	}

	
	//Current_Movement_Comp->SetbIsLockedOn(LockOn_Comp->IsLockedOnTarget());// temp //TODO--> do this is somewhere else
	// dont set it for every tick, just make
}

// Called to bind functionality to input
void ACustomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UE_LOG(IMC_Log,Log, TEXT("SetupPlayerInputComponent In Custom Character Activated"));
	//=== Start Valid Check ===//
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent=Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid Enhanced Input Component in CustomCharacter"));
		return;
	}
	
	/*Controller //---> Pawn class already has AController as Member Variable// also has previous one too. fuck yeah
	AController* CatchedController=GetController();*/
	PlayerController_Comp=Cast<AMyPlayerController>(Controller);
	if (PlayerController_Comp)
	{
		UE_LOG(IMC_Log,Log,TEXT("CustomController Casting Successful in CustomCharacter"));
		UE_LOG(IMC_Log,Log,TEXT("Calling Controller to Start IMC Process"));
		//CustomController->"Here, put a function to set a 
	}
	else
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid Controller in CustomCharacter"));
		return;
	}
	//===Checking done ===//
	
	
	//TODO--> Replace the manual action binding with new function
	/* 
	if (PlayerController_Comp->MoveForwardAction!=nullptr)
	{
		EnhancedInputComponent->BindAction(PlayerController_Comp->MoveForwardAction,ETriggerEvent::Triggered,this,&ACustomCharacter::MoveForward);
	}
	if (PlayerController_Comp->MoveRightAction!=nullptr)
	{
		EnhancedInputComponent->BindAction(NewController->MoveRightAction,ETriggerEvent::Triggered,this,&ACustomCharacter::MoveRight);
	}
	if (NewController->MoveUpAction!=nullptr)
	{
		EnhancedInputComponent->BindAction(NewController->MoveUpAction,ETriggerEvent::Triggered,this,&ACustomCharacter::MoveUp);
	}
	if (NewController->JumpAction!=nullptr)
	{
		EnhancedInputComponent->BindAction(NewController->JumpAction,ETriggerEvent::Triggered,this,&ACustomCharacter::StartJump);
		EnhancedInputComponent->BindAction(NewController->JumpAction,ETriggerEvent::Completed,this,&ACustomCharacter::StopJump);
	}
	if (NewController->SprintAction!=nullptr)
	{
		EnhancedInputComponent->BindAction(NewController->SprintAction,ETriggerEvent::Triggered,this,&ACustomCharacter::StartSprint);
		EnhancedInputComponent->BindAction(NewController->SprintAction,ETriggerEvent::Completed,this,&ACustomCharacter::StopSprint);
	}
	if (NewController->LookAction!=nullptr)
	{
		EnhancedInputComponent->BindAction(NewController->LookAction,ETriggerEvent::Triggered,this,&ACustomCharacter::Look);
	}
	*/
	
}

void ACustomCharacter::GetPossessedBy(AController* NewController)
{
	UE_LOG(CustomCharacter_Log,Display,TEXT("Got Possessed By Controller"));
	Super::PossessedBy(NewController);
}

void ACustomCharacter::MoveForward(const FInputActionValue& Value) //!!! to bind with Enhanced input, the parameter must be () or (value), no more
{
	if (!Current_Movement_Comp) return;

	const float MovementValue= Value.Get<float>();// make sure its float?
	float DeltaTime= GetWorld()->GetDeltaSeconds();
	
	Current_Movement_Comp->MoveForward(MovementValue,DeltaTime);// let movement compoentn move the actor
}

void ACustomCharacter::MoveRight(const FInputActionValue& Value) const
{
	if (!Current_Movement_Comp) return;

	const float MovementValue= Value.Get<float>();// make sure its float?
	float DeltaTime= GetWorld()->GetDeltaSeconds();
	
	Current_Movement_Comp->MoveRight(MovementValue,DeltaTime);// let movement compoentn move the actor
}

void ACustomCharacter::MoveUp(const FInputActionValue& Value)
{
	if (!Current_Movement_Comp) return;

	const float MovementValue= Value.Get<float>();// make sure its float?
	float DeltaTime= GetWorld()->GetDeltaSeconds();
	
	Current_Movement_Comp->MoveUp(MovementValue,DeltaTime);// let movement compoentn move the actor
}

/*void ACustomCharacter::RotateYaw(const FInputActionValue& Value)
{
	if (!Current_Movement_Comp) return;
	const float MovementValue= Value.Get<float>();
	float DeltaTime= GetWorld()->GetDeltaSeconds();
	Current_Movement_Comp->RotateYaw(MovementValue,DeltaTime);
}*/ // the controll is bit boring. make it rotate towards the movement direction when it is not locked 

void ACustomCharacter::LockOnOff(const FInputActionValue& value)
{
	if (!LockOn_Comp)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("LockOn Component invalid from lock onoff function"));
		return;
	}
	LockOn_Comp->ToggleTargetting();

	//== Camera Setting ===//
	if (LockOn_Comp->IsLockedOnTarget())// if it is locked on, change the springarm setting
	{
		SpringArm_Comp->SetUsingAbsoluteRotation(false);
		SpringArm_Comp->SetWorldRotation(GetActorRotation());
	}
	else// else, use the default setting
	{
		SpringArm_Comp->SetUsingAbsoluteRotation(true);
		SpringArm_Comp->SetWorldRotation(GetActorRotation());
	}
	// This is quite too limited way to lock the rotation of the spring arm. dont use spring arm anyway and use custom camera system
}

void ACustomCharacter::SwitchTarget(const FInputActionValue& value)
{
	if (!LockOn_Comp)
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("LockOn Component invalid from switch target function"));
		return;
	}

	if (!LockOn_Comp->IsLockedOnTarget()) return;

	const FVector2D Inputdirection=value.Get<FVector2D>();
	
	LockOn_Comp->SwitchTarget(Inputdirection);
	
}

/*void ACustomCharacter::AskController_To_BindActions()// activate this when the setup is all done. this is the function only for the player controller owned pawn
{
	//PlayerController_Comp->
	//make fucntion for add mapping context and bind inputactions
}*/

void ACustomCharacter::ToggleEngine(const FInputActionValue& value)
{
	if (!Current_Movement_Comp) return;
	const bool InputValue= value.Get<bool>();

	
	if (bIsEngineOn)
	{
		if (InputValue)
		{
			UE_LOG(MoveComp_Log,Warning,TEXT("Engine Off"))
			StopEngine();
		}
	}
	else
	{
		bIsProgressionRising=InputValue;// set it as start of rising progression
	}
	

}

bool ACustomCharacter::UpdateEngineProgression(float DeltaTime)
{
	if (bIsEngineOn)
	{
		return false;// engine is already running.
	}

	EngineProgression+= bIsProgressionRising? ProgressionSpeed*DeltaTime : -(ProgressionDecomposeSpeed*DeltaTime);
	EngineProgression=FMath::Clamp(EngineProgression,0,EngineStartTimer);// set min and max here
	
	if (EngineProgression>=EngineStartTimer)
	{
		StartEngine();
		return true;
	}
	return false;
}

void ACustomCharacter::StopEngine()
{
	if (!bIsEngineOn)return;

	bIsEngineOn=false;
	
	EngineProgression=0;//resset to 0

	Current_Movement_Comp->ApplyGravityOrNot(true);// enable gravity
}

void ACustomCharacter::StartEngine()
{
	if (bIsEngineOn) return;

	bIsEngineOn=true;

	Current_Movement_Comp->ApplyGravityOrNot(false);
	// the valid check for move comp was done in the toggle, so, not necessary in here?
}


void ACustomCharacter::StartBoost(const FInputActionValue& Value) const
{
	if (!Current_Movement_Comp) return;
	const bool MovementValue= Value.Get<bool>();

	if (MovementValue) Current_Movement_Comp->StartBoost(MovementValue);
}

void ACustomCharacter::StopBoost(const FInputActionValue& Value) const
{
	if (!Current_Movement_Comp) return;
	const bool MovementValue= Value.Get<bool>();

	if (!MovementValue) Current_Movement_Comp->StopBoost(MovementValue);
}

void ACustomCharacter::Look(const FInputActionValue& Value)
{
	if (!Current_Movement_Comp) return;
	if (!LockOn_Comp) 
	{
		UE_LOG(LockOnTarget_Log,Error,TEXT("LockOn Component invalid from look function"));
	return;
	}
	if (LockOn_Comp->IsLockedOnTarget())return;// do not rotate the camera when the target is locked
	const FVector2D MovementValue= Value.Get<FVector2D>();

	//Movement_Comp->Look --> this is not movement related. make it work in here
	float DeltaTime= GetWorld()->GetDeltaSeconds();
	//TODO--> Replace add controller input function with custom functions
	AddCameraYaw(MovementValue.X,DeltaTime);
	AddCameraPitch(MovementValue.Y,DeltaTime);
}

void ACustomCharacter::AddCameraYaw(float Value, float DeltaTime)
{
	if (FMath::IsNearlyZero(Value)) return;

	FRotator CurrentRotaton= SpringArm_Comp->GetComponentRotation();
	FRotator DeltaRotaton(0,Value*CameraRotationSpeed*DeltaTime,0);
	SpringArm_Comp->SetWorldRotation(CurrentRotaton+DeltaRotaton);
}

void ACustomCharacter::AddCameraPitch(float Value, float DeltaTime)
{
	if (FMath::IsNearlyZero(Value)) return;

	FRotator CurrentRotaton= SpringArm_Comp->GetComponentRotation();
	float PitchAngle=80;//temp. TODO : Make Better way to clamp the pitch value
	float NewPitch=FMath::Clamp(CurrentRotaton.Pitch+Value*CameraRotationSpeed*DeltaTime,-PitchAngle,PitchAngle);
	CurrentRotaton.Pitch=NewPitch;
	SpringArm_Comp->SetWorldRotation(CurrentRotaton);
}



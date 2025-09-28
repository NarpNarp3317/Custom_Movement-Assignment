#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputDelegate.h"// use delegate to use it as function ptr
#include "CustomCharacter.h"
#include "EnhancedInput/Public/InputTriggers.h"// for ETriggerEvent
//https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/EnhancedInput/ETriggerEvent?application_version=5.6
#include "InputActionData.generated.h"
/*
this stores every input actions to link the action binding with custom character and controller
 */

class UInputAction;

USTRUCT(BlueprintType)
struct FInputActionData
{
	GENERATED_BODY()
	
	FInputActionData(FName Name,UInputAction* IA, ETriggerEvent T_Event):
	FunctionName(Name),InputAction(IA),TriggerEvent(T_Event)
	{}

	FInputActionData():
	FunctionName(NAME_None),InputAction(nullptr),TriggerEvent()
	{}

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ActionName;*///---> this is done by the map key

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName FunctionName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputAction* InputAction;// default

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETriggerEvent TriggerEvent;// this is for the case when one inputaction have more than one events(ex. jump-> start jump, stop jump)

/* //--> Nolonger needed!!!!!
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EInputAction_Enums InputAction_Enum;// enum for input type
*/
	//FinputActionDelegate BindedFunction;// this is not using the reflection system, so make it work by using enum
	//void (ACustomCharacter::*BindedFunction)(const FInputActionValue&);
	//--> no delegator but this
	
	/*UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BindedFunction;//--> this is the key of map
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey Input_Key;*///---> will be done in imc, not in here
};
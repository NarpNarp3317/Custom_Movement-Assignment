#pragma once

#include "CoreMinimal.h"
//#include "InputMappingContext.h"
//#include "CustomCharacter.h" the APawnPtr will be casted into the owner 
#include "EnhancedInputComponent.h"
#include "Modular_IMC_Type.h"
#include "IMC_Module.h"
#include "Custom_LogCategory.h"
#include "IMC_Bundle_Str.generated.h"

/*
 this is will store the input actions by 4 different types.
	1. Movement(ground movement, flying movement, zerogravity movement)
	2. Interaction( attack, world interaction)
	3. ItemUsage
	4. Menu
	//-->3,4 wont change but 1,2 will be
 */

// make a struct to store 2 versions of same imc as one
USTRUCT(BlueprintType)
struct FIMC_Bundle// f for struct
{
	GENERATED_BODY()

	FIMC_Bundle(FName Name,UObject* Function_Owner,EModular_IMC_Type Type, FModular_IMC GP,FModular_IMC PC, int32 PriorityNum)//default constructor
	:FunctionOwner(Function_Owner),Module_Type(Type),GP_IMC(GP),PC_IMC(PC),LayerPriority(PriorityNum)
	{}

	FIMC_Bundle()
	:IMCB_Name(NAME_None),FunctionOwner(nullptr),Module_Type(EModular_IMC_Type::None),GP_IMC(),PC_IMC(),LayerPriority(0)
	{}// for default constructor

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IMCB")
	FName IMCB_Name;//only for debug purpose
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IMCB")
	UObject* FunctionOwner;//mutable --> ignore const
	// for now, the owner is set by the Owner of the Struct, the Membervaliable FunctionOwner here is owner of the function, not the IMCB
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IMCB")
	EModular_IMC_Type Module_Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IMCB")
	FModular_IMC GP_IMC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IMCB")
	FModular_IMC PC_IMC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IMCB")
	int32 LayerPriority;

	//===== !!!!!FINAL BINDING IN IMCBUNDLE!!!!! =====//
	void BindActions( UEnhancedInputComponent* Input_Comp,FModular_IMC NewIMC) const
	{
		if (!FunctionOwner)
		{
			UE_LOG(IMC_Log,Error,TEXT("Invalid Owner found in Bunle's BindAction"));
			return;//invalid
		}
		if (!Input_Comp)
		{
			UE_LOG(IMC_Log,Error,TEXT("Invalid Input Component found in Bunle's BindAction"));
			return;//invalid
		}
		
		for (const FInputActionData& Data: NewIMC.InputAction_Data)
		{
			if (!Data.InputAction)
			{
				UE_LOG(IMC_Log,Error,TEXT("Invalid ActionData found in Bunle's BindAction"));
				continue;
			}

			Input_Comp->BindAction(Data.InputAction, Data.TriggerEvent, FunctionOwner, Data.FunctionName);
			UE_LOG(IMC_Log,Log,TEXT("%s is binded Successfully"),*Data.FunctionName.ToString());
			//Input_Comp->BindAction(ActionData.InputAction, ActionData.TriggerEvent, Owner, &ACustomCharacter::MoveForward);//temp--> not using declare
			//--> BindAction does not send the delegate
			
		}
	}
	void BindAllActions( UEnhancedInputComponent* Input_Comp) const
	{
		UE_LOG(IMC_Log,Log,TEXT("BindAllActions in IMC_B called"));
		BindActions(Input_Comp,GP_IMC);
		BindActions(Input_Comp,PC_IMC);
	}
};
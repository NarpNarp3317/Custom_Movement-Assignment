// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "IMC_UELOG.h"// imc for now
#include "Custom_LogCategory.h"


AMyPlayerController::AMyPlayerController():
	LocalPlayer(nullptr),

	//=== Confirmations ===//
	bIsGamepad(false),//default
	Subsystem(nullptr)
	
{
	//=== Input Actions ===//
	//SetIMCBundles(); ----> this is constructor!!! . the setup for imcb is not done yet, so no, dont do that in here

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(IMC_Log,Warning,TEXT("BeginPlay"));

	//==== Get subsystem =====//
	LocalPlayer=GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(IMC_Log,Error,TEXT("Local Player Invalid"));
		return;//check if local player is valid
	}
	UE_LOG(IMC_Log,Log,TEXT("Local Player Successfully settled"));
	
	Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(IMC_Log,Error,TEXT("Subsystem Invalid"));
		return;// check if subsystem is valid
	}
	UE_LOG(IMC_Log,Log,TEXT("Subsystem Successfully settled"));
	UE_LOG(IMC_Log,Warning,TEXT("Controller IMC Setting Completed"));
}
//===== GamePad or PC Checking =====//----------------------------------------------------------------------------------
void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)//member varaible of parent of this class
	{
		
		//this is only for checking which button is pressed. not to pawn function
		InputComponent->BindKey(EKeys::AnyKey, IE_Pressed,this, &AMyPlayerController::OnAnyInputPressed);
		// bind to any updated key, when its pressed, call OnanyInputPressed function from this
		UE_LOG(IMC_Log,Log,TEXT("Binding OnAnyInputPressed Completed"));
	}
	UE_LOG(IMC_Log,Error,TEXT("Invalid IC in SetupInputComponent for Checking GP or PC"));
}

void AMyPlayerController::OnAnyInputPressed(FKey Key)
{
	if (Key.IsGamepadKey())//check if the updated key is from pc or gamepad
	{
		bIsGamepad=true;
		UE_LOG(IMC_Log,Log,TEXT("GamePad"));
	}
	else// if (Key.IsMouseButton()||Key.IsKeyboardKey()) --> for now, default pc, gamepad
	{
		bIsGamepad=false;
		UE_LOG(IMC_Log,Log,TEXT("PC"));
		//same here
	}
	//setting is done
}
//----------------------------------------------------------------------------------------------------------------------
void AMyPlayerController::OnPossess(APawn* NewPawn)// this happens before Beginplay
{
	Super::OnPossess(NewPawn);
	UE_LOG(IMC_Log,Log,TEXT("Controller Possessed Pawn"));
	
	PossessedPawn=NewPawn;
	//set the owner to the imc bundles
	
	if (ACustomCharacter* Char=Cast<ACustomCharacter>(NewPawn))
	{
		UE_LOG(IMC_Log,Log,TEXT("Casting Pawn to ACustomCharacter Successful"));

		if (!Char)
		{
			UE_LOG(IMC_Log,Error,TEXT("casted Owner Invalid"));
		}
		UE_LOG(IMC_Log,Log,TEXT("casted Owner valid"));
		
		
		Movement_IMCBundle.FunctionOwner=Char;
		World_Interaction_IMCBundle.FunctionOwner=Char;
		UI_Interaction_IMCBundle.FunctionOwner=this;
		Menu_IMCBundle.FunctionOwner=this;

		for (FIMC_Bundle& Custom_B: Custom_IMCBundles)
		{
			Custom_B.FunctionOwner=Char;
		};
	}
	else
	{
		UE_LOG(IMC_Log,Error,TEXT("Owner Casting Failed"));
	}
}

//=== IMC Management ===//----------------------------------------------------------------------------------------------

/*void AMyPlayerController::SetIMC_Bundle(const FIMC_Bundle& IMC_B)
{
	UE_LOG(IMC_Log,Log,TEXT("SetIMC_Bundle Activated"));
	/*if (!Subsystem)return;
	if (!IMC_B.GP_IMC.IMC||!IMC_B.PC_IMC.IMC) return;#1#

	EModular_IMC_Type Type=IMC_B.Module_Type;
	FIMC_Bundle* Target_IMC_B=nullptr;// make empty ptr

	switch (Type)
	{
	case EModular_IMC_Type::Movement_IMC:
		Target_IMC_B=&Movement_IMCBundle;
		break;
	case EModular_IMC_Type::World_Interaction_IMC:
		Target_IMC_B=&World_Interaction_IMCBundle;
		break;
	case EModular_IMC_Type::UI_Interaction_IMC:
		Target_IMC_B=&UI_Interaction_IMCBundle;
		break;
	case EModular_IMC_Type::Menu_IMC:
		Target_IMC_B=&Menu_IMCBundle;
		break;
	case EModular_IMC_Type::Custom_IMC:
		for(FIMC_Bundle& Current_IMC_Bs: Custom_IMCBundles)
		{
			if (Current_IMC_Bs.Module_Type != IMC_B.Module_Type)continue;
			if (Current_IMC_Bs.GP_IMC.IMC!=IMC_B.GP_IMC.IMC) continue;;
			if (Current_IMC_Bs.PC_IMC.IMC!= IMC_B.PC_IMC.IMC) continue;

			return;//the IMC_B is already in the map. so, no need for adding
		}
		// loop is finished, but no matching icm bundle has been found--> New IMC_B
		Custom_IMCBundles.Add(IMC_B);
		break;
	default:
		UE_LOG(IMC_Log,Error,TEXT("Invalid IMC Type"));
		break;
	}
	if (!Target_IMC_B)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid Target_IMC_B"));
		return;
	}
}*/
/*
void AMyPlayerController::SetIMCMode()// this is adding mapping context based on current imc bundle and input type
{
	UE_LOG(IMC_Log,Log,TEXT("SetIMCMode Activated"));
	if (!Subsystem)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid Subsystem in Controller in SetIMCMode"));
		return;// check if subsystem is valid
	}

	
	for (FIMC_Bundle& IMC_B : Custom_IMCBundles)// loop trough the bundles
	{
		if (!IMC_B.GP_IMC.IMC||!IMC_B.PC_IMC.IMC) continue;
		UInputMappingContext* Setting_IMC=(bIsGamepad)?  IMC_B.GP_IMC.IMC: IMC_B.PC_IMC.IMC;
		if (!Setting_IMC) continue;
		
		EModular_IMC_Type Type=IMC_B.Module_Type;
		

		if (Type == EModular_IMC_Type::Custom_IMC)
		{
			Subsystem->AddMappingContext(Setting_IMC,IMC_B.LayerPriority);
		}
		else
		{
			FIMC_Bundle** Target_IMC_B=IMCBundles.Find(Type);// fuck, double ptr
			// find the actual bundle struct in IMCBundles(Which is a contatiner for Ptr of IMCBundle)

			// Target_IMC_B==Ptr to Ptr that map is containing
			// *Target_IMC_B == Ptr that map is containing
			// **Target_IMC_B== Actual struct of IMCBundle

			if (!Target_IMC_B || !*Target_IMC_B)// if the find function failed or founded value of map is nullptr
				continue;
			
			UInputMappingContext* PreviousIMC=bIsGamepad? (*Target_IMC_B)->GP_IMC.IMC: (*Target_IMC_B)->PC_IMC.IMC;
			if (PreviousIMC!=Setting_IMC)
			{
				Subsystem->RemoveMappingContext(PreviousIMC);
				Subsystem->AddMappingContext(Setting_IMC,IMC_B.LayerPriority);
			}

			Target_IMC_B=IMC_B;
			//update the imc bundle in imcbundles map
		}
	}
	
	
	Add_MappingContext_By_IMC_B(Movement_IMCBundle);
	Add_MappingContext_By_IMC_B(World_Interaction_IMCBundle);
	Add_MappingContext_By_IMC_B(UI_Interaction_IMCBundle);
	Add_MappingContext_By_IMC_B(Menu_IMCBundle);

	for (const auto& IMC_B:Custom_IMCBundles)
	{
		Add_MappingContext_By_IMC_B(IMC_B);
	}
	UE_LOG(IMC_Log,Log,TEXT("SetIMCMode Completed"));
}
*/
void AMyPlayerController::Add_MappingContext_By_IMC_B(const FIMC_Bundle& IMC_B)
{
	if (!Subsystem)
	{
		UE_LOG(IMC_Log,Error, TEXT("Invalid Subsystem detected while add mapping context by imcb"))
		return;
	}

	
	if (!IMC_B.GP_IMC.IMC||!IMC_B.PC_IMC.IMC)
	{
		UE_LOG(IMC_Log,Warning,TEXT("Invalid IMC from PC or GP, Nothing to Add"));
		return;
	}

	if (IMC_B.Module_Type==EModular_IMC_Type::Custom_IMC)
	{
		for (const FIMC_Bundle& Current_CustomIMC_B : Custom_IMCBundles)
		{
			if (Current_CustomIMC_B.Module_Type != EModular_IMC_Type::Custom_IMC) continue;
			if (Current_CustomIMC_B.GP_IMC.IMC != IMC_B.GP_IMC.IMC) continue;
			if (Current_CustomIMC_B.PC_IMC.IMC != IMC_B.PC_IMC.IMC) continue;

			UE_LOG(IMC_Log,Error,TEXT("Already Existing Custom IMC_B"));
			return;
		}
		//==== duplication check done ===//

		Custom_IMCBundles.Add(IMC_B);
		UE_LOG(IMC_Log,Log,TEXT("Adding new Custom IMC_B"));
	}
	
	Subsystem->AddMappingContext(IMC_B.GP_IMC.IMC, IMC_B.LayerPriority);
	Subsystem->AddMappingContext(IMC_B.PC_IMC.IMC, IMC_B.LayerPriority);

	UE_LOG(IMC_Log,Log,TEXT("MappingContext Added for %d"), IMC_B.Module_Type);
	//enum cannot be a string --> think and treat them as int
}

void AMyPlayerController::RemoveBundle(const FIMC_Bundle& IMC_B)
{
	/*if (!IMC_B.GP_IMC.IMC||!IMC_B.PC_IMC.IMC)
	{
		UE_LOG(IMC_Log,Warning,TEXT("Invalid IMC from PC or GP, Nothing to remove"));
		return;
	}*/// ---> just remove it
	
	Subsystem->RemoveMappingContext(IMC_B.GP_IMC.IMC);
	Subsystem->RemoveMappingContext(IMC_B.PC_IMC.IMC);
}

bool AMyPlayerController::SetIMC_B_By_Type(const FIMC_Bundle& IMC_B, const EModular_IMC_Type Type)
{
	if (IMC_B.Module_Type != Type)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid IMC Type / Type Not Matching"));
		return false;
	}
	if (IMC_B.Module_Type==EModular_IMC_Type::None)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid IMC Type / Type:None Detected"));
		return false;
	}

	switch (Type)
	{
	case EModular_IMC_Type::Movement_IMC:
		Movement_IMCBundle=IMC_B;
		break;
	case EModular_IMC_Type::World_Interaction_IMC:
		World_Interaction_IMCBundle=IMC_B;
		break;
	case EModular_IMC_Type::UI_Interaction_IMC:
		UI_Interaction_IMCBundle=IMC_B;
		break;
	case EModular_IMC_Type::Menu_IMC:
		Menu_IMCBundle=IMC_B;
		break;
	case EModular_IMC_Type::Custom_IMC:
		Custom_IMCBundles.Add(IMC_B);// add to the array of custom imcbundles
		break;
		
	default:
		UE_LOG(IMC_Log,Error,TEXT("Invalid IMC Type"));
		return false;
	}

	//setting completed
	UE_LOG(IMC_Log,Log,TEXT("Setting Completed"));
	return true;
}
//--> not used yet

//----------------------------------------------------------------------------------------------------------------------

void AMyPlayerController::Bind_IMC_B_Actions(const FIMC_Bundle& IMC) const
{
	if (!IMC.FunctionOwner)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid IMC Owner"));
		return;
	}

	UEnhancedInputComponent* EnhancedInput_Comp=Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput_Comp)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid EnhancedInput_Comp in BindIMCAction"));
		return;
	}
	
	IMC.BindAllActions(EnhancedInput_Comp);
}



bool AMyPlayerController::ValidCheck_IMC_B(const FIMC_Bundle& IMC_B) const
{
	bool isvalid=true;
	
	//======= IMC_B Name Checking =====//
	if (IMC_B.IMCB_Name==NAME_None)
	{
		UE_LOG(IMC_Log,Error,TEXT("UnNamed IMC_B detected"));
		isvalid=false;
	}

	//===== GP IMC Module Checking =====//
	if (IMC_B.GP_IMC.IMC==nullptr)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid GP IMC in %s"),*IMC_B.IMCB_Name.ToString());
		isvalid=false;
	}
	if (IMC_B.GP_IMC.InputAction_Data.IsEmpty())
	{
		UE_LOG(IMC_Log,Error,TEXT("InputAction in GP IMC in %s is Empty"),*IMC_B.IMCB_Name.ToString());
		isvalid=false;
	}

	//===== PC IMC Module Checking =====//
	if (IMC_B.PC_IMC.IMC==nullptr)
	{
		UE_LOG(IMC_Log,Error,TEXT("Invalid PC IMC in %s"),*IMC_B.IMCB_Name.ToString());
		isvalid=false;
	}
	if (IMC_B.PC_IMC.InputAction_Data.IsEmpty())
	{
		UE_LOG(IMC_Log,Error,TEXT("InputAction in PC IMC in %s is Empty"),*IMC_B.IMCB_Name.ToString());
		isvalid=false;
	}

	return isvalid;
}

void AMyPlayerController::AddMappingAndBindActions(const FIMC_Bundle& IMC_B)
{
	Add_MappingContext_By_IMC_B(IMC_B);
	Bind_IMC_B_Actions(IMC_B);
}

//===== Setting IMC_Bs ======//
// these are only for setting the member variable of this controller. these don't add mapping or binding action.
// Order: 1) set imc_b --> 2) add mapping --> 3) bind action

void AMyPlayerController::SetMovementIMCBundles(const FIMC_Bundle& IMC_B)
{
	if (!ValidCheck_IMC_B(IMC_B)) return;
	if (IMC_B.Module_Type!=EModular_IMC_Type::Movement_IMC)
	{
		UE_LOG(IMC_Log,Error,TEXT("Incorrect Module Type detected. %s's type is not Movement type"), *IMC_B.IMCB_Name.ToString());
		return;
	}

	Movement_IMCBundle=IMC_B;
	UE_LOG(IMC_Log,Log,TEXT(" %s's Movement IMC is Settled successfully"),*IMC_B.IMCB_Name.ToString());
}
void AMyPlayerController::SetWorld_Interaction_IMCBundles(const FIMC_Bundle& IMC_B)
{
	if (!ValidCheck_IMC_B(IMC_B)) return;
	if (IMC_B.Module_Type!=EModular_IMC_Type::Movement_IMC)
	{
		UE_LOG(IMC_Log,Error,TEXT("Incorrect Module Type detected. %s's type is not W_Interaction type"), *IMC_B.IMCB_Name.ToString());
		return;
	}

	World_Interaction_IMCBundle=IMC_B;
	UE_LOG(IMC_Log,Log,TEXT(" %s's W_Interaction IMC is Settled successfully"),*IMC_B.IMCB_Name.ToString());
}
void AMyPlayerController::SetUIInteraction_IMCBundles(const FIMC_Bundle& IMC_B)
{
	if (!ValidCheck_IMC_B(IMC_B)) return;
	if (IMC_B.Module_Type!=EModular_IMC_Type::Movement_IMC)
	{
		UE_LOG(IMC_Log,Error,TEXT("Incorrect Module Type detected. %s's type is not UI_Interaction type"), *IMC_B.IMCB_Name.ToString());
		return;
	}

	UI_Interaction_IMCBundle=IMC_B;
	UE_LOG(IMC_Log,Log,TEXT(" %s's UI_Interaction is Settled successfully"),*IMC_B.IMCB_Name.ToString());
}
void AMyPlayerController::SetMenu_IMCBundles(const FIMC_Bundle& IMC_B)
{
	if (!ValidCheck_IMC_B(IMC_B)) return;
	if (IMC_B.Module_Type!=EModular_IMC_Type::Movement_IMC)
	{
		UE_LOG(IMC_Log,Error,TEXT("Incorrect Module Type detected. %s's type is not Menu type"), *IMC_B.IMCB_Name.ToString());
		return;
	}

	Menu_IMCBundle=IMC_B;
	UE_LOG(IMC_Log,Log,TEXT(" %s's Menu IMC is Settled successfully"),*IMC_B.IMCB_Name.ToString());
}
void AMyPlayerController::AddCustom_IMCBundle(const FIMC_Bundle& IMC_B)
{
	if (!ValidCheck_IMC_B(IMC_B)) return;
	if (IMC_B.Module_Type!=EModular_IMC_Type::Movement_IMC)
	{
		UE_LOG(IMC_Log,Error,TEXT("Incorrect Module Type detected. %s's type is not Custom type"), *IMC_B.IMCB_Name.ToString());
		return;
	}
	
	for (FIMC_Bundle& Custom_IMCB: Custom_IMCBundles)//for custom imc_b, duplication check is needed
	{
		if (Custom_IMCB.IMCB_Name==IMC_B.IMCB_Name)//name duplication
		{
			UE_LOG(IMC_Log,Error,TEXT("same name (%s) detected."), *IMC_B.IMCB_Name.ToString());
			return;
		}
	}

	Custom_IMCBundles.Add(IMC_B);
	UE_LOG(IMC_Log,Log,TEXT(" %s's Custom IMC is added successfully"),*IMC_B.IMCB_Name.ToString());
}


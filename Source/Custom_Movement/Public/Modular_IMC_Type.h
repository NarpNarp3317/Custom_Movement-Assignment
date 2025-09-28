#pragma once
#include "CoreMinimal.h"
#include "Modular_IMC_Type.generated.h"// to use in bp


// this is for setting imc for character movement mode, menu input, etc
UENUM(BlueprintType)
enum class EModular_IMC_Type: uint8
{
	None UMETA(DisplayName = "None"),
	Movement_IMC UMETA(DisplayName = "Movement Type"),
	World_Interaction_IMC UMETA(DisplayName = "World Interaction Type"),
	UI_Interaction_IMC UMETA(DisplayName = "UI_Interaction Type"),// not in the menu
	Menu_IMC UMETA(DisplayName = "Menu Type"),//in the menu
	
	//Menu_IMC UMETA(DisplayName = "Menu Type"),
	//--> Make interaction into 2 types
	
	/*
		Movement / World Interaction / UI Interaction
	*/
	
	Custom_IMC UMETA(DisplayName = "Custom Type")
	//--> this is for very special case
};

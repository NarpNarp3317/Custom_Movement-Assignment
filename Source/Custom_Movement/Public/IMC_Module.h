#pragma once

#include "CoreMinimal.h"
#include "InputActionData.h"
#include "IMC_Module.generated.h"

//== Forward Declarations ==//
class UObject;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct FModular_IMC
{
	GENERATED_BODY()

	FModular_IMC(UInputMappingContext* Setting_IMC, TArray<FInputActionData> IA_Data):
	IMC(Setting_IMC), InputAction_Data(IA_Data)
	{}
	
	FModular_IMC():// default construct
	IMC(nullptr),InputAction_Data()// default value.
	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* IMC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<FInputActionData> InputAction_Data;
	// there is a case of more than one inputaction shares the function, so, no tmap, tarray
};
	//fuck it just put bind function in struct, no extra manager classes
	// function in struct!! Primitive it is!



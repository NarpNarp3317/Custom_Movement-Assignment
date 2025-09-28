// Fill out your copyright notice in the Description page of Project Settings.

//**********************************************************************************************************************
//TODO --> Make a function to add Overlayable IMC_Bundle(ex. item usage, basic interaction, core movement input etc)
//**********************************************************************************************************************
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Modular_IMC_Type.h"// for using TMAP
#include "IMC_Bundle_Str.h"// 2 version storage struct
#include "MyPlayerController.generated.h"

//===== Forward Decleration ======//
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;

/*//--->Make Seperate header and share it with other class
// make a struct to store 2 versions of same imc as one
USTRUCT(BlueprintType)//
struct FIMC_Bundle// f for struct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* GP_IMC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* PC_IMC;

	FIMC_Bundle(UInputMappingContext* GP_IMC,UInputMappingContext* PC_IMC)//default constructor
	: GP_IMC(GP_IMC),PC_IMC(PC_IMC){}

	FIMC_Bundle(): GP_IMC(nullptr),PC_IMC(nullptr){}// for default constructor
};
*/

UCLASS()
class CUSTOM_MOVEMENT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	//=== Confirmation ===//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input / Confirmation")
	bool bIsGamepad;//checking if the current imc is set on gamepad or pc

	/*
	Each IMC has 2 versions (Gamepad, PC)
		when IMC mode is changed, GP and PC should be updated as pair --> should it be stored in one struct?


	
	//=== Control Input(PC) ===//

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* IMC_PC;
	//=== Control Input(GamePad) ===//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* IMC_GP;

	//---> now as one


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FIMC_Bundle Basic_IMCBundle;
	*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FIMC_Bundle Movement_IMCBundle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FIMC_Bundle World_Interaction_IMCBundle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FIMC_Bundle UI_Interaction_IMCBundle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")// this will be kept while basic imc bundle is changed
	FIMC_Bundle Menu_IMCBundle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")// this will be kept while basic imc bundle is changed
	TArray<FIMC_Bundle> Custom_IMCBundles;// could be multiple custom imc layers

	
	// so, imc allways has to have pc and gamepad version of imc

	/* This wont be needed, cause imc is now added as seperate modules
	UPROPERTY()//for garbage system
	UInputMappingContext* Current_IMC;// determined by the Current_IMCBundle
	*/

	UPROPERTY()
	APawn* PossessedPawn;
	UPROPERTY()
	ULocalPlayer* LocalPlayer;
	UPROPERTY()//for garbage system
	UEnhancedInputLocalPlayerSubsystem* Subsystem;

	
/* Dont Store Every InputActions in Controller, sotre them in imc bundle struct
	//=== Controller Inputs ===//
	// Basics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveRightAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveUpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RotateRight;
	// additional
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SprintAction;

	
	//=== UI Input ====//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* Menu;// this will switch the current imc to menu imc of current input type
*/

protected:
	virtual void BeginPlay() override;

	//==== functions ====//
public:
	//=== InputType Checking ===//
	virtual void SetupInputComponent() override;// set the mapping component
	void OnAnyInputPressed(FKey Key);// check if the input is from pc or gamepad( will return (is Gamepad?))
	//void SetPawnPtr(APawn* NewPawn);
	
	/*void Hard_ResetIMC_Bundle(FIMC_Bundle IMC_B);// remove all imc and set new imc
	void SwitchIMC_Bundle(FIMC_Bundle IMC_B);
	void AddIMC_Bundle(FIMC_Bundle IMC_B);
	void RemoveIMC_Bundle(FIMC_Bundle IMC_B);*/
	// not required

	virtual void OnPossess(APawn* NewPawn) override;
	
	UFUNCTION(BlueprintCallable)
	void Add_MappingContext_By_IMC_B(const FIMC_Bundle& IMC_B);
	void RemoveBundle(const FIMC_Bundle& IMC_B);
	
	bool SetIMC_B_By_Type(const FIMC_Bundle& IMC_B, const EModular_IMC_Type Type);// this will set the Member variabbles IMC_Bs in here
	// it will return true if the setting is done, and false for failed attempt

	//=== Final Binding ===//
	void Bind_IMC_B_Actions(const FIMC_Bundle& IMC) const;
	bool ValidCheck_IMC_B(const FIMC_Bundle& IMC_B) const;
	
	//===For setting the member variables ===//
	UFUNCTION(BlueprintCallable)
	void SetMovementIMCBundles(const FIMC_Bundle& IMC_B);
	UFUNCTION(BlueprintCallable)
	void SetWorld_Interaction_IMCBundles(const FIMC_Bundle& IMC_B);
	UFUNCTION(BlueprintCallable)
	void SetUIInteraction_IMCBundles(const FIMC_Bundle& IMC_B);
	UFUNCTION(BlueprintCallable)
	void SetMenu_IMCBundles(const FIMC_Bundle& IMC_B);
	UFUNCTION(BlueprintCallable)
	void AddCustom_IMCBundle(const FIMC_Bundle& IMC_B);

	UFUNCTION(BlueprintCallable)
	void AddMappingAndBindActions(const FIMC_Bundle& IMC_B);
};

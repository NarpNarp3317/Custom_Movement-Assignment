#pragma once
#include "CoreMinimal.h"
#include "CustomMovementMode.generated.h"// to use in bp

UENUM(BlueprintType)
enum class ECustomMovementMode: uint8
{
	None UMETA(DisplayName = "None"),
	Basic_Ground_Move UMETA(DisplayName = "basic ground movement mode"),
	Drone_Move UMETA(DisplayName = "Drone Flight mode"),
	Plane_Move UMETA(DisplayName = "Plane Flight mode"),
	Roll_Move UMETA(DisplayName = "Roll Move Mode"),
	Flip_Move UMETA(DisplayName = "Flip Move Mode"),// this will be done by flipping the character based on its contact surface
	
	// and many more like ladder, rope, wall climbing, bug like attached movement mode, swim, glide, etc
	// each move mode requires its own movement comp to be used
};

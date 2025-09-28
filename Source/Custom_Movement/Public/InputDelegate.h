#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputDelegate.h"// this one is not on reflection system, so, no generated.h

DECLARE_DELEGATE_OneParam(FinputActionDelegate, const FInputActionValue&)
//--> Declare a delegate that takes one parameter( non dynamic, single delegate)
//FinputActionDelegate== the declared delegate used as datatype
// const FInputActionValue& --> to match the param of the function call


#include "CodeForgeNode_Function.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"

FText UCodeForgeNode_Function::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Function: %s"), *FunctionDef.Name));
}

FLinearColor UCodeForgeNode_Function::GetNodeTitleColor() const
{
	return GetDefault<UCodeForgeSettings>()->FunctionNodeColor;
}

void UCodeForgeNode_Function::AllocateDefaultPins()
{
	FEdGraphPinType FunctionPinType;
	FunctionPinType.PinCategory = TEXT("PC_CodeForgeFunction");
	CreatePin(EGPD_Input, FunctionPinType, TEXT("Owner"));
}

void UCodeForgeNode_Function::SyncToBlueprint(UCodeForgeBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	// Only add if connected to a Class node
	UEdGraphPin* InputPin = FindPin(TEXT("Owner"), EGPD_Input);
	if (!InputPin || InputPin->LinkedTo.Num() == 0)
	{
		return;
	}

	Blueprint->Functions.Add(FunctionDef);
}

void UCodeForgeNode_Function::SyncFromBlueprint(const UCodeForgeBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return;
	}

	for (const FCodeForgeFunctionDef& Func : Blueprint->Functions)
	{
		if (Func.Name == FunctionDef.Name)
		{
			FunctionDef = Func;
			return;
		}
	}
}


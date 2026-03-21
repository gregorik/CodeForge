#include "CodeForgeNode_Class.h"
#include "CodeForgeBlueprint.h"

FText UCodeForgeNode_Class::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Class: %s"), *ClassName));
}

FLinearColor UCodeForgeNode_Class::GetNodeTitleColor() const
{
	return FLinearColor(0.0f, 0.2f, 0.8f);
}

void UCodeForgeNode_Class::AllocateDefaultPins()
{
	FEdGraphPinType PropertyPinType;
	PropertyPinType.PinCategory = TEXT("PC_CodeForgeProperty");
	CreatePin(EGPD_Output, PropertyPinType, TEXT("Properties"));

	FEdGraphPinType FunctionPinType;
	FunctionPinType.PinCategory = TEXT("PC_CodeForgeFunction");
	CreatePin(EGPD_Output, FunctionPinType, TEXT("Functions"));

	FEdGraphPinType RPCPinType;
	RPCPinType.PinCategory = TEXT("PC_CodeForgeRPC");
	CreatePin(EGPD_Output, RPCPinType, TEXT("RPCs"));
}

void UCodeForgeNode_Class::SyncToBlueprint(UCodeForgeBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	Blueprint->ClassName = ClassName;
	Blueprint->BlueprintKind = ECodeForgeBlueprintKind::Class;
	Blueprint->ClassType = ClassType;
	Blueprint->bReplicated = bReplicated;
}

void UCodeForgeNode_Class::SyncFromBlueprint(const UCodeForgeBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return;
	}

	ClassName = Blueprint->ClassName;
	ClassType = Blueprint->ClassType;
	bReplicated = Blueprint->bReplicated;
}

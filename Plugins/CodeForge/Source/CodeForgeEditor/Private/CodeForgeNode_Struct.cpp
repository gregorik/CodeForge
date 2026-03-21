#include "CodeForgeNode_Struct.h"
#include "CodeForgeBlueprint.h"

FText UCodeForgeNode_Struct::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Struct: %s"), *ClassName));
}

FLinearColor UCodeForgeNode_Struct::GetNodeTitleColor() const
{
	return FLinearColor(0.0f, 0.6f, 0.2f);
}

void UCodeForgeNode_Struct::AllocateDefaultPins()
{
	FEdGraphPinType PropertyPinType;
	PropertyPinType.PinCategory = TEXT("PC_CodeForgeProperty");
	CreatePin(EGPD_Output, PropertyPinType, TEXT("Properties"));
}

void UCodeForgeNode_Struct::SyncToBlueprint(UCodeForgeBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	Blueprint->ClassName = ClassName;
	Blueprint->BlueprintKind = ECodeForgeBlueprintKind::Struct;
}

void UCodeForgeNode_Struct::SyncFromBlueprint(const UCodeForgeBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return;
	}

	ClassName = Blueprint->ClassName;
}

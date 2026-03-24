#include "CodeForgeNode_Struct.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"

FText UCodeForgeNode_Struct::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Struct: %s"), *ClassName));
}

FLinearColor UCodeForgeNode_Struct::GetNodeTitleColor() const
{
	return GetDefault<UCodeForgeSettings>()->StructNodeColor;
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


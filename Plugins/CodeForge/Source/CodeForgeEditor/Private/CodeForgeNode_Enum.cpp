#include "CodeForgeNode_Enum.h"
#include "CodeForgeBlueprint.h"

FText UCodeForgeNode_Enum::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Enum: %s"), *ClassName));
}

FLinearColor UCodeForgeNode_Enum::GetNodeTitleColor() const
{
	return FLinearColor(0.8f, 0.7f, 0.0f);
}

void UCodeForgeNode_Enum::AllocateDefaultPins()
{
}

void UCodeForgeNode_Enum::SyncToBlueprint(UCodeForgeBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	Blueprint->ClassName = ClassName;
	Blueprint->BlueprintKind = ECodeForgeBlueprintKind::Enum;
	Blueprint->EnumEntries = EnumEntries;
	Blueprint->bBlueprintType = bBlueprintType;
}

void UCodeForgeNode_Enum::SyncFromBlueprint(const UCodeForgeBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return;
	}

	ClassName = Blueprint->ClassName;
	EnumEntries = Blueprint->EnumEntries;
	bBlueprintType = Blueprint->bBlueprintType;
}

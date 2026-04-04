// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "CodeForgeNode_Enum.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"

FText UCodeForgeNode_Enum::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Enum: %s"), *ClassName));
}

FLinearColor UCodeForgeNode_Enum::GetNodeTitleColor() const
{
	return GetDefault<UCodeForgeSettings>()->EnumNodeColor;
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



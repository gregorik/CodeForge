// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeAssetTypeActions.cpp
// FAssetTypeActions_Base implementation for UCodeForgeBlueprint.

#include "CodeForgeAssetTypeActions.h"
#include "CodeForgeAssetEditor.h"
#include "CodeForgeBlueprint.h"

#define LOCTEXT_NAMESPACE "CodeForgeAssetTypeActions"

FText FCodeForgeAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "CodeForge Blueprint");
}

FColor FCodeForgeAssetTypeActions::GetTypeColor() const
{
	return FColor(0, 120, 215);
}

UClass* FCodeForgeAssetTypeActions::GetSupportedClass() const
{
	return UCodeForgeBlueprint::StaticClass();
}

uint32 FCodeForgeAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FCodeForgeAssetTypeActions::OpenAssetEditor(
	const TArray<UObject*>& InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		UCodeForgeBlueprint* Blueprint = Cast<UCodeForgeBlueprint>(Obj);
		if (Blueprint)
		{
			TSharedRef<FCodeForgeAssetEditor> Editor = MakeShared<FCodeForgeAssetEditor>();
			Editor->InitEditor(Mode, EditWithinLevelEditor, Blueprint);
		}
	}
}

#undef LOCTEXT_NAMESPACE



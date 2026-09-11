// Copyright (c) 2026 GregOrigin. All Rights Reserved.
// CodeForgeAssetFactory.cpp
// Creates new UCodeForgeBlueprint assets when the user right-clicks in Content Browser.

#include "CodeForgeAssetFactory.h"

#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"
#include "Misc/App.h"

#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IMainFrameModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWindow.h"

UCodeForgeAssetFactory::UCodeForgeAssetFactory()
{
	SupportedClass = UCodeForgeBlueprint::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}



UObject* UCodeForgeAssetFactory::FactoryCreateNew(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn)
{
	UCodeForgeBlueprint* NewBlueprint = NewObject<UCodeForgeBlueprint>(
		InParent, InClass, InName, Flags);
	if (!NewBlueprint) return nullptr;

	NewBlueprint->ClassName = InName.ToString();

	const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
	if (NewBlueprint->ModuleTarget.IsEmpty())
	{
		NewBlueprint->ModuleTarget = (Settings && !Settings->DefaultModuleTarget.IsEmpty())
			? Settings->DefaultModuleTarget : FApp::GetProjectName();
		NewBlueprint->SubDirectory = Settings ? Settings->DefaultSubDirectory : TEXT("");
	}

	return NewBlueprint;
}

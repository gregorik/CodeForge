// CodeForgeAssetFactory.cpp
// Creates new UCodeForgeBlueprint assets when the user right-clicks in Content Browser.

#include "CodeForgeAssetFactory.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"
#include "Misc/App.h"

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

	if (NewBlueprint)
	{
		NewBlueprint->ClassName = InName.ToString();

		// Default ModuleTarget from settings, falling back to project name
		if (NewBlueprint->ModuleTarget.IsEmpty())
		{
			const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
			if (Settings && !Settings->DefaultModuleTarget.IsEmpty())
			{
				NewBlueprint->ModuleTarget = Settings->DefaultModuleTarget;
			}
			else
			{
				NewBlueprint->ModuleTarget = FApp::GetProjectName();
			}

			NewBlueprint->SubDirectory = Settings ? Settings->DefaultSubDirectory : TEXT("");
		}
	}

	return NewBlueprint;
}


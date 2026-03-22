// CodeForgeAssetFactory.cpp
// Creates new UCodeForgeBlueprint assets when the user right-clicks in Content Browser.

#include "CodeForgeAssetFactory.h"
#include "CodeForgeBlueprint.h"
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

		// Default ModuleTarget to the project's game module name
		if (NewBlueprint->ModuleTarget.IsEmpty())
		{
			NewBlueprint->ModuleTarget = FApp::GetProjectName();
		}
	}

	return NewBlueprint;
}

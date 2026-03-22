#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CodeForgeSettings.generated.h"

UCLASS(config=CodeForge, defaultconfig, meta=(DisplayName="CodeForge"))
class CODEFORGEEDITOR_API UCodeForgeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Default module target for newly created CodeForge blueprints. */
	UPROPERTY(config, EditAnywhere, Category = "Code Generation")
	FString DefaultModuleTarget;

	/** Default sub-directory under the module's Public/Private folders. */
	UPROPERTY(config, EditAnywhere, Category = "Code Generation")
	FString DefaultSubDirectory;

	/** Override template directory (leave empty to use plugin defaults). */
	UPROPERTY(config, EditAnywhere, Category = "Code Generation", meta=(RelativePath))
	FDirectoryPath CustomTemplatePath;

	/** Whether to automatically attempt Live Coding after behavioral changes. */
	UPROPERTY(config, EditAnywhere, Category = "Code Generation")
	bool bAutoLiveCodingOnBehavioralChange = true;

	/** Whether to show the code preview panel by default in new editors. */
	UPROPERTY(config, EditAnywhere, Category = "Editor")
	bool bShowPreviewByDefault = true;

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};

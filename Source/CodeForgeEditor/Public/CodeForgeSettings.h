// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CodeForgeSettings.generated.h"

UCLASS(config=CodeForge, defaultconfig, meta=(DisplayName="CodeForge Core"))
class CODEFORGEEDITOR_API UCodeForgeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// -----------------------------------------------------------------------
	// Code Generation
	// -----------------------------------------------------------------------

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

	// -----------------------------------------------------------------------
	// Editor
	// -----------------------------------------------------------------------

	/** Whether to show the code preview panel by default in new editors. */
	UPROPERTY(config, EditAnywhere, Category = "Editor")
	bool bShowPreviewByDefault = true;

	/** Whether to show the "Create CodeForge Blueprint" button in the level editor toolbar. */
	UPROPERTY(config, EditAnywhere, Category = "Editor")
	bool bShowToolbarButton = true;

	// -----------------------------------------------------------------------
	// Appearance â€” Node Colors
	// -----------------------------------------------------------------------

	/** Class node title color. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Node Colors")
	FLinearColor ClassNodeColor = FLinearColor(0.12f, 0.28f, 0.85f);

	/** Struct node title color. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Node Colors")
	FLinearColor StructNodeColor = FLinearColor(0.1f, 0.7f, 0.25f);

	/** Enum node title color. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Node Colors")
	FLinearColor EnumNodeColor = FLinearColor(0.85f, 0.65f, 0.0f);

	/** Property node title color. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Node Colors")
	FLinearColor PropertyNodeColor = FLinearColor(0.25f, 0.8f, 0.35f);

	/** Function node title color. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Node Colors")
	FLinearColor FunctionNodeColor = FLinearColor(0.9f, 0.5f, 0.0f);

	// -----------------------------------------------------------------------
	// Appearance â€” Branding
	// -----------------------------------------------------------------------

	/** Primary brand accent color used for top stripes, tab headers, and highlights. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Branding")
	FLinearColor BrandAccentColor = FLinearColor(0.0f, 0.72f, 0.88f);

	/** Secondary brand color used for watermarks and subtle highlights. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Branding")
	FLinearColor BrandSecondaryColor = FLinearColor(0.08f, 0.14f, 0.32f);

	/** Node body background darkness (0 = pitch black, 1 = white). */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Branding", meta=(ClampMin="0.01", ClampMax="0.2", UIMin="0.01", UIMax="0.2"))
	float NodeBodyBrightness = 0.055f;

	/** Whether type nodes (Class/Struct/Enum) get a subtle color tint on the body. */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Branding")
	bool bTintTypeNodeBodies = true;

	/** Title font size for type nodes (Class, Struct, Enum). */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Branding", meta=(ClampMin="8", ClampMax="18", UIMin="8", UIMax="18"))
	int32 TypeNodeFontSize = 12;

	/** Title font size for member nodes (Property, Function). */
	UPROPERTY(config, EditAnywhere, Category = "Appearance|Branding", meta=(ClampMin="8", ClampMax="16", UIMin="8", UIMax="16"))
	int32 MemberNodeFontSize = 10;

	// -----------------------------------------------------------------------

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};



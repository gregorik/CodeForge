// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "CodeForgeStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FCodeForgeStyle::StyleInstance = nullptr;

void FCodeForgeStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FCodeForgeStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

FName FCodeForgeStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("CodeForgeStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FCodeForgeStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(FCodeForgeStyle::GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("CodeForge")->GetBaseDir() / TEXT("Resources"));

	Style->Set("CodeForge.ToolbarIcon", new IMAGE_BRUSH(TEXT("CodeForgeToolbar"), Icon40x40));
	Style->Set("CodeForge.ToolbarIcon.Small", new IMAGE_BRUSH(TEXT("CodeForgeToolbar"), Icon20x20));

	Style->Set("CodeForge.RPGKitIcon", new IMAGE_BRUSH(TEXT("RPGKitToolbar"), Icon40x40));
	Style->Set("CodeForge.RPGKitIcon.Small", new IMAGE_BRUSH(TEXT("RPGKitToolbar"), Icon20x20));

	// Welcome window hero banner (uses the JPG with its black backdrop intact).
	Style->Set("CodeForge.WelcomeBanner", new FSlateImageBrush(
		Style->RootToContentDir(TEXT("CodeForgeBanner"), TEXT(".jpg")),
		FVector2D(800.f, 327.f)));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FCodeForgeStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FCodeForgeStyle::Get()
{
	return *StyleInstance;
}

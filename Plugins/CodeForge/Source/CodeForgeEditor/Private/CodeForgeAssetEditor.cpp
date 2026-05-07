// Copyright (c) 2026 GregOrigin. All Rights Reserved.
// CodeForgeAssetEditor.cpp
// Implements the graph-based asset editor for UCodeForgeBlueprint.

#include "CodeForgeAssetEditor.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeEdGraph.h"
#include "CodeForgeEdGraphSchema.h"
#include "CodeForgeGenerator.h"
#include "CodeForgeSettings.h"
#include "Wizard/CodeForgeWizardSession.h"

#include "GraphEditor.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Application/SlateApplication.h"
#include "GraphEditorActions.h"
#include "FileHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SOverlay.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeNode_Interface.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeSettings.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeRPCDef.h"
#include "CodeForgeEnumDef.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/App.h"
#include "Misc/PackageName.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "CodeForgeDelegateDef.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "CodeForgeAssetEditor"

namespace
{
	UClass* LoadRPGStarterKitClass(UClass* BaseClass, const FString& ModuleName, const TCHAR* ReflectedClassName)
	{
		const FString ClassPath = FString::Printf(TEXT("/Script/%s.%s"), *ModuleName, ReflectedClassName);
		return StaticLoadClass(BaseClass, nullptr, *ClassPath);
	}

	AActor* SpawnRPGDemoActor(UWorld* World, UClass* ActorClass, const FVector& Location, const FRotator& Rotation, const TCHAR* Label)
	{
		if (!World || !ActorClass)
		{
			return nullptr;
		}

		AActor* Actor = World->SpawnActor<AActor>(ActorClass, Location, Rotation);
		if (Actor)
		{
			Actor->SetActorLabel(Label);
		}
		return Actor;
	}

	bool CreateRPGStarterKitDemoMap(FString& OutStatus)
	{
		UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		if (!World)
		{
			OutStatus = TEXT("Demo map was not created: NewBlankMap failed.");
			return false;
		}

		const FString ModuleName = FApp::GetProjectName();
		UClass* GameModeClass = LoadRPGStarterKitClass(AGameModeBase::StaticClass(), ModuleName, TEXT("RPGGameMode"));
		UClass* EnemyClass = LoadRPGStarterKitClass(AActor::StaticClass(), ModuleName, TEXT("RPGEnemy"));
		UClass* PickupClass = LoadRPGStarterKitClass(AActor::StaticClass(), ModuleName, TEXT("RPGItemPickup"));

		if (AWorldSettings* Settings = World->GetWorldSettings())
		{
			Settings->DefaultGameMode = GameModeClass;
		}

		APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(FVector(-320.0f, 0.0f, 90.0f), FRotator::ZeroRotator);
		if (PlayerStart)
		{
			PlayerStart->SetActorLabel(TEXT("RPG Demo PlayerStart"));
		}

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(220.0f, 0.0f, -12.0f), FRotator::ZeroRotator);
		if (Floor)
		{
			Floor->SetActorLabel(TEXT("RPG Demo Floor"));
			Floor->SetActorScale3D(FVector(12.0f, 8.0f, 0.12f));
			if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
			{
				Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			}
			Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
		}

		SpawnRPGDemoActor(World, EnemyClass, FVector(420.0f, -140.0f, 90.0f), FRotator::ZeroRotator, TEXT("RPG Demo Enemy A"));
		SpawnRPGDemoActor(World, EnemyClass, FVector(640.0f, 180.0f, 90.0f), FRotator::ZeroRotator, TEXT("RPG Demo Enemy B"));
		SpawnRPGDemoActor(World, PickupClass, FVector(90.0f, -220.0f, 90.0f), FRotator::ZeroRotator, TEXT("RPG Demo Pickup A"));
		SpawnRPGDemoActor(World, PickupClass, FVector(260.0f, 240.0f, 90.0f), FRotator::ZeroRotator, TEXT("RPG Demo Pickup B"));

		const FString MapPath = TEXT("/Game/CodeForgeExamples/RPGStarterKit/Maps/RPG_Demo");
		const FString MapDirectory = FPackageName::LongPackageNameToFilename(TEXT("/Game/CodeForgeExamples/RPGStarterKit/Maps"));
		IFileManager::Get().MakeDirectory(*MapDirectory, true);

		const bool bSaved = UEditorLoadingAndSavingUtils::SaveMap(World, MapPath);
		if (!bSaved)
		{
			OutStatus = TEXT("Demo map was created in memory but was not saved.");
			return false;
		}

		if (!GameModeClass || !EnemyClass || !PickupClass)
		{
			OutStatus = TEXT("Demo map saved. Compile the project, then reopen the map if runtime classes were missing.");
			return false;
		}

		OutStatus = TEXT("Playable map saved to /Game/CodeForgeExamples/RPGStarterKit/Maps/RPG_Demo.");
		return true;
	}

	static FAutoConsoleCommand CreateRPGStarterKitDemoMapCommand(
		TEXT("CodeForge.CreateRPGStarterKitDemoMap"),
		TEXT("Creates the RPG Starter Kit playable demo map."),
		FConsoleCommandDelegate::CreateStatic([]()
		{
			FString Status;
			CreateRPGStarterKitDemoMap(Status);
			UE_LOG(LogTemp, Log, TEXT("CodeForge: %s"), *Status);
		}));

	const FString& GetRPGStarterKitContentPath()
	{
		static const FString Path = TEXT("/Game/CodeForgeExamples/RPGStarterKit");
		return Path;
	}

	const FString& GetRPGStarterKitDemoMapPackagePath()
	{
		static const FString Path = TEXT("/Game/CodeForgeExamples/RPGStarterKit/Maps/RPG_Demo");
		return Path;
	}

	const TArray<FString>& GetRPGStarterKitAssetNames()
	{
		static const TArray<FString> Names = {
			TEXT("EItemRarity"), TEXT("RPGItemData"), TEXT("IDamageable"), TEXT("IInteractable"),
			TEXT("RPGCharacter"), TEXT("RPGInventoryComponent"), TEXT("RPGEnemy"),
			TEXT("RPGItemPickup"), TEXT("RPGGameMode")
		};
		return Names;
	}

	const TArray<FString>& GetRPGStarterKitGeneratedFileNames()
	{
		static const TArray<FString> Names = {
			TEXT("EItemRarity.h"),
			TEXT("FRPGItemData.h"),
			TEXT("Damageable.h"),
			TEXT("Interactable.h"),
			TEXT("ARPGCharacter.h"),
			TEXT("ARPGCharacter.cpp"),
			TEXT("URPGInventoryComponent.h"),
			TEXT("URPGInventoryComponent.cpp"),
			TEXT("ARPGEnemy.h"),
			TEXT("ARPGEnemy.cpp"),
			TEXT("ARPGItemPickup.h"),
			TEXT("ARPGItemPickup.cpp"),
			TEXT("ARPGGameMode.h"),
			TEXT("ARPGGameMode.cpp")
		};
		return Names;
	}

	const TArray<FString>& GetRPGStarterKitRuntimeClassNames()
	{
		static const TArray<FString> Names = {
			TEXT("RPGCharacter"),
			TEXT("RPGInventoryComponent"),
			TEXT("RPGEnemy"),
			TEXT("RPGItemPickup"),
			TEXT("RPGGameMode")
		};
		return Names;
	}

	bool DoesLongPackageExist(const FString& LongPackageName)
	{
		FString Filename;
		return FPackageName::DoesPackageExist(LongPackageName, &Filename);
	}

	FString GetRPGStarterKitSourceDir()
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), FApp::GetProjectName());
	}

	FString GetRPGStarterKitGeneratedSamplesIndexPath()
	{
		return FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GeneratedCodeSamples"), TEXT("CodeForge_RPGStarterKit"), TEXT("index.html")));
	}

	FString GetRPGStarterKitManualPath()
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("CodeForge"));
		return Plugin.IsValid()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources"), TEXT("Docs"), TEXT("RPGKit_Manual.html")))
			: FString();
	}

	int32 CountRPGStarterKitAssets()
	{
		int32 Count = 0;
		for (const FString& Name : GetRPGStarterKitAssetNames())
		{
			if (DoesLongPackageExist(GetRPGStarterKitContentPath() / Name))
			{
				Count++;
			}
		}
		return Count;
	}

	int32 CountRPGStarterKitSourceFiles()
	{
		int32 Count = 0;
		const FString SourceDir = GetRPGStarterKitSourceDir();
		for (const FString& FileName : GetRPGStarterKitGeneratedFileNames())
		{
			if (FPaths::FileExists(FPaths::Combine(SourceDir, FileName)))
			{
				Count++;
			}
		}
		return Count;
	}

	int32 CountRPGStarterKitLoadedRuntimeClasses()
	{
		int32 Count = 0;
		const FString ModuleName = FApp::GetProjectName();
		for (const FString& ClassName : GetRPGStarterKitRuntimeClassNames())
		{
			if (LoadRPGStarterKitClass(UObject::StaticClass(), ModuleName, *ClassName))
			{
				Count++;
			}
		}
		return Count;
	}

	class SCodeForgeRPGSetupWizard : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SCodeForgeRPGSetupWizard) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			RefreshStatus();
			LastAction = LOCTEXT("RPGSetupWizardReady", "Ready. Create or refresh the RPG Starter Kit, then build if runtime classes are not loaded.");

			ChildSlot
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(16.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("RPGSetupWizardTitle", "RPG Starter Kit Setup"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 6.0f, 0.0f, 12.0f)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("RPGSetupWizardIntro", "Use this wizard to create the complete example, check what is already available, open the playable map, and launch the proof artifacts from one place."))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SSeparator)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0.0f, 12.0f, 0.0f, 12.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							MakeStepRow(
								LOCTEXT("RPGSetupAssetsTitle", "Starter assets"),
								LOCTEXT("RPGSetupAssetsBody", "Nine CodeForge assets covering enums, structs, interfaces, classes, components, delegates, RPCs, enemies, pickups, and game mode setup."),
								TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetAssetsStatusText)),
								TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetAssetsStatusColor)))
						]
						+ SScrollBox::Slot()
						[
							MakeStepRow(
								LOCTEXT("RPGSetupSourceTitle", "Generated C++"),
								LOCTEXT("RPGSetupSourceBody", "Native Unreal source files in the active project module, including UHT types, playable runtime classes, and interaction/combat code."),
								TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetSourceStatusText)),
								TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetSourceStatusColor)))
						]
						+ SScrollBox::Slot()
						[
							MakeStepRow(
								LOCTEXT("RPGSetupMapTitle", "Playable demo map"),
								LOCTEXT("RPGSetupMapBody", "A ready-to-open map with a PlayerStart, floor, pickups, enemies, and RPG game mode wiring."),
								TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetMapStatusText)),
								TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetMapStatusColor)))
						]
						+ SScrollBox::Slot()
						[
							MakeStepRow(
								LOCTEXT("RPGSetupRuntimeTitle", "Compiled runtime"),
								LOCTEXT("RPGSetupRuntimeBody", "Loaded C++ classes required for pressing Play and running the automation test."),
								TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetRuntimeStatusText)),
								TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetRuntimeStatusColor)))
						]
						+ SScrollBox::Slot()
						[
							MakeStepRow(
								LOCTEXT("RPGSetupProofTitle", "Proof bundle"),
								LOCTEXT("RPGSetupProofBody", "Generated code samples and build/test evidence under Saved/GeneratedCodeSamples."),
								TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetSamplesStatusText)),
								TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCodeForgeRPGSetupWizard::GetSamplesStatusColor)))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						MakeActionsPanel()
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 12.0f, 0.0f, 0.0f)
					[
						SAssignNew(LastActionText, STextBlock)
						.AutoWrapText(true)
						.Text(this, &SCodeForgeRPGSetupWizard::GetLastActionText)
					]
				]
			];
		}

	private:
		struct FSetupState
		{
			int32 AssetCount = 0;
			int32 SourceFileCount = 0;
			int32 RuntimeClassCount = 0;
			bool bDemoMapExists = false;
			bool bManualExists = false;
			bool bGeneratedSamplesIndexExists = false;
		};

		FSetupState State;
		FText LastAction;
		TSharedPtr<STextBlock> LastActionText;

		void RefreshStatus()
		{
			State.AssetCount = CountRPGStarterKitAssets();
			State.SourceFileCount = CountRPGStarterKitSourceFiles();
			State.RuntimeClassCount = CountRPGStarterKitLoadedRuntimeClasses();
			State.bDemoMapExists = DoesLongPackageExist(GetRPGStarterKitDemoMapPackagePath());
			State.bManualExists = FPaths::FileExists(GetRPGStarterKitManualPath());
			State.bGeneratedSamplesIndexExists = FPaths::FileExists(GetRPGStarterKitGeneratedSamplesIndexPath());
		}

		TSharedRef<SWidget> MakeStepRow(
			const FText& Title,
			const FText& Body,
			TAttribute<FText> StatusText,
			TAttribute<FSlateColor> StatusColor) const
		{
			return SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(10.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(Title)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 16.0f, 0.0f)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(Body)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(StatusText)
						.ColorAndOpacity(StatusColor)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
				];
		}

		TSharedRef<SWidget> MakeActionsPanel()
		{
			return SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 8.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupCreateButton", "Create / Refresh"))
						.ToolTipText(LOCTEXT("RPGSetupCreateTooltip", "Create the nine example assets, generate or preserve C++ output, and create the demo map."))
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnCreateClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 8.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupOpenMapButton", "Open Map"))
						.ToolTipText(LOCTEXT("RPGSetupOpenMapTooltip", "Open /Game/CodeForgeExamples/RPGStarterKit/Maps/RPG_Demo."))
						.IsEnabled(this, &SCodeForgeRPGSetupWizard::CanOpenDemoMap)
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnOpenMapClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 8.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupBrowseAssetsButton", "Browse Assets"))
						.ToolTipText(LOCTEXT("RPGSetupBrowseAssetsTooltip", "Focus the Content Browser on the RPG Starter Kit folder."))
						.IsEnabled(this, &SCodeForgeRPGSetupWizard::CanBrowseAssets)
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnBrowseAssetsClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 8.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupExploreSourceButton", "Source Folder"))
						.ToolTipText(LOCTEXT("RPGSetupExploreSourceTooltip", "Open the active project source directory containing the generated files."))
						.IsEnabled(this, &SCodeForgeRPGSetupWizard::CanExploreSource)
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnExploreSourceClicked)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupManualButton", "Manual"))
						.ToolTipText(LOCTEXT("RPGSetupManualTooltip", "Open the RPG Starter Kit manual."))
						.IsEnabled(this, &SCodeForgeRPGSetupWizard::CanOpenManual)
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnOpenManualClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupSamplesButton", "Samples"))
						.ToolTipText(LOCTEXT("RPGSetupSamplesTooltip", "Open the generated code sample bundle if it exists."))
						.IsEnabled(this, &SCodeForgeRPGSetupWizard::CanOpenGeneratedSamples)
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnOpenGeneratedSamplesClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupRunTestButton", "Run Test"))
						.ToolTipText(LOCTEXT("RPGSetupRunTestTooltip", "Queue CodeForge.RPGStarterKit.PlayableLoop in the Automation framework."))
						.IsEnabled(this, &SCodeForgeRPGSetupWizard::CanRunAutomation)
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnRunAutomationClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("RPGSetupRefreshButton", "Refresh Status"))
						.ToolTipText(LOCTEXT("RPGSetupRefreshTooltip", "Rescan assets, files, map, classes, docs, and samples."))
						.OnClicked(this, &SCodeForgeRPGSetupWizard::OnRefreshClicked)
					]
				];
		}

		FText GetAssetsStatusText() const
		{
			return FText::Format(LOCTEXT("RPGSetupAssetsStatus", "{0}/{1} ready"), State.AssetCount, GetRPGStarterKitAssetNames().Num());
		}

		FSlateColor GetAssetsStatusColor() const
		{
			return State.AssetCount == GetRPGStarterKitAssetNames().Num() ? ReadyColor() : AttentionColor();
		}

		FText GetSourceStatusText() const
		{
			return FText::Format(LOCTEXT("RPGSetupSourceStatus", "{0}/{1} files"), State.SourceFileCount, GetRPGStarterKitGeneratedFileNames().Num());
		}

		FSlateColor GetSourceStatusColor() const
		{
			return State.SourceFileCount == GetRPGStarterKitGeneratedFileNames().Num() ? ReadyColor() : AttentionColor();
		}

		FText GetMapStatusText() const
		{
			return State.bDemoMapExists ? LOCTEXT("RPGSetupMapReady", "Ready") : LOCTEXT("RPGSetupMapMissing", "Missing");
		}

		FSlateColor GetMapStatusColor() const
		{
			return State.bDemoMapExists ? ReadyColor() : AttentionColor();
		}

		FText GetRuntimeStatusText() const
		{
			return FText::Format(LOCTEXT("RPGSetupRuntimeStatus", "{0}/{1} loaded"), State.RuntimeClassCount, GetRPGStarterKitRuntimeClassNames().Num());
		}

		FSlateColor GetRuntimeStatusColor() const
		{
			return State.RuntimeClassCount == GetRPGStarterKitRuntimeClassNames().Num() ? ReadyColor() : WarningColor();
		}

		FText GetSamplesStatusText() const
		{
			return State.bGeneratedSamplesIndexExists ? LOCTEXT("RPGSetupSamplesReady", "Ready") : LOCTEXT("RPGSetupSamplesMissing", "Not built");
		}

		FSlateColor GetSamplesStatusColor() const
		{
			return State.bGeneratedSamplesIndexExists ? ReadyColor() : WarningColor();
		}

		FText GetLastActionText() const
		{
			return LastAction;
		}

		static FSlateColor ReadyColor()
		{
			return FSlateColor(FLinearColor(0.18f, 0.72f, 0.34f, 1.0f));
		}

		static FSlateColor AttentionColor()
		{
			return FSlateColor(FLinearColor(0.96f, 0.55f, 0.16f, 1.0f));
		}

		static FSlateColor WarningColor()
		{
			return FSlateColor(FLinearColor(0.82f, 0.68f, 0.22f, 1.0f));
		}

		bool CanOpenDemoMap() const
		{
			return State.bDemoMapExists;
		}

		bool CanBrowseAssets() const
		{
			return State.AssetCount > 0;
		}

		bool CanExploreSource() const
		{
			return State.SourceFileCount > 0 && FPaths::DirectoryExists(GetRPGStarterKitSourceDir());
		}

		bool CanOpenManual() const
		{
			return State.bManualExists;
		}

		bool CanOpenGeneratedSamples() const
		{
			return State.bGeneratedSamplesIndexExists;
		}

		bool CanRunAutomation() const
		{
			return State.bDemoMapExists && State.RuntimeClassCount == GetRPGStarterKitRuntimeClassNames().Num();
		}

		void SetLastAction(const FText& Message)
		{
			LastAction = Message;
			if (LastActionText.IsValid())
			{
				LastActionText->SetText(LastAction);
			}
		}

		FReply OnCreateClicked()
		{
			FCodeForgeAssetEditor::CreateExampleBlueprints();
			RefreshStatus();
			SetLastAction(LOCTEXT("RPGSetupCreateDone", "Starter Kit refreshed. Build the project if the compiled runtime status is still incomplete."));
			return FReply::Handled();
		}

		FReply OnOpenMapClicked()
		{
			FString MapFilename;
			if (!FPackageName::DoesPackageExist(GetRPGStarterKitDemoMapPackagePath(), &MapFilename))
			{
				SetLastAction(LOCTEXT("RPGSetupMapStillMissing", "The demo map is missing. Create or refresh the Starter Kit first."));
				return FReply::Handled();
			}

			const bool bLoaded = FEditorFileUtils::LoadMap(MapFilename, false, true);
			RefreshStatus();
			SetLastAction(bLoaded
				? LOCTEXT("RPGSetupMapOpened", "Demo map opened. Press Play to run the RPG loop.")
				: LOCTEXT("RPGSetupMapOpenFailed", "The map could not be opened. Check the Output Log for details."));
			return FReply::Handled();
		}

		FReply OnBrowseAssetsClicked()
		{
			FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			TArray<FString> Folders;
			Folders.Add(GetRPGStarterKitContentPath());
			CBModule.Get().SyncBrowserToFolders(Folders);
			SetLastAction(LOCTEXT("RPGSetupAssetsBrowsed", "Content Browser focused on the RPG Starter Kit assets."));
			return FReply::Handled();
		}

		FReply OnExploreSourceClicked()
		{
			const FString SourceDir = FPaths::ConvertRelativePathToFull(GetRPGStarterKitSourceDir());
			FPlatformProcess::ExploreFolder(*SourceDir);
			SetLastAction(LOCTEXT("RPGSetupSourceOpened", "Project source folder opened."));
			return FReply::Handled();
		}

		FReply OnOpenManualClicked()
		{
			const FString ManualPath = GetRPGStarterKitManualPath();
			if (!FPaths::FileExists(ManualPath))
			{
				SetLastAction(LOCTEXT("RPGSetupManualMissing", "The RPG Starter Kit manual was not found in the plugin Resources/Docs folder."));
				return FReply::Handled();
			}

			FPlatformProcess::LaunchFileInDefaultExternalApplication(*ManualPath);
			SetLastAction(LOCTEXT("RPGSetupManualOpened", "RPG Starter Kit manual opened in the default browser."));
			return FReply::Handled();
		}

		FReply OnOpenGeneratedSamplesClicked()
		{
			const FString SamplesPath = GetRPGStarterKitGeneratedSamplesIndexPath();
			if (!FPaths::FileExists(SamplesPath))
			{
				SetLastAction(LOCTEXT("RPGSetupSamplesStillMissing", "Generated samples are missing. Run Tools/New-CodeForgeGeneratedSamples.ps1 after build/test proof is available."));
				return FReply::Handled();
			}

			FPlatformProcess::LaunchFileInDefaultExternalApplication(*SamplesPath);
			SetLastAction(LOCTEXT("RPGSetupSamplesOpened", "Generated sample bundle opened in the default browser."));
			return FReply::Handled();
		}

		FReply OnRunAutomationClicked()
		{
			if (!GEngine)
			{
				SetLastAction(LOCTEXT("RPGSetupAutomationNoEngine", "Automation test was not started because GEngine is unavailable."));
				return FReply::Handled();
			}

			const bool bQueued = GEngine->Exec(nullptr, TEXT("Automation RunTests CodeForge.RPGStarterKit.PlayableLoop"));
			SetLastAction(bQueued
				? LOCTEXT("RPGSetupAutomationQueued", "Playable-loop automation test queued. Check the Automation or Output Log panels for results.")
				: LOCTEXT("RPGSetupAutomationFailed", "Automation command was not accepted. Check that the automation framework is enabled."));
			return FReply::Handled();
		}

		FReply OnRefreshClicked()
		{
			RefreshStatus();
			SetLastAction(LOCTEXT("RPGSetupStatusRefreshed", "Setup status refreshed."));
			return FReply::Handled();
		}
	};
}

// ---------------------------------------------------------------------------
// Static tab identifiers
// ---------------------------------------------------------------------------

const FName FCodeForgeAssetEditor::GraphTabId(TEXT("CodeForgeGraphTab"));
const FName FCodeForgeAssetEditor::DetailsTabId(TEXT("CodeForgeDetailsTab"));
const FName FCodeForgeAssetEditor::PreviewTabId(TEXT("CodeForgePreviewTab"));

// ---------------------------------------------------------------------------
// RPG Starter Kit setup wizard
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OpenRPGStarterKitSetupWizard()
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	TSharedRef<SWindow> WizardWindow = SNew(SWindow)
		.Title(LOCTEXT("RPGSetupWizardWindowTitle", "CodeForge RPG Starter Kit Setup"))
		.ClientSize(FVector2D(760.0f, 620.0f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	WizardWindow->SetContent(SNew(SCodeForgeRPGSetupWizard));

	const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (ParentWindow.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(WizardWindow, ParentWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(WizardWindow);
	}
}

// ---------------------------------------------------------------------------
// InitEditor
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::InitEditor(
	const EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UCodeForgeBlueprint* InBlueprint)
{
	Blueprint = InBlueprint;
	check(Blueprint);

	CreateEdGraphIfNeeded();
	EdGraph->PopulateFromBlueprint();
	EdGraph->SyncAllNodesFromBlueprint();

	// Set up graph editor commands
	GraphEditorCommands = MakeShared<FUICommandList>();
	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateLambda([this]()
		{
			if (GraphEditorWidget.IsValid())
			{
				const FScopedTransaction Transaction(FText::FromString(TEXT("Delete CodeForge Node")));
				GraphEditorWidget->GetCurrentGraph()->Modify();
				const FGraphPanelSelectionSet SelectedNodes = GraphEditorWidget->GetSelectedNodes();
				for (UObject* NodeObj : SelectedNodes)
				{
					if (UEdGraphNode* Node = Cast<UEdGraphNode>(NodeObj))
					{
						if (Node->CanUserDeleteNode())
						{
							Node->DestroyNode();
						}
					}
				}
			}
		}),
		FCanExecuteAction::CreateLambda([]() { return true; })
	);

	// Define the editor layout: graph (50%), details (25%), preview (25%)
	const TSharedRef<FTabManager::FLayout> Layout =
		FTabManager::NewLayout("CodeForgeAssetEditorLayout_v4")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.5f)
				->AddTab(GraphTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.25f)
				->AddTab(DetailsTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.25f)
				->AddTab(PreviewTabId, ETabState::OpenedTab)
			)
		);

	// Add a toolbar extender for the Generate button
	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda(
			[this](FToolBarBuilder& Builder)
			{
				Builder.AddToolBarButton(
					FUIAction(FExecuteAction::CreateRaw(this, &FCodeForgeAssetEditor::OnGenerateClicked)),
					NAME_None,
					LOCTEXT("GenerateButton", "Generate"),
					LOCTEXT("GenerateTooltip", "Generate C++ code from this CodeForge Blueprint"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Recompile")
				);
				Builder.AddToolBarButton(
					FUIAction(FExecuteAction::CreateLambda([]() { FCodeForgeAssetEditor::CreateExampleBlueprints(); })),
					NAME_None,
					LOCTEXT("ExamplesButton", "Examples"),
					LOCTEXT("ExamplesTooltip", "Create example CodeForge blueprints in /Game/CodeForgeExamples/"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.ContentBrowser")
				);
				Builder.AddToolBarButton(
					FUIAction(FExecuteAction::CreateStatic(&FCodeForgeAssetEditor::OpenRPGStarterKitSetupWizard)),
					NAME_None,
					LOCTEXT("SetupWizardButton", "Setup"),
					LOCTEXT("SetupWizardTooltip", "Open the guided RPG Starter Kit setup wizard"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings")
				);
				Builder.AddToolBarButton(
					FUIAction(
						FExecuteAction::CreateSP(this, &FCodeForgeAssetEditor::HandleToggleAdvancedSpecifiers),
						FCanExecuteAction(),
						FIsActionChecked::CreateSP(this, &FCodeForgeAssetEditor::IsAdvancedSpecifiersToggled)
					),
					NAME_None,
					LOCTEXT("AdvancedToggle", "Advanced"),
					LOCTEXT("AdvancedToggleTooltip", "Show advanced UBT specifiers in node details (off keeps the simple set)."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"),
					EUserInterfaceActionType::ToggleButton);
			}
		)
	);
	AddToolbarExtender(ToolbarExtender);

	// Initialize the asset editor toolkit
	InitAssetEditor(
		Mode,
		InitToolkitHost,
		TEXT("CodeForgeAssetEditorApp"),
		Layout,
		/*bCreateDefaultStandaloneMenu=*/ true,
		/*bCreateDefaultToolbar=*/ true,
		InBlueprint);

	RegenerateMenusAndToolbars();
	RefreshPreview();

	// Observe settings changes so the Details panel re-runs Common/Advanced customizations
	// without requiring the user to reopen the asset.
	if (UCodeForgeSettings* Settings = GetMutableDefault<UCodeForgeSettings>())
	{
		SettingsChangedHandle = Settings->OnSettingChanged().AddLambda(
			[this](UObject*, struct FPropertyChangedEvent&) { RefreshDetailsAfterSettingsChange(); });
	}
}

FCodeForgeAssetEditor::~FCodeForgeAssetEditor()
{
	if (SettingsChangedHandle.IsValid())
	{
		if (UCodeForgeSettings* Settings = GetMutableDefault<UCodeForgeSettings>())
		{
			Settings->OnSettingChanged().Remove(SettingsChangedHandle);
		}
		SettingsChangedHandle.Reset();
	}
}

void FCodeForgeAssetEditor::HandleToggleAdvancedSpecifiers()
{
	UCodeForgeSettings* Settings = GetMutableDefault<UCodeForgeSettings>();
	if (!Settings) return;

	Settings->bShowAdvancedSpecifiers = !Settings->bShowAdvancedSpecifiers;
	Settings->SaveConfig();

	RefreshDetailsAfterSettingsChange();
}

bool FCodeForgeAssetEditor::IsAdvancedSpecifiersToggled() const
{
	const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
	return Settings && Settings->bShowAdvancedSpecifiers;
}

void FCodeForgeAssetEditor::RefreshDetailsAfterSettingsChange()
{
	if (DetailsView.IsValid())
	{
		DetailsView->ForceRefresh();
	}
}

FReply FCodeForgeAssetEditor::HandleRetargetSample()
{
	// Open the wizard so the user can repoint the output paths.
	// v0.3 may add a deep link to Step 3 pre-filled with the asset's current settings.
	FCodeForgeWizardSession::BeginInteractive();
	return FReply::Handled();
}

// ---------------------------------------------------------------------------
// CreateEdGraphIfNeeded
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::CreateEdGraphIfNeeded()
{
	// Check if the blueprint already has a graph subobject
	ForEachObjectWithOuter(Blueprint, [this](UObject* Obj)
	{
		if (UCodeForgeEdGraph* ExistingGraph = Cast<UCodeForgeEdGraph>(Obj))
		{
			EdGraph = ExistingGraph;
		}
	});

	if (EdGraph)
	{
		EdGraph->CodeForgeBlueprint = Blueprint;
		return;
	}

	// No existing graph found -- create a new one as a subobject of the blueprint
	EdGraph = NewObject<UCodeForgeEdGraph>(Blueprint, NAME_None, RF_Transactional);
	EdGraph->Schema = UCodeForgeEdGraphSchema::StaticClass();
	EdGraph->CodeForgeBlueprint = Blueprint;
	Blueprint->MarkPackageDirty();
}

// ---------------------------------------------------------------------------
// FAssetEditorToolkit interface
// ---------------------------------------------------------------------------

FName FCodeForgeAssetEditor::GetToolkitFName() const
{
	return FName(TEXT("CodeForgeAssetEditor"));
}

FText FCodeForgeAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "CodeForge Editor");
}

FString FCodeForgeAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("CodeForge ");
}

FLinearColor FCodeForgeAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.47f, 0.84f, 1.0f);
}

// ---------------------------------------------------------------------------
// Tab spawners
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("WorkspaceMenu", "CodeForge Editor"));

	InTabManager->RegisterTabSpawner(
		GraphTabId,
		FOnSpawnTab::CreateRaw(this, &FCodeForgeAssetEditor::SpawnGraphTab))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(
		DetailsTabId,
		FOnSpawnTab::CreateRaw(this, &FCodeForgeAssetEditor::SpawnDetailsTab))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(
		PreviewTabId,
		FOnSpawnTab::CreateRaw(this, &FCodeForgeAssetEditor::SpawnPreviewTab))
		.SetDisplayName(LOCTEXT("PreviewTab", "Preview"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FCodeForgeAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	InTabManager->UnregisterTabSpawner(GraphTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(PreviewTabId);

	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

// ---------------------------------------------------------------------------
// SaveAsset_Execute — sync graph nodes to blueprint before saving
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::SaveAsset_Execute()
{
	if (EdGraph)
	{
		EdGraph->SyncAllNodesToBlueprint();
	}
	FAssetEditorToolkit::SaveAsset_Execute();
}

// ---------------------------------------------------------------------------
// SpawnGraphTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
	const FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	const FLinearColor BrandSecondary = CFSettings->BrandSecondaryColor;

	SGraphEditor::FGraphEditorEvents Events;
	Events.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateRaw(
		this, &FCodeForgeAssetEditor::OnGraphSelectionChanged);

	GraphEditorWidget = SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.GraphToEdit(EdGraph)
		.GraphEvents(Events)
		.IsEditable(true);

	return SNew(SDockTab)
		.Label(LOCTEXT("GraphTabLabel", "Graph"))
		.TabRole(ETabRole::DocumentTab)
		.TabColorScale(BrandAccent)
		[
			SNew(SVerticalBox)

			// ── Branded toolbar strip ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BrandSecondary)
				.Padding(FMargin(8.f, 5.f))
				[
					SNew(SHorizontalBox)

					// Brand label
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(0.f, 0.f, 8.f, 0.f))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GraphBrandLabel", "CODEFORGE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(BrandAccent)
					]

					// Separator
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(FMargin(0.f, 0.f, 8.f, 0.f))
					[
						SNew(SBox)
						.WidthOverride(1.f)
						.HeightOverride(16.f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(BrandAccent * 0.3f)
						]
					]

					// Search box
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(SearchBox, SSearchBox)
						.OnTextChanged(FOnTextChanged::CreateRaw(this, &FCodeForgeAssetEditor::OnSearchTextChanged))
						.HintText(LOCTEXT("SearchHint", "Search nodes..."))
					]

					// Auto layout button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.f, 0.f, 0.f, 0.f))
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("AutoLayout", "Auto Layout"))
						.OnClicked(FOnClicked::CreateRaw(this, &FCodeForgeAssetEditor::OnAutoLayoutClicked))
					]
				]
			]

			// ── Accent line ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent)
				]
			]

			// ── Sample retargeting hint (visible only for plugin samples) ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4)
			[
				SNew(SBorder)
				.Padding(8)
				.Visibility((Blueprint && Blueprint->GetPathName().StartsWith(TEXT("/CodeForge/Samples/")))
					? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("SampleHint",
							"This is a CodeForge sample. To generate it into your project, click 'Retarget module' to repoint the output paths."))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(8.f, 0.f, 0.f, 0.f))
					[
						SNew(SButton)
						.Text(LOCTEXT("RetargetModule", "Retarget module"))
						.OnClicked(this, &FCodeForgeAssetEditor::HandleRetargetSample)
					]
				]
			]

			// ── Graph editor with branded watermark ──
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SOverlay)

				// Layer 0: the graph editor
				+ SOverlay::Slot()
				[
					GraphEditorWidget.ToSharedRef()
				]

				// Layer 1: bottom-right watermark (non-interactive)
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(0.f, 0.f, 20.f, 16.f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WatermarkBrand", "CODEFORGE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(BrandAccent * FLinearColor(1.f, 1.f, 1.f, 0.06f))
						.Visibility(EVisibility::HitTestInvisible)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WatermarkSub", "Visual C++ Blueprint Designer"))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
						.ColorAndOpacity(FLinearColor(0.5f, 0.6f, 0.7f, 0.04f))
						.Visibility(EVisibility::HitTestInvisible)
					]
				]

				// Layer 2: top-left version badge (non-interactive)
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.Padding(FMargin(12.f, 8.f, 0.f, 0.f))
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.25f))
					.Padding(FMargin(6.f, 2.f))
					.Visibility(EVisibility::HitTestInvisible)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("VersionBadge", "CF v1.0"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
						.ColorAndOpacity(BrandAccent * FLinearColor(1.f, 1.f, 1.f, 0.35f))
					]
				]
			]
		];
}

// ---------------------------------------------------------------------------
// SpawnDetailsTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
	const FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	const FLinearColor BrandSecondary = CFSettings->BrandSecondaryColor;

	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bHideSelectionTip = true;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	if (Blueprint)
	{
		DetailsView->SetObject(Blueprint);
	}

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTabLabel", "Details"))
		.TabColorScale(BrandAccent)
		[
			SNew(SVerticalBox)

			// ── Header bar ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BrandSecondary)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DetailsHeaderTitle", "NODE PROPERTIES"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(BrandAccent)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNullWidget::NullWidget
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							if (!Blueprint) return FText::GetEmpty();
							return FText::FromString(Blueprint->ClassName);
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
						.ColorAndOpacity(FLinearColor(0.5f, 0.6f, 0.7f))
					]
				]
			]

			// ── Accent line ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent)
				]
			]

			// ── Details view ──
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				DetailsView.ToSharedRef()
			]
		];
}

// ---------------------------------------------------------------------------
// SpawnPreviewTab
// ---------------------------------------------------------------------------

TSharedRef<SDockTab> FCodeForgeAssetEditor::SpawnPreviewTab(const FSpawnTabArgs& Args)
{
	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
	const FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	const FLinearColor BrandSecondary = CFSettings->BrandSecondaryColor;
	const FLinearColor CodeBg(0.04f, 0.05f, 0.08f);

	// Section header builder
	auto MakeSectionHeader = [&](const FText& Label, const FText& FileExt) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BrandSecondary * 0.8f)
			.Padding(FMargin(10.f, 5.f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					.ColorAndOpacity(BrandAccent)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNullWidget::NullWidget
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FileExt)
					.Font(FCoreStyle::GetDefaultFontStyle("Mono", 8))
					.ColorAndOpacity(FLinearColor(0.4f, 0.5f, 0.6f))
				]
			];
	};

	return SNew(SDockTab)
		.Label(LOCTEXT("PreviewTabLabel", "Code Preview"))
		.TabColorScale(BrandAccent)
		[
			SNew(SVerticalBox)

			// ── Top header bar ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BrandSecondary)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PreviewHeaderTitle", "GENERATED CODE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(BrandAccent)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNullWidget::NullWidget
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PreviewLiveLabel", "LIVE"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
						.ColorAndOpacity(FLinearColor(0.2f, 0.9f, 0.4f))
					]
				]
			]

			// ── Accent line ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent)
				]
			]

			// ── Header (.h) section ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MakeSectionHeader(
					LOCTEXT("HeaderSectionLabel", "HEADER"),
					LOCTEXT("HeaderExt", ".h"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(CodeBg)
				.Padding(FMargin(4.f, 2.f))
				[
					SAssignNew(PreviewHeaderText, SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
					.ForegroundColor(FLinearColor(0.78f, 0.85f, 0.95f))
				]
			]

			// ── Divider ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(BrandAccent * 0.25f)
				]
			]

			// ── Source (.cpp) section ──
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MakeSectionHeader(
					LOCTEXT("SourceSectionLabel", "SOURCE"),
					LOCTEXT("SourceExt", ".cpp"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(CodeBg)
				.Padding(FMargin(4.f, 2.f))
				[
					SAssignNew(PreviewSourceText, SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
					.ForegroundColor(FLinearColor(0.78f, 0.85f, 0.95f))
				]
			]
		];
}

// ---------------------------------------------------------------------------
// RefreshPreview
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::RefreshPreview()
{
	if (!Blueprint || !EdGraph) return;

	EdGraph->SyncAllNodesToBlueprint();

	FCodeForgeGenerator Generator;
	FCodeForgeGeneratorResult Result = Generator.Generate(Blueprint);

	if (PreviewHeaderText.IsValid())
	{
		PreviewHeaderText->SetText(Result.bSuccess
			? FText::FromString(Result.HeaderContent)
			: FText::FromString(FString::Printf(TEXT("// Error: %s"), *Result.ErrorMessage)));
	}

	if (PreviewSourceText.IsValid())
	{
		PreviewSourceText->SetText(Result.bSuccess
			? FText::FromString(Result.SourceContent)
			: FText::GetEmpty());
	}
}

// ---------------------------------------------------------------------------
// OnGraphSelectionChanged
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OnGraphSelectionChanged(const TSet<UObject*>& SelectedNodes)
{
	if (DetailsView.IsValid())
	{
		if (SelectedNodes.Num() == 1)
		{
			for (UObject* Node : SelectedNodes)
			{
				DetailsView->SetObject(Node);
			}
		}
		else if (SelectedNodes.Num() == 0 && Blueprint)
		{
			// Nothing selected -- show the blueprint itself
			DetailsView->SetObject(Blueprint);
		}
		else
		{
			DetailsView->SetObject(nullptr);
		}
	}

	if (EdGraph)
	{
		EdGraph->RunValidationOnNodes();
	}

	RefreshPreview();
}

// ---------------------------------------------------------------------------
// OnGenerateClicked
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OnGenerateClicked()
{
	if (!Blueprint || !EdGraph)
	{
		return;
	}

	// 1. Sync graph nodes to the blueprint data
	{
		const FScopedTransaction Transaction(FText::FromString(TEXT("Sync Graph to Blueprint")));
		Blueprint->Modify();
		EdGraph->SyncAllNodesToBlueprint();
	}

	// 2. Run the code generator
	FCodeForgeGenerator Generator;
	FCodeForgeGeneratorResult Result = Generator.GenerateAndIntegrate(
		Blueprint, FPaths::ProjectDir());

	if (!Result.bSuccess)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("GenerateError", "Code generation failed:\n{0}"),
				FText::FromString(Result.ErrorMessage)));
		return;
	}

	// 3. Handle structural vs behavioral changes
	if (Result.bIsStructuralChange)
	{
		EAppReturnType::Type UserResponse = FMessageDialog::Open(
			EAppMsgType::YesNo,
			LOCTEXT("StructuralChange",
				"Code generation succeeded with structural changes.\n"
				"A hot reload or editor restart is required.\n\n"
				"Would you like to attempt Live Coding now?"));

		if (UserResponse == EAppReturnType::Yes)
		{
			IModuleInterface* LiveCodingModule =
				FModuleManager::Get().GetModule(TEXT("LiveCoding"));
			if (LiveCodingModule && GEngine)
			{
				GEngine->Exec(nullptr, TEXT("LiveCoding.Compile"));
			}
			else
			{
				FMessageDialog::Open(
					EAppMsgType::Ok,
					LOCTEXT("NoLiveCoding",
						"Live Coding is not available. Please restart the editor to pick up structural changes."));
			}
		}
	}
	else
	{
		// Behavioral change only -- try Live Coding automatically
		IModuleInterface* LiveCodingModule =
			FModuleManager::Get().GetModule(TEXT("LiveCoding"));
		if (LiveCodingModule && GEngine)
		{
			GEngine->Exec(nullptr, TEXT("LiveCoding.Compile"));
		}

		FMessageDialog::Open(
			EAppMsgType::Ok,
			LOCTEXT("GenerateSuccess", "Code generation succeeded."));
	}
}

// ---------------------------------------------------------------------------
// OnSearchTextChanged
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::OnSearchTextChanged(const FText& NewText)
{
	FString SearchText = NewText.ToString();

	if (!EdGraph) return;

	for (UEdGraphNode* Node : EdGraph->Nodes)
	{
		UCodeForgeEdGraphNode* CFNode = Cast<UCodeForgeEdGraphNode>(Node);
		if (!CFNode) continue;

		FString Title = CFNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString();
		bool bMatch = SearchText.IsEmpty() || Title.Contains(SearchText, ESearchCase::IgnoreCase);

		CFNode->bCommentBubbleMakeVisible = !bMatch && !SearchText.IsEmpty();
	}

	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->NotifyGraphChanged();
	}
}

// ---------------------------------------------------------------------------
// OnAutoLayoutClicked
// ---------------------------------------------------------------------------

FReply FCodeForgeAssetEditor::OnAutoLayoutClicked()
{
	if (!EdGraph) return FReply::Handled();

	const FScopedTransaction Transaction(FText::FromString(TEXT("Auto Layout Nodes")));
	EdGraph->Modify();

	TArray<UEdGraphNode*> TypeNodes;
	TArray<UEdGraphNode*> MemberNodes;

	for (UEdGraphNode* Node : EdGraph->Nodes)
	{
		if (Node->IsA<UCodeForgeNode_Class>() || Node->IsA<UCodeForgeNode_Struct>()
			|| Node->IsA<UCodeForgeNode_Enum>() || Node->IsA<UCodeForgeNode_Interface>())
		{
			TypeNodes.Add(Node);
		}
		else
		{
			MemberNodes.Add(Node);
		}
	}

	int32 YOffset = 0;
	const int32 NodeSpacingY = 200;
	const int32 ColumnSpacing = 400;

	for (UEdGraphNode* Node : TypeNodes)
	{
		Node->NodePosX = 0;
		Node->NodePosY = YOffset;
		YOffset += NodeSpacingY;
	}

	YOffset = 0;
	for (UEdGraphNode* Node : MemberNodes)
	{
		Node->NodePosX = ColumnSpacing;
		Node->NodePosY = YOffset;
		YOffset += 150;
	}

	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->NotifyGraphChanged();
	}

	return FReply::Handled();
}

// ---------------------------------------------------------------------------
// CreateExampleBlueprints
// ---------------------------------------------------------------------------

void FCodeForgeAssetEditor::CreateExampleBlueprints()
{
	const FString BaseDir = TEXT("/Game/CodeForgeExamples/RPGStarterKit/");
	const FString ModuleName = FApp::GetProjectName();

	// Names of all example assets we create
	const TArray<FString>& ExampleNames = GetRPGStarterKitAssetNames();

	// Clean up existing example packages to ensure fresh state and thumbnails
	for (const FString& Name : ExampleNames)
	{
		FString PkgPath = BaseDir + Name;
		UPackage* ExistingPkg = FindPackage(nullptr, *PkgPath);
		if (!ExistingPkg && FPackageName::DoesPackageExist(PkgPath))
		{
			ExistingPkg = LoadPackage(nullptr, *PkgPath, LOAD_NoWarn);
		}
		
		if (ExistingPkg)
		{
			ExistingPkg->FullyLoad();
			UCodeForgeBlueprint* OldBP = FindObject<UCodeForgeBlueprint>(ExistingPkg, *Name);
			if (OldBP)
			{
				OldBP->ClearFlags(RF_Public | RF_Standalone);
				OldBP->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
			}
		}
	}

	// Helper: create, register, and save a blueprint asset.
	// Renders the custom thumbnail and embeds it in the package so the
	// Content Browser shows it immediately — no lazy loading required.
	TArray<UCodeForgeBlueprint*> CreatedBPs;
	auto SaveBlueprint = [&CreatedBPs](UPackage* Package, UCodeForgeBlueprint* BP)
	{
		// Render thumbnail via the registered UCodeForgeThumbnailRenderer
		// and embed it in the package before saving.
		FObjectThumbnail Thumbnail;
		ThumbnailTools::RenderThumbnail(
			BP, 256, 256,
			ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
			nullptr, &Thumbnail);
		if (!Thumbnail.IsEmpty())
		{
			ThumbnailTools::CacheThumbnail(BP->GetFullName(), &Thumbnail, Package);
		}

		FAssetRegistryModule::AssetCreated(BP);
		BP->MarkPackageDirty();
		FString FilePath = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		if (!UPackage::SavePackage(Package, BP, *FilePath, SaveArgs))
		{
			UE_LOG(LogTemp, Warning, TEXT("CodeForge: Failed to save example blueprint %s"), *Package->GetName());
		}
		CreatedBPs.Add(BP);
	};

	// ── 1. EItemRarity (Enum) ──
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("EItemRarity")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("EItemRarity"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("EItemRarity");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Enum;
		BP->bBlueprintType = true;
		BP->ModuleTarget = ModuleName;

		auto AddEntry = [&](const FString& Name)
		{
			FCodeForgeEnumEntryDef Entry;
			Entry.Name = Name;
			Entry.DisplayName = Name;
			BP->EnumEntries.Add(Entry);
		};
		AddEntry(TEXT("Common"));
		AddEntry(TEXT("Uncommon"));
		AddEntry(TEXT("Rare"));
		AddEntry(TEXT("Epic"));
		AddEntry(TEXT("Legendary"));

		SaveBlueprint(Pkg, BP);
	}

	// ── 2. RPGItemData (Struct) ──
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("RPGItemData")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("RPGItemData"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("FRPGItemData");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Struct;
		BP->bBlueprintType = true;
		BP->ModuleTarget = ModuleName;

		auto AddProp = [&](const FString& Name, const FString& Type,
			const FString& Default = TEXT(""), const FString& IncPath = TEXT(""))
		{
			FCodeForgePropertyDef P;
			P.Name = Name;
			P.Type = Type;
			P.DefaultValue = Default;
			P.IncludePath = IncPath;
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->StructProperties.Add(P);
		};

		AddProp(TEXT("ItemName"),       TEXT("FString"));
		AddProp(TEXT("ItemDescription"), TEXT("FText"));
		AddProp(TEXT("Rarity"),         TEXT("EItemRarity"), TEXT(""), TEXT("EItemRarity.h"));
		AddProp(TEXT("StackCount"),     TEXT("int32"),       TEXT("1"));
		AddProp(TEXT("MaxStackSize"),   TEXT("int32"),       TEXT("99"));
		AddProp(TEXT("bIsConsumable"),  TEXT("bool"));
		AddProp(TEXT("ItemWeight"),     TEXT("float"),       TEXT("1.0f"));

		SaveBlueprint(Pkg, BP);
	}

	// ── 3. IDamageable (Interface) ──
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("IDamageable")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("IDamageable"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("Damageable");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Interface;
		BP->ModuleTarget = ModuleName;

		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("ReceiveDamage");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			FCodeForgeParamDef P;
			P.Name = TEXT("Amount");
			P.Type = TEXT("float");
			F.Parameters.Add(P);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetCurrentHealth");
			F.ReturnType = TEXT("float");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			F.bConst = true;
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetIsAlive");
			F.ReturnType = TEXT("bool");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			F.bConst = true;
			BP->Functions.Add(F);
		}

		SaveBlueprint(Pkg, BP);
	}

	// -- 4. IInteractable (Interface) --
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("IInteractable")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("IInteractable"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("Interactable");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Interface;
		BP->ModuleTarget = ModuleName;

		FCodeForgeFunctionDef F;
		F.Name = TEXT("Interact");
		F.ReturnType = TEXT("void");
		F.bBlueprintCallable = false;
		F.bBlueprintNativeEvent = true;
		F.IncludePath = TEXT("ARPGCharacter.h");
		FCodeForgeParamDef P;
		P.Name = TEXT("Interactor");
		P.Type = TEXT("ARPGCharacter*");
		F.Parameters.Add(P);
		BP->Functions.Add(F);

		SaveBlueprint(Pkg, BP);
	}

	// ── 5. RPGCharacter (Class — Character, Replicated) ──
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("RPGCharacter")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("RPGCharacter"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("ARPGCharacter");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::Character;
		BP->bReplicated = true;
		BP->ModuleTarget = ModuleName;

		// ─ Interfaces ─
		BP->Interfaces.Add(TEXT("Damageable"));

		// ─ Constructor Body ─
		BP->ConstructorBody = TEXT(
			"\tMaxHealth = 100.0f;\n"
			"\tHealth = MaxHealth;\n"
			"\tAttackPower = 10.0f;\n"
			"\tLevel = 1;\n"
			"\tExperience = 0;\n"
			"\tInventoryComponent = CreateDefaultSubobject<URPGInventoryComponent>(TEXT(\"Inventory\"));"
		);

		// ─ Properties ─
		auto AddProp = [&](const FString& Name, const FString& Type,
			const FString& Default = TEXT(""),
			bool bRep = false, bool bRepNotify = false,
			ECodeForgeRepCondition RepCond = ECodeForgeRepCondition::None,
			const FString& IncPath = TEXT(""),
			const FString& OnRepBodyStr = TEXT(""))
		{
			FCodeForgePropertyDef P;
			P.Name = Name;
			P.Type = Type;
			P.DefaultValue = Default;
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			P.bReplicated = bRep;
			P.bRepNotify = bRepNotify;
			P.ReplicationCondition = RepCond;
			P.IncludePath = IncPath;
			P.OnRepBody = OnRepBodyStr;
			BP->Properties.Add(P);
		};

		AddProp(TEXT("Health"),        TEXT("float"), TEXT("100.0f"), true, true,
			ECodeForgeRepCondition::None, TEXT(""), TEXT("\tOnHealthChanged.Broadcast(Health, MaxHealth);"));
		AddProp(TEXT("MaxHealth"),     TEXT("float"), TEXT("100.0f"));
		AddProp(TEXT("AttackPower"),   TEXT("float"), TEXT("10.0f"));
		AddProp(TEXT("Level"),         TEXT("int32"), TEXT("1"));
		AddProp(TEXT("Experience"),    TEXT("int32"), TEXT("0"));

		// InventoryComponent — VisibleAnywhere, BlueprintReadOnly
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("InventoryComponent");
			P.Type = TEXT("URPGInventoryComponent*");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("URPGInventoryComponent.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("CameraBoom");
			P.Type = TEXT("USpringArmComponent*");
			P.Category = TEXT("RPG|Camera");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("GameFramework/SpringArmComponent.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("FollowCamera");
			P.Type = TEXT("UCameraComponent*");
			P.Category = TEXT("RPG|Camera");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("Camera/CameraComponent.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("DemoVisualMesh");
			P.Type = TEXT("UStaticMeshComponent*");
			P.Category = TEXT("RPG|Demo");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("Components/StaticMeshComponent.h");
			BP->Properties.Add(P);
		}

		// ─ Functions ─
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GainExperience");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = true;
			FCodeForgeParamDef P;
			P.Name = TEXT("Amount");
			P.Type = TEXT("int32");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tExperience += Amount;\n"
				"\tconst int32 XPThreshold = Level * 100;\n"
				"\tif (Experience >= XPThreshold)\n"
				"\t{\n"
				"\t\tExperience -= XPThreshold;\n"
				"\t\tLevel++;\n"
				"\t\tOnLevelUp.Broadcast(Level);\n"
				"\t}"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetHealthPercent");
			F.ReturnType = TEXT("float");
			F.bBlueprintCallable = true;
			F.bBlueprintPure = true;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn (MaxHealth > 0.f) ? Health / MaxHealth : 0.f;");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("IsAlive");
			F.ReturnType = TEXT("bool");
			F.bBlueprintCallable = true;
			F.bBlueprintPure = true;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn Health > 0.f;");
			BP->Functions.Add(F);
		}
		// ─ Interface implementation functions (NativeEvent) ─
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("ReceiveDamage");
			F.ReturnType = TEXT("void");
			F.bBlueprintNativeEvent = true;
			F.bBlueprintCallable = false;
			FCodeForgeParamDef P;
			P.Name = TEXT("Amount");
			P.Type = TEXT("float");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tif (!IsAlive()) return;\n"
				"\tHealth = FMath::Clamp(Health - Amount, 0.f, MaxHealth);\n"
				"\tOnHealthChanged.Broadcast(Health, MaxHealth);\n"
				"\tif (!IsAlive())\n"
				"\t{\n"
				"\t\tOnDeath.Broadcast();\n"
				"\t}\n"
				"\tClientReceiveDamage(Amount);\n"
				"\tNetMulticastPlayHitEffect();"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetCurrentHealth");
			F.ReturnType = TEXT("float");
			F.bBlueprintNativeEvent = true;
			F.bBlueprintCallable = false;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn Health;");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetIsAlive");
			F.ReturnType = TEXT("bool");
			F.bBlueprintNativeEvent = true;
			F.bBlueprintCallable = false;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn IsAlive();");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("Attack");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = true;
			F.Category = TEXT("RPG|Combat");
			F.FunctionBody = TEXT("\tServerRequestAttack();");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("Interact");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = true;
			F.Category = TEXT("RPG|Interact");
			F.FunctionBody = TEXT("\t// Overlap nearby interactables and call IInteractable::Interact.");
			BP->Functions.Add(F);
		}

		// ─ Exec functions (console commands) ─
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("CheatAttack");
			F.ReturnType = TEXT("void");
			F.bExec = true;
			F.bBlueprintCallable = false;
			F.FunctionBody = TEXT("\tServerRequestAttack();");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("CheatGainXP");
			F.ReturnType = TEXT("void");
			F.bExec = true;
			F.bBlueprintCallable = false;
			FCodeForgeParamDef P;
			P.Name = TEXT("Amount");
			P.Type = TEXT("int32");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT("\tGainExperience(Amount);");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("CheatAddItem");
			F.ReturnType = TEXT("void");
			F.bExec = true;
			F.bBlueprintCallable = false;
			F.FunctionBody = TEXT(
				"\tif (!InventoryComponent) return;\n"
				"\tFRPGItemData TestItem;\n"
				"\tTestItem.ItemName = TEXT(\"Test Sword\");\n"
				"\tTestItem.ItemWeight = 5.0f;\n"
				"\tTestItem.StackCount = 1;\n"
				"\tTestItem.MaxStackSize = 1;\n"
				"\tTestItem.Rarity = EItemRarity::Rare;\n"
				"\tbool bAdded = InventoryComponent->AddItem(TestItem);\n"
				"\tUE_LOG(LogTemp, Log, TEXT(\"AddItem: %s (count: %d, weight: %.1f)\"),\n"
				"\t\tbAdded ? TEXT(\"success\") : TEXT(\"failed\"),\n"
				"\t\tInventoryComponent->GetItemCount(), InventoryComponent->CurrentWeight);"
			);
			BP->Functions.Add(F);
		}

		// ─ RPCs ─
		{
			FCodeForgeRPCDef R;
			R.Name = TEXT("ServerRequestAttack");
			R.Mode = ECodeForgeRPCMode::Server;
			R.bReliable = true;
			R.FunctionBody = TEXT(
				"\tFHitResult Hit;\n"
				"\tFVector Start = GetActorLocation();\n"
				"\tFVector End = Start + GetActorForwardVector() * 300.f;\n"
				"\tFCollisionQueryParams Params;\n"
				"\tParams.AddIgnoredActor(this);\n"
				"\tif (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))\n"
				"\t{\n"
				"\t\tif (AActor* Target = Hit.GetActor())\n"
				"\t\t{\n"
				"\t\t\tif (Target->Implements<UDamageable>())\n"
				"\t\t\t{\n"
				"\t\t\t\tIDamageable::Execute_ReceiveDamage(Target, AttackPower);\n"
				"\t\t\t}\n"
				"\t\t}\n"
				"\t}"
			);
			BP->RPCs.Add(R);
		}
		{
			FCodeForgeRPCDef R;
			R.Name = TEXT("ClientReceiveDamage");
			R.Mode = ECodeForgeRPCMode::Client;
			R.bReliable = true;
			R.FunctionBody = TEXT(
				"\t// Client-side feedback: screen shake, UI flash, etc.\n"
				"\tUE_LOG(LogTemp, Log, TEXT(\"%s received %.1f damage (client feedback)\"), *GetName(), Amount);"
			);
			FCodeForgeParamDef P;
			P.Name = TEXT("Amount");
			P.Type = TEXT("float");
			R.Parameters.Add(P);
			BP->RPCs.Add(R);
		}
		{
			FCodeForgeRPCDef R;
			R.Name = TEXT("NetMulticastPlayHitEffect");
			R.Mode = ECodeForgeRPCMode::NetMulticast;
			R.bReliable = false;
			R.FunctionBody = TEXT(
				"\t// Spawn VFX/SFX at actor location\n"
				"\tUE_LOG(LogTemp, Log, TEXT(\"%s: hit effect played\"), *GetName());"
			);
			BP->RPCs.Add(R);
		}
		{
			FCodeForgeRPCDef R;
			R.Name = TEXT("Server_PickUpItem");
			R.Mode = ECodeForgeRPCMode::Server;
			R.bReliable = true;
			FCodeForgeParamDef P;
			P.Name = TEXT("Item");
			P.Type = TEXT("ARPGItemPickup*");
			R.Parameters.Add(P);
			R.FunctionBody = TEXT(
				"\tif (Item)\n"
				"\t{\n"
				"\t\tItem->TryGiveItemTo(this);\n"
				"\t}"
			);
			BP->RPCs.Add(R);
		}

		// ─ Delegates (names WITHOUT F prefix) ─
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnHealthChanged");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			FCodeForgeParamDef P1; P1.Name = TEXT("NewHealth"); P1.Type = TEXT("float");
			FCodeForgeParamDef P2; P2.Name = TEXT("MaxHealth"); P2.Type = TEXT("float");
			D.Parameters.Add(P1);
			D.Parameters.Add(P2);
			BP->Delegates.Add(D);
		}
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnLevelUp");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			FCodeForgeParamDef P; P.Name = TEXT("NewLevel"); P.Type = TEXT("int32");
			D.Parameters.Add(P);
			BP->Delegates.Add(D);
		}
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnDeath");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			BP->Delegates.Add(D);
		}

		SaveBlueprint(Pkg, BP);
	}

	// ── 5. RPGInventoryComponent (Class — ActorComponent) ──
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("RPGInventoryComponent")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("RPGInventoryComponent"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("URPGInventoryComponent");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::ActorComponent;
		BP->bReplicated = false;
		BP->ModuleTarget = ModuleName;

		// ─ Properties ─
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("Items");
			P.Type = TEXT("TArray<FRPGItemData>");
			P.IncludePath = TEXT("FRPGItemData.h");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("MaxSlots");
			P.Type = TEXT("int32");
			P.DefaultValue = TEXT("20");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("CurrentWeight");
			P.Type = TEXT("float");
			P.bBlueprintReadOnly = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("MaxWeight");
			P.Type = TEXT("float");
			P.DefaultValue = TEXT("100.0f");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}

		// ─ Functions ─
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("AddItem");
			F.ReturnType = TEXT("bool");
			F.bBlueprintCallable = true;
			FCodeForgeParamDef P;
			P.Name = TEXT("Item");
			P.Type = TEXT("FRPGItemData");
			P.bIsConst = true;
			P.bIsRef = true;
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tif (Items.Num() >= MaxSlots) return false;\n"
				"\tfloat NewWeight = CurrentWeight + Item.ItemWeight * Item.StackCount;\n"
				"\tif (NewWeight > MaxWeight) return false;\n"
				"\tItems.Add(Item);\n"
				"\tCurrentWeight = NewWeight;\n"
				"\tOnItemAdded.Broadcast();\n"
				"\tOnInventoryChanged.Broadcast();\n"
				"\treturn true;"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("RemoveItem");
			F.ReturnType = TEXT("bool");
			F.bBlueprintCallable = true;
			FCodeForgeParamDef P;
			P.Name = TEXT("SlotIndex");
			P.Type = TEXT("int32");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tif (!Items.IsValidIndex(SlotIndex)) return false;\n"
				"\tCurrentWeight -= Items[SlotIndex].ItemWeight * Items[SlotIndex].StackCount;\n"
				"\tCurrentWeight = FMath::Max(0.f, CurrentWeight);\n"
				"\tItems.RemoveAt(SlotIndex);\n"
				"\tOnInventoryChanged.Broadcast();\n"
				"\treturn true;"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetItemCount");
			F.ReturnType = TEXT("int32");
			F.bBlueprintCallable = true;
			F.bBlueprintPure = true;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn Items.Num();");
			BP->Functions.Add(F);
		}

		// ─ Delegates (names WITHOUT F prefix) ─
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnInventoryChanged");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			BP->Delegates.Add(D);
		}
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnItemAdded");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			BP->Delegates.Add(D);
		}

		SaveBlueprint(Pkg, BP);
	}

	// -- 7. RPGEnemy (Class - Actor, Replicated) --
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("RPGEnemy")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("RPGEnemy"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("ARPGEnemy");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::Actor;
		BP->bReplicated = true;
		BP->ModuleTarget = ModuleName;
		BP->Interfaces.Add(TEXT("Damageable"));

		BP->ConstructorBody = TEXT(
			"\tVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT(\"VisualMesh\"));\n"
			"\tSetRootComponent(VisualMesh);\n"
			"\tVisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);\n"
			"\tVisualMesh->SetCollisionObjectType(ECC_Pawn);\n"
			"\tVisualMesh->SetCollisionResponseToAllChannels(ECR_Block);\n"
			"\tMaxHealth = 30.0f;\n"
			"\tHealth = MaxHealth;\n"
			"\tExperienceReward = 25;"
		);

		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("VisualMesh");
			P.Type = TEXT("UStaticMeshComponent*");
			P.Category = TEXT("RPG|Demo");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("Components/StaticMeshComponent.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("Health");
			P.Type = TEXT("float");
			P.Category = TEXT("RPG|Combat");
			P.DefaultValue = TEXT("30.0f");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("MaxHealth");
			P.Type = TEXT("float");
			P.Category = TEXT("RPG|Combat");
			P.DefaultValue = TEXT("30.0f");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("ExperienceReward");
			P.Type = TEXT("int32");
			P.Category = TEXT("RPG|Rewards");
			P.DefaultValue = TEXT("25");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}

		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("ReceiveDamage");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			FCodeForgeParamDef P;
			P.Name = TEXT("Amount");
			P.Type = TEXT("float");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tif (!GetIsAlive_Implementation()) return;\n"
				"\tHealth = FMath::Clamp(Health - Amount, 0.0f, MaxHealth);\n"
				"\tif (!GetIsAlive_Implementation())\n"
				"\t{\n"
				"\t\tOnEnemyDied.Broadcast();\n"
				"\t\tSetActorEnableCollision(false);\n"
				"\t\tSetLifeSpan(1.0f);\n"
				"\t\tif (VisualMesh)\n"
				"\t\t{\n"
				"\t\t\tVisualMesh->SetVisibility(false, true);\n"
				"\t\t}\n"
				"\t}"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetCurrentHealth");
			F.ReturnType = TEXT("float");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn Health;");
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("GetIsAlive");
			F.ReturnType = TEXT("bool");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			F.bConst = true;
			F.FunctionBody = TEXT("\treturn Health > 0.0f;");
			BP->Functions.Add(F);
		}

		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnEnemyDied");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			D.Category = TEXT("RPG|Combat");
			BP->Delegates.Add(D);
		}

		SaveBlueprint(Pkg, BP);
	}

	// -- 8. RPGItemPickup (Class - Actor, Replicated, Interactable) --
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("RPGItemPickup")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("RPGItemPickup"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("ARPGItemPickup");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::Actor;
		BP->bReplicated = true;
		BP->ModuleTarget = ModuleName;
		BP->Interfaces.Add(TEXT("Interactable"));

		BP->ConstructorBody = TEXT(
			"\tPickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT(\"PickupCollision\"));\n"
			"\tSetRootComponent(PickupCollision);\n"
			"\tPickupCollision->InitSphereRadius(80.0f);\n"
			"\tPickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);\n"
			"\tPickupCollision->SetCollisionObjectType(ECC_WorldDynamic);\n"
			"\tPickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);\n"
			"\tPickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);\n"
			"\tPickupCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);\n"
			"\tVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT(\"VisualMesh\"));\n"
			"\tVisualMesh->SetupAttachment(PickupCollision);\n"
			"\tVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);\n"
			"\tItemData.ItemName = TEXT(\"Epic Loot\");\n"
			"\tItemData.ItemDescription = FText::FromString(TEXT(\"A legendary item from a CodeForge generated loot box.\"));\n"
			"\tItemData.Rarity = EItemRarity::Epic;\n"
			"\tItemData.StackCount = 1;\n"
			"\tItemData.MaxStackSize = 1;\n"
			"\tItemData.ItemWeight = 2.0f;"
		);

		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("PickupCollision");
			P.Type = TEXT("USphereComponent*");
			P.Category = TEXT("RPG|Pickup");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("Components/SphereComponent.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("VisualMesh");
			P.Type = TEXT("UStaticMeshComponent*");
			P.Category = TEXT("RPG|Pickup");
			P.bVisibleAnywhere = true;
			P.bBlueprintReadOnly = true;
			P.IncludePath = TEXT("Components/StaticMeshComponent.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("ItemData");
			P.Type = TEXT("FRPGItemData");
			P.Category = TEXT("RPG|Pickup");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			P.IncludePath = TEXT("FRPGItemData.h");
			BP->Properties.Add(P);
		}

		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("TryGiveItemTo");
			F.ReturnType = TEXT("bool");
			F.bBlueprintCallable = true;
			F.Category = TEXT("RPG|Pickup");
			F.IncludePath = TEXT("ARPGCharacter.h");
			FCodeForgeParamDef P;
			P.Name = TEXT("Character");
			P.Type = TEXT("ARPGCharacter*");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tif (!Character || !Character->InventoryComponent) return false;\n"
				"\tconst bool bAdded = Character->InventoryComponent->AddItem(ItemData);\n"
				"\tif (!bAdded) return false;\n"
				"\tOnPickedUp.Broadcast(Character);\n"
				"\tDestroy();\n"
				"\treturn true;"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("Interact");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = false;
			F.bBlueprintNativeEvent = true;
			F.IncludePath = TEXT("ARPGCharacter.h");
			FCodeForgeParamDef P;
			P.Name = TEXT("Interactor");
			P.Type = TEXT("ARPGCharacter*");
			F.Parameters.Add(P);
			F.FunctionBody = TEXT(
				"\tUWorld* World = GetWorld();\n"
				"\tif (HasAuthority() || !World || !World->GetNetDriver())\n"
				"\t{\n"
				"\t\tTryGiveItemTo(Interactor);\n"
				"\t}\n"
				"\telse if (Interactor && Interactor->IsLocallyControlled())\n"
				"\t{\n"
				"\t\tInteractor->Server_PickUpItem(this);\n"
				"\t}"
			);
			BP->Functions.Add(F);
		}

		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnPickedUp");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			D.Category = TEXT("RPG|Pickup");
			FCodeForgeParamDef P;
			P.Name = TEXT("Character");
			P.Type = TEXT("ARPGCharacter*");
			D.Parameters.Add(P);
			BP->Delegates.Add(D);
		}

		SaveBlueprint(Pkg, BP);
	}

	// ── 9. RPGGameMode (Class — GameModeBase) ──
	{
		UPackage* Pkg = CreatePackage(*(BaseDir + TEXT("RPGGameMode")));
		UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>(Pkg, TEXT("RPGGameMode"),
			RF_Public | RF_Standalone);
		BP->ClassName = TEXT("ARPGGameMode");
		BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
		BP->ClassType = ECodeForgeClassType::GameModeBase;
		BP->bReplicated = false;
		BP->ModuleTarget = ModuleName;

		// ─ Constructor Body ─
		BP->ConstructorBody = TEXT(
			"\tDefaultPawnClass = ARPGCharacter::StaticClass();\n"
			"\tRespawnDelay = 5.0f;\n"
			"\tbAllowFriendlyFire = false;\n"
			"\tExperienceMultiplier = 1.0f;\n"
			"\tbAutoSpawnDemoLoop = true;\n"
			"\tEnemyClass = ARPGEnemy::StaticClass();\n"
			"\tPickupClass = ARPGItemPickup::StaticClass();\n"
			"\tDemoEnemyCount = 2;\n"
			"\tDemoPickupCount = 2;\n"
			"\tDemoSpawnRadius = 500.0f;"
		);

		// ─ Properties ─
		{
			// IncludePath for ARPGCharacter.h — needed by constructor's StaticClass() reference
			FCodeForgePropertyDef P;
			P.Name = TEXT("RespawnDelay");
			P.Type = TEXT("float");
			P.DefaultValue = TEXT("5.0f");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			P.IncludePath = TEXT("ARPGCharacter.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("bAllowFriendlyFire");
			P.Type = TEXT("bool");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("ExperienceMultiplier");
			P.Type = TEXT("float");
			P.DefaultValue = TEXT("1.0f");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("bAutoSpawnDemoLoop");
			P.Type = TEXT("bool");
			P.Category = TEXT("RPG|Demo");
			P.DefaultValue = TEXT("true");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("EnemyClass");
			P.Type = TEXT("TSubclassOf<ARPGEnemy>");
			P.Category = TEXT("RPG|Demo");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			P.IncludePath = TEXT("ARPGEnemy.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("PickupClass");
			P.Type = TEXT("TSubclassOf<ARPGItemPickup>");
			P.Category = TEXT("RPG|Demo");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			P.IncludePath = TEXT("ARPGItemPickup.h");
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("DemoEnemyCount");
			P.Type = TEXT("int32");
			P.Category = TEXT("RPG|Demo");
			P.DefaultValue = TEXT("2");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("DemoPickupCount");
			P.Type = TEXT("int32");
			P.Category = TEXT("RPG|Demo");
			P.DefaultValue = TEXT("2");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}
		{
			FCodeForgePropertyDef P;
			P.Name = TEXT("DemoSpawnRadius");
			P.Type = TEXT("float");
			P.Category = TEXT("RPG|Demo");
			P.DefaultValue = TEXT("500.0f");
			P.bEditAnywhere = true;
			P.bBlueprintReadWrite = true;
			BP->Properties.Add(P);
		}

		// ─ Functions ─
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("HandlePlayerDeath");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = true;
			F.FunctionBody = TEXT(
				"\tOnPlayerDied.Broadcast();\n"
				"\tFTimerHandle TimerHandle;\n"
				"\tGetWorldTimerManager().SetTimer(TimerHandle, this,\n"
				"\t\t&ARPGGameMode::RespawnPlayer, RespawnDelay, false);"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("SpawnDemoLoop");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = true;
			F.Category = TEXT("RPG|Demo");
			F.IncludePath = TEXT("Engine/World.h");
			F.FunctionBody = TEXT(
				"\tUWorld* World = GetWorld();\n"
				"\tif (!World) return;\n"
				"\tconst FVector Origin = FVector::ZeroVector;\n"
				"\tconst FRotator SpawnRotation = FRotator::ZeroRotator;\n"
				"\tfor (int32 Index = 0; Index < DemoEnemyCount && EnemyClass; ++Index)\n"
				"\t{\n"
				"\t\tWorld->SpawnActor<ARPGEnemy>(EnemyClass, Origin + FVector(DemoSpawnRadius + Index * 180.0f, (Index - 0.5f) * 220.0f, 80.0f), SpawnRotation);\n"
				"\t}\n"
				"\tfor (int32 Index = 0; Index < DemoPickupCount && PickupClass; ++Index)\n"
				"\t{\n"
				"\t\tWorld->SpawnActor<ARPGItemPickup>(PickupClass, Origin + FVector(180.0f + Index * 160.0f, -260.0f, 80.0f), SpawnRotation);\n"
				"\t}"
			);
			BP->Functions.Add(F);
		}
		{
			FCodeForgeFunctionDef F;
			F.Name = TEXT("RespawnPlayer");
			F.ReturnType = TEXT("void");
			F.bBlueprintCallable = true;
			F.FunctionBody = TEXT(
				"\tAPlayerController* PC = GetWorld()->GetFirstPlayerController();\n"
				"\tif (!PC) return;\n"
				"\tAActor* PlayerStart = FindPlayerStart(PC);\n"
				"\tif (!PlayerStart) return;\n"
				"\tAPawn* NewPawn = GetWorld()->SpawnActor<APawn>(\n"
				"\t\tDefaultPawnClass, PlayerStart->GetActorLocation(),\n"
				"\t\tPlayerStart->GetActorRotation());\n"
				"\tif (NewPawn)\n"
				"\t{\n"
				"\t\tPC->Possess(NewPawn);\n"
				"\t\tOnGameStarted.Broadcast();\n"
				"\t}"
			);
			BP->Functions.Add(F);
		}

		// ─ Delegates (names WITHOUT F prefix) ─
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnPlayerDied");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			BP->Delegates.Add(D);
		}
		{
			FCodeForgeDelegateDef D;
			D.Name = TEXT("OnGameStarted");
			D.DelegateType = ECodeForgeDelegateType::DynamicMulticast;
			BP->Delegates.Add(D);
		}

		SaveBlueprint(Pkg, BP);
	}

	// ── Generate C++ code for all example blueprints ──
	{
		FCodeForgeGenerator Generator;
		int32 SuccessCount = 0;
		int32 PreservedCount = 0;
		const TSet<FString> PreservePlayableRuntimeClasses = {
			TEXT("Interactable"),
			TEXT("ARPGCharacter"),
			TEXT("ARPGEnemy"),
			TEXT("ARPGItemPickup"),
			TEXT("ARPGGameMode")
		};

		for (UCodeForgeBlueprint* BP : CreatedBPs)
		{
			const FString HeaderPath = FPaths::Combine(
				FPaths::ProjectDir(), TEXT("Source"), ModuleName, BP->ClassName + TEXT(".h"));
			const FString SourcePath = FPaths::Combine(
				FPaths::ProjectDir(), TEXT("Source"), ModuleName, BP->ClassName + TEXT(".cpp"));

			const bool bHeaderOnlyOutput =
				BP->BlueprintKind == ECodeForgeBlueprintKind::Enum ||
				BP->BlueprintKind == ECodeForgeBlueprintKind::Interface;

			if (PreservePlayableRuntimeClasses.Contains(BP->ClassName)
				&& FPaths::FileExists(HeaderPath)
				&& (bHeaderOnlyOutput || FPaths::FileExists(SourcePath)))
			{
				PreservedCount++;
				SuccessCount++;
				UE_LOG(LogTemp, Log, TEXT("CodeForge: Preserved playable runtime source for %s"),
					*BP->ClassName);
				continue;
			}

			FCodeForgeGeneratorResult Result = Generator.GenerateAndIntegrate(
				BP, FPaths::ProjectDir());
			if (Result.bSuccess)
			{
				SuccessCount++;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("CodeForge: Failed to generate code for %s: %s"),
					*BP->ClassName, *Result.ErrorMessage);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("CodeForge: Generated/preserved C++ code for %d/%d example blueprints (%d preserved)"),
			SuccessCount, CreatedBPs.Num(), PreservedCount);
	}

	FString DemoMapStatus;
	const bool bDemoMapReady = CreateRPGStarterKitDemoMap(DemoMapStatus);
	UE_LOG(LogTemp, Log, TEXT("CodeForge: %s"), *DemoMapStatus);

	// Navigate the Content Browser to the folder so thumbnails render immediately
	{
		FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<FAssetData> CreatedAssets;
		IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
		AR.GetAssetsByPath(FName(*BaseDir), CreatedAssets, /*bRecursive=*/true);
		CBModule.Get().SyncBrowserToAssets(CreatedAssets);
	}

	// ── Completion notification ──
	const FString NotificationText = FString::Printf(
		TEXT("RPG Starter Kit created!\n9 blueprints + C++ code generated/preserved.\n%s%s"),
		*DemoMapStatus,
		bDemoMapReady ? TEXT("") : TEXT("\nCompile the project before playing the map."));
	FNotificationInfo Info(FText::FromString(NotificationText));
	Info.ExpireDuration = 5.0f;
	Info.bUseLargeFont = false;
	FSlateNotificationManager::Get().AddNotification(Info);
}

#undef LOCTEXT_NAMESPACE

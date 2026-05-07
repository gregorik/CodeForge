// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "Wizard/SCodeForgeNewAssetWizard.h"

#include "CodeForgeDefaults.h"
#include "CodeForgeModuleScanner.h"
#include "CodeForgeSettings.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CodeForgeWizard"

void SCodeForgeNewAssetWizard::Construct(const FArguments& InArgs)
{
    StepSwitcher = SNew(SWidgetSwitcher).WidgetIndex_Lambda([this]() { return CurrentStep; });
    StepSwitcher->AddSlot()[ BuildStep1NameAndArchetype() ];
    StepSwitcher->AddSlot()[ BuildStep2ParentClass() ];
    StepSwitcher->AddSlot()[ BuildStep3ModuleAndPath() ];
    StepSwitcher->AddSlot()[ BuildStep4ConfirmAndStarter() ];

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot().FillHeight(1.f).Padding(12)
        [
            StepSwitcher.ToSharedRef()
        ]

        // Footer: Back / Next / Cancel / Create
        + SVerticalBox::Slot().AutoHeight().Padding(12)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("Back", "Back"))
                .OnClicked(this, &SCodeForgeNewAssetWizard::OnBackClicked)
                .IsEnabled_Lambda([this]() { return CurrentStep > 0; })
            ]

            + SHorizontalBox::Slot().FillWidth(1.f) [ SNullWidget::NullWidget ]

            + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("Cancel", "Cancel"))
                .OnClicked(this, &SCodeForgeNewAssetWizard::OnCancelClicked)
            ]

            + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("Next", "Next"))
                .Visibility_Lambda([this]() { return CurrentStep < 3 ? EVisibility::Visible : EVisibility::Collapsed; })
                .OnClicked(this, &SCodeForgeNewAssetWizard::OnNextClicked)
                .IsEnabled_Lambda([this]() { return CanAdvance(); })
            ]

            + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
            [
                SNew(SButton)
                .Text(LOCTEXT("Create", "Create"))
                .Visibility_Lambda([this]() { return CurrentStep == 3 ? EVisibility::Visible : EVisibility::Collapsed; })
                .OnClicked(this, &SCodeForgeNewAssetWizard::OnCreateClicked)
                .IsEnabled_Lambda([this]() { return Spec.IsCompleteForCreation(); })
            ]
        ]
    ];
}

bool SCodeForgeNewAssetWizard::CanAdvance() const
{
    switch (CurrentStep)
    {
    case 0: return Spec.IsAssetNameValid();
    case 1: return Spec.BlueprintKind != ECodeForgeBlueprintKind::Class || !Spec.ParentClass.IsEmpty();
    case 2: return !Spec.TargetModule.IsNone();
    default: return true;
    }
}

FReply SCodeForgeNewAssetWizard::OnNextClicked() { CurrentStep = FMath::Clamp(CurrentStep + 1, 0, 3); return FReply::Handled(); }
FReply SCodeForgeNewAssetWizard::OnBackClicked() { CurrentStep = FMath::Clamp(CurrentStep - 1, 0, 3); return FReply::Handled(); }
FReply SCodeForgeNewAssetWizard::OnCancelClicked()
{
    bConfirmed = false;
    if (TSharedPtr<SWindow> Win = FSlateApplication::Get().FindWidgetWindow(AsShared())) Win->RequestDestroyWindow();
    return FReply::Handled();
}
FReply SCodeForgeNewAssetWizard::OnCreateClicked()
{
    bConfirmed = Spec.IsCompleteForCreation();
    if (TSharedPtr<SWindow> Win = FSlateApplication::Get().FindWidgetWindow(AsShared())) Win->RequestDestroyWindow();
    return FReply::Handled();
}

// =============================================================================
// Step 1 — Name + Archetype
// =============================================================================

namespace
{
    struct FArchetypeCard
    {
        ECodeForgeBlueprintKind Kind;
        ECodeForgeClassType     ClassType;
        FText                   Title;
        FText                   Caption;
    };

    static const TArray<FArchetypeCard>& GetArchetypeCards()
    {
        static const TArray<FArchetypeCard> Cards = {
            { ECodeForgeBlueprintKind::Class,     ECodeForgeClassType::Actor,          LOCTEXT("ArchActor",     "Actor"),
              LOCTEXT("ArchActorCaption",     "Something that lives in a level (player, enemy, pickup)") },
            { ECodeForgeBlueprintKind::Class,     ECodeForgeClassType::ActorComponent, LOCTEXT("ArchComponent", "Component"),
              LOCTEXT("ArchComponentCaption", "Modular behavior you attach to actors (Health, Inventory)") },
            { ECodeForgeBlueprintKind::Class,     ECodeForgeClassType::Object,         LOCTEXT("ArchObject",    "Object"),
              LOCTEXT("ArchObjectCaption",    "Lightweight UObject (DataAsset, SaveGame, Subsystem)") },
            { ECodeForgeBlueprintKind::Interface, ECodeForgeClassType::Object,         LOCTEXT("ArchInterface", "Interface"),
              LOCTEXT("ArchInterfaceCaption", "A set of functions other classes promise to implement") },
            { ECodeForgeBlueprintKind::Struct,    ECodeForgeClassType::Object,         LOCTEXT("ArchStruct",    "Struct"),
              LOCTEXT("ArchStructCaption",    "A small data record (name + score, position + rotation)") },
            { ECodeForgeBlueprintKind::Enum,      ECodeForgeClassType::Object,         LOCTEXT("ArchEnum",      "Enum"),
              LOCTEXT("ArchEnumCaption",      "A fixed list of named values (states, weapon types)") }
        };
        return Cards;
    }
}

TSharedRef<SWidget> SCodeForgeNewAssetWizard::BuildStep1NameAndArchetype()
{
    TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(6));

    int32 Index = 0;
    for (const FArchetypeCard& Card : GetArchetypeCards())
    {
        const int32 Col = Index % 3;
        const int32 Row = Index / 3;
        Grid->AddSlot(Col, Row)
        [
            SNew(SCheckBox)
            .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
            .HAlign(HAlign_Center)
            .Padding(8)
            .IsChecked_Lambda([this, Card]()
            {
                return (Spec.BlueprintKind == Card.Kind && Spec.ClassType == Card.ClassType) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
            .OnCheckStateChanged_Lambda([this, Card](ECheckBoxState NewState)
            {
                if (NewState == ECheckBoxState::Checked)
                {
                    Spec.BlueprintKind = Card.Kind;
                    Spec.ClassType     = Card.ClassType;
                }
            })
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                  [ SNew(STextBlock).Text(Card.Title).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
                  [ SNew(STextBlock).Text(Card.Caption).AutoWrapText(true) ]
            ]
        ];
        ++Index;
    }

    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
          [ SNew(STextBlock).Text(LOCTEXT("Step1Title", "What are you making?"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 8)
          [
              SAssignNew(NameBox, SEditableTextBox)
              .HintText(LOCTEXT("NameHint", "Class name (e.g. AMyActor, FMyStruct, EMyEnum)"))
              .OnTextChanged_Lambda([this](const FText& Text)
              {
                  Spec.AssetName = Text.ToString();
                  if (NameError.IsValid())
                  {
                      NameError->SetText(Spec.IsAssetNameValid() ? FText::GetEmpty()
                          : LOCTEXT("NameInvalid", "Name must start with a letter or _ and contain only letters, digits, and _."));
                  }
              })
          ]

        + SVerticalBox::Slot().AutoHeight()
          [ SAssignNew(NameError, STextBlock).ColorAndOpacity(FLinearColor::Red) ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 16)
          [ SNew(STextBlock).Text(LOCTEXT("ArchetypePrompt", "Pick an archetype:")) ]

        + SVerticalBox::Slot().FillHeight(1.f).Padding(0, 4)
          [ Grid ];
}

// =============================================================================
// Step 2 — Parent class
// =============================================================================

TSharedRef<SWidget> SCodeForgeNewAssetWizard::BuildStep2ParentClass()
{
    auto GetCuratedList = [this]() -> const TArray<FString>&
    {
        if (Spec.BlueprintKind != ECodeForgeBlueprintKind::Class)
        {
            static const TArray<FString> Empty;
            return Empty;
        }
        switch (Spec.ClassType)
        {
        case ECodeForgeClassType::Actor:          return CodeForgeDefaults::CommonActorParents;
        case ECodeForgeClassType::ActorComponent: return CodeForgeDefaults::CommonComponentParents;
        case ECodeForgeClassType::Object:         return CodeForgeDefaults::CommonObjectParents;
        default:
        {
            static const TArray<FString> Empty;
            return Empty;
        }
        }
    };

    TSharedRef<SVerticalBox> Box = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
          [ SNew(STextBlock).Text(LOCTEXT("Step2Title", "Pick a parent class"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) ];

    if (Spec.BlueprintKind != ECodeForgeBlueprintKind::Class)
    {
        Box->AddSlot().AutoHeight().Padding(0, 8)
        [ SNew(STextBlock).Text(LOCTEXT("Step2Skip", "(Skipped — structs, enums, and interfaces have no parent class.)")) ];
        return Box;
    }

    // Default selection
    if (Spec.ParentClass.IsEmpty())
    {
        switch (Spec.ClassType)
        {
        case ECodeForgeClassType::Actor:          Spec.ParentClass = CodeForgeDefaults::DefaultActorParent;     break;
        case ECodeForgeClassType::ActorComponent: Spec.ParentClass = CodeForgeDefaults::DefaultComponentParent; break;
        case ECodeForgeClassType::Object:         Spec.ParentClass = CodeForgeDefaults::DefaultObjectParent;    break;
        default: break;
        }
    }

    TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(4));
    int32 Index = 0;
    for (const FString& Parent : GetCuratedList())
    {
        const int32 Col = Index % 2;
        const int32 Row = Index / 2;
        Grid->AddSlot(Col, Row)
        [
            SNew(SButton)
            .Text(FText::FromString(Parent))
            .OnClicked_Lambda([this, Parent]() { Spec.ParentClass = Parent; return FReply::Handled(); })
            .HAlign(HAlign_Center).ContentPadding(6)
        ];
        ++Index;
    }

    Box->AddSlot().AutoHeight().Padding(0, 8)
    [ SNew(STextBlock).Text_Lambda([this]() {
        return FText::Format(LOCTEXT("Step2Selected", "Selected: {0}"), FText::FromString(Spec.ParentClass));
      })
    ];

    Box->AddSlot().FillHeight(1.f).Padding(0, 8) [ Grid ];

    Box->AddSlot().AutoHeight().Padding(0, 8)
    [
        SNew(STextBlock).Text(LOCTEXT("Step2ShowAll",
            "Need a parent not in this list? Pick something close, then change ParentClass after creation in the graph."))
        .AutoWrapText(true)
    ];

    return Box;
}

// =============================================================================
// Step 3 — Module + Path
// =============================================================================

TSharedRef<SWidget> SCodeForgeNewAssetWizard::BuildStep3ModuleAndPath()
{
    // Populate module options from the scanner; fall back to project name.
    static TArray<TSharedPtr<FString>> ModuleOptions;
    ModuleOptions.Reset();

    const FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    FCodeForgeModuleScanner Scanner;
    TArray<FCodeForgeModuleInfo> ScannedModules = Scanner.ScanProject(ProjectDir);
    TArray<FString> Modules;
    for (const FCodeForgeModuleInfo& Mi : ScannedModules)
    {
        Modules.Add(Mi.ModuleName);
    }
    if (Modules.Num() == 0)
    {
        Modules.Add(FApp::GetProjectName());
    }
    for (const FString& Mod : Modules)
    {
        ModuleOptions.Add(MakeShared<FString>(Mod));
    }

    // Default selection from settings or first scanner result
    if (Spec.TargetModule.IsNone())
    {
        const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
        const FString DefaultMod = (Settings && !Settings->DefaultModuleTarget.IsEmpty())
            ? Settings->DefaultModuleTarget : Modules[0];
        Spec.TargetModule = FName(*DefaultMod);
    }
    if (Spec.AssetSavePath.IsEmpty())
    {
        Spec.AssetSavePath = FString::Printf(TEXT("/Game/CodeForge/%s"), *Spec.AssetName);
    }

    TSharedPtr<FString> InitialSelection;
    for (TSharedPtr<FString>& Opt : ModuleOptions)
    {
        if (FName(**Opt) == Spec.TargetModule) { InitialSelection = Opt; break; }
    }

    return SNew(SVerticalBox)

        + SVerticalBox::Slot().AutoHeight()
          [ SNew(STextBlock).Text(LOCTEXT("Step3Title", "Where does it go?"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 12)
          [ SNew(STextBlock).Text(LOCTEXT("Step3Module", "Target module")) ]

        + SVerticalBox::Slot().AutoHeight()
          [
              SNew(SComboBox<TSharedPtr<FString>>)
              .OptionsSource(&ModuleOptions)
              .InitiallySelectedItem(InitialSelection)
              .OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewVal, ESelectInfo::Type)
              {
                  if (NewVal.IsValid()) Spec.TargetModule = FName(**NewVal);
              })
              .OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
              {
                  return SNew(STextBlock).Text(FText::FromString(*InItem));
              })
              [ SNew(STextBlock).Text_Lambda([this]() { return FText::FromName(Spec.TargetModule); }) ]
          ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 12)
          [ SNew(STextBlock).Text(LOCTEXT("Step3SubDir", "Subdirectory under module Source/")) ]

        + SVerticalBox::Slot().AutoHeight()
          [
              SNew(SEditableTextBox)
              .HintText(LOCTEXT("Step3SubDirHint", "e.g. Items/  (leave blank for module root)"))
              .Text_Lambda([this]() { return FText::FromString(Spec.TargetSubDirectory); })
              .OnTextChanged_Lambda([this](const FText& T) { Spec.TargetSubDirectory = T.ToString(); })
          ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 12)
          [ SNew(STextBlock).Text(LOCTEXT("Step3SavePath", "Asset save location (Content path)")) ]

        + SVerticalBox::Slot().AutoHeight()
          [
              SNew(SEditableTextBox)
              .Text_Lambda([this]() { return FText::FromString(Spec.AssetSavePath); })
              .OnTextChanged_Lambda([this](const FText& T) { Spec.AssetSavePath = T.ToString(); })
          ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 12)
          [
              SNew(SBorder).Padding(8)
              [
                  SNew(STextBlock).AutoWrapText(true)
                  .Text_Lambda([this]() {
                      return FText::Format(LOCTEXT("Step3Preview",
                          "Will create:\n  Source: {0}/Source/{1}/Public/{2}/{3}.h (+ .cpp)\n  Asset:  {4}.uasset"),
                          FText::FromString(FApp::GetProjectName()),
                          FText::FromName(Spec.TargetModule),
                          FText::FromString(Spec.TargetSubDirectory),
                          FText::FromString(Spec.AssetName),
                          FText::FromString(Spec.AssetSavePath));
                  })
              ]
          ];
}

// =============================================================================
// Step 4 — Starter content + Confirm
// =============================================================================

TSharedRef<SWidget> SCodeForgeNewAssetWizard::BuildStep4ConfirmAndStarter()
{
    return SNew(SVerticalBox)

        + SVerticalBox::Slot().AutoHeight()
          [ SNew(STextBlock).Text(LOCTEXT("Step4Title", "Starter content & confirm"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16)) ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 16)
          [
              SNew(SCheckBox)
              .IsChecked_Lambda([this]() {
                  return Spec.bIncludeStarterContent ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
              })
              .OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
                  Spec.bIncludeStarterContent = (State == ECheckBoxState::Checked);
              })
              [
                  SNew(STextBlock).Text(LOCTEXT("Step4Starter",
                      "Include starter content (recommended) — adds 1 example property and 1 example function so the graph isn't empty."))
                  .AutoWrapText(true)
              ]
          ]

        + SVerticalBox::Slot().AutoHeight().Padding(0, 24)
          [
              SNew(SBorder).Padding(8)
              [
                  SNew(STextBlock).AutoWrapText(true)
                  .Text_Lambda([this]() {
                      const TCHAR* KindStr = TEXT("Class");
                      switch (Spec.BlueprintKind)
                      {
                      case ECodeForgeBlueprintKind::Struct: KindStr = TEXT("Struct"); break;
                      case ECodeForgeBlueprintKind::Enum:   KindStr = TEXT("Enum"); break;
                      default: break;
                      }
                      return FText::Format(LOCTEXT("Step4Summary",
                          "About to create:\n"
                          "  Name: {0}\n"
                          "  Kind: {1}\n"
                          "  Parent: {2}\n"
                          "  Module: {3}\n"
                          "  Subdirectory: {4}\n"
                          "  Save path: {5}\n"
                          "  Starter content: {6}"),
                          FText::FromString(Spec.AssetName),
                          FText::FromString(KindStr),
                          FText::FromString(Spec.ParentClass.IsEmpty() ? TEXT("(n/a)") : *Spec.ParentClass),
                          FText::FromName(Spec.TargetModule),
                          FText::FromString(Spec.TargetSubDirectory.IsEmpty() ? TEXT("(none)") : *Spec.TargetSubDirectory),
                          FText::FromString(Spec.AssetSavePath),
                          FText::FromString(Spec.bIncludeStarterContent ? TEXT("yes") : TEXT("no")));
                  })
              ]
          ];
}

#undef LOCTEXT_NAMESPACE

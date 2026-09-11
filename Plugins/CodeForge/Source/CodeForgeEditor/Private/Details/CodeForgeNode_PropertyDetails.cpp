// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "Details/CodeForgeNode_PropertyDetails.h"

#include "CodeForgeDefaults.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeSettings.h"
#include "CodeForgeSpecifierCopy.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CodeForgePropertyDetails"

TSharedRef<IDetailCustomization> FCodeForgeNode_PropertyDetails::MakeInstance()
{
    return MakeShared<FCodeForgeNode_PropertyDetails>();
}

void FCodeForgeNode_PropertyDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
    const bool bShowAdvanced = Settings && Settings->bShowAdvancedSpecifiers;

    IDetailCategoryBuilder& CommonCat = DetailBuilder.EditCategory(
        TEXT("Common"), LOCTEXT("CommonCat", "Common"), ECategoryPriority::Important);

    IDetailCategoryBuilder& AdvancedCat = DetailBuilder.EditCategory(
        TEXT("Advanced"), LOCTEXT("AdvancedCat", "Advanced (UPROPERTY specifiers)"), ECategoryPriority::Default);

    // The node's only top-level UPROPERTY is FCodeForgePropertyDef PropertyDef.
    TSharedRef<IPropertyHandle> DefHandle = DetailBuilder.GetProperty(
        GET_MEMBER_NAME_CHECKED(UCodeForgeNode_Property, PropertyDef));
    DetailBuilder.HideProperty(DefHandle); // we re-add its children manually

    uint32 NumChildren = 0;
    DefHandle->GetNumChildren(NumChildren);

    using namespace CodeForgeSpecifierCopy;

    for (uint32 i = 0; i < NumChildren; ++i)
    {
        TSharedPtr<IPropertyHandle> ChildHandle = DefHandle->GetChildHandle(i);
        if (!ChildHandle.IsValid()) continue;

        const FName ChildName = ChildHandle->GetProperty() ? ChildHandle->GetProperty()->GetFName() : NAME_None;
        const bool bIsCommon = CodeForgeDefaults::CommonPropertyFields.Contains(ChildName);

        IDetailCategoryBuilder& Target = bIsCommon ? CommonCat : AdvancedCat;

        if (!bIsCommon && !bShowAdvanced)
        {
            continue; // Advanced specifier hidden by setting
        }

        IDetailPropertyRow& Row = Target.AddProperty(ChildHandle);

        // Plain-English tooltip override
        const FString Friendly = LookupTooltip(ESpecifierNodeKind::Property, ChildName);
        if (!Friendly.IsEmpty())
        {
            Row.ToolTip(FText::FromString(Friendly));
        }
    }
}

#undef LOCTEXT_NAMESPACE

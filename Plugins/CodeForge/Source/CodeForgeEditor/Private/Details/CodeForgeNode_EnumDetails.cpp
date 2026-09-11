// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "Details/CodeForgeNode_EnumDetails.h"

#include "CodeForgeDefaults.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeSettings.h"
#include "CodeForgeSpecifierCopy.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "CodeForgeEnumDetails"

TSharedRef<IDetailCustomization> FCodeForgeNode_EnumDetails::MakeInstance()
{
    return MakeShared<FCodeForgeNode_EnumDetails>();
}

void FCodeForgeNode_EnumDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
    const bool bShowAdvanced = Settings && Settings->bShowAdvancedSpecifiers;

    IDetailCategoryBuilder& CommonCat = DetailBuilder.EditCategory(
        TEXT("Common"), LOCTEXT("CommonCat", "Common"), ECategoryPriority::Important);
    IDetailCategoryBuilder& AdvancedCat = DetailBuilder.EditCategory(
        TEXT("Advanced"), LOCTEXT("AdvancedCat", "Advanced (UENUM specifiers)"), ECategoryPriority::Default);

    UClass* NodeClass = UCodeForgeNode_Enum::StaticClass();
    for (TFieldIterator<FProperty> It(NodeClass); It; ++It)
    {
        FProperty* Prop = *It;
        if (!Prop || !Prop->HasAnyPropertyFlags(CPF_Edit)) continue;

        TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(Prop->GetFName(), NodeClass);
        if (!Handle->IsValidHandle()) continue;

        DetailBuilder.HideProperty(Handle);

        const FName Name = Prop->GetFName();
        const bool bIsCommon = CodeForgeDefaults::CommonEnumFields.Contains(Name);
        if (!bIsCommon && !bShowAdvanced) continue;

        IDetailCategoryBuilder& Target = bIsCommon ? CommonCat : AdvancedCat;
        IDetailPropertyRow& Row = Target.AddProperty(Handle);

        const FString Friendly = CodeForgeSpecifierCopy::LookupTooltip(
            CodeForgeSpecifierCopy::ESpecifierNodeKind::Enum, Name);
        if (!Friendly.IsEmpty())
        {
            Row.ToolTip(FText::FromString(Friendly));
        }
    }
}

#undef LOCTEXT_NAMESPACE

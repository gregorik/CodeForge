// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "Details/CodeForgeNode_StructDetails.h"

#include "CodeForgeDefaults.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeSettings.h"
#include "CodeForgeSpecifierCopy.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "CodeForgeStructDetails"

TSharedRef<IDetailCustomization> FCodeForgeNode_StructDetails::MakeInstance()
{
    return MakeShared<FCodeForgeNode_StructDetails>();
}

void FCodeForgeNode_StructDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
    const bool bShowAdvanced = Settings && Settings->bShowAdvancedSpecifiers;

    IDetailCategoryBuilder& CommonCat = DetailBuilder.EditCategory(
        TEXT("Common"), LOCTEXT("CommonCat", "Common"), ECategoryPriority::Important);
    IDetailCategoryBuilder& AdvancedCat = DetailBuilder.EditCategory(
        TEXT("Advanced"), LOCTEXT("AdvancedCat", "Advanced (USTRUCT specifiers)"), ECategoryPriority::Default);

    UClass* NodeClass = UCodeForgeNode_Struct::StaticClass();
    for (TFieldIterator<FProperty> It(NodeClass); It; ++It)
    {
        FProperty* Prop = *It;
        if (!Prop || !Prop->HasAnyPropertyFlags(CPF_Edit)) continue;

        TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(Prop->GetFName(), NodeClass);
        if (!Handle->IsValidHandle()) continue;

        DetailBuilder.HideProperty(Handle);

        const FName Name = Prop->GetFName();
        const bool bIsCommon = CodeForgeDefaults::CommonStructFields.Contains(Name);
        if (!bIsCommon && !bShowAdvanced) continue;

        IDetailCategoryBuilder& Target = bIsCommon ? CommonCat : AdvancedCat;
        IDetailPropertyRow& Row = Target.AddProperty(Handle);

        const FString Friendly = CodeForgeSpecifierCopy::LookupTooltip(
            CodeForgeSpecifierCopy::ESpecifierNodeKind::Struct, Name);
        if (!Friendly.IsEmpty())
        {
            Row.ToolTip(FText::FromString(Friendly));
        }
    }
}

#undef LOCTEXT_NAMESPACE

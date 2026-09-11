// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "Details/CodeForgeNode_ClassDetails.h"

#include "CodeForgeDefaults.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeSettings.h"
#include "CodeForgeSpecifierCopy.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "CodeForgeClassDetails"

TSharedRef<IDetailCustomization> FCodeForgeNode_ClassDetails::MakeInstance()
{
    return MakeShared<FCodeForgeNode_ClassDetails>();
}

void FCodeForgeNode_ClassDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
    const bool bShowAdvanced = Settings && Settings->bShowAdvancedSpecifiers;

    IDetailCategoryBuilder& CommonCat = DetailBuilder.EditCategory(
        TEXT("Common"), LOCTEXT("CommonCat", "Common"), ECategoryPriority::Important);
    IDetailCategoryBuilder& AdvancedCat = DetailBuilder.EditCategory(
        TEXT("Advanced"), LOCTEXT("AdvancedCat", "Advanced (UCLASS specifiers)"), ECategoryPriority::Default);

    // Iterate every top-level property on the class node and route into Common/Advanced.
    TArray<TSharedRef<IPropertyHandle>> AllProps;
    UClass* NodeClass = UCodeForgeNode_Class::StaticClass();
    for (TFieldIterator<FProperty> It(NodeClass); It; ++It)
    {
        FProperty* Prop = *It;
        if (!Prop || !Prop->HasAnyPropertyFlags(CPF_Edit)) continue;

        TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(Prop->GetFName(), NodeClass);
        if (!Handle->IsValidHandle()) continue;

        DetailBuilder.HideProperty(Handle);

        const FName Name = Prop->GetFName();
        const bool bIsCommon = CodeForgeDefaults::CommonClassFields.Contains(Name);

        if (!bIsCommon && !bShowAdvanced) continue;

        IDetailCategoryBuilder& Target = bIsCommon ? CommonCat : AdvancedCat;
        IDetailPropertyRow& Row = Target.AddProperty(Handle);

        const FString Friendly = CodeForgeSpecifierCopy::LookupTooltip(
            CodeForgeSpecifierCopy::ESpecifierNodeKind::Class, Name);
        if (!Friendly.IsEmpty())
        {
            Row.ToolTip(FText::FromString(Friendly));
        }
    }
}

#undef LOCTEXT_NAMESPACE

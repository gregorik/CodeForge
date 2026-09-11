// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#include "Details/CodeForgeNode_FunctionDetails.h"

#include "CodeForgeDefaults.h"
#include "CodeForgeNode_Function.h"
#include "CodeForgeSettings.h"
#include "CodeForgeSpecifierCopy.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "CodeForgeFunctionDetails"

TSharedRef<IDetailCustomization> FCodeForgeNode_FunctionDetails::MakeInstance()
{
    return MakeShared<FCodeForgeNode_FunctionDetails>();
}

void FCodeForgeNode_FunctionDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
    const bool bShowAdvanced = Settings && Settings->bShowAdvancedSpecifiers;

    IDetailCategoryBuilder& CommonCat = DetailBuilder.EditCategory(
        TEXT("Common"), LOCTEXT("CommonCat", "Common"), ECategoryPriority::Important);
    IDetailCategoryBuilder& AdvancedCat = DetailBuilder.EditCategory(
        TEXT("Advanced"), LOCTEXT("AdvancedCat", "Advanced (UFUNCTION specifiers)"), ECategoryPriority::Default);

    TSharedRef<IPropertyHandle> DefHandle = DetailBuilder.GetProperty(
        GET_MEMBER_NAME_CHECKED(UCodeForgeNode_Function, FunctionDef));
    DetailBuilder.HideProperty(DefHandle);

    uint32 NumChildren = 0;
    DefHandle->GetNumChildren(NumChildren);

    using namespace CodeForgeSpecifierCopy;

    for (uint32 i = 0; i < NumChildren; ++i)
    {
        TSharedPtr<IPropertyHandle> ChildHandle = DefHandle->GetChildHandle(i);
        if (!ChildHandle.IsValid()) continue;

        const FName ChildName = ChildHandle->GetProperty() ? ChildHandle->GetProperty()->GetFName() : NAME_None;
        const bool bIsCommon = CodeForgeDefaults::CommonFunctionFields.Contains(ChildName);

        if (!bIsCommon && !bShowAdvanced) continue;

        IDetailCategoryBuilder& Target = bIsCommon ? CommonCat : AdvancedCat;
        IDetailPropertyRow& Row = Target.AddProperty(ChildHandle);

        const FString Friendly = LookupTooltip(ESpecifierNodeKind::Function, ChildName);
        if (!Friendly.IsEmpty())
        {
            Row.ToolTip(FText::FromString(Friendly));
        }
    }
}

#undef LOCTEXT_NAMESPACE

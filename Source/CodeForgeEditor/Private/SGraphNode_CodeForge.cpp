// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "SGraphNode_CodeForge.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"
#include "SGraphPin.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"
#include "ScopedTransaction.h"

// ---------------------------------------------------------------------------
// Branding helpers
// ---------------------------------------------------------------------------

namespace CodeForgeNodeStyle
{
	/** Short badge label for each node type. */
	static FString GetNodeBadge(const UCodeForgeEdGraphNode* Node)
	{
		if (!Node) return TEXT("?");
		if (Node->IsA<UCodeForgeNode_Class>())     return TEXT("C");
		if (Node->IsA<UCodeForgeNode_Struct>())    return TEXT("S");
		if (Node->IsA<UCodeForgeNode_Enum>())      return TEXT("E");
		if (Node->IsA<UCodeForgeNode_Property>())  return TEXT("P");
		if (Node->IsA<UCodeForgeNode_Function>())  return TEXT("fn");
		return TEXT("?");
	}

	/** Whether this is a "type" node (Class/Struct/Enum) vs a "member" node. */
	static bool IsTypeNode(const UCodeForgeEdGraphNode* Node)
	{
		if (!Node) return false;
		return Node->IsA<UCodeForgeNode_Class>()
			|| Node->IsA<UCodeForgeNode_Struct>()
			|| Node->IsA<UCodeForgeNode_Enum>();
	}

	/** Body background â€” reads brightness from settings with optional tinting. */
	static FLinearColor GetBodyColor(const UCodeForgeEdGraphNode* Node)
	{
		const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();
		float Base = CFSettings->NodeBodyBrightness;

		if (CFSettings->bTintTypeNodeBodies && IsTypeNode(Node))
		{
			FLinearColor Accent = Node->GetNodeTitleColor();
			return FLinearColor(
				Base + Accent.R * 0.04f,
				Base + Accent.G * 0.04f,
				Base + Accent.B * 0.04f);
		}
		return FLinearColor(Base, Base, Base);
	}
}

// ---------------------------------------------------------------------------
// Construct
// ---------------------------------------------------------------------------

void SGraphNode_CodeForge::Construct(const FArguments& InArgs, UCodeForgeEdGraphNode* InNode)
{
	CodeForgeNode = InNode;
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

// ---------------------------------------------------------------------------
// UpdateGraphNode â€” branded layout
// ---------------------------------------------------------------------------

void SGraphNode_CodeForge::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	const UCodeForgeSettings* CFSettings = GetDefault<UCodeForgeSettings>();

	FLinearColor TitleColor = CodeForgeNode ? CodeForgeNode->GetNodeTitleColor() : FLinearColor::White;
	FLinearColor BodyColor  = CodeForgeNodeStyle::GetBodyColor(CodeForgeNode);
	FString Badge           = CodeForgeNodeStyle::GetNodeBadge(CodeForgeNode);
	bool bIsType            = CodeForgeNodeStyle::IsTypeNode(CodeForgeNode);
	FLinearColor BrandAccent = CFSettings->BrandAccentColor;
	int32 FontSize          = bIsType ? CFSettings->TypeNodeFontSize : CFSettings->MemberNodeFontSize;

	// Brighter title color for the title bar
	FLinearColor TitleBarColor = TitleColor * 0.85f;
	TitleBarColor.A = 1.0f;

	// Badge background â€” saturated version of the title color
	FLinearColor BadgeBg = TitleColor * 1.2f;
	BadgeBg.A = 1.0f;

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
		.Padding(0)
		.BorderBackgroundColor(BodyColor)
		[
			SNew(SVerticalBox)

			// â”€â”€ Top accent stripe (2px brand line) â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(2.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bIsType ? BrandAccent : TitleColor)
				]
			]

			// â”€â”€ Title bar with badge â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
				.BorderBackgroundColor(TitleBarColor)
				.Padding(FMargin(0.f))
				[
					SNew(SHorizontalBox)

					// Badge pill
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.f, 4.f, 4.f, 4.f))
					.VAlign(VAlign_Center)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(BadgeBg)
						.Padding(FMargin(5.f, 1.f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(Badge))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
							.ColorAndOpacity(FLinearColor::White)
							.Justification(ETextJustify::Center)
						]
					]

					// Title text
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.Padding(FMargin(2.f, 5.f, 10.f, 5.f))
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(CodeForgeNode ? CodeForgeNode->GetNodeTitle(ENodeTitleType::FullTitle) : FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", FontSize))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]

			// â”€â”€ Pins area â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(6.f, 6.f, 6.f, 4.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.MinDesiredWidth(bIsType ? 60.f : 30.f)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]

			// â”€â”€ Validation badge slot â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ValidationBadge, SBox)
			]

			// â”€â”€ Bottom accent stripe (1px) â”€â”€
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(TitleColor * 0.5f)
				]
			]
		]
	];

	CreatePinWidgets();
}

// ---------------------------------------------------------------------------
// CreatePinWidgets
// ---------------------------------------------------------------------------

void SGraphNode_CodeForge::CreatePinWidgets()
{
	if (!CodeForgeNode) return;

	for (UEdGraphPin* Pin : CodeForgeNode->Pins)
	{
		if (!Pin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SGraphPin, Pin);
			NewPin->SetIsEditable(IsEditable);
			AddPin(NewPin.ToSharedRef());
		}
	}
}

// ---------------------------------------------------------------------------
// GetShadowBrush
// ---------------------------------------------------------------------------

const FSlateBrush* SGraphNode_CodeForge::GetShadowBrush(bool bSelected) const
{
	return bSelected
		? FAppStyle::GetBrush(TEXT("Graph.Node.ShadowSelected"))
		: FAppStyle::GetBrush(TEXT("Graph.Node.Shadow"));
}

// ---------------------------------------------------------------------------
// UpdateValidationDisplay
// ---------------------------------------------------------------------------

void SGraphNode_CodeForge::UpdateValidationDisplay()
{
	if (!CodeForgeNode || !ValidationBadge.IsValid()) return;

	if (CodeForgeNode->bHasCompilerMessage && !CodeForgeNode->ErrorMsg.IsEmpty())
	{
		FLinearColor BadgeColor = (CodeForgeNode->ErrorType == EMessageSeverity::Error)
			? FLinearColor(1.0f, 0.25f, 0.25f) : FLinearColor(1.0f, 0.85f, 0.2f);

		ValidationBadge->SetContent(
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(BadgeColor * 0.15f)
			.Padding(FMargin(8.f, 3.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(CodeForgeNode->ErrorType == EMessageSeverity::Error ? TEXT("\u26A0") : TEXT("\u25C9")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(BadgeColor)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(CodeForgeNode->ErrorMsg))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
					.ColorAndOpacity(BadgeColor)
					.AutoWrapText(true)
				]
			]
		);
	}
	else
	{
		ValidationBadge->SetContent(SNullWidget::NullWidget);
	}
}

// ---------------------------------------------------------------------------
// Drag-and-drop support
// ---------------------------------------------------------------------------

void SGraphNode_CodeForge::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FAssetDragDropOp> AssetOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
	if (AssetOp.IsValid() && CodeForgeNode && CodeForgeNode->IsA<UCodeForgeNode_Property>())
	{
		for (const FAssetData& Asset : AssetOp->GetAssets())
		{
			if (Asset.GetClass()->IsChildOf(UCodeForgeBlueprint::StaticClass()))
			{
				SetCursor(EMouseCursor::GrabHandClosed);
				return;
			}
		}
	}
	SGraphNode::OnDragEnter(MyGeometry, DragDropEvent);
}

void SGraphNode_CodeForge::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	SetCursor(EMouseCursor::CardinalCross);
	SGraphNode::OnDragLeave(DragDropEvent);
}

FReply SGraphNode_CodeForge::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FAssetDragDropOp> AssetOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
	if (!AssetOp.IsValid()) return FReply::Unhandled();

	UCodeForgeNode_Property* PropNode = Cast<UCodeForgeNode_Property>(CodeForgeNode);
	if (!PropNode) return FReply::Unhandled();

	for (const FAssetData& Asset : AssetOp->GetAssets())
	{
		UCodeForgeBlueprint* DroppedBP = Cast<UCodeForgeBlueprint>(Asset.GetAsset());
		if (!DroppedBP) continue;

		const FScopedTransaction Transaction(FText::FromString(TEXT("Set Property Type from Asset")));
		PropNode->Modify();

		if (DroppedBP->BlueprintKind == ECodeForgeBlueprintKind::Enum)
		{
			PropNode->PropertyDef.Type = DroppedBP->ClassName;
		}
		else if (DroppedBP->BlueprintKind == ECodeForgeBlueprintKind::Struct)
		{
			PropNode->PropertyDef.Type = TEXT("F") + DroppedBP->ClassName;
		}

		PropNode->PropertyDef.IncludePath = DroppedBP->ClassName + TEXT(".h");
		return FReply::Handled();
	}

	return FReply::Unhandled();
}




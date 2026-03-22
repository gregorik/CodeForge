#include "SGraphNode_CodeForge.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeNode_Enum.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"
#include "CodeForgeNode_RPC.h"
#include "SGraphPin.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/AppStyle.h"

void SGraphNode_CodeForge::Construct(const FArguments& InArgs, UCodeForgeEdGraphNode* InNode)
{
	CodeForgeNode = InNode;
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SGraphNode_CodeForge::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	// Title bar color based on node type
	FLinearColor TitleColor = CodeForgeNode ? CodeForgeNode->GetNodeTitleColor() : FLinearColor::White;

	// Build the node body
	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
		.Padding(0)
		.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.08f))
		[
			SNew(SVerticalBox)

			// Title bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
				.BorderBackgroundColor(TitleColor)
				.Padding(FMargin(8.f, 4.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(CodeForgeNode ? CodeForgeNode->GetNodeTitle(ENodeTitleType::FullTitle) : FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]

			// Pins area
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]

			// Validation badge slot
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ValidationBadge, SBox)
			]
		]
	];

	CreatePinWidgets();
}

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

const FSlateBrush* SGraphNode_CodeForge::GetShadowBrush(bool bSelected) const
{
	return bSelected
		? FAppStyle::GetBrush(TEXT("Graph.Node.ShadowSelected"))
		: FAppStyle::GetBrush(TEXT("Graph.Node.Shadow"));
}

void SGraphNode_CodeForge::UpdateValidationDisplay()
{
	if (!CodeForgeNode || !ValidationBadge.IsValid()) return;

	if (CodeForgeNode->bHasCompilerMessage && !CodeForgeNode->ErrorMsg.IsEmpty())
	{
		FLinearColor BadgeColor = (CodeForgeNode->ErrorType == EMessageSeverity::Error)
			? FLinearColor::Red : FLinearColor::Yellow;

		ValidationBadge->SetContent(
			SNew(SBorder)
			.BorderBackgroundColor(BadgeColor * 0.3f)
			.Padding(FMargin(6.f, 2.f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(CodeForgeNode->ErrorMsg))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				.ColorAndOpacity(BadgeColor)
			]
		);
	}
	else
	{
		ValidationBadge->SetContent(SNullWidget::NullWidget);
	}
}

// Drag-and-drop stubs — implemented in Task 11
void SGraphNode_CodeForge::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	SGraphNode::OnDragEnter(MyGeometry, DragDropEvent);
}

void SGraphNode_CodeForge::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	SGraphNode::OnDragLeave(DragDropEvent);
}

FReply SGraphNode_CodeForge::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return SGraphNode::OnDrop(MyGeometry, DragDropEvent);
}

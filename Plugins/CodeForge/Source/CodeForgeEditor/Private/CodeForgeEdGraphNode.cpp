#include "CodeForgeEdGraphNode.h"

FText UCodeForgeEdGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("CodeForge Node"));
}

FLinearColor UCodeForgeEdGraphNode::GetNodeTitleColor() const
{
	return FLinearColor::White;
}

void UCodeForgeEdGraphNode::AllocateDefaultPins()
{
}

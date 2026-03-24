#include "CodeForgeEdGraphNode.h"
#include "EdGraph/EdGraphSchema.h"

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

void UCodeForgeEdGraphNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (!FromPin)
	{
		return;
	}

	const UEdGraphSchema* Schema = GetSchema();
	if (!Schema)
	{
		return;
	}

	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction != FromPin->Direction
			&& Pin->PinType.PinCategory == FromPin->PinType.PinCategory)
		{
			if (Schema->TryCreateConnection(FromPin, Pin))
			{
				break;
			}
		}
	}
}


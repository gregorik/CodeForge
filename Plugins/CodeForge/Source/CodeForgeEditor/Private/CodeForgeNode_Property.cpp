#include "CodeForgeNode_Property.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"

FText UCodeForgeNode_Property::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Property: %s"), *PropertyDef.Name));
}

FLinearColor UCodeForgeNode_Property::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.8f, 0.2f);
}

void UCodeForgeNode_Property::AllocateDefaultPins()
{
	FEdGraphPinType PropertyPinType;
	PropertyPinType.PinCategory = TEXT("PC_CodeForgeProperty");
	CreatePin(EGPD_Input, PropertyPinType, TEXT("Owner"));
}

void UCodeForgeNode_Property::SyncToBlueprint(UCodeForgeBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	UEdGraphPin* InputPin = FindPin(TEXT("Owner"), EGPD_Input);
	if (!InputPin || InputPin->LinkedTo.Num() == 0)
	{
		return;
	}

	UEdGraphNode* ConnectedNode = InputPin->LinkedTo[0]->GetOwningNode();

	if (ConnectedNode->IsA<UCodeForgeNode_Class>())
	{
		Blueprint->Properties.Add(PropertyDef);
	}
	else if (ConnectedNode->IsA<UCodeForgeNode_Struct>())
	{
		Blueprint->StructProperties.Add(PropertyDef);
	}
}

void UCodeForgeNode_Property::SyncFromBlueprint(const UCodeForgeBlueprint* Blueprint)
{
}

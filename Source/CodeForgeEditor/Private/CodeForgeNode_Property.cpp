// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "CodeForgeNode_Property.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeNode_Class.h"
#include "CodeForgeNode_Struct.h"
#include "CodeForgeSettings.h"

FText UCodeForgeNode_Property::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Property: %s"), *PropertyDef.Name));
}

FLinearColor UCodeForgeNode_Property::GetNodeTitleColor() const
{
	return GetDefault<UCodeForgeSettings>()->PropertyNodeColor;
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
	if (!Blueprint)
	{
		return;
	}

	// Determine which array to pull from based on pin connection
	UEdGraphPin* InputPin = FindPin(TEXT("Owner"), EGPD_Input);
	if (InputPin && InputPin->LinkedTo.Num() > 0)
	{
		UEdGraphNode* ConnectedNode = InputPin->LinkedTo[0]->GetOwningNode();
		if (ConnectedNode->IsA<UCodeForgeNode_Struct>())
		{
			// Pull from StructProperties by matching Name
			for (const FCodeForgePropertyDef& Prop : Blueprint->StructProperties)
			{
				if (Prop.Name == PropertyDef.Name)
				{
					PropertyDef = Prop;
					return;
				}
			}
			return;
		}
	}

	// Default: pull from class Properties
	for (const FCodeForgePropertyDef& Prop : Blueprint->Properties)
	{
		if (Prop.Name == PropertyDef.Name)
		{
			PropertyDef = Prop;
			return;
		}
	}
}



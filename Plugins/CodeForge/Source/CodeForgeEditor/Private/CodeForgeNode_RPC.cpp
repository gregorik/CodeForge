#include "CodeForgeNode_RPC.h"
#include "CodeForgeBlueprint.h"

FText UCodeForgeNode_RPC::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("RPC: %s"), *RPCDef.Name));
}

FLinearColor UCodeForgeNode_RPC::GetNodeTitleColor() const
{
	return FLinearColor(0.9f, 0.1f, 0.1f);
}

void UCodeForgeNode_RPC::AllocateDefaultPins()
{
	FEdGraphPinType RPCPinType;
	RPCPinType.PinCategory = TEXT("PC_CodeForgeRPC");
	CreatePin(EGPD_Input, RPCPinType, TEXT("Owner"));
}

void UCodeForgeNode_RPC::SyncToBlueprint(UCodeForgeBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	Blueprint->RPCs.Add(RPCDef);
}

void UCodeForgeNode_RPC::SyncFromBlueprint(const UCodeForgeBlueprint* Blueprint)
{
}

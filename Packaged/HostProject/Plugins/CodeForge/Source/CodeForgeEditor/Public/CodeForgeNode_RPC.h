// Copyright (c) 2026 GregOrigin. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeRPCDef.h"
#include "CodeForgeNode_RPC.generated.h"

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeNode_RPC : public UCodeForgeEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CodeForge")
	FCodeForgeRPCDef RPCDef;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void AllocateDefaultPins() override;

	virtual void SyncToBlueprint(class UCodeForgeBlueprint* Blueprint) const override;
	virtual void SyncFromBlueprint(const class UCodeForgeBlueprint* Blueprint) override;
	virtual bool OwnsField(const FString& FieldName) const override { return FieldName == RPCDef.Name; }
};

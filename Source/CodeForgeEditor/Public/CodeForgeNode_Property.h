// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeNode_Property.generated.h"

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeNode_Property : public UCodeForgeEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CodeForge")
	FCodeForgePropertyDef PropertyDef;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void AllocateDefaultPins() override;

	virtual void SyncToBlueprint(class UCodeForgeBlueprint* Blueprint) const override;
	virtual void SyncFromBlueprint(const class UCodeForgeBlueprint* Blueprint) override;
	virtual bool OwnsField(const FString& FieldName) const override { return FieldName == PropertyDef.Name; }
};



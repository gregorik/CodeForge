// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeNode_Struct.generated.h"

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeNode_Struct : public UCodeForgeEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CodeForge")
	FString ClassName;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void AllocateDefaultPins() override;

	virtual void SyncToBlueprint(class UCodeForgeBlueprint* Blueprint) const override;
	virtual void SyncFromBlueprint(const class UCodeForgeBlueprint* Blueprint) override;
	virtual bool OwnsField(const FString& FieldName) const override { return FieldName == ClassName || FieldName.IsEmpty(); }
};



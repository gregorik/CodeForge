#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeTypes.h"
#include "CodeForgeNode_Class.generated.h"

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeNode_Class : public UCodeForgeEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CodeForge")
	FString ClassName;

	UPROPERTY(EditAnywhere, Category = "CodeForge")
	ECodeForgeClassType ClassType = ECodeForgeClassType::Actor;

	UPROPERTY(EditAnywhere, Category = "CodeForge")
	bool bReplicated = false;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void AllocateDefaultPins() override;

	virtual void SyncToBlueprint(class UCodeForgeBlueprint* Blueprint) const override;
	virtual void SyncFromBlueprint(const class UCodeForgeBlueprint* Blueprint) override;
	virtual bool OwnsField(const FString& FieldName) const override { return FieldName == ClassName || FieldName.IsEmpty(); }
};


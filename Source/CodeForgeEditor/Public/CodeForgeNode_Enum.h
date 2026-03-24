#pragma once

#include "CoreMinimal.h"
#include "CodeForgeEdGraphNode.h"
#include "CodeForgeEnumDef.h"
#include "CodeForgeNode_Enum.generated.h"

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeNode_Enum : public UCodeForgeEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CodeForge")
	FString ClassName;

	UPROPERTY(EditAnywhere, Category = "CodeForge")
	TArray<FCodeForgeEnumEntryDef> EnumEntries;

	UPROPERTY(EditAnywhere, Category = "CodeForge")
	bool bBlueprintType = true;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void AllocateDefaultPins() override;

	virtual void SyncToBlueprint(class UCodeForgeBlueprint* Blueprint) const override;
	virtual void SyncFromBlueprint(const class UCodeForgeBlueprint* Blueprint) override;
	virtual bool OwnsField(const FString& FieldName) const override { return FieldName == ClassName || FieldName.IsEmpty(); }
};


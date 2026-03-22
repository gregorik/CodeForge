#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "CodeForgeEdGraph.generated.h"

class UCodeForgeBlueprint;

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UCodeForgeBlueprint* CodeForgeBlueprint = nullptr;

	void SyncAllNodesToBlueprint();
	void SyncAllNodesFromBlueprint();
	void RunValidationOnNodes();
};

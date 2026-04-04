// Copyright (c) 2026 GregOrigin. All Rights Reserved.

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

	/** Creates graph nodes from blueprint data when opening a blueprint that has no graph yet. */
	void PopulateFromBlueprint();
};



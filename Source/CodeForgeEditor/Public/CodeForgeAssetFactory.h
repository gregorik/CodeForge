// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeAssetFactory.h
// UFactory subclass for creating UCodeForgeBlueprint assets from Content Browser.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CodeForgeAssetFactory.generated.h"

UCLASS()
class UCodeForgeAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UCodeForgeAssetFactory();

	virtual UObject* FactoryCreateNew(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn) override;
};



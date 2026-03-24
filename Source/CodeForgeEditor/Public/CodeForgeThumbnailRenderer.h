#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/ThumbnailRenderer.h"
#include "CodeForgeThumbnailRenderer.generated.h"

UCLASS()
class CODEFORGEEDITOR_API UCodeForgeThumbnailRenderer : public UThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual EThumbnailRenderFrequency GetThumbnailRenderFrequency(UObject* Object) const override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height,
		FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;
};


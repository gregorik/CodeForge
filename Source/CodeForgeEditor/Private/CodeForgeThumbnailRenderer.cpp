#include "CodeForgeThumbnailRenderer.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeSettings.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"

bool UCodeForgeThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object && Object->IsA<UCodeForgeBlueprint>();
}

EThumbnailRenderFrequency UCodeForgeThumbnailRenderer::GetThumbnailRenderFrequency(UObject* Object) const
{
	return EThumbnailRenderFrequency::Realtime;
}

void UCodeForgeThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y,
	uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas,
	bool bAdditionalViewFamily)
{
	UCodeForgeBlueprint* BP = Cast<UCodeForgeBlueprint>(Object);
	if (!BP || !Canvas) return;

	const UCodeForgeSettings* Settings = GetDefault<UCodeForgeSettings>();
	const float W = static_cast<float>(Width);
	const float H = static_cast<float>(Height);
	const float FX = static_cast<float>(X);
	const float FY = static_cast<float>(Y);

	// Kind-specific color, letter, and subtype
	FLinearColor KindColor;
	FString KindLetter;
	FString Subtype;
	int32 MemberCount = 0;
	switch (BP->BlueprintKind)
	{
	case ECodeForgeBlueprintKind::Class:
		KindColor = Settings->ClassNodeColor;
		KindLetter = TEXT("C");
		MemberCount = BP->Properties.Num() + BP->Functions.Num();
		// Show parent class type to distinguish class thumbnails
		Subtype = BP->GetParentClassName();
		// Strip the UE prefix (A/U) for display
		if (Subtype.Len() > 1 && (Subtype[0] == TEXT('A') || Subtype[0] == TEXT('U')))
		{
			Subtype.RightChopInline(1);
		}
		break;
	case ECodeForgeBlueprintKind::Struct:
		KindColor = Settings->StructNodeColor;
		KindLetter = TEXT("S");
		Subtype = TEXT("Struct");
		MemberCount = BP->StructProperties.Num();
		break;
	case ECodeForgeBlueprintKind::Enum:
		KindColor = Settings->EnumNodeColor;
		KindLetter = TEXT("E");
		Subtype = TEXT("Enum");
		MemberCount = BP->EnumEntries.Num();
		break;
	default:
		KindColor = FLinearColor::Gray;
		KindLetter = TEXT("?");
		break;
	}

	// =====================================================================
	// Background — solid dark with subtle kind-tinted vignette
	// =====================================================================
	FLinearColor BgColor(0.07f, 0.07f, 0.08f);
	FCanvasTileItem BgTile(FVector2D(FX, FY), FVector2D(W, H), BgColor);
	BgTile.BlendMode = SE_BLEND_Opaque;
	Canvas->DrawItem(BgTile);

	// Subtle tinted overlay in top half for depth
	FLinearColor TintOverlay = KindColor * 0.08f;
	TintOverlay.A = 1.f;
	FCanvasTileItem TintTile(FVector2D(FX, FY), FVector2D(W, H * 0.5f), TintOverlay);
	TintTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TintTile);

	// =====================================================================
	// Top accent stripe — thin, full-width, saturated kind color
	// =====================================================================
	float StripeH = FMath::Max(4.f, H * 0.05f);
	FCanvasTileItem TopStripe(FVector2D(FX, FY), FVector2D(W, StripeH), KindColor);
	TopStripe.BlendMode = SE_BLEND_Opaque;
	Canvas->DrawItem(TopStripe);

	// =====================================================================
	// Kind letter — dominant, centered, fills the thumbnail
	// =====================================================================
	UFont* LargeFont = GEngine ? GEngine->GetLargeFont() : nullptr;
	if (LargeFont)
	{
		float LetterScale = FMath::Clamp(W / 40.f, 2.5f, 5.0f);
		float CharW = 14.f * LetterScale;
		float CharH = 18.f * LetterScale;
		float LetterX = FX + (W - CharW) * 0.5f;
		float LetterY = FY + (H - CharH) * 0.38f;

		// Drop shadow
		FCanvasTextItem ShadowText(
			FVector2D(LetterX + 2.f, LetterY + 2.f),
			FText::FromString(KindLetter), LargeFont, FLinearColor(0.f, 0.f, 0.f, 0.5f));
		ShadowText.Scale = FVector2D(LetterScale, LetterScale);
		Canvas->DrawItem(ShadowText);

		// Main letter
		FCanvasTextItem LetterText(
			FVector2D(LetterX, LetterY),
			FText::FromString(KindLetter), LargeFont, KindColor);
		LetterText.Scale = FVector2D(LetterScale, LetterScale);
		Canvas->DrawItem(LetterText);
	}

	// =====================================================================
	// Subtype label — small text below the letter (e.g. "Character", "Actor")
	// =====================================================================
	UFont* SmallFont = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (SmallFont && !Subtype.IsEmpty())
	{
		float SubScale = FMath::Clamp(W / 160.f, 0.65f, 1.0f);
		float SubCharW = 6.5f * SubScale * Subtype.Len();
		float SubX = FX + (W - SubCharW) * 0.5f;
		float SubY = FY + H * 0.68f;

		FLinearColor SubColor = KindColor * 0.6f;
		SubColor.A = 1.f;
		FCanvasTextItem SubText(
			FVector2D(SubX, SubY),
			FText::FromString(Subtype), SmallFont, SubColor);
		SubText.Scale = FVector2D(SubScale, SubScale);
		Canvas->DrawItem(SubText);
	}

	// =====================================================================
	// Bottom bar — dark strip with member count
	// =====================================================================
	float BarH = FMath::Max(16.f, H * 0.15f);
	float BarY = FY + H - BarH;
	FLinearColor BarColor(0.04f, 0.04f, 0.05f);
	FCanvasTileItem BarTile(FVector2D(FX, BarY), FVector2D(W, BarH), BarColor);
	BarTile.BlendMode = SE_BLEND_Opaque;
	Canvas->DrawItem(BarTile);

	// Thin separator line at top of bar
	float SepH = FMath::Max(1.f, H * 0.01f);
	FLinearColor SepColor = KindColor * 0.4f;
	SepColor.A = 1.f;
	FCanvasTileItem SepTile(FVector2D(FX, BarY), FVector2D(W, SepH), SepColor);
	SepTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(SepTile);

	// Member count in bottom bar
	if (SmallFont && MemberCount > 0)
	{
		FString CountStr = FString::Printf(TEXT("%d members"), MemberCount);
		float CountScale = FMath::Clamp(W / 170.f, 0.6f, 0.9f);
		float CountY = BarY + (BarH - 10.f * CountScale) * 0.5f;
		FCanvasTextItem CountText(
			FVector2D(FX + W * 0.1f, CountY),
			FText::FromString(CountStr), SmallFont, FLinearColor(0.6f, 0.6f, 0.6f));
		CountText.Scale = FVector2D(CountScale, CountScale);
		Canvas->DrawItem(CountText);
	}
	else if (SmallFont && MemberCount == 0)
	{
		float EmptyScale = FMath::Clamp(W / 170.f, 0.6f, 0.9f);
		float EmptyY = BarY + (BarH - 10.f * EmptyScale) * 0.5f;
		FCanvasTextItem EmptyText(
			FVector2D(FX + W * 0.1f, EmptyY),
			FText::FromString(TEXT("empty")), SmallFont, FLinearColor(0.35f, 0.35f, 0.35f));
		EmptyText.Scale = FVector2D(EmptyScale, EmptyScale);
		Canvas->DrawItem(EmptyText);
	}

	// =====================================================================
	// Left accent edge
	// =====================================================================
	float EdgeW = FMath::Max(3.f, W * 0.03f);
	FCanvasTileItem LeftEdge(FVector2D(FX, FY), FVector2D(EdgeW, H), KindColor);
	LeftEdge.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(LeftEdge);
}


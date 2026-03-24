// CodeForgeChangeDetector.h
// Tracks manifest entries for generated classes and detects whether a
// regeneration constitutes a structural change (requiring hot-reload /
// live-coding) or merely a behavioral change.

#pragma once

#include "CoreMinimal.h"

/**
 * A single entry in the generation manifest, capturing the structural
 * signature of a generated class at the time it was last written to disk.
 */
struct FCodeForgeManifestEntry
{
	FString ClassName;
	FString ParentClassName;
	TArray<FString> PropertyNames;
	TArray<FString> PropertyTypes;
	TArray<FString> FunctionNames;
};

/**
 * FCodeForgeChangeDetector
 *
 * Maintains a JSON manifest of previously generated class signatures.
 * When code is regenerated, the detector compares the new signature
 * against the stored one to classify the change as "structural"
 * (interface / layout changed) or "behavioral" (only bodies changed).
 */
class CODEFORGEEDITOR_API FCodeForgeChangeDetector
{
public:
	/**
	 * Load the generation manifest from
	 * {ProjectDir}/Saved/CodeForge/generation_manifest.json.
	 */
	void LoadManifest(const FString& ProjectDir);

	/**
	 * Save the current manifest to
	 * {ProjectDir}/Saved/CodeForge/generation_manifest.json.
	 * Creates directories as needed.
	 */
	void SaveManifest(const FString& ProjectDir);

	/**
	 * Compare NewEntry against the stored entry for ClassName.
	 * Returns true when any structural aspect changed (parent class,
	 * properties, functions), or when no previous entry exists.
	 */
	bool IsStructuralChange(const FString& ClassName, const FCodeForgeManifestEntry& NewEntry) const;

	/**
	 * Store or overwrite the manifest entry for the given class.
	 */
	void UpdateEntry(const FString& ClassName, const FCodeForgeManifestEntry& Entry);

private:
	/** Map from ClassName to its manifest entry. */
	TMap<FString, FCodeForgeManifestEntry> Entries;

	/** Cached absolute path to the manifest file. */
	FString ManifestPath;
};



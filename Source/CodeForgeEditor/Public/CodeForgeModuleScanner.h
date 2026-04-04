// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeModuleScanner.h
// Scans an Unreal project directory to discover game modules, their
// source/public/private directories, existing subdirectories, and the
// derived API export macro name.

#pragma once

#include "CoreMinimal.h"

/**
 * Information about a single discovered module within the project.
 */
struct FCodeForgeModuleInfo
{
	FString ModuleName;
	FString SourceDir;
	FString PublicDir;
	FString PrivateDir;
	FString APIMacro;
	TArray<FString> ExistingSubdirectories;
};

/**
 * FCodeForgeModuleScanner
 *
 * Reads a .uproject file to enumerate declared modules, then locates
 * each module's Source directory, Public/Private layout, and existing
 * subdirectories.  The scanner also derives the MODULE_API export
 * macro from the module name.
 */
class CODEFORGEEDITOR_API FCodeForgeModuleScanner
{
public:
	/**
	 * Scan the given project directory for modules.
	 *
	 * @param ProjectDir  Absolute path to the project root (containing .uproject).
	 * @return            Array of discovered module descriptors.
	 */
	TArray<FCodeForgeModuleInfo> ScanProject(const FString& ProjectDir) const;

private:
	/** Convert a module name to its API export macro (e.g. "MyGame" -> "MYGAME_API"). */
	FString DeriveAPIMacro(const FString& ModuleName) const;

	/** Find immediate subdirectories under the given directory. */
	TArray<FString> FindSubdirectories(const FString& Dir) const;
};



// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeModuleScanner.cpp
// Implements FCodeForgeModuleScanner â€” discovers project modules by reading
// the .uproject file and scanning Source/ for *.Build.cs files.

#include "CodeForgeModuleScanner.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// ---------------------------------------------------------------------------
// ScanProject
// ---------------------------------------------------------------------------

TArray<FCodeForgeModuleInfo> FCodeForgeModuleScanner::ScanProject(const FString& ProjectDir) const
{
	TArray<FCodeForgeModuleInfo> Result;

	if (!FPaths::DirectoryExists(ProjectDir))
	{
		return Result;
	}

	// 1. Find the .uproject file
	TArray<FString> UProjectFiles;
	IFileManager::Get().FindFiles(UProjectFiles, *FPaths::Combine(ProjectDir, TEXT("*.uproject")), true, false);

	if (UProjectFiles.Num() == 0)
	{
		return Result;
	}

	FString UProjectPath = FPaths::Combine(ProjectDir, UProjectFiles[0]);

	// 2. Parse the .uproject JSON
	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *UProjectPath))
	{
		return Result;
	}

	TSharedPtr<FJsonObject> JsonRoot;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
	{
		return Result;
	}

	// 3. Extract declared module names from the "Modules" array
	TArray<FString> DeclaredModuleNames;
	const TArray<TSharedPtr<FJsonValue>>* ModulesArray = nullptr;
	if (JsonRoot->TryGetArrayField(TEXT("Modules"), ModulesArray))
	{
		for (const TSharedPtr<FJsonValue>& ModuleValue : *ModulesArray)
		{
			const TSharedPtr<FJsonObject>* ModuleObj = nullptr;
			if (ModuleValue->TryGetObject(ModuleObj))
			{
				FString ModName;
				if ((*ModuleObj)->TryGetStringField(TEXT("Name"), ModName) && !ModName.IsEmpty())
				{
					DeclaredModuleNames.Add(ModName);
				}
			}
		}
	}

	// 4. For each declared module, find its Build.cs under Source/
	FString SourceRoot = FPaths::Combine(ProjectDir, TEXT("Source"));

	for (const FString& ModuleName : DeclaredModuleNames)
	{
		// Look for Source/<ModuleName>/<ModuleName>.Build.cs
		FString ExpectedBuildCs = FPaths::Combine(
			SourceRoot, ModuleName, ModuleName + TEXT(".Build.cs"));

		if (!FPaths::FileExists(ExpectedBuildCs))
		{
			continue;
		}

		FCodeForgeModuleInfo Info;
		Info.ModuleName = ModuleName;
		Info.SourceDir  = FPaths::Combine(SourceRoot, ModuleName);
		Info.APIMacro    = DeriveAPIMacro(ModuleName);

		// Determine Public/Private dirs â€” fall back to SourceDir if they don't exist on disk
		FString CandidatePublic  = FPaths::Combine(Info.SourceDir, TEXT("Public"));
		FString CandidatePrivate = FPaths::Combine(Info.SourceDir, TEXT("Private"));
		Info.PublicDir  = FPaths::DirectoryExists(CandidatePublic)  ? CandidatePublic  : Info.SourceDir;
		Info.PrivateDir = FPaths::DirectoryExists(CandidatePrivate) ? CandidatePrivate : Info.SourceDir;

		// Normalize all paths
		FPaths::NormalizeFilename(Info.SourceDir);
		FPaths::NormalizeFilename(Info.PublicDir);
		FPaths::NormalizeFilename(Info.PrivateDir);

		// Discover existing subdirectories under Public/
		if (FPaths::DirectoryExists(Info.PublicDir))
		{
			Info.ExistingSubdirectories = FindSubdirectories(Info.PublicDir);
		}

		Result.Add(MoveTemp(Info));
	}

	return Result;
}

// ---------------------------------------------------------------------------
// DeriveAPIMacro
// ---------------------------------------------------------------------------

FString FCodeForgeModuleScanner::DeriveAPIMacro(const FString& ModuleName) const
{
	return ModuleName.ToUpper() + TEXT("_API");
}

// ---------------------------------------------------------------------------
// FindSubdirectories
// ---------------------------------------------------------------------------

TArray<FString> FCodeForgeModuleScanner::FindSubdirectories(const FString& Dir) const
{
	TArray<FString> SubDirs;

	IFileManager& FM = IFileManager::Get();
	FM.IterateDirectory(*Dir, [&SubDirs](const TCHAR* Path, bool bIsDirectory) -> bool
	{
		if (bIsDirectory)
		{
			SubDirs.Add(FPaths::GetCleanFilename(Path));
		}
		return true; // continue iteration
	});

	return SubDirs;
}



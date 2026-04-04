// Copyright (c) 2026 GregOrigin. All Rights Reserved.

// CodeForgeChangeDetector.cpp
// Implements FCodeForgeChangeDetector â€” loads, saves, and compares the
// generation manifest to classify regenerations as structural or behavioral.

#include "CodeForgeChangeDetector.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// ---------------------------------------------------------------------------
// LoadManifest
// ---------------------------------------------------------------------------

void FCodeForgeChangeDetector::LoadManifest(const FString& ProjectDir)
{
	Entries.Empty();

	ManifestPath = FPaths::Combine(
		ProjectDir, TEXT("Saved"), TEXT("CodeForge"), TEXT("generation_manifest.json"));
	FPaths::NormalizeFilename(ManifestPath);

	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *ManifestPath))
	{
		// No manifest yet â€” first run.  Entries stays empty.
		return;
	}

	TSharedPtr<FJsonObject> JsonRoot;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
	{
		return;
	}

	const TSharedPtr<FJsonObject>* EntriesObj = nullptr;
	if (!JsonRoot->TryGetObjectField(TEXT("Entries"), EntriesObj))
	{
		return;
	}

	for (const auto& Pair : (*EntriesObj)->Values)
	{
		const TSharedPtr<FJsonObject>* EntryObj = nullptr;
		if (!Pair.Value->TryGetObject(EntryObj))
		{
			continue;
		}

		FCodeForgeManifestEntry Entry;
		(*EntryObj)->TryGetStringField(TEXT("ClassName"), Entry.ClassName);
		(*EntryObj)->TryGetStringField(TEXT("ParentClassName"), Entry.ParentClassName);

		// PropertyNames
		const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
		if ((*EntryObj)->TryGetArrayField(TEXT("PropertyNames"), Arr))
		{
			for (const TSharedPtr<FJsonValue>& Val : *Arr)
			{
				FString Str;
				if (Val->TryGetString(Str))
				{
					Entry.PropertyNames.Add(Str);
				}
			}
		}

		// PropertyTypes
		if ((*EntryObj)->TryGetArrayField(TEXT("PropertyTypes"), Arr))
		{
			for (const TSharedPtr<FJsonValue>& Val : *Arr)
			{
				FString Str;
				if (Val->TryGetString(Str))
				{
					Entry.PropertyTypes.Add(Str);
				}
			}
		}

		// FunctionNames
		if ((*EntryObj)->TryGetArrayField(TEXT("FunctionNames"), Arr))
		{
			for (const TSharedPtr<FJsonValue>& Val : *Arr)
			{
				FString Str;
				if (Val->TryGetString(Str))
				{
					Entry.FunctionNames.Add(Str);
				}
			}
		}

		Entries.Add(Pair.Key, MoveTemp(Entry));
	}
}

// ---------------------------------------------------------------------------
// SaveManifest
// ---------------------------------------------------------------------------

void FCodeForgeChangeDetector::SaveManifest(const FString& ProjectDir)
{
	ManifestPath = FPaths::Combine(
		ProjectDir, TEXT("Saved"), TEXT("CodeForge"), TEXT("generation_manifest.json"));
	FPaths::NormalizeFilename(ManifestPath);

	// Ensure directory exists
	IFileManager& FM = IFileManager::Get();
	FM.MakeDirectory(*FPaths::GetPath(ManifestPath), /*Tree=*/true);

	// Build JSON
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	TSharedRef<FJsonObject> EntriesObj = MakeShared<FJsonObject>();

	for (const auto& Pair : Entries)
	{
		TSharedRef<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("ClassName"), Pair.Value.ClassName);
		EntryObj->SetStringField(TEXT("ParentClassName"), Pair.Value.ParentClassName);

		// PropertyNames
		TArray<TSharedPtr<FJsonValue>> PropNamesArr;
		for (const FString& Name : Pair.Value.PropertyNames)
		{
			PropNamesArr.Add(MakeShared<FJsonValueString>(Name));
		}
		EntryObj->SetArrayField(TEXT("PropertyNames"), PropNamesArr);

		// PropertyTypes
		TArray<TSharedPtr<FJsonValue>> PropTypesArr;
		for (const FString& TypeStr : Pair.Value.PropertyTypes)
		{
			PropTypesArr.Add(MakeShared<FJsonValueString>(TypeStr));
		}
		EntryObj->SetArrayField(TEXT("PropertyTypes"), PropTypesArr);

		// FunctionNames
		TArray<TSharedPtr<FJsonValue>> FuncNamesArr;
		for (const FString& Name : Pair.Value.FunctionNames)
		{
			FuncNamesArr.Add(MakeShared<FJsonValueString>(Name));
		}
		EntryObj->SetArrayField(TEXT("FunctionNames"), FuncNamesArr);

		EntriesObj->SetObjectField(Pair.Key, EntryObj);
	}

	Root->SetObjectField(TEXT("Entries"), EntriesObj);

	// Serialize and write
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root, Writer);

	FFileHelper::SaveStringToFile(OutputString, *ManifestPath);
}

// ---------------------------------------------------------------------------
// IsStructuralChange
// ---------------------------------------------------------------------------

bool FCodeForgeChangeDetector::IsStructuralChange(
	const FString& ClassName, const FCodeForgeManifestEntry& NewEntry) const
{
	const FCodeForgeManifestEntry* Existing = Entries.Find(ClassName);
	if (!Existing)
	{
		// First generation â€” always structural
		return true;
	}

	// Compare ClassName
	if (Existing->ClassName != NewEntry.ClassName)
	{
		return true;
	}

	// Compare ParentClassName
	if (Existing->ParentClassName != NewEntry.ParentClassName)
	{
		return true;
	}

	// Compare PropertyNames
	if (Existing->PropertyNames != NewEntry.PropertyNames)
	{
		return true;
	}

	// Compare PropertyTypes
	if (Existing->PropertyTypes != NewEntry.PropertyTypes)
	{
		return true;
	}

	// Compare FunctionNames
	if (Existing->FunctionNames != NewEntry.FunctionNames)
	{
		return true;
	}

	// No structural differences detected â€” behavioral only
	return false;
}

// ---------------------------------------------------------------------------
// UpdateEntry
// ---------------------------------------------------------------------------

void FCodeForgeChangeDetector::UpdateEntry(
	const FString& ClassName, const FCodeForgeManifestEntry& Entry)
{
	Entries.Add(ClassName, Entry);
}



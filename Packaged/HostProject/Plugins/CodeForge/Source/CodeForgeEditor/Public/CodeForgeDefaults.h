// Copyright (c) 2026 GregOrigin. All Rights Reserved.
// CodeForgeDefaults.h
// Single source of truth for default specifiers and "Common" specifier sets used by
// both the wizard's starter content and the Details panel's Common/Advanced split.
#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.h" // ECodeForgeBlueprintKind, ECodeForgeClassType

namespace CodeForgeDefaults
{
    // -------------------------------------------------------------------------
    // Default specifiers used when the wizard's "Include starter content" is ON.
    // -------------------------------------------------------------------------

    /** Class specifier set added to new Actor/Component/Object classes. */
    static const TArray<FString> DefaultClassSpecifiers = { TEXT("Blueprintable"), TEXT("BlueprintType") };

    /** Property specifiers applied to the starter Property node. */
    struct FDefaultPropertySpec
    {
        bool bEditAnywhere     = true;
        bool bBlueprintReadWrite = true;
        bool bReplicated       = false;
        FString DefaultName    = TEXT("bIsActive");
        FString DefaultType    = TEXT("bool");
    };

    /** Function specifiers applied to the starter Function node. */
    struct FDefaultFunctionSpec
    {
        bool bBlueprintCallable = true;
        bool bBlueprintPure    = false;
        FString DefaultName    = TEXT("DoSomething");
        FString DefaultReturnType = TEXT("void");
    };

    /** Struct member defaults. */
    struct FDefaultStructMembers
    {
        TArray<TPair<FString, FString>> Members = {
            { TEXT("Name"),  TEXT("FString") },
            { TEXT("Value"), TEXT("int32") }
        };
    };

    /** Enum member defaults. */
    struct FDefaultEnumValues
    {
        TArray<FString> Values = { TEXT("None"), TEXT("First"), TEXT("Second") };
    };

    // -------------------------------------------------------------------------
    // Curated parent-class lists for the wizard's Step 2.
    // -------------------------------------------------------------------------

    static const TArray<FString> CommonActorParents = {
        TEXT("AActor"), TEXT("APawn"), TEXT("ACharacter"),
        TEXT("APlayerController"), TEXT("AGameModeBase"),
        TEXT("APlayerState"), TEXT("AGameStateBase"), TEXT("AHUD"),
        TEXT("ASceneCapture"), TEXT("ATriggerVolume")
    };

    static const TArray<FString> CommonComponentParents = {
        TEXT("UActorComponent"), TEXT("USceneComponent"),
        TEXT("UStaticMeshComponent"), TEXT("USkeletalMeshComponent"),
        TEXT("UCameraComponent"), TEXT("USpringArmComponent"),
        TEXT("UAudioComponent"), TEXT("UParticleSystemComponent"),
        TEXT("UCapsuleComponent"), TEXT("UBoxComponent")
    };

    static const TArray<FString> CommonObjectParents = {
        TEXT("UObject"), TEXT("UDataAsset"), TEXT("UPrimaryDataAsset"),
        TEXT("USaveGame"), TEXT("UGameInstanceSubsystem"),
        TEXT("UWorldSubsystem"), TEXT("ULocalPlayerSubsystem"),
        TEXT("UDeveloperSettings"), TEXT("UEngineSubsystem"),
        TEXT("UEditorSubsystem")
    };

    /** Default selection per archetype (the most-likely-wanted parent). */
    static const FString DefaultActorParent     = TEXT("AActor");
    static const FString DefaultComponentParent = TEXT("UActorComponent");
    static const FString DefaultObjectParent    = TEXT("UObject");

    // -------------------------------------------------------------------------
    // "Common" specifier sets for the Details panel split.
    // -------------------------------------------------------------------------

    /** Property fields that appear in the Common tab. Matched by FName == UPROPERTY name. */
    static const TArray<FName> CommonPropertyFields = {
        TEXT("Name"), TEXT("Type"), TEXT("Category"),
        TEXT("bEditAnywhere"), TEXT("bBlueprintReadWrite"), TEXT("bReplicated")
    };

    static const TArray<FName> CommonFunctionFields = {
        TEXT("Name"), TEXT("ReturnType"), TEXT("Parameters"),
        TEXT("bBlueprintCallable"), TEXT("bBlueprintPure"), TEXT("RPCType")
    };

    static const TArray<FName> CommonClassFields = {
        TEXT("ClassName"), TEXT("ClassType"),
        TEXT("bBlueprintable"), TEXT("bBlueprintType"), TEXT("bAbstract")
    };

    static const TArray<FName> CommonRPCFields = {
        TEXT("Name"), TEXT("Parameters"), TEXT("RPCType"),
        TEXT("bReliable"), TEXT("bValidated")
    };

    static const TArray<FName> CommonStructFields = {
        TEXT("StructName"), TEXT("bBlueprintType"), TEXT("Members")
    };

    static const TArray<FName> CommonEnumFields = {
        TEXT("EnumName"), TEXT("bBlueprintType"), TEXT("Entries"), TEXT("bBitflag")
    };
}

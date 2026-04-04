// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CodeForgeTypes.generated.h"

UENUM(BlueprintType)
enum class ECodeForgeBlueprintKind : uint8
{
    Class,
    Struct,
    Enum
};

UENUM(BlueprintType)
enum class ECodeForgeClassType : uint8
{
    Actor,
    Pawn,
    Character,
    ActorComponent,
    SceneComponent,
    Object,
    GameModeBase,
    GameStateBase,
    PlayerController,
    PlayerState,
    HUD
};

UENUM(BlueprintType)
enum class ECodeForgeValidationSeverity : uint8
{
    Error,
    Warning
};

UENUM(BlueprintType)
enum class ECodeForgeRepCondition : uint8
{
    None,
    InitialOnly,
    OwnerOnly,
    SkipOwner,
    SimulatedOnly,
    AutonomousOnly,
    SimulatedOrPhysics,
    InitialOrOwner,
    Custom,
    ReplayOrOwner,
    ReplayOnly,
    SimulatedOnlyNoReplay,
    SimulatedOrPhysicsNoReplay,
    SkipReplay,
    Dynamic,
    Never
};



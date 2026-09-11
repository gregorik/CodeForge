// Copyright (c) 2026 GregOrigin. All Rights Reserved.
// CodeForgeSpecifierCopy.h
// Plain-English tooltip strings for graph-node specifier UPROPERTY fields.
// Looked up by (node kind, property name).
#pragma once

#include "CoreMinimal.h"

namespace CodeForgeSpecifierCopy
{
    enum class ESpecifierNodeKind : uint8
    {
        Property,
        Function,
        Class,
        RPC,
        Struct,
        Enum
    };

    /**
     * Returns a plain-English tooltip for the given (node kind, field name) pair,
     * or an empty FString when no entry is registered. Callers should fall back
     * to the engine's default UPROPERTY tooltip in that case.
     */
    CODEFORGEEDITOR_API FString LookupTooltip(ESpecifierNodeKind Kind, FName FieldName);
}

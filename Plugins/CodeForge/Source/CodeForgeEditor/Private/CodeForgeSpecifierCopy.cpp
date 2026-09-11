// Copyright (c) 2026 GregOrigin. All Rights Reserved.
// CodeForgeSpecifierCopy.cpp
#include "CodeForgeSpecifierCopy.h"

#include "UObject/NameTypes.h"

namespace CodeForgeSpecifierCopy
{
    namespace
    {
        struct FCopyKey
        {
            ESpecifierNodeKind Kind;
            FName              Field;

            bool operator==(const FCopyKey& Other) const
            {
                return Kind == Other.Kind && Field == Other.Field;
            }
        };

        FORCEINLINE uint32 GetTypeHash(const FCopyKey& Key)
        {
            return HashCombine(::GetTypeHash((uint8)Key.Kind), GetTypeHash(Key.Field));
        }

        const TMap<FCopyKey, FString>& GetCopyMap()
        {
            static const TMap<FCopyKey, FString> Map = []()
            {
                TMap<FCopyKey, FString> M;

                // Property
                M.Add({ ESpecifierNodeKind::Property, TEXT("bEditAnywhere") },
                      TEXT("Lets designers edit this in the Details panel of every instance.\nUPROPERTY(EditAnywhere)"));
                M.Add({ ESpecifierNodeKind::Property, TEXT("bBlueprintReadWrite") },
                      TEXT("Lets Blueprints read and write this from anywhere.\nMyActor.bIsActive = true;"));
                M.Add({ ESpecifierNodeKind::Property, TEXT("bReplicated") },
                      TEXT("Server changes are sent to clients automatically. Required for multiplayer state."));
                M.Add({ ESpecifierNodeKind::Property, TEXT("Category") },
                      TEXT("Groups this property in the Details panel. Use the same Category to group related fields."));

                // Function
                M.Add({ ESpecifierNodeKind::Function, TEXT("bBlueprintCallable") },
                      TEXT("Lets Blueprints call this function. The function will appear in the Blueprint right-click menu."));
                M.Add({ ESpecifierNodeKind::Function, TEXT("bBlueprintPure") },
                      TEXT("This function is side-effect-free and is allowed inline in Blueprint expressions (no execution pin)."));
                M.Add({ ESpecifierNodeKind::Function, TEXT("RPCType") },
                      TEXT("Network direction. Server = client→server. Client = server→one client. NetMulticast = server→all clients."));

                // Class
                M.Add({ ESpecifierNodeKind::Class, TEXT("bBlueprintable") },
                      TEXT("Lets users derive new Blueprint classes from this C++ class."));
                M.Add({ ESpecifierNodeKind::Class, TEXT("bBlueprintType") },
                      TEXT("Lets this class be used as a Blueprint variable type."));
                M.Add({ ESpecifierNodeKind::Class, TEXT("bAbstract") },
                      TEXT("This class cannot be instantiated directly. Useful as a base class only."));

                // RPC
                M.Add({ ESpecifierNodeKind::RPC, TEXT("bReliable") },
                      TEXT("Reliable RPCs are guaranteed to arrive in order. Unreliable RPCs may be dropped (use for cosmetic events)."));
                M.Add({ ESpecifierNodeKind::RPC, TEXT("bValidated") },
                      TEXT("Adds a server-side validation step. The function won't run if validation rejects the call."));

                // Struct
                M.Add({ ESpecifierNodeKind::Struct, TEXT("bBlueprintType") },
                      TEXT("Lets Blueprints declare variables of this struct type."));

                // Enum
                M.Add({ ESpecifierNodeKind::Enum, TEXT("bBlueprintType") },
                      TEXT("Lets Blueprints declare variables of this enum type."));
                M.Add({ ESpecifierNodeKind::Enum, TEXT("bBitflag") },
                      TEXT("Lets values of this enum be combined with bitwise OR (|). Each entry must be a power of two."));

                return M;
            }();

            return Map;
        }
    }

    FString LookupTooltip(ESpecifierNodeKind Kind, FName FieldName)
    {
        const FString* Found = GetCopyMap().Find({ Kind, FieldName });
        return Found ? *Found : FString();
    }
}

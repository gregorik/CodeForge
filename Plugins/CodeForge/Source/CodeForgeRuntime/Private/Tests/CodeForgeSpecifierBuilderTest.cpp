#include "Misc/AutomationTest.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgePropertyDef.h"
#include "CodeForgeFunctionDef.h"
#include "CodeForgeRPCDef.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Property specifier tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_PropertySpec_BasicEditAndBlueprint,
    "CodeForge.Schema.Property.BasicEditAndBlueprint",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_PropertySpec_BasicEditAndBlueprint::RunTest(const FString& Parameters)
{
    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Health");
    Prop.Type = TEXT("float");
    Prop.bEditAnywhere = true;
    Prop.bBlueprintReadWrite = true;
    Prop.Category = TEXT("Stats");

    const FString Result = Prop.BuildSpecifierString();

    TestTrue(TEXT("Contains EditAnywhere"), Result.Contains(TEXT("EditAnywhere")));
    TestTrue(TEXT("Contains BlueprintReadWrite"), Result.Contains(TEXT("BlueprintReadWrite")));
    TestTrue(TEXT("Contains Category"), Result.Contains(TEXT("Category=\"Stats\"")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_PropertySpec_ReplicatedWithRepNotify,
    "CodeForge.Schema.Property.ReplicatedWithRepNotify",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_PropertySpec_ReplicatedWithRepNotify::RunTest(const FString& Parameters)
{
    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Health");
    Prop.Type = TEXT("float");
    Prop.bReplicated = true;
    Prop.bRepNotify = true;

    const FString Result = Prop.BuildSpecifierString();

    TestTrue(TEXT("Contains ReplicatedUsing=OnRep_Health"),
        Result.Contains(TEXT("ReplicatedUsing=OnRep_Health")));
    // "ReplicatedUsing=..." must appear but never a bare "Replicated" token.
    // We check by stripping the "ReplicatedUsing" part and verifying no leftover "Replicated".
    FString Stripped = Result.Replace(TEXT("ReplicatedUsing"), TEXT("REPLACED"));
    TestFalse(TEXT("Does not contain bare Replicated token"), Stripped.Contains(TEXT("Replicated")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_PropertySpec_ReplicatedWithoutNotify,
    "CodeForge.Schema.Property.ReplicatedWithoutNotify",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_PropertySpec_ReplicatedWithoutNotify::RunTest(const FString& Parameters)
{
    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Ammo");
    Prop.Type = TEXT("int32");
    Prop.bReplicated = true;
    Prop.bRepNotify = false;

    const FString Result = Prop.BuildSpecifierString();

    TestTrue(TEXT("Contains Replicated"), Result.Contains(TEXT("Replicated")));
    TestFalse(TEXT("Does not contain ReplicatedUsing"), Result.Contains(TEXT("ReplicatedUsing")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_PropertySpec_MetaTags,
    "CodeForge.Schema.Property.MetaTags",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_PropertySpec_MetaTags::RunTest(const FString& Parameters)
{
    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Speed");
    Prop.Type = TEXT("float");
    Prop.Meta.Add(TEXT("ClampMin"), TEXT("0"));
    Prop.Meta.Add(TEXT("ClampMax"), TEXT("100"));

    const FString Result = Prop.BuildSpecifierString();

    TestTrue(TEXT("Contains meta=(...)"), Result.Contains(TEXT("meta=(")));
    TestTrue(TEXT("Contains ClampMin=0"), Result.Contains(TEXT("ClampMin=0")));
    TestTrue(TEXT("Contains ClampMax=100"), Result.Contains(TEXT("ClampMax=100")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_PropertySpec_MutuallyExclusive,
    "CodeForge.Schema.Property.MutuallyExclusiveEditSpecifiers",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_PropertySpec_MutuallyExclusive::RunTest(const FString& Parameters)
{
    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Level");
    Prop.Type = TEXT("int32");
    Prop.bEditAnywhere = true;
    Prop.bEditDefaultsOnly = true; // both true — EditAnywhere should win

    const FString Result = Prop.BuildSpecifierString();

    TestTrue(TEXT("Contains EditAnywhere"), Result.Contains(TEXT("EditAnywhere")));
    TestFalse(TEXT("Does not contain EditDefaultsOnly"), Result.Contains(TEXT("EditDefaultsOnly")));

    return true;
}

// ---------------------------------------------------------------------------
// Default-value suffix tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_DefaultValueSuffix,
    "CodeForge.Schema.Property.DefaultValueSuffix",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_DefaultValueSuffix::RunTest(const FString& Parameters)
{
    FCodeForgePropertyDef Prop;
    Prop.Name = TEXT("Speed");
    Prop.Type = TEXT("float");

    // Empty default → empty suffix
    Prop.DefaultValue = TEXT("");
    TestEqual(TEXT("Empty default gives empty suffix"), Prop.BuildDefaultValueSuffix(), FString());

    // Non-empty default → " = <value>"
    Prop.DefaultValue = TEXT("0.0f");
    TestEqual(TEXT("Non-empty default gives ' = 0.0f'"), Prop.BuildDefaultValueSuffix(), FString(TEXT(" = 0.0f")));

    Prop.DefaultValue = TEXT("100");
    TestEqual(TEXT("Int default gives ' = 100'"), Prop.BuildDefaultValueSuffix(), FString(TEXT(" = 100")));

    return true;
}

// ---------------------------------------------------------------------------
// Function specifier tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_FunctionSpec_BlueprintCallable,
    "CodeForge.Schema.Function.BlueprintCallable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_FunctionSpec_BlueprintCallable::RunTest(const FString& Parameters)
{
    FCodeForgeFunctionDef Func;
    Func.Name = TEXT("Fire");
    Func.bBlueprintCallable = true;
    Func.Category = TEXT("Combat");

    const FString Result = Func.BuildSpecifierString();

    TestTrue(TEXT("Contains BlueprintCallable"), Result.Contains(TEXT("BlueprintCallable")));
    TestTrue(TEXT("Contains Category=\"Combat\""), Result.Contains(TEXT("Category=\"Combat\"")));
    TestFalse(TEXT("Does not contain BlueprintPure"), Result.Contains(TEXT("BlueprintPure")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_FunctionSpec_BlueprintNativeEvent,
    "CodeForge.Schema.Function.BlueprintNativeEvent",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_FunctionSpec_BlueprintNativeEvent::RunTest(const FString& Parameters)
{
    FCodeForgeFunctionDef Func;
    Func.Name = TEXT("OnDamaged");
    Func.bBlueprintCallable = true;
    Func.bBlueprintNativeEvent = true;
    Func.Category = TEXT("Events");

    const FString Result = Func.BuildSpecifierString();

    TestTrue(TEXT("Contains BlueprintNativeEvent"), Result.Contains(TEXT("BlueprintNativeEvent")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_FunctionSpec_ParamList,
    "CodeForge.Schema.Function.ParamList",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_FunctionSpec_ParamList::RunTest(const FString& Parameters)
{
    FCodeForgeFunctionDef Func;
    Func.Name = TEXT("SetData");

    // Param 0: const FString& InName
    FCodeForgeParamDef P0;
    P0.Name = TEXT("InName");
    P0.Type = TEXT("FString");
    P0.bIsConst = true;
    P0.bIsRef = true;

    // Param 1: float InValue  (neither const nor ref)
    FCodeForgeParamDef P1;
    P1.Name = TEXT("InValue");
    P1.Type = TEXT("float");
    P1.bIsConst = false;
    P1.bIsRef = false;

    // Param 2: int32& OutCount  (ref only)
    FCodeForgeParamDef P2;
    P2.Name = TEXT("OutCount");
    P2.Type = TEXT("int32");
    P2.bIsConst = false;
    P2.bIsRef = true;

    Func.Parameters = {P0, P1, P2};

    const FString Result = Func.BuildParamListString();

    TestTrue(TEXT("Contains const FString& InName"), Result.Contains(TEXT("const FString& InName")));
    TestTrue(TEXT("Contains float InValue"), Result.Contains(TEXT("float InValue")));
    TestTrue(TEXT("Contains int32& OutCount"), Result.Contains(TEXT("int32& OutCount")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_FunctionSpec_BlueprintPureMutualExclusion,
    "CodeForge.Schema.Function.BlueprintPureExcludesCallable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_FunctionSpec_BlueprintPureMutualExclusion::RunTest(const FString& Parameters)
{
    FCodeForgeFunctionDef Func;
    Func.Name = TEXT("GetHealth");
    Func.bBlueprintCallable = true;
    Func.bBlueprintPure = true; // Pure should win

    const FString Result = Func.BuildSpecifierString();

    TestTrue(TEXT("Contains BlueprintPure"), Result.Contains(TEXT("BlueprintPure")));
    TestFalse(TEXT("Does not contain BlueprintCallable when Pure"), Result.Contains(TEXT("BlueprintCallable")));

    return true;
}

// ---------------------------------------------------------------------------
// RPC specifier tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_RPCSpec_ServerReliable,
    "CodeForge.Schema.RPC.ServerReliable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_RPCSpec_ServerReliable::RunTest(const FString& Parameters)
{
    FCodeForgeRPCDef RPC;
    RPC.Name = TEXT("ServerFire");
    RPC.Mode = ECodeForgeRPCMode::Server;
    RPC.bReliable = true;

    const FString Result = RPC.BuildSpecifierString();

    TestTrue(TEXT("Contains Server"), Result.Contains(TEXT("Server")));
    TestTrue(TEXT("Contains Reliable"), Result.Contains(TEXT("Reliable")));
    TestTrue(TEXT("Contains WithValidation"), Result.Contains(TEXT("WithValidation")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_RPCSpec_ClientUnreliable,
    "CodeForge.Schema.RPC.ClientUnreliable",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_RPCSpec_ClientUnreliable::RunTest(const FString& Parameters)
{
    FCodeForgeRPCDef RPC;
    RPC.Name = TEXT("ClientPlayEffect");
    RPC.Mode = ECodeForgeRPCMode::Client;
    RPC.bReliable = false;

    const FString Result = RPC.BuildSpecifierString();

    TestTrue(TEXT("Contains Client"), Result.Contains(TEXT("Client")));
    TestTrue(TEXT("Contains Unreliable"), Result.Contains(TEXT("Unreliable")));
    TestFalse(TEXT("Does not contain WithValidation for Client"), Result.Contains(TEXT("WithValidation")));
    // "Unreliable" contains "Reliable" as a substring; make sure there is no standalone "Reliable"
    FString StrippedReliable = Result.Replace(TEXT("Unreliable"), TEXT("UNRELIABLE"));
    TestFalse(TEXT("Does not contain standalone Reliable"), StrippedReliable.Contains(TEXT("Reliable")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_RPCSpec_NetMulticast,
    "CodeForge.Schema.RPC.NetMulticast",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_RPCSpec_NetMulticast::RunTest(const FString& Parameters)
{
    FCodeForgeRPCDef RPC;
    RPC.Name = TEXT("MulticastExplode");
    RPC.Mode = ECodeForgeRPCMode::NetMulticast;
    RPC.bReliable = true;

    const FString Result = RPC.BuildSpecifierString();

    TestTrue(TEXT("Contains NetMulticast"), Result.Contains(TEXT("NetMulticast")));
    TestFalse(TEXT("Does not contain WithValidation for Multicast"), Result.Contains(TEXT("WithValidation")));

    return true;
}

// ---------------------------------------------------------------------------
// Param string tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_ParamDef_ConstRef,
    "CodeForge.Schema.Param.ConstRef",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_ParamDef_ConstRef::RunTest(const FString& Parameters)
{
    // const + ref
    {
        FCodeForgeParamDef P;
        P.Name = TEXT("InName"); P.Type = TEXT("FString");
        P.bIsConst = true; P.bIsRef = true;
        TestEqual(TEXT("const ref"), P.BuildParamString(), FString(TEXT("const FString& InName")));
    }
    // const only
    {
        FCodeForgeParamDef P;
        P.Name = TEXT("InVal"); P.Type = TEXT("float");
        P.bIsConst = true; P.bIsRef = false;
        TestEqual(TEXT("const only"), P.BuildParamString(), FString(TEXT("const float InVal")));
    }
    // ref only
    {
        FCodeForgeParamDef P;
        P.Name = TEXT("OutCount"); P.Type = TEXT("int32");
        P.bIsConst = false; P.bIsRef = true;
        TestEqual(TEXT("ref only"), P.BuildParamString(), FString(TEXT("int32& OutCount")));
    }
    // neither
    {
        FCodeForgeParamDef P;
        P.Name = TEXT("Damage"); P.Type = TEXT("float");
        P.bIsConst = false; P.bIsRef = false;
        TestEqual(TEXT("neither"), P.BuildParamString(), FString(TEXT("float Damage")));
    }
    return true;
}

// ---------------------------------------------------------------------------
// UCodeForgeBlueprint helper tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_ParentClassName_AllTypes,
    "CodeForge.Schema.Blueprint.ParentClassName",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_ParentClassName_AllTypes::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();

    const TArray<TPair<ECodeForgeClassType, FString>> Expected =
    {
        { ECodeForgeClassType::Actor,            TEXT("AActor")            },
        { ECodeForgeClassType::Pawn,             TEXT("APawn")             },
        { ECodeForgeClassType::Character,        TEXT("ACharacter")        },
        { ECodeForgeClassType::ActorComponent,   TEXT("UActorComponent")   },
        { ECodeForgeClassType::SceneComponent,   TEXT("USceneComponent")   },
        { ECodeForgeClassType::Object,           TEXT("UObject")           },
        { ECodeForgeClassType::GameModeBase,     TEXT("AGameModeBase")     },
        { ECodeForgeClassType::GameStateBase,    TEXT("AGameStateBase")    },
        { ECodeForgeClassType::PlayerController, TEXT("APlayerController") },
        { ECodeForgeClassType::PlayerState,      TEXT("APlayerState")      },
        { ECodeForgeClassType::HUD,              TEXT("AHUD")              },
    };

    for (const TPair<ECodeForgeClassType, FString>& Pair : Expected)
    {
        BP->ClassType = Pair.Key;
        TestEqual(*FString::Printf(TEXT("Parent class for %d"), (int32)Pair.Key),
            BP->GetParentClassName(), Pair.Value);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_ParentInclude_AllTypes,
    "CodeForge.Schema.Blueprint.ParentIncludePath",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_ParentInclude_AllTypes::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();

    const TArray<TPair<ECodeForgeClassType, FString>> Expected =
    {
        { ECodeForgeClassType::Actor,            TEXT("GameFramework/Actor.h")             },
        { ECodeForgeClassType::Pawn,             TEXT("GameFramework/Pawn.h")              },
        { ECodeForgeClassType::Character,        TEXT("GameFramework/Character.h")         },
        { ECodeForgeClassType::ActorComponent,   TEXT("Components/ActorComponent.h")       },
        { ECodeForgeClassType::SceneComponent,   TEXT("Components/SceneComponent.h")       },
        { ECodeForgeClassType::Object,           TEXT("UObject/NoExportTypes.h")           },
        { ECodeForgeClassType::GameModeBase,     TEXT("GameFramework/GameModeBase.h")      },
        { ECodeForgeClassType::GameStateBase,    TEXT("GameFramework/GameStateBase.h")     },
        { ECodeForgeClassType::PlayerController, TEXT("GameFramework/PlayerController.h")  },
        { ECodeForgeClassType::PlayerState,      TEXT("GameFramework/PlayerState.h")       },
        { ECodeForgeClassType::HUD,              TEXT("GameFramework/HUD.h")               },
    };

    for (const TPair<ECodeForgeClassType, FString>& Pair : Expected)
    {
        BP->ClassType = Pair.Key;
        TestEqual(*FString::Printf(TEXT("Include path for %d"), (int32)Pair.Key),
            BP->GetParentIncludePath(), Pair.Value);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_ClassSpecifiers_Blueprintable,
    "CodeForge.Schema.Blueprint.ClassSpecifiers",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_ClassSpecifiers_Blueprintable::RunTest(const FString& Parameters)
{
    UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();

    // Non-replicated — should only contain Blueprintable
    BP->bReplicated = false;
    FString Specifiers = BP->GetClassSpecifiers();
    TestTrue(TEXT("Contains Blueprintable"), Specifiers.Contains(TEXT("Blueprintable")));
    TestFalse(TEXT("No Replicated when bReplicated=false"), Specifiers.Contains(TEXT("Replicated")));

    // Replicated — should contain both
    BP->bReplicated = true;
    Specifiers = BP->GetClassSpecifiers();
    TestTrue(TEXT("Still contains Blueprintable"), Specifiers.Contains(TEXT("Blueprintable")));
    TestTrue(TEXT("Contains Replicated when bReplicated=true"), Specifiers.Contains(TEXT("Replicated")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCodeForgeTest_RPC_ParamList,
    "CodeForge.Schema.RPC.ParamList",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_RPC_ParamList::RunTest(const FString& Parameters)
{
    FCodeForgeRPCDef RPC;
    RPC.Name = TEXT("ServerSetTarget");
    RPC.Mode = ECodeForgeRPCMode::Server;

    FCodeForgeParamDef P0;
    P0.Name = TEXT("InActor"); P0.Type = TEXT("AActor*");
    P0.bIsConst = false; P0.bIsRef = false;

    FCodeForgeParamDef P1;
    P1.Name = TEXT("InLocation"); P1.Type = TEXT("FVector");
    P1.bIsConst = true; P1.bIsRef = true;

    RPC.Parameters = {P0, P1};

    const FString Result = RPC.BuildParamListString();
    TestTrue(TEXT("Contains AActor* InActor"), Result.Contains(TEXT("AActor* InActor")));
    TestTrue(TEXT("Contains const FVector& InLocation"), Result.Contains(TEXT("const FVector& InLocation")));

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

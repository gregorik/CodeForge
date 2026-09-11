// Copyright (c) 2026 GregOrigin. All Rights Reserved.
// CodeForgeViabilityTest.cpp
// Tests for viability & marketability features.

#include "Misc/AutomationTest.h"
#include "CodeForgeBlueprint.h"
#include "CodeForgeNode_Property.h"
#include "CodeForgeNode_Function.h"
#include "CodeForgeNode_RPC.h"
#include "CodeForgeEdGraph.h"
#include "CodeForgeEdGraphSchema.h"
#include "CodeForgeGenerator.h"
#include "CodeForgeTypes.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// ===========================================================================
// SyncFromBlueprint — Property
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_SyncFromBlueprint_Property,
	"CodeForge.Viability.SyncFromBlueprint.Property",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_SyncFromBlueprint_Property::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	FCodeForgePropertyDef Prop;
	Prop.Name = TEXT("Health");
	Prop.Type = TEXT("float");
	Prop.bEditAnywhere = true;
	BP->Properties.Add(Prop);

	UCodeForgeNode_Property* Node = NewObject<UCodeForgeNode_Property>();
	Node->PropertyDef.Name = TEXT("Health"); // Name must match for lookup
	Node->SyncFromBlueprint(BP);

	TestEqual(TEXT("PropertyDef.Name"), Node->PropertyDef.Name, TEXT("Health"));
	TestEqual(TEXT("PropertyDef.Type"), Node->PropertyDef.Type, TEXT("float"));
	TestTrue(TEXT("PropertyDef.bEditAnywhere"), Node->PropertyDef.bEditAnywhere);
	return true;
}

// ===========================================================================
// SyncFromBlueprint — Function
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_SyncFromBlueprint_Function,
	"CodeForge.Viability.SyncFromBlueprint.Function",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_SyncFromBlueprint_Function::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	FCodeForgeFunctionDef Func;
	Func.Name = TEXT("FireWeapon");
	Func.ReturnType = TEXT("void");
	BP->Functions.Add(Func);

	UCodeForgeNode_Function* Node = NewObject<UCodeForgeNode_Function>();
	Node->FunctionDef.Name = TEXT("FireWeapon"); // Name must match for lookup
	Node->SyncFromBlueprint(BP);

	TestEqual(TEXT("FunctionDef.Name"), Node->FunctionDef.Name, TEXT("FireWeapon"));
	TestEqual(TEXT("FunctionDef.ReturnType"), Node->FunctionDef.ReturnType, TEXT("void"));
	return true;
}

// ===========================================================================
// SyncFromBlueprint — RPC
// ===========================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_SyncFromBlueprint_RPC,
	"CodeForge.Viability.SyncFromBlueprint.RPC",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_SyncFromBlueprint_RPC::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	FCodeForgeRPCDef RPC;
	RPC.Name = TEXT("ServerFire");
	RPC.Mode = ECodeForgeRPCMode::Server;
	RPC.bReliable = true;
	BP->RPCs.Add(RPC);

	UCodeForgeNode_RPC* Node = NewObject<UCodeForgeNode_RPC>();
	Node->RPCDef.Name = TEXT("ServerFire"); // Name must match for lookup
	Node->SyncFromBlueprint(BP);

	TestEqual(TEXT("RPCDef.Name"), Node->RPCDef.Name, TEXT("ServerFire"));
	TestTrue(TEXT("RPCDef.bReliable"), Node->RPCDef.bReliable);
	return true;
}

// ===========================================================================
// Include Paths in Generated Code
// ===========================================================================
static FString GetTemplatePath_Viability()
{
	return FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("CodeForge/Content/Templates"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodeForgeTest_IncludePathInContext,
	"CodeForge.Viability.IncludePaths",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FCodeForgeTest_IncludePathInContext::RunTest(const FString& Parameters)
{
	UCodeForgeBlueprint* BP = NewObject<UCodeForgeBlueprint>();
	BP->ClassName = TEXT("AMyActor");
	BP->BlueprintKind = ECodeForgeBlueprintKind::Class;
	BP->ClassType = ECodeForgeClassType::Actor;

	FCodeForgePropertyDef Prop;
	Prop.Name = TEXT("Inventory");
	Prop.Type = TEXT("FInventoryData");
	Prop.IncludePath = TEXT("InventoryData.h");
	BP->Properties.Add(Prop);

	FCodeForgePropertyDef Prop2;
	Prop2.Name = TEXT("Health");
	Prop2.Type = TEXT("float");
	// No IncludePath — built-in type
	BP->Properties.Add(Prop2);

	FCodeForgeGenerator Generator;
	Generator.SetTemplatePath(GetTemplatePath_Viability());
	FCodeForgeGeneratorResult Result = Generator.Generate(BP);

	TestTrue(TEXT("Generation succeeded"), Result.bSuccess);
	TestTrue(TEXT("Header contains InventoryData.h include"),
		Result.HeaderContent.Contains(TEXT("#include \"InventoryData.h\"")));
	TestFalse(TEXT("Header does NOT contain empty include"),
		Result.HeaderContent.Contains(TEXT("#include \"\"")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

#include "CodeForgeRuntime.h"

#define LOCTEXT_NAMESPACE "FCodeForgeRuntimeModule"

void FCodeForgeRuntimeModule::StartupModule() {}
void FCodeForgeRuntimeModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCodeForgeRuntimeModule, CodeForgeRuntime)


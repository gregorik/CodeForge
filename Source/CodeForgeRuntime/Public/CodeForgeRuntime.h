#pragma once

#include "Modules/ModuleManager.h"

class FCodeForgeRuntimeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};



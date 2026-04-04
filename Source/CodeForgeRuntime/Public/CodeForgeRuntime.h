// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FCodeForgeRuntimeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};




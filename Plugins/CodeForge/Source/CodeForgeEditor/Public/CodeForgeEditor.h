#pragma once

#include "Modules/ModuleManager.h"

class FCodeForgeEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

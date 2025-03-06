// Copyright 2022 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDTBPJsonModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

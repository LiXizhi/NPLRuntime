#pragma once

#include "MCIController.h"

class Win32MCI : public ParaEngine::MCIController::Interface
{
public:
	void Release();

	bool Start();

	bool Stop();

	bool Save(const char* fileName);
};

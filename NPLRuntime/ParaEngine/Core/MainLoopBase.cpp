#include "ParaEngine.h"
#include "MainLoopBase.h"

namespace ParaEngine
{
	MainLoopBase::MainLoopBase()
		: m_main_timer(m_main_io_service)
	{

	}

	MainLoopBase::~MainLoopBase()
	{

	}


	void MainLoopBase::MainLoopRun()
	{
		m_main_io_service.run();
	}
}
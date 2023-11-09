#include "ParaEngine.h"
#include "MainLoopBase.h"

namespace ParaEngine
{
#ifndef EMSCRIPTEN_SINGLE_THREAD
	MainLoopBase::MainLoopBase(): m_main_timer(m_main_io_service)
#else
	MainLoopBase::MainLoopBase()
#endif
	{

	}

	MainLoopBase::~MainLoopBase()
	{

	}


	void MainLoopBase::MainLoopRun()
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		m_main_io_service.run();
#endif
	}
}
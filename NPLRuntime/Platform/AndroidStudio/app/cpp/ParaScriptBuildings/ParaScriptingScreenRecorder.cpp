//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.cpp
// Authors: big
// CreateDate: 2022.2.25
//-----------------------------------------------------------------------------

#include "jni/ScreenRecorder.h"
#include "ParaScriptingScreenRecorder.h"

namespace ParaScripting
{
	void ParaScriptingScreenRecorder::Start()
	{
        ParaEngine::ScreenRecorder::start();
    }

    void ParaScriptingScreenRecorder::Stop()
    {
        ParaEngine::ScreenRecorder::stop();
    }

    void ParaScriptingScreenRecorder::Save()
    {
        ParaEngine::ScreenRecorder::save();
    }

    void ParaScriptingScreenRecorder::Play()
    {
        ParaEngine::ScreenRecorder::play();
    }
}

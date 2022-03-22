//-----------------------------------------------------------------------------
// ScreenRecorder.cpp
// Authors: big
// CreateDate: 2022.2.25
//-----------------------------------------------------------------------------

#include "ScreenRecorder.h"
#include "JniHelper.h"

namespace ParaEngine {
	const std::string ScreenRecorder::classname = "com/tatfook/paracraft/screenrecorder/ScreenRecorder";

	void ScreenRecorder::start()
	{
		JniHelper::callStaticVoidMethod(classname, "start");
    }

    void ScreenRecorder::stop()
	{
		JniHelper::callStaticVoidMethod(classname, "stop");
	}

	void ScreenRecorder::save()
	{
		JniHelper::callStaticVoidMethod(classname, "save");
	}

	void ScreenRecorder::play()
	{
		JniHelper::callStaticVoidMethod(classname, "play");
	}
}

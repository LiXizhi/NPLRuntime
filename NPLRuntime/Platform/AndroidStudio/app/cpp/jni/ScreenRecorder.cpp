//-----------------------------------------------------------------------------
// ScreenRecorder.cpp
// Authors: big
// CreateDate: 2022.2.25
//-----------------------------------------------------------------------------

#include "ScreenRecorder.h"
#include "JniHelper.h"
#include "Framework/Common/Bridge/LuaJavaBridge.h"

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

	std::string ScreenRecorder::save()
	{
		std::string savedPath = JniHelper::callStaticStringMethod(classname, "save");
		std::string code = "MyCompany.Aries.Game.Mobile.ScreenRecorderHandler.savedCallbackFunc(\"" + savedPath + "\")";
		ParaEngine::LuaJavaBridge::nplActivate(code, "");
		return savedPath;
	}

	void ScreenRecorder::play()
	{
		JniHelper::callStaticVoidMethod(classname, "play");
	}

	void ScreenRecorder::removeVideo()
	{
		JniHelper::callStaticVoidMethod(classname, "removeVideo");
	}
}

using namespace ParaEngine;

extern "C" {
	JNIEXPORT void Java_com_tatfook_paracraft_screenrecorder_ScreenRecorder_nativeStartedCallbackFunc(JNIEnv *env, jclass)
	{
		std::string code = "MyCompany.Aries.Game.Mobile.ScreenRecorderHandler.startedCallbackFunc()";
		ParaEngine::LuaJavaBridge::nplActivate(code, "");
	}

	JNIEXPORT void Java_com_tatfook_paracraft_screenrecorder_ScreenRecorder_nativeRecordFinishedCallbackFunc(JNIEnv *env, jclass, jstring filePath)
	{
		std::string savedPath = JniHelper::jstring2string(filePath);
		std::string code = "MyCompany.Aries.Game.Mobile.ScreenRecorderHandler.recordFinishedCallbackFunc(\"" + savedPath + "\")";
		ParaEngine::LuaJavaBridge::nplActivate(code, "");
	}
}

//-----------------------------------------------------------------------------
// ScreenRecorder.h
// Authors: big
// CreateDate: 2022.2.25
//-----------------------------------------------------------------------------

#pragma once

#include <jni.h>
#include <string>

namespace ParaEngine {
	class ScreenRecorder
	{
	public:
        static void start();
        static void stop();
        static void save();
        static void play();
    private:
		static const std::string classname;
    };
}
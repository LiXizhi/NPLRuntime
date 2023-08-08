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
      static std::string save();
      static void play();
      static void removeVideo();
  private:
      static const std::string classname;
    };
}
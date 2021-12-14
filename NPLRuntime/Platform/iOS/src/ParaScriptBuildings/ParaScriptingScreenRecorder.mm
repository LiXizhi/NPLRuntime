//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.cpp
// Authors: big
// CreateDate: 2021.12.14
//-----------------------------------------------------------------------------

#import "ScreenRecorder/ScreenRecorder.h"

#include "ParaScriptingScreenRecorder.h"

namespace ParaScripting
{
	void ParaScriptingScreenRecorder::Start()
	{
        ScreenRecorder *screenRecorder = [[ScreenRecorder alloc] init];
        
        [screenRecorder startRecordingWithHandler:^(NSError *error) {}];
	}
}

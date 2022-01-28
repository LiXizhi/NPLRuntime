//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.cpp
// Authors: big
// CreateDate: 2021.12.14
//-----------------------------------------------------------------------------

#import "AppDelegate.h"
#import "ScreenRecorder/ScreenRecorder.h"

#include "ParaScriptingScreenRecorder.h"

namespace ParaScripting
{
	void ParaScriptingScreenRecorder::Start()
	{
        if (![ScreenRecorder getInstance]) {
            ScreenRecorder *instance = [[ScreenRecorder alloc] init];
            [ScreenRecorder setInstance:instance];
        }

        ScreenRecorder *instance = [ScreenRecorder getInstance];

        [instance startRecordingWithHandler:^(NSError *error) {}];
        //[instance screenRecording:^(NSTimeInterval duration) {}];
	}

    void ParaScriptingScreenRecorder::Stop()
    {
        ScreenRecorder *instance = [ScreenRecorder getInstance];

        [instance stopRecordingWithHandler:^(UIViewController *previewViewController, NSError *error) {
            auto appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];

            [appDelegate.viewController presentViewController:previewViewController animated:YES completion:^{}];
        }];
    }
}

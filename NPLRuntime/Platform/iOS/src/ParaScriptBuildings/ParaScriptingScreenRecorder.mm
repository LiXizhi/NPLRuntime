//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.cpp
// Authors: big
// CreateDate: 2021.12.14
//-----------------------------------------------------------------------------

#import "ScreenRecorder/ScreenRecorder.h"
#import <AVFoundation/AVFoundation.h>

#include "ParaScriptingScreenRecorder.h"

namespace ParaScripting
{
	void ParaScriptingScreenRecorder::Start()
	{
        [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryPlayAndRecord error: nil];
        UInt32 doChangeDefault = 1;
        AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryDefaultToSpeaker, sizeof(doChangeDefault), &doChangeDefault);

//        [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryPlayAndRecord error: nil];
//        UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
//        AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audioRouteOverride), &audioRouteOverride);
        
        if (![ScreenRecorder getInstance]) {
            ScreenRecorder *instance = [[ScreenRecorder alloc] init];
            [ScreenRecorder setInstance:instance];
        }

        ScreenRecorder *instance = [ScreenRecorder getInstance];

        [instance startRecordingWithHandler:^(NSError *error) {}];
	}

    void ParaScriptingScreenRecorder::Stop()
    {
        ScreenRecorder *instance = [ScreenRecorder getInstance];

        [instance stopRecordingWithHandler];
    }

    std::string ParaScriptingScreenRecorder::Save()
    {
        ScreenRecorder *instance = [ScreenRecorder getInstance];

        NSString *savePath = [instance saveVideo];
        return [savePath UTF8String];
    }

    void ParaScriptingScreenRecorder::Play()
    {
        ScreenRecorder *instance = [ScreenRecorder getInstance];
        [instance playPreview];
    }

    void ParaScriptingScreenRecorder::RemoveVideo()
    {
        ScreenRecorder *instance = [ScreenRecorder getInstance];
        [instance removeVideo];
    }
}

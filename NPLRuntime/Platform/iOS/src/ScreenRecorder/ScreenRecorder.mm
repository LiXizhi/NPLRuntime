//-----------------------------------------------------------------------------
// Class: ScreenRecoder.mm
// Authors: big
// CreateDate: 2021.12.13
//-----------------------------------------------------------------------------

#define ScreenRecorderFrameTime 0.05
#import "ScreenRecorder.h"

@interface ScreenRecorder ()<RPScreenRecorderDelegate, RPPreviewViewControllerDelegate>
{
    ScreenRecording _screenRecording;
    ScreenRecordStop _screenRecordStop;
    NSInteger _frameCount;
    AVAssetWriter *_videoWriter;
    AVAssetWriterInput *_videoWriterInput;
    AVAssetWriterInputPixelBufferAdaptor *_adaptor;
}
@end

@implementation ScreenRecorder

- (BOOL)available
{
    return YES;
}

- (void)startRecordingWithHandler:(void (^)(NSError *))handler
{
    if (NSClassFromString(@"RPScreenRecorder") == nil ||
        ![[RPScreenRecorder sharedRecorder] isAvailable]) {
        return;
    }

    [[RPScreenRecorder sharedRecorder]
        startRecordingWithMicrophoneEnabled:YES
        handler:^(NSError *error) {
            if (handler) {
                handler(error);
            }

            if (error) {
                NSLog(@"Screen record error message: %@", error);
            } else {
                _recording = YES;
            }
        }
    ];
}

- (void)stopRecordingWithHandler:(ScreenRecordStop)handler
{
    _recording = NO;

    [[RPScreenRecorder sharedRecorder]
        stopRecordingWithHandler:^(RPPreviewViewController *previewViewController, NSError *error) {
            if (error) {
                NSLog(@"Stop record fail!");
            } else {
                previewViewController.previewControllerDelegate = self;
            }

            dispatch_async(
               dispatch_get_main_queue(),
               ^{
                   if (handler) {
                       handler(previewViewController, nil, error);
                   }
               }
            );
        }
    ];
}

- (void)previewController:(RPPreviewViewController *)previewController didFinishWithActivityTypes:(NSSet<NSString *> *)activityTypes
{
    if ([activityTypes containsObject:@"com.apple.UIKit.activity.SaveToCameraRoll"]) {
        dispatch_async(
            dispatch_get_main_queue(),
           ^{
               NSLog(@"Sava success!");
           }
        );
    }

    if ([activityTypes containsObject:@"com.apple.UIKit.activity.CopyToPasteboard"]) {
        dispatch_async(
           dispatch_get_main_queue(),
           ^{
               NSLog(@"Copy success!");
           }
        );
    }
}

- (void)previewControllerDidFinish:(RPPreviewViewController *)previewController
{
    [previewController
        dismissViewControllerAnimated:YES
        completion:^{
            // TODO: ...
        }
    ];
}

- (void)screenRecording:(ScreenRecording)screenRecording
{
    _screenRecording = [screenRecording copy];
}

@end

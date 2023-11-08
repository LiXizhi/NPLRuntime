//-----------------------------------------------------------------------------
// Class: ScreenRecoder.mm
// Authors: big
// CreateDate: 2021.12.13
// ModifyDate: 2022.1.20
//-----------------------------------------------------------------------------

#import "AppDelegate.h"
#import "ScreenRecorder.h"
#import <Photos/Photos.h>
#import "LuaObjcBridge/CCLuaObjcBridge.h"

#include "ParaEngine.h"

@interface ScreenRecorder ()<RPScreenRecorderDelegate, RPPreviewViewControllerDelegate>
{
}
@end

@implementation ScreenRecorder

static ScreenRecorder *instance = nil;

+ (ScreenRecorder *)getInstance
{
    return instance;
}

+ (void)setInstance:(ScreenRecorder *)curInstance
{
    instance = curInstance;
}

+ (void)demandForAuthorization:(void(^)(void))authorizedResultBlock {
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];

    if (status == PHAuthorizationStatusAuthorized) {
        if (authorizedResultBlock) {
            authorizedResultBlock();
        }
    } else {
        [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
            if (status == PHAuthorizationStatusAuthorized) {
                dispatch_async(
                    dispatch_get_main_queue(), ^{
                        if (authorizedResultBlock) {
                            authorizedResultBlock();
                        }
                    }
                );
            }
        }];
    }
}

- (void)startRecordingWithHandler:(void (^)(NSError *))handler
{
    if (@available(iOS 14.0, *)) {
        [ScreenRecorder demandForAuthorization:^{
            if (NSClassFromString(@"RPScreenRecorder") == nil ||
                ![[RPScreenRecorder sharedRecorder] isAvailable]) {
                return;
            }

            RPScreenRecorder *sharedRecorder = [RPScreenRecorder sharedRecorder];
            sharedRecorder.microphoneEnabled = YES;
            [sharedRecorder startRecordingWithHandler:^(NSError * _Nullable error) {
                NSLog(@"The video recording real started.");
                std::string code = "MyCompany.Aries.Game.Mobile.ScreenRecorderHandler.startedCallbackFunc()";
                ParaEngine::LuaObjcBridge::nplActivate(code, "");
            }];
        }];
    }
}

- (void)stopRecordingWithHandler
{
    if (@available(iOS 14.0, *)) {
        _recording = NO;

        dispatch_async(
            dispatch_get_main_queue(),
            ^{
                NSDateFormatter *dateformat = [NSDateFormatter new];
                [dateformat setDateFormat:@"yyyy_MM_dd_HH_mm_ss"];

                NSString *fileName = [NSString stringWithFormat:@"record_screen_%@.mp4",[dateformat stringFromDate:[NSDate date]]];
                NSString *documentDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
                NSString *dirPath = [documentDirectory stringByAppendingPathComponent:@"record_screen_video"];

                if (![[NSFileManager defaultManager] fileExistsAtPath:dirPath]) {
                    [[NSFileManager defaultManager] createDirectoryAtPath:dirPath withIntermediateDirectories:YES attributes:nil error:nil];
                }

                NSString *filePath = [dirPath stringByAppendingPathComponent:fileName];

                self.saveVideoPath = filePath;

                RPScreenRecorder *sharedRecorder = [RPScreenRecorder sharedRecorder];

                [sharedRecorder stopRecordingWithOutputURL:[NSURL fileURLWithPath:filePath] completionHandler:^(NSError * _Nullable error) {
                    std::string savedPath = [self.saveVideoPath UTF8String];
                    std::string code = "MyCompany.Aries.Game.Mobile.ScreenRecorderHandler.recordFinishedCallbackFunc(\"" + savedPath + "\")";
                    ParaEngine::LuaObjcBridge::nplActivate(code, "");
                }];
            }
       );
    }
}

- (void)playPreview
{
    if (!self.saveVideoPath) {
        return;
    }
    
    dispatch_async(
        dispatch_get_main_queue(),
        ^{
            auto appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];

            AVPlayerItem *playerItem = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:self.saveVideoPath]];
            AVPlayer *player = [AVPlayer playerWithPlayerItem:playerItem];
            AVPlayerViewController *playerViewController = [[AVPlayerViewController alloc] init];
            playerViewController.player = player;
            [player play];
            playerViewController.showsPlaybackControls = YES;
            playerViewController.view.frame = appDelegate.viewController.view.frame;
            [appDelegate.viewController presentViewController:playerViewController animated:YES completion:^{}];
        }
    );
}

- (NSString *)saveVideo
{
    if (!self.saveVideoPath) {
        return @"";
    }

    UISaveVideoAtPathToSavedPhotosAlbum(self.saveVideoPath, self, @selector(videoSaved:didFinishSavingWithError:contextInfo:), nil);
    return self.saveVideoPath;
}

- (void)videoSaved:(NSString *)videoPath didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo
{
    // ignore ALAssetsLibraryErrorDomain.
    if (error && ![error.domain isEqualToString:@"ALAssetsLibraryErrorDomain"]) {
        // Handle the error if saving fails.
        NSLog(@"iOS save video fail.");
    } else {
        // Video saved successfully.
        std::string savedPath = [self.saveVideoPath UTF8String];
        std::string code = "MyCompany.Aries.Game.Mobile.ScreenRecorderHandler.savedCallbackFunc(\"" + savedPath + "\")";
        ParaEngine::LuaObjcBridge::nplActivate(code, "");
    }
}

- (void)removeVideo
{
    if (!self.saveVideoPath) {
        return;
    }

    NSError *error;
    BOOL success = [[NSFileManager defaultManager] removeItemAtPath:self.saveVideoPath error:&error];

    if (success) {
        // File deleted successfully.
        NSLog(@"Delete file successed: %@", self.saveVideoPath);
    } else {
        // Handle the error if the file deletion fails.
    }
}

@end

//-----------------------------------------------------------------------------
// Class: ScreenRecoder.h
// Authors: big
// CreateDate: 2021.12.13
// ModifyDate: 2022.1.20
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <ReplayKit/ReplayKit.h>
#import <AVKit/AVKit.h>
#import <AVFoundation/AVFoundation.h>

@interface ScreenRecorder : NSObject

// recoder status
@property(nonatomic, readonly) BOOL recording;
// is recoder available
@property(nonatomic, readonly) BOOL available;
// temp video save path
@property(nonatomic) NSString *saveVideoPath;

+ (ScreenRecorder *)getInstance;
+ (void)setInstance:(ScreenRecorder *)curInstance;
+ (void)demandForAuthorization:(void(^)(void))authorizedResultBlock;

// start recoding, auto select different SDK version
- (void)startRecordingWithHandler:(void(^)(NSError *error))handler;
// stop recoding
- (void)stopRecordingWithHandler;
// play preview
- (void)playPreview;
// save video
- (NSString *)saveVideo;
// remove video
- (void)removeVideo;

@end


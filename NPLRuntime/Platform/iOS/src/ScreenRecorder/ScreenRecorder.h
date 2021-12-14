//-----------------------------------------------------------------------------
// Class: ScreenRecoder.h
// Authors: big
// CreateDate: 2021.12.13
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <ReplayKit/ReplayKit.h>
#import <MediaPlayer/MediaPlayer.h>

typedef void(^ScreenRecording)(NSTimeInterval duration);
typedef void(^ScreenRecordStop)(UIViewController *previewViewController, NSString *videoPath, NSError *error);

@interface ScreenRecorder : NSObject

// recoder status
@property(nonatomic, readonly) BOOL recording;
// recoder time
@property(nonatomic, readonly) NSTimeInterval duration;
// is recoder available
@property(nonatomic, readonly) BOOL available;

// start recoding, auto select different SDK version
- (void)startRecordingWithHandler:(void(^)(NSError *error))handler;
// stop recoding
- (void)stopRecordingWithHandler:(ScreenRecordStop)handler;
// recoding callback
- (void)screenRecording:(ScreenRecording)screenRecording;

@end


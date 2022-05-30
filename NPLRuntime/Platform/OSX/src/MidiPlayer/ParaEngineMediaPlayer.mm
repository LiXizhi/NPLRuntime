//
//  MidiPlayer.m
//  Paracraft
//
//  Created by Paracraft on 23/3/2022.
//

#import <Foundation/Foundation.h>
#import <AVFAudio/AVFAudio.h>
#include "ParaEngineMediaPlayer.h"

@interface MidiHelper: NSObject{
    AVMIDIPlayer * _player;
    AVMIDIPlayer * _playerWithFile;
    NSData * _midiData;
}

+ (MidiHelper*) getInstance;

- (void)PlayMidiNote:(NSNumber *) note withVelocity:(NSNumber *)velocity baseNote:(NSNumber *)baseNote;
- (void)PlayMidiNote:(NSNumber *) note withVelocity:(NSNumber *)velocity baseNote:(NSNumber *)baseNote channel:(NSNumber *)channel;
- (void)PlayMidiFile:(NSString*)filename;
- (void)SetVolume:(NSNumber*)volume;
- (void)Stop;

@end

@implementation MidiHelper

- (id)init
{
    self = [super init];

    if (self) {

    }

    _midiData = [NSData alloc];
    return self;
}

+ (MidiHelper*) getInstance
{
    static MidiHelper* _instance = nil;

    if (_instance == nil)
    {
        _instance = [[MidiHelper alloc] init];
    }

    return _instance;
}

- (void)PlayMidiNote:(NSNumber *) note withVelocity:(NSNumber*)velocity baseNote:(NSNumber*)baseNote
{
    static Byte midiFileData[37] = {
        0x4d, 0x54, 0x68, 0x64, // MThd
        0x00, 0x00, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x01, // track count
        0x00, 0x78, // tick
        0x4d, 0x54, 0x72, 0x6b, // MTrk
        0x00, 0x00, 0x00, 0x0f,
        0x00, 0xc0, 0x00,
        0x00, 0x90, 0x48, 0x40,
        0x78, 0x80, 0x48, 0x40,
        0x00, 0xff, 0x2f, 0x00
    };

    midiFileData[24] = [baseNote intValue];
    midiFileData[27] = [note intValue];
    midiFileData[28] = [velocity intValue];
    midiFileData[31] = [note intValue];
    midiFileData[32] = [velocity intValue];

    _midiData = [_midiData initWithBytes:midiFileData length:37];

    NSError *err = nil;
    NSURL *bankUrl = nil;

//    if (false) {
//        NSString *resPath = [[NSBundle mainBundle] resourcePath];
//        resPath = [resPath stringByAppendingPathComponent:@"res/gs_instruments.dls"];
//        bankUrl = [NSURL fileURLWithPath:resPath];
//    }

    if (_player == nil) {
        _player = [AVMIDIPlayer alloc];
    }

    _player = [_player initWithData:_midiData soundBankURL:bankUrl error:&err];

    if (err == nil and _player != nil) {
        [_player prepareToPlay];
        float dur = _player.duration;
        BOOL isPlay = [_player isPlaying];

        [_player play:^{
            // NSLog(@"播放完成");
        }];
    }
}

- (void)PlayMidiNote:(NSNumber *)note withVelocity:(NSNumber *)velocity baseNote:(NSNumber *)baseNote channel:(NSNumber *)channel
{
    static Byte midiFileData[37] = {
        0x4d, 0x54, 0x68, 0x64, // MThd
        0x00, 0x00, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x01, // track count
        0x00, 0x78, // tick
        0x4d, 0x54, 0x72, 0x6b, // MTrk
        0x00, 0x00, 0x00, 0x0f,
        0x00, 0xc0, 0x00,
        0x00, 0x90, 0x48, 0x40,
        0x78, 0x80, 0x48, 0x40,
        0x00, 0xff, 0x2f, 0x00
    };

    midiFileData[23] = 0xc0 + [channel intValue];
    midiFileData[24] = [baseNote intValue];
    midiFileData[26] = 0x90 + [channel intValue];
    midiFileData[27] = [note intValue];
    midiFileData[28] = [velocity intValue];
    midiFileData[30] = 0x80 + [channel intValue];
    midiFileData[31] = [note intValue];
    midiFileData[32] = [velocity intValue];

    _midiData = [_midiData initWithBytes:midiFileData length:37];

    NSError *err = nil;
    NSURL *bankUrl = nil;

//    if (false) {
//        NSString *resPath = [[NSBundle mainBundle] resourcePath];
//        resPath = [resPath stringByAppendingPathComponent:@"res/gs_instruments.dls"];
//        bankUrl = [NSURL fileURLWithPath:resPath];
//    }

    if (_player == nil) {
        _player = [AVMIDIPlayer alloc];
    }

    _player = [_player initWithData:_midiData soundBankURL:bankUrl error:&err];

    if (err == nil and _player != nil) {
        [_player prepareToPlay];
        float dur = _player.duration;
        BOOL isPlay = [_player isPlaying];

        [_player play:^{
            // NSLog(@"播放完成");
        }];
    }
}

- (void)PlayMidiFile:(NSString*)filename
{
    if (filename==nil or [filename isEqual:@""]) {
        return;
    }

    NSError * err = nil;
    NSURL *url = [NSURL URLWithString:filename];
    NSURL *bankUrl = nil;

    if (false) {
        NSString *resPath = [[NSBundle mainBundle] resourcePath];
        resPath = [resPath stringByAppendingPathComponent:@"res/gs_instruments.dls"];
        bankUrl = [NSURL fileURLWithPath:resPath];
    }

    [self Stop];

    if(_playerWithFile == nil) {
        _playerWithFile = [AVMIDIPlayer alloc];
    }

    _playerWithFile = [_playerWithFile initWithContentsOfURL:url soundBankURL:bankUrl error:&err];

    if (err==nil and _playerWithFile!=nil) {
        float dur = _playerWithFile.duration;
        BOOL isPlay = [_playerWithFile isPlaying];

        [_playerWithFile play:^{
            NSLog(@"播放完成");
        }];
    }
}

- (void)Stop{
    if(_playerWithFile!=nil and [_playerWithFile isPlaying]){
        [_playerWithFile stop];
    }
}

- (void)SetVolume:(NSNumber *)volume
{
    
}

@end

namespace ParaEngine {

    ParaEngineMediaPlayer* ParaEngineMediaPlayer::GetSingleton() {
        static ParaEngine::ParaEngineMediaPlayer s_media_player;
        return &s_media_player;
    }

    void ParaEngineMediaPlayer::PlayMidiNote(int note, int velocity) {
        
    }

    void ParaEngineMediaPlayer::PlayMidiNote(int note, int velocity, int baseNote) {
        [[MidiHelper getInstance]
            PlayMidiNote:[NSNumber numberWithInt:note]
            withVelocity:[NSNumber numberWithInt:velocity]
            baseNote:[NSNumber numberWithInt: baseNote]];
    }

    void ParaEngineMediaPlayer::PlayMidiNote(int note, int velocity, int baseNote, int channel) {
        [[MidiHelper getInstance]
            PlayMidiNote:[NSNumber numberWithInt:note]
            withVelocity:[NSNumber numberWithInt:velocity]
            baseNote:[NSNumber numberWithInt: baseNote]
            channel:[NSNumber numberWithInt: channel]];
    }

    void ParaEngineMediaPlayer::PlayMidiFile(std::string filename) {
        NSString *str = [NSString stringWithCString:filename.c_str() encoding:NSUTF8StringEncoding];
        [[MidiHelper getInstance] PlayMidiFile:str];
    }

    void ParaEngineMediaPlayer::SetVolume(float volume) {
        [[MidiHelper getInstance] SetVolume:[NSNumber numberWithFloat:volume]];
    }

    void ParaEngineMediaPlayer::PlayMidiData(std::vector<char> datas)
    {
        
    }
}

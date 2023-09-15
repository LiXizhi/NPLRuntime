//-----------------------------------------------------------------------------
// Class: ParaScriptingAudioiOS.mm
// Authors: big
// CreateDate: 2023.9.13
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#include "ParaScriptingAudioiOS.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisenc.h>

//UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
//AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audioRouteOverride), &audioRouteOverride);

static AVAudioRecorder *audioRecorder;

bool ParaScripting::ParaScriptingAudioiOS::StartRecording() {
    // 设置录音文件的保存位置和格式
    NSArray *pathArray = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = pathArray[0];
    NSString *outputFilePath = [documentsDirectory stringByAppendingPathComponent:@"Paracraft/files/temp/recordedAudio.wav"];
    NSURL *outputFileURL = [NSURL fileURLWithPath:outputFilePath];

    // 设置录音参数
    NSDictionary *recordSettings = @{
        AVFormatIDKey: @(kAudioFormatLinearPCM),
        AVSampleRateKey: @44100.0,
        AVNumberOfChannelsKey: @2,
        AVEncoderAudioQualityKey: @(AVAudioQualityHigh)
    };
    
    [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryPlayAndRecord error: nil];
    
    NSError *error;
    audioRecorder = [[AVAudioRecorder alloc] initWithURL:outputFileURL settings:recordSettings error:&error];

    if (error) {
        NSLog(@"Error initializing the audio recorder: %@", [error localizedDescription]);
        return false;
    }
    
    [audioRecorder prepareToRecord];
    [audioRecorder record];
    
    return false;
}

void ParaScripting::ParaScriptingAudioiOS::StopRecording() {
    [audioRecorder stop];
    
    [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryPlayAndRecord error: nil];
    UInt32 doChangeDefault = 1;
    AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryDefaultToSpeaker, sizeof(doChangeDefault), &doChangeDefault);
}

#define READ 1024
signed char readbuffer[READ * 4 + 44]; /* out of the data segment, not the stack */

bool ParaScripting::ParaScriptingAudioiOS::SaveRecording(const std::string &filepath, const float quality) {
    NSArray *pathArray = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = pathArray[0];
    NSString *inputFilePath = [documentsDirectory stringByAppendingPathComponent:@"Paracraft/files/temp/recordedAudio.wav"];
    NSString *outputFilePath = [[NSString alloc] initWithBytes:filepath.c_str() length:filepath.length() encoding:NSUTF8StringEncoding];

    ogg_stream_state os; /* take physical pages, weld into a logical  stream of packets */
    ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */
    
    vorbis_info      vi; /* struct that stores all the static vorbis bitstream settings */
    vorbis_comment   vc; /* struct that stores all the user comments */
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    int eos = 0, ret;

    FILE *fd = fopen([inputFilePath UTF8String], "rb");
    if (fd == NULL) {
        perror("open fail:");
        return false;
    }

    FILE *out = fopen([outputFilePath UTF8String], "wb");
    if(out == NULL)
    {
        perror("open fail:");
        return false;
    }

    vorbis_info_init(&vi);

    ret = vorbis_encode_init_vbr(&vi, 2, 44100, quality);
    if (ret) {
        NSLog(@"vorbis_encode_init_vbr fail");
        return false;
    }
    
    /* add a comment */
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "ParaScriptingAudioiOS.mm");

    /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);
    
    /* set up our packet->stream encoder */
    /* pick a random serial number; that way we can more likely build
       chained streams just by concatenation */
    srand((unsigned int)time(NULL));
    ogg_stream_init(&os, rand());
    
    /* Vorbis streams begin with three headers; the initial header (with
       most of the codec setup parameters) which is mandated by the Ogg
       bitstream spec.  The second header holds any comment fields.  The
       third header holds the bitstream codebook.  We merely need to
       make the headers, then pass them to libvorbis one at a time;
       libvorbis handles the additional Ogg bitstream constraints */
    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header); /* automatically placed in its own page */
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        /* This ensures the actual
         * audio data will start on a new page, as per spec
         */
        while (!eos)
        {
            int result = ogg_stream_flush(&os, &og);
            if (result==0) break;
            fwrite(og.header, 1, og.header_len, out);
            fwrite(og.body, 1, og.body_len, out);
        }
    }
    
    while(!eos) {
        long i;
        long bytes = fread(readbuffer, 1, READ * 4, fd); /* stereo hardwired here */
        if (bytes == 0) {
            /* end of file.  this can be done implicitly in the mainline,
               but it's easier to see here in non-clever fashion.
               Tell the library we're at end of stream so that it can handle
               the last frame and mark end of stream in the output properly */
            vorbis_analysis_wrote(&vd, 0);
        } else {
            /* data to encode */
            /* expose the buffer to submit data */
            float **buffer = vorbis_analysis_buffer(&vd, READ);

            /* uninterleave samples */
            for(i = 0;i < bytes / 4; i++)
            {
                buffer[0][i] = ((readbuffer[i * 4 + 1] << 8)|
                                (0x00ff & (int)readbuffer[i * 4])) / 32768.f;
                buffer[1][i] = ((readbuffer[i * 4 + 3] << 8)|
                                (0x00ff & (int)readbuffer[i * 4 + 2])) / 32768.f;
            }

            /* tell the library how much we actually submitted */
            vorbis_analysis_wrote(&vd, (int)i);
        }

        /* vorbis does some data preanalysis, then divvies up blocks for
           more involved (potentially parallel) processing.  Get a single
           block for encoding now */
        while (vorbis_analysis_blockout(&vd, &vb) == 1) {
            /* analysis, assume we want to use bitrate management */
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);
            while (vorbis_bitrate_flushpacket(&vd, &op)) {
                /* weld the packet into the bitstream */
                ogg_stream_packetin(&os, &op);

                /* write out pages (if any) */
                while(!eos) {
                    int result = ogg_stream_pageout(&os, &og);
                    if (result == 0) break;
                    fwrite(og.header, 1, og.header_len, out);
                    fwrite(og.body, 1, og.body_len, out);

                    /* this could be set above, but for illustrative purposes, I do
                       it here (to show that vorbis does know where the stream ends) */

                    if (ogg_page_eos(&og)) {
                        eos = 1;
                    }
                }
            }
        }
    }
    
    /* clean up and exit.  vorbis_info_clear() must be called last */
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    fclose(fd);
    fclose(out);
    /* ogg_page and ogg_packet structs always point to storage in
       libvorbis.  They're never freed or manipulated directly */
    
    return true;
}

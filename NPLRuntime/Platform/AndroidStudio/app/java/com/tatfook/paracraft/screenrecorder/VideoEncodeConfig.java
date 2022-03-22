//-----------------------------------------------------------------------------
// ScreenRecorder.java
// Authors: big
// CreateDate: 2022.2.22
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.screenrecorder;

import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import java.util.Objects;

public class VideoEncodeConfig {
    final int width;
    final int height;
    final int bitrate;
    final int framerate;
    final int iframeInterval;
    final String codecName;
    final String mimeType;
    final MediaCodecInfo.CodecProfileLevel codecProfileLevel;

    /**
     * @param codecName         selected codec name, maybe null
     * @param mimeType          video MIME type, cannot be null
     * @param codecProfileLevel profile level for video encoder nullable
     */
    public VideoEncodeConfig(int width,
                             int height,
                             int bitrate,
                             int framerate,
                             int iframeInterval,
                             String codecName,
                             String mimeType,
                             MediaCodecInfo.CodecProfileLevel codecProfileLevel) {
        this.width = width;
        this.height = height;
        this.bitrate = bitrate;
        this.framerate = framerate;
        this.iframeInterval = iframeInterval;
        this.codecName = codecName;
        this.mimeType = Objects.requireNonNull(mimeType);
        this.codecProfileLevel = codecProfileLevel;
    }

    MediaFormat toFormat() {
        MediaFormat format = MediaFormat.createVideoFormat(mimeType, width, height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, bitrate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, framerate);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, iframeInterval);
        if (codecProfileLevel != null && codecProfileLevel.profile != 0 && codecProfileLevel.level != 0) {
            format.setInteger(MediaFormat.KEY_PROFILE, codecProfileLevel.profile);
            format.setInteger("level", codecProfileLevel.level);
        }
        // maybe useful
        // format.setInteger(MediaFormat.KEY_REPEAT_PREVIOUS_FRAME_AFTER, 10_000_000);
        return format;
    }

    @Override
    public String toString() {
        return "VideoEncodeConfig{" +
                "width=" + width +
                ", height=" + height +
                ", bitrate=" + bitrate +
                ", framerate=" + framerate +
                ", iframeInterval=" + iframeInterval +
                ", codecName='" + codecName + '\'' +
                ", mimeType='" + mimeType + '\'' +
                ", codecProfileLevel=" + (codecProfileLevel == null ? "" : Utils.avcProfileLevelToString(codecProfileLevel)) +
                '}';
    }
}

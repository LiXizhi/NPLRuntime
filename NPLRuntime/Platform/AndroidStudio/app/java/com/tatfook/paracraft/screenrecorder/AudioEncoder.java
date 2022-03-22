//-----------------------------------------------------------------------------
// ScreenRecorder.java
// Authors: big
// CreateDate: 2022.2.22
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.screenrecorder;

import android.media.MediaFormat;

public class AudioEncoder extends BaseEncoder {
    private final AudioEncodeConfig mConfig;

    AudioEncoder(AudioEncodeConfig config) {
        super(config.codecName);
        this.mConfig = config;
    }

    @Override
    protected MediaFormat createMediaFormat() {
        return mConfig.toFormat();
    }
}

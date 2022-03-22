//-----------------------------------------------------------------------------
// ScreenRecorder.java
// Authors: big
// CreateDate: 2022.2.22
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.screenrecorder;

import java.io.IOException;

interface Encoder {
    void prepare() throws IOException;

    void stop();

    void release();

    void setCallback(Callback callback);

    interface Callback {
        void onError(Encoder encoder, Exception exception);
    }
}

//-----------------------------------------------------------------------------
// ScreenRecorder.java
// Authors: big
// CreateDate: 2022.2.22
// ModifyDate: 2022.3.7
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.screenrecorder;

import android.app.Service;
import android.content.Intent;
import android.media.projection.MediaProjection;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.util.Log;

import java.util.Objects;

public class ScreenRecorderService extends Service {
    private String TAG = "ScreenRecorderService";

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        startForeground(
            Notifications.id,
            ScreenRecorder
                .getInstance()
                .getNotifications()
                .recording(0)
        );

        int resultCode = intent.getIntExtra("code", -1);
        Intent resultData = intent.getParcelableExtra("data");

        MediaProjection mediaProjection =
            ScreenRecorder
                .getInstance()
                .getMediaProjectionManager()
                .getMediaProjection(resultCode, Objects.requireNonNull(resultData));

        if (mediaProjection == null) {
            Log.e("%s", "Media projection is NULL");
        }

        ScreenRecorder.getInstance().setMediaProjection(mediaProjection);
        ScreenRecorder.getInstance().startRecorder(mediaProjection);

        return super.onStartCommand(intent, flags, startId);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

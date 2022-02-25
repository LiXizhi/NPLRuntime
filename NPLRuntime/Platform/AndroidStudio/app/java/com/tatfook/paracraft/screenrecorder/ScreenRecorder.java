//-----------------------------------------------------------------------------
// ScreenRecorder.java
// Authors: big
// CreateDate: 2022.2.22
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.screenrecorder;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.projection.MediaProjectionManager;
import android.util.Log;

import com.tatfook.paracraft.BuildConfig;
import com.tatfook.paracraft.ParaEngineActivity;

import static android.content.Context.MEDIA_PROJECTION_SERVICE;

public class ScreenRecorder {
    private static ScreenRecorder instance;
    private MediaProjectionManager mMediaProjectionManager;
    private ScreenRecorderHelper mRecorderHelper;
    private BroadcastReceiver mStopActionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ACTION_STOP.equals(intent.getAction())) {
//                stopRecordingAndOpenFile(context);
            }
        }
    };

    static final String ACTION_STOP = BuildConfig.APPLICATION_ID + ".action.STOP";

    public static ScreenRecorder getInstance() {
        return instance;
    }

    public static void start() {
        Log.d("ScreenRecorder!!!!!", "test!!!!!!");
    }

    public static void stop() {

    }

    public static void save() {

    }

    public static void play() {

    }

    public ScreenRecorder() {
        mMediaProjectionManager = (MediaProjectionManager) ParaEngineActivity.getContext()
                                                                             .getApplicationContext()
                                                                             .getSystemService(MEDIA_PROJECTION_SERVICE);

        instance = this;
    }

    public MediaProjectionManager getMediaProjectionManager() {
        return mMediaProjectionManager;
    }

    public void exec() {

    }

    private void startRecord() {
        Log.d("ScreenRecorder", "from start record");

        if (mRecorderHelper == null) {
            return;
        }

        mRecorderHelper.start();
//        mButton.setText(getString(R.string.stop_recorder));
        ParaEngineActivity.getContext().registerReceiver(mStopActionReceiver, new IntentFilter(ACTION_STOP));
    }

//    private void stopRecordingAndOpenFile(Context context) {
//        File file = new File(mRecorder.getSavedPath());
//        stopRecorder();
//        Toast.makeText(context, getString(R.string.recorder_stopped_saved_file) + " " + file, Toast.LENGTH_LONG).show();
//        StrictMode.VmPolicy vmPolicy = StrictMode.getVmPolicy();
//
//        try {
//            // disable detecting FileUriExposure on public file
//            StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder().build());
//            viewResult(file);
//        } finally {
//            StrictMode.setVmPolicy(vmPolicy);
//        }
//    }
}

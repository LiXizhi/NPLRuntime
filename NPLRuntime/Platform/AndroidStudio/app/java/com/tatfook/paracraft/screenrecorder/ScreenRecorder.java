//-----------------------------------------------------------------------------
// ScreenRecorder.java
// Authors: big
// CreateDate: 2023.8.7
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.screenrecorder;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.MediaCodecInfo;
import android.media.MediaScannerConnection;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.Environment;
import androidx.annotation.Keep;

import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.shuyu.gsyvideoplayer.video.StandardGSYVideoPlayer;
import com.tatfook.paracraft.ParaEngineActivity;
import com.tatfook.paracraft.RequestAndroidPermission;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Stack;

import static android.Manifest.permission.RECORD_AUDIO;
import static android.Manifest.permission.WRITE_EXTERNAL_STORAGE;
import static android.content.Context.MEDIA_PROJECTION_SERVICE;
import static com.tatfook.paracraft.screenrecorder.ScreenRecorderHelper.AUDIO_AAC;
import static com.tatfook.paracraft.screenrecorder.ScreenRecorderHelper.VIDEO_AVC;

public class ScreenRecorder {
    private static final int REQUEST_MEDIA_PROJECTION = 1;
    private static ScreenRecorder mInstance;
    private static String mLastFilePath;
    private static String mLastFileSavePath;
    private final Notifications mNotifications;
    private final MediaProjectionManager mMediaProjectionManager;
    private MediaProjection mMediaProjection;
    private ScreenRecorderHelper mRecorderHelper;
    private Intent mScreenRecorderServiceIntent;
    private String mVideoCodecName;
    private String mAudioCodecName;
    private VirtualDisplay mVirtualDisplay;
    private static native void nativeStartedCallbackFunc();
    private static native void nativeRecordFinishedCallbackFunc(String filePath);

//    private BroadcastReceiver mStopActionReceiver = new BroadcastReceiver() {
//        @Override
//        public void onReceive(Context context, Intent intent) {
//            if (ACTION_STOP.equals(intent.getAction())) {
//            }
//        }
//    };

//    static final String ACTION_STOP = BuildConfig.APPLICATION_ID + ".action.STOP";

    interface Callback {
        void onResult();
    }

    public static ScreenRecorder getInstance() {
        return mInstance;
    }

    @Keep
    public static void start() {
        if (!hasPermissions()) {
            requestPermissions();
            return;
        }

        if (getInstance() == null) {
            new ScreenRecorder(() -> {
                // request media projection
                Intent captureIntent = getInstance().getMediaProjectionManager().createScreenCaptureIntent();
                ParaEngineActivity.getContext().startActivityForResult(captureIntent, REQUEST_MEDIA_PROJECTION);
            });
        } else {
            if (getInstance().getMediaProjection() != null) {
                getInstance().startRecorder(getInstance().getMediaProjection());
            }
        }
    }

    @Keep
    public static void stop() {
        if (getInstance() == null) {
            return;
        }

        getInstance().stopRecorder();
    }

    @Keep
    public static String save() throws IOException {
        if (mLastFilePath == null) {
            return "";
        }

        File file = new File(mLastFilePath);
        File saveFile = new File(mLastFileSavePath);

        boolean bSucceeded = false;

        try {
            FileChannel in = new FileInputStream(file).getChannel();
            FileChannel out = new FileOutputStream(saveFile).getChannel();

            in.transferTo(0, in.size(), out);

            bSucceeded = true;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        if (!bSucceeded) {
            return "";
        }

        // ContentValues contentValues = new ContentValues();
        // long currentTime = System.currentTimeMillis();

        // contentValues.put(MediaStore.MediaColumns.TITLE, saveFile.getName());
        // contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, saveFile.getName());
        // contentValues.put(MediaStore.MediaColumns.DATE_MODIFIED, currentTime);
        // contentValues.put(MediaStore.MediaColumns.DATE_ADDED, currentTime);
        // contentValues.put(MediaStore.MediaColumns.SIZE, saveFile.length());
        // contentValues.put(MediaStore.Video.VideoColumns.DATE_TAKEN, currentTime);
        // contentValues.put(MediaStore.Video.VideoColumns.MIME_TYPE, "video/mp4");

        // ParaEngineActivity.getContext().getContentResolver().insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, contentValues);

        MediaScannerConnection.scanFile(
            ParaEngineActivity.getContext(),
            new String[]{ mLastFileSavePath },
            null,
            null
        );

        return mLastFileSavePath;
    }

    @Keep
    public static void play() {
        if (mLastFilePath == null) {
            return;
        }

        ParaEngineActivity.getContext().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                RelativeLayout relativeLayout = new RelativeLayout(ParaEngineActivity.getContext());

                RelativeLayout.LayoutParams relativeLayoutParams =
                        new RelativeLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                        );

                relativeLayoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                relativeLayoutParams.addRule(RelativeLayout.CENTER_HORIZONTAL, RelativeLayout.TRUE);

                StandardGSYVideoPlayer player = new StandardGSYVideoPlayer(ParaEngineActivity.getContext());
                player.setUp(mLastFilePath, true, "预览");
                player.getFullscreenButton().setVisibility(View.INVISIBLE);
                player.getBackButton().setVisibility(View.VISIBLE);
                player.getBackButton().setOnClickListener(v -> {
                    if (com.shuyu.gsyvideoplayer.R.id.back == v.getId()) {
                        ParaEngineActivity.getContext().getFrameLayout().removeView(relativeLayout);
                    }
                });

                // add view
                relativeLayout.addView(player);
                ParaEngineActivity.getContext().getFrameLayout().addView(relativeLayout, relativeLayoutParams);

                relativeLayout.requestFocus();
            }
        });
    }

    @Keep
    public static void removeVideo() {
        // remove temp file
        File fileToDelete = new File(mLastFilePath);

        if (fileToDelete.exists()) {
            fileToDelete.delete();
        }
    }

    public static void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_MEDIA_PROJECTION) {
            getInstance().mScreenRecorderServiceIntent =
                new Intent(ParaEngineActivity.getContext(), ScreenRecorderService.class);

            getInstance()
                .mScreenRecorderServiceIntent
                .putExtra("code", resultCode);

            getInstance()
                .mScreenRecorderServiceIntent
                .putExtra("data", data);

            if (data == null) {
                mInstance = null;
                return;
            }

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                ParaEngineActivity
                    .getContext()
                    .startForegroundService(getInstance().mScreenRecorderServiceIntent);
            }
        }
    }

    public static void onRequestPermissionsResult(int granted) {
        if (granted == 0) {
            start();
        }
    }

    private static void requestPermissions() {
        ParaEngineActivity
            .getContext()
            .requestPermissions(
                new String[]{WRITE_EXTERNAL_STORAGE, RECORD_AUDIO},
                RequestAndroidPermission.SCREEN_RECORD_PERMISSIONS_CODE
            );
    }

    private static boolean hasPermissions() {
        PackageManager pm = ParaEngineActivity.getContext().getPackageManager();
        String packageName = ParaEngineActivity.getContext().getPackageName();

        int granted = pm.checkPermission(RECORD_AUDIO, packageName) | pm.checkPermission(WRITE_EXTERNAL_STORAGE, packageName);

        return granted == PackageManager.PERMISSION_GRANTED;
    }

    public ScreenRecorder(Callback callback) {
        mInstance = this;
        mNotifications = new Notifications(ParaEngineActivity.getContext().getApplicationContext());
        mMediaProjectionManager =
            (MediaProjectionManager) ParaEngineActivity.getContext()
            .getApplicationContext()
            .getSystemService(MEDIA_PROJECTION_SERVICE);

        // prepare video codec.
        Utils.findEncodersByTypeAsync(VIDEO_AVC, videoInfos -> {
            String[] videoNames = new String[videoInfos.length];

            for (int i = 0; i < videoInfos.length; i++) {
                videoNames[i] = videoInfos[i].getName();
            }

            if (videoNames[0] != null) {
                mVideoCodecName = videoNames[0];
            }

            // prepare audio codec.
            Utils.findEncodersByTypeAsync(AUDIO_AAC, audioInfos -> {
                String[] audioNames = new String[videoInfos.length];

                for (int i = 0; i < audioInfos.length; i++) {
                    audioNames[i] = audioInfos[i].getName();
                }

                if (audioNames[0] != null) {
                    mAudioCodecName = audioNames[0];
                }
            });

            callback.onResult();
        });
    }

    public MediaProjectionManager getMediaProjectionManager() {
        return mMediaProjectionManager;
    }

    public Notifications getNotifications() {
        return mNotifications;
    }

    public void setMediaProjection(MediaProjection mediaProjection) {
        mMediaProjection = mediaProjection;
    }

    public MediaProjection getMediaProjection() {
        return mMediaProjection;
    }

    private ScreenRecorderHelper recorderHelper(MediaProjection mediaProjection,
                                                VideoEncodeConfig video,
                                                AudioEncodeConfig audio,
                                                File output) {
        final VirtualDisplay display = getOrCreateVirtualDisplay(mediaProjection, video);

        ScreenRecorderHelper r = new ScreenRecorderHelper(video, audio, display, output.getAbsolutePath());

        r.setCallback(new ScreenRecorderHelper.Callback() {
            long startTime = 0;

            @Override
            public void onStop(Throwable error) {
                ParaEngineActivity.getContext().runOnUiThread(() -> stopRecorder());

                ParaEngineActivity
                    .getContext()
                    .stopService(mScreenRecorderServiceIntent);

                mInstance = null;
                nativeRecordFinishedCallbackFunc(mLastFilePath);
            }

            @Override
            public void onStart() {
                nativeStartedCallbackFunc();
            }

            @Override
            public void onRecording(long presentationTimeUs) {
                if (startTime <= 0) {
                    startTime = presentationTimeUs;
                }

                long time = (presentationTimeUs - startTime) / 1000;
                mNotifications.recording(time);
            }
        });

        return r;
    }

    public void startRecorder(MediaProjection mediaProjection) {
        if (mRecorderHelper == null) {
            VideoEncodeConfig video = createVideoConfig();
            AudioEncodeConfig audio = createAudioConfig(); // audio can be null

            if (video == null) {
                return;
            }

            File dir;
            File saveDir;

            // dir = new File(
            //         ParaEngineActivity
            //             .getContext()
            //             .getExternalFilesDir(Environment.DIRECTORY_MOVIES),
            //             "ParacraftScreenRecorder/temp"
            // );

            // saveDir = new File(
            //         ParaEngineActivity
            //             .getContext()
            //             .getExternalFilesDir(Environment.DIRECTORY_MOVIES),
            //             "ParacraftScreenRecorder"
            // );

            dir = new File(
                Environment
                    .getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES),
                "ParacraftScreenRecorder/temp"
            );

            saveDir = new File(
                Environment
                    .getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES),
                "ParacraftScreenRecorder"
            );

            Stack<File> tmpFileStack = new Stack<File>();
            tmpFileStack.push(dir);

            try {
                while(!tmpFileStack.isEmpty()) {
                    File curFile = tmpFileStack.pop();

                    if (null == curFile) {
                        continue;
                    }

                    if (curFile.isFile()) {
                        curFile.delete();
                    } else {
                        File[] tmpSubFileList = curFile.listFiles();

                        if (tmpSubFileList == null ||tmpSubFileList.length == 0) {	//空文件夹直接删
                            curFile.delete();
                        } else {
                            // tmpFileStack.push(curFile); // may cause dead cycle

                            for (File item : tmpSubFileList) {
                                tmpFileStack.push(item);
                            }
                        }
                    }
                }
            } catch (Exception e) {}

            if (!saveDir.exists() && !saveDir.mkdirs()) {
                cancelRecorder();
                return;
            }

            if (!dir.exists() && !dir.mkdirs()) {
                cancelRecorder();
                return;
            }

            SimpleDateFormat format = new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.SIMPLIFIED_CHINESE);
            final File file = new File(dir, "Paracraft-Screenshots-" + format.format(new Date())  + ".mp4");
            final File saveFile = new File(saveDir, "Paracraft-Screenshots-" + format.format(new Date())  + ".mp4");

            mLastFilePath = file.toString();
            mLastFileSavePath = saveFile.toString();

            mRecorderHelper = recorderHelper(mediaProjection, video, audio, file);

            // permission
            if (!hasPermissions()) {
                cancelRecorder();
                return;
            }
        }

        if (mRecorderHelper == null) {
            return;
        }

        mRecorderHelper.start();
//        ParaEngineActivity.getContext().registerReceiver(mStopActionReceiver, new IntentFilter(ACTION_STOP));
    }

    private void stopRecorder() {
        if (mRecorderHelper != null) {
            mRecorderHelper.quit();
        }

        mRecorderHelper = null;

//        try {
//            ParaEngineActivity.getContext().unregisterReceiver(mStopActionReceiver);
//        } catch (Exception e) {
//            //ignored
//        }
    }

    private void cancelRecorder() {
        if (mRecorderHelper == null) return;

        stopRecorder();
    }

    private VirtualDisplay getOrCreateVirtualDisplay(MediaProjection mediaProjection, VideoEncodeConfig config) {
        if (mVirtualDisplay == null) {
            mVirtualDisplay =
                mediaProjection
                    .createVirtualDisplay(
                        "ScreenRecorder-display0",
                        config.width,
                        config.height,
                       1 /*dpi*/,
                        DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC,
                       null /*surface*/,
                       null,
                       null
                    );
        } else {
            // resize if size not matched
            Point size = new Point();
            mVirtualDisplay.getDisplay().getSize(size);

            if (size.x != config.width || size.y != config.height) {
                mVirtualDisplay.resize(config.width, config.height, 1);
            }
        }

        return mVirtualDisplay;
    }

    private VideoEncodeConfig createVideoConfig() {
        final String codec = mVideoCodecName;

        if (codec == null) {
            return null;
        }

        // video size
        int width = 1920;
        int height = 1080;
        int frameRate = 60;
        int iframe = 1;
        int bitrate = 16000 * 1000;
        MediaCodecInfo.CodecProfileLevel profileLevel = Utils.toProfileLevel("Default");

        return new VideoEncodeConfig(width, height, bitrate, frameRate, iframe, codec, VIDEO_AVC, profileLevel);
    }

    private AudioEncodeConfig createAudioConfig() {
        String codec = mAudioCodecName;

        if (codec == null) {
            return null;
        }

        int bitrate = 320 * 1000;
        int sampleRate = 44100;
        int channelCount = 2;
        int profile = MediaCodecInfo.CodecProfileLevel.AACObjectMain;

        return new AudioEncodeConfig(codec, AUDIO_AAC, bitrate, sampleRate, channelCount, profile);
    }
}

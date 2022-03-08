package com.tatfook.paracraft.screenrecorder;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.SystemClock;
import android.text.format.DateUtils;
import android.util.Log;

import com.tatfook.paracraft.R;

import static com.tatfook.paracraft.screenrecorder.ScreenRecorder.ACTION_STOP;

public class Notifications extends ContextWrapper {
    public static final int id = 0xffff;
    private static final String CHANNEL_ID = "Paracraft";
    private static final String CHANNEL_NAME = "屏幕录制";

    private long mLastFiredTime = 0;
    private NotificationManager mManager;
    private Notification.Action mStopAction;
    private Notification.Builder mBuilder;

    Notifications(Context context) {
        super(context);
        createNotificationChannel();
    }

    public Notification recording(long timeMs) {
        if (SystemClock.elapsedRealtime() - mLastFiredTime < 1000) {
            return null;
        }

        Notification notification =
            getBuilder()
            .setContentText("时长: " + DateUtils.formatElapsedTime(timeMs / 1000))
            .build();



        getNotificationManager().notify(id, notification);

        mLastFiredTime = SystemClock.elapsedRealtime();

        return notification;
    }

    public void clear() {
        mLastFiredTime = 0;
        mBuilder = null;
        mStopAction = null;

        getNotificationManager().cancelAll();
    }

    public NotificationManager getNotificationManager() {
        if (mManager == null) {
            mManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        }

        return mManager;
    }

    private Notification.Builder getBuilder() {
        if (mBuilder == null) {
            Notification.Builder builder =
                new Notification.Builder(this)
                .setContentTitle("录制中...")
                .setOngoing(true)
                .setLocalOnly(true)
                .setOnlyAlertOnce(true)
                .addAction(stopAction())
                .setWhen(System.currentTimeMillis())
                .setSmallIcon(R.drawable.ic_launcher);

            builder.setChannelId(CHANNEL_ID).setUsesChronometer(true);

            mBuilder = builder;
        }

        return mBuilder;
    }

    private void createNotificationChannel() {
        NotificationChannel channel =
            new NotificationChannel(CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);

        channel.setShowBadge(false);
        getNotificationManager().createNotificationChannel(channel);
    }

    private Notification.Action stopAction() {
        if (mStopAction == null) {
            Intent intent = new Intent(ACTION_STOP).setPackage(getPackageName());

            PendingIntent pendingIntent =
                PendingIntent
                .getBroadcast(
                    this,
                    1,
                    intent,
                    PendingIntent.FLAG_ONE_SHOT
                );

            mStopAction = new Notification.Action(android.R.drawable.ic_media_pause, "Stop", pendingIntent);
        }

        return mStopAction;
    }
}

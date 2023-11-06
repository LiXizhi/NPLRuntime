package com.tatfook.paracraft;

import android.util.Log;
import android.Manifest;
import android.content.pm.PackageManager;

public class RequestAndroidPermission {
    static private String TAG = "RequestAndroidPermission";
    static private final int RECORD_AUDIO_PERMISSION_REQUEST_CODE = 1;

    static public boolean RequestRecordAudio() {
        int permission = ParaEngineActivity.getContext().checkSelfPermission(Manifest.permission.RECORD_AUDIO);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            ParaEngineActivity.getContext().requestPermissions(
                new String[]{Manifest.permission.RECORD_AUDIO},
                RECORD_AUDIO_PERMISSION_REQUEST_CODE
            );
            return false;
        } else {
            return true;
        }
    }

    static public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == RECORD_AUDIO_PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // 用户授予了录音权限
            } else {
                // 用户拒绝了录音权限，你可以在这里进行相应处理
            }
        }
    }
}

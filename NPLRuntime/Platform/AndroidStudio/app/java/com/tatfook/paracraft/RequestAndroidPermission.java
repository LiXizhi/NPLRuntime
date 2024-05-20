package com.tatfook.paracraft;

import android.util.Log;
import android.Manifest;
import android.content.pm.PackageManager;

import com.tatfook.plugin.bluetooth.InterfaceBluetooth;

public class RequestAndroidPermission {
    static private String TAG = "RequestAndroidPermission";
    static private ParaEngineActivity mParaEngineActivity = ParaEngineActivity.getContext();

    // all android permission request codes.
    static public final int RECORD_AUDIO_PERMISSION_REQUEST_CODE = 1;
    static public final int BLUETOOTH_SCAN_PERMISSION_REQUEST_CODE = 2;
    static public final int PHONE_STATE_PERMISSION_CODE = 3;
    static public final int SCREEN_RECORD_PERMISSIONS_CODE = 4;
    static public final int COARSE_LOCATION_PERMISSION_CODE = 5;
    static public final int BLUETOOTH_CONNECT_PERMISSION_REQUEST_CODE = 6;
    static public final int CAMERA_PERMISSION_REQUEST_CODE = 7;

    public interface RequestPermissionCallback {
        void Callback(Boolean succeeded);
    }

    static private RequestPermissionCallback curCallback;

    static public boolean RequestLocation(RequestPermissionCallback callback) {
        curCallback = callback;
        int permissionCoarseLocation = mParaEngineActivity.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION);
        int permissionFineLocation = mParaEngineActivity.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION);

        if (permissionFineLocation != PackageManager.PERMISSION_GRANTED ||
            permissionCoarseLocation != PackageManager.PERMISSION_GRANTED) {
            ParaEngineActivity.getContext().requestPermissions(
                new String[]{Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_FINE_LOCATION},
                COARSE_LOCATION_PERMISSION_CODE
            );
            return false;
        } else {
            curCallback.Callback(true);
            return true;
        }
    }

    static public boolean RequestBLEConnect(RequestPermissionCallback callback) {
        curCallback = callback;
        int permission = mParaEngineActivity.checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            ParaEngineActivity.getContext().requestPermissions(
                new String[]{Manifest.permission.BLUETOOTH_CONNECT},
                BLUETOOTH_CONNECT_PERMISSION_REQUEST_CODE
            );
            return false;
        } else {
            curCallback.Callback(true);
            return true;
        }
    }

    static public boolean RequestBLEScan(RequestPermissionCallback callback) {
        curCallback = callback;
        int permission = mParaEngineActivity.checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            ParaEngineActivity.getContext().requestPermissions(
                new String[]{Manifest.permission.BLUETOOTH_SCAN},
                BLUETOOTH_SCAN_PERMISSION_REQUEST_CODE
            );
            return false;
        } else {
            curCallback.Callback(true);
            return true;
        }
    }

    static public boolean RequestRecordAudio() {
        int permission = mParaEngineActivity.checkSelfPermission(Manifest.permission.RECORD_AUDIO);

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

    static public boolean RequestCamera() {
        int permission = mParaEngineActivity.checkSelfPermission(Manifest.permission.CAMERA);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            ParaEngineActivity.getContext().requestPermissions(
                new String[]{Manifest.permission.CAMERA},
                CAMERA_PERMISSION_REQUEST_CODE
            );
            return false;
        } else {
            return true;
        }
    }

    static public boolean HasPermission(int permissionCode) {
        String permissionString = "";
        
        switch (permissionCode) {
            case RECORD_AUDIO_PERMISSION_REQUEST_CODE:
                permissionString = Manifest.permission.RECORD_AUDIO;
                break;
            case COARSE_LOCATION_PERMISSION_CODE:
                permissionString = Manifest.permission.ACCESS_COARSE_LOCATION;
                break;
            case BLUETOOTH_CONNECT_PERMISSION_REQUEST_CODE:
                permissionString = Manifest.permission.BLUETOOTH_CONNECT;
                break;
            case BLUETOOTH_SCAN_PERMISSION_REQUEST_CODE:
                permissionString = Manifest.permission.BLUETOOTH_SCAN;
                break;
            case CAMERA_PERMISSION_REQUEST_CODE:
                permissionString = Manifest.permission.CAMERA;
                break;
            default:
                Log.e(TAG, "Unknown permission code: " + permissionCode);
                return false;
        }

        int permission = mParaEngineActivity.checkSelfPermission(permissionString);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            return false;
        } else {
            return true;
        }
    }

    static public void RequestPermission(int permissionCode) {
        RequestPermissionCallback callback = new RequestPermissionCallback() {
            @Override
            public void Callback(Boolean succeeded) {}
        };

        switch (permissionCode) {
            case RECORD_AUDIO_PERMISSION_REQUEST_CODE:
                RequestRecordAudio();
                break;
            case COARSE_LOCATION_PERMISSION_CODE:
                RequestLocation(callback);
                break;
            case BLUETOOTH_CONNECT_PERMISSION_REQUEST_CODE:
                RequestBLEConnect(callback);
                break;
            case BLUETOOTH_SCAN_PERMISSION_REQUEST_CODE:
                RequestBLEScan(callback);
                break;
            case CAMERA_PERMISSION_REQUEST_CODE:
                RequestCamera();
                break;
            default:
                Log.e(TAG, "Unknown permission code: " + permissionCode);
                break;
        }
    }

    static public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == RECORD_AUDIO_PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // 用户授予了录音权限
            } else {
                // 用户拒绝了录音权限，你可以在这里进行相应处理
            }
        } else if (requestCode == COARSE_LOCATION_PERMISSION_CODE) {
            int permissionCoarseLocation = mParaEngineActivity.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION);
            int permissionFineLocation = mParaEngineActivity.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION);

            if (permissionFineLocation != PackageManager.PERMISSION_GRANTED ||
                permissionCoarseLocation != PackageManager.PERMISSION_GRANTED) {
                curCallback.Callback(false);
            } else {
                curCallback.Callback(true);
            }
        } else if (requestCode == BLUETOOTH_CONNECT_PERMISSION_REQUEST_CODE) {
            int permission = mParaEngineActivity.checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT);

            if (permission != PackageManager.PERMISSION_GRANTED) {
                curCallback.Callback(false);
            } else {
                curCallback.Callback(true);
            }
        } else if (requestCode == BLUETOOTH_SCAN_PERMISSION_REQUEST_CODE) {
            int permission = mParaEngineActivity.checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN);

            if (permission != PackageManager.PERMISSION_GRANTED) {
                curCallback.Callback(false);
            } else {
                curCallback.Callback(true);
            }
        } else if (requestCode == CAMERA_PERMISSION_REQUEST_CODE) {

        }
    }
}

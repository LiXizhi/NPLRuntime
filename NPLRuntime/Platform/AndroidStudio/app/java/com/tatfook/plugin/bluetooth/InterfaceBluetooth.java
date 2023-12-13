//-----------------------------------------------------------------------------
// ParaEngineRenderer.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2023.12.11
//-----------------------------------------------------------------------------

package com.tatfook.plugin.bluetooth;

import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.json.JSONObject;
import org.json.JSONException;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;

import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;

import androidx.annotation.Keep;
import androidx.core.app.ActivityCompat;

import android.provider.Settings;
import android.util.Log;

import com.tatfook.paracraft.ParaEngineActivity;
import com.tatfook.paracraft.ParaEngineLuaJavaBridge;
import com.tatfook.paracraft.LuaFunction;
import com.tatfook.paracraft.ParaEnginePluginInterface;
import com.tatfook.paracraft.ParaEnginePluginWrapper.PluginWrapperListener;
import com.tatfook.paracraft.RequestAndroidPermission;

@Keep
public class InterfaceBluetooth implements ParaEnginePluginInterface {
    public final static String TAG = "InterfaceBluetooth";

    // java call lua enum
    public final static int CHECK_DEVICE = 1101;
    public final static int SET_BLUE_STATUS = 1102;
    public final static int ON_READ_CHARACTERISTIC_FINSHED = 1103;
    public final static int ON_CHARACTERISTIC = 1104;
    public final static int ON_DESCRIPTOR = 1105;
    public final static int ON_SERVICE = 1107;

    private BluetoothAdapter mBluetoothAdapter;
    private final static int REQUEST_BLUETOOTH_CONNECT = 2;

    private boolean mScanning = false;
    private Handler mHandler;
    private String mDeviceAddress;

    public static ParaEngineActivity mMainActivity;

//    private static final long SCAN_PERIOD = 10000;

    private BluetoothLeService mBluetoothLeService;

    private boolean mConnected = false;
    private boolean mReconnect = false;

    private static InterfaceBluetooth mSingle = null;

    private static LuaFunction mLuaFunction = null;

    public String mLuaPath = "";

    public InterfaceBluetooth() {}

    private final ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mBluetoothLeService = ((BluetoothLeService.LocalBinder)service).getService();
            if (!mBluetoothLeService.initialize()) {
                Log.i(TAG, "Unable to initialize Bluetooth");
                mMainActivity.finish();
            }

            // Automatically connects to the device upon successful start-up initialization.
            mBluetoothLeService.connect(mDeviceAddress);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };

    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
                Log.i(TAG, "mGattUpdateReceiver.onReceive.connect");
                mConnected = true;
                callBaseBridge(SET_BLUE_STATUS, "1");
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                Log.i(TAG, "mGattUpdateReceiver.onReceive.disconnect");
                mConnected = false;
                callBaseBridge(SET_BLUE_STATUS, "0");

                if (mReconnect) {
                    mSingle.scanLeDevice(true);
                    mReconnect = false;
                }
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                Log.i(TAG, "mGattUpdateReceiver.onReceive.ACTION_GATT_SERVICES_DISCOVERED");
                String characteristicUUID = intent.getStringExtra("characteristicUUID");
                String serviceUUID = intent.getStringExtra("serviceUUID");

                JSONObject onCharacteristicParams = new JSONObject();

                try {
                    onCharacteristicParams.put("uuid", characteristicUUID);
                } catch (JSONException e) {
                    e.printStackTrace();
                }

                callBaseBridge(ON_CHARACTERISTIC, onCharacteristicParams.toString());

                JSONObject onServiceParams = new JSONObject();

                try {
                    onServiceParams.put("uuid", serviceUUID);
                } catch (JSONException e) {
                    throw new RuntimeException(e);
                }

                callBaseBridge(ON_SERVICE, onServiceParams.toString());
            } else if (BluetoothLeService.ACTION_DATA_CHARACTERISTIC.equals(action)) {
                Log.i(TAG, "mGattUpdateReceiver.onReceive.ACTION_DATA_CHARACTERISTIC");
//                String uuid = intent.getStringExtra(BluetoothLeService.ON_CHARACTERISTIC_UUID);
//                String io = intent.getStringExtra(BluetoothLeService.ON_CHARACTERISTIC_IO);
//                String status = intent.getStringExtra(BluetoothLeService.ON_CHARACTERISTIC_STATUS);
//                String data = intent.getStringExtra(BluetoothLeService.ON_CHARACTERISTIC_DATA);
//                JSONObject luajs_value = new JSONObject();
//                try {
//                    luajs_value.put("uuid", uuid);
//                    luajs_value.put("io", io);
//                    luajs_value.put("status", status);
//                    luajs_value.put("data", data);
//                } catch (JSONException e) {
//                    e.printStackTrace();
//                }
//                System.out.println(luajs_value.toString());
//                callBaseBridge(ON_CHARACTERISTIC, luajs_value.toString());
            } else if (BluetoothLeService.ACTION_DATA_DESCRIPTOR.equals(action)) {
//                warpCheckUUid();

//                String uuid = intent.getStringExtra(BluetoothLeService.ON_DESCRIPTOR_UUID);
//                String io = intent.getStringExtra(BluetoothLeService.ON_DESCRIPTOR_IO);
//                String status = intent.getStringExtra(BluetoothLeService.ON_DESCRIPTOR_STATUS);
//
//                JSONObject luajs_value = new JSONObject();
//                try {
//                    luajs_value.put("uuid", uuid);
//                    luajs_value.put("io", io);
//                    luajs_value.put("status", status);
//                } catch (JSONException e) {
//                    e.printStackTrace();
//                }
//                callBaseBridge(ON_DESCRIPTOR, luajs_value.toString());
            }
        }
    };

    private PluginWrapperListener mOnInitCallback = null;

    @Override
    public boolean onCreate(Context ctx, Bundle savedInstanceState, PluginWrapperListener listener) {
        mMainActivity = (ParaEngineActivity)ctx;

        mHandler = new Handler();
        if (!mMainActivity.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            return false;
        }

        mSingle = this;
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (mBluetoothAdapter != null) {
            Intent intent = new Intent(mMainActivity, BluetoothLeService.class);
            mMainActivity.bindService(intent, mServiceConnection, mMainActivity.BIND_AUTO_CREATE);
            mMainActivity.registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {}

    @Override
    public void onStart() {}

    @Override
    public void onStop() {}

    @Override
    public void onAppBackground() {}

    @Override
    public void onAppForeground() {}

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {}

    @Override
    public void setDebugMode(boolean debug) {}

    @Override
    public void onSaveInstanceState(Bundle outState) {}

    public void onInit(Map<String, Object> cpInfo, boolean bDebug) {
        Log.i(TAG, "onInit:");
    }

    public static void registerLuaCall(String luaPath) {
        mSingle.mLuaPath = luaPath;
    }

    private static BluetoothGattCharacteristic getCharacteristic(String ser_uuid, String cha_uuid) {
        BluetoothGattCharacteristic retcha = null;

        UUID ser_uuid_ = UUID.fromString(ser_uuid);
        BluetoothGattService gattService = mSingle.mBluetoothLeService.getBluetoothGatt().getService(ser_uuid_);
        UUID cha_uuid_ = UUID.fromString(cha_uuid);
        if (gattService != null)
            retcha = gattService.getCharacteristic(cha_uuid_);
        return retcha;
    }

    private static BluetoothGattDescriptor getDescriptor(String ser_uuid, String cha_uuid, String desc_uuid) {
        BluetoothGattCharacteristic retcha = getCharacteristic(ser_uuid, cha_uuid);

        if (retcha != null) {
            UUID desc_uuid_ = UUID.fromString(desc_uuid);
            return retcha.getDescriptor(desc_uuid_);
        }
        return null;
    }

    public static void connectDevice(String deviceAddr) {
        mSingle.mDeviceAddress = deviceAddr;
        if (mSingle.mBluetoothLeService != null) {
            final boolean result = mSingle.mBluetoothLeService.connect(mSingle.mDeviceAddress);
            Log.d(TAG, "link bluetooth Connect request result=" + result);
            if (result)
                mSingle._stopScanLeDevice();
        }
    }

    public static void disconnectBlueTooth() {
        if (mSingle.mBluetoothLeService != null) {
            mSingle.mBluetoothLeService.disconnect();
        }
    }

    public static void writeToCharacteristic(String ser_uuid, String cha_uuid, String data) {
        byte[] dataBytes = data.getBytes();
        int chunkSize = 20;
        ScheduledExecutorService scheduledExecutorService = Executors.newScheduledThreadPool(1);
        int delayBetweenTask = 200;
        int delayCount = 1;

        BluetoothGattCharacteristic characteristic = getCharacteristic(ser_uuid, cha_uuid);
        if (characteristic != null) {
            scheduledExecutorService.schedule(new Runnable() {
                @Override
                public void run() {
                    characteristic.setValue("start".getBytes());
                    mSingle.mBluetoothLeService.writeCharacteristic(characteristic);
                }
            }, delayCount * delayBetweenTask, TimeUnit.MILLISECONDS);

            for (int offset = 0; offset < dataBytes.length; offset += chunkSize) {
                delayCount++;
                byte[] chunk = Arrays.copyOfRange(dataBytes, offset, offset + Math.min(chunkSize, dataBytes.length - offset));

                scheduledExecutorService.schedule(new Runnable() {
                    @Override
                    public void run() {
                        characteristic.setValue(chunk);
                        mSingle.mBluetoothLeService.writeCharacteristic(characteristic);
                    }
                }, delayCount * delayBetweenTask, TimeUnit.MILLISECONDS);
            }

            delayCount++;

            scheduledExecutorService.schedule(new Runnable() {
                @Override
                public void run() {
                    characteristic.setValue("end".getBytes());
                    mSingle.mBluetoothLeService.writeCharacteristic(characteristic);
                    mSingle.mReconnect = true;
                    disconnectBlueTooth();
                }
            }, delayCount * delayBetweenTask, TimeUnit.MILLISECONDS);

            scheduledExecutorService.shutdown();
        }
    }

//    public static byte[] HexString2Bytes(String str) {
//        if (str == null || str.trim().equals("")) {
//            return new byte[0];
//        }
//
//        byte[] bytes = new byte[str.length() / 2];
//        for (int i = 0; i < str.length() / 2; i++) {
//            String subStr = str.substring(i * 2, i * 2 + 2);
//            bytes[i] = (byte) Integer.parseInt(subStr, 16);
//        }
//
//        return bytes;
//    }

    public static String characteristicGetStrValue(String ser_uuid, String cha_uuid) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(ser_uuid, cha_uuid);
        if (characteristic != null) {
            final byte[] data = characteristic.getValue();
            String currDataStr = BluetoothLeService.characteristicData2JsStrValue(data);
            Log.i(TAG, "characteristicGetStrValue currDataStr:" + currDataStr);

            return currDataStr;
        }
        return null;
    }

    public static int characteristicGetIntValue(String ser_uuid, String cha_uuid, String wdata_str) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(ser_uuid, cha_uuid);
        if (characteristic != null) {
            int flag = characteristic.getProperties();
            int format = -1;
            if ((flag & 0x01) != 0) {
                format = BluetoothGattCharacteristic.FORMAT_UINT16;
            } else {
                format = BluetoothGattCharacteristic.FORMAT_UINT8;
            }
            final int value = characteristic.getIntValue(format, 1);
            return value;
        }
        return 0;
    }

    public static void readCharacteristic(String ser_uuid, String cha_uuid) {
        BluetoothGattCharacteristic rcharacteristic = getCharacteristic(ser_uuid, cha_uuid);

        if (rcharacteristic != null) {
            mSingle.mBluetoothLeService.readCharacteristic(rcharacteristic);
        } else
            Log.e(TAG, "readCharacteristic is null" + ser_uuid + "," + cha_uuid);
    }

    public static void setCharacteristicNotification(String ser_uuid, String cha_uuid, boolean isNotify) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(ser_uuid, cha_uuid);
        if (characteristic != null) {
            Log.i(TAG, "setCharacteristicNotification:" + ser_uuid + "," + cha_uuid + "," + isNotify);
            mSingle.mBluetoothLeService.setCharacteristicNotification(characteristic, isNotify);
        }
    }

    public static void setDescriptorNotification(String ser_uuid, String cha_uuid, String desc_uuid) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(ser_uuid, cha_uuid);
        if (characteristic != null) {
            mSingle.mBluetoothLeService.setCharacteristicDescriptor(characteristic, UUID.fromString(desc_uuid));
        }
    }

    public static boolean isBlueConnected() {
        return mSingle.mConnected;
    }

    static ArrayList<String> s_checkUuids = new ArrayList<String>();
    static ArrayList<String> s_checkUuidsForWarp = new ArrayList<String>();

    static String s_deviceName = null;

    public static void setDeviceName(String deviceName) {
        Log.i(TAG, "setDeviceName: " + deviceName);
        s_deviceName = deviceName;
    }

    public static boolean isLocationEnabled() {
        LocationManager locationManager = (LocationManager) mMainActivity.getSystemService(Context.LOCATION_SERVICE);

        if (locationManager != null) {
            // 获取GPS状态
            boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
            // 获取网络定位状态
            boolean isNetworkEnabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);

            // 判断GPS或网络定位是否开启
            return isGpsEnabled || isNetworkEnabled;
        }

        return false;
    }

    public static void openLocationSettings() {
        // 打开设备的定位设置页面
        mMainActivity.startActivity(new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS));
    }

    public static void setupBluetoothDelegate() {
        Log.i(TAG, "setupBluetoothDelegate");

        if (mSingle == null || mSingle.mConnected)
            return;

        if (!mSingle.mBluetoothAdapter.isEnabled()) {
            mMainActivity.startActivityForResult(new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE), REQUEST_BLUETOOTH_CONNECT);
            return;
        }

        if (!isLocationEnabled()) {
            openLocationSettings();
            return;
        }

        RequestAndroidPermission.RequestBLEConnect(new RequestAndroidPermission.RequestPermissionCallback() {
            @Override
            public void Callback(Boolean succeeded) {
                if (succeeded) {
                    RequestAndroidPermission.RequestLocation(new RequestAndroidPermission.RequestPermissionCallback() {
                        @Override
                        public void Callback(Boolean succeeded) {
                            if (succeeded) {
                                RequestAndroidPermission.RequestBLEScan(new RequestAndroidPermission.RequestPermissionCallback() {
                                    @Override
                                    public void Callback(Boolean succeeded) {
                                        if (succeeded) {
                                            mSingle.scanLeDevice(true);
                                        } else {
                                            Log.e(TAG, "scan permission not granted!");
                                        }
                                    }
                                });
                            } else {
                                Log.e(TAG, "location permission not granted!");
                            }
                        }
                    });
                } else {
                    Log.e(TAG, "connect permission not granted!");
                }
            }
        });
    }

    public static void setCharacteristicsUuid(String serUuid, String chaUuid) {
        s_checkUuids.add(serUuid);
        s_checkUuids.add(chaUuid);
    }

    public static String readAllBlueGatt() {
        List<BluetoothGattService> gattServices = mSingle.mBluetoothLeService.getBluetoothGatt().getServices();

        if (gattServices == null)
            return null;

        HashMap<String, String> luaTableMap = new HashMap<String, String>();
        String uuid = null;
        JSONObject luajs_value = new JSONObject();

        try {
            for (BluetoothGattService gattService : gattServices) {
                uuid = gattService.getUuid().toString();
                Log.i(TAG, "charas-gattService-uuid:" + uuid);

                JSONObject serviceChild_js = new JSONObject();

                List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
                for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {
                    String uuid_cha = gattCharacteristic.getUuid().toString();
                    Log.i(TAG, "charas-gattCharacteristic-uuid:" + uuid_cha);

                    luaTableMap.put(uuid_cha, uuid);

                    List<BluetoothGattDescriptor> gattDescriptors = gattCharacteristic.getDescriptors();

                    JSONObject characteristicChild_js = new JSONObject();
                    for (BluetoothGattDescriptor gattDescriptor : gattDescriptors) {
                        String uuid_desc = gattDescriptor.getUuid().toString();
                        characteristicChild_js.put(uuid_desc, "");
                    }
                    serviceChild_js.put(uuid_cha, characteristicChild_js);
                }
                luajs_value.put(uuid, serviceChild_js);
                return luajs_value.toString();
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    private static void callBaseBridge(int pId, String extData) {
        final String mergeData = "msg = [[" + pId + "_" + extData + "]]";

//        final LuaFunction luaFunction = mLuaFunction;
        if (mSingle.mLuaPath != null) {
            mMainActivity.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    String filePath = mSingle.mLuaPath;

                    ParaEngineLuaJavaBridge.nplActivate(filePath, mergeData);
                }
            });
        }
    }

//    private void warpCheckUUid() {
//        if (s_checkUuidsForWarp.size() >= 2) {
//            String ser_uuid = s_checkUuidsForWarp.get(0);
//            String cha_uuid = s_checkUuidsForWarp.get(1);
//            BluetoothGattCharacteristic characteristic = getCharacteristic(ser_uuid, cha_uuid);
//            mSingle.mBluetoothLeService.setCharacteristicNotification(characteristic, true);
//            List<BluetoothGattDescriptor> gattDescriptors = characteristic.getDescriptors();
//            for (BluetoothGattDescriptor gattDescriptor : gattDescriptors) {
//                Log.i(TAG, "set notify ser:" + ser_uuid + ",cha:" + cha_uuid + ",desc:" + gattDescriptor.getUuid());
//                mSingle.mBluetoothLeService.setCharacteristicDescriptor(characteristic, gattDescriptor.getUuid());
//            }
//
//            //mSingle.mBluetoothLeService.readCharacteristic(characteristic);
//            s_checkUuidsForWarp.remove(0);
//            s_checkUuidsForWarp.remove(0);
//        }
//    }

//    private void enableTXNotification(List<BluetoothGattService> gattServices) {
//        warpCheckUUid();
//    }

    private void _stopScanLeDevice() {
        if (mScanning) {
            mScanning = false;
            if (mBluetoothAdapter != null) {
                BluetoothLeScanner bluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
                if (bluetoothLeScanner != null)
                    bluetoothLeScanner.stopScan(mLeScanCallback);
            }
        }
    }

    private void _startScanLeDevice() {
        if (!mScanning && !mConnected) {
            mScanning = true;
            if (mBluetoothAdapter != null) {
                BluetoothLeScanner bluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();

                if (bluetoothLeScanner != null) {
                    if (mMainActivity.checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                        return;
                    } else {
                        bluetoothLeScanner.startScan(mLeScanCallback);
                    }
                }
            }
        }
    }

    private void scanLeDevice(final boolean enable) {
        if (enable) {
            _startScanLeDevice();
//            mHandler.postDelayed(new Runnable() {
//                @Override
//                public void run() {
//                    if (mConnected == false) {
//                        _stopScanLeDevice();
//                        scanLeDevice(true);
//                    }
//                }
//            }, SCAN_PERIOD);
        } else {
            _stopScanLeDevice();
        }
    }

    public void onPause() {
    }

    public void onResume() {
//        if(!mConnected)
//        {
//
//            // searchBlueDevice();
//            mMainActivity.registerReceiver(
//                mGattUpdateReceiver,
//                makeGattUpdateIntentFilter()
//            );
//
//            if (mBluetoothLeService != null) {
//                if (mDeviceAddress != null) {
//                    final boolean result = mBluetoothLeService.connect(mDeviceAddress);
//                    Log.d(TAG, "Connect request result=" + result);
//                }
//            }
//        }
    }

    public void onDestroy() {
        Log.i(TAG, "app-activity-onDestroy");

        if (mBluetoothLeService != null) {
            mBluetoothLeService.disconnect();
        }

        mMainActivity.unbindService(mServiceConnection);
        mBluetoothLeService = null;
    }

    private ScanCallback mLeScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if (!mScanning) {
                return;
            }

            if (mConnected) {
                return;
            }

            BluetoothDevice device = result.getDevice();

            if (s_deviceName != null) {
                if (device.getName() == null || device.getName().equals(s_deviceName) == false)
                    return;
            }

            int checkrssi = result.getRssi();

            JSONObject luajs_value = new JSONObject();
            try {
                luajs_value.put("name", device.getName());
                luajs_value.put("addr", device.getAddress());
                luajs_value.put("rssi", checkrssi);
            } catch(JSONException e) {
                e.printStackTrace();
            }

            callBaseBridge(CHECK_DEVICE, luajs_value.toString());
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            super.onBatchScanResults(results);
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
        }
    };

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_CHARACTERISTIC);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_DESCRIPTOR);
        return intentFilter;
    }
}

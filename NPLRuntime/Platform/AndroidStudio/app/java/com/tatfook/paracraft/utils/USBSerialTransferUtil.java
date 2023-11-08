package com.tatfook.paracraft.utils;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;

import androidx.annotation.Keep;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;
import com.tatfook.paracraft.BuildConfig;
import com.tatfook.paracraft.ParaEngineActivity;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.List;

public class USBSerialTransferUtil {
    static String TAG = "USBSerialTransferUtil";
    ParaEngineActivity APP = ParaEngineActivity.getContext(); // MainActivity.
    public UsbManager usbManager = (UsbManager) APP.getSystemService(Context.USB_SERVICE);
    private enum UsbPermission { Unknown, Requested, Granted, Denied }

    private BroadcastReceiver usbSerialReceiver = null; // Broadcast monitoring: Determine device authorization operations.
    private List<UsbSerialDriver> availableDrivers;
    private UsbSerialDriver driver;
    private UsbSerialPort usbSerialPort = null;
    private UsbPermission usbPermission = UsbPermission.Unknown;
    private SerialInputOutputManager ioManager = null;
    private Handler mainLooper = null;
    private boolean isConnectUSBSerial = false;
    private static final int WRITE_WAIT_MILLIS = 2000;
    private static final int READ_WAIT_MILLIS = 2000;
    private static final String INTENT_ACTION_GRANT_USB = BuildConfig.APPLICATION_ID + ".GRANT_USB"; // usb permission request identification.

    private static USBSerialTransferUtil usbSerialTransferUtil;
    private static native void nativeRead(String data);

    // ----- Singleton -----
    public static USBSerialTransferUtil getInstance() {
        if (usbSerialTransferUtil == null) {
            usbSerialTransferUtil = new USBSerialTransferUtil();
            usbSerialTransferUtil.mainLooper = new Handler(Looper.getMainLooper());
        }
        return usbSerialTransferUtil;
    }

    @Keep
    private static void nativeOpen() {
        getInstance().init();
    }

    @Keep
    private static void nativeClose() {
        getInstance().disconnect(true);
    }

    private static void nativeSend(String data) {
        getInstance().send(data);
    }

    public void init() {
        if (!isConnectUSBSerial) {
            if (usbSerialReceiver == null) {
                registerReceiver(); // Register broadcast listening.
            }

            try {
                refresh(); // Get the equipment.
                requestPermission();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void requestPermission() {
        if (usbPermission == UsbPermission.Granted || driver == null) {
            return;
        }

        int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_MUTABLE : 0;
        PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(APP, 0, new Intent(INTENT_ACTION_GRANT_USB), flags);
        usbManager.requestPermission(driver.getDevice(), usbPermissionIntent);
        usbPermission = UsbPermission.Requested;
    }

    // Register USB authorization to listen to broadcasts.
    public void registerReceiver() {
        usbSerialReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (INTENT_ACTION_GRANT_USB.equals(intent.getAction())) {
                    // Ensure that only one thread performs the tasks inside and does not conflict with other applications.
                    synchronized (this) {
                        // Determine whether permission is granted.
                        boolean havePermission = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false);
                        if (havePermission) {
                            usbPermission = UsbPermission.Granted;

                            try {
                                connect(); // Authorization successful, connect directly.
                            } catch (IOException e) {
                                throw new RuntimeException(e);
                            }
                        } else {
                            usbPermission = UsbPermission.Denied;
                        }
                    }
                } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(intent.getAction())) {
                    init();
                } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(intent.getAction())) {
                    disconnect(false);
                }
            }
        };

        IntentFilter filter = new IntentFilter(INTENT_ACTION_GRANT_USB);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        APP.registerReceiver(usbSerialReceiver, filter);
    }

    public void connect() throws IOException {
        if (availableDrivers == null ||
            driver == null ||
            usbPermission == UsbPermission.Unknown ||
            usbPermission == UsbPermission.Denied ||
            isConnectUSBSerial) {
            return;
        }

        // Open a connection to the first available driver.
        UsbDeviceConnection connection = usbManager.openDevice(driver.getDevice());
        if (connection == null) {
            return;
        }

        usbSerialPort = driver.getPorts().get(0); // Most devices have just one port (port 0)

        if (usbSerialPort == null) {
            return;
        }

        usbSerialPort.open(connection);
        usbSerialPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

        ioManager = new SerialInputOutputManager(usbSerialPort);
        ioManager.setListener(new SerialInputOutputManager.Listener() {
            @Override
            public void onNewData(byte[] data) {
                mainLooper.post(() -> {
                    receive(data);
                });
            }

            @Override
            public void onRunError(Exception e) {

            }
        });
        ioManager.start();

        isConnectUSBSerial = true;
    }

    public void refresh() throws IOException {
        availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager);

        if (availableDrivers == null || availableDrivers.size() == 0) {
            return;
        }

        driver = availableDrivers.get(0);
    }

    // Disconnect
    public void disconnect(boolean flag) {
        try {
            isConnectUSBSerial = false;
            driver = null;
            availableDrivers = null;
            usbPermission = UsbPermission.Unknown;

            if (ioManager != null) {
                ioManager.stop();
                ioManager = null;
            }

            if (usbSerialPort != null) {
                usbSerialPort.close();
                usbSerialPort = null;
            }

            if (flag && usbSerialReceiver != null) {
                APP.unregisterReceiver(usbSerialReceiver);
                usbSerialReceiver = null;
            }
        } catch (Exception e){
            e.printStackTrace();
        }
    }

    public void send(String str) {
        if (!isConnectUSBSerial) {
            return;
        }

        try {
            byte[] data;
            data = (str + "\r\n").getBytes();
            usbSerialPort.write(data, WRITE_WAIT_MILLIS);

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void receive(byte[] data) {
        String receiveStr = new String(data, StandardCharsets.UTF_8); //HexDump.dumpHexString(data);
        nativeRead(receiveStr);
    }
}

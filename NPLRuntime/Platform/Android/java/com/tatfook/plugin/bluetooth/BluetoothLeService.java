/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package plugin.Bluetooth;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.UUID;


public class BluetoothLeService extends Service {
	public final static String  TAG = "ParaEngine";

    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private String mBluetoothDeviceAddress;
    private BluetoothGatt mBluetoothGatt;
    private int mConnectionState = STATE_DISCONNECTED;

    private static final int STATE_DISCONNECTED = 0;
    private static final int STATE_CONNECTING = 1;
    private static final int STATE_CONNECTED = 2;

	//intent action
    public final static String ACTION_GATT_CONNECTED = "plugin.Bluetooth.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED = "plugin.Bluetooth.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED = "plugin.Bluetooth.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_CHARACTERISTIC = "plugin.Bluetooth.ACTION_DATA_CHARACTERISTIC";
    public final static String ACTION_DATA_DESCRIPTOR = "plugin.Bluetooth.ACTION_DATA_DESCRIPTOR";

	// put data
    public final static String ON_CHARACTERISTIC_UUID = "plugin.Bluetooth.CharacteristicId";
	public final static String ON_CHARACTERISTIC_IO = "plugin.Bluetooth.CharacteristicIo";
	public final static String ON_CHARACTERISTIC_STATUS = "plugin.Bluetooth.CharacteristicStatus";

    public final static String ON_DESCRIPTOR_UUID = "plugin.Bluetooth.DescriptorId";
	public final static String ON_DESCRIPTOR_IO = "plugin.Bluetooth.DescriptorIo";
	public final static String ON_DESCRIPTOR_STATUS = "plugin.Bluetooth.DescriptorStatus";

    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() 
	{
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
				Log.i(TAG, "Attempting to start service discovery:" + mBluetoothGatt.discoverServices());

                mConnectionState = STATE_CONNECTED;

				final Intent intent = new Intent(BluetoothLeService.ACTION_GATT_CONNECTED);
				sendBroadcast(intent);                
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
				Log.i(TAG, "Disconnected from GATT server.");
				mBluetoothGatt.close();
                mConnectionState = STATE_DISCONNECTED;

				final Intent intent = new Intent(BluetoothLeService.ACTION_GATT_DISCONNECTED);
				sendBroadcast(intent);   
            }
		}
        
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
				final Intent intent = new Intent(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
				sendBroadcast(intent);   
            } else {
                Log.e(TAG, "onServicesDiscovered received: " + status);
            }
        }
        
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) 
		{
        	Log.e(TAG, "onCharacteristicRead uuid: " + characteristic.getUuid().toString() + ",data:" + new String(characteristic.getValue()) );


			final Intent intent = new Intent(BluetoothLeService.ACTION_DATA_CHARACTERISTIC);
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_UUID, characteristic.getUuid().toString());
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_IO, "r");
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_STATUS, status);
			sendBroadcast(intent);
        }
        
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) 
		{
        	Log.e(TAG, "onCharacteristicChanged uuid: " + characteristic.getUuid().toString() + ",data:" + new String(characteristic.getValue()) );
			final Intent intent = new Intent(BluetoothLeService.ACTION_DATA_CHARACTERISTIC);

			final byte[] data = characteristic.getValue();
			String data_str = new String(data);

			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_UUID, characteristic.getUuid().toString());
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_IO, "c");
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_STATUS, data_str);
			sendBroadcast(intent);
        }


        public void onCharacteristicWrite(BluetoothGatt paramBluetoothGatt, BluetoothGattCharacteristic paramBluetoothGattCharacteristic, int paramInt)
        {
        	Log.e(TAG, "onCharacteristicWrite: ");
			final Intent intent = new Intent(BluetoothLeService.ACTION_DATA_CHARACTERISTIC);
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_UUID, paramBluetoothGattCharacteristic.getUuid().toString());
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_IO, "w");
			intent.putExtra(BluetoothLeService.ON_CHARACTERISTIC_STATUS, "" + paramInt);
			sendBroadcast(intent);
        }

		
        public void onDescriptorWrite(BluetoothGatt paramBluetoothGatt, BluetoothGattDescriptor paramBluetoothGattDescriptor, int paramInt)
        {
        	Log.e(TAG, "onDescriptorWrite: ");
			super.onDescriptorWrite(paramBluetoothGatt, paramBluetoothGattDescriptor, paramInt);

			Log.e(TAG, "onDescriptorWrite-gogogo:" + paramBluetoothGattDescriptor.getCharacteristic().getUuid().toString());
			final Intent intent = new Intent(BluetoothLeService.ACTION_DATA_DESCRIPTOR);
			intent.putExtra(BluetoothLeService.ON_DESCRIPTOR_UUID, paramBluetoothGattDescriptor.getUuid().toString());
			intent.putExtra(BluetoothLeService.ON_DESCRIPTOR_IO, "w");
			intent.putExtra(BluetoothLeService.ON_DESCRIPTOR_STATUS, "" + paramInt);
			sendBroadcast(intent);
        }
    };

    public class LocalBinder extends Binder 
	{
        public BluetoothLeService getService() {
            return BluetoothLeService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) 
	{
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) 
	{
        // After using a given device, you should make sure that BluetoothGatt.close() is called
        // such that resources are cleaned up properly.  In this particular example, close() is
        // invoked when the UI is disconnected from the Service.
        close();
        return super.onUnbind(intent);
    }

    private final IBinder mBinder = new LocalBinder();

    public boolean initialize() 
	{
        // For API level 18 and above, get a reference to BluetoothAdapter through
        // BluetoothManager.
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager == null) {
                Log.e(TAG, "Unable to initialize BluetoothManager.");
                return false;
            }
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }

        return true;
    }

    public boolean connect(final String address) 
	{
        if (mBluetoothAdapter == null || address == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        // Previously connected device.  Try to reconnect.
		/*
        if (mBluetoothDeviceAddress != null && address.equals(mBluetoothDeviceAddress)
                && mBluetoothGatt != null) {
            Log.d(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            if (mBluetoothGatt.connect()) {
                mConnectionState = STATE_CONNECTING;
                return true;
            } else {
                return false;
            }
        }
		*/

        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        if (device == null) {
            Log.w(TAG, "Device not found.  Unable to connect.");
            return false;
        }
        // We want to directly connect to the device, so we are setting the autoConnect
        // parameter to false.
        mBluetoothGatt = device.connectGatt(this, false, mGattCallback);
        Log.d(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;
        return true;
    }

	public BluetoothGatt getBluetoothGatt()
	{
		return mBluetoothGatt;
	}

    public void disconnect() 
	{
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.disconnect();
    }

    public void close() 
	{
        if (mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.close();
        mBluetoothGatt = null;
    }

    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
		Log.w(TAG, "read characteristic!!!");
        mBluetoothGatt.readCharacteristic(characteristic);
    }

    public void writeCharacteristic(BluetoothGattCharacteristic characteristic) 
	{
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        
        boolean isWrite = mBluetoothGatt.writeCharacteristic(characteristic);
        Log.e(TAG, "mBluetoothGatt writeCharacteristic="+isWrite);
        
    }

    public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) 
	{
        if (mBluetoothAdapter == null || mBluetoothGatt == null) 
		{
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }

        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
		mBluetoothGatt.setCharacteristicNotification(characteristic, false);
		mBluetoothGatt.readCharacteristic(characteristic);

		characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        mBluetoothGatt.setCharacteristicNotification(characteristic, true); 
    }

	public void setCharacteristicDescriptor(BluetoothGattCharacteristic characteristic, UUID desc_uuid)
	{
		BluetoothGattDescriptor descriptor = characteristic.getDescriptor(desc_uuid);
		if(descriptor!=null)
		{
			Log.e(TAG, "BluetoothAdapter initialized");
			descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
			this.mBluetoothGatt.writeDescriptor(descriptor);
		}
	}

    public List<BluetoothGattService> getSupportedGattServices() 
	{
        if (mBluetoothGatt == null) return null;
        return mBluetoothGatt.getServices();
    }
}

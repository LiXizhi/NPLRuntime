//-----------------------------------------------------------------------------
// ParaEngineHelper.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import java.net.NetworkInterface;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings;
import androidx.annotation.Keep;
import android.telephony.TelephonyManager;
import android.util.Log;

@Keep
public class ParaEngineHelper {

	private static native void nativeSetContext(final Context pContext, final AssetManager pAssetManager);
	private static native String GetWorldDirectory();
	private static native void OpenFileDialogNativeCallback(String filepath);

	private static boolean mCanReadPhoneState = false;

	public static void init(final Activity activity, boolean bReadPhoneState) {
		mCanReadPhoneState = bReadPhoneState;
		ParaEngineHelper.nativeSetContext((Context)activity, activity.getAssets());
	}

	public static String getCurrentLanguage() {
		return Locale.getDefault().getLanguage();
	}

	public static boolean CopyTextToClipboard(final String text) {
		ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
		try {
			final boolean[] result = {false};
			final Object obj = new Object();
			Runnable runnable = new Runnable() {
				@Override
				public void run() {
					synchronized(obj)
					{
						result[0] = _CopyTextToClipboard(text);
						obj.notify();
					}
				}
			};
			context.runOnUiThread(runnable);
			synchronized(obj)
			{
				obj.wait();
			}
			return  result[0];
		}
		catch (Exception e) {
			return false;
		}
	}

	public static String GetTextFromClipboard() {
		ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
		try {
			final String[] result = {""};
			final Object obj = new Object();
			Runnable runnable = new Runnable() {
				@Override
				public void run() {
					synchronized(obj)
					{
						result[0] = _GetTextFromClipboard();
						obj.notify();
					}
				}
			};
			context.runOnUiThread(runnable);
			synchronized(obj)
			{
				obj.wait();
			}
			return  result[0];
		}
		catch (Exception e) {
			return "";
		}
	}

	private static boolean _CopyTextToClipboard(String text) {
		try {
			ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
			ClipboardManager cm = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);

			ClipData clipData = ClipData.newPlainText("Label", text);
			cm.setPrimaryClip(clipData);

			return true;
		}
		catch (Exception e) {
			return false;
		}
	}

	private static String _GetTextFromClipboard() {
		try {
			ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
			ClipboardManager cm = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);

			ClipData primaryClip = cm.getPrimaryClip();

			if (primaryClip == null)
				return "";
			ClipData.Item itemAt = primaryClip.getItemAt(0);

			if (itemAt == null)
				return "";

			return itemAt.getText().toString();
		}
		catch (Exception e) {
			return "";
		}

	}

	public static String getPackageName() {
		ParaEngineActivity context = ParaEngineActivity.getContext();
		ApplicationInfo applicationInfo = context.getApplicationInfo();
		return applicationInfo.packageName;
	}

	public static String getWritablePath() {
		ParaEngineActivity context = ParaEngineActivity.getContext();
		return context.getFilesDir().getAbsolutePath();
	}

	public static String getExternalStoragePath() {
		ParaEngineActivity context = ParaEngineActivity.getContext();
		return context.getExternalFilesDir(null).getAbsolutePath();
	}

	public static String getObbPath() {
		ParaEngineActivity context = ParaEngineActivity.getContext();
		return context.getObbDir().getAbsolutePath();
	}

	public static int getWifiIP() {
		WifiManager wifiManager = (WifiManager) ParaEngineActivity.getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
		WifiInfo wifiInfo = wifiManager.getConnectionInfo();
		int ipAddress = wifiInfo.getIpAddress();

		return ipAddress;
	}

	private  static String _getWifiMAC() {
		try	{
			WifiManager wifiManager = (WifiManager) ParaEngineActivity.getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
			WifiInfo wifiInfo = wifiManager.getConnectionInfo();

			String macAddress = wifiInfo.getMacAddress();

			return macAddress;
		}
		catch (Exception e) {
			return "";
		}
	}

	public static String getWifiMAC() {
		try {
			List<NetworkInterface> all = Collections.list(NetworkInterface.getNetworkInterfaces());

			for (NetworkInterface nif : all) {
				if (!nif.getName().equalsIgnoreCase("wlan0"))
					continue;
				byte[] macBytes = nif.getHardwareAddress();
				if (macBytes == null)
					return "";
				StringBuffer res1 = new StringBuffer();
				for (byte b : macBytes) {
					res1.append(String.format("%02X", b));
				}

				if (res1.length() > 0)
					res1.deleteCharAt((res1.length() - 1));

				return res1.toString();
			}

			return "";
		} catch (Exception e) {
			return "";
		}
	}

	public static String getMachineID()
	{
		if (mCanReadPhoneState) {
			TelephonyManager tm = (TelephonyManager) ParaEngineActivity.getContext().getSystemService(Context.TELEPHONY_SERVICE);
			String sn = tm.getDeviceId();
			return sn;
		}else {
			return "";
		}
	}

	public static void OpenFileDialog(String filter) {
		ParaEngineActivity context = ParaEngineActivity.getContext();
		context.OpenFileDialog(filter);
	}

	public static void OpenFileDialogCallback(String filepath) {
//		Log.i("Helper", filepath);
		OpenFileDialogNativeCallback(filepath);
	}
}

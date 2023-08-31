//-----------------------------------------------------------------------------
// ParaEngineHelper.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.NetworkInterface;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Environment;
import android.os.FileUtils;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.provider.Settings;
import androidx.annotation.Keep;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.webkit.MimeTypeMap;

@Keep
public class ParaEngineHelper {

	private static native void nativeSetContext(final Context pContext, final AssetManager pAssetManager);
	private static native String GetWorldDirectory();
	private static native void OpenFileDialogNativeCallback(String filepath);

	public static native void setKeyState(int keyCode, int keyState) ;

	public static native void onCmdProtocol(String strProtocol);
	private static boolean mCanReadPhoneState = false;
	public static void setCanReadPhoneState(boolean bool){
		mCanReadPhoneState = bool;
	}

	public static void init(final Activity activity, boolean bReadPhoneState) {
		mCanReadPhoneState = bReadPhoneState;
		ParaEngineHelper.nativeSetContext((Context)activity, activity.getAssets());
	}

	private static boolean bHasAgreeUserPrivacy = false;
	public static void onAgreeUserPrivacy(){
		bHasAgreeUserPrivacy = true;
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
		if(!bHasAgreeUserPrivacy){
			return 0;
		}
		WifiManager wifiManager = (WifiManager) ParaEngineActivity.getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
		WifiInfo wifiInfo = wifiManager.getConnectionInfo();
		int ipAddress = wifiInfo.getIpAddress();

		return ipAddress;
	}

	private  static String _getWifiMAC() {
		if(!bHasAgreeUserPrivacy){
			return "";
		}
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
		if(!bHasAgreeUserPrivacy){
			return "";
		}
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
		if(!bHasAgreeUserPrivacy){
			return "";
		}
		String sn =
			Settings.Secure.getString(
				ParaEngineActivity.getContext().getContentResolver(),
				Settings.Secure.ANDROID_ID
			);

		if (sn != null) {
			return sn;
		} else {
			return "";
		}
	}

	public static void OpenFileDialog(String filter) {
		ParaEngineActivity context = ParaEngineActivity.getContext();

		String[] filterArray = filter.split(";");
		Boolean isMatch = true;

		for (String item : filterArray) {
			if (!item.matches("[a-z*]+/[a-z*]")) {
				isMatch = false;
				break;
			}
		}

		if (isMatch) {
			context.OpenFileDialog(filter);
		}
	}

	public static void OpenFileDialogCallback(String filepath) {
//		Log.i("Helper", filepath);
		OpenFileDialogNativeCallback(filepath);
	}



	/**
	 * 根据Uri获取文件绝对路径，解决Android4.4以上版本Uri转换 兼容Android 10
	 *
	 * @param context
	 * @param imageUri
	 */
	public static String getFileAbsolutePath(Context context, Uri imageUri) {
		if (context == null || imageUri == null) {
			return null;
		}

		if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.KITKAT) {
			return getRealFilePath(context, imageUri);
		}

		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT && android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.Q && DocumentsContract.isDocumentUri(context, imageUri)) {
			if (isExternalStorageDocument(imageUri)) {
				String docId = DocumentsContract.getDocumentId(imageUri);
				String[] split = docId.split(":");
				String type = split[0];
				if ("primary".equalsIgnoreCase(type)) {
					return Environment.getExternalStorageDirectory() + "/" + split[1];
				}
			} else if (isDownloadsDocument(imageUri)) {
				String id = DocumentsContract.getDocumentId(imageUri);
				Uri contentUri = ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));
				return getDataColumn(context, contentUri, null, null);
			} else if (isMediaDocument(imageUri)) {
				String docId = DocumentsContract.getDocumentId(imageUri);
				String[] split = docId.split(":");
				String type = split[0];
				Uri contentUri = null;
				if ("image".equals(type)) {
					contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
				} else if ("video".equals(type)) {
					contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
				} else if ("audio".equals(type)) {
					contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
				}
				String selection = MediaStore.Images.Media._ID + "=?";
				String[] selectionArgs = new String[]{split[1]};
				return getDataColumn(context, contentUri, selection, selectionArgs);
			}
		}
		// MediaStore (and general)
		if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q){
			return uriToFileApiQ(context,imageUri);
		}
		else if ("content".equalsIgnoreCase(imageUri.getScheme())) {
			// Return the remote address
			if (isGooglePhotosUri(imageUri)) {
				return imageUri.getLastPathSegment();
			}
			return getDataColumn(context, imageUri, null, null);
		}
		// File
		else if ("file".equalsIgnoreCase(imageUri.getScheme())) {
			return imageUri.getPath();
		}
		return null;
	}

	//此方法 只能用于4.4以下的版本
	private static String getRealFilePath(final Context context, final Uri uri) {
		if (null == uri) {
			return null;
		}
		final String scheme = uri.getScheme();
		String data = null;
		if (scheme == null) {
			data = uri.getPath();
		} else if (ContentResolver.SCHEME_FILE.equals(scheme)) {
			data = uri.getPath();
		} else if (ContentResolver.SCHEME_CONTENT.equals(scheme)) {
			String[] projection = {MediaStore.Images.ImageColumns.DATA};
			Cursor cursor = context.getContentResolver().query(uri, projection, null, null, null);

//            Cursor cursor = context.getContentResolver().query(uri, new String[]{MediaStore.Images.ImageColumns.DATA}, null, null, null);
			if (null != cursor) {
				if (cursor.moveToFirst()) {
					int index = cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA);
					if (index > -1) {
						data = cursor.getString(index);
					}
				}
				cursor.close();
			}
		}
		return data;
	}


	/**
	 * @param uri The Uri to check.
	 * @return Whether the Uri authority is ExternalStorageProvider.
	 */
	private static boolean isExternalStorageDocument(Uri uri) {
		return "com.android.externalstorage.documents".equals(uri.getAuthority());
	}

	/**
	 * @param uri The Uri to check.
	 * @return Whether the Uri authority is DownloadsProvider.
	 */
	private static boolean isDownloadsDocument(Uri uri) {
		return "com.android.providers.downloads.documents".equals(uri.getAuthority());
	}

	private static String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {
		Cursor cursor = null;
		String column = MediaStore.Images.Media.DATA;
		String[] projection = {column};
		try {
			cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs, null);
			if (cursor != null && cursor.moveToFirst()) {
				int index = cursor.getColumnIndexOrThrow(column);
				return cursor.getString(index);
			}
		} finally {
			if (cursor != null) {
				cursor.close();
			}
		}
		return null;
	}

	/**
	 * @param uri The Uri to check.
	 * @return Whether the Uri authority is MediaProvider.
	 */
	private static boolean isMediaDocument(Uri uri) {
		return "com.android.providers.media.documents".equals(uri.getAuthority());
	}

	/**
	 * @param uri The Uri to check.
	 * @return Whether the Uri authority is Google Photos.
	 */
	private static boolean isGooglePhotosUri(Uri uri) {
		return "com.google.android.apps.photos.content".equals(uri.getAuthority());
	}


	/**
	 * Android 10 以上适配 另一种写法
	 * @param context
	 * @param uri
	 * @return
	 */
	private static String getFileFromContentUri(Context context, Uri uri) {
		if (uri == null) {
			return null;
		}
		String filePath;
		String[] filePathColumn = {MediaStore.MediaColumns.DATA, MediaStore.MediaColumns.DISPLAY_NAME};
		ContentResolver contentResolver = context.getContentResolver();
		Cursor cursor = contentResolver.query(uri, filePathColumn, null,
				null, null);
		if (cursor != null) {
			cursor.moveToFirst();
			try {
				filePath = cursor.getString(cursor.getColumnIndex(filePathColumn[0]));
				return filePath;
			} catch (Exception e) {
			} finally {
				cursor.close();
			}
		}
		return "";
	}

	/**
	 * Android 10 以上适配
	 * @param context
	 * @param uri
	 * @return
	 */
//	@RequiresApi(api = Build.VERSION_CODES.Q)
	private static String uriToFileApiQ(Context context, Uri uri) {
		File file = null;
		//android10以上转换
		if (uri.getScheme().equals(ContentResolver.SCHEME_FILE)) {
			file = new File(uri.getPath());
		} else if (uri.getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
			//把文件复制到沙盒目录
			ContentResolver contentResolver = context.getContentResolver();
			Cursor cursor = contentResolver.query(uri, null, null, null, null);
			if (cursor.moveToFirst()) {
				String displayName = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
				try {
					InputStream is = contentResolver.openInputStream(uri);
					File cache = new File(context.getExternalCacheDir().getAbsolutePath(), Math.round((Math.random() + 1) * 1000) + displayName);
					FileOutputStream fos = new FileOutputStream(cache);
					FileUtils.copy(is, fos);
					file = cache;
					fos.close();
					is.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return file.getAbsolutePath();
	}


	/**
	 * 通过文件路径 uri的转字符也可以
	 * @param filePath
	 * @return
	 */
	public static String getMimeType(String filePath) {
		String ext = MimeTypeMap.getFileExtensionFromUrl(filePath);
		return MimeTypeMap.getSingleton().getMimeTypeFromExtension(ext);
	}
}

package com.tatfook.paracraft;

import java.util.Locale;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import android.support.annotation.Keep;

@Keep
public class ParaEngineHelper {

	private static native void nativeSetContext(final Context pContext, final AssetManager pAssetManager);

	public static void init(final Activity activity) {
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
		ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
		ApplicationInfo applicationInfo = context.getApplicationInfo();
		return applicationInfo.packageName;
	}

	public static String getWritablePath() {
		ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
		return context.getFilesDir().getAbsolutePath();
	}

	public static String getExternalStoragePath() {
		ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
		return context.getExternalFilesDir(null).getAbsolutePath();
	}

	public static String getObbPath() {
		ParaEngineActivity context = (ParaEngineActivity)ParaEngineActivity.getContext();
		return context.getObbDir().getAbsolutePath();
	}
}
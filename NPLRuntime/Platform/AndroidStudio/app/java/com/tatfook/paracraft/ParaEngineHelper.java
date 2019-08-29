package com.tatfook.paracraft;

import java.util.Locale;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.support.annotation.Keep;

@Keep
public class ParaEngineHelper {

	public static String getCurrentLanguage() {
		return Locale.getDefault().getLanguage();
	}

	public static boolean CopyTextToClipboard(final String text) {
		AppActivity context = (AppActivity)AppActivity.getContext();
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
		AppActivity context = (AppActivity)AppActivity.getContext();
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
			AppActivity context = (AppActivity)AppActivity.getContext();
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
			AppActivity context = (AppActivity)AppActivity.getContext();
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
}
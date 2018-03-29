package com.tatfook.paracraft;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class ParaEngineNativeView extends View {
	AppActivity mActivity;

	public ParaEngineNativeView(Context context) {
		super(context);
	}
	
	public ParaEngineNativeView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
}
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
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.KeyEvent;
import android.util.Log;

public class ParaEngineNativeView extends SurfaceView {
	AppActivity mActivity;

	public ParaEngineNativeView(Context context) {
		super(context);
	}
	
	public ParaEngineNativeView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public void init(AppActivity activity) {
		mActivity = activity;

		SurfaceHolder holder = getHolder();

		holder.addCallback(new SurfaceHolder.Callback2() {
			@Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
				mActivity.surfaceChanged(holder, format, width, height);
            }

			@Override
            public void surfaceCreated(SurfaceHolder holder) {
				mActivity.surfaceCreated(holder);
			}

			@Override
			public void surfaceRedrawNeeded(SurfaceHolder holder) {
				mActivity.surfaceRedrawNeeded(holder);
			}

			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				mActivity.surfaceDestroyed(holder);
			}
		});
	}

	@Override    
    public boolean onKeyUp(int keyCode, KeyEvent event) {    
		boolean bResult = super.onKeyUp(keyCode, event);
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {    
			Log.i("ParaEngine", "ParaEngineNativeView::onKeyDown");
            return false; 
		}
		else if (keyCode == KeyEvent.KEYCODE_MENU) {
			return false;
		}
        else {    
            return bResult;
        }    
	}
}
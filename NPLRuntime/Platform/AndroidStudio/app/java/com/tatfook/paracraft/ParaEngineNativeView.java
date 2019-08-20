package com.tatfook.paracraft;


import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class ParaEngineNativeView extends SurfaceView {
	private static final String TAG = "ParaEngine";

	private final static int HANDLER_OPEN_IME_KEYBOARD = 2;
	private final static int HANDLER_CLOSE_IME_KEYBOARD = 3;

	private static AppActivity sActivity;
	private ParaEngineEditBox mEditText = null;
	private static ParaTextInputWrapper sParaTextInputWrapper = null;

	/*

	private static native void nativeDeleteBackward();
	private static native void nativeInsertText(String text);
	*/
	private static native void nativeOnUnicodeChar(String text);

	// TODO Static handler -> Potential leak!
	private static Handler sHandler = null;

	public ParaEngineNativeView(Context context) {
		super(context);

		this.initView();
	}
	
	public ParaEngineNativeView(Context context, AttributeSet attrs) {
		super(context, attrs);

		this.initView();
	}

	private boolean mSoftKeyboardShown = false;

	public boolean isSoftKeyboardShown() {
		return mSoftKeyboardShown;
	}

	public void setSoftKeyboardShown(boolean softKeyboardShown) {
		this.mSoftKeyboardShown = softKeyboardShown;
	}

	public void init(AppActivity activity) {
		sActivity = activity;

		SurfaceHolder holder = getHolder();

		holder.addCallback(new SurfaceHolder.Callback2() {
			@Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
				sActivity.surfaceChanged(holder, format, width, height);
            }

			@Override
            public void surfaceCreated(SurfaceHolder holder) {
				sActivity.surfaceCreated(holder);
			}

			@Override
			public void surfaceRedrawNeeded(SurfaceHolder holder) {
				sActivity.surfaceRedrawNeeded(holder);
			}

			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				sActivity.surfaceDestroyed(holder);
			}
		});
	}

	protected void initView() {
		this.setFocusableInTouchMode(true);
		sParaTextInputWrapper = new ParaTextInputWrapper(this);

		sHandler = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				switch(msg.what) {
					case HANDLER_OPEN_IME_KEYBOARD:
						if (mEditText != null) {
							mEditText.setEnabled(true);
							if (mEditText.requestFocus()) {
								mEditText.removeTextChangedListener(sParaTextInputWrapper);
								mEditText.setText("");
								mEditText.addTextChangedListener(sParaTextInputWrapper);
								InputMethodManager imm = (InputMethodManager)sActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
								imm.showSoftInput(mEditText, 0);
								Log.d(TAG, "ShowSoftInput");
							}
							else {
								mEditText.setEnabled(false);
							}
							}
						break;
					case HANDLER_CLOSE_IME_KEYBOARD:
						if (mEditText != null) {
							mEditText.removeTextChangedListener(sParaTextInputWrapper);
							InputMethodManager imm = (InputMethodManager)sActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
							imm.hideSoftInputFromWindow(mEditText.getWindowToken(), 0);
							ParaEngineNativeView.this.requestFocus();
							mEditText.setEnabled(false);
							Log.d(TAG, "HideSoftInput");
						}
						break;
				}
			}
		};
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

	public ParaEngineEditBox getParaEditText() {
		return mEditText;
	}

	public void setParaEditText(ParaEngineEditBox editText) {
		mEditText = editText;
		if (mEditText != null && sParaTextInputWrapper != null) {
			mEditText.setOnEditorActionListener(sParaTextInputWrapper);
			this.requestFocus();
		}
	}

	public void onUnicodeText(final String text) {
		sActivity.runOnGLThread(new Runnable() {
			@Override
			public void run() {
				nativeOnUnicodeChar(text);
			}
		});
	}


	public static void openIMEKeyboard() {
		Message msg = new Message();
		msg.what = HANDLER_OPEN_IME_KEYBOARD;
		sHandler.sendMessage(msg);
	}

	public static void closeIMEKeyboard() {
		Message msg = new Message();
		msg.what = HANDLER_CLOSE_IME_KEYBOARD;
		sHandler.sendMessage(msg);
	}
}
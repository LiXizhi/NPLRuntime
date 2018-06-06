package com.tatfook.paracraft;

import android.view.KeyEvent;  
import android.content.pm.PackageManager;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.view.ViewGroup;
import android.view.View;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Bundle;
import android.os.Looper;
import android.os.MessageQueue;
import android.util.AttributeSet;
import android.view.InputQueue;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import dalvik.system.BaseDexClassLoader;
import android.app.Activity;
import android.view.ViewGroup;
import java.io.File;
import android.preference.PreferenceManager.OnActivityResultListener;
import android.util.Log;
import android.content.Intent;
import java.lang.reflect.Method;
import java.util.ArrayList;

/**
 * Convenience for implementing an activity that will be implemented
 * purely in native code.  That is, a game (or game-like thing).  There
 * is no need to derive from this class; you can simply declare it in your
 * manifest, and use the NDK APIs from there.
 *
 * <p>A <a href="https://github.com/googlesamples/android-ndk/tree/master/native-activity">sample
 * native activity</a> is available in the NDK samples.
 */
public class AppActivity extends Activity implements InputQueue.Callback, OnGlobalLayoutListener {

	private static AppActivity sContext = null;
	private static final String TAG = "ParaEngine";

	/**
     * Optional meta-that can be in the manifest for this component, specifying
     * the name of the native shared library to load.  If not specified,
     * "main" is used.
     */
    public static final String META_DATA_LIB_NAME = "android.app.lib_name";
	private static final String KEY_NATIVE_SAVED_STATE = "android:native_state";

	private ParaEngineNativeView mNativeContentView;
	private InputMethodManager mIMM;

	private long mNativeHandle;

	private InputQueue mCurInputQueue;
	private long mCurInputQueuePtr;
    private SurfaceHolder mCurSurfaceHolder;

	final int[] mLocation = new int[2];

	int mLastContentX;
    int mLastContentY;
    int mLastContentWidth;
    int mLastContentHeight;

	protected ResizeLayout mFrameLayout = null;

	private ParaEngineWebViewHelper mWebViewHelper = null;

	private boolean mDispatchingUnhandledKey;

	private boolean mDestroyed = false;

	private native long nativeInit(Looper looper, String internalDataPath, String obbPath, String externalDataPath, int sdkVersion, AssetManager assetMgr, byte[] savedState);
	private native void unloadNativeCode(long handle);
	private native void onPauseNative(long handle);
	private native void onResumeNative(long handle);
	//private native void onNewIntentNative(long handle);
	private native void onStartNative(long handle);
	private native void onStopNative(long handle);
	private native void onConfigurationChangedNative(long handle);
	private native void onLowMemoryNative(long handle);
	private native void onWindowFocusChangedNative(long handle, boolean hasFocus);
	private native void onSurfaceCreatedNative(long handle, Surface surface);	
	private native void onSurfaceChangedNative(long handle, Surface surface, int format, int width, int height);
	private native void onSurfaceRedrawNeededNative(long handle, Surface surface);
	private native void onSurfaceDestroyedNative(long handle);
	
	

	private native byte[] onSaveInstanceStateNative(long handle);

	private native void onInputQueueCreatedNative(long handle, long queuePtr);
	private native void onInputQueueDestroyedNative(long handle, long queuePtr);

	private native void onContentRectChangedNative(long handle, int x, int y, int w, int h);

	private ArrayList<Runnable> mEventQueue = new ArrayList<Runnable>();


	@Override    
    public boolean onKeyUp(int keyCode, KeyEvent event) {    
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {   
            return false; 
		}
        else {    
            return super.onKeyUp(keyCode, event);    
        }    
	}

	protected void _init(Bundle savedInstanceState) {
		String libname = "main";
        String funcname = "ANativeActivity_onCreate";
        ActivityInfo ai;

		mIMM = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);

		//getWindow().takeSurface(this);
        getWindow().takeInputQueue(this);
        getWindow().setFormat(PixelFormat.RGB_565);
        getWindow().setSoftInputMode(
                WindowManager.LayoutParams.SOFT_INPUT_STATE_UNSPECIFIED
                | WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mNativeContentView = new ParaEngineNativeView(this);
       // mNativeContentView.mActivity = this;
	   mNativeContentView.init(this);
        //setContentView(mNativeContentView);

		this.initLayout();

        mNativeContentView.requestFocus();
        mNativeContentView.getViewTreeObserver().addOnGlobalLayoutListener(this);

		try {
			ai = getPackageManager().getActivityInfo(
                    getIntent().getComponent(), PackageManager.GET_META_DATA);
            if (ai.metaData != null) {
                String ln = ai.metaData.getString(META_DATA_LIB_NAME);
                if (ln != null) libname = ln;
			}
		}catch (PackageManager.NameNotFoundException e) {
            throw new RuntimeException("Error getting activity info", e);
        }

		byte[] nativeSavedState = savedInstanceState != null
                ? savedInstanceState.getByteArray(KEY_NATIVE_SAVED_STATE) : null;

		System.loadLibrary(libname);

		mNativeHandle = nativeInit(Looper.myLooper()
			, getAbsolutePath(getFilesDir())
			, getAbsolutePath(getObbDir())
			, getAbsolutePath(getExternalFilesDir(null))
			, Build.VERSION.SDK_INT
			, getAssets()
			, nativeSavedState);
		
		if (mNativeHandle == 0) {
            throw new UnsatisfiedLinkError("Unable to init native handle");
        }

		if (mWebViewHelper == null)
			mWebViewHelper = new ParaEngineWebViewHelper(mFrameLayout);
	}

	@Override
    protected void onCreate(Bundle savedInstanceState) {
		
		sContext = this;

		super.onCreate(savedInstanceState);

		final Bundle si = savedInstanceState;

		 // init plugin
        if(!ParaEnginePluginWrapper.init(this, 
                savedInstanceState ,
                new ParaEnginePluginWrapper.PluginWrapperListener() {
                    @Override
                    public void onInit() {
                        AppActivity.this._init(si);
                    }
                })) {
            
            this._init(si);
        }
	}

	private static String getAbsolutePath(File file) {
        return (file != null) ? file.getAbsolutePath() : null;
    }

	public void runOnGLThread(final Runnable pRunnable) {

		synchronized(this)
		{
			mEventQueue.add(pRunnable);
		}

    }

	public void processGLEvent() {
		synchronized(this)
		{
			while(!mEventQueue.isEmpty()) {
				Runnable event = mEventQueue.remove(0);
				if (event != null)
					event.run();
			}
		}
	}

	private boolean m_bFirstGet = true;

	public String getLauncherIntentData() {
		if (m_bFirstGet) { 
			Intent intent = getIntent();
			if (intent != null) {
				String action = intent.getAction();
				if (Intent.ACTION_VIEW == action) {
					m_bFirstGet = false;
					return intent.getDataString();
				}
			}
		}
		return "";
	}

	public String getFileDirsPath() {
		return getFilesDir().getAbsolutePath();
	}

	public void initLayout() {
		// FrameLayout
        ViewGroup.LayoutParams framelayout_params =
            new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                                       ViewGroup.LayoutParams.MATCH_PARENT);

		mFrameLayout = new ResizeLayout(this);
		mFrameLayout.setLayoutParams(framelayout_params);

		// ...add to FrameLayout
        mFrameLayout.addView(mNativeContentView);

		// Set framelayout as the content view
        setContentView(mFrameLayout);
	}

	@Override
    protected void onDestroy() {
        mDestroyed = true;
        if (mCurSurfaceHolder != null) {
            onSurfaceDestroyedNative(mNativeHandle);
            mCurSurfaceHolder = null;
        }
        if (mCurInputQueue != null) {
		    onInputQueueDestroyedNative(mNativeHandle, mCurInputQueuePtr);
			mCurInputQueuePtr = 0;
			mCurInputQueue = null;
        }
        unloadNativeCode(mNativeHandle);
        super.onDestroy();
		ParaEnginePluginWrapper.onDestroy();
    }

	public static Context getContext()
	{
		return sContext;
	}

	

	@Override
    protected void onPause() {
        super.onPause();
		ParaEnginePluginWrapper.onPause();
        onPauseNative(mNativeHandle);
    }

	@Override
    protected void onResume() {
        super.onResume();
		ParaEnginePluginWrapper.onResume();
        onResumeNative(mNativeHandle);
    }

	 @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		ParaEnginePluginWrapper.onActivityResult(requestCode, resultCode, data);
	}

	@Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

		ParaEnginePluginWrapper.onSaveInstanceState(outState);
        byte[] state = onSaveInstanceStateNative(mNativeHandle);
        if (state != null) {
            outState.putByteArray(KEY_NATIVE_SAVED_STATE, state);
        }
    }

	@Override
    protected void onStart() {
        super.onStart();
		ParaEnginePluginWrapper.onStart();
        onStartNative(mNativeHandle);
    }

	@Override
    protected void onStop() {
        super.onStop();
		ParaEnginePluginWrapper.onStop();
        onStopNative(mNativeHandle);
    }

	@Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (!mDestroyed) {
            onConfigurationChangedNative(mNativeHandle);
        }
    }

	@Override
    public void onLowMemory() {
        super.onLowMemory();
        if (!mDestroyed) {
            onLowMemoryNative(mNativeHandle);
        }
    }

	@Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (!mDestroyed) {
            onWindowFocusChangedNative(mNativeHandle, hasFocus);
        }
    }

	
	public void surfaceCreated(SurfaceHolder holder) {
        if (!mDestroyed) {
            mCurSurfaceHolder = holder;
            onSurfaceCreatedNative(mNativeHandle, holder.getSurface());
        }
    }

	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (!mDestroyed) {
            mCurSurfaceHolder = holder;
            onSurfaceChangedNative(mNativeHandle, holder.getSurface(), format, width, height);
        }
    }

	public void surfaceRedrawNeeded(SurfaceHolder holder) {
        if (!mDestroyed) {
            mCurSurfaceHolder = holder;
            onSurfaceRedrawNeededNative(mNativeHandle, holder.getSurface());
        }
    }

	public void surfaceDestroyed(SurfaceHolder holder) {
        mCurSurfaceHolder = null;
        if (!mDestroyed) {
            onSurfaceDestroyedNative(mNativeHandle);
        }
    }
	

	private static long getQueueNativePtr(InputQueue queue) {
		
		try
		{
			Class<InputQueue> c = InputQueue.class;
			Method m = c.getDeclaredMethod("getNativePtr");

			Object retValue = m.invoke(queue);

			if (retValue instanceof Integer)
				return ((Integer)retValue).intValue() & 0x0FFFFFFFFL;
			if (retValue instanceof Number)
				return ((Number)retValue).longValue();

			throw new UnsatisfiedLinkError("Unknow type of return value to call InputQueue.getNativePtr");

		} catch (Exception e){
			Log.e("ParaEngine", e.toString());

			throw new UnsatisfiedLinkError("Unable to call InputQueue.getNativePtr");
		}
		
	}

	public void onInputQueueCreated(InputQueue queue) {
        if (!mDestroyed) {
            mCurInputQueue = queue;
			mCurInputQueuePtr = getQueueNativePtr(queue);
            onInputQueueCreatedNative(mNativeHandle, mCurInputQueuePtr);
        }
    }

	public void onInputQueueDestroyed(InputQueue queue) {
        if (!mDestroyed) {
            onInputQueueDestroyedNative(mNativeHandle, mCurInputQueuePtr);
			mCurInputQueuePtr = 0;
            mCurInputQueue = null;
        }
    }

	public void onGlobalLayout() {
        mNativeContentView.getLocationInWindow(mLocation);
        int w = mNativeContentView.getWidth();
        int h = mNativeContentView.getHeight();
        if (mLocation[0] != mLastContentX || mLocation[1] != mLastContentY
                || w != mLastContentWidth || h != mLastContentHeight) {
            mLastContentX = mLocation[0];
            mLastContentY = mLocation[1];
            mLastContentWidth = w;
            mLastContentHeight = h;
            if (!mDestroyed) {
                onContentRectChangedNative(mNativeHandle, mLastContentX,
                        mLastContentY, mLastContentWidth, mLastContentHeight);
            }
        }
    }
}
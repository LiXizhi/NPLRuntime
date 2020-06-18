package com.tatfook.paracraft;

import android.Manifest;
import android.app.KeyguardManager;
import android.os.Build;
import android.os.PowerManager;
import android.support.annotation.Keep;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public class ParaEngineActivity extends AppCompatActivity {

    // ===========================================================
    // Constants
    // ===========================================================

    private static ParaEngineActivity sContext = null;
    private static final String TAG = "ParaEngine";

    private final static int PERMISSION_REQUEST_PHONE_STATE = 100;

    //native method,call GLViewImpl::getGLContextAttrs() to get the OpenGL ES context attributions
    private static native int[] getGLContextAttrs();
    private native void onKeyBack(boolean bDown);

    /**
     * Optional meta-that can be in the manifest for this component, specifying
     * the name of the native shared library to load.  If not specified,
     * "main" is used.
     */
    public static final String META_DATA_LIB_NAME = "android.app.lib_name";
    private static final String KEY_NATIVE_SAVED_STATE = "android:native_state";

    // ===========================================================
    // Fields
    // ===========================================================

    private ParaEngineGLSurfaceView mGLSurfaceView = null;
    private int[] mGLContextAttrs = null;
    private ParaEngineWebViewHelper mWebViewHelper = null;
    protected ResizeLayout mFrameLayout = null;

    public ParaEngineGLSurfaceView getGLSurfaceView() { return mGLSurfaceView; }
    public static Context getContext() { return sContext; }

    protected void onLoadNativeLibraries() {
        try {
            String libname = "main";

            try {
                ActivityInfo ai;
                ai = getPackageManager().getActivityInfo(
                        getIntent().getComponent(), PackageManager.GET_META_DATA);
                if (ai.metaData != null) {
                    String ln = ai.metaData.getString(META_DATA_LIB_NAME);
                    if (ln != null) libname = ln;
                }
            }catch (PackageManager.NameNotFoundException e) {
                throw new RuntimeException("Error getting activity info", e);
            }

            System.loadLibrary(libname);
        }catch (Exception e) {
            e.printStackTrace();
        }
    }

    protected void initLayout() {
        // FrameLayout
        ViewGroup.LayoutParams framelayout_params =
                new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT);

        mFrameLayout = new ResizeLayout(this);
        mFrameLayout.setLayoutParams(framelayout_params);

        // ParaEngineEditBox layout
        ViewGroup.LayoutParams edittext_layout_params =
                new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);
        ParaEngineEditBox edittext = new ParaEngineEditBox(this);
        edittext.setLayoutParams(edittext_layout_params);
        edittext.setMultilineEnabled(false);
        edittext.setReturnType(0);
        edittext.setInputMode(6);
        edittext.setEnabled(false);

        mFrameLayout.addView(edittext);

        // Cocos2dxGLSurfaceView
        this.mGLSurfaceView = this.onCreateView();
        // ...add to FrameLayout
        mFrameLayout.addView(this.mGLSurfaceView);

        this.mGLSurfaceView.setParaEngineRenderer(new ParaEngineRenderer());
        this.mGLSurfaceView.setParaEditText(edittext);

        // Set framelayout as the content view
        setContentView(mFrameLayout);
    }

    protected void _init(Bundle savedInstanceState, boolean bGranted) {
        onLoadNativeLibraries();

        ParaEngineHelper.init(this, bGranted);

        this.mGLContextAttrs = getGLContextAttrs();

        this.initLayout();

        if (mWebViewHelper == null)
            mWebViewHelper = new ParaEngineWebViewHelper(mFrameLayout);
    }

    protected ParaEngineGLSurfaceView onCreateView() {
        ParaEngineGLSurfaceView view = new ParaEngineGLSurfaceView(this);

        view.initView(this);

        ParaEngineEGLConfigChooser chooser = new ParaEngineEGLConfigChooser(this.mGLContextAttrs);
        view.setEGLConfigChooser(chooser);

        return view;
    }

    protected  void onCheckPerissionFinish(final Bundle savedInstanceState, boolean bGranted)
    {
        final Bundle si = savedInstanceState;
        final boolean _bGranted = bGranted;
        // init plugin
        if(!ParaEnginePluginWrapper.init(this,
                savedInstanceState ,
                new ParaEnginePluginWrapper.PluginWrapperListener() {
                    @Override
                    public void onInit() {
                        ParaEngineActivity.this._init(si, _bGranted);
                    }
                })) {

            this._init(si, _bGranted);
        }
    }

    private Bundle mSavedInstanceState;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {

        sContext = this;

        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_PHONE_STATE}, PERMISSION_REQUEST_PHONE_STATE);

            mSavedInstanceState = savedInstanceState;

        } else {
            onCheckPerissionFinish(savedInstanceState, true);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if (requestCode == PERMISSION_REQUEST_PHONE_STATE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                onCheckPerissionFinish(mSavedInstanceState, true);
            } else {
                onCheckPerissionFinish(mSavedInstanceState, false);
            }
        }
        else
        {
            ParaEnginePluginWrapper.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    public void runOnGLThread(final Runnable pRunnable) {
        this.mGLSurfaceView.queueEvent(pRunnable);
    }

    private boolean m_bFirstGet = true;

    @Keep
    public static String getLauncherIntentData() {
        if (sContext == null)
            return "";

        if (sContext.m_bFirstGet) {
            Intent intent = sContext.getIntent();
            if (intent != null) {
                String action = intent.getAction();
                if (Intent.ACTION_VIEW == action) {
                    sContext.m_bFirstGet = false;
                    return intent.getDataString();
                }
            }
        }
        return "";
    }

    @Keep
    public String getFileDirsPath() {
        return getFilesDir().getAbsolutePath();
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {

            this.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    onKeyBack(false);
                }
            });


            return false;
        }
        else {
            return super.onKeyUp(keyCode, event);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {
            this.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    onKeyBack(true);
                }
            });

            return false;
        }
        else {
            return super.onKeyDown(keyCode, event);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        ParaEnginePluginWrapper.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        ParaEnginePluginWrapper.onPause();

        if (mGLSurfaceView != null)
            mGLSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        ParaEnginePluginWrapper.onResume();
        resumeIfHasFocus();
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
    }

    @Override
    protected void onStart() {
        super.onStart();
        ParaEnginePluginWrapper.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
        ParaEnginePluginWrapper.onStop();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        Log.d(TAG, "onWindowFocusChanged() hasFocus=" + hasFocus);
        super.onWindowFocusChanged(hasFocus);

        this.hasFocus = hasFocus;
        resumeIfHasFocus();
    }

    private boolean hasFocus = false;

    private void resumeIfHasFocus() {
        //It is possible for the app to receive the onWindowsFocusChanged(true) event
        //even though it is locked or asleep
        boolean readyToPlay = !isDeviceLocked() && !isDeviceAsleep();

        if(hasFocus && readyToPlay && mGLSurfaceView != null) {
            mGLSurfaceView.onResume();
        }
    }

    private static boolean isDeviceLocked() {
        KeyguardManager keyguardManager = (KeyguardManager)getContext().getSystemService(Context.KEYGUARD_SERVICE);
        boolean locked = keyguardManager.inKeyguardRestrictedInputMode();
        return locked;
    }

    private static boolean isDeviceAsleep() {
        PowerManager powerManager = (PowerManager)getContext().getSystemService(Context.POWER_SERVICE);
        if(powerManager == null) {
            return false;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT_WATCH) {
            return !powerManager.isInteractive();
        } else {
            return !powerManager.isScreenOn();
        }
    }

    private class ParaEngineEGLConfigChooser implements GLSurfaceView.EGLConfigChooser
    {
        private int[] mConfigAttributes;
        private  final int EGL_OPENGL_ES2_BIT = 0x04;
        private  final int EGL_OPENGL_ES3_BIT = 0x40;

        public ParaEngineEGLConfigChooser(int redSize, int greenSize, int blueSize, int alphaSize, int depthSize, int stencilSize, int multisamplingCount)
        {
            mConfigAttributes = new int[] {redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize, multisamplingCount};
        }

        public ParaEngineEGLConfigChooser(int[] attributes)
        {
            mConfigAttributes = attributes;
        }

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            int[][] EGLAttributes = {
                    {
                            // GL ES 2 with user set
                            EGL10.EGL_RED_SIZE, mConfigAttributes[0],
                            EGL10.EGL_GREEN_SIZE, mConfigAttributes[1],
                            EGL10.EGL_BLUE_SIZE, mConfigAttributes[2],
                            EGL10.EGL_ALPHA_SIZE, mConfigAttributes[3],
                            EGL10.EGL_DEPTH_SIZE, mConfigAttributes[4],
                            EGL10.EGL_STENCIL_SIZE, mConfigAttributes[5],
                            EGL10.EGL_SAMPLE_BUFFERS, (mConfigAttributes[6] > 0) ? 1 : 0,
                            EGL10.EGL_SAMPLES, mConfigAttributes[6],
                            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL10.EGL_NONE
                    },
                    {
                            // GL ES 2 with user set 16 bit depth buffer
                            EGL10.EGL_RED_SIZE, mConfigAttributes[0],
                            EGL10.EGL_GREEN_SIZE, mConfigAttributes[1],
                            EGL10.EGL_BLUE_SIZE, mConfigAttributes[2],
                            EGL10.EGL_ALPHA_SIZE, mConfigAttributes[3],
                            EGL10.EGL_DEPTH_SIZE, mConfigAttributes[4] >= 24 ? 16 : mConfigAttributes[4],
                            EGL10.EGL_STENCIL_SIZE, mConfigAttributes[5],
                            EGL10.EGL_SAMPLE_BUFFERS, (mConfigAttributes[6] > 0) ? 1 : 0,
                            EGL10.EGL_SAMPLES, mConfigAttributes[6],
                            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL10.EGL_NONE
                    },
                    {
                            // GL ES 2 with user set 16 bit depth buffer without multisampling
                            EGL10.EGL_RED_SIZE, mConfigAttributes[0],
                            EGL10.EGL_GREEN_SIZE, mConfigAttributes[1],
                            EGL10.EGL_BLUE_SIZE, mConfigAttributes[2],
                            EGL10.EGL_ALPHA_SIZE, mConfigAttributes[3],
                            EGL10.EGL_DEPTH_SIZE, mConfigAttributes[4] >= 24 ? 16 : mConfigAttributes[4],
                            EGL10.EGL_STENCIL_SIZE, mConfigAttributes[5],
                            EGL10.EGL_SAMPLE_BUFFERS, 0,
                            EGL10.EGL_SAMPLES, 0,
                            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL10.EGL_NONE
                    },
                    {
                            // GL ES 2 by default
                            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL10.EGL_NONE
                    }
            };

            EGLConfig result = null;
            for (int[] eglAtribute : EGLAttributes) {
                result = this.doChooseConfig(egl, display, eglAtribute);
                if (result != null)
                    return result;
            }

            Log.e(DEVICE_POLICY_SERVICE, "Can not select an EGLConfig for rendering.");
            return null;
        }

        private EGLConfig doChooseConfig(EGL10 egl, EGLDisplay display, int[] attributes) {
            EGLConfig[] configs = new EGLConfig[1];
            int[] matchedConfigNum = new int[1];
            boolean result = egl.eglChooseConfig(display, attributes, configs, 1, matchedConfigNum);
            if (result && matchedConfigNum[0] > 0) {
                return configs[0];
            }
            return null;
        }
    }
}

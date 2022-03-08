//-----------------------------------------------------------------------------
// ParaEngineActivity.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.Manifest;
import android.app.KeyguardManager;
import android.content.res.Configuration;
import android.os.Build;
import android.os.CountDownTimer;
import android.os.PowerManager;
import android.support.annotation.Keep;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageView;

import com.smarx.notchlib.NotchScreenManager;
import com.tatfook.paracraft.screenrecorder.ScreenRecorder;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public class ParaEngineActivity extends AppCompatActivity {
    private static ParaEngineActivity sContext = null;
    private static ParaEngineRenderer sRenderer = null;
    private static boolean sBGranted = false;
    private static final int PERMISSION_REQUEST_PHONE_STATE = 100;
    // native method,call GLViewImpl::getGLContextAttrs() to get the OpenGL ES context attributions
    private static native int[] getGLContextAttrs();
    // Optional meta-that can be in the manifest for this component,
    // specifying the name of the native shared library to load. If not specified, "main" is used.
    public static final String META_DATA_LIB_NAME = "android.app.lib_name";

    protected ResizeLayout mFrameLayout = null ;
    private ParaEngineGLSurfaceView mGLSurfaceView = null;
    private ParaEngineWebViewHelper mWebViewHelper = null;
    private int[] mGLContextAttrs = null;
    private boolean mUsbMode = false;
    private boolean hasFocus = false;
    private Bundle mSavedInstanceState;

    public static ParaEngineActivity getContext() {
        return sContext;
    }

    @Keep
    public static String getLauncherIntentData() {
        if (sContext == null)
            return "";

        Intent intent = sContext.getIntent();

        if (intent != null) {
            return intent.getDataString();
        } else {
            return "";
        }
    }

    @Keep
    public static void setScreenOrientation(int type){
        if (type == 1) {
            sContext.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else if (type == 0) {
            sContext.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }
    }

    private static boolean isDeviceLocked() {
        Context context = getContext();

        if (context != null) {
            KeyguardManager keyguardManager = (KeyguardManager)context.getSystemService(Context.KEYGUARD_SERVICE);
            boolean locked = keyguardManager.inKeyguardRestrictedInputMode();
            return locked;
        } else {
            return false;
        }
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

    @Keep
    public static boolean getUsbMode() {
        return sContext.mUsbMode;
    }

    @Keep
    public static void onExit(){
        sContext.finish();
        System.exit(0);
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        sContext = this;

        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (checkSelfPermission(Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED)
        {
            requestPermissions(
                new String[]{
                    Manifest.permission.READ_PHONE_STATE,
                    Manifest.permission.RECORD_AUDIO
                },
                PERMISSION_REQUEST_PHONE_STATE
            );

            mSavedInstanceState = savedInstanceState;
        } else {
            onCheckPermissionFinish(savedInstanceState, true);
        }

        NotchScreenManager.getInstance().setDisplayInNotch(this);

        // Determine whether the device uses USB
        Configuration configuration = getResources().getConfiguration();
        Boolean mouseExist = false;

        final int[] devices = InputDevice.getDeviceIds();

        for (int i = 0; i < devices.length; i++) {
            InputDevice device = InputDevice.getDevice(devices[i]);

            if (device != null && device.getName().contains("Mouse")) {
                mouseExist = true;
            }
        }

        if (configuration.keyboard != Configuration.KEYBOARD_NOKEYS || mouseExist) {
            mUsbMode = true;
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
        ScreenRecorder.onActivityResult(requestCode, resultCode, data);
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

    protected void onLoadNativeLibraries() {
        try {
            String libname = "main";

            try {
                ActivityInfo ai;
                ai = getPackageManager().getActivityInfo(getIntent().getComponent(),
                                                         PackageManager.GET_META_DATA);
                if (ai.metaData != null) {
                    String ln = ai.metaData.getString(META_DATA_LIB_NAME);
                    if (ln != null)
                        libname = ln;
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
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            );

        mFrameLayout = new ResizeLayout(this);
        mFrameLayout.setLayoutParams(framelayout_params);

        // ParaEngineEditBox layout
        ViewGroup.LayoutParams edittext_layout_params =
                new ViewGroup.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT
                );

        ParaEngineEditBox edittext = new ParaEngineEditBox(this);
        edittext.setLayoutParams(edittext_layout_params);
        edittext.setMultilineEnabled(false);
        edittext.setReturnType(0);
        edittext.setInputMode(6);
        edittext.setEnabled(false);

        mFrameLayout.addView(edittext);

        // GLSurfaceView
        mGLSurfaceView = this.onCreateView();
        mFrameLayout.addView(mGLSurfaceView);

        if (sRenderer == null) {
            sRenderer = new ParaEngineRenderer();
            mGLSurfaceView.setParaEngineRenderer(sRenderer);
        } else {
            sRenderer.termWindow();
            mGLSurfaceView.setParaEngineRenderer(sRenderer);
        }

        mGLSurfaceView.setParaEditText(edittext);

        final ImageView imageView = new ImageView(this);
        imageView.setLayoutParams(
            new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        );
        imageView.setScaleType(ImageView.ScaleType.FIT_XY);
        imageView.setImageResource(R.drawable.splash);
        mFrameLayout.addView(imageView);

        CountDownTimer countDownTimer = new CountDownTimer(3000, 1000) {
            @Override
            public void onTick(long millisUntilFinished) {}

            @Override
            public void onFinish() {
                mFrameLayout.removeView(imageView);
            }
        };

        countDownTimer.start();

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
        final ParaEngineGLSurfaceView view = new ParaEngineGLSurfaceView(this);

        view.initView(this);

        ParaEngineEGLConfigChooser chooser = new ParaEngineEGLConfigChooser(this.mGLContextAttrs);
        view.setEGLConfigChooser(chooser);

        view.setOnGenericMotionListener(new View.OnGenericMotionListener() {
            @Override
            public boolean onGenericMotion(View v, MotionEvent event) {
                view.setMousePosition(event);

                return false;
            }
        });

        return view;
    }

    protected void onCheckPermissionFinish(final Bundle savedInstanceState, boolean bGranted)
    {
        final boolean _bGranted = bGranted;
        this._init(savedInstanceState, _bGranted);

        // init plugin
        if (!ParaEnginePluginWrapper.init(
              this,
               savedInstanceState,
               new ParaEnginePluginWrapper.PluginWrapperListener() {
                   @Override
                   public void onInit() {
                       ParaEngineActivity.this._init(savedInstanceState, _bGranted);
                   }
               }
          ))
        {
           this._init(savedInstanceState, _bGranted);
        }
    }

    private void resumeIfHasFocus() {
        // It is possible for the app to receive the onWindowsFocusChanged(true) event
        // even though it is locked or asleep
        boolean readyToPlay = !isDeviceLocked() && !isDeviceAsleep();

        if(hasFocus && readyToPlay && mGLSurfaceView != null) {
            mGLSurfaceView.onResume();
        }
    }

    private class ParaEngineEGLConfigChooser implements GLSurfaceView.EGLConfigChooser
    {
        private int[] mConfigAttributes;
        private final int EGL_OPENGL_ES2_BIT = 0x04;
        private final int EGL_OPENGL_ES3_BIT = 0x40;

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

    public ParaEngineGLSurfaceView getGLSurfaceView() {
        return mGLSurfaceView;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if (requestCode == PERMISSION_REQUEST_PHONE_STATE) {
            Log.d("111111", "2222222");
            if (sContext == null) {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    sBGranted = true;
                    onCheckPermissionFinish(mSavedInstanceState, true);
                } else {
                    sBGranted = false;
                    onCheckPermissionFinish(mSavedInstanceState, false);
                }
            } else {
                onCheckPermissionFinish(mSavedInstanceState, sBGranted);
            }
        } else if (requestCode == ScreenRecorder.REQUEST_PERMISSIONS) {
            int granted = PackageManager.PERMISSION_GRANTED;

            for (int r : grantResults) {
                granted |= r;
            }

            Log.d("55555", String.valueOf(granted));
        } else {
            // TODO: Fix crash.
            // ParaEnginePluginWrapper.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    public void runOnGLThread(final Runnable pRunnable) {
        mGLSurfaceView.queueEvent(pRunnable);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        this.hasFocus = hasFocus;
        resumeIfHasFocus();
    }

    @Keep
    public String getFileDirsPath() {
        return getFilesDir().getAbsolutePath();
    }
}

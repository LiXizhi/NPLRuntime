//-----------------------------------------------------------------------------
// ParaEngineActivity.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2025.8.12
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import static android.Manifest.permission.READ_PHONE_STATE;

import android.Manifest;
import android.app.Activity;
import android.app.KeyguardManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.res.Configuration;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.os.CountDownTimer;
import android.os.Environment;
import android.os.Handler;
import android.os.PowerManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.TypedArray;
import android.opengl.GLSurfaceView;
import android.os.Bundle;

import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContract;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import android.provider.MediaStore;
import android.util.Base64;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ProgressBar;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebSettings;

import com.smarx.notchlib.NotchScreenManager;
import com.tatfook.paracraft.screenrecorder.ScreenRecorder;
import com.tatfook.paracraft.utils.USBSerialTransferUtil;
import com.tatfook.plugin.bluetooth.InterfaceBluetooth;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public class ParaEngineActivity extends AppCompatActivity {
    private static ParaEngineActivity sContext = null;
    private static ParaEngineRenderer sRenderer = null;
    private static boolean sBGranted = false;
    // native method,call GLViewImpl::getGLContextAttrs() to get the OpenGL ES context attributions
    private static native int[] getGLContextAttrs();
    // Optional meta-that can be in the manifest for this component,
    // specifying the name of the native shared library to load. If not specified, "main" is used.
    public static final String META_DATA_LIB_NAME = "android.app.lib_name";

    private ResizeLayout mFrameLayout = null ;
    private ParaEngineGLSurfaceView mGLSurfaceView = null;
    private ParaEngineWebViewHelper mWebViewHelper = null;
    private int[] mGLContextAttrs = null;
    private boolean mUsbMode = false;
    private boolean hasFocus = false;
    private Bundle mSavedInstanceState;
    private ActivityResultLauncher<String> mOpenFileDialogLuancher;
    private WebView mLoadingWebView = null;
    private Handler mLoadingHandler = new Handler();

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
            return keyguardManager.inKeyguardRestrictedInputMode();
        } else {
            return false;
        }
    }

    private static boolean isDeviceAsleep() {
        PowerManager powerManager = (PowerManager)getContext().getSystemService(Context.POWER_SERVICE);

        if(powerManager == null) {
            return false;
        }

        return !powerManager.isInteractive();
    }

    public static boolean getUsbMode() {
        return sContext.mUsbMode;
    }

    public static void sendMsgToJava(String msg) {
        if (!sContext.isAarLaunchMode()) {
            return;
        }

        try {
            JSONObject jsonObject = new JSONObject(msg);
            
            // 检查是否是customLoader消息
            if (jsonObject.has("msgname") && "customLoader".equals(jsonObject.getString("msgname"))) {
                JSONObject msgdata = jsonObject.getJSONObject("msgdata");
                Log.d("ParaEngineActivity", "msgdata: " + msgdata.toString());

                // 获取name和progress值
                if (msgdata.has("name") && msgdata.has("progress")) {
                    String name = msgdata.getString("name");
                    int progress = msgdata.getInt("progress");
                    String message = msgdata.optString("message", null);
                    
                    Log.d("ParaEngineActivity", "Loading type: " + name + ", progress: " + progress + "%");
                    
                    // 更新WebView中的进度条
                    if (sContext != null) {
                        // 根据加载类型映射到对应的进度条
                        String progressType = "loadingProgress1"; // 默认使用第一个进度条
                        if ("gameLoading".equals(name)) {
                            progressType = "loadingProgress1";
                        } else if ("assetLoading".equals(name)) {
                            progressType = "loadingProgress2";
                        }
                        
                        // 更新进度条
                        sContext.updateLoadingProgress(progressType, progress, message);
                    }
                    
                    // 检查是否为gameLoading的100%，隐藏loading动画
                    if ("gameLoading".equals(name) && progress >= 100) {
                        if (sContext != null) {
                            sContext.runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    Log.d("ParaEngineActivity", "Game loading completed, hiding loading animation");
                                    // 延迟一点时间再隐藏，确保用户能看到100%的进度
                                    sContext.mLoadingHandler.postDelayed(new Runnable() {
                                        @Override
                                        public void run() {
                                            sContext.hideLoadingAnimation();
                                        }
                                    }, 500); // 延迟500ms
                                }
                            });
                        }
                    }
                }
            }
        } catch (JSONException e) {
            Log.e("ParaEngineActivity", "Error parsing JSON message: " + e.getMessage());
        }
    }


    private boolean isAarLaunchMode() {
        try {
            TypedArray a = this.getTheme().obtainStyledAttributes(new int[0]);
            a.recycle();
            ActivityInfo activityInfo = this.getPackageManager().getActivityInfo(this.getComponentName(), 128);
            int themeResId = activityInfo.getThemeResource();
            String themeName = this.getResources().getResourceEntryName(themeResId);
            Log.d("ParaEngineActivity", "Current theme: " + themeName);
            return "AarTheme".equals(themeName);
        }
        catch (Exception e) {
            Log.e("ParaEngineActivity", "Error checking AAR launch mode", e);
            return false;
        }
    }

    private void createAndSetupGLSurfaceView(ParaEngineEditBox edittext) {
        Log.d("ParaEngineActivity", "Creating GLSurfaceView");
        this.mGLSurfaceView = this.onCreateView();
        this.mFrameLayout.addView(this.mGLSurfaceView);
        if (sRenderer == null) {
            sRenderer = new ParaEngineRenderer();
            this.mGLSurfaceView.setParaEngineRenderer(sRenderer);
        } else {
            sRenderer.termWindow();
            this.mGLSurfaceView.setParaEngineRenderer(sRenderer);
        }
        final ParaEngineEditBox finalEdittext = edittext;
        this.runOnGLThread(new Runnable(){
            @Override
            public void run() {
                ParaEngineActivity.this.runOnUiThread(new Runnable(){

                    @Override
                    public void run() {
                        ParaEngineActivity.this.mFrameLayout.addView(finalEdittext);
                        ParaEngineActivity.this.mGLSurfaceView.setParaEditText(finalEdittext);
                        ParaEngineActivity.this.mGLSurfaceView.bringToFront();

                        if (ParaEngineActivity.this.isAarLaunchMode()) {
                            ParaEngineActivity.this.mLoadingWebView.bringToFront();
                        }
                        
                        Log.d("ParaEngineActivity", "EditText added and configured after GL rendering");
                    }
                });
            }
        });
        Log.d("ParaEngineActivity", "GLSurfaceView created and configured");
    }

    private void startLoadingAnimation() {
        Log.d("ParaEngineActivity", "Starting loading animation with WebView for AAR mode");
        
        // 创建WebView
        mLoadingWebView = new WebView(this);
        
        // 配置WebView设置
        WebSettings webSettings = mLoadingWebView.getSettings();
        webSettings.setJavaScriptEnabled(true);
        webSettings.setDomStorageEnabled(true);
        webSettings.setLoadWithOverviewMode(true);
        webSettings.setUseWideViewPort(true);
        webSettings.setBuiltInZoomControls(false);
        webSettings.setDisplayZoomControls(false);
        webSettings.setSupportZoom(false);
        webSettings.setDefaultTextEncodingName("utf-8");
        
        // 设置WebViewClient
        mLoadingWebView.setWebViewClient(new WebViewClient() {
            @Override
            public void onPageFinished(WebView view, String url) {
                super.onPageFinished(view, url);
                Log.d("ParaEngineActivity", "Loading page finished: " + url);
                
                // 注入JavaScript代码
                injectProgressJavaScript(view);
            }
            
            @Override
            public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) {
                super.onReceivedError(view, errorCode, description, failingUrl);
                Log.e("ParaEngineActivity", "WebView error: " + description);
            }
        });
        
        // 设置WebView为全屏
        ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, 
            ViewGroup.LayoutParams.MATCH_PARENT
        );
        mLoadingWebView.setLayoutParams(layoutParams);
        
        // 添加到主布局
        mFrameLayout.addView(mLoadingWebView);
        
        // 加载指定的URL
        String loadingUrl = "https://keepwork.com/api/raw/maisi/maisi/webgames/data/custom_loader1?isLoading=true";
        mLoadingWebView.loadUrl(loadingUrl);
        
        Log.d("ParaEngineActivity", "Loading WebView with URL: " + loadingUrl);
    }

    
    private void hideLoadingAnimation() {
        if (mLoadingWebView != null && mFrameLayout != null) {
            mFrameLayout.removeView(mLoadingWebView);
            mLoadingWebView.destroy();
            mLoadingWebView = null;
        }
        mLoadingHandler.removeCallbacksAndMessages(null);
    }
    
    /**
     * 注入JavaScript代码到WebView中
     */
    private void injectProgressJavaScript(WebView webView) {
        String jsCode = """
            // 在页面中添加错误处理和回调
            window.setLoadingProgress = function(type, progress, message) {
                try {
                    // 参数验证
                    if (typeof progress !== 'number' || progress < 0 || progress > 100) {
                        console.warn('Invalid progress value:', progress);
                        return false;
                    }
                    
                    // 更新进度
                    if (type === 'loadingProgress1') {
                        if (typeof loadingProgress1 !== 'undefined') {
                            loadingProgress1 = progress;
                        }
                    } else if (type === 'loadingProgress2') {
                        if (typeof loadingProgress2 !== 'undefined') {
                            loadingProgress2 = progress;
                        }
                    } else {
                        // 默认更新主进度
                        if (typeof loadingProgress1 !== 'undefined') {
                            loadingProgress1 = progress;
                        }
                        if (typeof loadingProgress2 !== 'undefined') {
                            loadingProgress2 = 0;
                        }
                    }
                    
                    // 调用更新进度函数（如果存在）
                    if (typeof updateProgress === 'function') {
                        updateProgress();
                    }
                    
                    // 更新加载文本（如果存在）
                    if (message) {
                        var loadingTextElement = document.getElementById('loadingText');
                        if (loadingTextElement) {
                            loadingTextElement.textContent = message;
                        }
                    }
                    
                    // 通知原生端更新成功
                    if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.progressCallback) {
                        window.webkit.messageHandlers.progressCallback.postMessage({
                            type: 'progressUpdated',
                            progress: progress
                        });
                    }
                    
                    console.log('Progress updated:', type, progress, message);
                    return true;
                } catch (error) {
                    console.error('设置进度失败:', error);
                    return false;
                }
            }""";
        
        webView.evaluateJavascript(jsCode, null);
        Log.d("ParaEngineActivity", "JavaScript progress function injected successfully");
    }
    
    /**
     * 从Java代码更新WebView中的加载进度
     * @param type 进度类型 ("loadingProgress1", "loadingProgress2", 或其他)
     * @param progress 进度值 (0-100)
     * @param message 可选的消息文本
     */
    public void updateLoadingProgress(String type, int progress, String message) {
        if (mLoadingWebView != null) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    String jsCall = String.format(
                        "if (typeof window.setLoadingProgress === 'function') { " +
                        "window.setLoadingProgress('%s', %d, %s); }",
                        type,
                        progress,
                        message != null ? "'" + message.replace("'", "\\'") + "'" : "null"
                    );
                    
                    mLoadingWebView.evaluateJavascript(jsCall, null);
                    Log.d("ParaEngineActivity", "Called setLoadingProgress: " + type + ", " + progress + "%, " + message);
                }
            });
        } else {
            Log.w("ParaEngineActivity", "Cannot update progress: mLoadingWebView is null");
        }
    }
    
    /**
     * 从Java代码更新WebView中的加载进度（无消息）
     * @param type 进度类型
     * @param progress 进度值 (0-100)
     */
    public void updateLoadingProgress(String type, int progress) {
        updateLoadingProgress(type, progress, null);
    }
    
    /**
     * 静态方法：更新加载进度
     * @param type 进度类型 ("loadingProgress1", "loadingProgress2", 或其他)
     * @param progress 进度值 (0-100)
     * @param message 可选的消息文本
     */
    @Keep
    public static void setLoadingProgress(String type, int progress, String message) {
        if (sContext != null) {
            sContext.updateLoadingProgress(type, progress, message);
        } else {
            Log.w("ParaEngineActivity", "Cannot update progress: sContext is null");
        }
    }
    
    /**
     * 静态方法：更新加载进度（无消息）
     * @param type 进度类型
     * @param progress 进度值 (0-100)
     */
    @Keep
    public static void setLoadingProgress(String type, int progress) {
        setLoadingProgress(type, progress, null);
    }

    public static void onExit(){
        sContext.finish();
        System.exit(0);
    }

    public static boolean HasPermission(String strPermission){
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }
        else{
            return ContextCompat.checkSelfPermission(sContext,strPermission) == PackageManager.PERMISSION_GRANTED;
        }
    }

    // 写入base64图片数据到相册
    public static void saveImageToGallery(String imageData) throws JSONException {
        // 将Base64字符串解码为byte数组
        String imageName = "default.png";
        Log.d("ParaEngineHelper", "saveImageToGallery: " + imageData);

        JSONObject jsonObject = new JSONObject(imageData);

        String base64Data = jsonObject.getString("base64");
        if (base64Data.length() == 0){
            return;
        }
        byte[] decodedString = Base64.decode(base64Data, Base64.DEFAULT);
        if (jsonObject.getString("paraFilePath") != ""){
            imageName = jsonObject.getString("paraFilePath");
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            // Android Q及以上版本需要使用ContentResolver.insert()方法插入数据
            ContentValues values = new ContentValues();
            values.put(MediaStore.Images.Media.DISPLAY_NAME, imageName);
            values.put(MediaStore.Images.Media.MIME_TYPE, "image/png");
            values.put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES + "/paracrat");

            ContentResolver resolver = sContext.getContentResolver();
            Uri uri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
            if (uri == null) {
                return;
            }

            try (OutputStream outputStream = resolver.openOutputStream(uri)) {
                outputStream.write(decodedString);
                outputStream.flush();
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            // Android Q以下版本可以直接向外部存储写入数据
            File dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "paracrat");
            if (!dir.exists()) {
                dir.mkdirs();
            }
            File file = new File(dir, imageName);

            try (FileOutputStream stream = new FileOutputStream(file)) {
                stream.write(decodedString);
                stream.flush();
                stream.close();

                // 通知系统图库更新文件
                MediaScannerConnection.scanFile(sContext, new String[]{file.getAbsolutePath()}, null, null);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void OpenFileDialog(String filter) {
        mOpenFileDialogLuancher.launch(filter);
    }

    protected void RegisterActivityResultLauncher() {
        mOpenFileDialogLuancher = registerForActivityResult(
            new ActivityResultContract<String, Uri>() {
                @NonNull
                @Override
                public Intent createIntent(@NonNull Context context, String input) {
                    return new Intent(Intent.ACTION_PICK).setType(input);
                }
            @Override
            public Uri parseResult(int resultCode, @Nullable Intent intent) {
                if (intent == null || resultCode != Activity.RESULT_OK) return null;
                return intent.getData();
            }
        }, new ActivityResultCallback<Uri>() {
            @Override
            public void onActivityResult(Uri result) {
                String filepath = result == null ? "" : ParaEngineHelper.getFileAbsolutePath(getContext(), result);
                ParaEngineHelper.OpenFileDialogCallback(filepath);
            }
        });
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        sContext = this;
        GlobalObject.setActivity(this);
        super.onCreate(savedInstanceState);

        RegisterActivityResultLauncher();

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (checkSelfPermission(Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED)
        {
            this._init(savedInstanceState, false);
            mSavedInstanceState = savedInstanceState;
        } else {
            this._init(savedInstanceState, true);
        }

        if (isAarLaunchMode()) {
            handleIntent(getIntent());
        }

        NotchScreenManager.getInstance().setDisplayInNotch(this);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        handleIntent(intent);
        super.onNewIntent(intent);
    }

    private void handleIntent(Intent intent) {
        if (intent != null && Intent.ACTION_VIEW.equals(intent.getAction())) {
            Uri data = intent.getData();
            if (data != null) {
                String protocol = data.getScheme();
                String protocolData = intent.getDataString();
                // 在这里处理传入的protocol
                if (protocol == null || protocol == ""){
                    return;
                }
                if (protocolData == null || protocolData == ""){
                    return;
                }
                ParaEngineHelper.onCmdProtocol(protocolData);
            }
        }
    }

    private void checkUsbMode(){
        // Determine whether the device uses USB
        Configuration configuration = getResources().getConfiguration();
        boolean mouseExist = false;

        final int[] devices = InputDevice.getDeviceIds();

        for (int j : devices) {
            InputDevice device = InputDevice.getDevice(j);

            if (device != null && device.getName().contains("Mouse")) {
                mouseExist = true;
            }
        }

        if (configuration.keyboard != Configuration.KEYBOARD_NOKEYS || mouseExist) {
            mUsbMode = true;
        }
    }

    // lua端调用，用户同意隐私政策和用户协议以后，再去进行相关敏感操作
    public void onAgreeUserPrivacy(){
        this.checkUsbMode();
        ParaEngineHelper.onAgreeUserPrivacy();
        ParaEngineHelper.setCanReadPhoneState(false);
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

//        if (mGLSurfaceView != null)
//            mGLSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        ParaEnginePluginWrapper.onResume();
//        resumeIfHasFocus();
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
        if (mGLSurfaceView != null)
            mGLSurfaceView.setVisibility(View.VISIBLE);
    }

    @Override
    protected void onStop() {
        super.onStop();
        ParaEnginePluginWrapper.onStop();
        if (mGLSurfaceView != null)
            mGLSurfaceView.setVisibility(View.INVISIBLE);
    }

    protected void onLoadNativeLibraries() {
        try {
            String libname = "main";

            try {
                ActivityInfo ai;
                ai = getPackageManager()
                        .getActivityInfo(
                            getIntent().getComponent(),
                            PackageManager.GET_META_DATA
                        );

                if (ai.metaData != null) {
                    String ln = ai.metaData.getString(META_DATA_LIB_NAME);

                    if (ln != null)
                        libname = ln;
                }
            } catch (PackageManager.NameNotFoundException e) {
                throw new RuntimeException("Error getting activity info", e);
            }

            System.loadLibrary(libname);
        }catch (Exception e) {
            e.printStackTrace();
        }
    }

    protected void initLayout() {
        ViewGroup.LayoutParams framelayout_params = new ViewGroup.LayoutParams(-1, -1);
        this.mFrameLayout = new ResizeLayout(this);
        this.mFrameLayout.setLayoutParams(framelayout_params);
        ViewGroup.LayoutParams edittext_layout_params = new ViewGroup.LayoutParams(-1, -2);
        final ParaEngineEditBox edittext = new ParaEngineEditBox(this);
        edittext.setLayoutParams(edittext_layout_params);
        edittext.setMultilineEnabled(false);
        edittext.setReturnType(0);
        edittext.setInputMode(6);
        edittext.setEnabled(false);
        edittext.setBackground(null);
        boolean isAarMode = this.isAarLaunchMode();
        if (isAarMode) {
            Log.d("ParaEngineActivity", "AAR launch mode detected, delaying GLSurfaceView creation by 3 seconds");
            this.startLoadingAnimation();
            new Handler().postDelayed(new Runnable(){
                @Override
                public void run() {
                    ParaEngineActivity.this.createAndSetupGLSurfaceView(edittext);
                }
            }, 3000L);
        } else {
            Log.d("ParaEngineActivity", "Normal launch mode, creating GLSurfaceView immediately");
            this.createAndSetupGLSurfaceView(edittext);
        }
        if (!this.isAarLaunchMode()) {
            final ImageView imageView = new ImageView(this);
            imageView.setLayoutParams(new ViewGroup.LayoutParams(-1, -1));
            imageView.setScaleType(ImageView.ScaleType.FIT_XY);
            imageView.setImageResource(this.getResources().getIdentifier("app_splash", "drawable", this.getPackageName()));
            this.mFrameLayout.addView(imageView);
            CountDownTimer countDownTimer = new CountDownTimer(3000L, 1000L){

                public void onTick(long millisUntilFinished) {
                }

                public void onFinish() {
                    ParaEngineActivity.this.mFrameLayout.removeView(imageView);
                }
            };
            countDownTimer.start();
        }
        this.setContentView(this.mFrameLayout);
    }

    protected void _init(Bundle savedInstanceState, boolean bGranted) {
        onLoadNativeLibraries();

        ParaEngineHelper.init(this, bGranted);

        this.mGLContextAttrs = getGLContextAttrs();

        this.initLayout();

        if (mWebViewHelper == null)
            mWebViewHelper = new ParaEngineWebViewHelper(mFrameLayout);

        ParaEnginePluginWrapper.init(
            this,
            savedInstanceState,
            new ParaEnginePluginWrapper.PluginWrapperListener() {
            @Override
                public void onInit() {
                    // TODO...
                }
            }
        );
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
        ParaEngineHelper.setCanReadPhoneState(bGranted);
//        final boolean _bGranted = bGranted;
//        this._init(savedInstanceState, _bGranted);
//
//        // init plugin
//        if (!ParaEnginePluginWrapper.init(
//              this,
//               savedInstanceState,
//               new ParaEnginePluginWrapper.PluginWrapperListener() {
//                   @Override
//                   public void onInit() {
//                       ParaEngineActivity.this._init(savedInstanceState, _bGranted);
//                   }
//               }
//          ))
//        {
//           this._init(savedInstanceState, _bGranted);
//        }
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
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        ParaEnginePluginWrapper.onRequestPermissionsResult(requestCode, permissions, grantResults);
        RequestAndroidPermission.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == RequestAndroidPermission.PHONE_STATE_PERMISSION_CODE) {
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
        } else if (requestCode == RequestAndroidPermission.SCREEN_RECORD_PERMISSIONS_CODE) {
            int granted = PackageManager.PERMISSION_GRANTED;

            for (int r : grantResults) {
                granted |= r;
            }

            ScreenRecorder.onRequestPermissionsResult(granted);
        }
    }

    public void runOnGLThread(final Runnable pRunnable) {
        mGLSurfaceView.queueEvent(pRunnable);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        ParaEngineHelper.setKeyState(keyCode,1);
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        ParaEngineHelper.setKeyState(keyCode,0);
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        this.hasFocus = hasFocus;
        resumeIfHasFocus();
        //沉浸模式设置
        if(hasFocus && Build.VERSION.SDK_INT>=19) {
            View decorView=getWindow().getDecorView();
            decorView.setSystemUiVisibility( View.SYSTEM_UI_FLAG_LAYOUT_STABLE |View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |View.SYSTEM_UI_FLAG_FULLSCREEN |View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    @Keep
    public String getFileDirsPath() {
        return getFilesDir().getAbsolutePath();
    }

    public ResizeLayout getFrameLayout() {
        return mFrameLayout;
    }
}

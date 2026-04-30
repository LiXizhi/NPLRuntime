//-----------------------------------------------------------------------------
// WebView.java
// Authors: LanZhiHong, LiXizhi, big
// Emails: LiXizhi@yeah.net
// Company: ParaEngine
// CreateDate: 2018.3.31
// ModifyDate: 2023.5.17
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.annotation.SuppressLint;
import android.content.ClipData;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.graphics.Rect;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.ViewGroup;
import android.webkit.PermissionRequest;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.JavascriptInterface;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.view.KeyEvent;
import android.os.Build;

import android.net.Uri;
import android.widget.RelativeLayout;
import android.content.Intent;
import android.provider.MediaStore;
import android.os.Environment;
import androidx.core.content.FileProvider;

import java.util.concurrent.CountDownLatch;
import java.io.File;

class ShouldStartLoadingWorker implements Runnable {
    private CountDownLatch mLatch;
    private boolean[] mResult;
    private final int mViewTag;
    private final String mUrlString;

    ShouldStartLoadingWorker(CountDownLatch latch, boolean[] result, int viewTag, String urlString) {
        this.mLatch = latch;
        this.mResult = result;
        this.mViewTag = viewTag;
        this.mUrlString = urlString;
    }

    @Override
    public void run() {
        this.mResult[0] = ParaEngineWebViewHelper._shouldStartLoading(mViewTag, mUrlString);
        this.mLatch.countDown(); // notify that result is ready
    }
}

public class ParaEngineWebView extends WebView {
    private static final String TAG = "ParaEngineWebView";
    private int mViewTag;
    private static String mAppScheme = "paracraft";
    private boolean mHideViewWhenClickBack = false;
    private static boolean m_bIgnoreCloseWhenClickBack = false;
    private int lastWebViewHeight = 0;
    public int defaultWidth = 0;
    public int defaultHeight = 0;
    private String mOriginalUrl = "";
    private boolean mIsShowingErrorPage = false;
    private android.webkit.ValueCallback<Uri[]> mFilePathCallback;
    private Uri mCapturedImageUri;

    public ParaEngineWebView(Context context) {
        this(context, -1);
    }

    public int getViewTag() {
        return mViewTag;
    }

    public void SetHideViewWhenClickBack(boolean b) {
        mHideViewWhenClickBack = b;
    }

    public void SetIgnoreCloseWhenClickBack(boolean b) {
        m_bIgnoreCloseWhenClickBack = b;
    }

    public void setTransparentBackground(boolean transparent) {
        if (transparent) {
            this.setBackgroundColor(Color.TRANSPARENT);
            ViewGroup parent = (ViewGroup) this.getParent();
            if (parent != null) {
                parent.setBackgroundColor(Color.TRANSPARENT);
            }
        } else {
            this.setBackgroundColor(Color.WHITE);
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {    
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {
//            if(m_bIgnoreCloseWhenClickBack){
//                return false;
//            }
//
//            if (mHideViewWhenClickBack) {
//                setVisibility(WebView.GONE);
//            } else {
//                ParaEngineWebViewHelper._onCloseView(this);
//            }
            ParaEngineWebViewHelper._onCloseView(this);
            return false;
        } else {
            return super.onKeyUp(keyCode, event);
        }    
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {
            return false;
        } else {
            return super.onKeyDown(keyCode, event);
        }
    }

    @SuppressLint("SetJavaScriptEnabled")
    public ParaEngineWebView(Context context, int viewTag) {
        super(context);

        mViewTag = viewTag;

        this.setFocusable(true);
        this.setFocusableInTouchMode(true);

        // this.getSettings().setBuiltInZoomControls(true);
        // this.setInitialScale(1);
        this.getSettings().setSupportZoom(true);
        this.getSettings().setLoadWithOverviewMode(true);
        this.getSettings().setUseWideViewPort(true);
        this.getSettings().setDomStorageEnabled(true);
        this.getSettings().setJavaScriptEnabled(true);
        this.setLayerType(LAYER_TYPE_HARDWARE, null);
        this.getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
        this.getSettings().setMediaPlaybackRequiresUserGesture(false);
        
        // 允许文件访问
        this.getSettings().setAllowFileAccess(true);
        this.getSettings().setAllowContentAccess(true);
        
        // 配置混合内容模式（允许HTTPS页面加载HTTP资源）
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            this.getSettings().setMixedContentMode(android.webkit.WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
        }

        // This risk does not exist above API level 19. We are using API level 24 and above。
        // // `searchBoxJavaBridge_` has big security risk. http://jvn.jp/en/jp/JVN53768697
        // try {
        //     Method method = this.getClass().getMethod("removeJavascriptInterface", new Class[]{String.class});
        //     method.invoke(this, "searchBoxJavaBridge_");
        // } catch (Exception e) {
        //     Log.d(TAG, "This API level do not support `removeJavascriptInterface`");
        // }

        // 启用WebView调试功能（用于查看console.log输出）
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            WebView.setWebContentsDebuggingEnabled(true);
        }
        
        // 添加JavaScript接口用于错误页面重试
        this.addJavascriptInterface(new WebViewJavaScriptInterface(), "ErrorPageInterface");
        
        this.setWebViewClient(new ParaEngineWebViewClient());
        this.setWebChromeClient(new WebChromeClient() {
            @Override public Bitmap getDefaultVideoPoster() {
                // hide android ugly default poster.
                return Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888);
            }

            @Override
            public void onPermissionRequest(PermissionRequest request) {
                for (String resource : request.getResources()) {
                    if (PermissionRequest.RESOURCE_VIDEO_CAPTURE.equals(resource)) {
                        // Authorization is required here for WebView to access the camera.
                        request.grant(request.getResources());
                    }
                }
            }

            @Override
            public boolean onShowFileChooser(WebView webView, ValueCallback<Uri[]> filePathCallback, FileChooserParams fileChooserParams) {
                if (mFilePathCallback != null) {
                    mFilePathCallback.onReceiveValue(null);
                }
                mFilePathCallback = filePathCallback;

                // 检查并请求相机权限
                RequestAndroidPermission.RequestCamera(new RequestAndroidPermission.RequestPermissionCallback() {
                    @Override
                    public void Callback(Boolean succeeded) {
                        if (succeeded) {
                            // 权限授予成功，启动相机
                            startCamera();
                        } else {
                            // 权限被拒绝
                            if (mFilePathCallback != null) {
                                mFilePathCallback.onReceiveValue(null);
                                mFilePathCallback = null;
                            }
                        }
                    }
                });

                return true;
            }
        });

        ParaEngineWebView.this.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            Rect r = new Rect();
            ParaEngineWebView.this.getWindowVisibleDisplayFrame(r);

            DisplayMetrics metrics = new DisplayMetrics();
            ParaEngineActivity.getContext().getWindowManager().getDefaultDisplay().getMetrics(metrics);
            final int screenHeight = metrics.heightPixels;
            final int screenWidth = metrics.widthPixels;
            // keyboard logic is only enabled in fullscreen mode.
            String strData = ""+ screenWidth+","+screenHeight+","+r.width()+","+r.height();
            ParaEngineWebViewHelper.SetSoftKeyBoard(strData);
//            if ((Math.abs(screenWidth - defaultWidth) < 100) && (Math.abs(screenHeight - defaultHeight) < 5)) {
//                if (lastWebViewHeight != r.height()) {
//                    RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(defaultWidth, r.height());
//                    setLayoutParams(layoutParams);
//                    lastWebViewHeight = r.height();
//                }
//            }
        });
    }

    class ParaEngineWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(final WebView view, final String urlString) {
            ParaEngineActivity activity = (ParaEngineActivity)getContext();

            try {
                Uri uri = Uri.parse(urlString);

                if (uri != null)
                {
                    if (uri.getScheme().equals(mAppScheme) || uri.getScheme().equals("papa")) {
                        activity.runOnGLThread(new Runnable() {
                            @Override
                            public void run() {
                                ParaEngineWebViewHelper.transportCmdLine(urlString);
                            }
                        });

                        if (mHideViewWhenClickBack)
                            setVisibility( WebView.GONE);
                        else
                            ParaEngineWebViewHelper._onCloseView((ParaEngineWebView)view);

                        return false;
                    }
//                    else if (uri.getScheme().equals(mJSScheme)) {
//                        activity.runOnGLThread(new Runnable() {
//                            @Override
//                            public void run() {
//                                ParaEngineWebViewHelper._onJsCallback(mViewTag, urlString);
//                            }
//                        });
//                        return true;
//                    }
                }
            } catch (Exception e) {
                Log.d(TAG, "Failed to create URI from url");
            }

            boolean[] result = new boolean[] { true };
            CountDownLatch latch = new CountDownLatch(1);
            // run worker on gl thread
            activity.runOnGLThread(new ShouldStartLoadingWorker(latch, result, mViewTag, urlString));

            // wait for result from gl thread
            try {
                latch.await();
            } catch (InterruptedException ex) {
                Log.d(TAG, "'shouldOverrideUrlLoading' failed");
            }

            //if (result[0])
            //	view.loadUrl(urlString);

            return result[0];
        }

        @Override
        public void onPageStarted(WebView view, String url, Bitmap favicon) {
            super.onPageStarted(view, url, favicon);
            
            // 保存原始URL，但不保存错误页面的URL
            if (url != null && !url.startsWith("file:///android_asset/error_page.html")) {
                mOriginalUrl = url;
            } else {
                Log.d(TAG, "Skipped saving URL (error page): " + url);
            }
        }

        @Override
        public void onPageFinished(WebView view, final String url) {
            super.onPageFinished(view, url);
            
            // 如果成功加载了非错误页面，重置错误页面状态
            if (url != null && !url.startsWith("file:///android_asset/error_page.html")) {
                mIsShowingErrorPage = false;
            }
        }

        @Override
        public void onReceivedError(WebView view, int errorCode, String description, final String failingUrl) {
            super.onReceivedError(view, errorCode, description, failingUrl);
            
            if (!mIsShowingErrorPage && isNetworkError(errorCode)) {
                loadErrorPage(failingUrl, errorCode, description);
                return;
            }
            
            ParaEngineActivity activity = (ParaEngineActivity)getContext();
            activity.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    ParaEngineWebViewHelper._didFailLoading(mViewTag, failingUrl);
                }
            });
        }

        @Override
        public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
            super.onReceivedError(view, request, error);
            
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                if (!request.isForMainFrame()) {
                    return;
                }
                
                String failingUrl = request.getUrl().toString();
                int errorCode = error.getErrorCode();
                String description = error.getDescription().toString();
                
                if (!mIsShowingErrorPage && isNetworkError(errorCode)) {
                    loadErrorPage(failingUrl, errorCode, description);
                    return;
                }
            }
        }

        // 判断是否为网络相关错误
        private boolean isNetworkError(int errorCode) {
            return errorCode == WebViewClient.ERROR_HOST_LOOKUP ||
                   errorCode == WebViewClient.ERROR_CONNECT ||
                   errorCode == WebViewClient.ERROR_TIMEOUT ||
                   errorCode == WebViewClient.ERROR_UNKNOWN ||
                   errorCode == WebViewClient.ERROR_IO;
        }
    }

//    public void setJavascriptInterfaceScheme(String scheme) {
//        this.mJSScheme = scheme != null ? scheme : "";
//    }

//    public void setWebViewRect(int left, int top, int maxWidth, int maxHeight) {
//        FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
//                FrameLayout.LayoutParams.WRAP_CONTENT);
//        layoutParams.leftMargin = left;
//        layoutParams.topMargin = top;
//        layoutParams.width = maxWidth;
//        layoutParams.height = maxHeight;
//        layoutParams.gravity = Gravity.TOP | Gravity.LEFT;
//        this.setLayoutParams(layoutParams);
//    }

    public void setScalesPageToFit(boolean scalesPageToFit) {
        this.getSettings().setSupportZoom(scalesPageToFit);
    }

    // JavaScript接口类，用于错误页面与Android的交互
    public class WebViewJavaScriptInterface {
        @JavascriptInterface
        public void onRetryClicked() {
            post(new Runnable() {
                @Override
                public void run() {
                    // 重置错误页面状态
                    mIsShowingErrorPage = false;
                    
                    // 确定要加载的URL
                    String urlToLoad = null;
                    if (!mOriginalUrl.isEmpty() && !mOriginalUrl.startsWith("file:///android_asset/error_page.html")) {
                        urlToLoad = mOriginalUrl;
                    }
                    
                    if (urlToLoad != null) {
                        loadUrl(urlToLoad);
                    } else {
                        reload();
                    }
                }
            });
        }

        @JavascriptInterface
        public void onCheckNetworkClicked() {
            post(new Runnable() {
                @Override
                public void run() {
                    boolean isNetworkAvailable = ParaEngineWebView.this.isNetworkAvailable();
                    
                    // 调用JavaScript方法更新网络状态显示
                    String jsCode = "if (typeof showNetworkStatus === 'function') { " +
                            "showNetworkStatus(" + isNetworkAvailable + ", '" + 
                            (isNetworkAvailable ? "网络连接正常" : "网络连接不可用") + "'); }";
                    
                    evaluateJavascript(jsCode, null);
                }
            });
        }

        @JavascriptInterface
        public void retryLoadUrl(String url) {
            post(new Runnable() {
                @Override
                public void run() {
                    // 重置错误页面状态
                    mIsShowingErrorPage = false;
                    
                    // 确定要加载的URL
                    String urlToLoad = null;
                    if (url != null && !url.isEmpty() && !url.startsWith("file:///android_asset/error_page.html")) {
                        urlToLoad = url;
                    } else if (!mOriginalUrl.isEmpty() && !mOriginalUrl.startsWith("file:///android_asset/error_page.html")) {
                        urlToLoad = mOriginalUrl;
                    }
                    
                    if (urlToLoad != null) {
                        loadUrl(urlToLoad);
                    } else {
                        reload();
                    }
                }
            });
        }

        @JavascriptInterface
        public void onPageLoaded() {
            // 错误页面加载完成回调
        }

        @JavascriptInterface
        public boolean isNetworkAvailable() {
            return ParaEngineWebView.this.isNetworkAvailable();
        }
    }

    // 加载错误页面的方法
    private void loadErrorPage(String failingUrl, int errorCode, String description) {
        mIsShowingErrorPage = true;
        
        // 只有在mOriginalUrl为空或者当前URL不是错误页面时才更新mOriginalUrl
        if (mOriginalUrl.isEmpty() || !mOriginalUrl.startsWith("file:///android_asset/error_page.html")) {
            mOriginalUrl = failingUrl;
        }
        
        // 构建错误页面URL，包含错误信息参数
        String errorPageUrl = "file:///android_asset/error_page.html" +
                "?url=" + Uri.encode(failingUrl != null ? failingUrl : "") +
                "&title=" + Uri.encode("网页无法打开") +
                "&description=" + Uri.encode(description != null ? description : "请检查网络连接或稍后重试") +
                "&details=" + Uri.encode("错误代码: " + errorCode) +
                "&code=" + errorCode +
                "&networkAvailable=" + isNetworkAvailable();
        
        loadUrl(errorPageUrl);
    }

    // 检查网络是否可用
    private boolean isNetworkAvailable() {
        try {
            ConnectivityManager connectivityManager = 
                (ConnectivityManager) getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
            return activeNetworkInfo != null && activeNetworkInfo.isConnected();
        } catch (Exception e) {
            return false;
        }
    }

    // 创建临时图片文件
    private File createImageFile() throws Exception {
        String timeStamp = new java.text.SimpleDateFormat("yyyyMMdd_HHmmss").format(new java.util.Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = getContext().getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        return File.createTempFile(imageFileName, ".jpg", storageDir);
    }

    // 启动相机
    private void startCamera() {
        post(new Runnable() {
            @Override
            public void run() {
                try {
                    startCameraInternal();
                } catch (Exception e) {
                    Log.e(TAG, "Critical error in startCamera", e);
                    if (mFilePathCallback != null) {
                        mFilePathCallback.onReceiveValue(null);
                        mFilePathCallback = null;
                    }
                }
            }
        });
    }

    private void startCameraInternal() {
        Log.d(TAG, "startCameraInternal called");
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        
        // 尝试创建文件
        File photoFile = null;
        try {
            photoFile = createImageFile();
        } catch (Exception ex) {
            Log.e(TAG, "Unable to create Image File", ex);
        }

        if (photoFile != null) {
            try {
                mCapturedImageUri = FileProvider.getUriForFile(getContext(),
                        getContext().getPackageName() + ".fileprovider",
                        photoFile);
                Log.d(TAG, "Created file URI: " + mCapturedImageUri.toString());
            } catch (Exception e) {
                Log.e(TAG, "Failed to get URI for file", e);
            }
        }

        if (mCapturedImageUri != null) {
            takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, mCapturedImageUri);
            
            // 使用ClipData传递权限，这在Android 10+上更可靠
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                takePictureIntent.setClipData(ClipData.newRawUri("", mCapturedImageUri));
            }
            
            takePictureIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            takePictureIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } else {
            Log.w(TAG, "mCapturedImageUri is null, camera will not save high-res image");
        }

        ParaEngineActivity activity = (ParaEngineActivity)getContext();
        
        // 尝试检查是否有相机应用
        if (takePictureIntent.resolveActivity(getContext().getPackageManager()) != null) {
            try {
                activity.startActivityForResult(takePictureIntent, RequestAndroidPermission.FILE_CHOOSER_REQUEST_CODE);
                Log.d(TAG, "Camera intent started successfully");
                return;
            } catch (Exception ex) {
                Log.e(TAG, "Failed to start camera intent with resolveActivity check", ex);
            }
        } else {
            Log.w(TAG, "resolveActivity returned null");
        }
        
        // Fallback: 强制启动
        try {
            Log.d(TAG, "Attempting fallback start");
            activity.startActivityForResult(takePictureIntent, RequestAndroidPermission.FILE_CHOOSER_REQUEST_CODE);
            Log.d(TAG, "Fallback camera intent started successfully");
            return;
        } catch (Exception ex) {
            Log.e(TAG, "Fallback camera start failed", ex);
        }

        // 如果带URI启动完全失败，尝试不带URI启动（仅作为最后的保底，防止无响应）
        if (mCapturedImageUri != null) {
            Log.w(TAG, "Attempting start without URI as last resort");
            try {
                Intent simpleIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                activity.startActivityForResult(simpleIntent, RequestAndroidPermission.FILE_CHOOSER_REQUEST_CODE);
                Log.d(TAG, "Simple camera intent started");
                // 注意：这种情况下 onActivityResult 可能拿不到 mCapturedImageUri 的内容，
                // 但至少用户能看到相机启动。我们需要在 onActivityResult 里处理 data 返回的缩略图。
                return;
            } catch (Exception e) {
                Log.e(TAG, "Last resort start failed", e);
            }
        }

        // 所有尝试都失败
        Log.e(TAG, "All camera start attempts failed");
        if (mFilePathCallback != null) {
            mFilePathCallback.onReceiveValue(null);
            mFilePathCallback = null;
        }
    }

    // 处理文件选择结果
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == RequestAndroidPermission.FILE_CHOOSER_REQUEST_CODE) {
            Log.d(TAG, "onActivityResult - requestCode: " + requestCode + ", resultCode: " + resultCode);
            Uri[] results = null;

            if (resultCode == android.app.Activity.RESULT_OK) {
                if (data == null) {
                    Log.d(TAG, "Data is null, using captured image URI");
                    if (mCapturedImageUri != null) {
                        results = new Uri[]{mCapturedImageUri};
                        Log.d(TAG, "Using captured image URI: " + mCapturedImageUri.toString());
                    } else {
                        Log.e(TAG, "Both data and captured image URI are null");
                    }
                } else {
                    String dataString = data.getDataString();
                    Log.d(TAG, "Data is not null, dataString: " + dataString);
                    if (dataString != null) {
                        results = new Uri[]{Uri.parse(dataString)};
                    }
                }
            } else {
                Log.d(TAG, "Result code is not OK: " + resultCode);
            }

            if (mFilePathCallback != null) {
                mFilePathCallback.onReceiveValue(results);
                mFilePathCallback = null;
                Log.d(TAG, "File path callback executed with " + (results != null ? results.length : 0) + " results");
            } else {
                Log.e(TAG, "File path callback is null!");
            }
            mCapturedImageUri = null;
        }
    }
}

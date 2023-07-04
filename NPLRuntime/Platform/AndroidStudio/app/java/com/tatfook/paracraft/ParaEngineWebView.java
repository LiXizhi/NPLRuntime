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
import android.content.Context;
import android.graphics.Rect;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.ViewGroup;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.graphics.Bitmap;
import android.view.KeyEvent;

import android.net.Uri;
import android.widget.RelativeLayout;

import java.util.concurrent.CountDownLatch;

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

        // This risk does not exist above API level 19. We are using API level 24 and above。
        // // `searchBoxJavaBridge_` has big security risk. http://jvn.jp/en/jp/JVN53768697
        // try {
        //     Method method = this.getClass().getMethod("removeJavascriptInterface", new Class[]{String.class});
        //     method.invoke(this, "searchBoxJavaBridge_");
        // } catch (Exception e) {
        //     Log.d(TAG, "This API level do not support `removeJavascriptInterface`");
        // }

        this.setWebViewClient(new ParaEngineWebViewClient());
        this.setWebChromeClient(new WebChromeClient() {
            @Override public Bitmap getDefaultVideoPoster() {
            // hide android ugly default poster.
            return Bitmap.createBitmap(10, 10, Bitmap.Config.ARGB_8888);
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
            if ((Math.abs(screenWidth - defaultWidth) < 5) && (Math.abs(screenHeight - defaultHeight) < 5)) {
                if (lastWebViewHeight != r.height()) {
                    RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(screenWidth, r.height());
                    setLayoutParams(layoutParams);
                    lastWebViewHeight = r.height();
                }
            }
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
                    if (uri.getScheme().equals(mAppScheme)) {
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
        public void onPageFinished(WebView view, final String url) {
            super.onPageFinished(view, url);
            ParaEngineActivity activity = (ParaEngineActivity)getContext();
            activity.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    ParaEngineWebViewHelper._didFinishLoading(mViewTag, url);
                }
            });
        }

        @Override
        public void onReceivedError(WebView view, int errorCode, String description, final String failingUrl) {
            super.onReceivedError(view, errorCode, description, failingUrl);
            ParaEngineActivity activity = (ParaEngineActivity)getContext();
            activity.runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    ParaEngineWebViewHelper._didFailLoading(mViewTag, failingUrl);
                }
            });
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
}

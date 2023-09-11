//-----------------------------------------------------------------------------
// ParaEngineWebViewHelper.java
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2023.5.17
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.SparseArray;
import android.view.View;
import android.webkit.JavascriptInterface;
import android.widget.Button;
import android.widget.FrameLayout;
import android.webkit.WebView;
import android.util.Log;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;
import androidx.annotation.Keep;

class JsToAndroid extends Object {
    private native void receive(String filename, String msg);

    @JavascriptInterface
    public void nplActivate(String filename, String msg) {
        this.receive(filename, msg);
    }
}

public class ParaEngineWebViewHelper {
	private static ParaEngineActivity sActivity;
	private static Handler sHandler;
	private static FrameLayout sLayout;
	private static SparseArray<ParaEngineWebView> webViews;
	private static int viewTag = 0;
    private static boolean isOpenUrlLoaded = false;
    private static int openUrlViewTag = 0;

	private static native void onJsCallback(int index, String message);
	private static native void didFailLoading(int index, String message);
	private static native void didFinishLoading(int index, String message);
	private static native boolean shouldStartLoading(int index, String message);
//	private static native void onCloseView(int index);
	public  static native void transportCmdLine(String cmdStr);

    public  static native void SetSoftKeyBoard(String strData);
	public ParaEngineWebViewHelper(FrameLayout layout) {
		ParaEngineWebViewHelper.sLayout = layout;
        ParaEngineWebViewHelper.sHandler = new Handler(Looper.myLooper());
        ParaEngineWebViewHelper.sActivity = ParaEngineActivity.getContext();
        ParaEngineWebViewHelper.webViews = new SparseArray<ParaEngineWebView>();
	}

	public static void _onJsCallback(int index, String message) {
        onJsCallback(index, message);
    }

	public static void _didFailLoading(int index, String message) {
        didFailLoading(index, message);
    }

	public static void _didFinishLoading(int index, String message) {
        didFinishLoading(index, message);
    }

	public static boolean _shouldStartLoading(int index, String message) {
        return !shouldStartLoading(index, message);
    }

	public static void _onCloseView(ParaEngineWebView webView) {
		final int index = webView.getViewTag();

        if (canGoBack(index)) {
            goBack(index);
        } else {
            if (isOpenUrlLoaded && index == openUrlViewTag) {
                closeWebViewByIndex(index);
            }
        }
	}

	public static void closeWebViewByIndex(int index) {
        ParaEngineWebView webView = webViews.get(index);

        if (webView == null) {
            return;
        }

        if (webView.getViewTag() == openUrlViewTag) {
            openUrlViewTag = -1;
            isOpenUrlLoaded = false;
        }

        webViews.remove(index);
        RelativeLayout parentView = (RelativeLayout) webView.getParent();
        sLayout.removeView(parentView);
        webView.destroy();

//        if (m_maskView != null) {
//            sLayout.removeView(m_maskView);
//            m_maskView = null;
//        }

//        sActivity.runOnGLThread(new Runnable() {
//            @Override
//            public void run() {
//                onCloseView(index);
//            }
//        });
    }

    @Keep
    public static void openExternalBrowser(final String url) {
        try {
            Intent intent = new Intent();

            intent.setAction("android.intent.action.VIEW");
            Uri content_url = Uri.parse(url);
            intent.setData(content_url);

            sActivity.startActivity(intent);
        } catch (Exception e) {
            sActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(sActivity, "无法打开" + url, Toast.LENGTH_SHORT).show();
                }
            });
            e.printStackTrace();
        }
    }

    @Keep
    public static void openWebView(final int x, final int y, final int w, final int h, final String url) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (isOpenUrlLoaded) {
                    ParaEngineWebView webView = webViews.get(openUrlViewTag);
                    webView.loadUrl(url);

                    // /open command can only open one web page at the same time.
                    webView.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            webView.clearHistory();
                        }
                    }, 1000);

                    return;
                }

                int index = createWebView(x, y, w, h);
                ParaEngineWebView webView = webViews.get(index);
                webView.setAlpha(0.95f);
                webView.loadUrl(url);

                Button closeBtn = new Button(sActivity);
                LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(80, 80);
                layoutParams.leftMargin = 40;
                layoutParams.topMargin = h / 2 - 40;
                closeBtn.setLayoutParams(layoutParams);

                closeBtn.setBackgroundResource(R.drawable.webview_close_btn);
                closeBtn.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        closeWebViewByIndex(index);
                    }
                });

                RelativeLayout webViewWrapper = (RelativeLayout) webView.getParent();
                webViewWrapper.addView(closeBtn);

                isOpenUrlLoaded = true;
                openUrlViewTag = index;
            }
        });
    }

	@Keep
	public static int createWebView(final int x, final int y, final int w, final int h) {
		final int index = viewTag;

		sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = new ParaEngineWebView(sActivity, index);

				Log.i("ParaEngine", String.format("create web view with %d, %d, %d, %d", x, y, w, h));

                RelativeLayout webViewWrapper = new RelativeLayout(sActivity);
                RelativeLayout.LayoutParams wrapperLayoutParams = new RelativeLayout.LayoutParams(w, h);
                wrapperLayoutParams.leftMargin = x;
                wrapperLayoutParams.topMargin = y;
                webViewWrapper.setLayoutParams(wrapperLayoutParams);

                RelativeLayout.LayoutParams layoutParams =
                    new RelativeLayout.LayoutParams(
                        RelativeLayout.LayoutParams.MATCH_PARENT,
                        RelativeLayout.LayoutParams.MATCH_PARENT
                    );

                webView.setLayoutParams(layoutParams);
                webView.setScrollBarStyle(WebView.SCROLLBARS_OUTSIDE_OVERLAY);
                webView.addJavascriptInterface(new JsToAndroid(), "android");

                webViewWrapper.addView(webView);

                webView.requestFocus();
                sLayout.addView(webViewWrapper);

                webViews.put(index, webView);
            }
        });

        viewTag++;

        return index;
	}

	@Keep
	public static void removeWebView(final int index) {
        closeWebViewByIndex(index);
    }

    @Keep
	public static void setVisible(final int index, final boolean visible) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    webView.setVisibility(visible ? View.VISIBLE : View.GONE);
                }
            }
        });
    }

    @Keep
	public static void setJavascriptInterfaceScheme(final int index, final String scheme) {
        // sActivity.runOnUiThread(new Runnable() {
        //     @Override
        //     public void run() {
        //         ParaEngineWebView webView = webViews.get(index);
        //         if (webView != null) {
        //             webView.setJavascriptInterfaceScheme(scheme);
        //         }
        //     }
        // });
    }

    @Keep
	public static void loadData(final int index, final String data, final String mimeType, final String encoding, final String baseURL) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.loadDataWithBaseURL(baseURL, data, mimeType, encoding, null);
                }
            }
        });
    }

    @Keep
	public static void loadHTMLString(final int index, final String data, final String baseUrl) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.loadDataWithBaseURL(baseUrl, data, null, null, null);
                }
            }
        });
    }

    @Keep
	public static void loadUrl(final int index, final String url) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    webView.loadUrl(url);
                }
            }
        });
    }

    @Keep
	public static void loadFile(final int index, final String filePath) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    webView.loadUrl(filePath);
                }
            }
        });
    }

    @Keep
	public static void stopLoading(final int index) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.stopLoading();
                }
            }
        });
    }

	public static void reload(final int index) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    webView.reload();
                }
            }
        });
    }

	public static <T> T callInMainThread(Callable<T> call) throws ExecutionException, InterruptedException {
        FutureTask<T> task = new FutureTask<T>(call);
        sHandler.post(task);
        return task.get();
    }

    @Keep
	public static boolean canGoBack(final int index) {
        ParaEngineWebView webView = webViews.get(index);
        return webView != null && webView.canGoBack();
//        Callable<Boolean> callable = new Callable<Boolean>() {
//            @Override
//            public Boolean call() throws Exception {
//                ParaEngineWebView webView = webViews.get(index);
//                return webView != null && webView.canGoBack();
//            }
//        };
//        try {
//            return callInMainThread(callable);
//        } catch (ExecutionException e) {
//            return false;
//        } catch (InterruptedException e) {
//            return false;
//        }
    }

    @Keep
	public static boolean canGoForward(final int index) {
        Callable<Boolean> callable = new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                ParaEngineWebView webView = webViews.get(index);
                return webView != null && webView.canGoForward();
            }
        };
        try {
            return callInMainThread(callable);
        } catch (ExecutionException e) {
            return false;
        } catch (InterruptedException e) {
            return false;
        }
    }

    @Keep
	public static void goBack(final int index) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.goBack();
                }
            }
        });
    }

    @Keep
	public static void goForward(final int index) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.goForward();
                }
            }
        });
    }

    @Keep
	public static void evaluateJS(final int index, final String js) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.loadUrl("javascript:" + js);
                }
            }
        });
    }

    @Keep
	public static void setScalesPageToFit(final int index, final boolean scalesPageToFit) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.setScalesPageToFit(scalesPageToFit);
                }
            }
        });
    }

    @Keep
	public static void setViewAlpha(final int index, final float alpha) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
					webView.setAlpha(alpha);
                }
            }
        });
    }

    @Keep
    public static void move(final int index, final int x, final int y) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    RelativeLayout webViewWrapper = (RelativeLayout) webView.getParent();
                    FrameLayout.LayoutParams wp = (FrameLayout.LayoutParams) webViewWrapper.getLayoutParams();
                    wp.leftMargin = x;
                    wp.topMargin = y;
                    webViewWrapper.setLayoutParams(wp);
                }
            }
        });
    }

    @Keep
    public static void resize(final int index, final int w, final int h) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    RelativeLayout webViewWrapper = (RelativeLayout) webView.getParent();
                    FrameLayout.LayoutParams wp = (FrameLayout.LayoutParams) webViewWrapper.getLayoutParams();
                    webView.defaultWidth = w;
                    webView.defaultHeight = h;
                    wp.width = w;
                    wp.height = h;
                    webViewWrapper.setLayoutParams(wp);
                }
            }
        });
    }
}

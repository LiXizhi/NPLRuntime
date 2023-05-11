//-----------------------------------------------------------------------------
// ParaEngineWebViewHelper.java
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2023.5.10
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.SparseArray;
import android.view.View;
import android.webkit.JavascriptInterface;
import android.widget.FrameLayout;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.webkit.WebViewClient;
import android.util.Log;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.widget.RelativeLayout;
import android.widget.Toast;
import androidx.annotation.Keep;

//class ParaWebViewClient extends WebViewClient {
//	@Override
//    public boolean shouldOverrideUrlLoading(WebView view, String url) {
//		view.loadUrl(url);
//        return true;
//    }
//
//    public void onPageFinished(WebView view, String url)
//    {
//    }
//}

class AndroidToJs extends Object {
    @JavascriptInterface
    public void sendMessageToNative() {

    }
}

public class ParaEngineWebViewHelper {
	private static ParaEngineActivity sActivity;
	private static Handler sHandler;
	private static FrameLayout sLayout;

	private static SparseArray<ParaEngineWebView> webViews;
	private static int viewTag = 0;

	private static RelativeLayout m_maskView = null;

	private static native void onJsCallback(int index, String message);
	private static native void didFailLoading(int index, String message);
	private static native void didFinishLoading(int index, String message);
	private static native boolean shouldStartLoading(int index, String message);
	private static native void onCloseView(int index);
	public  static native void transportCmdLine(String cmdStr);

	public ParaEngineWebViewHelper(FrameLayout layout) {
		ParaEngineWebViewHelper.sLayout = layout;
        ParaEngineWebViewHelper.sHandler = new Handler(Looper.myLooper());
        ParaEngineWebViewHelper.sActivity = ParaEngineActivity.getContext();
        ParaEngineWebViewHelper.webViews = new SparseArray<ParaEngineWebView>();

		// mWebView = null;
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

	// private static WebView mWebView = null;

	// @Keep
	// public static void closeWebView() {
	// 	sActivity.runOnUiThread(new Runnable() {
    //         @Override
    //         public void run() {
	// 			if(mWebView != null)
	// 			{
	// 				ViewGroup vg = (ViewGroup)mWebView.getParent();
	// 				vg.removeView(mWebView);
	// 				mWebView.destroy();
	// 				mWebView = null;
	// 			}
    //         }
    //     });
	// }

	public static void _onCloseView(ParaEngineWebView webView) {
		final int index = webView.getViewTag();

        if (canGoBack(index)) {
            goBack(index);
        } else {
            closeWebViewByIndex(index);
        }
	}

	public static void closeWebViewByIndex(int index){
        ParaEngineWebView webView = webViews.get(index);

        if (webView == null) {
            return;
        }

        webViews.remove(index);
        sLayout.removeView(webView);
        webView.destroy();

        if (m_maskView != null) {
            sLayout.removeView(m_maskView);
            m_maskView = null;
        }

        sActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                //onCloseView(index);
            }
        });
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
                int index = createWebView(x, y, w, h);
                ParaEngineWebView webView = webViews.get(index);
                webView.setAlpha(0.95f);
                webView.loadUrl(url);
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

				Log.i("ParaEngine", String.format("open web view with %d, %d, %d, %d", x,y, w, h));

                RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(w, h);
                layoutParams.leftMargin = x;
                layoutParams.topMargin = y;

//                if(m_maskView == null){
//                    m_maskView = new RelativeLayout(sActivity);
//
//                    RelativeLayout.LayoutParams mask_layout =
//                        new RelativeLayout.LayoutParams(
//                            ViewGroup.LayoutParams.FILL_PARENT,
//                            ViewGroup.LayoutParams.FILL_PARENT
//                        );
//
//                    sLayout.addView(m_maskView, mask_layout);
//                    m_maskView.setOnClickListener(new View.OnClickListener() {
//                        @Override
//                        public void onClick(View view) {
//                            ParaEngineWebViewHelper.closeWebViewByIndex(index);
//                        }
//                    });
//
//                    m_maskView.setVisibility(View.GONE);
//                }

                sLayout.addView(webView, layoutParams);
                webView.setScrollBarStyle(WebView.SCROLLBARS_OUTSIDE_OVERLAY);
				webView.requestFocus();

                webViews.put(index, webView);
            }
        });

        viewTag++;

        return index;
	}

	public static void setMaskVisible(final boolean visible){
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(m_maskView!=null){
                    m_maskView.setVisibility(visible?View.VISIBLE:View.GONE);
                }
            }
        });
    }

	@Keep
	public static void removeWebView(final int index) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webViews.remove(index);
                    sLayout.removeView(webView);
					webView.destroy();
                }
            }
        });
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

                removeWebView(index);
            }
        });
    }

    @Keep
	public static void SetHideViewWhenClickBack(final int index, final boolean b) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    webView.SetHideViewWhenClickBack(b);
                }
            }
        });
    }

    @Keep
    public static void SetIgnoreCloseWhenClickBack(final int index,final boolean bool) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView == null){
                    return;
                }

                webView.SetIgnoreCloseWhenClickBack(bool);
            }
        });
    }
    
    @Keep
    public static void SetCloseWhenClickBackground(final int index,final boolean bool) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(m_maskView!=null){
                    m_maskView.setVisibility(bool?View.VISIBLE:View.GONE);
                }
            }
        });
    }
    
    @Keep
	public static void setWebViewRect(final int index, final int left, final int top, final int maxWidth, final int maxHeight) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.setWebViewRect(left, top, maxWidth, maxHeight);
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
	public static void loadUrl(final int index, final String url, final boolean cleanCachedData) {
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
                    //webView.setAlpha((int)(255 * alpha));
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
                    FrameLayout.LayoutParams p = (FrameLayout.LayoutParams)webView.getLayoutParams();
                    p.leftMargin = x;
                    p.topMargin = y;
                    webView.setLayoutParams(p);
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
                    FrameLayout.LayoutParams p = (FrameLayout.LayoutParams) webView.getLayoutParams();
                    p.width = w;
                    p.height = h;
                    webView.setLayoutParams(p);
                }
            }
        });
    }
}

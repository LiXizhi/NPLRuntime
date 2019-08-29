package com.tatfook.paracraft;

import android.os.Handler;
import android.os.Looper;
import android.support.annotation.Keep;
import android.util.SparseArray;
import android.view.View;
import android.widget.FrameLayout;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.view.Gravity;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.webkit.WebViewClient;
import android.util.Log;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import com.tatfook.paracraft.AppActivity;
import com.tatfook.paracraft.ParaEngineWebView;

import android.util.Log;

class HelloWebViewClient extends WebViewClient { 
	@Override
    public boolean shouldOverrideUrlLoading(WebView view, String url) { 
		view.loadUrl(url); 
        return true; 
    } 
    public void onPageFinished(WebView view, String url)
    {

    }
}

public class ParaEngineWebViewHelper {
	private static final String TAG = "ParaEngine";
	private static AppActivity sActivity;
	private static Handler sHandler;
	private static FrameLayout sLayout;

	private static SparseArray<ParaEngineWebView> webViews;
	private static int viewTag = 0;

	private static native void onJsCallback(int index, String message);
	private static native void didFailLoading(int index, String message);
	private static native void didFinishLoading(int index, String message);
	private static native boolean shouldStartLoading(int index, String message);
	private static native void onCloseView(int index);
	public  static native void transportCmdLine(String cmdStr);

	public ParaEngineWebViewHelper(FrameLayout layout) {
		ParaEngineWebViewHelper.sLayout = layout;
        ParaEngineWebViewHelper.sHandler = new Handler(Looper.myLooper());

        ParaEngineWebViewHelper.sActivity = (AppActivity)AppActivity.getContext();
        ParaEngineWebViewHelper.webViews = new SparseArray<ParaEngineWebView>();

		mWebView = null;
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

	private static WebView mWebView = null;

	@Keep
	public static void closeWebView() {
		sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
				if(mWebView != null)
				{
					ViewGroup vg = (ViewGroup)mWebView.getParent();
					vg.removeView(mWebView);
					mWebView.destroy();
					mWebView = null;
				}
            }
        });
	}

	
   
    @Keep
	public static void openWebView(final int x, final int y, final int w, final int h, final String url) {
		sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
				if(mWebView != null)
				{
					ViewGroup vg = (ViewGroup)mWebView.getParent();
					vg.removeView(mWebView);
					//sLayout.removeView(mWebView);
					mWebView.destroy();
					mWebView = null;
				}

				WebView webView = new WebView(sActivity);
				mWebView = webView;
				webView.getSettings().setJavaScriptEnabled(true);
				webView.requestFocus();
				webView.setScrollBarStyle(WebView.SCROLLBARS_OUTSIDE_OVERLAY);
				webView.loadUrl(url);
				webView.setWebViewClient(new HelloWebViewClient());

				if (x >= 0 && y >= 0)
				{
					MarginLayoutParams pa = new MarginLayoutParams(w , h);
					pa.setMargins(x ,  y ,  0 ,  0);
					FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(pa);
					sLayout.addView(webView, params);

					//sActivity.getWindowManager().addView(webView, params);
				}
				else
				{
					// FrameLayout.LayoutParams.WRAP_CONTENT not supported
					FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(w, h);   
				    // params.gravity = Gravity.CENTER | Gravity.CENTER_HORIZONTAL;
			        sLayout.addView(webView, params);
				}
            }
        });
	}

	public static void _onCloseView(ParaEngineWebView webView) {
		final int index = webView.getViewTag();
		webViews.remove(index);
		sLayout.removeView(webView);
		webView.destroy();

		sActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
				onCloseView(index);
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

				if (x >= 0 && y >= 0)
				{
					MarginLayoutParams pa = new MarginLayoutParams(w , h);
					pa.setMargins(x ,  y ,  0 ,  0);
					FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(pa);
					sLayout.addView(webView, params);

					//sActivity.getWindowManager().addView(webView, params);
				}
				else
				{
					// FrameLayout.LayoutParams.WRAP_CONTENT not supported
					FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(w, h);   
				    // params.gravity = Gravity.CENTER | Gravity.CENTER_HORIZONTAL;
			        sLayout.addView(webView, params);
				}
				webView.requestFocus();
                webViews.put(index, webView);
            }
        });
        return viewTag++;
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
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);
                if (webView != null) {
                    webView.setJavascriptInterfaceScheme(scheme);
                }
            }
        });
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
                    webView.getSettings().setCacheMode(cleanCachedData ? WebSettings.LOAD_NO_CACHE
                                                                       : WebSettings.LOAD_DEFAULT);
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
        Callable<Boolean> callable = new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                ParaEngineWebView webView = webViews.get(index);
                return webView != null && webView.canGoBack();
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
}
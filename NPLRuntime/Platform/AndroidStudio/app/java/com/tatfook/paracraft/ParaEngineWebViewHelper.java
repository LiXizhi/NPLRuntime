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
import android.widget.FrameLayout;
import android.webkit.WebView;
import android.util.Log;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.MotionEvent;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;
import androidx.annotation.Keep;

import com.tatfook.paracraft.luabridge.PlatformBridge;

public class ParaEngineWebViewHelper {
    public static class JsToAndroid extends Object {
        private native void receive(String filename, String msg);

        @JavascriptInterface
        public void nplActivate(String filename, String msg) {
            this.receive(filename, msg);
        }
    }
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

    /**
     * Create a Bitmap with a close (X) icon: dark circle background + white X.
     */
    private static android.graphics.Bitmap createCloseButtonBitmap(int size) {
        android.graphics.Bitmap bitmap = android.graphics.Bitmap.createBitmap(size, size, android.graphics.Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);

        // Draw circle background
        Paint bgPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        bgPaint.setColor(Color.parseColor("#CC333333"));
        float radius = size / 2f;
        canvas.drawCircle(radius, radius, radius, bgPaint);

        // Draw X mark
        Paint xPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        xPaint.setColor(Color.WHITE);
        xPaint.setStrokeWidth(size * 0.12f);
        xPaint.setStrokeCap(Paint.Cap.ROUND);
        float padding = size * 0.28f;
        canvas.drawLine(padding, padding, size - padding, size - padding, xPaint);
        canvas.drawLine(size - padding, padding, padding, size - padding, xPaint);

        return bitmap;
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

                // Create a close button with circle background and X icon
                final ImageView closeBtn = new ImageView(sActivity);
                int btnSize = (int) (48 * sActivity.getResources().getDisplayMetrics().density + 0.5f);
                int margin = (int) (8 * sActivity.getResources().getDisplayMetrics().density + 0.5f);
                RelativeLayout.LayoutParams closeBtnParams = new RelativeLayout.LayoutParams(btnSize, btnSize);
                closeBtnParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                closeBtnParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                closeBtnParams.topMargin = margin;
                closeBtnParams.rightMargin = margin;
                closeBtn.setLayoutParams(closeBtnParams);

                // Draw a close (X) icon programmatically
                closeBtn.setImageBitmap(createCloseButtonBitmap(btnSize));
                closeBtn.setPadding(0, 0, 0, 0);

                closeBtn.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        closeWebViewByIndex(index);
                    }
                });

                // Make the close button draggable
                closeBtn.setOnTouchListener(new View.OnTouchListener() {
                    private int initialX, initialY;
                    private float initialTouchX, initialTouchY;
                    private boolean isDragging = false;
                    private static final int DRAG_THRESHOLD = 10;

                    @Override
                    public boolean onTouch(View view, MotionEvent event) {
                        switch (event.getAction()) {
                            case MotionEvent.ACTION_DOWN:
                                initialX = view.getLeft();
                                initialY = view.getTop();
                                initialTouchX = event.getRawX();
                                initialTouchY = event.getRawY();
                                isDragging = false;
                                return true;
                            case MotionEvent.ACTION_MOVE:
                                float dx = event.getRawX() - initialTouchX;
                                float dy = event.getRawY() - initialTouchY;
                                if (!isDragging && (Math.abs(dx) > DRAG_THRESHOLD || Math.abs(dy) > DRAG_THRESHOLD)) {
                                    isDragging = true;
                                }
                                if (isDragging) {
                                    RelativeLayout parent = (RelativeLayout) view.getParent();
                                    int newLeft = initialX + (int) dx;
                                    int newTop = initialY + (int) dy;
                                    // Clamp within parent bounds
                                    newLeft = Math.max(0, Math.min(newLeft, parent.getWidth() - view.getWidth()));
                                    newTop = Math.max(0, Math.min(newTop, parent.getHeight() - view.getHeight()));
                                    RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
                                    // Clear alignment rules so manual positioning works
                                    params.removeRule(RelativeLayout.ALIGN_PARENT_TOP);
                                    params.removeRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                                    params.removeRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                                    params.removeRule(RelativeLayout.ALIGN_PARENT_LEFT);
                                    params.leftMargin = newLeft;
                                    params.topMargin = newTop;
                                    view.setLayoutParams(params);
                                }
                                return true;
                            case MotionEvent.ACTION_UP:
                                if (!isDragging) {
                                    view.performClick();
                                }
                                return true;
                        }
                        return false;
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

                webView.getSettings().setTextZoom(100); //设置webview字体大小不随系统改变
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
    public static void setTransparent(final int index, final boolean transparent) {
        sActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ParaEngineWebView webView = webViews.get(index);

                if (webView != null) {
                    webView.setTransparentBackground(transparent);
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

    public static ParaEngineWebView getCurrentWebView() {
        if (isOpenUrlLoaded) {
            return webViews.get(openUrlViewTag);
        }
        if (webViews.size() > 0) {
            return webViews.valueAt(webViews.size() - 1);
        }
        return null;
    }
}

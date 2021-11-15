package com.tatfook.paracraft;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.hardware.usb.UsbManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Keep;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.SurfaceHolder;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

public class ParaEngineGLSurfaceView extends GLSurfaceView {
    // ===========================================================
    // Constants
    // ===========================================================

    private static final String TAG = "ParaEngine";

    // ===========================================================
    // Fields
    // ===========================================================
    private static ParaEngineGLSurfaceView mGLSurfaceView = null;

    private static ParaEngineActivity sActivity = null;

    private final static int HANDLER_OPEN_IME_KEYBOARD = 2;
    private final static int HANDLER_CLOSE_IME_KEYBOARD = 3;

    private ParaEngineEditBox mEditText = null;
    public String lastText = "";
    private static ParaTextInputWrapper sParaTextInputWrapper = null;

    private ParaEngineRenderer mRenderer;

    private boolean mSoftKeyboardShown = false;
    private boolean mMultipleTouchEnabled = true;
    private boolean mIsPressMouseRightKey = false;
    private boolean mIsPressMouseLeftKey = false;
    private boolean mUsbMode = false;
    private int curPointerId = 0;
    private float curPointerX = 0;
    private float curPointerY = 0;
    private long exitTime;

    // TODO Static handler -> Potential leak!
    private static Handler sHandler = null;

    ViewTreeObserver.OnGlobalLayoutListener mGlobalLayoutListener = null;

    private static native void nativeDeleteBackward();
    private static native void nativeOnUnicodeChar(String text);
    private static native void nativeSetSurface(Surface surface);
    //private native void onKeyBack(boolean bDown);
   // private native void onKeyMenu(boolean bDown);

    @Keep
    public boolean isSoftKeyboardShown() {
        return mSoftKeyboardShown;
    }

    @Keep
    public void setSoftKeyboardShown(boolean softKeyboardShown) {
        this.mSoftKeyboardShown = softKeyboardShown;
    }

    @Keep
    public boolean isMultipleTouchEnabled() {
        return mMultipleTouchEnabled;
    }

    @Keep
    public void setMultipleTouchEnabled(boolean multipleTouchEnabled) {
        this.mMultipleTouchEnabled = multipleTouchEnabled;
    }

    public ParaEngineGLSurfaceView(final Context context) {
        super(context);
    }

    public ParaEngineGLSurfaceView(final Context context, final AttributeSet attrs) {
        super(context, attrs);
    }

    protected void initView(ParaEngineActivity activity) {
        sActivity = activity;

        this.setEGLContextClientVersion(2);
        this.setFocusableInTouchMode(true);
         mGLSurfaceView = this;

        sParaTextInputWrapper = new ParaTextInputWrapper(this);

        DisplayMetrics metrics = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        final int screenHeight = metrics.heightPixels;

        sHandler = new Handler(new Handler.Callback() {
            @Override
            public boolean handleMessage(Message msg) {
                final boolean bMoveView = msg.getData().getBoolean("MoveView");

                // convert to android coordinate system
                final int ctrlBottom = msg.getData().getInt("ctrlBottom");

                switch(msg.what) {
                    case HANDLER_OPEN_IME_KEYBOARD:
                        if (mEditText != null) {
                            mEditText.setEnabled(true);

                            if (mEditText.requestFocus()) {
                                mEditText.removeTextChangedListener(sParaTextInputWrapper);
                                mEditText.setText("");
                                mEditText.addTextChangedListener(sParaTextInputWrapper);
                                mEditText.setOnKeyListener(new OnKeyListener() {
                                    @Override
                                    public boolean onKey(View v, int keyCode, KeyEvent event) {
                                        if (keyCode == 67 && event.getAction() == 0) {
                                            onDeleteBackward();
                                        }
                                        return false;
                                    }
                                });
                                InputMethodManager imm = (InputMethodManager)sActivity.getSystemService(Context.INPUT_METHOD_SERVICE);

                                imm.showSoftInput(mEditText, 0);

                                if (bMoveView) {
                                    if (ParaEngineGLSurfaceView.this.mGlobalLayoutListener != null)
                                        ParaEngineGLSurfaceView.this.getViewTreeObserver().removeOnGlobalLayoutListener(mGlobalLayoutListener);

                                    ParaEngineGLSurfaceView.this.mGlobalLayoutListener = new ViewTreeObserver.OnGlobalLayoutListener() {
                                        @Override
                                        public void onGlobalLayout() {
                                            Rect r = new Rect();
                                            ParaEngineGLSurfaceView.this.getWindowVisibleDisplayFrame(r);
                                            int deltaHeight = screenHeight - r.bottom;

                                            if (deltaHeight > 150 && ctrlBottom > r.bottom) {
                                                int offset = ctrlBottom - r.bottom;
                                                ParaEngineGLSurfaceView.this.offsetTopAndBottom(-offset);
                                            }
                                        }
                                    };

                                    ParaEngineGLSurfaceView.this.getViewTreeObserver().addOnGlobalLayoutListener(mGlobalLayoutListener);
                                }
                            } else {
                                lastText = "";
                                mEditText.setEnabled(false);
                            }
                        }
                        break;
                    case HANDLER_CLOSE_IME_KEYBOARD:
                        lastText = "";
                        if (mEditText != null) {
                            mEditText.removeTextChangedListener(sParaTextInputWrapper);
                            InputMethodManager imm = (InputMethodManager)sActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                            imm.hideSoftInputFromWindow(mEditText.getWindowToken(), 0);
                            ParaEngineGLSurfaceView.this.requestFocus();
                            mEditText.setEnabled(false);
                            Log.d(TAG, "HideSoftInput");
                        }
                        break;
                }
                return false;
            }
        });

        SurfaceHolder holder = getHolder();

        holder.addCallback(new SurfaceHolder.Callback2() {
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                nativeSetSurface(holder.getSurface());
            }

            @Override
            public void surfaceRedrawNeeded(SurfaceHolder holder) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });

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

    public static ParaEngineGLSurfaceView getInstance() { return mGLSurfaceView; }

    public void setParaEngineRenderer(final ParaEngineRenderer renderer) {
        this.mRenderer = renderer;
        this.setRenderer(renderer);
    }

    public ParaEngineEditBox getParaEditText() {
        return mEditText;
    }

    public void setParaEditText(ParaEngineEditBox editText) {
        mEditText = editText;
        if (mEditText != null && sParaTextInputWrapper != null) {
            mEditText.setOnEditorActionListener(sParaTextInputWrapper);
            this.requestFocus();
        }
    }

    @Keep
    public static void openIMEKeyboard(boolean bMoveView, int ctrlBottom) {
        Message msg = new Message();
        msg.what = HANDLER_OPEN_IME_KEYBOARD;

        Bundle bundle = new Bundle();
        bundle.putBoolean("MoveView", bMoveView);
        bundle.putInt("ctrlBottom", ctrlBottom);

        msg.setData(bundle);
        sHandler.sendMessage(msg);
    }

    @Keep
    public static void closeIMEKeyboard(boolean bMoveView, int ctrlBottom) {
        Message msg = new Message();
        msg.what = HANDLER_CLOSE_IME_KEYBOARD;

        Bundle bundle = new Bundle();
        bundle.putBoolean("MoveView", bMoveView);
        bundle.putInt("ctrlBottom", ctrlBottom);

        msg.setData(bundle);
        sHandler.sendMessage(msg);
    }

    public void onUnicodeText(final String text) {
        sActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                nativeOnUnicodeChar(text);
            }
        });
    }

    public void onDeleteBackward() {
        sActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                nativeDeleteBackward();
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();
        this.setRenderMode(RENDERMODE_CONTINUOUSLY);

        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                ParaEngineGLSurfaceView.this.mRenderer.handleOnResume();
            }
        });
    }

    @Override
    public void onPause() {
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                ParaEngineGLSurfaceView.this.mRenderer.handleOnPause();
            }
        });

        this.setRenderMode(RENDERMODE_WHEN_DIRTY);
        //super.onPause();
    }


    @Override
    public boolean onTouchEvent(final MotionEvent pMotionEvent) {
        // these data are used in ACTION_MOVE and ACTION_CANCEL
        final int pointerNumber = pMotionEvent.getPointerCount();
        final int[] ids = new int[pointerNumber];
        final float[] xs = new float[pointerNumber];
        final float[] ys = new float[pointerNumber];

        if (mSoftKeyboardShown){
            InputMethodManager imm = (InputMethodManager)this.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            View view = ((Activity)this.getContext()).getCurrentFocus();
            imm.hideSoftInputFromWindow(view.getWindowToken(),0);
            this.requestFocus();
            mSoftKeyboardShown = false;
        }

        for (int i = 0; i < pointerNumber; i++) {
            ids[i] = pMotionEvent.getPointerId(i);
            xs[i] = pMotionEvent.getX(i);
            ys[i] = pMotionEvent.getY(i);
        }

        switch (pMotionEvent.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_POINTER_DOWN:
                final int indexPointerDown = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                if (!mMultipleTouchEnabled && indexPointerDown != 0) {
                    break;
                }

                final int idPointerDown = pMotionEvent.getPointerId(indexPointerDown);
                final float xPointerDown = pMotionEvent.getX(indexPointerDown);
                final float yPointerDown = pMotionEvent.getY(indexPointerDown);

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        ParaEngineGLSurfaceView.this.mRenderer.handleActionDown(idPointerDown, xPointerDown, yPointerDown);
                    }
                });

                break;

            case MotionEvent.ACTION_DOWN:
                if (pMotionEvent.getButtonState() == MotionEvent.BUTTON_PRIMARY) {
                    // there are only one finger on the screen
                    final int idDown = pMotionEvent.getPointerId(0);
                    final float xDown = xs[0];
                    final float yDown = ys[0];

                    if (mUsbMode) {
                        this.queueEvent(new Runnable() {
                            @Override
                            public void run() {
                                ParaEngineGLSurfaceView.this.mRenderer.handleMouseDown(0, idDown, xDown, yDown);
                            }
                        });

                        mIsPressMouseLeftKey = true;
                    } else {
                        this.queueEvent(new Runnable() {
                            @Override
                            public void run() {
                                ParaEngineGLSurfaceView.this.mRenderer.handleActionDown(idDown, xDown, yDown);
                            }
                        });
                    }
                } else if (pMotionEvent.getButtonState() == MotionEvent.BUTTON_SECONDARY) {
                    onMouseRightKeyDown();
                } else if (pMotionEvent.getButtonState() == MotionEvent.BUTTON_TERTIARY) {

                }

                break;
            case MotionEvent.ACTION_MOVE:
                if (mUsbMode) {
                    this.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            ParaEngineGLSurfaceView.this.mRenderer.handleMouseMove(ids, xs, ys);
                        }
                    });
                } else {
                    if (!mMultipleTouchEnabled) {
                        // handle only touch with id == 0
                        for (int i = 0; i < pointerNumber; i++) {
                            if (ids[i] == 0) {
                                final int[] idsMove = new int[]{0};
                                final float[] xsMove = new float[]{xs[i]};
                                final float[] ysMove = new float[]{ys[i]};
                                this.queueEvent(new Runnable() {
                                    @Override
                                    public void run() {
                                        ParaEngineGLSurfaceView.this.mRenderer.handleActionMove(idsMove, xsMove, ysMove);
                                    }
                                });
                                break;
                            }
                        }
                    } else {
                        this.queueEvent(new Runnable() {
                            @Override
                            public void run() {
                                ParaEngineGLSurfaceView.this.mRenderer.handleActionMove(ids, xs, ys);
                            }
                        });
                    }
                }

                break;
            case MotionEvent.ACTION_POINTER_UP:
                final int indexPointUp = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                if (!mMultipleTouchEnabled && indexPointUp != 0) {
                    break;
                }

                final int idPointerUp = pMotionEvent.getPointerId(indexPointUp);
                final float xPointerUp = pMotionEvent.getX(indexPointUp);
                final float yPointerUp = pMotionEvent.getY(indexPointUp);

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        ParaEngineGLSurfaceView.this.mRenderer.handleActionUp(idPointerUp, xPointerUp, yPointerUp);
                    }
                });

                break;
            case MotionEvent.ACTION_UP:
                // there are only one finger on the screen
                final int idUp = pMotionEvent.getPointerId(0);
                final float xUp = xs[0];
                final float yUp = ys[0];

                if (mUsbMode) {
                    if (mIsPressMouseLeftKey) {
                        // left mouse up
                        this.queueEvent(new Runnable() {
                            @Override
                            public void run() {
                                ParaEngineGLSurfaceView.this.mRenderer.handleMouseUp(0, idUp, xUp, yUp);
                            }
                        });

                        mIsPressMouseLeftKey = false;
                    }

                    if (mIsPressMouseRightKey) {
                        // right mouse up
                        onMouseRightKeyUp();
                    }
                } else {
                    this.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            ParaEngineGLSurfaceView.this.mRenderer.handleActionUp(idUp, xUp, yUp);
                        }
                    });
                }

                break;
            case MotionEvent.ACTION_CANCEL:
                if (!mMultipleTouchEnabled) {
                    // handle only touch with id == 0
                    for (int i = 0; i < pointerNumber; i++) {
                        if (ids[i] == 0) {
                            final int[] idsCancel = new int[]{0};
                            final float[] xsCancel = new float[]{xs[i]};
                            final float[] ysCancel = new float[]{ys[i]};
                            this.queueEvent(new Runnable() {
                                @Override
                                public void run() {
                                    ParaEngineGLSurfaceView.this.mRenderer.handleActionCancel(idsCancel, xsCancel, ysCancel);
                                }
                            });
                            break;
                        }
                    }
                } else {
                    this.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            ParaEngineGLSurfaceView.this.mRenderer.handleActionCancel(ids, xs, ys);
                        }
                    });
                }
                break;
        }

        return true;
    }

    /*
     * This function is called before Cocos2dxRenderer.nativeInit(), so the
     * width and height is correct.
     */
    @Override
    protected void onSizeChanged(final int pNewSurfaceWidth, final int pNewSurfaceHeight, final int pOldSurfaceWidth, final int pOldSurfaceHeight) {
        if(!this.isInEditMode()) {
            this.mRenderer.setScreenWidthAndHeight(pNewSurfaceWidth, pNewSurfaceHeight);
        }
    }

    @Override
    public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
        if (mUsbMode && pKeyCode == KeyEvent.KEYCODE_BACK) {
            onMouseRightKeyDown();
            return false;
        } else {
            this.queueEvent(new Runnable() {
                @Override
                public void run() {
                    ParaEngineGLSurfaceView.this.mRenderer.handleKeyDown(pKeyCode);
                }
            });
            return super.onKeyDown(pKeyCode, pKeyEvent);
        }
    }

    @Override
    public boolean onKeyUp(final int pKeyCode, KeyEvent pKeyEvent) {
        if (mUsbMode && pKeyCode == KeyEvent.KEYCODE_BACK) {
            onMouseRightKeyUp();
            return false;
        } else if (pKeyCode == KeyEvent.KEYCODE_BACK) {
            if ((System.currentTimeMillis() - exitTime) > 2000) {
                Toast.makeText(sActivity, "再按一次退出", Toast.LENGTH_LONG).show();
                exitTime = System.currentTimeMillis();
            } else {
                ParaEngineActivity.onExit();
            }

            return super.onKeyUp(pKeyCode, pKeyEvent);
        } else {
             this.queueEvent(new Runnable() {
                 @Override
                 public void run() {
                     ParaEngineGLSurfaceView.this.mRenderer.handleKeyUp(pKeyCode);
                 }
             });

            return super.onKeyUp(pKeyCode, pKeyEvent);
        }
    }

    public void setMousePosition(MotionEvent pMotionEvent) {
        if (!mUsbMode) {
            return;
        }

        if (pMotionEvent.getAction() == MotionEvent.ACTION_SCROLL) {
            if (pMotionEvent.getAxisValue(MotionEvent.AXIS_VSCROLL) < 0.0f) {
                ParaEngineGLSurfaceView.this.mRenderer.handleMouseScroll(-1);
            } else {
                ParaEngineGLSurfaceView.this.mRenderer.handleMouseScroll(1);
            }
        }

        if (mIsPressMouseLeftKey) {
            return;
        }

        final int pointerNumber = pMotionEvent.getPointerCount();
        final int[] ids = new int[pointerNumber];
        final float[] xs = new float[pointerNumber];
        final float[] ys = new float[pointerNumber];

        for (int i = 0; i < pointerNumber; i++) {
            ids[i] = pMotionEvent.getPointerId(i);
            xs[i] = pMotionEvent.getX(i);
            ys[i] = pMotionEvent.getY(i);
        }

        curPointerId = pMotionEvent.getPointerId(0);
        curPointerX = xs[0];
        curPointerY = ys[0];

        this.queueEvent(new Runnable() {
            @Override
            public void run() {
            ParaEngineGLSurfaceView.this.mRenderer.handleMouseMove(ids, xs, ys);
            }
        });
    }

    private void onMouseRightKeyDown() {
        if (!mIsPressMouseRightKey) {
            mIsPressMouseRightKey = true;

            this.queueEvent(new Runnable() {
                @Override
                public void run() {
                    ParaEngineGLSurfaceView.this.mRenderer.handleMouseDown(
                            1,
                            curPointerId,
                            curPointerX,
                            curPointerY
                    );
                }
            });
        }
    }

    private void onMouseRightKeyUp() {
        if (mIsPressMouseRightKey) {
            mIsPressMouseRightKey = false;

            this.queueEvent(new Runnable() {
                @Override
                public void run() {
                    ParaEngineGLSurfaceView.this.mRenderer.handleMouseUp(
                            1,
                            curPointerId,
                            curPointerX,
                            curPointerY
                    );
                }
            });
        }
    }
}

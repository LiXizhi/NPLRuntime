//-----------------------------------------------------------------------------
// ParaEngineGLSurfaceView.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.11.2
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.SurfaceHolder;
import android.view.inputmethod.InputMethodManager;
import androidx.annotation.Keep;

import org.json.JSONException;
import org.json.JSONObject;

public class ParaEngineGLSurfaceView extends GLSurfaceView {
    private static ParaEngineGLSurfaceView mGLSurfaceView = null;
    private static ParaEngineActivity sActivity = null;
    private static ParaTextInputWrapper sParaTextInputWrapper = null;
    private static int mMsgCount = 0;

    private ParaEngineRenderer mRenderer;
    private ParaEngineEditBox mEditText = null;
    public Handler sHandler = null;
    private boolean mSoftKeyboardShown = false;
    private boolean mMultipleTouchEnabled = true;
    private boolean mIsPressMouseRightKey = false;
    private boolean mIsPressMouseLeftKey = false;
    private int curPointerId = 0;
    private float curPointerX = 0;
    private float curPointerY = 0;
    private int mKeyboardHeight = 0;
    private boolean mIsKeyboardOpened = false;
    private int mScreenOffset = 0;
    private boolean mIsOpen = false;
    private boolean mIsMoveView = false;
    private int mCtrlBottom = 0;
    private static native void nativeDeleteBackward();
    private static native void nativePressEnterKey();
    private static native void nativeOnUnicodeChar(String text);
    public static native void nativeOnSetEditBoxText(String text);
    private static native void nativeSetSurface(Surface surface);

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

        sHandler = new Handler(msg -> {
            mMsgCount = mMsgCount - 1;

            if (mEditText == null) {
                return false;
            }

            // String defaultValue = msg.getData().getString("defaultValue");

            // if (defaultValue != null) {
            //     boolean bOpen = msg.getData().getBoolean("bOpen");
            //     int maxLength = msg.getData().getInt("maxLength");
            //     boolean isMultiline = msg.getData().getBoolean("isMultiline");
            //     boolean confirmHold = msg.getData().getBoolean("confirmHold");
            //     String confirmType = msg.getData().getString("confirmType");
            //     String inputType = msg.getData().getString("inputType");

            //     if (bOpen) {
            //         ParaEngineEditBoxActivity.showNative(
            //             defaultValue,
            //             maxLength,
            //             isMultiline,
            //             confirmHold,
            //             confirmType,
            //             inputType
            //         );
            //     } else {
            //         ParaEngineEditBoxActivity.hideNative();
            //     }

            //     return true;
            // }

            mIsOpen = msg.getData().getBoolean("bOpen");
            mIsMoveView = msg.getData().getBoolean("bMoveView");
            mCtrlBottom = msg.getData().getInt("ctrlBottom");
            String curEditText = msg.getData().getString("curEditText").replaceAll("\r\n","\n");
            boolean isGuiEdit = msg.getData().getBoolean("isGuiEdit");
            sParaTextInputWrapper.setIsGuiEdit(isGuiEdit);

            if (mIsOpen) {
                int selStart = msg.getData().getInt("selStart");
                int selEnd = msg.getData().getInt("selEnd");

                if (selEnd <= 0 && curEditText.length() > 0) {
                    selStart = selEnd = Math.max(curEditText.length(), 0);
                }

                sParaTextInputWrapper.onFocus(curEditText, selStart, selEnd);
                mEditText.setEnabled(true);

                if (mIsKeyboardOpened) {
                    if (!isGuiEdit) {
                        if ((screenHeight - mCtrlBottom) < mKeyboardHeight) {
                            int screenOffset = mKeyboardHeight - (screenHeight - mCtrlBottom);

                            if (screenOffset > mScreenOffset) {
                                ParaEngineGLSurfaceView.this.offsetTopAndBottom(-(screenOffset - mScreenOffset));
                            } else {
                                ParaEngineGLSurfaceView.this.offsetTopAndBottom(mScreenOffset - screenOffset);
                            }

                            mScreenOffset = screenOffset;
                        }
                    }
                } else {
                    if (mEditText.requestFocus()) {
                        mEditText.removeTextChangedListener(sParaTextInputWrapper);
                        mEditText.addTextChangedListener(sParaTextInputWrapper);

                        if (!isGuiEdit) {
                            mEditText.setOnKeyListener(sParaTextInputWrapper);
                        }

                        InputMethodManager imm = (InputMethodManager)sActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                        imm.showSoftInput(mEditText, 0);
                    } else {
                        mEditText.setEnabled(false);
                    }
                }
            } else {
                mEditText.setEnabled(false);
                mEditText.removeTextChangedListener(sParaTextInputWrapper);
                mEditText.setText("");

                InputMethodManager imm = (InputMethodManager)sActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mEditText.getWindowToken(), 0);
                mIsKeyboardOpened = false;

                if (Build.VERSION.SDK_INT <= 29) {
                    mScreenOffset = 0;
                } else {
                    ParaEngineGLSurfaceView.this.offsetTopAndBottom(mScreenOffset);
                    mIsKeyboardOpened = false;
                    mIsOpen = false;
                    mScreenOffset = 0;
                }

                ParaEngineGLSurfaceView.this.requestFocus();
            }

            return true;
        });

        ParaEngineGLSurfaceView.this.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            if (!mIsMoveView) {
                return;
            }

            if (mIsOpen) {
                Rect r = new Rect();
                ParaEngineGLSurfaceView.this.getWindowVisibleDisplayFrame(r);
                mKeyboardHeight = screenHeight - r.bottom; // bottom属性的值将等于屏幕高度减去虚拟键盘的高度
                mIsKeyboardOpened = true;

                if ((screenHeight - mCtrlBottom) < mKeyboardHeight) {
                    if (mScreenOffset != 0) {
                        ParaEngineGLSurfaceView.this.offsetTopAndBottom(mScreenOffset);
                    }

                    mScreenOffset = mKeyboardHeight - (screenHeight - mCtrlBottom);
                    ParaEngineGLSurfaceView.this.offsetTopAndBottom(-mScreenOffset);
                }

                if (Build.VERSION.SDK_INT > 29) {
                    if (mKeyboardHeight == 0) {
                        ParaEngineGLSurfaceView.this.offsetTopAndBottom(mScreenOffset);
                        mIsKeyboardOpened = false;
                        mIsOpen = false;
                        mScreenOffset = 0;
                    }
                }
            } else {
                if (Build.VERSION.SDK_INT > 29) {
                    ParaEngineGLSurfaceView.this.offsetTopAndBottom(mScreenOffset);
                } else {
                    ParaEngineGLSurfaceView.this.offsetTopAndBottom(0);
                }

                mIsKeyboardOpened = false;
                mScreenOffset = 0;
            }
        });

        getHolder().addCallback(new SurfaceHolder.Callback2() {
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
    public static void setIMEKeyboardState(boolean bOpen, boolean bMoveView, int ctrlBottom,String jsonEditParams, boolean isGuiEdit) {
        Message msg = new Message();
        Bundle bundle = new Bundle();
        bundle.putBoolean("bOpen", bOpen);
        bundle.putBoolean("bMoveView", bMoveView);
        bundle.putInt("ctrlBottom", ctrlBottom);
        bundle.putBoolean("isGuiEdit", isGuiEdit);

        String curEditText = "";
        int selStart = 0;
        int selEnd = 0;

        try{
            JSONObject obj = new JSONObject(jsonEditParams);
            curEditText = obj.optString("curEditText");
            selStart = obj.optInt("selStart");
            selEnd = obj.optInt("selEnd");
        } catch (JSONException e) {
            e.printStackTrace();
        }

        bundle.putString("curEditText", curEditText);
        bundle.putInt("selStart", selStart);
        bundle.putInt("selEnd", selEnd);
        msg.setData(bundle);

        mMsgCount = mMsgCount + 1;
        mGLSurfaceView.sHandler.sendMessageDelayed(msg, mMsgCount * 100);
    }

    public static void setIMEKeyboardState(boolean bOpen, String defaultValue, int maxLength, boolean isMultiline, boolean confirmHold, String confirmType, String inputType){
        Message msg = new Message();
        Bundle bundle = new Bundle();
        bundle.putBoolean("bOpen", bOpen);

        bundle.putString("defaultValue", defaultValue);
        bundle.putInt("maxLength", maxLength);
        bundle.putBoolean("isMultiline", isMultiline);
        bundle.putBoolean("confirmHold", confirmHold);
        bundle.putString("confirmType", confirmType);
        bundle.putString("inputType", inputType);

        msg.setData(bundle);

        mGLSurfaceView.sHandler.sendMessage(msg);
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

    public void onPressEnterKey() {
        sActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                nativePressEnterKey();
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

        if (mSoftKeyboardShown) {
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
                if (sActivity.getUsbMode()) {
                    if (pMotionEvent.getButtonState() == MotionEvent.BUTTON_PRIMARY) {
                        final int idDown = pMotionEvent.getPointerId(0);
                        final float xDown = xs[0];
                        final float yDown = ys[0];

                        this.queueEvent(new Runnable() {
                            @Override
                            public void run() {
                                ParaEngineGLSurfaceView.this.mRenderer.handleMouseDown(0, idDown, xDown, yDown);
                            }
                        });

                        mIsPressMouseLeftKey = true;
                    } else if (pMotionEvent.getButtonState() == MotionEvent.BUTTON_SECONDARY) {
                        onMouseRightKeyDown();
                    } else if (pMotionEvent.getButtonState() == MotionEvent.BUTTON_TERTIARY) {

                    }
                } else {
                    // there are only one finger on the screen
                    final int idDown = pMotionEvent.getPointerId(0);
                    final float xDown = xs[0];
                    final float yDown = ys[0];

                    this.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            ParaEngineGLSurfaceView.this.mRenderer.handleActionDown(idDown, xDown, yDown);
                        }
                    });
                }

                break;
            case MotionEvent.ACTION_MOVE:
                if (sActivity.getUsbMode()) {
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

                if (sActivity.getUsbMode()) {
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

    @Override
    protected void onSizeChanged(final int pNewSurfaceWidth, final int pNewSurfaceHeight, final int pOldSurfaceWidth, final int pOldSurfaceHeight) {
        if(!this.isInEditMode()) {
            this.mRenderer.setScreenWidthAndHeight(pNewSurfaceWidth, pNewSurfaceHeight);
        }
    }

    @Override
    public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
        if (sActivity.getUsbMode() && pKeyCode == KeyEvent.KEYCODE_BACK) {
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
        if (sActivity.getUsbMode() && pKeyCode == KeyEvent.KEYCODE_BACK) {
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
        if (!sActivity.getUsbMode()) {
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

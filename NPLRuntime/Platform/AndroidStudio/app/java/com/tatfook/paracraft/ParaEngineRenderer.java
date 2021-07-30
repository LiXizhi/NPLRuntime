package com.tatfook.paracraft;


import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class ParaEngineRenderer implements GLSurfaceView.Renderer {
    // ===========================================================
    // Constants
    // ===========================================================

    private final static long NANOSECONDSPERSECOND = 1000000000L;
    private final static long NANOSECONDSPERMICROSECOND = 1000000L;

    // The final animation interval which is used in 'onDrawFrame'
    private static long sAnimationInterval = (long) (1.0f / 60f * ParaEngineRenderer.NANOSECONDSPERSECOND);

    // ===========================================================
    // Fields
    // ===========================================================
    private long mLastTickInNanoSeconds;
    private int mScreenWidth;
    private int mScreenHeight;
    private boolean mNativeInitCompleted = false;

    // ===========================================================
    // Getter & Setter
    // ===========================================================

    public static void setAnimationInterval(float interval) {
        sAnimationInterval = (long) (interval * ParaEngineRenderer.NANOSECONDSPERSECOND);
    }

    public void setScreenWidthAndHeight(final int surfaceWidth, final int surfaceHeight) {
        this.mScreenWidth = surfaceWidth;
        this.mScreenHeight = surfaceHeight;
    }

    @Override
    public void onSurfaceCreated(final GL10 GL10, final EGLConfig EGLConfig) {
        ParaEngineRenderer.nativeInit(this.mScreenWidth, this.mScreenHeight);
        this.mLastTickInNanoSeconds = System.nanoTime();
        mNativeInitCompleted = true;
    }

    @Override
    public void onSurfaceChanged(final GL10 GL10, final int width, final int height) {
        ParaEngineRenderer.nativeOnSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(final GL10 gl) {
        /*
         * No need to use algorithm in default(60 FPS) situation,
         * since onDrawFrame() was called by system 60 times per second by default.
         */

        if (ParaEngineRenderer.sAnimationInterval <= 1.0f / 60f * ParaEngineRenderer.NANOSECONDSPERSECOND) {
            ParaEngineRenderer.nativeRender();
        } else {
            final long now = System.nanoTime();
            final long interval = now - this.mLastTickInNanoSeconds;

            if (interval < ParaEngineRenderer.sAnimationInterval) {
                try {
                    Thread.sleep((ParaEngineRenderer.sAnimationInterval - interval) / ParaEngineRenderer.NANOSECONDSPERMICROSECOND);
                } catch (final Exception e) {
                }
            }

            /*
             * Render time MUST be counted in, or the FPS will slower than appointed.
             */
            this.mLastTickInNanoSeconds = System.nanoTime();
            ParaEngineRenderer.nativeRender();
        }
    }

    private static native void nativeMouseBegin(final int keyType, final int id, final float x, final float y);
    private static native void nativeMouseEnd(final int keyType, final int id, final float x, final float y);
    private static native void nativeMouseMove(final int[] ids, final float[] xs, final float[] ys);
    private static native void nativeMouseScroll(int forward);
    private static native void nativeTouchesBegin(final int id, final float x, final float y);
    private static native void nativeTouchesEnd(final int id, final float x, final float y);
    private static native void nativeTouchesMove(final int[] ids, final float[] xs, final float[] ys);
    private static native void nativeTouchesCancel(final int[] ids, final float[] xs, final float[] ys);
    private static native boolean nativeKeyEvent(final int keyCode,boolean isPressed);
    private static native void nativeRender();
    private static native void nativeInit(final int width, final int height);
    private static native void nativeOnSurfaceChanged(final int width, final int height);
    private static native void nativeOnPause();
    private static native void nativeOnResume();

    public void handleMouseDown(final int keyType, final int id, final float x, final float y) {
        ParaEngineRenderer.nativeMouseBegin(keyType, id, x, y);
    }

    public void handleMouseUp(final int keyType, final int id, final float x, final float y) {
        ParaEngineRenderer.nativeMouseEnd(keyType, id, x, y);
    }

    public void handleMouseMove(final int[] ids, final float[] xs, final float[] ys)
    {
        ParaEngineRenderer.nativeMouseMove(ids, xs, ys);
    }

    public void handleMouseScroll(int forward)
    {
        ParaEngineRenderer.nativeMouseScroll(forward);
    }

    public void handleActionDown(final int id, final float x, final float y) {
        ParaEngineRenderer.nativeTouchesBegin(id, x, y);
    }

    public void handleActionUp(final int id, final float x, final float y) {
        ParaEngineRenderer.nativeTouchesEnd(id, x, y);
    }

    public void handleActionCancel(final int[] ids, final float[] xs, final float[] ys) {
        ParaEngineRenderer.nativeTouchesCancel(ids, xs, ys);
    }

    public void handleActionMove(final int[] ids, final float[] xs, final float[] ys) {
        ParaEngineRenderer.nativeTouchesMove(ids, xs, ys);
    }

    public void handleKeyDown(final int keyCode) {
        ParaEngineRenderer.nativeKeyEvent(keyCode, true);
    }

    public void handleKeyUp(final int keyCode) {
        ParaEngineRenderer.nativeKeyEvent(keyCode, false);
    }

    public void handleOnPause() {
        /**
         * onPause may be invoked before onSurfaceCreated,
         * and engine will be initialized correctly after
         * onSurfaceCreated is invoked. Can not invoke any
         * native method before onSurfaceCreated is invoked
         */
        if (! mNativeInitCompleted)
            return;

        ParaEngineRenderer.nativeOnPause();
    }

    public void handleOnResume() {
        ParaEngineRenderer.nativeOnResume();
    }
}

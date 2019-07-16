package com.tatfook.paracraft;

import android.content.Context;
import android.os.Handler;
import android.util.AttributeSet;
import android.widget.FrameLayout;

public class ResizeLayout extends FrameLayout {
    private  boolean mEnableForceDoLayout = false;

    public ResizeLayout(Context context){
        super(context);
    }

    public ResizeLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setEnableForceDoLayout(boolean flag){
        mEnableForceDoLayout = flag;
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);
        if(mEnableForceDoLayout){
            /*This is a hot-fix for some android devices which don't do layout when the main window
            * is paned. We refresh the layout in 24 frames per seconds.
            * When the editBox is lose focus or when user begin to type, the do layout is disabled.
            */
            final Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    //Do something after 100ms
                    requestLayout();
                    invalidate();
                }
            }, 1000 / 24);

        }

    }

}

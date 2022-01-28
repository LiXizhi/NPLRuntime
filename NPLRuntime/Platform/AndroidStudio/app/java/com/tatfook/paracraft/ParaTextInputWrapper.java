//-----------------------------------------------------------------------------
// ParaTextInputWrapper.java
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;

public class ParaTextInputWrapper implements TextWatcher, TextView.OnEditorActionListener {

    private ParaEngineGLSurfaceView mNativeView;

    public ParaTextInputWrapper(ParaEngineGLSurfaceView nativeView) {
        mNativeView = nativeView;
    }

    private boolean isFullScreenEdit() {
        TextView textField = mNativeView.getParaEditText();
        InputMethodManager imm = (InputMethodManager)textField.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        return imm.isFullscreenMode();
    }

    @Override
    public void afterTextChanged(Editable s) {
        if (this.isFullScreenEdit())
            return;

        String text = s.toString();

        if (text.length() < mNativeView.lastText.length()) {
            mNativeView.onDeleteBackward();
            mNativeView.lastText = text;
            return;
        }

        String repText = "";
        repText = text.replaceAll("^" + mNativeView.lastText, "");

        mNativeView.onUnicodeText(repText);
        mNativeView.lastText = text;
    }

    @Override
    public void beforeTextChanged(CharSequence pCharSequence, int start, int count, int after) {
    }

    @Override
    public void onTextChanged(CharSequence pCharSequence, int start, int before, int count) {
    }

    @Override
    public boolean onEditorAction(TextView textView, int actionID, KeyEvent keyEvent) {
        if (actionID == EditorInfo.IME_ACTION_DONE) {
            mNativeView.requestFocus();
            textView.setEnabled(false);
        }
        return false;
    }
}

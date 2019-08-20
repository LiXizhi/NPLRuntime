package com.tatfook.paracraft;

import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;

public class ParaTextInputWrapper implements TextWatcher, TextView.OnEditorActionListener {

    private ParaEngineNativeView mNativeView;

    public ParaTextInputWrapper(ParaEngineNativeView nativeView) {
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

        TextView view = mNativeView.getParaEditText();
        view.removeTextChangedListener(this);
        view.setText("");
        view.append(ParaEngineEditBox.sPlaceholder);
        view.addTextChangedListener(this);

        if (text.length() < ParaEngineEditBox.sPlaceholder.length()) {
            mNativeView.onDeleteBackward();
        }
        else {
            text = text.substring(ParaEngineEditBox.sPlaceholder.length());
            mNativeView.onUnicodeText(text);
        }
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

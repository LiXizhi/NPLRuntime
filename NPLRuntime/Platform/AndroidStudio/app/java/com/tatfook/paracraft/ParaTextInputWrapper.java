//-----------------------------------------------------------------------------
// ParaTextInputWrapper.java
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.11.2
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;

public class ParaTextInputWrapper implements TextWatcher, TextView.OnEditorActionListener, View.OnKeyListener {
    private ParaEngineGLSurfaceView mNativeView;
    private static native void nativeSetText(String text);
    private static native void nativeSetCaretPosition(int caretPosition);
    private static native int nativeGetCaretPosition();
    private static native String nativeGetText();

    private boolean isGuiEdit = false;
    private boolean hasDeletedInChangeFunc = false;
    private boolean hasDeletedInKeyEvent = false;

    public ParaTextInputWrapper(ParaEngineGLSurfaceView nativeView) {
        mNativeView = nativeView;
    }

    private boolean isFullScreenEdit() {
        TextView textField = mNativeView.getParaEditText();
        InputMethodManager imm = (InputMethodManager)textField.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        return imm.isFullscreenMode();
    }

    public void onFocus() {
        ParaEngineEditBox editText = mNativeView.getParaEditText();
        if(this.isGuiEdit){
            editText.setText(nativeGetText());
            editText.setSelection(nativeGetCaretPosition());
        }
        hasDeletedInKeyEvent = false;
        hasDeletedInChangeFunc = false;
    }

    public void SetIsGuiEdit(boolean isGuiEdit){
        this.isGuiEdit = isGuiEdit;
    }

    @Override
    public void afterTextChanged(Editable s) {
        if (this.isFullScreenEdit())
            return;
        if(this.isGuiEdit){
            ParaEngineEditBox editText = mNativeView.getParaEditText();
            String text = s.toString();

            ParaEngineActivity.getContext().runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    nativeSetText(text);
                    nativeSetCaretPosition(editText.getSelectionEnd());
                }
            });
        }else {
            String text = s.toString();
            hasDeletedInChangeFunc = false;
            if (text.length() < mNativeView.lastText.length()) {
                if(hasDeletedInKeyEvent){
                    hasDeletedInKeyEvent = false;
                    return;
                }
                mNativeView.onDeleteBackward();
                mNativeView.lastText = text;
                hasDeletedInChangeFunc = true;
                return;
            }

            String repText = getAddedString(text,mNativeView.lastText);
            if(repText.isEmpty()||repText.equals("\n")){
                return;
            }
            mNativeView.onUnicodeText(repText);
            mNativeView.lastText = text;
        }
    }

    private String getAddedString(String text,String lastText){
        String repText = "";
        int startIdx = 0;
        int endIdx = text.length();
        for(int i=1;i<=lastText.length();i++){
            if(lastText.substring(0,i).equals(text.substring(0,i))){
                startIdx = i;
            }else{
                break;
            }
        }

        for(int i=0;i<text.length();i++){
            int j = text.length()-1 - i;
            int k = lastText.length()-1 - i;
            if(k<0){
                break;
            }
            if(text.substring(j).equals(lastText.substring(k))){
                endIdx = j;
            }else{
                break;
            }
        }
        if(startIdx<0||endIdx<0||startIdx>endIdx){
            return "";
        }
        repText = text.substring(startIdx,endIdx);
        return repText;
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
        }else if (actionID == EditorInfo.IME_ACTION_NONE){
            if(this.isGuiEdit){
                mNativeView.onUnicodeText("\n");
            }
        }

        return false;
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {

        if(keyCode == KeyEvent.KEYCODE_DEL || keyCode == KeyEvent.KEYCODE_ENTER) {
            if(event.getAction()== KeyEvent.ACTION_UP){
                return true;
            }
            if(keyCode == KeyEvent.KEYCODE_ENTER) {
                if(isGuiEdit){
                    mNativeView.requestFocus();
                    mNativeView.getParaEditText().setEnabled(false);
                    return true;
                }
            }
            Editable text = mNativeView.getParaEditText().getText();
            int selStart = mNativeView.getParaEditText().getSelectionStart();
            int selEnd = mNativeView.getParaEditText().getSelectionEnd();



            hasDeletedInKeyEvent = false;
            if(keyCode == KeyEvent.KEYCODE_DEL){
                if(hasDeletedInChangeFunc){
                    hasDeletedInChangeFunc = false;
                    return true;
                }
                hasDeletedInKeyEvent = true;
            }

            if(keyCode == KeyEvent.KEYCODE_DEL){
                selStart = Math.max(selStart-1,0);
                text.replace(selStart,selEnd,"");
                mNativeView.lastText = text.toString();
                mNativeView.onDeleteBackward();
            }else{
                text.replace(selStart,selEnd,"");
                text.insert(selStart,"\n");
                selStart = selStart + 1;
                mNativeView.lastText = text.toString();
                mNativeView.onPressEnterKey();
            }

            mNativeView.getParaEditText().setText(text);
            mNativeView.getParaEditText().setSelection(selStart,selStart);

            return true;
        }
        return false;
    }
}

//-----------------------------------------------------------------------------
// ParaTextInputWrapper.java
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2023.2.21
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

    private boolean isGuiEdit = false;
    private String lastText = "";

    public ParaTextInputWrapper(ParaEngineGLSurfaceView nativeView) {
        mNativeView = nativeView;
    }

    private boolean isFullScreenEdit() {
        TextView textField = mNativeView.getParaEditText();
        InputMethodManager imm = (InputMethodManager)textField.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        return imm.isFullscreenMode();
    }

    public void onFocus(String lastText, int selStart, int selEnd) {
        ParaEngineEditBox editText = mNativeView.getParaEditText();

        if (this.isGuiEdit) {
            editText.setText(lastText);
            editText.setSelection(selStart, selEnd);
        } else {
            this.lastText = "";
            editText.setText("");
        }
    }

    public void setIsGuiEdit(boolean isGuiEdit){
        this.isGuiEdit = isGuiEdit;
    }

    @Override
    public void afterTextChanged(Editable s) {
        if (this.isFullScreenEdit())
            return;

        if (this.isGuiEdit) {
            ParaEngineEditBox editText = mNativeView.getParaEditText();
            String text = s.toString();

            ParaEngineActivity.getContext().runOnGLThread(new Runnable() {
                @Override
                public void run() {
                    nativeSetText(text);
                    nativeSetCaretPosition(editText.getSelectionEnd());
                }
            });
        } else {
            String text = s.toString();

            if (text.equals("") && this.lastText.equals("")) {
                return;
            }

            if (text.contains("\n")) {
                this.lastText = "";
                mNativeView.getParaEditText().setText("");
                mNativeView.onPressEnterKey();
                return;
            }

            if (this.lastText.equals(text)) {
                return;
            }

            if (this.lastText.length() > text.length()) {
                String sameStr = "";
                String lastDiffStr = "";
                String textFieldDiffStr = "";
                boolean forceDiff = false;

                for (int i = 0;i < this.lastText.length();i++) {
                    char lastTextCharItem = this.lastText.charAt(i);

                    if (i < text.length()) {
                        char textFieldCharItem = text.charAt(i);

                        if (lastTextCharItem == textFieldCharItem && !forceDiff) {
                            sameStr = new StringBuffer(sameStr).append(textFieldCharItem).toString();
                        } else {
                            forceDiff = true;
                            lastDiffStr = new StringBuffer(lastDiffStr).append(lastTextCharItem).toString();
                            textFieldDiffStr = new StringBuffer(textFieldDiffStr).append(textFieldCharItem).toString();
                        }
                    } else {
                        lastDiffStr = new StringBuffer(lastDiffStr).append(lastTextCharItem).toString();
                    }
                }

                for (int i = 0;i < lastDiffStr.length();i++) {
                    mNativeView.onUnicodeText("[#backspace]");
                }

                for (int i = 0;i < textFieldDiffStr.length();i++) {
                    mNativeView.onUnicodeText(String.valueOf(textFieldDiffStr.charAt(i)));
                }
            } else {
                String sameStr = "";
                String lastDiffStr = "";
                String textFieldDiffStr = "";
                boolean forceDiff = false;

                for (int i = 0;i < text.length();i++) {
                    char textFieldCharItem = text.charAt(i);

                    if (i < this.lastText.length()) {
                        char lastTextCharItem = this.lastText.charAt(i);

                        if (lastTextCharItem == textFieldCharItem && !forceDiff) {
                            sameStr = new StringBuffer(sameStr).append(lastTextCharItem).toString();
                        } else {
                            forceDiff = true;
                            lastDiffStr = new StringBuffer(lastDiffStr).append(lastTextCharItem).toString();
                            textFieldDiffStr = new StringBuffer(textFieldDiffStr).append(textFieldCharItem).toString();
                        }
                    } else {
                        textFieldDiffStr = new StringBuffer(textFieldDiffStr).append(textFieldCharItem).toString();
                    }
                }

                for (int i = 0;i < lastDiffStr.length();i++) {
                    mNativeView.onUnicodeText("[#backspace]");
                }

                for (int i = 0;i< textFieldDiffStr.length();i++) {
                    mNativeView.onUnicodeText(String.valueOf(textFieldDiffStr.charAt(i)));
                }
            }

            this.lastText = text;
        }
    }

//    private String getAddedString(String text,String lastText,int sel_start,int sel_end){
//        String repText = "";
//        int startIdx = 0;
//        int endIdx = text.length();
//        for(int i=1;i<=lastText.length();i++){
//            String temp = lastText.substring(0,i);
//            if(temp.equals(text.substring(0,i))){
//                startIdx = i;
//            }else{
//                break;
//            }
//        }
//
//        for(int i=1;i<=text.length();i++){
//            int j = text.length() - i;
//            int k = lastText.length() - i;
//            if(k<0){
//                break;
//            }
//            if(text.substring(j).equals(lastText.substring(k))){
//                endIdx = j;
//            }else{
//                break;
//            }
//        }
//        if(startIdx<0||endIdx<0||startIdx>endIdx){
//            if(text.length()==lastText.length()+1){
//                return text.substring(text.length()-1);
//            }
//            return "";
//        }
//        repText = text.substring(startIdx,endIdx);
//        return repText;
//    }

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
        if (event.getAction() == KeyEvent.ACTION_UP){
           return true;
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_DEL:
                if (this.lastText.length() == 0) {
                    mNativeView.onDeleteBackward();
                    return true;
                } else {
                    return false;
                }
            case KeyEvent.KEYCODE_ENTER:
                if (this.isGuiEdit) {
                    return true;
                } else {
                    this.lastText = "";
                    mNativeView.getParaEditText().setText("");
                }

                mNativeView.onPressEnterKey();
                return true;
        }

        return false;
    }
}

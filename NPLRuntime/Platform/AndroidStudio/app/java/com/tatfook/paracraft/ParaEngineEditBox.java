//-----------------------------------------------------------------------------
// ParaEngineEditBox.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.11.2
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.content.Context;
import android.graphics.Typeface;
import android.text.InputFilter;
import android.text.InputType;
import android.text.method.PasswordTransformationMethod;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;

public class ParaEngineEditBox extends EditText {
    public static String sPlaceholder = "P";

    /**
     * The user is allowed to enter any text, including line breaks.
     */
    private final int kEditBoxInputModeAny = 0;

    /**
     * The user is allowed to enter an e-mail address.
     */
    private final int kEditBoxInputModeEmailAddr = 1;

    /**
     * The user is allowed to enter an integer value.
     */
    private final int kEditBoxInputModeNumeric = 2;

    /**
     * The user is allowed to enter a phone number.
     */
    private final int kEditBoxInputModePhoneNumber = 3;

    /**
     * The user is allowed to enter a URL.
     */
    private final int kEditBoxInputModeUrl = 4;

    /**
     * The user is allowed to enter a real number value. This extends kEditBoxInputModeNumeric by allowing a decimal point.
     */
    private final int kEditBoxInputModeDecimal = 5;

    /**
     * The user is allowed to enter any text, except for line breaks.
     */
    private final int kEditBoxInputModeSingleLine = 6;

    /**
     * Indicates that the text entered is confidential data that should be obscured whenever possible. This implies EDIT_BOX_INPUT_FLAG_SENSITIVE.
     */
    private final int kEditBoxInputFlagPassword = 0;

    /**
     * Indicates that the text entered is sensitive data that the implementation must never store into a dictionary or table for use in predictive, auto-completing, or other accelerated input schemes. A credit card number is an example of sensitive data.
     */
    private final int kEditBoxInputFlagSensitive = 1;

    /**
     * This flag is a hint to the implementation that during text editing, the initial letter of each word should be capitalized.
     */
    private final int kEditBoxInputFlagInitialCapsWord = 2;

    /**
     * This flag is a hint to the implementation that during text editing, the initial letter of each sentence should be capitalized.
     */
    private final int kEditBoxInputFlagInitialCapsSentence = 3;

    /**
     * Capitalize all characters automatically.
     */
    private final int kEditBoxInputFlagInitialCapsAllCharacters = 4;

    /**
     *  Lowercase all characters automatically.
     */
    private final int kEditBoxInputFlagLowercaseAllCharacters = 5;

    private final int kKeyboardReturnTypeDefault = 0;
    private final int kKeyboardReturnTypeDone = 1;
    private final int kKeyboardReturnTypeSend = 2;
    private final int kKeyboardReturnTypeSearch = 3;
    private final int kKeyboardReturnTypeGo = 4;
    private final int kKeyboardReturnTypeNext = 5;

    public static final int kEndActionUnknown = 0;
    public static final int kEndActionNext = 1;
    public static final int kEndActionReturn = 3;

    private static final int kTextHorizontalAlignmentLeft = 0;
    private static final int kTextHorizontalAlignmentCenter = 1;
    private static final int kTextHorizontalAlignmentRight = 2;

    private static final int kTextVerticalAlignmentTop = 0;
    private static final int kTextVerticalAlignmentCenter = 1;
    private static final int kTextVerticalAlignmentBottom = 2;

    private int mInputFlagConstraints;
    private int mInputModeConstraints;
    private  int mMaxLength;

    private Boolean changedTextProgrammatically = false;

    public Boolean getChangedTextProgrammatically() {
        return changedTextProgrammatically;
    }

    public void setChangedTextProgrammatically(Boolean changedTextProgrammatically) {
        this.changedTextProgrammatically = changedTextProgrammatically;
    }

    // OpenGL view scaleX
    private float mScaleX;

    // package private
    int endAction = kEndActionUnknown;

    public ParaEngineEditBox(Context context){
        super(context);
    }

    public float getOpenGLViewScaleX() {
        return mScaleX;
    }

    public void setOpenGLViewScaleX(float mScaleX) {
        this.mScaleX = mScaleX;
    }

    public void setMaxLength(int maxLength) {
        this.mMaxLength = maxLength;

        this.setFilters(new InputFilter[]{ new InputFilter.LengthFilter(this.mMaxLength) });
    }

    public void setMultilineEnabled(boolean flag) {
        if (flag) {
            this.mInputModeConstraints |= InputType.TYPE_TEXT_FLAG_MULTI_LINE;
        } else {
            this.mInputModeConstraints &= (~InputType.TYPE_TEXT_FLAG_MULTI_LINE);
        }
    }

    public void setReturnType(int returnType) {
        switch (returnType) {
            case kKeyboardReturnTypeDefault:
                this.setImeOptions(EditorInfo.IME_ACTION_NONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
            case kKeyboardReturnTypeDone:
                this.setImeOptions(EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
            case kKeyboardReturnTypeSend:
                this.setImeOptions(EditorInfo.IME_ACTION_SEND | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
            case kKeyboardReturnTypeSearch:
                this.setImeOptions(EditorInfo.IME_ACTION_SEARCH | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
            case kKeyboardReturnTypeGo:
                this.setImeOptions(EditorInfo.IME_ACTION_GO | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
            case kKeyboardReturnTypeNext:
                this.setImeOptions(EditorInfo.IME_ACTION_NEXT | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
            default:
                this.setImeOptions(EditorInfo.IME_ACTION_NONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                break;
        }
    }

    public void setInputMode(int inputMode) {
         switch (inputMode) {
            case kEditBoxInputModeAny:
                this.mInputModeConstraints = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE;
                break;
            case kEditBoxInputModeEmailAddr:
                this.mInputModeConstraints = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS;
                break;
            case kEditBoxInputModeNumeric:
                this.mInputModeConstraints = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED;
                break;
            case kEditBoxInputModePhoneNumber:
                this.mInputModeConstraints = InputType.TYPE_CLASS_PHONE;
                break;
            case kEditBoxInputModeUrl:
                this.mInputModeConstraints = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI;
                break;
            case kEditBoxInputModeDecimal:
                this.mInputModeConstraints = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_DECIMAL | InputType.TYPE_NUMBER_FLAG_SIGNED;
                break;
            case kEditBoxInputModeSingleLine:
                this.mInputModeConstraints = InputType.TYPE_CLASS_TEXT;
                break;
            default:
                break;
        }

        this.setInputType(this.mInputModeConstraints | this.mInputFlagConstraints);
    }

    @Override
    public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
        switch (pKeyCode) {
            case KeyEvent.KEYCODE_BACK:
                ParaEngineActivity activity = (ParaEngineActivity)this.getContext();
                // To prevent program from going to background
                activity.getGLSurfaceView().requestFocus();
                return true;
            default:
                return super.onKeyDown(pKeyCode, pKeyEvent);
        }
    }

    public void setInputFlag(int inputFlag) {
        switch (inputFlag) {
            case kEditBoxInputFlagPassword:
                this.mInputFlagConstraints = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD;
                this.setTypeface(Typeface.DEFAULT);
                this.setTransformationMethod(new PasswordTransformationMethod());
                break;
            case kEditBoxInputFlagSensitive:
                this.mInputFlagConstraints = InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
                break;
            case kEditBoxInputFlagInitialCapsWord:
                this.mInputFlagConstraints = InputType.TYPE_TEXT_FLAG_CAP_WORDS;
                break;
            case kEditBoxInputFlagInitialCapsSentence:
                this.mInputFlagConstraints = InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
                break;
            case kEditBoxInputFlagInitialCapsAllCharacters:
                this.mInputFlagConstraints = InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS;
                break;
            case kEditBoxInputFlagLowercaseAllCharacters:
                this.mInputFlagConstraints = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_NORMAL;
                break;
            default:
                this.mInputFlagConstraints = 0;
                break;
        }

        this.setInputType(this.mInputFlagConstraints | this.mInputModeConstraints);
    }
}

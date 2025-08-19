//-----------------------------------------------------------------------------
// Class: KeyboardiOS.mm
// Authors: big
// CreateDate: 2021.06.26
// ModifyDate: 2021.12.13
//-----------------------------------------------------------------------------

#include "KeyboardiOS.h"
#include <unordered_map>
#include "Framework/InputSystem/VirtualKey.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIEdit.h"
#include "ParaEngineSettings.h"
#include "util/StringHelper.h"


@implementation KeyboardiOSController

static KeyboardiOSController *instance = nil;
static int mCtrlBottom = 0;
static BOOL mUpdateViewSizeWhenKeyboardChange = NO;
static ParaTextField *mTextField = nil;
static BOOL isGuiEdit;
static NSString *curEditText;
static NSString *lastText = @"";
static int selStart;
static int selEnd;
static UIView *mContainerView = nil; // Store container view for embedded mode

// 辅助方法：安全获取宿主应用的主视图控制器
+ (UIViewController *)getHostAppRootViewController {
    id hostAppDelegate = [UIApplication sharedApplication].delegate;
    
    if (hostAppDelegate && [hostAppDelegate respondsToSelector:@selector(viewController)]) {
        UIViewController *viewController = [hostAppDelegate performSelector:@selector(viewController)];
        return viewController;
    }
    
    // 如果AppDelegate没有viewController属性，尝试通过window获取
    if (hostAppDelegate && [hostAppDelegate respondsToSelector:@selector(window)]) {
        UIWindow *window = [hostAppDelegate performSelector:@selector(window)];
        if (window && window.rootViewController) {
            return window.rootViewController;
        }
    }
    
    // 最后的fallback：通过keyWindow获取
    UIWindow *keyWindow = [UIApplication sharedApplication].keyWindow;
    if (keyWindow && keyWindow.rootViewController) {
        return keyWindow.rootViewController;
    }
    
    return nil;
}

+ (void)InitLanguage
{
    NSArray *appLangs = [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleLanguages"];
    NSString *langName = [appLangs objectAtIndex:0];
    
    // get the current language code.(such as English is "en", Chinese is "zh" and so on)
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:langName];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    
    ParaEngine::LanguageType ret = ParaEngine::LanguageType::ENGLISH;
    if ([languageCode isEqualToString:@"zh"]) {
        ret = ParaEngine::LanguageType::CHINESE;
    } else if ([languageCode isEqualToString:@"en"]) {
        ret = ParaEngine::LanguageType::ENGLISH;
    } else if ([languageCode isEqualToString:@"fr"]) {
        ret = ParaEngine::LanguageType::FRENCH;
    } else if ([languageCode isEqualToString:@"it"]) {
        ret = ParaEngine::LanguageType::ITALIAN;
    } else if ([languageCode isEqualToString:@"de"]) {
        ret = ParaEngine::LanguageType::GERMAN;
    } else if ([languageCode isEqualToString:@"es"]) {
        ret = ParaEngine::LanguageType::SPANISH;
    } else if ([languageCode isEqualToString:@"ru"]) {
        ret = ParaEngine::LanguageType::RUSSIAN;
    } else if ([languageCode isEqualToString:@"nl"]) {
        ret = ParaEngine::LanguageType::DUTCH;
    } else if ([languageCode isEqualToString:@"ko"]) {
        ret = ParaEngine::LanguageType::KOREAN;
    } else if ([languageCode isEqualToString:@"ja"]) {
        ret = ParaEngine::LanguageType::JAPANESE;
    } else if ([languageCode isEqualToString:@"hu"]) {
        ret = ParaEngine::LanguageType::HUNGARIAN;
    } else if ([languageCode isEqualToString:@"pt"]) {
        ret = ParaEngine::LanguageType::PORTUGUESE;
    } else if ([languageCode isEqualToString:@"ar"]) {
        ret = ParaEngine::LanguageType::ARABIC;
    } else if ([languageCode isEqualToString:@"nb"]) {
        ret = ParaEngine::LanguageType::NORWEGIAN;
    } else if ([languageCode isEqualToString:@"pl"]) {
        ret = ParaEngine::LanguageType::POLISH;
    } else if ([languageCode isEqualToString:@"tr"]) {
        ret = ParaEngine::LanguageType::TURKISH;
    } else if ([languageCode isEqualToString:@"uk"]) {
        ret = ParaEngine::LanguageType::UKRAINIAN;
    } else if ([languageCode isEqualToString:@"ro"]) {
        ret = ParaEngine::LanguageType::ROMANIAN;
    } else if ([languageCode isEqualToString:@"bg"]) {
        ret = ParaEngine::LanguageType::BULGARIAN;
    }

    ParaEngine::ParaEngineSettings::GetSingleton().SetCurrentLanguage(ret);
}

+ (void)keyboardInit:(AppDelegate *)appDelegate
{
    if (instance != nil) {
        return;
    }

    UIView *targetView = nil;
    if (appDelegate && appDelegate.viewController) {
        targetView = appDelegate.viewController.view;
    }
    [self keyboardInitCommon:targetView];
    instance.appDelegate = appDelegate; // keep for legacy code requiring delegate based layout adjustments
}

+ (void)keyboardInitWithView:(UIView *)containerView
{
    if (instance != nil) {
        return;
    }
    [self keyboardInitCommon:containerView];
}

+ (void)keyboardInitCommon:(UIView *)targetView
{
    if (instance != nil) {
        return;
    }

    instance = [KeyboardiOSController alloc];
    mContainerView = targetView; // Store container view reference

    // Use a minimal 1x1 transparent text field. A hidden view (hidden=YES) will NOT become first responder,
    // which prevents the keyboard from appearing in some host apps.
    mTextField = [[ParaTextField alloc] initWithFrame:CGRectMake(0, 0, 1, 1)];
    mTextField.backgroundColor = [UIColor clearColor];
    mTextField.textColor = [UIColor clearColor];
    mTextField.tintColor = [UIColor clearColor];
    mTextField.delegate = instance;
    mTextField.keyboardType = UIKeyboardTypeDefault;
    mTextField.returnKeyType = UIReturnKeyDefault;
    mTextField.userInteractionEnabled = NO;
    mTextField.autocorrectionType = UITextAutocorrectionTypeNo;
    mTextField.spellCheckingType = UITextSpellCheckingTypeNo;

    if (targetView) {
        // ensure on main thread (host may call from non-main thread during startup)
        dispatch_async(dispatch_get_main_queue(), ^{
            [targetView addSubview:mTextField];
        });
    } else {
        NSLog(@"[KeyboardiOS] Warning: targetView is nil. Keyboard input will not work until keyboardInitWithView: is called again with a valid view.");
    }

    [[NSNotificationCenter defaultCenter]
        addObserver:instance
        selector:@selector(keyboardPressed:)
        name:UITextFieldTextDidChangeNotification object:nil];

    [[NSNotificationCenter defaultCenter]
        addObserver:instance
        selector:@selector(keyBoardWillShow:)
        name:UIKeyboardWillShowNotification
        object:nil];

    [[NSNotificationCenter defaultCenter]
        addObserver:instance
        selector:@selector(keyBoardWillHide:)
        name:UIKeyboardWillHideNotification
        object:nil];
}

+ (BOOL)getIsGuiEdit
{
    return isGuiEdit;
}

+ (ParaTextField *)GetTextField
{
    return mTextField;
}

+ (void)setIMEKeyboardState:(BOOL)bOpen bMoveView:(BOOL)bMoveView ctrlBottom:(int)ctrlBottom editParams:(NSString *)editParams inputType:(NSString *)inputType;
{
    if (mTextField == nil) {
        return;
    }

    isGuiEdit = NO;
    ParaEngine::CGUIEditBox *pGUI = dynamic_cast<ParaEngine::CGUIEditBox*>((ParaEngine::CGUIRoot::GetInstance()->GetUIKeyFocus()));

    if (pGUI != NULL) {
        isGuiEdit = YES;
    }

    NSData *editParamsData = [editParams dataUsingEncoding:NSUTF8StringEncoding];
    id editParamsJson = [NSJSONSerialization JSONObjectWithData:editParamsData options:NSJSONReadingAllowFragments error:nil];
    NSDictionary *editParamsDictionary;

    if ([editParamsJson isKindOfClass:[NSDictionary class]]) {
        editParamsDictionary = (NSDictionary *)editParamsJson;
    }

    curEditText = [editParamsDictionary valueForKey: @"curEditText"];
    selStart = (int)[editParamsDictionary[@"selStart"] integerValue];
    selEnd = (int)[editParamsDictionary[@"selEnd"] integerValue];
    NSString *_inputType = [editParamsDictionary valueForKey:@"inputType"];

    if (_inputType && ![_inputType isEqualToString:@""]) {
        inputType = _inputType;
    }

    mUpdateViewSizeWhenKeyboardChange = bMoveView;
    mCtrlBottom = ctrlBottom;

    if (bOpen) {
        [mTextField becomeFirstResponder];
        
        if ([inputType isEqualToString:@"text"]) {
            mTextField.secureTextEntry = NO;
            mTextField.keyboardType = UIKeyboardTypeDefault;
        } else if ([inputType isEqualToString:@"password"]) {
            mTextField.secureTextEntry = YES;
            mTextField.keyboardType = UIKeyboardTypeDefault;
        } else if ([inputType isEqualToString:@"phone"]) {
            mTextField.secureTextEntry = NO;
            mTextField.keyboardType = UIKeyboardTypePhonePad;
        } else if ([inputType isEqualToString:@"number"]) {
            mTextField.secureTextEntry = NO;
            mTextField.keyboardType = UIKeyboardTypeNumbersAndPunctuation;
        } else if ([inputType isEqualToString:@"email"]) {
            mTextField.secureTextEntry = NO;
            mTextField.keyboardType = UIKeyboardTypeEmailAddress;
        } else if ([inputType isEqualToString:@"url"]) {
            mTextField.secureTextEntry = NO;
            mTextField.keyboardType = UIKeyboardTypeURL;
        } else {
            mTextField.secureTextEntry = NO;
            mTextField.keyboardType = UIKeyboardTypeDefault;
        }
        
        [NSTimer scheduledTimerWithTimeInterval:0.01 target:instance selector:@selector(keyboardOnPressed) userInfo:nil repeats:NO];
    }

    if (bOpen && mTextField.userInteractionEnabled == NO) {
        mTextField.enablesReturnKeyAutomatically = NO;
        mTextField.clearsOnBeginEditing = YES;
        mTextField.userInteractionEnabled = YES;
        [mTextField becomeFirstResponder];
    } else if (!bOpen && mTextField.userInteractionEnabled == YES) {
        mTextField.userInteractionEnabled = NO;
        [mTextField resignFirstResponder];
    }
}

- (void)keyBoardWillShow:(NSNotification*)notification
{
    NSLog(@"[KeyboardiOS] Keyboard will show - view adjustment disabled");

    if (mUpdateViewSizeWhenKeyboardChange)
    {
        NSDictionary *userInfo = [notification userInfo];
        CGSize keyboardSize = [[userInfo objectForKey:UIKeyboardBoundsUserInfoKey] CGRectValue].size;

        UIInterfaceOrientation ori = [UIApplication sharedApplication].statusBarOrientation;
        CGFloat keyboardHeight = UIInterfaceOrientationIsLandscape(ori) ? keyboardSize.height : keyboardSize.width;

        CGRect currentFrame = mContainerView.frame; // _appDelegate.viewController.view.frame;

        _isKeyboardOpened = true;
        _keyboardHeight = keyboardHeight;

        if ((currentFrame.size.height - mCtrlBottom) < keyboardHeight) {
            CGFloat glViewOffset = keyboardHeight - (currentFrame.size.height - mCtrlBottom);
            
            // 使用更安全的方式获取根视图控制器
            UIViewController *rootViewController = [KeyboardiOSController getHostAppRootViewController];
            if (rootViewController && rootViewController.view) {
                // 调整宿主应用视图框架
                rootViewController.view.frame = CGRectMake(0, -glViewOffset, currentFrame.size.width, currentFrame.size.height);
                NSLog(@"[KeyboardiOS] Adjusted host app view frame by offset: %.1f", glViewOffset);
            } else {
                NSLog(@"[KeyboardiOS] Unable to find host app root view controller");
            }
        }
    }
    
    // NSLog(@"[KeyboardiOS] Keyboard height: %.1f", keyboardHeight);
}

- (void)keyBoardWillHide:(NSNotification*)notification
{
    NSLog(@"[KeyboardiOS] Keyboard will hide - view adjustment disabled");
    
    mTextField.text = @"";
    // _isKeyboardOpened = false;

    if (mUpdateViewSizeWhenKeyboardChange)
    {
        CGRect currentFrame = mContainerView.frame;
        
        // 使用更安全的方式获取根视图控制器
        UIViewController *rootViewController = [KeyboardiOSController getHostAppRootViewController];
        if (rootViewController && rootViewController.view) {
            // 恢复宿主应用视图框架
            rootViewController.view.frame = CGRectMake(0, 0, currentFrame.size.width, currentFrame.size.height);
            NSLog(@"[KeyboardiOS] Restored host app view frame");
        } else {
            NSLog(@"[KeyboardiOS] Unable to find host app root view controller");
        }
    }
}

- (void)keyboardOnPressed
{
    ParaEngine::CGUIBase *pGUI = ParaEngine::CGUIRoot::GetInstance()->GetUIKeyFocus();
    
    if (pGUI == NULL) {
        return;
    }

    if (!isGuiEdit) {
        if (selEnd <= 0 && [curEditText length] > 0) {
            selStart = selEnd = (int)MAX([curEditText length], 0);
        }

        mTextField.text = @"";
        lastText = @"";

        // View adjustment logic removed - keyboard will show without view movement
        NSLog(@"[KeyboardiOS] Keyboard input ready - view adjustment disabled");
        if (self.isKeyboardOpened) {
            // 使用更安全的方式获取根视图控制器
            UIViewController *rootViewController = [KeyboardiOSController getHostAppRootViewController];
            if (rootViewController && rootViewController.view) {
                CGRect currentFrame = rootViewController.view.frame;
                
                if ((currentFrame.size.height - mCtrlBottom) < self.keyboardHeight) {
                    CGFloat glViewOffset = self.keyboardHeight - (currentFrame.size.height - mCtrlBottom);
                    // 调整宿主应用视图框架
                    rootViewController.view.frame = CGRectMake(0, -glViewOffset, currentFrame.size.width, currentFrame.size.height);
                    NSLog(@"[KeyboardiOS] Adjusted host app view frame by offset: %.1f", glViewOffset);
                }
            } else {
                NSLog(@"[KeyboardiOS] Unable to find host app root view controller");
            }
        }
    } else {
        NSInteger curCaretPosition = pGUI->GetCaretPosition();
        
        std::string _curText;
        pGUI->GetTextA(_curText);

        NSString *curText = [NSString stringWithUTF8String:_curText.c_str()];
        
        mTextField.text = curText;
        
        UITextPosition *newPos = [mTextField positionFromPosition:mTextField.beginningOfDocument offset:curCaretPosition];
        UITextRange *newRange = [mTextField textRangeFromPosition:newPos toPosition:newPos];
        mTextField.selectedTextRange = newRange;
    }
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string;
{
    return YES;
}

- (BOOL)keyboardInputShouldDelete:(UITextField *)textField
{
    return YES;
}

- (void)keyboardPressed:(NSNotification*)notification
{
    if (ParaEngine::CGlobals::GetApp()->GetAppState() == ParaEngine::PEAppState_Ready)
    {
        ParaEngine::CGUIBase *pGUI = ParaEngine::CGUIRoot::GetInstance()->GetUIKeyFocus();
        
        if (pGUI == NULL) {
            [KeyboardiOSController setIMEKeyboardState:NO bMoveView:YES ctrlBottom:0 editParams:@"" inputType:@""];
            return;
        }

        if (isGuiEdit) {
            const char *curTextUTF8 = [mTextField.text UTF8String];
            pGUI->SetTextA(curTextUTF8);
            pGUI->SetModified();

            NSInteger setCaretPosition = [mTextField offsetFromPosition:mTextField.beginningOfDocument toPosition:[mTextField.selectedTextRange start]];

            pGUI->SetCaretPosition((int)setCaretPosition);
        } else {
            auto pGUIIns = ParaEngine::CGUIRoot::GetInstance();

            if ([lastText isEqualToString:mTextField.text]) {
                return;
            }

            // diff
            if ([lastText length] > [mTextField.text length]) {
                NSString *sameStr = @"";
                NSString *lastDiffStr = @"";
                NSString *textFieldDiffStr = @"";
                BOOL forceDiff = NO;

                for (int i = 0;i < [lastText length];i++) {
                    unichar lastTextCharItem = [lastText characterAtIndex:i];

                    if (i < [mTextField.text length]) {
                        unichar textFieldCharItem = [mTextField.text characterAtIndex:i];

                        if (lastTextCharItem == textFieldCharItem && !forceDiff) {
                            sameStr = [sameStr stringByAppendingString:[NSString stringWithCharacters:&textFieldCharItem length:1]];
                        } else {
                            forceDiff = YES;
                            lastDiffStr = [lastDiffStr stringByAppendingString:[NSString stringWithCharacters:&lastTextCharItem length:1]];
                            textFieldDiffStr = [textFieldDiffStr stringByAppendingString:[NSString stringWithCharacters:&textFieldCharItem length:1]];
                        }
                    } else {
                        lastDiffStr = [lastDiffStr stringByAppendingString:[NSString stringWithCharacters:&lastTextCharItem length:1]];
                    }
                }

                for (int i = 0;i < [lastDiffStr length];i++) {
                    std::string str = "[#backspace]";
                    std::wstring w_str = ParaEngine::StringHelper::MultiByteToWideChar(str.c_str(), CP_UTF8);

                    pGUI->OnHandleWinMsgChars(w_str);
                }

                for (int i = 0;i < [textFieldDiffStr length];i++) {
                    std::wstring s;
                    s = (WCHAR)[textFieldDiffStr characterAtIndex:i];
                    unichar charItem = [textFieldDiffStr characterAtIndex:i];

                    pGUI->OnHandleWinMsgChars(s);
                }
            } else {
                NSString *sameStr = @"";
                NSString *lastDiffStr = @"";
                NSString *textFieldDiffStr = @"";
                BOOL forceDiff = NO;

                for (int i = 0;i < [mTextField.text length];i++) {
                    unichar textFieldCharItem = [mTextField.text characterAtIndex:i];
 
                    if (i < [lastText length]) {
                        unichar lastTextCharItem = [lastText characterAtIndex:i];
                        
                        if (lastTextCharItem == textFieldCharItem && !forceDiff) {
                            sameStr = [sameStr stringByAppendingString:[NSString stringWithCharacters:&lastTextCharItem length:1]];
                        } else {
                            forceDiff = YES;
                            lastDiffStr = [lastDiffStr stringByAppendingString:[NSString stringWithCharacters:&lastTextCharItem length:1]];
                            textFieldDiffStr = [textFieldDiffStr stringByAppendingString:[NSString stringWithCharacters:&textFieldCharItem length:1]];
                        }
                    } else {
                        textFieldDiffStr = [textFieldDiffStr stringByAppendingString:[NSString stringWithCharacters:&textFieldCharItem length:1]];
                    }
                }

                ParaEngine::CGUIBase *pGUI = ParaEngine::CGUIRoot::GetInstance()->GetUIKeyFocus();

                for (int i = 0;i < [lastDiffStr length];i++) {
                    std::string str = "[#backspace]";
                    std::wstring w_str = ParaEngine::StringHelper::MultiByteToWideChar(str.c_str(), CP_UTF8);

                    pGUI->OnHandleWinMsgChars(w_str);
                }

                for (int i = 0;i < [textFieldDiffStr length];i++) {
                    std::wstring s;
                    s = (WCHAR)[textFieldDiffStr characterAtIndex:i];
                    unichar charItem = [textFieldDiffStr characterAtIndex:i];

                    pGUI->OnHandleWinMsgChars(s);
                }
            }

            lastText = mTextField.text;
        }
    }
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if (isGuiEdit) {
        return NO;
    } else {
        mTextField.text = @"";
    }

    if (ParaEngine::CGlobals::GetApp()->GetAppState() == ParaEngine::PEAppState_Ready)
    {
        ParaEngine::CGUIRoot *pGUIRoot = ParaEngine::CGUIRoot::GetInstance();

        if (pGUIRoot)
        {
            pGUIRoot->SendKeyDownEvent(ParaEngine::EVirtualKey::KEY_RETURN);
            pGUIRoot->SendKeyUpEvent(ParaEngine::EVirtualKey::KEY_RETURN);
        }
    }

    return NO;
}

@end


namespace ParaEngine
{
    vector<string> KeyboardiOS::allPressesKey;

    string KeyboardiOS::intToString(int i) {
        char cStr = i;
        char *cStrArr = new char();
        cStrArr[0] = cStr;
        string str = cStrArr;

        return str;
    }

    EVirtualKey KeyboardiOS::toVirtualKey(const string key) {
        static std::unordered_map<string, EVirtualKey> s_keymap;

        if (s_keymap.size() == 0) {
            // [a-z]
            s_keymap[intToString(97)] = EVirtualKey::KEY_A;
            s_keymap[intToString(98)] = EVirtualKey::KEY_B;
            s_keymap[intToString(99)] = EVirtualKey::KEY_C;
            s_keymap[intToString(100)] = EVirtualKey::KEY_D;
            s_keymap[intToString(101)] = EVirtualKey::KEY_E;
            s_keymap[intToString(102)] = EVirtualKey::KEY_F;
            s_keymap[intToString(103)] = EVirtualKey::KEY_G;
            s_keymap[intToString(104)] = EVirtualKey::KEY_H;
            s_keymap[intToString(105)] = EVirtualKey::KEY_I;
            s_keymap[intToString(106)] = EVirtualKey::KEY_J;
            s_keymap[intToString(107)] = EVirtualKey::KEY_K;
            s_keymap[intToString(108)] = EVirtualKey::KEY_L;
            s_keymap[intToString(109)] = EVirtualKey::KEY_M;
            s_keymap[intToString(110)] = EVirtualKey::KEY_N;
            s_keymap[intToString(111)] = EVirtualKey::KEY_O;
            s_keymap[intToString(112)] = EVirtualKey::KEY_P;
            s_keymap[intToString(113)] = EVirtualKey::KEY_Q;
            s_keymap[intToString(114)] = EVirtualKey::KEY_R;
            s_keymap[intToString(115)] = EVirtualKey::KEY_S;
            s_keymap[intToString(116)] = EVirtualKey::KEY_T;
            s_keymap[intToString(117)] = EVirtualKey::KEY_U;
            s_keymap[intToString(118)] = EVirtualKey::KEY_V;
            s_keymap[intToString(119)] = EVirtualKey::KEY_W;
            s_keymap[intToString(120)] = EVirtualKey::KEY_X;
            s_keymap[intToString(121)] = EVirtualKey::KEY_Y;
            s_keymap[intToString(122)] = EVirtualKey::KEY_Z;
            
            // [0-9]
            s_keymap[intToString(48)] = EVirtualKey::KEY_0;
            s_keymap[intToString(49)] = EVirtualKey::KEY_1;
            s_keymap[intToString(50)] = EVirtualKey::KEY_2;
            s_keymap[intToString(51)] = EVirtualKey::KEY_3;
            s_keymap[intToString(52)] = EVirtualKey::KEY_4;
            s_keymap[intToString(53)] = EVirtualKey::KEY_5;
            s_keymap[intToString(54)] = EVirtualKey::KEY_6;
            s_keymap[intToString(55)] = EVirtualKey::KEY_7;
            s_keymap[intToString(56)] = EVirtualKey::KEY_8;
            s_keymap[intToString(57)] = EVirtualKey::KEY_9;
            
            // special key
            s_keymap[intToString(32)] = EVirtualKey::KEY_SPACE;
            s_keymap[intToString(13)] = EVirtualKey::KEY_RETURN;
            s_keymap["UIKeyInputUpArrow"] = EVirtualKey::KEY_UP;
            s_keymap["UIKeyInputDownArrow"] = EVirtualKey::KEY_DOWN;
            s_keymap["UIKeyInputLeftArrow"] = EVirtualKey::KEY_LEFT;
            s_keymap["UIKeyInputRightArrow"] = EVirtualKey::KEY_RIGHT;
            s_keymap["UIKeyInputEscape"] = EVirtualKey::KEY_ESCAPE;
            s_keymap["UIKeyInputPageUp"] = EVirtualKey::KEY_PRIOR;
            s_keymap["UIKeyInputPageDown"] = EVirtualKey::KEY_NEXT;
            s_keymap["UIKeyInputHome"] = EVirtualKey::KEY_HOME;
            s_keymap["UIKeyInputEnd"] = EVirtualKey::KEY_END;
            s_keymap["UIKeyInputF1"] = EVirtualKey::KEY_F1;
            s_keymap["UIKeyInputF2"] = EVirtualKey::KEY_F2;
            s_keymap["UIKeyInputF3"] = EVirtualKey::KEY_F3;
            s_keymap["UIKeyInputF4"] = EVirtualKey::KEY_F4;
            s_keymap["UIKeyInputF5"] = EVirtualKey::KEY_F5;
            s_keymap["UIKeyInputF6"] = EVirtualKey::KEY_F6;
            s_keymap["UIKeyInputF7"] = EVirtualKey::KEY_F7;
            s_keymap["UIKeyInputF8"] = EVirtualKey::KEY_F8;
            s_keymap["UIKeyInputF9"] = EVirtualKey::KEY_F9;
            s_keymap["UIKeyInputF10"] = EVirtualKey::KEY_F10;
            s_keymap["UIKeyInputF11"] = EVirtualKey::KEY_F11;
            s_keymap["UIKeyInputF12"] = EVirtualKey::KEY_F12;
            s_keymap["FlagKeyInputControl"] = EVirtualKey::KEY_LCONTROL;
            s_keymap["FlagKeyInputCommand"] = EVirtualKey::KEY_LCONTROL;
            s_keymap["FlagKeyInputAlt"] = EVirtualKey::KEY_LMENU;
            s_keymap["FlagKeyInputShift"] = EVirtualKey::KEY_LSHIFT;
        }
        
        std::unordered_map<string, EVirtualKey>::iterator ret = s_keymap.find(key);

        if (ret != s_keymap.end()) {
            return ret->second;
        } else {
            return EVirtualKey::KEY_UNKNOWN;
        }
    }

    void KeyboardiOS::OnKeyDown(const string key)
    {
        if (IsKeyPress(key)) {
            return;
        }

        CGUIRoot::GetInstance()->SendKeyDownEvent(toVirtualKey(key));

        allPressesKey.push_back(key);
    }

    void KeyboardiOS::OnKeyUp()
    {
        if (allPressesKey.size() > 0) {
            CGUIRoot::GetInstance()->SendKeyUpEvent(toVirtualKey(allPressesKey.back()));

            allPressesKey.pop_back();
        }
    }

    bool KeyboardiOS::IsKeyPress(const string key)
    {
        for (int i = 0; i < allPressesKey.size(); i++) {
            if (allPressesKey[i] == key) {
                return true;
            }
        }

        return false;
    }
}

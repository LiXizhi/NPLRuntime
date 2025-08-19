//-----------------------------------------------------------------------------
// Class: KeyboardiOS.h
// Authors: big
// CreateDate: 2021.06.26
// ModifyDate: 2021.12.13
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UITextField.h>
// AppDelegate was previously required for keyboard init. Keep optional to avoid breaking
// existing internal code, but new embedding API can initialize with a container UIView directly.
#import "AppDelegate.h"
#import "ParaTextField.h"

#include <stdio.h>
#include "ParaEngine.h"
#include "Framework/InputSystem/VirtualKey.h"

namespace ParaEngine
{
    class KeyboardiOS
    {
        public:
            static string intToString(int i);
            static EVirtualKey toVirtualKey(const string key);
            static void OnChar(const string key);
            static void OnKeyDown(const string key);
            static void OnKeyUp();
            static bool IsKeyPress(const string key);

            static vector<string> allPressesKey;
    };
}

@class KeyboardiOSController;

@interface KeyboardiOSController : UITextField <UITextFieldDelegate>

@property(nonatomic) AppDelegate *appDelegate;
@property(nonatomic) CGFloat keyboardHeight;
@property(nonatomic) BOOL isKeyboardOpened;

+ (void)InitLanguage;

+ (void)keyboardInit:(AppDelegate *)appDelegate;

// New: initialize keyboard handling with a container view instead of AppDelegate
// Use this when embedding framework into an external host app whose AppDelegate
// does not expose the original viewController API.
+ (void)keyboardInitWithView:(UIView *)containerView;

+ (BOOL)getIsGuiEdit;

+ (ParaTextField *)GetTextField;

+ (void)setIMEKeyboardState:(BOOL)bOpen bMoveView:(BOOL)bMoveView ctrlBottom:(int)ctrlBottom editParams:(NSString *)editParams inputType:(NSString *)inputType;

@end

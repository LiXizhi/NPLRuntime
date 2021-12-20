//-----------------------------------------------------------------------------
// Class: KeyboardiOS.h
// Authors: big
// CreateDate: 2021.06.26
// ModifyDate: 2021.12.13
//-----------------------------------------------------------------------------

//#ifndef KeyboardiOS_h
//#define KeyboardiOS_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "AppDelegate.h"

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

@interface KeyboardiOSController : UIResponder <UITextFieldDelegate>

@property(nonatomic) AppDelegate *appDelegate;

+ (void)InitLanguage;

+ (void)keyboardInit:(AppDelegate *)appDelegate;

+ (void)setIMEKeyboardState:(BOOL)bOpen bMoveView:(BOOL)bMoveView ctrlBottom:(int)ctrlBottom;

@end

//#endif /* KeyboardiOS_h */

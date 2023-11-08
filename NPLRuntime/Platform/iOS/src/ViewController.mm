//-----------------------------------------------------------------------------
// Class: ViewController.m
// Authors: 袁全伟, big
// CreateDate: 2021.7.1
// ModifyDate: 2022.1.5
//-----------------------------------------------------------------------------

#import "ViewController.h"
#import "GLView.h"
#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
#include "KeyboardiOS.h"

using namespace ParaEngine;

@implementation ViewController

- (void)addObject: (NSMutableArray *)keys Str:(NSString *)s
{
    [keys addObject: [UIKeyCommand keyCommandWithInput:s
                                   modifierFlags:0
                                   action:@selector(keyAction:)
     ]
    ];

    [keys addObject: [UIKeyCommand keyCommandWithInput:s
                                   modifierFlags:UIKeyModifierControl
                                   action:@selector(keyAction:)
                     ]
    ];
    
    [keys addObject: [UIKeyCommand keyCommandWithInput:s
                                   modifierFlags:UIKeyModifierCommand
                                   action:@selector(keyAction:)
                     ]
    ];
    
    [keys addObject: [UIKeyCommand keyCommandWithInput:s
                                   modifierFlags:UIKeyModifierAlternate
                                   action:@selector(keyAction:)
                     ]
    ];
    
    [keys addObject: [UIKeyCommand keyCommandWithInput:s
                                   modifierFlags:UIKeyModifierShift
                                   action:@selector(keyAction:)
                     ]
    ];
}

- (NSArray<UIKeyCommand *> *)keyCommands
{
    NSMutableArray *keys = [NSMutableArray new];

    ParaEngine::PEAppState appState = CGlobals::GetApp()->GetAppState();

    if (appState == PEAppState_Ready) {
        CGUIBase *pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI == NULL)
        {
            // add [a-z]
            for (int i = 97; i <= 122; i++) {
                [self addObject:keys Str:[NSString stringWithFormat: @"%c", i]];
            }
            
            // add [0-9]
            for (int i = 48; i <= 57; i++) {
                [self addObject:keys Str:[NSString stringWithFormat: @"%c", i]];
            }
            
            // add space
            [self addObject:keys Str:[NSString stringWithFormat: @"%c", 32]];
            
            // add Enter
            [self addObject:keys Str:[NSString stringWithFormat: @"%c", 13]];

            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputUpArrow modifierFlags:0 action:@selector(keyAction:)]];
            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputDownArrow modifierFlags:0 action:@selector(keyAction:)]];
            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputLeftArrow modifierFlags:0 action:@selector(keyAction:)]];
            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputRightArrow modifierFlags:0 action:@selector(keyAction:)]];
            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputEscape modifierFlags:0 action:@selector(keyAction:)]];
            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputPageUp modifierFlags:0 action:@selector(keyAction:)]];
            [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputPageDown modifierFlags:0 action:@selector(keyAction:)]];

            if (@available(iOS 13.4, *)) {
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputHome modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputEnd modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF1 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF2 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF3 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF4 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF5 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF6 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF7 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF8 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF9 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF10 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF11 modifierFlags:0 action:@selector(keyAction:)]];
                [keys addObject:[UIKeyCommand keyCommandWithInput:UIKeyInputF12 modifierFlags:0 action:@selector(keyAction:)]];
            }
        }
    }

    return keys;
}

- (void)keyAction: (UIKeyCommand *)keyCommand
{
    KeyboardiOS::OnKeyDown([keyCommand.input UTF8String]);
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    if (@available(iOS 13.4, *)) {
        string flagKey;
        
        if (event.modifierFlags == 262144) { // control key
            flagKey = "FlagKeyInputControl";
        }
        else if (event.modifierFlags == 1048576) { // command key
            flagKey = "FlagKeyInputCommand";
        }
        else if (event.modifierFlags == 524288) { // alt key
            flagKey = "FlagKeyInputAlt";
        }
        else if (event.modifierFlags == 131072) { // shift key
            flagKey = "FlagKeyInputShift";
        }

        if (flagKey != "" && !KeyboardiOS::IsKeyPress(flagKey)) {
            KeyboardiOS::OnKeyDown(flagKey);
        } else {
            KeyboardiOS::OnKeyUp();
        }

        [super pressesBegan:presses withEvent:event];
    } else {
        [super pressesBegan:presses withEvent:event];
    }
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    KeyboardiOS::OnKeyUp();
}

@end

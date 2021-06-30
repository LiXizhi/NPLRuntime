//
//  ViewController.m
//  ios_gl
//
//  Created by 袁全伟 on 2017/11/11.
//  Copyright © 2017年 XRenderAPI. All rights reserved.
//

#import "ViewController.h"
#import "GLView.h"

using namespace ParaEngine;

@implementation ViewController

- (NSArray<UIKeyCommand *> *)keyCommands
{
    NSMutableArray *keys = [NSMutableArray new];

    ParaEngine::PEAppState appState = CGlobals::GetApp()->GetAppState();

    if (appState == PEAppState_Ready) {
        CGUIBase *pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI == NULL)
        {
            NSLog(@"from key commands.....");

            // add [a-z] [0-9] space
            [keys addObject:[UIKeyCommand keyCommandWithInput:@"w" modifierFlags:0 action:@selector(keyAction:)]];



            // add shift + [a-z] [0-9]

            // add control/command + [a-z] [0-9]

            // add alt + [a-z] [0-9]

//            for (int i = 0; i <= 127; i++) {
//                [keys addObject:
//                  [UIKeyCommand
//                  keyCommandWithInput:[NSString stringWithFormat: @"%c", i]
//                  modifierFlags:0
//                  action:@selector(keyAction:)]
//                ];
//            }

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
    NSLog(@"from key action.....");
    NSLog(@"test var is: %d ", self.pressesStatus);
    KeyboardiOS::OnKey([keyCommand.input UTF8String]);
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    // TODO: handle control, command, shift, alt key
    NSLog(@"press began!!!!! %d \n", event.modifierFlags);
    [super pressesBegan:presses withEvent:event];
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    NSLog(@"press ended!!!!!");
}

@end

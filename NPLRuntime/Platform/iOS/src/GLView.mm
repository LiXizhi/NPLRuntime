//
//  GLView.m
//  ios_gl
//
//  Created by 袁全伟 on 2017/11/12.
//  Copyright © 2017年 XRenderAPI. All rights reserved.
//

#import "GLView.h"
#include "ParaEngine.h"
#include "ParaAppiOS.h"

@interface GLView ()
{

}
@end


@implementation GLView

+(Class) layerClass
{
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    return self;
}

-(void) touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto gui = CGUIRoot::GetInstance();
    if (!gui)return;
    
    int msCurTime = ::GetTickCount();
    NSEnumerator* enumerator = [[event allTouches] objectEnumerator];
    UITouch * touch = nil;
    while (touch = [enumerator nextObject]) {
        intptr_t touchId = (uintptr_t)touch;
        CGPoint location = [ touch locationInView: self ];
        float touchX = location.x* [UIScreen mainScreen].scale;
        float touchY = location.y* [UIScreen mainScreen].scale;
        TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, TouchEvent::TouchEvent_POINTER_DOWN, touchId, touchX, touchY, msCurTime);
        touchId++;
        gui->handleTouchEvent(*touchEvent);
    }
}

-(void) touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto gui = CGUIRoot::GetInstance();
    if (!gui)return;
    
    int msCurTime = ::GetTickCount();
    NSEnumerator* enumerator = [[event allTouches] objectEnumerator];
    UITouch * touch = nil;
    while (touch = [enumerator nextObject]) {
        intptr_t touchId = (uintptr_t)touch;
        CGPoint location = [ touch locationInView: self ];
        float touchX = location.x * [UIScreen mainScreen].scale;
        float touchY = location.y * [UIScreen mainScreen].scale;
        TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, TouchEvent::TouchEvent_POINTER_UP, touchId, touchX, touchY, msCurTime);
        touchId++;
        gui->handleTouchEvent(*touchEvent);
    }
}
-(void) touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto gui = CGUIRoot::GetInstance();
    if (!gui)return;
    
    int msCurTime = ::GetTickCount();
    NSEnumerator* enumerator = [[event allTouches] objectEnumerator];
    UITouch * touch = nil;
    while (touch = [enumerator nextObject]) {
        CGPoint location = [ touch locationInView: self ];
        intptr_t touchId = (uintptr_t)touch;
        float touchX = location.x* [UIScreen mainScreen].scale;
        float touchY = location.y* [UIScreen mainScreen].scale;
        TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, TouchEvent::TouchEvent_POINTER_UPDATE,touchId, touchX, touchY, msCurTime);
        touchId++;
        gui->handleTouchEvent(*touchEvent);
    }
}
-(void) touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto gui = CGUIRoot::GetInstance();
    if (!gui)return;
    
    int msCurTime = ::GetTickCount();
   
    NSEnumerator* enumerator = [[event allTouches] objectEnumerator];
    UITouch * touch = nil;
    while (touch = [enumerator nextObject]) {
        intptr_t touchId = (uintptr_t)touch;
        CGPoint location = [ touch locationInView: self ];
        float touchX = location.x* [UIScreen mainScreen].scale;
        float touchY = location.y* [UIScreen mainScreen].scale;
        TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, TouchEvent::TouchEvent_POINTER_UP, touchId, touchX, touchY, msCurTime);
        gui->handleTouchEvent(*touchEvent);
    }
}

@end

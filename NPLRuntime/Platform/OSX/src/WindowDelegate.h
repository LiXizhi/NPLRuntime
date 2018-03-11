#pragma once
#import <Cocoa/Cocoa.h>
#include "RenderWindowOSX.h"
@interface WindowDelegate : NSObject <NSWindowDelegate>
@property ParaEngine::RenderWindowOSX* renderWindow;

- (id)InitWithRenderWindow:(ParaEngine::RenderWindowOSX*) window;

@end

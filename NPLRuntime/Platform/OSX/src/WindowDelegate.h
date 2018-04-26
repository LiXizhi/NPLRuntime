#pragma once
#import <Cocoa/Cocoa.h>
#import "ConsoleWindowController.h"

#include <string>

namespace ParaEngine {
    class RenderWindowOSX;
    class CParaEngineAppOSX;
}

@interface WindowDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{
    ParaEngine::RenderWindowOSX* _renderWindow;
    ParaEngine::CParaEngineAppOSX* _app;
    
    //console
    ConsoleWindowController* _consoleController;
    
    //console pipe
    NSPipe* _pipe;
    NSFileHandle* _pipeReadHandle;
}

+ (WindowDelegate*) sharedDelegate;
+ (void) closeDelegate;


@end




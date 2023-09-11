#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"
#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"
#include "WindowDelegate.h"

//using namespace ParaEngine;
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        WindowDelegate* winDelegate = [WindowDelegate sharedDelegate];
        [NSApp setDelegate:winDelegate];
        [NSApp run];
        [WindowDelegate closeDelegate];
        
        return 0;
    }
}

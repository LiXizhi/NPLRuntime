#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"
#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"
#include "WindowDelegate.h"

//using namespace ParaEngine;
int main(int argc, const char * argv[]) {

    /*
    RenderWindowOSX renderWindow(1280,720);
    CParaEngineAppOSX app;
    auto url = renderWindow.getUrl();
    
    OUTPUT_LOG("url %s", url.c_str());
    
    bool ret = app.InitApp(&renderWindow, url.c_str());
    if(!ret)
    {
        OUTPUT_LOG("Initialize ParaEngineApp failed.");
        return 1;
    }
    
    return app.Run(0);
     */
    auto pool  = [[NSAutoreleasePool alloc] init];

    [NSApplication sharedApplication];
    WindowDelegate* winDelegate = [WindowDelegate sharedDelegate];
    [NSApp setDelegate:winDelegate];
    [NSApp run];
    [WindowDelegate closeDelegate];
    [pool release];

    return 0;
}

#include "ParaEngine.h"
#include "WindowDelegate.h"
#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"

static WindowDelegate* sInstance = nil;

@implementation WindowDelegate

+ (WindowDelegate*) sharedDelegate
{
    if (!sInstance)
    {
        sInstance = [[self alloc] init];
        [sInstance retain];
    }
    
    return sInstance;
}

+ (void) closeDelegate
{
    if (sInstance)
    {
        [sInstance release];
        sInstance = nullptr;
    }
}

- (void)dealloc
{
    if (_app)
        delete _app;
    if (_renderWindow)
        delete _renderWindow;
}

- (BOOL) windowShouldClose:(NSWindow *)sender
{
    return _renderWindow->OnShouldClose();
}


- (void) windowWillClose:(NSNotification *)notification
{
    [[NSRunningApplication currentApplication] terminate];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}


- (void) handleNotification : (NSNotification *)note
{
    [_pipeReadHandle readInBackgroundAndNotify];
    NSData* data = [[note userInfo] objectForKey:NSFileHandleNotificationDataItem];
    NSString *str = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
    
    if (str)
    {
        [_consoleController trace:str];
    }
    
}

- (void) openConsoleWindow
{
    if (!_consoleController)
    {
        _consoleController = [[ConsoleWindowController alloc] initWithWindowNibName:@"ConsoleWindow"];
    }
    [_consoleController.window orderFrontRegardless];
    
    // set console pipe
    _pipe = [NSPipe pipe];
    _pipeReadHandle = [_pipe fileHandleForReading];
    
    int outfd = [[_pipe fileHandleForWriting] fileDescriptor];
    if (dup2(outfd, fileno(stderr)) != fileno(stderr) || dup2(outfd, fileno(stdout)) != fileno(stdout))
    {
        perror("Unable to redirect output");
    }
    else
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleNotification:) name:NSFileHandleReadCompletionNotification object:_pipeReadHandle];
        [_pipeReadHandle readInBackgroundAndNotify];
    }
}

- (void) startup
{
    //RenderWindowOSX renderWindow(1280,720);
    _renderWindow = new RenderWindowOSX(1280, 720);
    _app = new CParaEngineAppOSX();
    
    //CParaEngineAppOSX app;
    
    bool ret = _app->InitApp(_renderWindow, "");
    if(!ret)
    {
        OUTPUT_LOG("Initialize ParaEngineApp failed.");
        return;
    }
    
    _app->Run(0);
}

- (void) application:(NSApplication *)application openURLs:(nonnull NSArray<NSURL *> *)urls
{
    if (_app)
    {
        for (NSUInteger i = 0; i < [urls count]; i++)
        {
            const char* cmdline = [[urls[i] relativeString] UTF8String];
            _app->onCmdLine(cmdline);
        }
    }

}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    
#ifdef DEBUG
    [self openConsoleWindow];
#endif
    [self startup];
}



@end



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
    
    [super dealloc];
}


/*
- (void) windowWillClose:(NSNotification *)notificationƒurl
{
    [[NSRunningApplication currentApplication] terminate];
}
 

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}
*/

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

- (void) closeConsoleWindow
{
    if (_consoleController)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSFileHandleReadCompletionNotification object:_pipeReadHandle];
        //close([_pipeReadHandle fileDescriptor]);
        _pipeReadHandle = nil;
        _pipe = nil;
        
        [_consoleController close];
        [_consoleController release];
        _consoleController = nil;
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    SAFE_DELETE(_app);
    SAFE_DELETE(_renderWindow);
}

- (BOOL) windowShouldClose:(NSWindow *)sender
{
#ifdef DEBUG
    [self closeConsoleWindow];
#endif
    return _renderWindow->OnShouldClose();
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
    _renderWindow = new RenderWindowOSX(1280, 720);
    
    _renderWindow->PollEvents();

    _app = new CParaEngineAppOSX();
    
    bool ret = _app->InitApp(_renderWindow, _url.c_str());
    if(!ret)
    {
        OUTPUT_LOG("Initialize ParaEngineApp failed.");
        return;
    }
    
    _app->Run(0);
    
    SAFE_DELETE(_app);
    SAFE_DELETE(_renderWindow);
    
    [NSApp terminate:self];
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
    else
    {
        if ([urls count] > 0)
        {
            _url = [[urls[0] relativeString] UTF8String];
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



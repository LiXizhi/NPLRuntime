
#import "AppDelegate.h"
#import "GLView.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    NSInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    
    
    GLView* view = [[GLView alloc ] initWithFrame:CGRectMake(0, 0, 800, 600)];
    
    
    // menu
 
    _mWindow = [[NSWindow alloc] initWithContentRect:CGRectMake(0, 0, 800, 600) styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [_mWindow setTitle:@"Paracraft"];
    //[_mWindow setBackgroundColor:[NSColor blackColor]];
    [_mWindow setContentView:view];
    [_mWindow makeKeyAndOrderFront:nil];
    
    [NSApp setDelegate:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}



- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end

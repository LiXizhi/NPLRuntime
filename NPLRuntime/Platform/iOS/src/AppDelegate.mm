//-----------------------------------------------------------------------------
// Class: AppDelegate.m
// Authors: 袁全伟, big
// CreateDate: 2018.11.02
// ModifyDate: 2021.12.13
//-----------------------------------------------------------------------------

#import "AppDelegate.h"
#import "ViewController.h"
#import "GLView.h"
#import "KeyboardiOS.h"

#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"

using namespace ParaEngine;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    UIScreen* mainScreen = [UIScreen mainScreen];
    CGRect bounds = [mainScreen bounds];

    self.window = [[UIWindow alloc] initWithFrame: bounds];

    GLView* view = [[GLView alloc] initWithFrame: bounds];
    view.multipleTouchEnabled = YES;

    self.viewController = [[ViewController alloc] init];
    self.viewController.view = view;

    if([[UIDevice currentDevice].systemVersion floatValue] < 6.0)
    {
        [self.window addSubview: view];
    }
    else
    {
        [self.window setRootViewController: self.viewController];
    }

    [self.window makeKeyAndVisible];

    [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
    [[UIApplication sharedApplication] setStatusBarHidden: YES];

    RenderWindowiOS* renderWindow = new RenderWindowiOS(view);

    const char* cmdline = "";
    if  (launchOptions)
    {
        NSURL *url = [launchOptions objectForKey:UIApplicationLaunchOptionsURLKey];
        if (url)
        {
            cmdline = [[url relativeString] UTF8String];
        }
    }

    [KeyboardiOSController InitLanguage];

    // Init app
    self.app = new CParaEngineAppiOS();
    self.app->InitApp(renderWindow, cmdline);

    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    self.displayLink.paused = NO;
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    [KeyboardiOSController keyboardInit:self];

    return YES;
}

- (void) update
{
    self.app->DoWork();
}

- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options {
    if (!url)
        return NO;
    
    if (self.app)
    {
        const char* cmdline = [[url relativeString] UTF8String];
        self.app->onCmdLine(cmdline);
        
        return YES;
    }
    else
    {
        return NO;
    }

}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    
    self.app->OnPause();
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
    self.app->OnResume();
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end

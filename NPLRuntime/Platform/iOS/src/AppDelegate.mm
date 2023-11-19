//-----------------------------------------------------------------------------
// Class: AppDelegate.m
// Authors: 袁全伟, big
// CreateDate: 2018.11.02
// ModifyDate: 2023.9.13
//-----------------------------------------------------------------------------

#import "AppDelegate.h"
#import "ViewController.h"
#import "KeyboardiOS.h"
#import "Bluetooth/InterfaceBluetooth/InterfaceBluetooth.h"

#import <CoreTelephony/CTCellularData.h>
#import <CoreTelephony/CTTelephonyNetworkInfo.h>

#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"
#include "2dengine/GUIRoot.h"

using namespace ParaEngine;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    const char* cmdline = "";

    if (launchOptions)
    {
        NSURL *url = [launchOptions objectForKey:UIApplicationLaunchOptionsURLKey];

        if (url)
        {
            cmdline = [[url relativeString] UTF8String];
        }
    }
    
    // Override point for customization after application launch.
    UIScreen* mainScreen = [UIScreen mainScreen];
    CGRect bounds = [mainScreen bounds];

    self.window = [[UIWindow alloc] initWithFrame: bounds];

    self.view = [[GLView alloc] initWithFrame: bounds];
    self.view.multipleTouchEnabled = YES;

    self.viewController = [[ViewController alloc] init];
    self.viewController.view = self.view;

    [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
    [[UIApplication sharedApplication] setStatusBarHidden: YES];

//    if ([[UIDevice currentDevice].systemVersion floatValue] < 16.0) {
//        self.isNoNetToNet = NO;
//        [self fetchProtocolVersionReq];
//        
//        return YES;
//    }

    CTTelephonyNetworkInfo *networkInfo = [[CTTelephonyNetworkInfo alloc] init];

    if ([networkInfo subscriberCellularProvider] == nil) {
        self.isNoNetToNet = NO;
        [self fetchProtocolVersionReq];

        return YES;
    }

    self.isNoNetToNet = NO;
    __block BOOL isStarted = NO;
    CTCellularData *cellularData = [[CTCellularData alloc] init];

    cellularData.cellularDataRestrictionDidUpdateNotifier = ^(CTCellularDataRestrictedState state) {
        switch (state) {
            case kCTCellularDataRestricted:
            {
                self.isNoNetToNet = YES;
                [self performSelectorOnMainThread:@selector(fetchProtocolVersionReq) withObject:nil waitUntilDone:YES];
            }
                break;
            case kCTCellularDataNotRestricted:
            {
                if (isStarted) {
                    return;
                }

                isStarted = YES;

                dispatch_async(dispatch_get_main_queue(), ^{
                    self.isNoNetToNet = NO;
                    [self performSelector:@selector(fetchProtocolVersionReq) withObject:nil afterDelay:self.isNoNetToNet ? 1.0f : 0.0];
                });
            }
                break;
            case kCTCellularDataRestrictedStateUnknown:
            {
                NSLog(@"kCTCellularDataRestrictedStateUnknown");
            }
                break;
            default:
                break;
        }
    };
    
    return YES;
}

- (void)fetchProtocolVersionReq
{
    if (self.isNoNetToNet) {
        UIAlertController *alert =
            [UIAlertController
                alertControllerWithTitle:@"权限需求"
                message:@"Paracraft需要联网权限，否则Paracraft无法正常运作，请点击“无线局域网与蜂窝数据”。如果点击错误请在设置中更改。"
                preferredStyle:UIAlertControllerStyleAlert];

        [alert addAction:
            [UIAlertAction
                actionWithTitle:@"请求权限"
                style:UIAlertActionStyleDefault
                handler:^(UIAlertAction * action){
                    NSString *urlStr = @"http://tmlog.paraengine.com/version.php";
                    NSURL *url = [[NSURL alloc] initWithString:urlStr];
        
                    NSURLRequest *request = [[NSURLRequest alloc] initWithURL:url];
                    [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
                }
            ]];

        UIViewController *viewController = [[UIViewController alloc] init];
        UIScreen* mainScreen = [UIScreen mainScreen];
        CGRect bounds = [mainScreen bounds];

        viewController.view = [[UIView alloc] initWithFrame: bounds];
        [self.window setRootViewController: viewController];
        [self.window makeKeyAndVisible];

        [viewController presentViewController:alert animated:true completion:^{}];

        return;
    }

    [self.window setRootViewController: self.viewController];
    [self.window makeKeyAndVisible];

    RenderWindowiOS *renderWindow = new RenderWindowiOS(self.view);

    [KeyboardiOSController InitLanguage];

    // Init app
    self.app = new CParaEngineAppiOS();
    self.app->InitApp(renderWindow, "");

    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    self.displayLink.paused = NO;
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    [KeyboardiOSController keyboardInit:self];

    CGUIRoot::GetInstance()->SetUIScale(renderWindow->GetScaleX(), renderWindow->GetScaleY(), true, true, false);
}

- (void)update
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

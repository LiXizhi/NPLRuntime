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

- (void)connectWindowScene:(UIWindowScene *)scene {
    self.window = [[UIWindow alloc] initWithWindowScene:scene];
    if (self.viewController) {
        self.window.rootViewController = self.viewController;
        [self.window makeKeyAndVisible];
        return;
    }

    CGRect bounds = self.window.bounds;
    self.view = [[GLView alloc] initWithFrame: bounds];
    self.view.multipleTouchEnabled = YES;

    self.viewController = [[ViewController alloc] init];
    self.viewController.view = self.view;

    [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
    [[UIApplication sharedApplication] setStatusBarHidden: YES];

    CTTelephonyNetworkInfo *networkInfo = [[CTTelephonyNetworkInfo alloc] init];

    if ([networkInfo subscriberCellularProvider] == nil) {
        self.isNoNetToNet = NO;
        [self fetchProtocolVersionReq];

        return;
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

    // Prepare command line for engine initialization
    const char* cmdline = "";
    if (self.pendingCmdLine && [self.pendingCmdLine length] > 0)
    {
        cmdline = [self.pendingCmdLine UTF8String];
    }

    // Init app
    self.app = new CParaEngineAppiOS();
    self.app->InitApp(renderWindow, cmdline);

    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    self.app->SetDisplayLink((__bridge void*)self.displayLink);
    self.displayLink.paused = NO;
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    [KeyboardiOSController keyboardInit:self];

    CGUIRoot::GetInstance()->SetUIScale(renderWindow->GetScaleX(), renderWindow->GetScaleY(), true, true, false);
    
    // Clear pending command line since it's been processed
    if (self.pendingCmdLine)
    {
        self.pendingCmdLine = nil;
    }
}

- (void)update
{
    self.app->DoWork();
}

- (void)openGameURL:(NSURL *)url {
    if (!url)
        return;
    if (self.app)
        self.app->onCmdLine(url.absoluteString.UTF8String);
    else
        self.pendingCmdLine = url.absoluteString;
}

@end

@interface ParacraftSceneDelegate : UIResponder <UIWindowSceneDelegate>
@end

@implementation ParacraftSceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)options {
    AppDelegate *delegate = (AppDelegate *)UIApplication.sharedApplication.delegate;
    [delegate openGameURL:options.URLContexts.anyObject.URL];
    [delegate connectWindowScene:(UIWindowScene *)scene];
}

- (void)scene:(UIScene *)scene openURLContexts:(NSSet<UIOpenURLContext *> *)contexts {
    AppDelegate *delegate = (AppDelegate *)UIApplication.sharedApplication.delegate;
    for (UIOpenURLContext *context in contexts)
        [delegate openGameURL:context.URL];
}

- (void)sceneDidEnterBackground:(UIScene *)scene {
    AppDelegate *delegate = (AppDelegate *)UIApplication.sharedApplication.delegate;
    if (delegate.app)
        delegate.app->OnPause();
}

- (void)sceneWillEnterForeground:(UIScene *)scene {
    AppDelegate *delegate = (AppDelegate *)UIApplication.sharedApplication.delegate;
    if (delegate.app)
        delegate.app->OnResume();
}

@end

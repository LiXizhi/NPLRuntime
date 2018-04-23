//
//  AppDelegate.m
//  ios_gl
//
//  Created by 袁全伟 on 2017/11/11.
//  Copyright © 2017年 XRenderAPI. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "GLView.h"
#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"
using namespace ParaEngine;

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    
    UIScreen* mainScreen = [UIScreen mainScreen];
    id modes = [mainScreen availableModes];
    
    CGRect bounds = [[UIScreen mainScreen]bounds];
    
    self.window = [[UIWindow alloc] initWithFrame: bounds];
    
    GLView* view = [[GLView alloc] initWithFrame:bounds];
    
    self.viewController = [[ViewController alloc ] init];
    self.viewController.view = view;

    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];

    
    
    RenderWindowiOS* renderWindow = new RenderWindowiOS(view);
    
    // Init app
    self.app = new CParaEngineAppiOS();
    self.app->InitApp(renderWindow, "");
    
    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    self.displayLink.paused = NO;
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
    
    return YES;
}

- (void) update
{
    self.app->DoWork();
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end

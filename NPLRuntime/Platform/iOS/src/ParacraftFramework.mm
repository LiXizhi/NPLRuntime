//
//  ParacraftFramework.mm
//  ParacraftFramework
//
//  Framework implementation for Paracraft iOS
//

#import "ParacraftFramework.h"
#import "ParaAppiOS.h"
#import "GLView.h"
#import "RenderWindowiOS.h"
#import "KeyboardiOS.h"
#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#include <string>

// Include necessary ParaEngine headers
#include "2dengine/GUIRoot.h"

// Simple display link handler for embedded mode
@interface SimpleDisplayLinkHandler : NSObject
- (void)update;
@end

@implementation SimpleDisplayLinkHandler
- (void)update {
    extern ParaEngine::CParaEngineAppiOS* g_pParaApp;
    if (g_pParaApp) {
        g_pParaApp->DoWork();
    }
}
@end

// Framework global variables
ParaEngine::CParaEngineAppiOS* g_pParaApp = nullptr;
static std::string g_cmdlineString = "";

// Framework version
double ParacraftFrameworkVersionNumber = 2.022;
const unsigned char ParacraftFrameworkVersionString[] = "2.0.22";

static bool g_bFrameworkInitialized = false;
static UIWindow* g_pFrameworkWindow = nullptr;
static GLView* g_pFrameworkView = nullptr;
// Message callback for external applications
static ParacraftFramework_MessageCallback g_messageCallback = nullptr;
// Removed internal AppDelegate reference; external application's AppDelegate will be used.

int ParacraftFramework_Initialize(void) {
    if (g_bFrameworkInitialized) {
        return 0; // Already initialized
    }
    
    @try {
        // Framework initialization - we'll get the engine instance from app later
        g_bFrameworkInitialized = true;
        return 0;
    } @catch (NSException *exception) {
        NSLog(@"ParacraftFramework_Initialize failed: %@", exception.reason);
        return -1;
    }
}

int ParacraftFramework_Start(void) {
    if (!g_bFrameworkInitialized) {
        return -1;
    }
    
    @try {
        // Check if UIApplication is ready
        UIApplication* app = [UIApplication sharedApplication];
        if (!app) {
            NSLog(@"ParacraftFramework: UIApplication not ready");
            return -1;
        }
        
        // Find the main window and add our GLView to it
        if (!g_pFrameworkView) {
            NSLog(@"ParacraftFramework: Looking for main window in UIApplication");
            
            UIWindow* mainWindow = nil;
            
            // Try to get the main window from the app
            if (@available(iOS 13.0, *)) {
                // iOS 13+ way
                for (UIWindowScene* scene in app.connectedScenes) {
                    if (scene.activationState == UISceneActivationStateForegroundActive) {
                        for (UIWindow* window in scene.windows) {
                            if (window.isKeyWindow) {
                                mainWindow = window;
                                break;
                            }
                        }
                        if (mainWindow) break;
                    }
                }
            }
            
            // Fallback for older iOS or if not found
            if (!mainWindow) {
                mainWindow = app.keyWindow;
            }
            
            // Final fallback
            if (!mainWindow && app.windows.count > 0) {
                mainWindow = app.windows.firstObject;
            }
            
            if (!mainWindow) {
                NSLog(@"ParacraftFramework: No main window found");
                return -1;
            }
            
            NSLog(@"ParacraftFramework: Found main window: %@", mainWindow);
            NSLog(@"Window frame: %@", NSStringFromCGRect(mainWindow.frame));
            NSLog(@"Window rootViewController: %@", mainWindow.rootViewController);
            
            // Get the root view controller's view as container
            UIView* containerView = mainWindow.rootViewController.view;
            if (!containerView) {
                containerView = mainWindow;
            }
            
            NSLog(@"ParacraftFramework: Using container view: %@", containerView);
            NSLog(@"Container frame: %@", NSStringFromCGRect(containerView.frame));
            
            // Create GLView to fit the container
            g_pFrameworkView = [[GLView alloc] initWithFrame:containerView.bounds];
            g_pFrameworkView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            g_pFrameworkView.backgroundColor = [UIColor blackColor];
            
            // Add GLView to container at the bottom layer (index 0)
            [containerView insertSubview:g_pFrameworkView atIndex:0];
            
            // Keep reference to the window
            g_pFrameworkWindow = mainWindow;
            
            NSLog(@"ParacraftFramework: GLView added to container view");
            NSLog(@"GLView frame: %@", NSStringFromCGRect(g_pFrameworkView.frame));
            
            // Initialize render window and engine
            ParaEngine::RenderWindowiOS *renderWindow = new ParaEngine::RenderWindowiOS(g_pFrameworkView);

            [KeyboardiOSController InitLanguage];
            // Initialize hidden textfield keyboard handler with container view instead of internal AppDelegate
            [KeyboardiOSController keyboardInitWithView:containerView];
            
            // Create engine instance
            g_pParaApp = new ParaEngine::CParaEngineAppiOS();
            g_pParaApp->InitApp(renderWindow, g_cmdlineString.c_str());
            
            // Create and start display link
            static SimpleDisplayLinkHandler* s_handler = nil;
            static CADisplayLink* s_displayLink = nil;
            
            s_handler = [[SimpleDisplayLinkHandler alloc] init];
            s_displayLink = [CADisplayLink displayLinkWithTarget:s_handler selector:@selector(update)];
            s_displayLink.paused = NO;
            [s_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            
            // Initialize UI scale
            ParaEngine::CGUIRoot::GetInstance()->SetUIScale(renderWindow->GetScaleX(), renderWindow->GetScaleY(), true, true, false);
            
            NSLog(@"ParacraftFramework: Successfully started and embedded in app's main window");
            
            return 0;
        } else {
            NSLog(@"ParacraftFramework: Framework already started");
            return 0;
        }
    } @catch (NSException *exception) {
        NSLog(@"ParacraftFramework_Start failed: %@", exception.reason);
        // Cleanup on exception
        g_pFrameworkWindow = nil;
        g_pFrameworkView = nil;
        return -1;
    }
}

int ParacraftFramework_StartWithCommandLine(const char* cmdline) {
    // Set command line first
    ParacraftFramework_SetCommandLine(cmdline);
    
    // Then start the framework
    return ParacraftFramework_Start();
}

int ParacraftFramework_StartWhenReady(int maxRetries, double retryInterval) {
    NSLog(@"ParacraftFramework: Starting with retry mechanism, maxRetries=%d, interval=%.1f", maxRetries, retryInterval);
    
    for (int i = 0; i < maxRetries; i++) {
        NSLog(@"ParacraftFramework: Attempt %d/%d", i + 1, maxRetries);
        
        int result = ParacraftFramework_Start();
        if (result == 0) {
            NSLog(@"ParacraftFramework: Successfully started on attempt %d", i + 1);
            return 0;
        }
        
        if (i < maxRetries - 1) {
            NSLog(@"ParacraftFramework: Waiting %.1f seconds before retry...", retryInterval);
            [NSThread sleepForTimeInterval:retryInterval];
        }
    }
    
    NSLog(@"ParacraftFramework: Failed to start after %d attempts", maxRetries);
    return -1;
}

int ParacraftFramework_StartWhenReadyWithCommandLine(const char* cmdline, int maxRetries, double retryInterval) {
    // Set command line first
    ParacraftFramework_SetCommandLine(cmdline);
    
    // Then start with retry mechanism
    return ParacraftFramework_StartWhenReady(maxRetries, retryInterval);
}

void ParacraftFramework_Stop(void) {
    if (!g_bFrameworkInitialized) {
        return;
    }
    
    @try {
        // Clear our references to the shared app components
        // App's DisplayLink will continue running
        g_pFrameworkWindow = nil;
        g_pFrameworkView = nil;
    } @catch (NSException *exception) {
        NSLog(@"ParacraftFramework_Stop failed: %@", exception.reason);
    }
}

void ParacraftFramework_Cleanup(void) {
    // Clear our references to Framework components
    g_pFrameworkWindow = nil;
    g_pFrameworkView = nil;
    // Internal AppDelegate reference removed.
    
    // Clear our reference to the engine (it's owned by AppDelegate)
    g_pParaApp = nullptr;
    
    g_bFrameworkInitialized = false;
}

UIView* ParacraftFramework_GetView(void) {
    return (UIView*)g_pFrameworkView;
}

UIWindow* ParacraftFramework_GetWindow(void) {
    return g_pFrameworkWindow;
}

void ParacraftFramework_SetCommandLine(const char* cmdline) {
    if (cmdline) {
        g_cmdlineString = std::string(cmdline);
        NSLog(@"ParacraftFramework: Command line set to: %s", cmdline);
    } else {
        g_cmdlineString = "";
        NSLog(@"ParacraftFramework: Command line cleared");
    }
}

void ParacraftFramework_DiagnoseAppState(void) {
    NSLog(@"=== ParacraftFramework App State Diagnosis ===");
    
    UIApplication* app = [UIApplication sharedApplication];
    NSLog(@"UIApplication: %@", app ? @"✓ Available" : @"✗ Not available");
    
    if (!app) {
        NSLog(@"=== Diagnosis Complete ===");
        return;
    }
    
    NSLog(@"UIApplication state: %ld", (long)app.applicationState);
    
    // Check external app delegate (for reference only)
    id externalDelegate = [app delegate];
    NSLog(@"External app delegate: %@", externalDelegate ? @"✓ Available" : @"✗ Not available");
    NSLog(@"External app delegate class: %@", externalDelegate ? NSStringFromClass([externalDelegate class]) : @"N/A");
    
    // Check framework components
    NSLog(@"Framework initialized: %@", g_bFrameworkInitialized ? @"✓ Yes" : @"✗ No");
    // Internal Framework AppDelegate removed; relying on host application's AppDelegate.
    NSLog(@"Framework window: %@", g_pFrameworkWindow ? @"✓ Available" : @"✗ Not available");
    NSLog(@"Framework view: %@", g_pFrameworkView ? @"✓ Available" : @"✗ Not available");
    NSLog(@"Framework engine: %@", g_pParaApp ? @"✓ Available" : @"✗ Not available");
    
    if (g_pFrameworkView) {
        NSLog(@"View class: %@", NSStringFromClass([g_pFrameworkView class]));
        NSLog(@"View frame: %@", NSStringFromCGRect(g_pFrameworkView.frame));
    }
    
    if (g_pFrameworkWindow) {
        NSLog(@"Window class: %@", NSStringFromClass([g_pFrameworkWindow class]));
        NSLog(@"Window visible: %@", g_pFrameworkWindow.hidden ? @"No" : @"Yes");
        NSLog(@"Window key: %@", g_pFrameworkWindow.isKeyWindow ? @"Yes" : @"No");
    }
    
    NSLog(@"=== Diagnosis Complete ===");
}

void ParacraftFramework_SetMessageCallback(ParacraftFramework_MessageCallback callback) {
    g_messageCallback = callback;
    if (callback) {
        NSLog(@"ParacraftFramework: Message callback set");
    } else {
        NSLog(@"ParacraftFramework: Message callback cleared");
    }
}

void ParacraftFramework_HandleMessage(const char* message) {
    if (!message) {
        NSLog(@"ParacraftFramework: Received null message");
        return;
    }
    
    // NSLog(@"ParacraftFramework: Received message: %s", message);
    
    // Call the external app's callback if set
    if (g_messageCallback) {
        g_messageCallback(message);
    } else {
        NSLog(@"ParacraftFramework: No message callback set, message ignored");
    }
}

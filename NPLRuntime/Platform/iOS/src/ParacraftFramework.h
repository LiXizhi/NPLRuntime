//
//  ParacraftFramework.h
//  ParacraftFramework
//
//  Framework interface for Paracraft iOS
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

//! Project version number for ParacraftFramework.
FOUNDATION_EXPORT double ParacraftFrameworkVersionNumber;

//! Project version string for ParacraftFramework.
FOUNDATION_EXPORT const unsigned char ParacraftFrameworkVersionString[];

// In this header, you should import all the public headers of your framework
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize Paracraft Framework
 * @return 0 on success, -1 on error
 */
int ParacraftFramework_Initialize(void);

/**
 * Start Paracraft Engine
 * Framework will automatically find the app's main window and embed GLView into it.
 * @return 0 on success, -1 on error
 */
int ParacraftFramework_Start(void);

/**
 * Start Paracraft Engine with delay and retry
 * Waits for app to be fully initialized before starting framework.
 * @param maxRetries Maximum number of retry attempts
 * @param retryInterval Interval between retries in seconds
 * @return 0 on success, -1 on error
 */
int ParacraftFramework_StartWhenReady(int maxRetries, double retryInterval);

/**
 * Stop Paracraft Engine
 */
void ParacraftFramework_Stop(void);

/**
 * Cleanup Paracraft Framework
 */
void ParacraftFramework_Cleanup(void);

/**
 * Get the Framework's UIView (available after Start is called)
 * @return UIView* or nil if not started
 */
UIView* ParacraftFramework_GetView(void);

/**
 * Get the Framework's UIWindow (available after Start is called)
 * @return UIWindow* or nil if not started
 */
UIWindow* ParacraftFramework_GetWindow(void);

/**
 * Diagnostic function to check app readiness
 * Prints detailed information about app delegate state
 */
void ParacraftFramework_DiagnoseAppState(void);

#ifdef __cplusplus
}
#endif

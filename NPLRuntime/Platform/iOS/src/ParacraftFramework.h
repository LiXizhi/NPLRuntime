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

// Message callback function type
typedef void (*ParacraftFramework_MessageCallback)(const char* message);

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
 * Start Paracraft Engine with command line parameters
 * Framework will automatically find the app's main window and embed GLView into it.
 * @param cmdline Command line string to be passed to the engine
 * @return 0 on success, -1 on error
 */
int ParacraftFramework_StartWithCommandLine(const char* cmdline);

/**
 * Start Paracraft Engine with delay and retry
 * Waits for app to be fully initialized before starting framework.
 * @param maxRetries Maximum number of retry attempts
 * @param retryInterval Interval between retries in seconds
 * @return 0 on success, -1 on error
 */
int ParacraftFramework_StartWhenReady(int maxRetries, double retryInterval);

/**
 * Start Paracraft Engine with command line and delay and retry
 * Waits for app to be fully initialized before starting framework.
 * @param cmdline Command line string to be passed to the engine
 * @param maxRetries Maximum number of retry attempts
 * @param retryInterval Interval between retries in seconds
 * @return 0 on success, -1 on error
 */
int ParacraftFramework_StartWhenReadyWithCommandLine(const char* cmdline, int maxRetries, double retryInterval);

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
 * Set command line parameters for Paracraft Engine
 * This should be called before ParacraftFramework_Start()
 * @param cmdline Command line string to be passed to the engine
 */
void ParacraftFramework_SetCommandLine(const char* cmdline);

/**
 * Diagnostic function to check app readiness
 * Prints detailed information about app delegate state
 */
void ParacraftFramework_DiagnoseAppState(void);

/**
 * Set message callback for receiving messages from ParaEngine
 * @param callback Function to be called when a message is received, or NULL to unsubscribe
 */
void ParacraftFramework_SetMessageCallback(ParacraftFramework_MessageCallback callback);

/**
 * Internal function to handle messages from ParaEngine
 * This should not be called directly by external applications
 * @param message The message string received from ParaEngine
 */
void ParacraftFramework_HandleMessage(const char* message);

#ifdef __cplusplus
}
#endif

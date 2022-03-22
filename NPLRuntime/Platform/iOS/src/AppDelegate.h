//-----------------------------------------------------------------------------
// Class: AppDelegate.m
// Authors: 袁全伟, big
// CreateDate: 2018.11.02
// ModifyDate: 2022.1.5
//-----------------------------------------------------------------------------

#import <UIKit/UIKit.h>

namespace ParaEngine {
    class CParaEngineAppiOS;
}

@interface AppDelegate : UIResponder <UIApplicationDelegate, UITextFieldDelegate>
{
}

@property(strong, nonatomic) UIWindow *window;
@property(strong, nonatomic) UIViewController* viewController;
@property(nonatomic) ParaEngine::CParaEngineAppiOS* app;
@property(strong, nonatomic) CADisplayLink* displayLink;

@end


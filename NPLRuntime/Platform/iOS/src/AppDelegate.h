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


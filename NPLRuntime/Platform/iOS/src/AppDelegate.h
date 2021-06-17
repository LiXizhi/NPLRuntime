#import <UIKit/UIKit.h>

namespace ParaEngine {
    class CParaEngineAppiOS;
}

@interface AppDelegate : UIResponder <UIApplicationDelegate, UITextFieldDelegate>
{
@private
    UITextField *mTextField;
    NSString *mLastText;
}

@property (strong,nonatomic)   UIWindow *window;
@property (strong,nonatomic)    UIViewController* viewController;
@property (nonatomic)    ParaEngine::CParaEngineAppiOS* app;
@property (strong,nonatomic) CADisplayLink* displayLink;

@property (nonatomic, retain) UITextField *mTextField;

@property(nonatomic) BOOL mUpdateViewSizeWhenKeyboardChange;
@property(nonatomic) int mCtrlBottom;

- (void)setIMEKeyboardState:(BOOL)bOpen bMoveView:(BOOL)bMoveView ctrlBottom:(int)ctrlBottom;

@end


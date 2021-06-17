//
//  AppDelegate.m
//
//  Created by 袁全伟 on 2017/11/11.
//  Copyright © 2017年 XRenderAPI. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "GLView.h"

#include "ParaAppiOS.h"
#include "RenderWindowiOS.h"
#include "ParaEngineSettings.h"
#include "2dengine/GUIIMEDelegate.h"
#include "2dengine/GUIRoot.h"

using namespace ParaEngine;

@implementation AppDelegate

@synthesize mTextField;

- (void)InitLanguage
{
    NSArray *appLangs = [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleLanguages"];
    NSString *langName = [appLangs objectAtIndex:0];
	
	// get the current language code.(such as English is "en", Chinese is "zh" and so on)
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:langName];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    
	LanguageType ret = LanguageType::ENGLISH;
	if ([languageCode isEqualToString:@"zh"])
	{
		ret = LanguageType::CHINESE;
	}
	else if ([languageCode isEqualToString:@"en"])
	{
		ret = LanguageType::ENGLISH;
	}
	else if ([languageCode isEqualToString:@"fr"])
	{
		ret = LanguageType::FRENCH;
	}
	else if ([languageCode isEqualToString:@"it"])
	{
		ret = LanguageType::ITALIAN;
	}
	else if ([languageCode isEqualToString:@"de"])
	{
		ret = LanguageType::GERMAN;
	}
	else if ([languageCode isEqualToString:@"es"])
	{
		ret = LanguageType::SPANISH;
	}
	else if ([languageCode isEqualToString:@"ru"])
	{
		ret = LanguageType::RUSSIAN;
	}
	else if ([languageCode isEqualToString:@"nl"])
	{
		ret = LanguageType::DUTCH;
	}
	else if ([languageCode isEqualToString:@"ko"])
	{
		ret = LanguageType::KOREAN;
	}
	else if ([languageCode isEqualToString:@"ja"])
	{
		ret = LanguageType::JAPANESE;
	}
	else if ([languageCode isEqualToString:@"hu"])
	{
		ret = LanguageType::HUNGARIAN;
	}
	else if ([languageCode isEqualToString:@"pt"])
	{
		ret = LanguageType::PORTUGUESE;
	}
	else if ([languageCode isEqualToString:@"ar"])
	{
		ret = LanguageType::ARABIC;
	}
	else if ([languageCode isEqualToString:@"nb"])
	{
		ret = LanguageType::NORWEGIAN;
	}
	else if ([languageCode isEqualToString:@"pl"])
	{
		ret = LanguageType::POLISH;
	}
	else if ([languageCode isEqualToString:@"tr"])
	{
		ret = LanguageType::TURKISH;
	}
	else if ([languageCode isEqualToString:@"uk"])
	{
		ret = LanguageType::UKRAINIAN;
	}
	else if ([languageCode isEqualToString:@"ro"])
	{
		ret = LanguageType::ROMANIAN;
	}
	else if ([languageCode isEqualToString:@"bg"])
	{
		ret = LanguageType::BULGARIAN;
	}
	
	ParaEngineSettings::GetSingleton().SetCurrentLanguage(ret);
}

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
    
    [self InitLanguage];

    // Init app
    self.app = new CParaEngineAppiOS();
    self.app->InitApp(renderWindow, cmdline);

    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    self.displayLink.paused = NO;
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    //
    mTextField = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, -10, -10)];
    mTextField.delegate = self;
    mTextField.keyboardType = UIKeyboardTypeDefault;
    mTextField.userInteractionEnabled = NO;
    [view addSubview:mTextField];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardOnPressed:) name:UITextFieldTextDidBeginEditingNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardPressed:) name:UITextFieldTextDidChangeNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyBoardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyBoardWillHide:) name:UIKeyboardWillHideNotification object:nil];

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

- (void)setIMEKeyboardState:(BOOL)bOpen bMoveView:(BOOL)bMoveView ctrlBottom:(int)ctrlBottom;
{
    self.mUpdateViewSizeWhenKeyboardChange = bMoveView;
    self.mCtrlBottom = ctrlBottom;

    if (bOpen && mTextField.userInteractionEnabled == NO)
    {
        mTextField.enablesReturnKeyAutomatically = NO;
        mTextField.clearsOnBeginEditing = YES;
        mTextField.userInteractionEnabled = YES;
        [mTextField becomeFirstResponder];
    }
    else if (!bOpen && mTextField.userInteractionEnabled == YES)
    {
        mTextField.userInteractionEnabled = NO;
        [mTextField resignFirstResponder];
    }
}

- (void)keyBoardWillShow:(NSNotification*)notification
{
    if (self.mUpdateViewSizeWhenKeyboardChange)
    {
        auto userInfo = [notification userInfo];
        auto keyboardSize = [[userInfo objectForKey:UIKeyboardBoundsUserInfoKey] CGRectValue].size;

        auto ori = [UIApplication sharedApplication].statusBarOrientation;
        auto keyboardHeight = UIInterfaceOrientationIsLandscape(ori) ? keyboardSize.height : keyboardSize.width;

        auto currentFrame = self.viewController.view.frame;

        if ((currentFrame.size.height - self.mCtrlBottom) < keyboardHeight)
        {
            auto offset = keyboardHeight - (currentFrame.size.height - self.mCtrlBottom);
            self.viewController.view.frame = CGRectMake(0, -offset, currentFrame.size.width, currentFrame.size.height);
        }
    }
}

- (void)keyBoardWillHide:(NSNotification*)notification
{
    if (self.mUpdateViewSizeWhenKeyboardChange)
    {
        auto currentFrame = self.viewController.view.frame;
        self.viewController.view.frame = CGRectMake(0, 0, currentFrame.size.width, currentFrame.size.height);
    }
}

- (void)keyboardOnPressed: (NSNotification*)notification
{
    mTextField.text = @"-----------------------------------"; // length: 35
    mLastText = @"";
}

- (void)keyboardPressed:(NSNotification*)notification
{
    UITextInputMode *inputMode = [mTextField textInputMode];
    NSString *inputLang = inputMode.primaryLanguage;

    if (CGlobals::GetApp()->GetAppState() == PEAppState_Ready)
    {
        auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
        auto pGUIIns = CGUIRoot::GetInstance();

        if ([mTextField.text length] < 35) {
            pGUIIns->SendKeyDownEvent(EVirtualKey::KEY_BACK);
            pGUIIns->SendKeyUpEvent(EVirtualKey::KEY_BACK);
            mTextField.text = @"-----------------------------------";
            return;
        }

        NSString *repText = [mTextField.text stringByReplacingOccurrencesOfString:@"-----------------------------------" withString:@""];

        if ([inputLang isEqual:@"zh-Hans"]) {
            NSUInteger count = [[NSRegularExpression
               regularExpressionWithPattern:@"[A-Za-z]"
               options:NSRegularExpressionCaseInsensitive
               error:NULL]
             numberOfMatchesInString:repText
             options:NSMatchingProgress
             range:NSMakeRange(0, [repText length])
            ];

            if (count > 0) {
                return;
            }
        }

        NSString *insertCharacter = [[NSRegularExpression
            regularExpressionWithPattern:[NSString stringWithFormat:@"^%@", mLastText]
            options:NSRegularExpressionUseUnicodeWordBoundaries
            error:NULL]
           stringByReplacingMatchesInString:repText
           options:0
           range:NSMakeRange(0, [repText length])
           withTemplate:@""
        ];

        if ([repText length] > [mLastText length]) {
            mLastText = repText;
            unsigned long size = [insertCharacter length];

            if (size >= 1) {
                for (int i = 0; i < size; i++) {
                    std::wstring s;
                    s = (WCHAR)[insertCharacter characterAtIndex:i];
                    pGUI->OnHandleWinMsgChars(s);
                }
            }
        } else if ([repText length] < [mLastText length]) {
            mLastText = repText;
            pGUIIns->SendKeyDownEvent(EVirtualKey::KEY_BACK);
            pGUIIns->SendKeyUpEvent(EVirtualKey::KEY_BACK);
        }
    }
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if (textField == mTextField && CGlobals::GetApp()->GetAppState() == PEAppState_Ready)
    {
        auto pGUI = CGUIRoot::GetInstance();
        if (pGUI)
        {
            pGUI->SendKeyDownEvent(EVirtualKey::KEY_RETURN);
            pGUI->SendKeyUpEvent(EVirtualKey::KEY_RETURN);
        }
    }
    
    return NO;
}

@end

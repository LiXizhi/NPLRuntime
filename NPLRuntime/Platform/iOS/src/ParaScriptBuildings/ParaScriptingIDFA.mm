//-----------------------------------------------------------------------------
// Class: ParaScriptingIDFA.mm
// Authors: big
// CreateDate: 2023.9.5
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <AdSupport/ASIdentifierManager.h>
#import <AppTrackingTransparency/AppTrackingTransparency.h>

#include "ParaScriptingIDFA.h"

static std::string IDFAStr = "";

namespace ParaScripting
{
    void ParaScriptingIDFA::requestTrackingAuthorizationAndFetchIDEA(std::function<void(int)> callback)
    {
        if (@available(iOS 14, *)) {
            [ATTrackingManager requestTrackingAuthorizationWithCompletionHandler:^(ATTrackingManagerAuthorizationStatus status)
            {
                if (status == ATTrackingManagerAuthorizationStatusAuthorized) {
                    // 用户已授权广告跟踪
                    NSString *idfaString = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
                    NSLog(@"Authorized. IDFA: %@", idfaString);
                } else {
                    // 用户未授权广告跟踪
                    NSLog(@"Not authorized for tracking.");
                }
            }];
        } else {
            
        }
        
    }

	std::string ParaScriptingIDFA::Get()
	{
        NSLog(@"from ParaScriptingIDFA::Get().....");
        if (IDFAStr != "") {
            NSLog(@"55555");
            return IDFAStr;
        }

        NSLog(@"111111111");
        if ([ASIdentifierManager sharedManager].advertisingTrackingEnabled) {
            NSLog(@"222222222");
            NSUUID *advertisingTracking = [ASIdentifierManager sharedManager].advertisingIdentifier;
            NSString *idfaString = [advertisingTracking UUIDString];

            IDFAStr = [idfaString UTF8String];
        } else {
            requestTrackingAuthorizationAndFetchIDEA([](int result) {
                NSLog(@"from ParaScriptingIDFA::Get() 222222 ........");
                
                NSLog(@"222222222");
                NSUUID *advertisingTracking = [ASIdentifierManager sharedManager].advertisingIdentifier;
                NSString *idfaString = [advertisingTracking UUIDString];

                IDFAStr = [idfaString UTF8String];
            });
        }
        
        NSLog(@"4444444");
        
        return IDFAStr;
    }
}

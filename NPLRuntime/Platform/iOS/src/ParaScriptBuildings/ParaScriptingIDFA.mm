//-----------------------------------------------------------------------------
// Class: ParaScriptingIDFA.mm
// Authors: big
// CreateDate: 2023.9.5
// ModifyDate: 2023.9.11
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <AdSupport/ASIdentifierManager.h>
#import <AppTrackingTransparency/AppTrackingTransparency.h>
#import "LuaObjcBridge/CCLuaObjcBridge.h"

#include "ParaScriptingIDFA.h"

static std::string IDFAStr = "";

namespace ParaScripting
{
    void ParaScriptingIDFA::requestTrackingAuthorizationAndFetchIDEA(std::function<void(std::string)> callback)
    {
        if (@available(iOS 14, *)) {
            [ATTrackingManager requestTrackingAuthorizationWithCompletionHandler:^(ATTrackingManagerAuthorizationStatus status)
            {
                if (status == ATTrackingManagerAuthorizationStatusAuthorized) {
                    // User has authorized ad tracking
                    NSString *idfaString = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
                    callback([idfaString UTF8String]);
                } else {
                    // User did not authorize ad tracking
                    NSLog(@"Not authorized for tracking.");
                }
            }];
        }
    }

	std::string ParaScriptingIDFA::Get()
	{
        if (IDFAStr != "") {
            return IDFAStr;
        }

        if ([ASIdentifierManager sharedManager].advertisingTrackingEnabled) {
            NSUUID *advertisingTracking = [ASIdentifierManager sharedManager].advertisingIdentifier;
            NSString *idfaString = [advertisingTracking UUIDString];

            IDFAStr = [idfaString UTF8String];
        }
        
        return IDFAStr;
    }

    void ParaScriptingIDFA::RequestTrackingAuthorization()
    {
        if (@available(iOS 14, *)) {
            requestTrackingAuthorizationAndFetchIDEA([](std::string result) {
                IDFAStr = result;
            });
        }
    }
}

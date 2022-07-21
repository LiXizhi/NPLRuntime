//
//  DeviceInfo.h
//  Paracraft
//
//  Created by Paracraft on 5/7/2022.
//

#ifndef DeviceInfo_h
#define DeviceInfo_h

#import <Foundation/Foundation.h>

@interface DeviceInfo : NSObject

+(NSString*)getDeviceInfoJsonStr;
+(NSString*)getAppInfoJsonStr;

@end


#endif /* DeviceInfo_h */

//
//  DeviceInfo.m
//  Paracraft
//
//  Created by Paracraft on 5/7/2022.
//

#import "DeviceInfo.h"
//获取设备唯一标识的库
#import <sys/utsname.h>
#import <UIKit/UIDevice.h>
#import <math.h>

NSString* dict2json(NSDictionary *dic){
    NSError *parseError = nil;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dic                 options:NSJSONWritingPrettyPrinted error:&parseError];

    return [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];

}

@implementation DeviceInfo

+ (NSString *)device_name {
    return UIDevice.currentDevice.name;
}
 
+ (NSString *)device_model {
    return [[UIDevice currentDevice] model];
}
 
+ (NSString *)device_system_name {
    return  [[UIDevice currentDevice] systemName];
}
 
+ (NSString *)device_system_version {
    return  [[UIDevice currentDevice] systemVersion];
}
 
+(NSString*)getDeviceInfoJsonStr
{
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    [dict setObject:[self model] forKey:@"ios_model"];
    [dict setObject:[self device_model_name:[self model]] forKey:@"ios_model_name"];
    [dict setObject:[self device_name] forKey:@"device_name"];
    [dict setObject:[self device_model] forKey:@"device_model"];
    [dict setObject:[self device_system_name] forKey:@"device_system_name"];
    [dict setObject:[self device_system_version] forKey:@"device_system_version"];
    [dict setObject:[self getLanguageCode] forKey:@"language"];
    [dict setObject:[self getCountryCode] forKey:@"region"];
    
    float total = [self getTotalDiskSpace]/(1024*1024*1024);
    float free = [self getFreeDiskSpace]/(1024*1024*1024);
    total = floor(total);
    free = floor(free);
    
    NSString * sdcardStr = [[NSString alloc]initWithFormat:@"%dG / %dG",(int)free,(int)total];
    [dict setObject:sdcardStr forKey:@"sdcard"];
    
    return dict2json(dict);
}

+ (float)getTotalDiskSpace{
    float totalsize = 0.0;
    NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSDictionary *dictionary = [[NSFileManager defaultManager] attributesOfFileSystemForPath:[paths lastObject] error: &error];
    if (dictionary)
    {
       NSNumber *_total = [dictionary objectForKey:NSFileSystemSize];
       totalsize = [_total unsignedLongLongValue]*1.0;
    } else
    {
       NSLog(@"Error Obtaining System Memory Info: Domain = %@, Code = %ld", [error domain], (long)[error code]);
    }
    return totalsize;
}

+(float)getFreeDiskSpace{
    float freesize = 0.0;
    NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSDictionary *dictionary = [[NSFileManager defaultManager] attributesOfFileSystemForPath:[paths lastObject] error: &error];
    if (dictionary)
    {
       NSNumber *_free = [dictionary objectForKey:NSFileSystemFreeSize];
       freesize = [_free unsignedLongLongValue]*1.0;
    } else
    {
       NSLog(@"Error Obtaining System Memory Info: Domain = %@, Code = %ld", [error domain], (long)[error code]);
    }
    return  freesize;
}

+ (NSString *)model{
    struct utsname systemInfo;
    uname(&systemInfo);
          
    NSString * deviceModel = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    return deviceModel;
}
 
+ (NSString *)device_model_name: (NSString*)deviceModel {
#pragma mark iPod
    if ([deviceModel isEqualToString:@"iPod1,1"]) {
        return @"iPod Touch 1";
    }
    if ([deviceModel isEqualToString:@"iPod2,1"]) {
        return @"iPod Touch 2";
    }
    if ([deviceModel isEqualToString:@"iPod3,1"]) {
        return @"iPod Touch 3";
    }
    if ([deviceModel isEqualToString:@"iPod4,1"]) {
        return @"iPod Touch 4";
    }
    if ([deviceModel isEqualToString:@"iPod5,1"]) {
        return @"iPod Touch (5 Gen)";
    }
    if ([deviceModel isEqualToString:@"iPod7,1"]) {
        return @"iPod Touch 6";
    }
#pragma mark --iPhone
    if ([deviceModel isEqualToString:@"iPhone7,2"]) {
        return @"iPhone 6";
    }
    if ([deviceModel isEqualToString:@"iPhone7,1"]) {
        return @"iPhone 6 Plus";
    }
    if ([deviceModel isEqualToString:@"iPhone8,1"]) {
        return @"iPhone 6s";
    }
    if ([deviceModel isEqualToString:@"iPhone8,2"]) {
        return @"iPhone 6s Plus";
    }
    if ([deviceModel isEqualToString:@"iPhone8,4"]) {
        return @"iPhone SE";
    }
    if ([deviceModel isEqualToString:@"iPhone9,1"]) {
        return @"国行、日版、港行 iPhone 7";
    }
    if ([deviceModel isEqualToString:@"iPhone9,2"]) {
        return @"港行、国行 iPhone 7 Plus";
    }
    if ([deviceModel isEqualToString:@"iPhone9,3"]) {
        return @"美版、台版 iPhone 7";
    }
    if ([deviceModel isEqualToString:@"iPhone9,4"]) {
        return @"美版、台版 iPhone 7 Plus";
    }
    if ([deviceModel isEqualToString:@"iPhone10,1"] || [deviceModel isEqualToString:@"iPhone10,4"]) {
        return @"iPhone 8";
    }
    if ([deviceModel isEqualToString:@"iPhone10,2"] || [deviceModel isEqualToString:@"iPhone10,5"]) {
        return @"iPhone 8 Plus";
    }
    if ([deviceModel isEqualToString:@"iPhone10,3"] || [deviceModel isEqualToString:@"iPhone10,6"]) {
        return @"iPhone X";
    }
    if ([deviceModel isEqualToString:@"iPhone11,8"]) {
        return @"iPhone XR";
    }
    if ([deviceModel isEqualToString:@"iPhone11,2"]) {
        return @"iPhone XS";
    }
    if ([deviceModel isEqualToString:@"iPhone11,6"]) {
        return @"iPhone XS Max";
    }
    if ([deviceModel isEqualToString:@"iPhone11,4"]) {
        return @"iPhone XS Max (China)";
    }
    if ([deviceModel isEqualToString:@"iPhone12,1"]) {
        return @"iPhone 11";
    }
    if ([deviceModel isEqualToString:@"iPhone12,3"]) {
        return @"iPhone 11 Pro";
    }
    if ([deviceModel isEqualToString:@"iPhone12,5"]) {
        return @"iPhone 11 Pro Max";
    }
    if ([deviceModel isEqualToString:@"iPhone13,1"]) {
        return @"iPhone 12 mini";
    }
    if ([deviceModel isEqualToString:@"iPhone13,2"]) {
        return @"iPhone 12";
    }
    if ([deviceModel isEqualToString:@"iPhone13,3"]) {
        return @"iPhone 12 Pro";
    }
    if ([deviceModel isEqualToString:@"iPhone13,4"]) {
        return @"iPhone 12 Pro Max";
    }
    if ([deviceModel isEqualToString:@"iPhone14,4"]) {
        return @"iPhone 13 mini";
    }
    if ([deviceModel isEqualToString:@"iPhone14,5"]) {
        return @"iPhone 13";
    }
    if ([deviceModel isEqualToString:@"iPhone14,2"]) {
        return @"iPhone 13 Pro";
    }
    if ([deviceModel isEqualToString:@"iPhone14,3"]) {
        return @"iPhone 13 Pro Max";
    }if ([deviceModel isEqualToString:@"iPhone14,6"]) {
        return @"iPhone SE (3rd generation)";
    }

#pragma mark ---iPad
    if ([deviceModel isEqualToString:@"iPad1,1"]) {
        return @"iPad";
    }
    if ([deviceModel isEqualToString:@"iPad1,2"]) {
        return @"iPad 3G";
    }
    if ([deviceModel isEqualToString:@"iPad2,1"] || [deviceModel isEqualToString:@"iPad2,2"] || [deviceModel isEqualToString:@"iPad2,3"] || [deviceModel isEqualToString:@"iPad2,4"]) {
        return @"iPad 2";
    }
    if ([deviceModel isEqualToString:@"iPad3,1"] || [deviceModel isEqualToString:@"iPad3,2"] || [deviceModel isEqualToString:@"iPad3,3"]) {
        return @"iPad 3";
    }
    if ([deviceModel isEqualToString:@"iPad3,4"] || [deviceModel isEqualToString:@"iPad3,5"] || [deviceModel isEqualToString:@"iPad3,6"]) {
        return @"iPad 4";
    }
    if ([deviceModel isEqualToString:@"iPad6,11"] || [deviceModel isEqualToString:@"iPad6,12"]) {
        return @"iPad 5";
    }
    if ([deviceModel isEqualToString:@"iPad7,5"] || [deviceModel isEqualToString:@"iPad7,6"]) {
        return @"iPad 6";
    }
    if ([deviceModel isEqualToString:@"iPad7,11"] || [deviceModel isEqualToString:@"iPad7,12"]) {
        return @"iPad 7";
    }
    if ([deviceModel isEqualToString:@"iPad11,6"] || [deviceModel isEqualToString:@"iPad11,7"]) {
        return @"iPad 8";
    }
    if ([deviceModel isEqualToString:@"iPad12,1"] || [deviceModel isEqualToString:@"iPad12,2"]) {
        return @"iPad 9";
    }

#pragma mark ---iPad air
    if ([deviceModel isEqualToString:@"iPad4,1"] || [deviceModel isEqualToString:@"iPad4,2"] || [deviceModel isEqualToString:@"iPad4,3"]) {
        return @"iPad Air";
    }
    if ([deviceModel isEqualToString:@"iPad5,3"] || [deviceModel isEqualToString:@"iPad5,4"]) {
        return @"iPad Air 2";
    }
    if ([deviceModel isEqualToString:@"iPad11,3"] || [deviceModel isEqualToString:@"iPad11,4"]) {
        return @"iPad Air 3";
    }
    if ([deviceModel isEqualToString:@"iPad13,1"] || [deviceModel isEqualToString:@"iPad13,2"]) {
        return @"iPad Air 4";
    }
    if ([deviceModel isEqualToString:@"iPad13,6"] || [deviceModel isEqualToString:@"iPad13,7"]) {
        return @"iPad Air 5";
    }

#pragma mark ---iPad Pro
    if ([deviceModel isEqualToString:@"iPad6,3"] || [deviceModel isEqualToString:@"iPad6,4"]) {
        return @"iPad Pro 9.7";
    }
    if ([deviceModel isEqualToString:@"iPad6,7"] || [deviceModel isEqualToString:@"iPad6,8"]) {
        return @"iPad Pro 12.9";
    }
    if ([deviceModel isEqualToString:@"iPad7,1"] || [deviceModel isEqualToString:@"iPad7,2"]) {
        return @"iPad Pro (12.9-inch) (2nd generation)";
    }
    if ([deviceModel isEqualToString:@"iPad7,3"] || [deviceModel isEqualToString:@"iPad7,4"]) {
        return @"iPad Pro (10.5-inch)";
    }
    if ([deviceModel isEqualToString:@"iPad8,1"] || [deviceModel isEqualToString:@"iPad8,2"] || [deviceModel isEqualToString:@"iPad8,3"] || [deviceModel isEqualToString:@"iPad8,4"]) {
        return @"iPad Pro (11-inch)";
    }
    if ([deviceModel isEqualToString:@"iPad8,5"] || [deviceModel isEqualToString:@"iPad8,6"] || [deviceModel isEqualToString:@"iPad8,7"] || [deviceModel isEqualToString:@"iPad8,8"]) {
        return @"iPad Pro (12.9-inch) (3rd generation)";
    }
    if ([deviceModel isEqualToString:@"iPad8,9"] || [deviceModel isEqualToString:@"iPad8,10"]) {
        return @"iPad Pro (11-inch) (2nd generation)";
    }
    if ([deviceModel isEqualToString:@"iPad8,11"] || [deviceModel isEqualToString:@"iPad8,12"]) {
        return @"iPad Pro (12.9-inch) (4th generation)";
    }
    if ([deviceModel isEqualToString:@"iPad13,4"] || [deviceModel isEqualToString:@"iPad13,5"] || [deviceModel isEqualToString:@"iPad13,6"] || [deviceModel isEqualToString:@"iPad13,7"]) {
        return @"iPad Pro (11-inch) (3rd generation)";
    }
    if ([deviceModel isEqualToString:@"iPad13,8"] || [deviceModel isEqualToString:@"iPad13,9"] || [deviceModel isEqualToString:@"iPad13,10"] || [deviceModel isEqualToString:@"iPad13,11"]) {
        return @"iPad Pro (12.9-inch) (5th generation)";
    }

#pragma mark ---iPad mini
    if ([deviceModel isEqualToString:@"iPad2,5"] || [deviceModel isEqualToString:@"iPad2,6"] || [deviceModel isEqualToString:@"iPad2,7"]) {
        return @"iPad Mini";
    }
    if ([deviceModel isEqualToString:@"iPad4,4"] || [deviceModel isEqualToString:@"iPad4,5"] || [deviceModel isEqualToString:@"iPad4,6"]) {
        return @"iPad Mini 2";
    }
    if ([deviceModel isEqualToString:@"iPad4,7"] || [deviceModel isEqualToString:@"iPad4,8"] || [deviceModel isEqualToString:@"iPad4,9"]) {
        return @"iPad Mini 3";
    }
    if ([deviceModel isEqualToString:@"iPad5,1"] || [deviceModel isEqualToString:@"iPad5,2"]) {
        return @"iPad Mini 4";
    }
    if ([deviceModel isEqualToString:@"iPad11,1"] || [deviceModel isEqualToString:@"iPad11,2"]) {
        return @"iPad Mini 5";
    }
    if ([deviceModel isEqualToString:@"iPad14,1"] || [deviceModel isEqualToString:@"iPad14,2"]) {
        return @"iPad Mini 6";
    }
#pragma mark --- Apple TV
    if ([deviceModel isEqualToString:@"AppleTV2,1"]) {
        return @"Apple TV 2";
    }
    if ([deviceModel isEqualToString:@"AppleTV3,1"] || [deviceModel isEqualToString:@"AppleTV3,2"]) {
        return @"Apple TV 3";
    }
    if ([deviceModel isEqualToString:@"AppleTV5,3"]) {
        return @"Apple TV 4";
    }
    if ([deviceModel isEqualToString:@"i386"] || [deviceModel isEqualToString:@"x86_64"]) {
        return @"Simulator";
    }
    
    return deviceModel;
}


+(NSString*)getLanguageCode
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];

    // get the current language code.(such as English is "en", Chinese is "zh" and so on)
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    NSString * countryCode = [temp objectForKey:NSLocaleCountryCode];
    return [languageCode lowercaseString];
}

+(NSString*)getCountryCode
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];

    // get the current language code.(such as English is "en", Chinese is "zh" and so on)
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    NSString * countryCode = [temp objectForKey:NSLocaleCountryCode];
    return [countryCode lowercaseString];
}

+(NSString*)getBundleID
{
    NSString* pkgname=[[NSBundle mainBundle] bundleIdentifier];
    return pkgname;
}

+(NSString*)getVersionName{
    NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSString *applocalversion = [infoDictionary objectForKey:@"CFBundleShortVersionString"];
    return applocalversion;
}

+(NSString*)getBuildNumber{
    NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSString *applocalversion = [infoDictionary objectForKey:@"CFBundleVersion"];
    return applocalversion;
}

+(NSString*)getAppName
{
    NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
    CFShow((__bridge CFDictionaryRef)infoDictionary);
    // app名称
    NSString *app_Name = [infoDictionary objectForKey:@"CFBundleDisplayName"];
    if(app_Name==nil){
        app_Name = [infoDictionary objectForKey:@"CFBundleName"];
    }
    return app_Name;
}

+(NSString*)getAppInfoJsonStr{
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    [dict setObject:[self getAppName] forKey:@"appName"];
    [dict setObject:[self getBundleID] forKey:@"bundleId"];
    [dict setObject:[self getBuildNumber] forKey:@"buildNumber"];
    [dict setObject:[self getVersionName] forKey:@"appVersion"];
    
    return dict2json(dict);
}
 
@end

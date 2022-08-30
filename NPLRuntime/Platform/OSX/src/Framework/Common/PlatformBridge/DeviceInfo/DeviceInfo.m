//
//  DeviceInfo.m
//  Paracraft
//
//  Created by Paracraft on 5/7/2022.
//

#import "DeviceInfo.h"
#import "NSHost+Hardware.h"

NSString* dict2json(NSDictionary *dic){
    NSError *parseError = nil;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dic                 options:NSJSONWritingPrettyPrinted error:&parseError];

    return [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];

}

@implementation DeviceInfo


+ (NSString *)getDeviceInfoJsonStr{
    NSHost *host = [NSHost currentHost];
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
    [dict setObject:[host model] forKey:@"mac_model"];
    [dict setObject:[host generation] forKey:@"mac_model_name"];
    [dict setObject:[self getLanguageCode] forKey:@"language"];
    [dict setObject:[self getCountryCode] forKey:@"region"];
    
    return dict2json(dict);
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

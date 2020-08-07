//-----------------------------------------------------------------------------
// Class:	iOS ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.11
//-----------------------------------------------------------------------------
#import <Foundation/Foundation.h>

#include "ParaEngine.h"
#include "ParaEngineSettings.h"

@interface KeyChainHelper : NSObject
+ (void) saveString:(NSString*)stringValue
            service:(NSString*)service;

+ (NSString*) getString:(NSString*)service;

@end

@implementation KeyChainHelper

+ (NSMutableDictionary*) getKeyChainQuery:(NSString*)service
{
    return [NSMutableDictionary dictionaryWithObjectsAndKeys:(id)kSecClassGenericPassword, (id)kSecClass
            , service, (id)kSecAttrService
            , service, (id)kSecAttrAccount
            , (id)kSecAttrAccessibleAfterFirstUnlock
            , (id)kSecAttrAccessible
            , nil];
}

+ (void) saveString:(NSString *)stringValue service:(NSString *)service
{
    NSMutableDictionary *keychainQuery = [self getKeyChainQuery:service];
    
    SecItemDelete((CFDictionaryRef)keychainQuery);
    [keychainQuery setObject:[NSKeyedArchiver archivedDataWithRootObject:stringValue] forKey:(id)kSecValueData];
    SecItemAdd((CFDictionaryRef)keychainQuery, nil);
    
    
}

+ (NSString*)getString:(NSString *)service
{
    NSString* ret = nil;
    
    NSMutableDictionary *keychainQuery = [self getKeyChainQuery:service];
    [keychainQuery setObject:(id)kCFBooleanTrue forKey:(id)kSecReturnData];
    [keychainQuery setObject:(id)kSecMatchLimitOne forKey:(id)kSecMatchLimit];
    
    CFDataRef keyData = nullptr;
    if (SecItemCopyMatching((CFDictionaryRef)keychainQuery, (CFTypeRef*)&keyData) == noErr)
    {
        @try {
            ret = [NSKeyedUnarchiver unarchiveObjectWithData:(__bridge NSData*)keyData];
        } @catch (NSException *exception) {
            OUTPUT_LOG("Unarchive of %s failed: %s\n", [service UTF8String], [[exception description] UTF8String]);
        } @finally {
    
        }
    }
    
    return ret;
}

@end


namespace ParaEngine {

    NSString* const kUUIDKey = @"com.tatfook.paracraft.uuid";



    const std::string& ParaEngineSettings::GetMachineID()
    {
        static std::string str = "";
        
        if (str.empty())
        {
            NSString *getUUIDInKeychain = [KeyChainHelper getString:kUUIDKey];
            
            if (!getUUIDInKeychain || [getUUIDInKeychain isEqualToString:@""] || [getUUIDInKeychain isKindOfClass:[NSNull class]])
            {
                CFUUIDRef puuid = CFUUIDCreate(nil);
                CFStringRef uuidString = CFUUIDCreateString(nil, puuid);
                NSString *result = (NSString*)CFBridgingRelease(CFStringCreateCopy(nil, uuidString));
                CFRelease(puuid);
                CFRelease(uuidString);
                
                [KeyChainHelper saveString:result service:kUUIDKey];
                
                str = [[KeyChainHelper getString:kUUIDKey] UTF8String];
            }
            else
            {
                str = [getUUIDInKeychain UTF8String];
            }
        }
        
        return str;
    }

} // namespace ParaEngine

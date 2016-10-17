#ifdef PLATFORM_IOS
#include "LocalBridgeHandler_ios.h"
#include "LocalBridge_ios.h"
#include "platform/ios/LocalBridgePB.pb.h"
#import <AudioToolbox/AudioToolbox.h>
#include <string>

#import <sys/types.h>
#import <sys/sysctl.h>
#import <mach/host_info.h>
#import <mach/mach_host.h>
#import <mach/task_info.h>
#import <mach/task.h>
#import <UIKit/UIKit.h>
using namespace org::cocos2dx::lua;

void LocalBridgeHandler::makeResponse(const char* key, const char* data, const char* tag)
{
    LocalBridge::callLuaFunctionWithString(key, data);
}
void LocalBridgeHandler::ios_localCall(const char* api_key, const char* data)
{
    std::string key = api_key;
    if(key == "getMemoryInfo")
        getMemoryInfo_Request(api_key,data);
    if(key == "openURL")
        openURL_Request(api_key,data);
}
void LocalBridgeHandler::vibrate_Request(const char* api_key, const char* data)
{
    org::cocos2dx::lua::vibrate_Request req;
    req.ParseFromString(data);
    double time = req.time();
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    vibrate_Response response;
    response.set_issuccess(1);
    makeResponse("vibrate_Response", response.SerializeAsString().c_str(),response.DebugString().c_str());
}
void LocalBridgeHandler::getMemoryInfo_Request(const char* api_key, const char* data)
{
    mach_port_t host_port;
    mach_msg_type_number_t host_size;
    vm_size_t pagesize;
    
    host_port = mach_host_self();
    host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    host_page_size(host_port, &pagesize);
    
    vm_statistics_data_t vm_stat;
    
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) {
        NSLog(@"Failed to fetch vm statistics");
    }
    
    /* Stats in bytes */
    natural_t mem_used = (vm_stat.active_count +
                          vm_stat.inactive_count +
                          vm_stat.wire_count) * pagesize;
    natural_t mem_free = vm_stat.free_count * pagesize;
    natural_t mem_total = mem_used + mem_free;
    
    getMemoryInfo_Response response;
    response.set_availmem(mem_free);
    response.set_totalmem(mem_total);

    
    makeResponse("getMemoryInfo_Response", response.SerializeAsString().c_str(),response.DebugString().c_str());
}
void LocalBridgeHandler::openURL_Request(const char* api_key, const char* data)
{
    org::cocos2dx::lua::openURL_Request req;
    req.ParseFromString(data);
    std::string url = req.url();
    NSString *str= [NSString stringWithCString:url.c_str() encoding:[NSString defaultCStringEncoding]];
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:str]];
}
#endif
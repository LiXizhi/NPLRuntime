//-----------------------------------------------------------------------------
// Class:	Mac OSX ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.11
//-----------------------------------------------------------------------------
#import <Cocoa/Cocoa.h>

#include "ParaEngine.h"
#include "ParaEngineSettings.h"

namespace ParaEngine {

    const std::string& ParaEngineSettings::GetMachineID()
    {
        static std::string str = "";
        
        if (str.empty())
        {
            NSTask *task = [[NSTask alloc] init];
            [task setLaunchPath:@"/usr/sbin/ioreg"];
            
            NSArray *arguments = [NSArray arrayWithObjects:@"-rd1", @"-c", @"IOPlatformExpertDevice", nil];
            [task setArguments:arguments];
            
            NSPipe *pipe = [NSPipe pipe];
            [task setStandardOutput:pipe];
            
            NSFileHandle *file = [pipe fileHandleForReading];
            
            [task launch];
            
            NSData *data = [file readDataToEndOfFile];
            
            NSString *string = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            
            NSString *key = @"IOPlatformUUID";
            NSRange range = [string rangeOfString:key];
            
            NSInteger location = range.location + [key length] + 5;
            NSInteger length = 32 + 4;
            range.location =location;
            range.length = length;
            
            NSString *UUID = [string substringWithRange:range];
            
            str = [UUID UTF8String];
        }
        
        return str;
    }


} // namespace ParaEngine

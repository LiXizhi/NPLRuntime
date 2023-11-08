//-----------------------------------------------------------------------------
// Class: SerialPortMac.mm
// Authors: big
// Emails: onedou@126.com
// Date: 2023.8.11
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import <IOKit/serial/IOSerialKeys.h>
#import <IOKit/IOKitLib.h>

#import "SerialPortMac.h"

namespace ParaEngine {
    std::vector<std::string> SerialPortMac::GetPortNames() {
        std::vector<std::string> ports;
        CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
        CFDictionarySetValue(matchingDict, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));

        io_iterator_t serialPortIterator;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &serialPortIterator);

        if (kr != KERN_SUCCESS) {
            NSLog(@"IOServiceGetMatchingServices error: %d", kr);
            return ports;
        }

        io_object_t modemService;
        while ((modemService = IOIteratorNext(serialPortIterator))) {
            CFStringRef bsdPath = (CFStringRef)IORegistryEntryCreateCFProperty(modemService, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
            if (bsdPath) {
                NSString *serialPath = (__bridge NSString *)bsdPath;
                NSLog(@"Serial port path: %@", serialPath);
                ports.push_back([serialPath UTF8String]);
                CFRelease(bsdPath);
            }

            IOObjectRelease(modemService);
        }

        IOObjectRelease(modemService);

        return ports;
    }
}

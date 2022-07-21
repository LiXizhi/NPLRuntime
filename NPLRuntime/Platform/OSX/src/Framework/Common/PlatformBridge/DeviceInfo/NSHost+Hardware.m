//
//  NSHost+Hardware.m
//  macdevice
//
//  Created by 李坚 on 2021/10/8.
//

#import "NSHost+Hardware.h"
#import <sys/sysctl.h>

#pragma mark - iMac
NSString *iMacPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"iMac9,1"])        return @"iMac (20-inch, 2009)";
    if ([platform isEqualToString:@"iMac9,2"])        return @"iMac (24-inch, 2009)";
    if ([platform isEqualToString:@"iMac10,1"])      return @"iMac (21.5-inch, 2009)";
    if ([platform isEqualToString:@"iMac10,2"])      return @"iMac (27-inch, 2009)";
    if ([platform isEqualToString:@"iMac11,2"])      return @"iMac (21.5-inch, 2010)";
    if ([platform isEqualToString:@"iMac11,3"])      return @"iMac (27-inch, 2010)";
    if ([platform isEqualToString:@"iMac12,1"])      return @"iMac (21.5-inch, 2011)";
    if ([platform isEqualToString:@"iMac12,2"])      return @"iMac (27-inch, 2011)";
    if ([platform isEqualToString:@"iMac13,1"])      return @"iMac (21.5-inch, 2012)";
    if ([platform isEqualToString:@"iMac13,2"])      return @"iMac (27-inch, 2012)";
    if ([platform isEqualToString:@"iMac14,1"])      return @"iMac (21.5-inch, 2013)";
    if ([platform isEqualToString:@"iMac14,2"])      return @"iMac (27-inch, 2013)";
    if ([platform isEqualToString:@"iMac14,4"])      return @"iMac (21.5-inch, 2014)";
    if ([platform isEqualToString:@"iMac15,1"])      return @"iMac (Retina 5K, 27-inch, Middle 2015)";
    if ([platform isEqualToString:@"iMac16,1"])      return @"iMac (Retina 4K, 21.5-inch, Late 2015)";
    if ([platform isEqualToString:@"iMac16,2"])      return @"iMac (21.5-inch, Late 2015)";
    if ([platform isEqualToString:@"iMac17,1"])      return @"iMac (Retina 5K, 27-inch, Late 2015)";
    if ([platform isEqualToString:@"iMac18,1"])      return @"iMac (21.5-inch, 2017)";
    if ([platform isEqualToString:@"iMac18,2"])      return @"iMac (Retina 4K, 21.5-inch, 2017)";
    if ([platform isEqualToString:@"iMac18,3"])      return @"iMac (Retina 5K, 27-inch, 2017)";
    if ([platform isEqualToString:@"iMacPro1,1"])   return @"iMac Pro (2017)";
    if ([platform isEqualToString:@"iMac19,1"])      return @"iMac (Retina 5K, 27-inch, 2019)";
    if ([platform isEqualToString:@"iMac19,2"])      return @"iMac (Retina 5K, 21.5-inch, 2019)";
    if ([platform isEqualToString:@"iMac20,1"])      return @"iMac (Retina 5K, 27-inch, 2020)";
    if ([platform isEqualToString:@"iMac20,2"])      return @"iMac (Retina 5K, 27-inch, 2020)";
    if ([platform isEqualToString:@"iMac21,1"])      return @"iMac (24-inch, M1, 2021)";
    if ([platform isEqualToString:@"iMac21,2"])      return @"iMac (24-inch, M1, 2021)";

    return platform;
}

#pragma mark - Mac mini
NSString *MacminiPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"Macmini3,1"])      return @"Mac mini (2009)";
    if ([platform isEqualToString:@"Macmini4,1"])      return @"Mac mini (2010)";
    if ([platform isEqualToString:@"Macmini5,1"])      return @"Mac mini (2011)";
    if ([platform isEqualToString:@"Macmini5,2"])      return @"Mac mini (2011)";
    if ([platform isEqualToString:@"Macmini6,1"])      return @"Mac mini (2012)";
    if ([platform isEqualToString:@"Macmini6,2"])      return @"Mac mini (2012)";
    if ([platform isEqualToString:@"Macmini7,1"])      return @"Mac mini (2014)";
    if ([platform isEqualToString:@"Macmini8,1"])      return @"Mac mini (2018)";
    if ([platform isEqualToString:@"Macmini9,1"])      return @"Mac mini (M1, 2020)";

    return platform;
}

#pragma mark - MacBook Air
NSString *MacBookAirPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"MacBookAir2,1"])      return @"MacBook Air (2009)";
    if ([platform isEqualToString:@"MacBookAir3,1"])      return @"MacBook Air (11-inch, 2010)";
    if ([platform isEqualToString:@"MacBookAir3,2"])      return @"MacBook Air (13-inch, 2010)";
    if ([platform isEqualToString:@"MacBookAir4,1"])      return @"MacBook Air (11-inch, 2011)";
    if ([platform isEqualToString:@"MacBookAir4,2"])      return @"MacBook Air (13-inch, 2011)";
    if ([platform isEqualToString:@"MacBookAir5,1"])      return @"MacBook Air (11-inch, 2012)";
    if ([platform isEqualToString:@"MacBookAir5,2"])      return @"MacBook Air (13-inch, 2012)";
    if ([platform isEqualToString:@"MacBookAir6,1"])      return @"MacBook Air (11-inch, 2013/2014)";
    if ([platform isEqualToString:@"MacBookAir6,2"])      return @"MacBook Air (13-inch, 2013/2014)";
    if ([platform isEqualToString:@"MacBookAir7,1"])      return @"MacBook Air (11-inch, 2015)";
    if ([platform isEqualToString:@"MacBookAir7,2"])      return @"MacBook Air (13-inch, 2015/2017)";
    if ([platform isEqualToString:@"MacBookAir8,1"])      return @"MacBook Air (Retina, 13-inch, 2018)";
    if ([platform isEqualToString:@"MacBookAir8,2"])      return @"MacBook Air (Retina, 13-inch, 2019)";
    if ([platform isEqualToString:@"MacBookAir9,1"])      return @"MacBook Air (Retina, 13-inch, 2020)";
    if ([platform isEqualToString:@"MacBookAir10,1"])    return @"MacBook Air (M1, 2020)";
    if ([platform isEqualToString:@"MacBookAir14,1"])    return @"MacBook Air (M2, 2022)";
    if ([platform isEqualToString:@"MacBookAir14,2"])    return @"MacBook Air (M2, 2022)";

    return platform;
}

#pragma mark - MacBook Pro
NSString *MacBookProPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"MacBookPro4,1"])      return @"MacBook Pro (15-inch, Early 2008)";
    if ([platform isEqualToString:@"MacBookPro4,2"])      return @"MacBook Pro (17-inch, Early 2008)";
    if ([platform isEqualToString:@"MacBookPro5,1"])      return @"MacBook Pro (15-inch, Late 2008)";
    if ([platform isEqualToString:@"MacBookPro5,2"])      return @"MacBook Pro (17-inch, 2009)";
    if ([platform isEqualToString:@"MacBookPro5,3"])      return @"MacBook Pro (15-inch, 2009)";
    if ([platform isEqualToString:@"MacBookPro5,5"])      return @"MacBook Pro (13-inch, 2009)";
    if ([platform isEqualToString:@"MacBookPro6,1"])      return @"MacBook Pro (17-inch, 2010)";
    if ([platform isEqualToString:@"MacBookPro6,2"])      return @"MacBook Pro (15-inch, 2010)";
    if ([platform isEqualToString:@"MacBookPro8,1"])      return @"MacBook Pro (13-inch, 2011)";
    if ([platform isEqualToString:@"MacBookPro8,2"])      return @"MacBook Pro (15-inch, 2011)";
    if ([platform isEqualToString:@"MacBookPro8,3"])      return @"MacBook Pro (17-inch, 2011)";
    if ([platform isEqualToString:@"MacBookPro9,1"])      return @"MacBook Pro (15-inch, 2012)";
    if ([platform isEqualToString:@"MacBookPro9,2"])      return @"MacBook Pro (13-inch, 2012)";
    if ([platform isEqualToString:@"MacBookPro10,1"])    return @"MacBook Pro (Retina, 15-inch, 2012/2013)";
    if ([platform isEqualToString:@"MacBookPro10,2"])    return @"MacBook Pro (Retina, 13-inch, 2012/2013)";
    if ([platform isEqualToString:@"MacBookPro11,1"])    return @"MacBook Pro (Retina, 13-inch, Late 2013/Middle 2014)";
    if ([platform isEqualToString:@"MacBookPro11,2"])    return @"MacBook Pro (Retina, 15-inch, Late 2013/Middle 2014)";
    if ([platform isEqualToString:@"MacBookPro11,3"])    return @"MacBook Pro (Retina, 15-inch, Late 2013/Middle 2014)";
    if ([platform isEqualToString:@"MacBookPro11,4"])    return @"MacBook Pro (Retina, 15-inch, Middle 2015)";
    if ([platform isEqualToString:@"MacBookPro11,5"])    return @"MacBook Pro (Retina, 15-inch, Middle 2015)";
    if ([platform isEqualToString:@"MacBookPro12,1"])    return @"MacBook Pro (13-inch, Early 2015)";
    if ([platform isEqualToString:@"MacBookPro13,1"])    return @"MacBook Pro (13-inch, 2016, Thunderbolt * 2)";
    if ([platform isEqualToString:@"MacBookPro13,2"])    return @"MacBook Pro (13-inch, 2016, Thunderbolt * 4)";
    if ([platform isEqualToString:@"MacBookPro13,3"])    return @"MacBook Pro (15-inch, 2016)";
    if ([platform isEqualToString:@"MacBookPro14,1"])    return @"MacBook Pro (13-inch, 2017, Thunderbolt * 2)";
    if ([platform isEqualToString:@"MacBookPro14,2"])    return @"MacBook Pro (13-inch, 2017, Thunderbolt * 4)";
    if ([platform isEqualToString:@"MacBookPro14,3"])    return @"MacBook Pro (15-inch, 2017)";
    if ([platform isEqualToString:@"MacBookPro15,1"])    return @"MacBook Pro (15-inch, 2018)";
    if ([platform isEqualToString:@"MacBookPro15,3"])    return @"MacBook Pro (15-inch, 2019)";
    if ([platform isEqualToString:@"MacBookPro15,2"])    return @"MacBook Pro (13-inch, 2019, Thunderbolt * 2)";
    if ([platform isEqualToString:@"MacBookPro15,4"])    return @"MacBook Pro (13-inch, 2019, Thunderbolt * 4)";
    if ([platform isEqualToString:@"MacBookPro16,1"])    return @"MacBook Pro (16-inch, 2019)";
    if ([platform isEqualToString:@"MacBookPro16,4"])    return @"MacBook Pro (16-inch, 2019)";
    if ([platform isEqualToString:@"MacBookPro16,2"])    return @"MacBook Pro (13-inch, 2020, Thunderbolt * 4)";
    if ([platform isEqualToString:@"MacBookPro16,3"])    return @"MacBook Pro (13-inch, 2020, Thunderbolt * 2)";
    if ([platform isEqualToString:@"MacBookPro17,1"])    return @"MacBook Pro (13-inch, M1, 2020)";
    if ([platform isEqualToString:@"MacBookPro18,3"])    return @"MacBook Pro (14-inch, 2021)";
    if ([platform isEqualToString:@"MacBookPro18,4"])    return @"MacBook Pro (14-inch, 2021)";
    if ([platform isEqualToString:@"MacBookPro18,1"])    return @"MacBook Pro (16-inch, 2021)";
    if ([platform isEqualToString:@"MacBookPro18,2"])    return @"MacBook Pro (16-inch, 2021)";
//    TODO:新款M2芯片MacBook Pro的firmware identifier有待确认
    if ([platform isEqualToString:@"MacBookPro14,7"])    return @"MacBook Pro (13-inch, M2, 2022)";

    return platform;
}

#pragma mark - MacBook
NSString *MacBookPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"MacBook10,1"])    return @"MacBook (Retina, 12-inch, 2017)";
    if ([platform isEqualToString:@"MacBook9,1"])      return @"MacBook (Retina, 12-inch, 2016)";
    if ([platform isEqualToString:@"MacBook8,1"])      return @"MacBook (Retina, 12-inch, 2015)";
    if ([platform isEqualToString:@"MacBook7,1"])      return @"MacBook (13-inch, 2010)";
    if ([platform isEqualToString:@"MacBook6,1"])      return @"MacBook (13-inch, 2009)";
    if ([platform isEqualToString:@"MacBook5,2"])      return @"MacBook (13-inch, 2009)";
    if ([platform isEqualToString:@"MacBook5,2"])      return @"MacBook (13-inch, 2009)";

    return platform;
}

#pragma mark - Mac Pro
NSString *MacProPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"MacPro4,1"])      return @"Mac Pro (2009)";
    if ([platform isEqualToString:@"MacPro5,1"])      return @"Mac Pro (2012)";
    if ([platform isEqualToString:@"MacPro6,1"])      return @"Mac Pro (2013)";
    if ([platform isEqualToString:@"MacPro7,1"])      return @"Mac Pro (2019)";
    
    return platform;
}

#pragma mark - Mac Studio
NSString *MacStudioPlatform(NSString *platform){
    
    if ([platform isEqualToString:@"Mac13,1"])      return @"Mac Studio (M1 Max)";
    if ([platform isEqualToString:@"Mac13,2"])      return @"Mac Studio (M1 Ultra)";
    
    return platform;
}

@implementation NSHost (Hardware)

#pragma mark - model identifier
- (NSString *)model{

    size_t len = 0;
    sysctlbyname("hw.model", NULL, &len, NULL, 0);
    
    if(len){
        NSMutableData *data = [NSMutableData dataWithLength:len];
        sysctlbyname("hw.model", [data mutableBytes], &len, NULL, 0);
        return [NSString stringWithUTF8String:[data bytes]];
    }
    
    return @"";
}

#pragma mark - identifier转generation
- (NSString *)generation{
    
    NSString *model = [self model];

    if([model hasPrefix:@"MacBookAir"]){
        return MacBookAirPlatform(model);
    }
    if([model hasPrefix:@"MacBookPro"]){
        return MacBookProPlatform(model);
    }
    if([model hasPrefix:@"MacBook"]){
        return MacBookPlatform(model);
    }
    if([model hasPrefix:@"iMac"]){
        return iMacPlatform(model);
    }
    if([model hasPrefix:@"Macmini"]){
        return MacminiPlatform(model);
    }
    if([model hasPrefix:@"MacPro"]){
        return MacProPlatform(model);
    }
    if([model hasPrefix:@"Mac"]){
        return MacStudioPlatform(model);
    }
    if([model hasPrefix:@"Mac"]){
        return MacStudioPlatform(model);
    }
    
    NSLog(@"Unknown Identifier: %@", model);
    
    return model;
}

@end

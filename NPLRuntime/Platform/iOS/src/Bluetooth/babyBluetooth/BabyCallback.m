//-----------------------------------------------------------------------------
// Class: BabyCallback.m
// Authors: kkvskkkk, big
// CreateDate: 2018.11.6
// ModifyDate: 2023.10.19
//-----------------------------------------------------------------------------

#import "BabyCallback.h"

@implementation BabyCallback

- (instancetype)init {
    self = [super init];
    if (self) {
        [self setFilterOnDiscoverPeripherals:^BOOL(NSString *peripheralName, NSDictionary *advertisementData, NSNumber *RSSI) {
            if (![peripheralName isEqualToString:@""]) {
                return YES;
            }
            return NO;
        }];
        [self setFilterOnconnectToPeripherals:^BOOL(NSString *peripheralName, NSDictionary *advertisementData, NSNumber *RSSI) {
            if (![peripheralName isEqualToString:@""]) {
                return YES;
            }
            return NO;
        }];
    }
    return self;
}
@end

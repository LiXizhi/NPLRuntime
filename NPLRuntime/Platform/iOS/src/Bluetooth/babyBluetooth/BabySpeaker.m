//-----------------------------------------------------------------------------
// Class: BabySpeaker.m
// Authors: kkvskkkk, big
// CreateDate: 2018.11.6
// ModifyDate: 2023.10.20
//-----------------------------------------------------------------------------

#import "BabySpeaker.h"
#import "BabyDefine.h"

typedef NS_ENUM(NSUInteger, BabySpeakerType) {
    BabySpeakerTypeDiscoverPeripherals,
    BabySpeakerTypeConnectedPeripheral,
    BabySpeakerTypeDiscoverPeripheralsFailToConnect,
    BabySpeakerTypeDiscoverPeripheralsDisconnect,
    BabySpeakerTypeDiscoverPeripheralsDiscoverServices,
    BabySpeakerTypeDiscoverPeripheralsDiscoverCharacteristics,
    BabySpeakerTypeDiscoverPeripheralsReadValueForCharacteristic,
    BabySpeakerTypeDiscoverPeripheralsDiscoverDescriptorsForCharacteristic,
    BabySpeakerTypeDiscoverPeripheralsReadValueForDescriptorsBlock
};

@implementation BabySpeaker {
    // 所有委托频道
    NSMutableDictionary *channels;
    // 当前委托频道
    NSString *currChannel;
    // notifyList
    NSMutableDictionary *notifyList;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        BabyCallback *defaultCallback = [[BabyCallback alloc]init];
        notifyList = [[NSMutableDictionary alloc]init];
        channels = [[NSMutableDictionary alloc]init];
        currChannel = KBABY_DETAULT_CHANNEL;
        [channels setObject:defaultCallback forKey:KBABY_DETAULT_CHANNEL];
    }
    return self;
}

- (BabyCallback *)callback {
    return [channels objectForKey:KBABY_DETAULT_CHANNEL];
}

- (BabyCallback *)callbackOnCurrChannel {
    return [self callbackOnChnnel:currChannel];
}

- (BabyCallback *)callbackOnChnnel:(NSString *)channel {
    if (!channel) {
        [self callback];
    }
    return [channels objectForKey:channel];
}

- (BabyCallback *)callbackOnChnnel:(NSString *)channel createWhenNotExist:(BOOL)createWhenNotExist {
    BabyCallback *callback = [channels objectForKey:channel];

    if (!callback && createWhenNotExist) {
        callback = [[BabyCallback alloc]init];
        [channels setObject:callback forKey:channel];
    }
    
    return callback;
}

- (void)switchChannel:(NSString *)channel {
    if (channel) {
        if ([self callbackOnChnnel:channel]) {
            currChannel = channel;
            BabyLog(@">>>已切换到%@",channel);
        } else {
            BabyLog(@">>>所要切换的channel不存在");
        }
    } else {
        currChannel = KBABY_DETAULT_CHANNEL;
            BabyLog(@">>>已切换到默认频道");
    }
}

// 添加到notify list
- (void)addNotifyCallback:(CBCharacteristic *)c
    withBlock:(void(^)(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error))block {
    [notifyList setObject:block forKey:c.UUID.description];
}

// 添加到notify list
- (void)removeNotifyCallback:(CBCharacteristic *)c {
    [notifyList removeObjectForKey:c.UUID.description];
}

// 获取notify list
- (NSMutableDictionary *)notifyCallBackList {
    return notifyList;
}

// 获取notityBlock
- (void(^)(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error))notifyCallback:(CBCharacteristic *)c {
    return [notifyList objectForKey:c.UUID.description];
}

@end

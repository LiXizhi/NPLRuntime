

#import "BabyCallback.h"
#import <CoreBluetooth/CoreBluetooth.h>


@interface BabySpeaker : NSObject

- (BabyCallback *)callback;
- (BabyCallback *)callbackOnCurrChannel;
- (BabyCallback *)callbackOnChnnel:(NSString *)channel;
- (BabyCallback *)callbackOnChnnel:(NSString *)channel
               createWhenNotExist:(BOOL)createWhenNotExist;

//切换频道
- (void)switchChannel:(NSString *)channel;

//添加到notify list
- (void)addNotifyCallback:(CBCharacteristic *)c
           withBlock:(void(^)(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error))block;

//添加到notify list
- (void)removeNotifyCallback:(CBCharacteristic *)c;

//获取notify list
- (NSMutableDictionary *)notifyCallBackList;

//获取notityBlock
- (void(^)(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error))notifyCallback:(CBCharacteristic *)c;

@end

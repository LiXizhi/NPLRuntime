//-----------------------------------------------------------------------------
// Class: InterfaceBluetooth.h
// Authors: big
// CreateDate: 2024.04.24
//-----------------------------------------------------------------------------
#pragma once
//#import <Cocoa/Cocoa.h>
#import "../babyBluetooth/BabyBluetooth.h"
#import <CoreBluetooth/CoreBluetooth.h>

@interface InterfaceBluetooth:NSObject{
    @public
    BabyBluetooth *bblue;
    bool connected;
    bool openBlued;
}

@property (nonatomic,strong)CBPeripheral *currPeripheral;
@property (nonatomic,strong)CBCentralManager *cbCentralMgr;

+ (void)setDeviceName:(NSDictionary *)dict;
+ (void)setCharacteristicsUuid:(NSDictionary *)dict;
+ (void)setupBluetoothDelegate;

+ (void)registerLuaCall:(NSDictionary *)dict;
+ (void)reconnectBlueTooth:(NSDictionary *)dict;
+ (void)disconnectBlueTooth:(NSDictionary *)dict;
+ (void)linkDevice:(NSDictionary *)dict;
+ (void)writeToCharacteristic:(NSDictionary *)dict;
+ (NSString *)characteristicGetStrValue:(NSDictionary *)dict;
+ (void)readAllBlueGatt:(NSDictionary *)dict;
+ (void)readCharacteristic:(NSDictionary *)dict;
+ (void)setCharacteristicNotification:(NSDictionary *)dict;
+ (void)_setCharacteristicNotification:(CBCharacteristic*)characteristic;
+ (void)stopBlu;
+ (instancetype)shareInstance;

+ (CBCharacteristic *)getCharacteristic:(NSString *)ser_uuid _:(NSString *)char_uuid;

@end

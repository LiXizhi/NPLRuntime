//-----------------------------------------------------------------------------
// Class: InterfaceBluetooth.h
// Authors: kkvskkkk, big
// CreateDate: 2018.11.6
// ModifyDate: 2023.10.20
//-----------------------------------------------------------------------------

/****************************************************************************
 Copyright (c) 2010-2011 cocos2d-x.org
 Copyright (c) 2010      Ricardo Quesada
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#import <UIKit/UIKit.h>
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



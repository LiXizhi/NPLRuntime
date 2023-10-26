//-----------------------------------------------------------------------------
// Class: InterfaceBluetooth.mm
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

#include <sstream>

#import <Foundation/Foundation.h>
#import "InterfaceBluetooth.h"

#import "../../LuaObjcBridge/CCLuaObjcBridge.h"

#include "json/json.h"

#define channelOnRootView @"rootView"

static int CHECK_DEVICE = 1101;
static int SET_BLUE_STATUS = 1102;
static int ON_READ_CHARACTERISTIC_FINSH = 1103;
static int ON_CHARACTERISTIC = 1104;
static int ON_DESCRIPTOR = 1105;
static int ON_READ_ALL_GATT_UUID = 1106;

@interface InterfaceBluetooth () {
    // NSMutableArray *peripheralDataArray;
    // NSString* logitowdevice;
    // CBUUID *SERVICE_UUID;
    // CBUUID *CLIENT_UUID;
}

@end

@implementation InterfaceBluetooth

- (id)init
{
    if(self = [super init])
    {
        connected = false;
    }

    bblue = [BabyBluetooth shareBabyBluetooth];

    return self;
}

- (void)viewWillDisappear:(BOOL)animated
{
    NSLog(@"RootViewController,viewWillDisappear");
}

static InterfaceBluetooth *s_interfaceBluetooth = nil;
static std::string s_luaPath = "";
static std::string s_lastAddr = "";

static std::map<std::string, CBPeripheral*> s_peripherals;

NSString *s_checkDeviceName = NULL;
static std::map<std::string, std::vector<std::string>> s_checkUuids;

static void callBaseBridge(const int& pId, const std::string& extData)
{
    char tempArray[20];
    sprintf(tempArray, "%d", pId);
    std::string mergeData = "msg = [[";
    mergeData = mergeData + tempArray + "_" + extData + "]]";
    ParaEngine::LuaObjcBridge::nplActivate(mergeData.c_str(), s_luaPath); //"(gl)Mod/PluginBlueTooth/main.lua");
}

+ (void)registerLuaCall:(NSDictionary *) dict
{
    s_interfaceBluetooth = [InterfaceBluetooth shareInstance];
    s_luaPath = [dict[@"luaPath"] UTF8String];
}

+ (instancetype)shareInstance
{
    static dispatch_once_t onceToken ;
    dispatch_once(&onceToken, ^{
        s_interfaceBluetooth = [[self alloc] init] ;
    });
    
    return s_interfaceBluetooth;
}

// 重新连接蓝牙
+ (void)reconnectBlu:(NSDictionary *) dict
{
    NSLog(@"%@",@"reconnectBlu true");
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];

    if (_self->connected) {
        NSLog(@"%@",@"reconnectBlu true");
        NSString *statstr = @"1";
        //[LogitowAppManager setBlueStatus:statstr];
        return;
    } else {
        if (_self->bblue != nil) {
            NSLog(@"%@",@"reconnectBlu false");
            _self.cbCentralMgr = [[CBCentralManager alloc] initWithDelegate:nil queue:nil];
            //[bblue cancelAllPeripheralsConnection];
            _self->bblue.scanForPeripherals().begin();
        }
    }
}

+ (void)setupBluetoothDelegate
{
    InterfaceBluetooth *_self = [InterfaceBluetooth shareInstance];
    _self->bblue = [BabyBluetooth shareBabyBluetooth];
    // 设置蓝牙委托
    [self blueDelegate];
}

+ (void)setDeviceName:(NSDictionary *) dict
{
    s_checkDeviceName = dict[@"name"];
}

+ (void)setCharacteristicsUuid:(NSDictionary *) dict
{
    std::string serUuid = [dict[@"serUuid"] UTF8String];
    transform(serUuid.begin(), serUuid.end(), serUuid.begin(), ::toupper);
    std::string chaUuid = [dict[@"chaUuid"] UTF8String];
    transform(chaUuid.begin(), chaUuid.end(), chaUuid.begin(), ::toupper);
    auto itr = s_checkUuids.find(serUuid);
    if (itr == s_checkUuids.end()) {
        std::vector<std::string> chaUuids;
        chaUuids.push_back(chaUuid);
        s_checkUuids.insert(std::make_pair(serUuid, chaUuids));
    } else {
        itr->second.push_back(chaUuid);
    }
}

+ (void)linkDevice:(NSDictionary *) dict
{
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
    [_self->bblue cancelScan];
    if (dict[@"addr"] != nil) {
        s_lastAddr = [dict[@"addr"] UTF8String];
    }
    if (!s_lastAddr.empty()) {
        _self.currPeripheral = s_peripherals[s_lastAddr];
        _self->bblue.having(_self.currPeripheral).connectToPeripherals().discoverServices().discoverCharacteristics().readValueForCharacteristic().discoverDescriptorsForCharacteristic().readValueForDescriptors().begin();
    }
}

+ (void)writeToCharacteristic:(NSDictionary *) dict
{
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
    std::string wdata = [dict[@"writeByte"] UTF8String];

    auto characteristic = [InterfaceBluetooth getCharacteristic:dict[@"ser_uuid"] _:dict[@"cha_uuid"]];
    if (characteristic != NULL) {
        size_t bsLen = wdata.length() / 2;
        Byte *bs = new Byte[bsLen];
        for(int i = 0; i < bsLen; i++) {
            std::string subStr = wdata.substr(i * 2, i * 2 + 2);
            std::stringstream ss;
            unsigned int bit;
            ss << std::hex << subStr;
            ss >> bit;
            bs[i] = (Byte)bit;
        }

        NSData *data = [NSData dataWithBytes:bs length:bsLen];
        [_self.currPeripheral writeValue:data forCharacteristic:characteristic type:CBCharacteristicWriteWithResponse];
        delete [] bs;
    }
}

+ (NSString *)characteristicGetStrValue:(NSDictionary *) dict
{
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
    auto characteristic = [InterfaceBluetooth getCharacteristic:dict[@"ser_uuid"] _:dict[@"cha_uuid"]];
    if (characteristic != NULL) {
        NSString * result = [[[[NSString stringWithFormat:@"%@",characteristic.value]
            stringByReplacingOccurrencesOfString: @"<" withString: @""]
            stringByReplacingOccurrencesOfString: @">" withString: @""]
            stringByReplacingOccurrencesOfString: @" " withString: @""];
        
        // NSDictionary* empty_ret = [NSDictionary dictionaryWithObjectsAndKeys:result, @"value"];
        // return empty_ret;
        return result;
    }
    
    // NSDictionary* empty_ret = [NSDictionary dictionary];
    // return empty_ret;
    return @"";
}

+ (void)readCharacteristic:(NSDictionary *) dict
{
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
    auto characteristic = [InterfaceBluetooth getCharacteristic:dict[@"ser_uuid"] _:dict[@"cha_uuid"]];
    if (characteristic != NULL) {	
		_self->bblue.channel(channelOnRootView).characteristicDetails(_self.currPeripheral, characteristic);
	}
}

+ (void)setCharacteristicNotification:(NSDictionary *) dict
{
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
    auto characteristic = [InterfaceBluetooth getCharacteristic:dict[@"ser_uuid"] _:dict[@"cha_uuid"]];
    if (characteristic != NULL) {	
		[InterfaceBluetooth _setCharacteristicNotification:characteristic];
	}
}

// 停止搜索蓝牙
+ (void)stopBlu
{
    InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
    [_self->bblue cancelAllPeripheralsConnection];
    [_self->bblue cancelScan];
    //bblue.scanForPeripherals().stop(1);
}

+ (CBCharacteristic *)getCharacteristic:(NSString *)ser_uuid _:(NSString *)char_uuid
{
    CBCharacteristic *ret = NULL;
    InterfaceBluetooth *_self = [InterfaceBluetooth shareInstance];

    if (_self.currPeripheral == NULL)
        return ret;

	if (char_uuid == NULL || ser_uuid == NULL)
		return ret;

	NSString *cha = [char_uuid uppercaseString];
    NSString *ser = [ser_uuid uppercaseString];

    for (CBService *s in _self.currPeripheral.services) {
        if ([ser isEqualToString:s.UUID.UUIDString]) {
            for (CBCharacteristic *c in s.characteristics) {
                if ([cha isEqualToString:c.UUID.UUIDString]) {
                    ret = c;
                    return ret;
                }
            }
        }
    }

    return ret;
}

+ (std::string)characteristicData2JsStrValue:(CBCharacteristic *)characteristics
{
    NSString * result = [[[[NSString stringWithFormat:@"%@", characteristics.value]
        stringByReplacingOccurrencesOfString: @"<" withString: @""]
        stringByReplacingOccurrencesOfString: @">" withString: @""]
        stringByReplacingOccurrencesOfString: @" " withString: @""];
    NSLog(@"bluetooth1:%@", result);
    const char *anm = [result UTF8String];

    Json::Value luajs_value2;
    luajs_value2["len"] = Json::UInt(characteristics.value.length);
    luajs_value2["data"] = anm;
    Json::FastWriter writer2;
    std::string cv_jsonstr = writer2.write(luajs_value2);
    return cv_jsonstr;
}

// 设置蓝牙委托
+ (void)blueDelegate
{
    InterfaceBluetooth *_self = [InterfaceBluetooth shareInstance];
    [_self->bblue setBlockOnCentralManagerDidUpdateState:^(CBCentralManager *central) {
        if (central.state == CBCentralManagerStatePoweredOn) {
            NSLog(@"设备打开成功，开始扫描设备");
        }
    }];

    // 设置扫描到设备的委托
    [_self->bblue setBlockOnDiscoverToPeripherals:^(CBCentralManager *central, CBPeripheral *peripheral, NSDictionary *advertisementData, NSNumber *RSSI) {
        Json::Value luajs_value;

        luajs_value["rssi"] = Json::Value([RSSI intValue]);
        luajs_value["name"] = [peripheral.name UTF8String];
        luajs_value["addr"] = luajs_value["name"];

        s_peripherals[[peripheral.name UTF8String]] = peripheral;

        Json::FastWriter writer;
        std::string jsonstr = writer.write(luajs_value);
        callBaseBridge(CHECK_DEVICE, jsonstr);
    }];

    // 设置发现设service的Characteristics的委托
    [_self->bblue setBlockOnDiscoverCharacteristics:^(CBPeripheral *peripheral, CBService *service, NSError *error) {
        NSLog(@"===service name:%@",service.UUID.UUIDString);
    }];

    // 设置读取characteristics的委托
    [_self->bblue setBlockOnReadValueForCharacteristic:^(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error) {
        NSLog(@"characteristic name:%@ value is:%@", characteristics.UUID,characteristics.value);

	    Json::Value luajs_value;

	    luajs_value["uuid"] = [characteristics.UUID.UUIDString UTF8String];
	    luajs_value["io"] = "r";
	    luajs_value["status"] = "";
        luajs_value["data"] = [InterfaceBluetooth characteristicData2JsStrValue:characteristics];

	    Json::FastWriter writer;
	    std::string jsonstr = writer.write(luajs_value);

	    callBaseBridge(ON_CHARACTERISTIC, jsonstr);		
    }];

    // 设置发现characteristics的descriptors的委托
    [_self->bblue setBlockOnDiscoverDescriptorsForCharacteristic:^(CBPeripheral *peripheral, CBCharacteristic *characteristic, NSError *error) {
        NSLog(@"===characteristic name:%@",characteristic.UUID);
        for (CBDescriptor *d in characteristic.descriptors) {
            NSLog(@"CBDescriptor name is :%@",d.UUID);
        }
    }];

    // 设置读取Descriptor的委托
    [_self->bblue setBlockOnReadValueForDescriptors:^(CBPeripheral *peripheral, CBDescriptor *descriptor, NSError *error) {
        NSLog(@"Descriptor name:%@ value is:%@",descriptor.characteristic.UUID, descriptor.value);

        if (_self->connected != true) {
            [InterfaceBluetooth setNotifiy];
            callBaseBridge(SET_BLUE_STATUS, "1");
            _self->connected = true;
        }
    }];

    // 设置查找设备的过滤器
    [_self->bblue setFilterOnDiscoverPeripherals:^BOOL(NSString *peripheralName, NSDictionary *advertisementData, NSNumber *RSSI) {
        // 最常用的场景是查找某一个前缀开头的设备
        if (s_checkDeviceName != NULL && [peripheralName hasPrefix:s_checkDeviceName] ) {
            return YES;
        }
        return NO;
    }];

    // [bblue setBlockOnCancelAllPeripheralsConnectionBlock:^(CBCentralManager *centralManager) {
    //     NSLog(@"setBlockOnCancelAllPeripheralsConnectionBlock");
    // }];

    // [bblue setBlockOnCancelScanBlock:^(CBCentralManager *centralManager) {
    //     NSLog(@"setBlockOnCancelScanBlock");
    // }];
    
    // 设置设备连接成功的委托,同一个baby对象，使用不同的channel切换委托回调
    [_self->bblue setBlockOnConnectedAtChannel:channelOnRootView block:^(CBCentralManager *central, CBPeripheral *peripheral) {
        NSLog(@"设备：%@--连接成功");
    }];

    [_self->bblue setBlockOnConnected:^(CBCentralManager *central, CBPeripheral *peripheral) {
        NSLog(@"设备：%@--连接成功", peripheral.name);
    }];

    // 设置设备连接失败的委托
    [_self->bblue setBlockOnFailToConnectAtChannel:channelOnRootView block:^(CBCentralManager *central, CBPeripheral *peripheral, NSError *error) {
        NSLog(@"设备：%@--连接失败", peripheral.name);
        NSString *statstr = @"0";
    }];

    // 设置设备断开连接的委托
    [_self->bblue setBlockOnDisconnectAtChannel:channelOnRootView block:^(CBCentralManager *central, CBPeripheral *peripheral, NSError *error) {
        NSLog(@"设备：%@--断开连接1", peripheral.name);
        _self->connected = false;
        callBaseBridge(SET_BLUE_STATUS, "0");
        
        NSDictionary *dictNil = @{};
        [InterfaceBluetooth linkDevice:dictNil];
    }];

    [_self->bblue setBlockOnDiscoverServicesAtChannel:channelOnRootView block:^(CBPeripheral *peripheral, NSError *error) { }];
 
    [_self->bblue setBlockOnReadValueForCharacteristicAtChannel:channelOnRootView block:^(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error) {
        NSLog(@"CharacteristicViewController===characteristic name:%@ value is:%@", characteristics.UUID, characteristics.value);
    }];

    // 设置写数据成功的block
    [_self->bblue setBlockOnDidWriteValueForCharacteristicAtChannel:channelOnRootView block:^(CBCharacteristic *characteristic, NSError *error) {
        NSLog(@"setBlockOnDidWriteValueForCharacteristicAtChannel characteristic:%@ and new value:%@", characteristic.UUID, characteristic.value);
    }];

    // 示例:
    // 扫描选项->CBCentralManagerScanOptionAllowDuplicatesKey:忽略同一个Peripheral端的多个发现事件被聚合成一个发现事件
    NSDictionary *scanForPeripheralsWithOptions = @{CBCentralManagerScanOptionAllowDuplicatesKey:@YES};

    // 连接设备->
    [_self->bblue
         setBabyOptionsWithScanForPeripheralsWithOptions:scanForPeripheralsWithOptions
         connectPeripheralWithOptions:nil
         scanForPeripheralsWithServices:nil
         discoverWithServices:nil
         discoverWithCharacteristics:nil];
}

// 订阅一个值
+ (void)setNotifiy
{
    // NSLog(@"setNotifiy");
    InterfaceBluetooth *_self = [InterfaceBluetooth shareInstance];
    
    if (_self.currPeripheral.state != CBPeripheralStateConnected) {
        NSLog(@"peripheral已经断开连接，请重新连接");
        return;
    }
    
    for (auto &s_itr: s_checkUuids)
    {
        NSString *ser_uuid_nstr = [NSString stringWithCString:s_itr.first.c_str() encoding:[NSString defaultCStringEncoding]];
        
        for (auto &c_itr: s_itr.second)
        {
            NSString *cha_uuid_nstr = [NSString stringWithCString:c_itr.c_str() encoding:[NSString defaultCStringEncoding]];
            CBCharacteristic *c = [InterfaceBluetooth getCharacteristic:ser_uuid_nstr _:cha_uuid_nstr];
            if (c != NULL) {
                [InterfaceBluetooth _setCharacteristicNotification:c];
            }
        }
    }

	callBaseBridge(ON_READ_CHARACTERISTIC_FINSH, "");
}

+ (void)readAllBlueGatt:(NSDictionary *) dict
{
    InterfaceBluetooth *_self = [InterfaceBluetooth shareInstance];
    Json::Value luajs_value;
    for (CBService *s in _self.currPeripheral.services)
    {
        Json::Value child_s_js;
        for (CBCharacteristic *c in s.characteristics)
        {
            Json::Value child_c_js;
            for (CBDescriptor *d in c.descriptors)
            {
                child_c_js[[d.UUID.UUIDString UTF8String]] = "";
            }
            child_s_js[[c.UUID.UUIDString UTF8String]] = child_c_js;
        }
        luajs_value[[s.UUID.UUIDString UTF8String]] = child_s_js;
    }

    Json::FastWriter writer;
    std::string jsonstr = writer.write(luajs_value);
    callBaseBridge(ON_READ_ALL_GATT_UUID, jsonstr);
}

+ (void)_setCharacteristicNotification:(CBCharacteristic*)characteristic
{
	InterfaceBluetooth* _self = [InterfaceBluetooth shareInstance];
	// 读取服务
	_self->bblue.channel(channelOnRootView).characteristicDetails(_self.currPeripheral, characteristic);
	if (characteristic.properties & CBCharacteristicPropertyNotify ||  characteristic.properties & CBCharacteristicPropertyIndicate) 
	{
		if(!characteristic.isNotifying) 
		{
			[_self->bblue notify:_self.currPeripheral
                characteristic:characteristic
                block:^(CBPeripheral *peripheral, CBCharacteristic *characteristics, NSError *error) {
                    NSLog(@"new value %@",characteristics.value);
                    // 应用状态LogitowAppManager

                    NSString * result = [[[[NSString stringWithFormat:@"%@",characteristics.value]
                        stringByReplacingOccurrencesOfString: @"<" withString: @""]
                        stringByReplacingOccurrencesOfString: @">" withString: @""]
                        stringByReplacingOccurrencesOfString: @" " withString: @""];
                    NSLog(@"bluetooth1:%@",result);
                    const char * anm =[result UTF8String];

                    Json::Value luajs_value;

                    luajs_value["uuid"] = [characteristics.UUID.UUIDString UTF8String];
                    luajs_value["io"] = "c";
                    luajs_value["status"] = "";
                    luajs_value["data"] = [InterfaceBluetooth characteristicData2JsStrValue:characteristics];

                    Json::FastWriter writer;
                    std::string jsonstr = writer.write(luajs_value);

                    callBaseBridge(ON_CHARACTERISTIC, jsonstr);
                }];
		}
	} else {
		NSLog(@"这个characteristic没有nofity的权限");
		return;
	}
}

// Override to allow orientations other than the default portrait orientation.
// This method is deprecated on ios6
+ (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return UIInterfaceOrientationIsLandscape(interfaceOrientation);
}

// For ios6, use supportedInterfaceOrientations & shouldAutorotate instead
+ (NSUInteger)supportedInterfaceOrientations
{
#ifdef __IPHONE_6_0
    return UIInterfaceOrientationMaskAllButUpsideDown;
#endif
}

- (BOOL)shouldAutorotate
{
    return YES;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    //[super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
}

// fix not hide status on ios7
- (BOOL)prefersStatusBarHidden
{
    return YES;
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    //[super didReceiveMemoryWarning];

    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload
{
    //[super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)dealloc
{
}

@end

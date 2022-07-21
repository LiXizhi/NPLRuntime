//
//  NSHost+Hardware.h
//  macdevice
//
//  Created by 李坚 on 2021/10/8.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSHost (Hardware)

- (NSString *)model;
- (NSString *)generation;

@end

NS_ASSUME_NONNULL_END

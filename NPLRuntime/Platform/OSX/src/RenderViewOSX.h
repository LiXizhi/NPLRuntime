#pragma once
#import <Cocoa/Cocoa.h>

@interface RenderView : NSView<NSTextInputClient>
{
    NSTrackingArea *trackingArea;
    NSMutableAttributedString *markedText;
    NSMutableArray *pressedKeys;
}

- (instancetype)initWithFrame:(NSRect)frameRect;

@end

#pragma once
#import <Cocoa/Cocoa.h>

@interface RenderView : NSView<NSTextInputClient>
{
    NSTrackingArea *trackingArea;
    NSMutableAttributedString *markedText;
    NSMutableArray *pressedKeys;
    BOOL imeEnabled;
}

- (instancetype)initWithFrame:(NSRect)frameRect;
- (void)setIMEEnabled:(BOOL)enabled;
- (BOOL)isIMEEnabled;

@end

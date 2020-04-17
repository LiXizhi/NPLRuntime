#pragma once
#import <Cocoa/Cocoa.h>

@interface RenderView : NSView<NSTextInputClient>
{
    NSTrackingArea* trackingArea;
    NSMutableAttributedString* markedText;
}

- (instancetype)initWithFrame:(NSRect)frameRect;

@end

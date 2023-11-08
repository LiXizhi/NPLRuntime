#include "RenderViewOSX.h"
#include "ParaEngine.h"
#include "RenderWindowOSX.h"

static const NSRange kEmptyRange = { NSNotFound, 0 };

@implementation RenderView

- (instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame: frameRect];
    
    if (self != nil)
    {
        trackingArea = nil;
        markedText = [[NSMutableAttributedString alloc] init];
        pressedKeys = [[NSMutableArray alloc] init];
        
        [self updateTrackingAreas];
    }
    
    return self;
}

- (void)dealloc
{
    [trackingArea release];
    [markedText release];
    [super dealloc];
}

+ (NSWindow*)getWindowObject
{
    auto pApp = ParaEngine::CGlobals::GetApp();
    if (pApp)
    {
        auto pRenderWindow = pApp->GetRenderWindow();
        
        if (pRenderWindow)
        {
            return (NSWindow *)pRenderWindow->GetNativeHandle();
        }
    }
    
    return nil;
}

+ (ParaEngine::RenderWindowOSX*) getRenderWindow
{
    auto pApp = ParaEngine::CGlobals::GetApp();
    if (pApp)
    {
        auto pRenderWindow = pApp->GetRenderWindow();
        return (ParaEngine::RenderWindowOSX*)pRenderWindow;
    }
    
    return nil;
}

- (void)updateTrackingAreas
{
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }
    
    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow |  NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate | NSTrackingInVisibleRect | NSTrackingAssumeInside;
    
    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];
    
    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

- (BOOL)isOpaque
{
    return [[RenderView getWindowObject] isOpaque];
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

/*
- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{
    
}
 
- (void)cursolUpdate:(NSEvent *)event
 {
 
 }
 */

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseDown(ParaEngine::EMouseButton::LEFT, event);
    }
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseUp(ParaEngine::EMouseButton::LEFT, event);
    }
}

- (void)mouseMoved:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseMove(event);
    }
}

- (void)rightMouseUp:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseUp(ParaEngine::EMouseButton::RIGHT, event);
    }
}

- (void)rightMouseDown:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseDown(ParaEngine::EMouseButton::RIGHT, event);
    }
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseUp(ParaEngine::EMouseButton::MIDDLE, event);
    }
}

- (void)otherMouseDown:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnMouseDown(ParaEngine::EMouseButton::MIDDLE, event);
    }
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseEntered:(NSEvent *)event
{
    
}

- (void)mouseExited:(NSEvent *)event
{
    
}

- (void)keyDown:(NSEvent *)event
{
    if (![self.window.firstResponder isEqual:self]) {
        [self.window makeFirstResponder:self];
    }
    
    int keyCode = [event keyCode];
    NSNumber *keyCodeNumber = [NSNumber numberWithInt:keyCode];
    
    for (NSNumber *curKeyCode in pressedKeys) {
        if (curKeyCode == keyCodeNumber) {
            return;
        }
    }

    [pressedKeys addObject:keyCodeNumber];

    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnKey(ParaEngine::EKeyState::PRESS, event);
    }

    [self interpretKeyEvents: @[event]];
}

- (void)keyUp:(NSEvent *)event
{
    int keyCode = [event keyCode];
    NSNumber *keyCodeNumber = [NSNumber numberWithInt:keyCode];
    
    int index = 0;
    for (NSNumber *curKeyCode in pressedKeys) {
        if (curKeyCode == keyCodeNumber) {
            [pressedKeys removeObjectAtIndex:index];
            break;
        }
        
        index++;
    }

    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
       pRenderWindow->OnKey(ParaEngine::EKeyState::RELEASE, event);
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnFlagsChanged(event);
    }
}

- (void)scrollWheel:(NSEvent *)event
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnScrollWheel(event);
    }
}

- (void)doCommandBySelector: (SEL)selector
{
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationGeneric;
}

- (BOOL)hasMarkedText
{
    return [markedText length] > 0;
}

- (NSRange)markedRange
{
    if ([markedText length] > 0)
        return NSMakeRange(0, [markedText length] - 1);
    else
        return kEmptyRange;
}

- (NSRange)selectedRange
{
    return kEmptyRange;
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    [markedText release];
    if ([string isKindOfClass:[NSAttributedString class]])
        markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
    [[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        return pRenderWindow->GetCharacterRect();
    }
    else
    {
        return NSMakeRect(0, 0, 0, 0);
    }
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    NSString* characters;
//    NSEvent* event = [NSApp currentEvent];
    
    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*)string;
    
    auto pRenderWindow = [RenderView getRenderWindow];
    if (pRenderWindow)
    {
        pRenderWindow->OnInsertText(characters);
    }
}

@end

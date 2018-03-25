#include "ParaEngine.h"
#include "WindowDelegate.h"

@implementation WindowDelegate

- (id)InitWithRenderWindow:(ParaEngine::RenderWindowOSX *)window
{
    self = [super init];
    _renderWindow = window;
    return self;
}

- (BOOL) windowShouldClose:(NSWindow *)sender
{
    return _renderWindow->OnShouldClose();
}

@end

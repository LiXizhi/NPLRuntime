#import <Cocoa/Cocoa.h>
#include "RenderViewOSX.h"
#include "RenderWindowOSX.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIContainer.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include <cstdio>

using namespace ParaEngine;

// AppKit has no public constructor for scroll events with gesture phases.
@interface TestScrollEvent : NSEvent
@property NSEventPhase phase;
@property NSEventPhase momentumPhase;
@property CGFloat scrollingDeltaX;
@property CGFloat scrollingDeltaY;
@end

@implementation TestScrollEvent
@synthesize phase, momentumPhase, scrollingDeltaX, scrollingDeltaY;
- (BOOL)hasPreciseScrollingDeltas { return YES; }
- (NSPoint)locationInWindow { return NSMakePoint(300, 300); }
@end

int RunMacTrackpadDragTest(RenderView* view, RenderWindowOSX& window)
{
    auto* root = CGUIRoot::GetInstance();
    auto* mouse = root->GetMouse();
    auto* camera = static_cast<CAutoCamera*>(CGlobals::GetScene()->GetCurrentCamera());
    camera->FreeCameraMode();
    camera->EnableAlwaysRotateCameraWhenFPS(false);
    camera->SetEnableTerrainCollision(false);

    auto frame = [&]() {
        root->HandleUserInput();
        camera->HandleUserInput();
        camera->FrameMove(1.f / 60.f);
    };
    auto* event = [[[TestScrollEvent alloc] init] autorelease];
    auto swipe = [&](NSEventPhase phase, CGFloat dx, CGFloat dy, NSEventPhase momentum = NSEventPhaseNone) {
        event.phase = phase;
        event.momentumPhase = momentum;
        event.scrollingDeltaX = dx;
        event.scrollingDeltaY = dy;
        [view scrollWheel:event];
    };

#define CHECK(condition) do { if (!(condition)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); return 1; } } while (false)
    swipe(NSEventPhaseBegan, 0, 0);
    frame();
    CHECK(camera->IsMouseRButtonDown());
    for (int i = 0; i < 12; ++i)
    {
        const auto before = camera->GetWorldAhead();
        swipe(NSEventPhaseChanged, 1.25, -2);
        swipe(NSEventPhaseChanged, 2.75, -1);
        frame();
        CHECK(mouse->GetMouseXDeltaSteps() == 4 && mouse->GetMouseYDeltaSteps() == -3);
        CHECK(camera->GetWorldAhead() != before);
    }
    frame();
    CHECK(camera->IsMouseRButtonDown() && mouse->GetMouseXDeltaSteps() == 0);
    swipe(NSEventPhaseChanged, -7, 2);
    frame();
    CHECK(mouse->GetMouseXDeltaSteps() == -7 && mouse->GetMouseYDeltaSteps() == 2);
    swipe(NSEventPhaseEnded, 0, 0);
    frame();
    CHECK(!camera->IsMouseRButtonDown());
    swipe(NSEventPhaseNone, 10, 10, NSEventPhaseChanged);
    CHECK(mouse->GetBufferedMessageCount() == 0);

    auto* panel = new CGUIContainer();
    panel->InitObject("scroll_panel", "_fi", 0, 0, 0, 0);
    panel->SetScrollable(true);
    root->AttachGUIElement(root, panel);
    root->SetMouseFocus(panel);
    frame();
    CHECK(root->IsMouseOverScrollableUI());
    swipe(NSEventPhaseBegan, 0, 0);
    swipe(NSEventPhaseChanged, 5, -10);
    swipe(NSEventPhaseEnded, 0, 0);
    swipe(NSEventPhaseNone, 0, -5, NSEventPhaseChanged);
    frame();
    CHECK(!mouse->IsButtonDown(EMouseButton::RIGHT));
    CHECK(mouse->m_dwElements == 2);
    for (unsigned i = 0; i < mouse->m_dwElements; ++i)
    {
        CHECK(mouse->m_didod[i]->GetEventType() == EMouseEventType::Wheel);
        auto* wheel = static_cast<DeviceMouseWheelEvent*>(mouse->m_didod[i].get());
        CHECK(wheel->IsPrecise());
        CHECK(wheel->GetWheel() == (wheel->IsHorizontal() ? 15 : -45));
    }
    root->DestroyChildren();
    frame();
    swipe(NSEventPhaseBegan, 0, 0);
    frame();
    CHECK(camera->IsMouseRButtonDown());
    window.CancelScrollGesture();
    frame();
    CHECK(!camera->IsMouseRButtonDown());
    swipe(NSEventPhaseBegan, 0, 0);
    frame();
    [view rightMouseDown:event];
    swipe(NSEventPhaseCancelled, 0, 0);
    frame();
    CHECK(camera->IsMouseRButtonDown());
    [view rightMouseUp:event];
    frame();
    CHECK(!camera->IsMouseRButtonDown());
    return 0;
}

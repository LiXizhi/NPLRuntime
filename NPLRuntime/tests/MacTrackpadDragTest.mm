#import <Cocoa/Cocoa.h>
#include "RenderViewOSX.h"
#include "RenderWindowOSX.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIContainer.h"
#include "2dengine/GUIKeyboardVirtual.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include <cmath>
#include <cstdio>

using namespace ParaEngine;

// AppKit has no public constructor for scroll events with gesture phases.
@interface TestScrollEvent : NSEvent
@property NSEventPhase phase;
@property NSEventPhase momentumPhase;
@property CGFloat scrollingDeltaX;
@property CGFloat scrollingDeltaY;
@property NSEventModifierFlags modifierFlags;
@end

@implementation TestScrollEvent
@synthesize phase, momentumPhase, scrollingDeltaX, scrollingDeltaY, modifierFlags;
- (BOOL)hasPreciseScrollingDeltas { return YES; }
- (NSPoint)locationInWindow { return NSMakePoint(300, 300); }
@end

@interface TestMagnifyEvent : NSEvent
@property NSEventPhase phase;
@property CGFloat magnification;
@end

@implementation TestMagnifyEvent
@synthesize phase, magnification;
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
    auto ctrlDown = [&]() { return root->GetKeyboard()->IsKeyPressed(EVirtualKey::KEY_LCONTROL); };
    event.modifierFlags = NSEventModifierFlagCommand;
    [view flagsChanged:event];
    CHECK(ctrlDown());
    // Command was released elsewhere. The first ordinary right drag must see Ctrl up.
    event.modifierFlags = 0;
    [view rightMouseDown:event];
    CHECK(!ctrlDown());
    [view rightMouseUp:event];
    frame();

    event.modifierFlags = NSEventModifierFlagControl | NSEventModifierFlagCommand;
    [view flagsChanged:event];
    event.modifierFlags = NSEventModifierFlagControl;
    [view flagsChanged:event];
    [view rightMouseDown:event];
    CHECK(ctrlDown());
    [view rightMouseUp:event];
    frame();
    // A trackpad swipe also generates a right drag and carries its own modifier snapshot.
    event.modifierFlags = 0;
    swipe(NSEventPhaseBegan, 0, 0);
    CHECK(!ctrlDown());
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

    auto* pinchEvent = [[[TestMagnifyEvent alloc] init] autorelease];
    auto pinch = [&](NSEventPhase phase, CGFloat delta) {
        pinchEvent.phase = phase;
        pinchEvent.magnification = delta;
        [view magnifyWithEvent:pinchEvent];
    };
    CGlobals::GetScene()->EnableScene(true);
    camera->SetCameraMode(CameraFollowThirdPerson);
    camera->EnableMouseWheel(false); // Paracraft uses the wheel for inventory.
    camera->SetCameraObjectDistance(8);
    pinch(NSEventPhaseBegan, 0);
    swipe(NSEventPhaseBegan, 1, 1);
    swipe(NSEventPhaseChanged, 4, 4);
    pinch(NSEventPhaseChanged, 0.15);
    CHECK(camera->GetCameraObjectDistance() < 8);
    pinch(NSEventPhaseChanged, -0.30);
    CHECK(camera->GetCameraObjectDistance() > 8);
    pinch(NSEventPhaseEnded, 0.15);
    CHECK(std::abs(camera->GetCameraObjectDistance() - 8) < 1e-6);
    CHECK(mouse->GetBufferedMessageCount() == 0);

    pinch(NSEventPhaseBegan, 0);
    window.CancelScrollGesture();
    pinch(NSEventPhaseChanged, 0.2);
    CHECK(std::abs(camera->GetCameraObjectDistance() - 8) < 1e-6);

    auto* panel = new CGUIContainer();
    panel->InitObject("scroll_panel", "_fi", 0, 0, 0, 0);
    panel->SetScrollable(true);
    root->AttachGUIElement(root, panel);
    root->SetMouseFocus(panel);
    frame();
    CHECK(root->IsMouseOverScrollableUI());
    pinch(NSEventPhaseBegan, 0);
    pinch(NSEventPhaseChanged, 0.2);
    pinch(NSEventPhaseEnded, 0);
    CHECK(std::abs(camera->GetCameraObjectDistance() - 8) < 1e-6);
    camera->FreeCameraMode();
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

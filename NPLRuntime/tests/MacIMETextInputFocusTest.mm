//-----------------------------------------------------------------------------
// File:    MacIMETextInputFocusTest.mm
// Desc:    Verifies macOS RenderView / RenderWindowOSX IME focus gating:
//          1. When starting in the scene with no edit box focused, IME is disabled
//             (inputContext == nil) and WASD keys drive character control without
//             invoking interpretKeyEvents: or showing IME candidates.
//          2. Focusing a CGUIIMEEditBox activates IME composition and commits text.
//          3. Losing focus or hiding/destroying the edit box discards marked text
//             and disables IME again.
//          4. Focusing a non-IME CGUIEditBox (password / command mode) keeps IME
//             disabled while still accepting plain ASCII character input.
//-----------------------------------------------------------------------------
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

#include "ParaEngine.h"
#include "Core/ParaEngineAppBase.h"
#include "ParaWorldAsset.h"
#include "ViewportManager.h"
#include "SceneObject.h"
#include "Framework/Interface/Render/IRenderContext.h"
#include "Framework/Interface/Render/IRenderDevice.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIContainer.h"
#include "2dengine/GUIEdit.h"
#include "2dengine/GUIIMEEditBox.h"
#include "2dengine/GUIKeyboardVirtual.h"
#include "util/StringHelper.h"
#include "RenderViewOSX.h"
#include "RenderWindowOSX.h"
#include "WindowDelegate.h"
#import "LuaObjcBridge/CCLuaObjcBridge.h"

#include <cstdio>
#include <string>

using namespace ParaEngine;

// Minimal stubs for symbols referenced by RenderWindowOSX.mm in headless test builds.
@implementation WindowDelegate
+ (WindowDelegate*)sharedDelegate { return nil; }
+ (void)closeDelegate {}
- (void)setInitialCommandLine:(const std::string&)cmdLine { (void)cmdLine; }
@end

void ParaEngine::LuaObjcBridge::nplActivate(const std::string&, const std::string&) {}

@interface SpyRenderView : RenderView
@property (nonatomic, assign) int interpretKeyEventsCount;
@end

@implementation SpyRenderView
- (void)interpretKeyEvents:(NSArray<NSEvent*>*)eventArray
{
    self.interpretKeyEventsCount += 1;
    [super interpretKeyEvents:eventArray];
}
@end

namespace
{
    class CTestAppOSX : public CParaEngineAppBase
    {
    public:
        CTestAppOSX(IRenderWindow* win, bool withScene)
        {
            m_pRenderWindow = win;
            g_pCurrentApp = this;
            m_pRenderContext = IRenderContext::Create(ERHIBackend::Null);
            if (m_pRenderContext)
            {
                RenderConfiguration cfg;
                cfg.screenWidth = 800;
                cfg.screenHeight = 600;
                cfg.renderWindow = win;
                cfg.preferredBackend = ERHIBackend::Null;
                m_pRenderDevice = m_pRenderContext->CreateDevice(cfg);
                CGlobals::SetRenderDevice(m_pRenderDevice);
            }
            m_pParaWorldAsset.reset(new CParaWorldAsset());
            m_pViewportManager.reset(new CViewportManager());
            m_pGUIRoot.reset(CGUIRoot::CreateInstance());
            if (withScene)
                m_pRootScene.reset(new CSceneObject());
            m_pViewportManager->SetLayout(VIEW_LAYOUT_DEFAULT, m_pRootScene.get(), m_pGUIRoot.get());
            if (m_pGUIRoot.get())
            {
                m_pGUIRoot->OneTimeGUIInit();
            }
            m_nAppState = PEAppState_Ready;
        }
        ~CTestAppOSX() override
        {
            m_pRootScene.reset();
            m_pRenderWindow = nullptr;
            if (m_pGUIRoot.get())
            {
                m_pGUIRoot->DeleteDeviceObjects();
                m_pGUIRoot.reset();
            }
            m_pViewportManager.reset();
            if (m_pParaWorldAsset.get())
            {
                m_pParaWorldAsset->Cleanup();
                m_pParaWorldAsset.reset();
            }
            CGlobals::SetRenderDevice(nullptr);
            SAFE_DELETE(m_pRenderDevice);
            SAFE_DELETE(m_pRenderContext);
            if (g_pCurrentApp == this)
                g_pCurrentApp = nullptr;
        }
    };

    static void AppendWideCharsToUtf8(const std::wstring& sChars, std::string& ioUtf8)
    {
        std::u16string u16;
        u16.reserve(sChars.size());
        for (wchar_t ch : sChars)
            u16.push_back(static_cast<char16_t>(ch));
        std::string chunk;
        if (StringHelper::UTF16ToUTF8(u16, chunk))
            ioUtf8 += chunk;
    }

    class CTestIMEEditBox : public CGUIIMEEditBox
    {
    public:
        std::string receivedUtf8;
        int OnHandleWinMsgChars(const std::wstring& sChars) override
        {
            AppendWideCharsToUtf8(sChars, receivedUtf8);
            return 0;
        }
    };

    class CTestASCIIEditBox : public CGUIEditBox
    {
    public:
        std::string receivedUtf8;
        int OnHandleWinMsgChars(const std::wstring& sChars) override
        {
            AppendWideCharsToUtf8(sChars, receivedUtf8);
            return 0;
        }
    };

    int g_passed = 0;
    int g_failed = 0;

    void Check(bool cond, const char* expr, const char* file, int line)
    {
        if (cond)
        {
            ++g_passed;
        }
        else
        {
            ++g_failed;
            std::fprintf(stderr, "[FAIL] %s:%d: %s\n", file, line, expr);
        }
    }

#define TEST_CHECK(expr) Check((expr), #expr, __FILE__, __LINE__)

    NSEvent* MakeKeyEvent(NSEventType type, unsigned short keyCode, NSString* chars, NSEventModifierFlags flags = 0)
    {
        return [NSEvent keyEventWithType:type
                                location:NSMakePoint(0, 0)
                           modifierFlags:flags
                               timestamp:0.0
                            windowNumber:0
                                 context:nil
                              characters:chars
             charactersIgnoringModifiers:chars
                               isARepeat:NO
                                 keyCode:keyCode];
    }
}

int RunMacTrackpadDragTest(RenderView* view, RenderWindowOSX& window);

int main(int argc, char** argv)
{
    @autoreleasepool
    {
        [NSApplication sharedApplication];

        NSWindow* nsWin = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
                                                       styleMask:NSWindowStyleMaskBorderless
                                                         backing:NSBackingStoreBuffered
                                                           defer:YES] autorelease];
        SpyRenderView* view = [[[SpyRenderView alloc] initWithFrame:NSMakeRect(0, 0, 800, 600)] autorelease];
        [nsWin setContentView:view];
        [nsWin makeFirstResponder:view];

        RenderWindowOSX renderWin(nsWin);
        bool testTrackpad = argc == 2 && std::string(argv[1]) == "--trackpad";
        CTestAppOSX app(&renderWin, testTrackpad);

        CGUIRoot* root = CGUIRoot::GetInstance();
        TEST_CHECK(root != nullptr);
        root->SetEnabled(true);
        root->SetVisible(true);
        root->SetKeyFocus(nullptr);

        if (testTrackpad)
            return RunMacTrackpadDragTest(view, renderWin);

        // 1. Scene startup: no edit box has focus -> IME must be disabled and WASD must not invoke interpretKeyEvents:.
        renderWin.SyncIMEState();
        TEST_CHECK(root->GetUIKeyFocus() == nullptr);
        TEST_CHECK(!renderWin.ShouldAcceptTextInput());
        TEST_CHECK(!renderWin.ShouldEnableIME());
        TEST_CHECK([view isIMEEnabled] == NO);
        TEST_CHECK([view inputContext] == nil);

        const struct { unsigned short vk; NSString* ch; EVirtualKey expectedKey; } wasd[] = {
            { kVK_ANSI_W, @"w", EVirtualKey::KEY_W },
            { kVK_ANSI_A, @"a", EVirtualKey::KEY_A },
            { kVK_ANSI_S, @"s", EVirtualKey::KEY_S },
            { kVK_ANSI_D, @"d", EVirtualKey::KEY_D },
        };
        for (const auto& k : wasd)
        {
            [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, k.vk, k.ch)];
            TEST_CHECK(root->GetKeyboard()->IsKeyPressed(k.expectedKey));
            [view keyUp:MakeKeyEvent(NSEventTypeKeyUp, k.vk, k.ch)];
            TEST_CHECK(!root->GetKeyboard()->IsKeyPressed(k.expectedKey));
        }
        TEST_CHECK(view.interpretKeyEventsCount == 0);
        TEST_CHECK([view hasMarkedText] == NO);

        // 2. Focused container with InputMethodEnabled == false (e.g. System.UI / System.Windows window with no input focused):
        CGUIContainer* hudContainer = new CGUIContainer();
        hudContainer->InitObject("hud_window", "_lt", 0, 0, 400, 300);
        hudContainer->SetCanHaveFocus(true);
        hudContainer->SetInputMethodEnabled(false);
        root->AttachGUIElement(root, hudContainer);
        root->SetKeyFocus(hudContainer);

        renderWin.SyncIMEState();
        TEST_CHECK(root->GetUIKeyFocus() == hudContainer);
        TEST_CHECK(renderWin.ShouldAcceptTextInput());
        TEST_CHECK(!renderWin.ShouldEnableIME());
        TEST_CHECK([view isIMEEnabled] == NO);
        TEST_CHECK([view inputContext] == nil);
        [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, kVK_ANSI_W, @"w")];
        [view keyUp:MakeKeyEvent(NSEventTypeKeyUp, kVK_ANSI_W, @"w")];
        TEST_CHECK(view.interpretKeyEventsCount == 0);

        // 3. Focused CGUIIMEEditBox -> IME activates, accepts marked composition, commits Chinese text, and clears markedText.
        CTestIMEEditBox* imeEdit = new CTestIMEEditBox();
        imeEdit->InitObject("chat_ime_edit", "_lt", 20, 500, 300, 28);
        root->AttachGUIElement(root, imeEdit);
        root->SetKeyFocus(imeEdit);

        renderWin.SyncIMEState();
        TEST_CHECK(root->GetUIKeyFocus() == imeEdit);
        TEST_CHECK(renderWin.ShouldAcceptTextInput());
        TEST_CHECK(renderWin.ShouldEnableIME());
        TEST_CHECK([view isIMEEnabled] == YES);
        TEST_CHECK([view inputContext] != nil);

        [view setMarkedText:@"wsd" selectedRange:NSMakeRange(3, 0) replacementRange:NSMakeRange(NSNotFound, 0)];
        TEST_CHECK([view hasMarkedText] == YES);
        TEST_CHECK(NSEqualRanges([view markedRange], NSMakeRange(0, 3)));

        // While marked text is active, Return/Backspace must go to interpretKeyEvents: without prematurely triggering OnKey.
        root->GetKeyboard()->SetKeyPressed(EVirtualKey::KEY_RETURN, false);
        const int prevInterpretCount = view.interpretKeyEventsCount;
        [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, kVK_Return, @"\r")];
        TEST_CHECK(view.interpretKeyEventsCount == prevInterpretCount + 1);
        TEST_CHECK(!root->GetKeyboard()->IsKeyPressed(EVirtualKey::KEY_RETURN));

        [view insertText:@"我是的" replacementRange:NSMakeRange(NSNotFound, 0)];
        TEST_CHECK([view hasMarkedText] == NO);
        TEST_CHECK(imeEdit->receivedUtf8 == "我是的");

        // 4. Leaving the edit box back to the scene discards any uncommitted marked text and disables IME again.
        [view setMarkedText:@"ni" selectedRange:NSMakeRange(2, 0) replacementRange:NSMakeRange(NSNotFound, 0)];
        TEST_CHECK([view hasMarkedText] == YES);
        root->SetKeyFocus(nullptr);

        const int countBeforeSceneWASD = view.interpretKeyEventsCount;
        [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, kVK_ANSI_D, @"d")];
        [view keyUp:MakeKeyEvent(NSEventTypeKeyUp, kVK_ANSI_D, @"d")];
        TEST_CHECK([view isIMEEnabled] == NO);
        TEST_CHECK([view inputContext] == nil);
        TEST_CHECK([view hasMarkedText] == NO);
        TEST_CHECK(view.interpretKeyEventsCount == countBeforeSceneWASD);
        TEST_CHECK(imeEdit->receivedUtf8 == "我是的");

        // 5. Focused plain CGUIEditBox (InputMethodEnabled == false, e.g. password / slash command) -> IME stays disabled, ASCII still inserts.
        CTestASCIIEditBox* asciiEdit = new CTestASCIIEditBox();
        asciiEdit->InitObject("password_edit", "_lt", 20, 400, 200, 28);
        TEST_CHECK(!asciiEdit->IsInputMethodEnabled());
        root->AttachGUIElement(root, asciiEdit);
        root->SetKeyFocus(asciiEdit);

        renderWin.SyncIMEState();
        TEST_CHECK(renderWin.ShouldAcceptTextInput());
        TEST_CHECK(!renderWin.ShouldEnableIME());
        TEST_CHECK([view isIMEEnabled] == NO);
        TEST_CHECK([view inputContext] == nil);

        [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, kVK_ANSI_Slash, @"/")];
        [view keyUp:MakeKeyEvent(NSEventTypeKeyUp, kVK_ANSI_Slash, @"/")];
        [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, kVK_ANSI_W, @"w")];
        [view keyUp:MakeKeyEvent(NSEventTypeKeyUp, kVK_ANSI_W, @"w")];
        [view keyDown:MakeKeyEvent(NSEventTypeKeyDown, kVK_ANSI_C, @"c", NSEventModifierFlagCommand)];
        [view keyUp:MakeKeyEvent(NSEventTypeKeyUp, kVK_ANSI_C, @"c", NSEventModifierFlagCommand)];
        TEST_CHECK(view.interpretKeyEventsCount == countBeforeSceneWASD);
        TEST_CHECK(asciiEdit->receivedUtf8 == "/w");

        root->SetKeyFocus(nullptr);
        root->DestroyChildren();
    }

    std::printf("[MacIMETextInputFocusTest] %d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}

//-----------------------------------------------------------------------------
// Class: WebView.mm
// Authors: kkvskkkk, big
// Emails: onedou@126.com
// CreateDate: 2018.5.25
// ModifyDate: 2023.5.15
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "LuaObjcBridge/CCLuaObjcBridge.h"

#include "ParaEngine.h"
#include <unordered_map>
#include "RenderWindowOSX.h"
#include "WebView.h"

// A click returns to the game. Once the pointer moves past the drag threshold,
// releasing it only places the button, even if it returns to the starting point.
@interface WebViewReturnButton : NSButton
{
    NSPoint pressPoint;
    NSPoint initialOrigin;
    BOOL dragged;
}
@end

@implementation WebViewReturnButton
- (void)drawRect:(NSRect)dirtyRect
{
    NSRect circle = NSInsetRect(self.bounds, 1, 1);
    NSBezierPath *background = [NSBezierPath bezierPathWithOvalInRect:circle];
    [[NSColor colorWithWhite:0.12 alpha:self.highlighted ? 1.0 : 0.88] setFill];
    [background fill];
    [[NSColor colorWithWhite:1.0 alpha:0.85] setStroke];
    [background stroke];

    CGFloat x = NSMidX(circle);
    CGFloat y = NSMidY(circle);
    NSBezierPath *arrow = [NSBezierPath bezierPath];
    [arrow moveToPoint:NSMakePoint(x + 3, y - 8)];
    [arrow lineToPoint:NSMakePoint(x - 5, y)];
    [arrow lineToPoint:NSMakePoint(x + 3, y + 8)];
    arrow.lineWidth = 3;
    arrow.lineCapStyle = NSLineCapStyleRound;
    arrow.lineJoinStyle = NSLineJoinStyleRound;
    [[NSColor whiteColor] setStroke];
    [arrow stroke];
}

- (void)placeAt:(NSPoint)origin
{
    NSRect bounds = self.superview.bounds;
    origin.x = MAX(NSMinX(bounds), MIN(origin.x, NSMaxX(bounds) - NSWidth(self.frame)));
    origin.y = MAX(NSMinY(bounds), MIN(origin.y, NSMaxY(bounds) - NSHeight(self.frame)));
    [self setFrameOrigin:origin];
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
    [self placeAt:self.frame.origin];
}

- (BOOL)mouseDownCanMoveWindow { return NO; }

- (void)mouseDown:(NSEvent *)event
{
    pressPoint = [self.superview convertPoint:event.locationInWindow fromView:nil];
    initialOrigin = self.frame.origin;
    dragged = NO;
    [self highlight:YES];
}

- (void)mouseDragged:(NSEvent *)event
{
    NSPoint point = [self.superview convertPoint:event.locationInWindow fromView:nil];
    CGFloat dx = point.x - pressPoint.x;
    CGFloat dy = point.y - pressPoint.y;
    if (!dragged && dx * dx + dy * dy < 16)
        return;
    dragged = YES;
    [self highlight:NO];
    [self placeAt:NSMakePoint(initialOrigin.x + dx, initialOrigin.y + dy)];
}

- (void)mouseUp:(NSEvent *)event
{
    [self highlight:NO];
    if (!dragged && NSPointInRect([self convertPoint:event.locationInWindow fromView:nil], self.bounds))
        [self performClick:self];
}
@end

@interface WebViewWindowController : NSWindowController
{
    std::function<void()> _onCloseCallback;
}

@end

@interface WebViewWindowController () <WKNavigationDelegate, WKUIDelegate, WKScriptMessageHandler>
{
    WKWebView *webView;
}
@property(nonatomic, retain) WKWebView *webView;
@property(nonatomic, retain) NSButton *uiCloseBtn;
@property(nonatomic) BOOL closeButtonHidden;

- (void)setCloseCB:(const std::function<void()>&)cb;

@end

@implementation WebViewWindowController
@synthesize webView;

- (void)setCloseCB:(const std::function<void()>&)cb {
    _onCloseCallback = cb;
}

- (void)installReturnButton
{
    NSView *content = webView.superview;
    webView.frame = content.bounds;
    webView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    const CGFloat size = 44;
    WebViewReturnButton *button = [[WebViewReturnButton alloc] initWithFrame:NSMakeRect(0, 0, size, size)];
    button.bordered = NO;
    button.title = @"";
    button.toolTip = NSLocalizedString(@"Return to Paracraft (drag to move)", nil);
    [button setAccessibilityLabel:NSLocalizedString(@"Return to Paracraft", nil)];
    button.target = self;
    button.action = @selector(onCloseBtn);
    [content addSubview:button positioned:NSWindowAbove relativeTo:webView];
    [button placeAt:NSMakePoint(16, NSMidY(content.bounds) - size / 2)];
    self.uiCloseBtn = button;
    [button release];
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation {
}

- (void)webView:(WKWebView *)webView
    decidePolicyForNavigationResponse:(WKNavigationResponse *)navigationResponse
    decisionHandler:(void (^)(WKNavigationResponsePolicy))decisionHandler {
    decisionHandler(WKNavigationResponsePolicyAllow);
}

- (BOOL)windowShouldClose:(id)sender {
    if (_onCloseCallback)
        _onCloseCallback();

    return YES;
}

- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message
{
    if (![message.name isEqualToString:@"activate"]) {
        return;
    }

    NSString *activate = [message.body valueForKey:@"filename"];
    NSString *msg = [message.body valueForKey:@"msg"];

    std::string activateStr = [activate UTF8String];
    std::string msgStr = [msg UTF8String];
    std::string code = "NPL.activate('" + activateStr + "', { msg = [[" + msgStr + "]]});";

    ParaEngine::LuaObjcBridge::nplActivate(code, "");
}

- (void)onCloseBtn
{
    [self.webView setHidden:true];
    [self.uiCloseBtn setHidden:true];
    [self.webView.window makeFirstResponder:self.webView.superview];
    [self.webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"about:blank"]]];
}

- (WKWebView *)webView:(WKWebView *)webView
    createWebViewWithConfiguration:(WKWebViewConfiguration *)configuration
    forNavigationAction:(WKNavigationAction *)navigationAction
    windowFeatures:(WKWindowFeatures *)windowFeatures
{
    if (!navigationAction.targetFrame.isMainFrame) {
        [webView loadRequest:navigationAction.request];
    }

    return nil;
}

@end

namespace ParaEngine {
    static std::unordered_map<std::string, ParaEngineWebView *> webviews;
    static int viewTags = 0;
    static bool isOpenUrlLoaded = false;
    static int openUrlViewTag = 0;

    IParaWebView *IParaWebView::createWebView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }

    IParaWebView *IParaWebView::createSubViewView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }

    ParaEngineWebView *ParaEngineWebView::createWebView(int x, int y, int w, int h)
    {
        ParaEngineWebView *webview = new ParaEngineWebView();
        webview->_createWebView(x, y, w, h);
        webview->viewTag = viewTags;

        webviews.insert(std::make_pair(std::to_string(viewTags), webview));

        viewTags++;
        
        return webview;
    }

    void ParaEngineWebView::openWebView(const std::string &url, bool hideReturnButton)
    {
        ParaEngineWebView *view = isOpenUrlLoaded ? getWebViewByIndex(openUrlViewTag) : nullptr;
        if (!view) {
            view = createWebView(0, 0, 0, 0);
            if (!view)
                return;
            [view->_webViewController installReturnButton];
            view->setAlpha(0.95f);
            openUrlViewTag = view->viewTag;
            isOpenUrlLoaded = true;
        }

        view->hideCloseButton(hideReturnButton);
        view->loadUrl(url);
        view->setVisible(true);
    }

    ParaEngineWebView *ParaEngineWebView::getWebViewByIndex(int viewTag)
    {
        auto it = webviews.find(std::to_string(viewTag));
        if (it == webviews.end())
            return nullptr;
        return it->second;
    }

    void ParaEngineWebView::_createWebView(int x, int y, int w, int h)
    {
        if (!_webViewController)
        {
            NSWindow *renderWindow = (NSWindow *)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
            _webViewController = [[WebViewWindowController alloc] init];

            WKWebViewConfiguration *webViewConfig = [[WKWebViewConfiguration alloc] init];
            [webViewConfig.userContentController addScriptMessageHandler:_webViewController name:@"activate"];

            _webViewController.webView =
                [
                    [WKWebView alloc]
                        initWithFrame:CGRectMake(x, y, w, h)
                        configuration:webViewConfig
                ];

            [renderWindow.contentView addSubview:_webViewController.webView];

            _webViewController.webView.navigationDelegate = _webViewController;
            _webViewController.webView.UIDelegate = _webViewController;

            auto cb = [this]() {
                if (this->_onClose == nullptr) {
                    this->Release();
                } else {
                    if (!this->_onClose())
                        this->Release();
                }
            };
            
            [_webViewController setCloseCB:cb];
        }
    }

    ParaEngineWebView::ParaEngineWebView() : _webViewController(nil)
    {
    }

    ParaEngineWebView::~ParaEngineWebView()
    {
        [_webViewController.uiCloseBtn removeFromSuperview];
        _webViewController.uiCloseBtn = nil;
        [_webViewController.webView stopLoading];
        [_webViewController.webView removeFromSuperview];
        _webViewController.webView = nil;
        [_webViewController release];
        _webViewController = nil;
    }

    void ParaEngineWebView::loadUrl(const std::string &urlString, bool cleanCachedData)
    {
        if (urlString.empty())
            return;
        if (_webViewController)
        {
            NSString *_urlString = @(urlString.c_str());
            _urlString =
                [_urlString
                 stringByAddingPercentEncodingWithAllowedCharacters:
                     [NSCharacterSet characterSetWithCharactersInString:@"`%^{}\"[]|\\<> "].invertedSet];

            if (_webViewController.window)
                [_webViewController.window setTitle:_urlString];
            NSURL *url = [NSURL URLWithString:_urlString];
            if (!url)
                return;

            NSURLRequest *request = nil;

            if (cleanCachedData)
                request = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:60];
            else
                request = [NSURLRequest requestWithURL:url];

            [_webViewController.webView loadRequest:request];
        }
    }

    void ParaEngineWebView::setAlpha(float a)
    {
        if (_webViewController)
        {
            _webViewController.webView.alphaValue = a;
        }
    }

    void ParaEngineWebView::hideCloseButton(bool bHide)
    {
        _webViewController.closeButtonHidden = bHide;
        _webViewController.uiCloseBtn.hidden = bHide || _webViewController.webView.hidden;
    }

    void ParaEngineWebView::setVisible(bool bVisible)
    {
        _webViewController.webView.hidden = !bVisible;
        _webViewController.uiCloseBtn.hidden = !bVisible || _webViewController.closeButtonHidden;
    }

    void ParaEngineWebView::SetHideViewWhenClickBack(bool b)
    {
         if (_webViewController)
         {
         }
    }

    void ParaEngineWebView::SetIgnoreCloseWhenClickBack(bool b)
    {
         if (_webViewController)
         {
         }
    }

    void ParaEngineWebView::SetCloseWhenClickBackground(bool b)
    {
         if (_webViewController)
         {
         }
    }
    
    void ParaEngineWebView::Refresh()
    {
        if (_webViewController)
        {
             [_webViewController.webView reload];
        }
    }
    
    void ParaEngineWebView::addCloseListener(onCloseFunc fun)
    {
        _onClose = fun;
    }
    
    void ParaEngineWebView::bringToTop()
    {
    }

    void ParaEngineWebView::move(int x, int y)
    {
        if (_webViewController)
        {
            NSWindow * renderWindow = (NSWindow *)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
            int webViewHeight = _webViewController.webView.frame.size.height;
            y = [renderWindow contentRectForFrameRect:renderWindow.frame].size.height - (y + webViewHeight);

            [_webViewController.webView setFrameOrigin:NSMakePoint(x, y)];
        }
    }

    void ParaEngineWebView::resize(int width, int height)
    {
        if (_webViewController)
        {
            [_webViewController.webView setFrameSize:NSMakeSize(width, height)];
        }
    }

    void ParaEngineWebView::activate(const std::string &filepath, const std::string &msg)
    {
        NSString *msgStr = [NSString stringWithUTF8String:msg.c_str()];
        NSString *filepathStr = [NSString stringWithCString:filepath.c_str() encoding:[NSString defaultCStringEncoding]];

        NSString *jsStr = [NSString stringWithFormat:@"window.NPL.receive('%@', `%@`)", filepathStr, msgStr];

        if (_webViewController)
        {
            [_webViewController.webView evaluateJavaScript:jsStr completionHandler:nil];
        }
    }
} // end namespcae

//-----------------------------------------------------------------------------
// Class: WebView.mm
// Authors: kkvskkkk, big
// Emails: onedou@126.com
// CreateDate: 2018.5.25
// ModifyDate: 2023.3.23
//-----------------------------------------------------------------------------

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#include "ParaEngine.h"
#import "LuaObjcBridge/CCLuaObjcBridge.h"

#include "WebView.h"

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
//@property (nonatomic) std::function<void()> onCloseCallback;

- (void)setCloseCB:(const std::function<void()>&)cb;

@end

@implementation WebViewWindowController
@synthesize webView;

- (void)setCloseCB:(const std::function<void()>&)cb {
    _onCloseCallback = cb;
}

- (void)autoResize {
    [[NSNotificationCenter defaultCenter] addObserver:self.window selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self];
}

- (void)windowDidResize:(NSNotification*)aNotification {
    auto windowRect = [self.window frame];
    auto contentRect = [self.window contentRectForFrameRect:windowRect];
    [self.window.contentView setFrameSize:contentRect.size];
    [webView setFrameSize:contentRect.size];
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
    std::string code = "NPL.activate(\"" + activateStr + "\", " + msgStr + ");";

    ParaEngine::LuaObjcBridge::nplActivate(code, "");
}

- (void)onCloseBtn
{
    [self.webView setHidden:true];
    [self.uiCloseBtn setHidden:true];
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
    IParaWebView *IParaWebView::createWebView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }

    IParaWebView *IParaWebView::createSubViewView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createSubWebView(x, y, w, h);
    }

    static ParaEngineWebView *webview;
    static ParaEngineWebView *subWebview;

    ParaEngineWebView *ParaEngineWebView::createWebView(int x, int y, int w, int h)
    {
        if (!webview) {
            webview = new ParaEngineWebView();
            webview->openWindow(x, y, w, h, false);
        }

        return webview;
    }

    ParaEngineWebView *ParaEngineWebView::createSubWebView(int x, int y, int w, int h)
    {
        if (!subWebview) {
            subWebview = new ParaEngineWebView();
            subWebview->openWindow(x, y, w, h, true);
        }

        return subWebview;
    }

    void ParaEngineWebView::openWindow(int x, int y, int w, int h, bool bSub)
    {
        if (!_webViewController)
        {
            NSWindow *renderWindow = (NSWindow *)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
            
            _webViewController = [[WebViewWindowController alloc] init];
            
            if (bSub)
            {
                y = [renderWindow contentRectForFrameRect:renderWindow.frame].size.height - (y + h);
                
                WKWebViewConfiguration *webViewConfig = [[WKWebViewConfiguration alloc] init];
                [webViewConfig.userContentController addScriptMessageHandler:_webViewController name:@"activate"];
                
                _webViewController.webView = [[WKWebView alloc] initWithFrame:CGRectMake(x, y, w, h) configuration:webViewConfig];
                
            }
            else
            {
                WKWebViewConfiguration *webViewConfig = [[WKWebViewConfiguration alloc] init];
                
                _webViewController.webView =
                [
                    [WKWebView alloc]
                        initWithFrame:CGRectMake(0, 0, renderWindow.frame.size.width, [renderWindow contentRectForFrameRect:renderWindow.frame].size.height)
                        configuration:webViewConfig
                ];
                
                if (!_webViewController.uiCloseBtn)
                {
                    NSBundle *bundle = [NSBundle mainBundle];
                    NSString *resPath = [bundle resourcePath];
                    resPath = [resPath stringByAppendingPathComponent:@"res/WebViewCloseBtn.png"];
                    
                    NSImage *icon = [[NSImage alloc] initWithContentsOfFile:resPath];
                    float w = icon.size.width;
                    float h = icon.size.height;
                    
                    float iconY = (renderWindow.contentView.frame.size.height - h) / 2;
                    
                    _webViewController.uiCloseBtn = [[NSButton alloc] initWithFrame:CGRectMake(30, iconY, w, h)];
                    [_webViewController.uiCloseBtn setImage:icon];
                    [_webViewController.uiCloseBtn setTarget:_webViewController];
                    [_webViewController.uiCloseBtn setAction:@selector(onCloseBtn)];
                }
            }
            
            [renderWindow.contentView addSubview:_webViewController.webView];
            
            if (_webViewController.uiCloseBtn) {
                [renderWindow.contentView addSubview:_webViewController.uiCloseBtn];
            }
            
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
        _webViewController = nil;
    }

    void ParaEngineWebView::loadUrl(const std::string &urlString, bool cleanCachedData)
    {
        if (_webViewController)
        {
            NSString *_urlString = @(urlString.c_str());
            _urlString = [_urlString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];

            [_webViewController.window setTitle:_urlString];
            NSURL *url = [NSURL URLWithString:_urlString];

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
         if (_webViewController)
         {
         }
    }

    void ParaEngineWebView::setVisible(bool bVisible)
    {
        if (_webViewController)
        {
            [_webViewController.webView setHidden:!bVisible];
            
            if (_webViewController.uiCloseBtn) {
                NSWindow *renderWindow = (NSWindow *)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
                float iconHeight = _webViewController.uiCloseBtn.frame.size.height;
                float iconX = _webViewController.uiCloseBtn.frame.origin.x;
                float iconY = (renderWindow.contentView.frame.size.height - iconHeight) / 2;

                [_webViewController.uiCloseBtn setFrameOrigin:NSMakePoint(iconX, iconY)];
                [_webViewController.uiCloseBtn setHidden:!bVisible];
            }
        }
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

    void ParaEngineWebView::activate(const std::string &msg)
    {
        
        NSString *msgStr = [NSString stringWithCString:msg.c_str() encoding:[NSString defaultCStringEncoding]];
        NSString *jsStr = [NSString stringWithFormat:@"window.NPL.receive(\"%@\")", msgStr];
        
        if (_webViewController)
        {
            [_webViewController.webView evaluateJavaScript:jsStr completionHandler:nil];
        }
    }
} // end namespcae

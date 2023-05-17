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
    std::string code = "NPL.activate('" + activateStr + "', { msg = [[" + msgStr + "]]});";

    ParaEngine::LuaObjcBridge::nplActivate(code, "");
}

- (void)onCloseBtn
{
    [self.webView setHidden:true];
    [self.uiCloseBtn setHidden:true];
    [self.webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@""]]];
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
    static std::unordered_map<std::string, std::shared_ptr<ParaEngineWebView>> webviews;
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

    void ParaEngineWebView::openWebView(const std::string &url)
    {
        if (isOpenUrlLoaded) {
            ParaEngineWebView *pView = getWebViewByIndex(openUrlViewTag);

            if (pView) {
                pView->loadUrl(url);
                pView->setVisible(true);
            }
            
            return;
        }
        
        RenderWindowOSX *pWnd = (RenderWindowOSX *)CGlobals::GetApp()->GetRenderWindow();
        int w = pWnd->GetWidth();
        int h = pWnd->GetHeight();
        int scale = pWnd->currentBackingScaleFactor;
        w = w / scale;
        h = h / scale;

        ParaEngineWebView *pView = createWebView(0, 0, w, h);

        if (!pView)
            return;

        pView->loadUrl(url);
        pView->setVisible(true);
        pView->resize(w, h);
        pView->move(0, 0);
        pView->setAlpha(0.95f);
        
        // add close btn.
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *resPath = [bundle resourcePath];
        resPath = [resPath stringByAppendingPathComponent:@"res/WebViewCloseBtn.png"];

        NSImage *closeBtn = [[NSImage alloc] initWithContentsOfFile:resPath];

        float closeBtnW = closeBtn.size.width;
        float closeBtnH = closeBtn.size.height;

        NSWindow *renderWindow = (NSWindow *)pWnd->GetNativeHandle();
        float iconY = (renderWindow.contentView.frame.size.height - closeBtnH) / 2;

        pView->_webViewController.uiCloseBtn = [[NSButton alloc] initWithFrame:CGRectMake(30, iconY, closeBtnW, closeBtnH)];
        [pView->_webViewController.uiCloseBtn setImage:closeBtn];
        [pView->_webViewController.uiCloseBtn setTarget:pView->_webViewController];
        [pView->_webViewController.uiCloseBtn setAction:@selector(onCloseBtn)];

        [renderWindow.contentView addSubview:pView->_webViewController.uiCloseBtn];

        openUrlViewTag = pView->viewTag;
        isOpenUrlLoaded = true;
    }

    ParaEngineWebView *ParaEngineWebView::getWebViewByIndex(int viewTag)
    {
        ParaEngineWebView *pView;
        auto it = webviews.find(std::to_string(viewTag));

        if (it != webviews.end()) {
            pView = it->second.get();
        }

        if (!pView)
            return;

        return pView;
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
                        initWithFrame:CGRectMake(0, 0, renderWindow.frame.size.width, [renderWindow contentRectForFrameRect:renderWindow.frame].size.height)
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
        _webViewController = nil;
    }

    void ParaEngineWebView::loadUrl(const std::string &urlString, bool cleanCachedData)
    {
        if (_webViewController)
        {
            NSString *_urlString = @(urlString.c_str());
            _urlString =
                [_urlString
                 stringByAddingPercentEncodingWithAllowedCharacters:
                     [NSCharacterSet characterSetWithCharactersInString:@"`%^{}\"[]|\\<> "].invertedSet];

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
//            if (!bVisible) {
//                [_webViewController.webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@""]]];
//            }

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

    void ParaEngineWebView::activate(const std::string &filepath, const std::string &msg)
    {
        NSString *msgStr = [NSString stringWithCString:msg.c_str() encoding:[NSString defaultCStringEncoding]];
        NSString *filepathStr = [NSString stringWithCString:filepath.c_str() encoding:[NSString defaultCStringEncoding]];

        NSString *jsStr = [NSString stringWithFormat:@"window.NPL.receive('%@', '%@')", filepathStr, msgStr];

        if (_webViewController)
        {
            [_webViewController.webView evaluateJavaScript:jsStr completionHandler:nil];
        }
    }
} // end namespcae

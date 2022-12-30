//
//  WebView.mm
//  buildvm21
//
//  Created by apple on 2018/5/25.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#include "ParaEngine.h"
#include "WebView.h"


@interface WebViewWindowController : NSWindowController
{
    std::function<void()> _onCloseCallback;
}

@end

@interface WebViewWindowController () <WKNavigationDelegate, WKUIDelegate>
{
    WKWebView *webView;
}
@property (assign) IBOutlet WKWebView *webView;
//@property (nonatomic) std::function<void()> onCloseCallback;
@property (nonatomic) BOOL hideViewWhenClickClose;
@property (nonatomic) BOOL ignoreCloseWhenClickClose; //忽略返回键
@property (nonatomic) BOOL bCloseWhenClickBackground; //非全屏时，是否点击背景时关闭WebView

- (void) setCloseCB: (const std::function<void()>&) cb;
@end


@implementation WebViewWindowController
@synthesize webView;
@synthesize hideViewWhenClickClose;
@synthesize ignoreCloseWhenClickClose;
@synthesize bCloseWhenClickBackground;

- (void) setCloseCB: (const std::function<void()>&) cb {
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
//    NSLog(@"%s", __FUNCTION__);
}

- (void)webView:(WKWebView *)webView decidePolicyForNavigationResponse:(WKNavigationResponse *)navigationResponse decisionHandler:(void (^)(WKNavigationResponsePolicy))decisionHandler {
    decisionHandler(WKNavigationResponsePolicyAllow);
}

- (BOOL)windowShouldClose:(id)sender {
    if (ignoreCloseWhenClickClose)
    {
        return NO;
    }

    if (hideViewWhenClickClose)
    {
        [self.window setIsVisible:false];
        return NO;
    }
    
    if (_onCloseCallback)
        _onCloseCallback();
    return YES;
}

@end

namespace ParaEngine {
    IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }

    IParaWebView* IParaWebView::createSubViewView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createSubWebView(x, y, w, h);
    }

    ParaEngineWebView* ParaEngineWebView::createWebView(int x, int y, int w, int h)
    {
        auto p = new ParaEngineWebView();
        
        p->openWindow(x, y, w, h, false);
        
        return p;
    }
    
    ParaEngineWebView* ParaEngineWebView::createSubWebView(int x, int y, int w, int h)
    {
        auto p = new ParaEngineWebView();
        
        p->openWindow(x, y, w, h, true);
        
        return p;
    }

    void ParaEngineWebView::openWindow(int x, int y, int w, int h, bool bSub)
    {
        if (!_webViewController)
        {
            if (bSub)
            {
                auto renderWindow = (NSWindow*)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
                
                _webViewController = [[WebViewWindowController alloc] initWithWindowNibName:@"WebViewWindowWithoutTitleBar"];
                
                [renderWindow addChildWindow:_webViewController.window ordered:NSWindowAbove];
                
                x += renderWindow.frame.origin.x;
                y = renderWindow.frame.origin.y + (renderWindow.contentView.frame.size.height - h) - y;
            }
            else
            {
                _webViewController = [[WebViewWindowController alloc] initWithWindowNibName:@"WebViewWindow"];
                [_webViewController autoResize];
                y = [[NSScreen mainScreen] visibleFrame].size.height - h - y;
            }

            _webViewController.hideViewWhenClickClose = FALSE;
            _webViewController.ignoreCloseWhenClickClose = FALSE;
            _webViewController.bCloseWhenClickBackground = FALSE;
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

        [_webViewController.window orderFront:nil];

        [_webViewController.webView setFrameSize:NSMakeSize(w, h)];
        [_webViewController.window setContentSize:_webViewController.webView.frame.size];
        [_webViewController.window setFrameOrigin:NSMakePoint(x, y)];
    }

    ParaEngineWebView::ParaEngineWebView()
        : _webViewController(nil)
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
            NSString* _urlString = @(urlString.c_str());
            [_webViewController.window setTitle:_urlString];
            NSURL *url = [NSURL URLWithString:_urlString];

            NSURLRequest * request = nil;

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
            [_webViewController.window setOpaque:NO];
            [_webViewController.window setBackgroundColor:[NSColor clearColor]];
            _webViewController.window.contentView.alphaValue = a;
        }
    }
    
    void ParaEngineWebView::hideCloseButton(bool bHide)
    {
         if (_webViewController)
         {
             [[_webViewController.window standardWindowButton:NSWindowCloseButton] setHidden:bHide];
         }
    }
    
    void ParaEngineWebView::setVisible(bool bVisible)
    {
        if (_webViewController)
        {
            [_webViewController.window setIsVisible:bVisible];
            
            if (bVisible) {
                auto renderWindow = (NSWindow*)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
                [renderWindow addChildWindow:_webViewController.window ordered:NSWindowAbove];
            }
        }
    }
    
    void ParaEngineWebView::SetHideViewWhenClickBack(bool b)
    {
         if (_webViewController)
         {
             _webViewController.hideViewWhenClickClose = b;
         }
    }

    void ParaEngineWebView::SetIgnoreCloseWhenClickBack(bool b)
    {
         if (_webViewController)
         {
             _webViewController.ignoreCloseWhenClickClose = b;
         }
    }

    void ParaEngineWebView::SetCloseWhenClickBackground(bool b)
    {
         if (_webViewController)
         {
             _webViewController.bCloseWhenClickBackground = b;
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
        if (_webViewController)
            [_webViewController.window orderFront:nil];
    }

    void ParaEngineWebView::move(int x, int y)
    {
        if (_webViewController)
        {
            auto renderWindow = (NSWindow*)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
            auto h = _webViewController.window.frame.size.height;

            x += renderWindow.frame.origin.x;
            y = renderWindow.frame.origin.y + (renderWindow.contentView.frame.size.height - h) - y;

            [_webViewController.window setFrameOrigin:NSMakePoint(x, y)];
        }
     }

    void ParaEngineWebView::resize(int width, int height)
    {
        if (_webViewController)
        {
            [_webViewController.webView setFrameSize:NSMakeSize(width, height)];
            [_webViewController.window setContentSize:_webViewController.webView.frame.size];
        }
    }
} // end namespcae

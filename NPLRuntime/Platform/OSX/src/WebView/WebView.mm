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
@end

@interface WebViewWindowController () <WKNavigationDelegate, WKUIDelegate>
{
    WKWebView *webView;
}
@property (assign) IBOutlet WKWebView *webView;
@property (nonatomic) std::function<void()> onCloseCallback;
@property (nonatomic) BOOL hideViewWhenClickClose;
@end



@implementation WebViewWindowController
@synthesize webView;
@synthesize hideViewWhenClickClose;

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation {
    NSLog(@"%s", __FUNCTION__);
}


- (void)webView:(WKWebView *)webView decidePolicyForNavigationResponse:(WKNavigationResponse *)navigationResponse decisionHandler:(void (^)(WKNavigationResponsePolicy))decisionHandler {
    decisionHandler(WKNavigationResponsePolicyAllow);
}

- (BOOL)windowShouldClose:(id)sender {
    if(hideViewWhenClickClose)
    {
        [self.window setIsVisible:false];
        return NO;
    }
    
    if (self.onCloseCallback)
        self.onCloseCallback();
    return YES;
}

@end

namespace ParaEngine {
    
    IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }
    
    ParaEngineWebView* ParaEngineWebView::createWebView(int x, int y, int w, int h)
    {
        auto  p = new ParaEngineWebView();
        
        p->openWindow(x, y, w, h);
        
        return p;
    }
    
    void ParaEngineWebView::openWindow(int x, int y, int w, int h)
    {
        if (!_webViewController)
        {
            _webViewController = [[WebViewWindowController alloc] initWithWindowNibName:@"WebViewWindow"];
            
            _webViewController.hideViewWhenClickClose = FALSE;
            _webViewController.webView.navigationDelegate = _webViewController;
            _webViewController.webView.UIDelegate = _webViewController;
            _webViewController.onCloseCallback = [this]() {
                if (this->_onClose == nullptr)
                    this->Release();
                else
                {
                    if (!this->_onClose())
                        this->Release();
                }
            };
        }
        [_webViewController.window orderFront:nil];
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
        }
    }
    
    void ParaEngineWebView::SetHideViewWhenClickBack(bool b)
    {
         if (_webViewController)
         {
             _webViewController.hideViewWhenClickClose = b;
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
} // end namespcae

//
//  WebView.m
//  jsoncpp
//
//  Created by apple on 2018/5/22.
//
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#include "ParaEngine.h"
#include "WebView.h"
#import "GLView.h"

static std::string getFixedBaseUrl(const std::string& baseUrl)
{
    std::string fixedBaseUrl;
    if (baseUrl.empty() || baseUrl.at(0) != '/')
    {
        fixedBaseUrl = [[[NSBundle mainBundle] resourcePath] UTF8String];
        fixedBaseUrl += "/";
        fixedBaseUrl += baseUrl;
    }
    else
    {
        fixedBaseUrl = baseUrl;
    }
    
    size_t pos = 0;
    while((pos = fixedBaseUrl.find(" ")) != std::string::npos)
    {
        fixedBaseUrl.replace(pos, 1, "%20");
    }
    
    if (fixedBaseUrl.at(fixedBaseUrl.length() - 1) != '/')
        fixedBaseUrl += "/";
    
    return fixedBaseUrl;
}

@interface UIWebViewWrapper : NSObject
@property (nonatomic) std::function<bool(const std::string& url)> shouldStartLoading;
@property (nonatomic) std::function<void(const std::string& url)> didFinishLoading;
@property (nonatomic) std::function<void(const std::string& url)> didFailLoading;
@property (nonatomic) std::function<void(const std::string& url)> onJsCallback;
@property (nonatomic) std::function<void()> onCloseCallback;


@property(nonatomic, readonly, getter=canGoBack) BOOL canGoBack;
@property(nonatomic, readonly, getter=canGoForward) BOOL canGoForward;


//HideCloseButton
@property(nonatomic, readwrite, getter = HideCloseButton,  setter = setHideCloseButton:) BOOL HideCloseButton;
@property(nonatomic) BOOL hideViewWhenClickClose;

+ (instancetype)webViewWrapper;

- (void)setVisible:(bool)visible;

- (void)setBounces:(bool)bounces;

- (void)setFrameWithX:(float)x y:(float)y width:(float)width height:(float)height;

- (void)setJavascriptInterfaceScheme:(const std::string&)scheme;

- (void)loadData:(const std::string&)data MIMEType:(const std::string&)MIMEType textEncodingName:(const std::string&)encodingName baseURL:(const std::string&)baseURL;

- (void)loadHTMLString:(const std::string& )string baseURL:(const std::string&)baseURL;

- (void)loadUrl:(const std::string&)urlString cleanCachedData:(BOOL) needCleanCachedData;

- (void)loadFile:(const std::string&)filePath;

- (void)stopLoading;

- (void)reload;

- (void)evaluateJS:(const std::string&)js;

- (void)goBack;

- (void)goForward;

- (void)setScalesPageToFit:(bool)scalesPageToFit;

- (void)setAlpha:(float)a;




@end

@interface UIWebViewWrapper () <UIWebViewDelegate>
@property(nonatomic, retain) UIWebView * uiWebView;
@property(nonatomic, retain) UIButton* uiCloseBtn;
@property(nonatomic, copy) NSString * jsScheme;
@end

@implementation UIWebViewWrapper
{
    
}

+ (instancetype)webViewWrapper
{
    return [[self alloc] init] ;
}
- (instancetype) init
{
    self = [super init];
    if (self)
    {
        self.uiWebView = nil;
        self.shouldStartLoading = nullptr;
        self.didFinishLoading = nullptr;
        self.didFailLoading = nullptr;
        self.onJsCallback = nullptr;
    }
    
    return self;
}

- (void)dealloc
{
    if (self.uiWebView)
    {
        self.uiWebView.delegate = nil;
        [self.uiWebView removeFromSuperview];
        self.uiWebView = nil;
        self.uiCloseBtn = nil;
    }
    
    self.jsScheme = nil;
}

- (void)onCloseBtn:(UIButton*)btn
{
    if (self.hideViewWhenClickClose)
    {
        [self setVisible:NO];
    }
    else
    {
        self.uiWebView.delegate = nil;
        [self.uiWebView removeFromSuperview];
        self.uiWebView = nil;
        self.uiCloseBtn = nil;
        if (self.onCloseCallback)
            self.onCloseCallback();
     }
}

- (void)setupWebView
{
    if (!self.uiWebView)
    {
        self.uiWebView = [[UIWebView alloc] init];
        self.uiWebView.delegate = self;
    }
    
    if (!self.uiWebView.superview)
    {
        //GLView* view = (GLView*)CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
        void* p = (void*)ParaEngine::CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();
        if (p)
        {
            GLView* view = (__bridge GLView*)p;
            [view addSubview:self.uiWebView];
        }
    }
    
    if (!self.uiCloseBtn)
    {
        self.uiCloseBtn = [UIButton buttonWithType:UIButtonTypeCustom];
        [self.uiCloseBtn addTarget:self action:@selector(onCloseBtn:) forControlEvents:UIControlEventTouchUpInside];
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *resPath = [bundle resourcePath];
        resPath = [resPath stringByAppendingPathComponent:@"res/WebViewCloseBtn.png"];
        UIImage* img = [UIImage imageWithContentsOfFile:resPath];
        float w = img.size.width * img.scale;
        float h = img.size.height * img.scale;
        self.uiCloseBtn.frame = CGRectMake(0, 0, w, h);
        [self.uiCloseBtn setImage:img forState:UIControlStateNormal];
        [self.uiWebView addSubview:self.uiCloseBtn];
    }
}

- (void)setAlpha:(float)a
{
    self.uiWebView.alpha = a;
}

- (void)setVisible:(bool)visible
{
    self.uiWebView.hidden = !visible;
}

- (void)setBounces:(bool)bounces
{
    self.uiWebView.scrollView.bounces = bounces;
}

- (void)setFrameWithX:(float)x y:(float)y width:(float)width height:(float)height
{
    if (!self.uiWebView)
        [self setupWebView];
    
    float btnHeigh = self.uiCloseBtn.frame.size.height;
    float btnY = (height - btnHeigh) / 2;
    self.uiCloseBtn.frame = CGRectMake(5, btnY, self.uiCloseBtn.frame.size.width, btnHeigh);
    self.uiWebView.frame = CGRectMake(x, y, width , height);
}

- (void)setJavascriptInterfaceScheme:(const std::string&)scheme
{
    self.jsScheme = @(scheme.c_str());
}

- (void)loadData:(const std::string&)data MIMEType:(const std::string&)MIMEType textEncodingName:(const std::string&)encodingName baseURL:(const std::string&)baseURL
{
    if (!self.uiWebView)
        [self setupWebView];
    
    [self.uiWebView loadData:[NSData dataWithBytes:data.c_str() length:data.length()]
                    MIMEType:@(MIMEType.c_str())
                    textEncodingName:@(encodingName.c_str())
                    baseURL:[NSURL URLWithString:@(getFixedBaseUrl(baseURL).c_str())]];
}

- (void)loadHTMLString:(const std::string& )string baseURL:(const std::string&)baseURL
{
    if (!self.uiWebView)
        [self setupWebView];
    
    [self.uiWebView loadHTMLString:@(string.c_str()) baseURL:[NSURL URLWithString:@(getFixedBaseUrl(baseURL).c_str())]];
}

- (void)loadUrl:(const std::string&)urlString cleanCachedData:(BOOL) needCleanCachedData
{
    if (!self.uiWebView)
        [self setupWebView];
    
    NSURL *url = [NSURL URLWithString:@(urlString.c_str())];
    
    NSURLRequest * request = nil;
    if (needCleanCachedData)
        request = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:60];
    else
        request = [NSURLRequest requestWithURL:url];
    
    [self.uiWebView loadRequest:request];
}

- (void)loadFile:(const std::string&)filePath
{
    if (!self.uiWebView)
        [self setupWebView];
    
    NSURL *url = [NSURL fileURLWithPath:@(filePath.c_str())];
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    [self.uiWebView loadRequest:request];
}

- (void)stopLoading
{
    [self.uiWebView stopLoading];
}

- (void)reload
{
    [self.uiWebView reload];
}

- (BOOL)canGoForward
{
    return self.uiWebView.canGoForward;
}

- (BOOL)canGoBack
{
    return self.uiWebView.canGoBack;
}

- (BOOL)HideCloseButton
{
   if (self.uiCloseBtn)
       return self.uiCloseBtn.hidden;
    else
       return YES;
}

- (void)setHideCloseButton:(BOOL)bHide
{
    if (self.uiCloseBtn)
        self.uiCloseBtn.hidden = bHide;
}

- (void)goBack
{
    [self.uiWebView goBack];
}

- (void)goForward
{
    [self.uiWebView goForward];
}

- (void)evaluateJS:(const std::string &)js
{
    if (!self.uiWebView)
        [self setupWebView];
    
    [self.uiWebView stringByEvaluatingJavaScriptFromString:@(js.c_str())];
}

- (void)setScalesPageToFit:(bool)scalesPageToFit
{
    if (!self.uiWebView)
        [self setupWebView];
    
    self.uiWebView.scalesPageToFit = scalesPageToFit;
}

#pragma mark - UIWebViewDelegate
- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
    NSString *url = [[request URL] absoluteString];
    if ([[[request URL] scheme] isEqualToString:self.jsScheme])
    {
        if (self.onJsCallback)
            self.onJsCallback([url UTF8String]);
        return NO;
    }
    
    if (self.shouldStartLoading && url)
    {
        return self.shouldStartLoading([url UTF8String]);
    }
    
    return YES;
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    if (self.didFinishLoading)
    {
        NSString *url = [[webView.request URL] absoluteString];
        self.didFinishLoading([url UTF8String]);
    }
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
    if (self.didFailLoading)
    {
        NSString *url = error.userInfo[NSURLErrorFailingURLStringErrorKey];
        if (url)
            self.didFailLoading([url UTF8String]);
    }
}

@end

namespace ParaEngine {
    
    IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }

    IParaWebView* IParaWebView::createSubViewView(int x, int y, int w, int h)
    {
        return ParaEngineWebView::createWebView(x, y, w, h);
    }
    
    ParaEngineWebView* ParaEngineWebView::createWebView(int x, int y, int w, int h)
    {
        auto  p = new ParaEngineWebView();
        [p->_uiWebViewWrapper setFrameWithX:(float)x y:(float)y width:(float)w height:(float)h];
        [p->_uiWebViewWrapper setScalesPageToFit:true];
        
        return p;
    }
    
    ParaEngineWebView::ParaEngineWebView()
    {
        _uiWebViewWrapper = [UIWebViewWrapper webViewWrapper];
        
        _uiWebViewWrapper.onCloseCallback = [this]() {
            if (this->_onClose == nullptr)
                this->Release();
            else
            {
                if (!this->_onClose())
                    this->Release();
            }
        };
    }
    
    ParaEngineWebView::~ParaEngineWebView()
    {
        _uiWebViewWrapper = nil;
    }
    
    
    void ParaEngineWebView::loadUrl(const std::string &url, bool cleanCachedData)
    {
        [this->_uiWebViewWrapper loadUrl:url cleanCachedData:cleanCachedData];
    }
    
    void ParaEngineWebView::setAlpha(float a)
    {
        [this->_uiWebViewWrapper setAlpha:a];
    }
    
    void ParaEngineWebView::hideCloseButton(bool bHide)
    {
        this->_uiWebViewWrapper.HideCloseButton = bHide;
    }
    
    void ParaEngineWebView::setVisible(bool bVisible)
    {
        [this->_uiWebViewWrapper setVisible:bVisible];
    }
    
    void ParaEngineWebView::SetHideViewWhenClickBack(bool b)
    {
        this->_uiWebViewWrapper.hideViewWhenClickClose = b;
    }
    
    void ParaEngineWebView::Refresh()
    {
        [this->_uiWebViewWrapper reload];
    }
    
    void ParaEngineWebView::addCloseListener(onCloseFunc fun)
    {
        _onClose = fun;
    }

    void ParaEngineWebView::move()
    {
        return;
    }

    void ParaEngineWebView::resize()
    {
        return;
    }
} // end namespcae

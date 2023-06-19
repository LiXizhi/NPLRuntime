//-----------------------------------------------------------------------------
// Class: WebView.mm
// Authors: kkvskkkk, big
// Emails: onedou@126.com
// CreateDate: 2018.5.22
// ModifyDate: 2023.5.15
//-----------------------------------------------------------------------------

#import <WebKit/WebKit.h>
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import "GLView.h"
#import "LuaObjcBridge/CCLuaObjcBridge.h"

#include <unordered_map>
#include "ParaEngine.h"
#include "3dengine/AudioEngine2.h"

#import "WebView.h"
#import "RenderWindowiOS.h"

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
@property (nonatomic) std::function<bool(const std::string &url)> shouldStartLoading;
@property (nonatomic) std::function<void(const std::string &url)> didFinishLoading;
@property (nonatomic) std::function<void(const std::string &url)> didFailLoading;
@property (nonatomic) std::function<void(const std::string &url)> onJsCallback;
@property (nonatomic) std::function<void()> onCloseCallback;

@property(nonatomic, readonly, getter=canGoBack) BOOL canGoBack;
@property(nonatomic, readonly, getter=canGoForward) BOOL canGoForward;

//HideCloseButton
@property(nonatomic, readwrite, getter = HideCloseButton, setter = setHideCloseButton:) BOOL HideCloseButton;
//@property(nonatomic) BOOL hideViewWhenClickClose;
//@property(nonatomic) BOOL ignoreCloseWhenClickClose;
//@property(nonatomic) BOOL bCloseWhenClickBackground;
@property(nonatomic, retain) UIButton *uiCloseBtn;
@property(nonatomic, copy) NSString *jsScheme;
@property(nonatomic, retain) WKWebView *uiWebView;

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

- (void)move:(int)x y:(int)y;

- (void)resize:(int)width height:(int)height;

- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message;

- (void)activate:(const std::string &)filepath msg:(const std::string &)msg;

@end

@interface UIWebViewWrapper () <WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler>
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
        self.uiWebView.UIDelegate = nil;
        self.uiWebView.navigationDelegate = nil;
        [self.uiWebView removeFromSuperview];
        self.uiWebView = nil;
        self.uiCloseBtn = nil;
    }
    
    self.jsScheme = nil;
}

- (void)onCloseBtn:(UIButton*)btn
{
    [self setVisible:NO];
    [self loadUrl:"" cleanCachedData:YES];
    ParaEngine::CAudioEngine2::GetInstance()->ResetAudioDevice("");
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

- (void)setupWebView
{
    if (!self.uiWebView)
    {
        WKWebViewConfiguration *webViewConfig = [[WKWebViewConfiguration alloc] init];
        webViewConfig.allowsInlineMediaPlayback = YES;
        webViewConfig.mediaPlaybackRequiresUserAction = false;

        [webViewConfig.userContentController addScriptMessageHandler:self name:@"activate"];

        self.uiWebView = [[WKWebView alloc] initWithFrame:CGRectMake(0, 0, 0, 0) configuration:webViewConfig];
        self.uiWebView.UIDelegate = self;
        self.uiWebView.navigationDelegate = self;
    }

    if (!self.uiWebView.superview)
    {
        void *p = (void *)ParaEngine::CGlobals::GetApp()->GetRenderWindow()->GetNativeHandle();

        if (p)
        {
            GLView *view = (__bridge GLView *)p;
            [view addSubview:self.uiWebView];
        }
    }
}

- (void)activate:(const std::string &)filepath msg:(const std::string &)msg
{
    NSString *msgStr = [NSString stringWithCString:msg.c_str() encoding:[NSString defaultCStringEncoding]];
    NSString *filepathStr = [NSString stringWithCString:filepath.c_str() encoding:[NSString defaultCStringEncoding]];

    NSString *jsStr = [NSString stringWithFormat:@"window.NPL.receive(\"%@\",\"%@\")", filepathStr, msgStr];
    
    if (self.uiWebView)
    {
        [self.uiWebView evaluateJavaScript:jsStr completionHandler:nil];
    }
}

- (void)move:(int)x y:(int)y
{
    auto height = self.uiWebView.frame.size.height;
    auto btnHeigh = self.uiCloseBtn.frame.size.height;

    float btnY = (height - btnHeigh) / 2;
    self.uiCloseBtn.frame = CGRectMake(5, btnY, self.uiCloseBtn.frame.size.width, btnHeigh);

    self.uiWebView.frame = CGRectMake(x, y, self.uiWebView.frame.size.width, self.uiWebView.frame.size.height);
}

- (void)resize:(int)width height:(int)height {
    auto btnHeigh = self.uiCloseBtn.frame.size.height;
    float btnY = (height - btnHeigh) / 2;
    self.uiCloseBtn.frame = CGRectMake(5, btnY, self.uiCloseBtn.frame.size.width, btnHeigh);

    self.uiWebView.frame = CGRectMake(self.uiWebView.frame.origin.x, self.uiWebView.frame.origin.y, width, height);
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

    self.uiWebView.frame = CGRectMake(x, y, width, height);
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
                    characterEncodingName:@(encodingName.c_str())
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

    NSString *nsStringUrl = @(urlString.c_str());
    nsStringUrl =
        [nsStringUrl
         stringByAddingPercentEncodingWithAllowedCharacters:
             [NSCharacterSet characterSetWithCharactersInString:@"`%^{}\"[]|\\<> "].invertedSet];

    NSURL *url = [NSURL URLWithString: nsStringUrl];
    auto originWidth = self.uiWebView.frame.size.width;
    auto originHeight = self.uiWebView.frame.size.height;

    if ([nsStringUrl rangeOfString:@"screenOrientation=portrait"].location != NSNotFound) {
        self.uiWebView.transform = CGAffineTransformMakeRotation(-M_PI/2);
        self.uiWebView.bounds = CGRectMake(0, 0, originHeight, originWidth);

        self.uiCloseBtn.frame = CGRectMake(
            (originHeight / 2) - (self.uiCloseBtn.frame.size.width / 2),
            originWidth - self.uiCloseBtn.frame.size.height - 10,
            self.uiCloseBtn.frame.size.width,
            self.uiCloseBtn.frame.size.height
        );
    }
    
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

//- (BOOL)HideCloseButton
//{
//   if (self.uiCloseBtn)
//       return self.uiCloseBtn.hidden;
//    else
//       return YES;
//}
//
//- (void)setHideCloseButton:(BOOL)bHide
//{
//    if (self.uiCloseBtn)
//        self.uiCloseBtn.hidden = bHide;
//}

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
    
    [self.uiWebView evaluateJavaScript:@(js.c_str()) completionHandler:nil];
}

- (void)setScalesPageToFit:(bool)scalesPageToFit
{
    //it doesn't support setting it dynamically
}

#pragma mark - WKNavigationDelegate
- (void)webView:(WKWebView *)webView
        decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction
        decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
{
    NSString *url = [[[navigationAction request] URL] absoluteString];

    if ([[webView.URL scheme] isEqualToString:self.jsScheme])
    {
        self.onJsCallback([url UTF8String]);
        decisionHandler(WKNavigationActionPolicyAllow);
    }
    
    if (self.shouldStartLoading && url)
    {
        if (self.shouldStartLoading([url UTF8String]))
            decisionHandler(WKNavigationActionPolicyAllow);
        else
            decisionHandler(WKNavigationActionPolicyCancel);
        
        return;
    }

    decisionHandler(WKNavigationActionPolicyAllow);
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(null_unspecified WKNavigation *)navigation
{
    if (self.didFinishLoading)
    {
        NSString *url = [webView.URL absoluteString];
        self.didFinishLoading([url UTF8String]);
    }
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(null_unspecified WKNavigation *)navigation withError:(nonnull NSError *)error
{
    if (self.didFailLoading)
    {
        NSString *errorInfo = error.userInfo[NSURLErrorFailingURLErrorKey];
        if (errorInfo)
        {
            self.didFailLoading([errorInfo UTF8String]);
        }
    }
}

#pragma mark - WKUIDelegate
- (void)webView:(WKWebView *)webView
        runJavaScriptAlertPanelWithMessage:(nonnull NSString *)message
        initiatedByFrame:(nonnull WKFrameInfo *)frame
        completionHandler:(nonnull void (^)())completionHandler
{
    /*
    UIAlertController *alertController = [UIAlertController alertControllerWithTitle:message message:nil preferredStyle:UIAlertControllerStyleAlert];
    [alertController addAction:[UIAlertAction actionWithTitle:@"ok" style:UIAlertActionStyleCancel handler:^(UIAlertAction *action){ completionHandler();}]];
    
    */
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
        [webview->_uiWebViewWrapper setFrameWithX:(float)x y:(float)y width:(float)w height:(float)h];

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

        RenderWindowiOS *pWnd = (RenderWindowiOS *)CGlobals::GetApp()->GetRenderWindow();
        int w = pWnd->GetWidth();
        int h = pWnd->GetHeight();
        float scaleX = pWnd->GetScaleX();
        float scaleY = pWnd->GetScaleY();

        w = w / scaleX;
        h = h / scaleY;

        ParaEngineWebView *pView = createWebView(0, 0, w, h);

        if (!pView)
            return;

        pView->loadUrl(url);
        pView->setAlpha(0.95f);
        
        pView->_uiWebViewWrapper.uiCloseBtn = [UIButton buttonWithType:UIButtonTypeCustom];
        [pView->_uiWebViewWrapper.uiCloseBtn
            addTarget:pView->_uiWebViewWrapper
            action:@selector(onCloseBtn:)
            forControlEvents:UIControlEventTouchUpInside];

        NSBundle *bundle = [NSBundle mainBundle];
        NSString *resPath = [bundle resourcePath];
        resPath = [resPath stringByAppendingPathComponent:@"res/WebViewCloseBtn.png"];
        UIImage* img = [UIImage imageWithContentsOfFile:resPath];

        float iconW = img.size.width * img.scale;
        float iconH = img.size.height * img.scale;
        float iconY = (h - iconH) / 2;

        pView->_uiWebViewWrapper.uiCloseBtn.frame = CGRectMake(5, iconY, iconW, iconH);
        [pView->_uiWebViewWrapper.uiCloseBtn setImage:img forState:UIControlStateNormal];
        [pView->_uiWebViewWrapper.uiWebView addSubview:pView->_uiWebViewWrapper.uiCloseBtn];

        openUrlViewTag = pView->viewTag;
        isOpenUrlLoaded = true;
    }

    void ParaEngineWebView::openExternalBrowser(const char *url)
    {
        NSString *nsUrl = [[NSString alloc] initWithUTF8String:url];
        [[UIApplication sharedApplication] openURL: [NSURL URLWithString: nsUrl]];
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
//        this->_uiWebViewWrapper.hideViewWhenClickClose = b;
    }

    void ParaEngineWebView::SetIgnoreCloseWhenClickBack(bool b)
    {
//        this->_uiWebViewWrapper.ignoreCloseWhenClickClose = b;
    }

    void ParaEngineWebView::SetCloseWhenClickBackground(bool b)
    {
//        this->_uiWebViewWrapper.bCloseWhenClickBackground = b;
    }
    
    void ParaEngineWebView::Refresh()
    {
        [this->_uiWebViewWrapper reload];
    }
    
    void ParaEngineWebView::addCloseListener(onCloseFunc fun)
    {
        _onClose = fun;
    }

    void ParaEngineWebView::move(int x, int y)
    {
        [this->_uiWebViewWrapper move:x y:y];
    }

    void ParaEngineWebView::resize(int width, int height)
    {
        [this->_uiWebViewWrapper resize:width height:height];
    }

    void ParaEngineWebView::activate(const std::string &filepath, const std::string &msg)
    {
        [this->_uiWebViewWrapper activate: filepath msg:msg];
    }
} // end namespcae

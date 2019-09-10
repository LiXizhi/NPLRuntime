//
//  WebView.h
//  NPLRuntime_mac_client
//
//  Created by apple on 2018/5/25.
//

#ifndef WebView_h
#define WebView_h

#include <stdint.h>
#include <iosfwd>
#include "IParaWebView.h"


@class WebViewWindowController;

namespace ParaEngine {
    
    class ParaEngineWebView : public IParaWebView
    {
    public:
        virtual ~ParaEngineWebView();
        
        static ParaEngineWebView* createWebView(int x, int y, int w, int h);
        static ParaEngineWebView* createSubWebView(int x, int y, int w, int h);
        
        virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
        virtual void setAlpha(float a) override;
        virtual void setVisible(bool bVisible) override;
        virtual void SetHideViewWhenClickBack(bool b) override;
        virtual void Refresh() override;
        virtual void hideCloseButton(bool bHide) override;
        virtual void addCloseListener(onCloseFunc fun) override;
        virtual void bringToTop() override;
        virtual void move() override;
        virtual void resize() override;
        virtual void close() override;
        
    protected:
        void openWindow(int x, int y, int w, int h, bool bSub);
        
        
    protected:
        ParaEngineWebView();
    private:
        WebViewWindowController* _webViewController;
        onCloseFunc _onClose;
    };
    
} // end namespace

#endif /* WebView_h */

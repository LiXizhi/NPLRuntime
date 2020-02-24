#pragma once

#include "IParaWebView.h"

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
        virtual void move(int x, int y) override;
        virtual void resize(int width, int height) override;
        
    protected:
        void openWindow(int x, int y, int w, int h, bool bSub);
        
        
    protected:
        ParaEngineWebView();
    private:
        //WebViewWindowController* _webViewController;
        onCloseFunc _onClose;
    };
} // end namespace
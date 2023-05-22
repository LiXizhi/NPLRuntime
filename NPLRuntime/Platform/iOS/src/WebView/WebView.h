//-----------------------------------------------------------------------------
// Class: WebView.h
// Authors: kkvskkkk, big
// Emails: onedou@126.com
// CreateDate: 2018.5.22
// ModifyDate: 2023.5.15
//-----------------------------------------------------------------------------

#ifndef WebView_h
#define WebView_h

#include <stdint.h>
#include <iosfwd>
#include "IParaWebView.h"

@class UIWebViewWrapper;

namespace ParaEngine {
    class ParaEngineWebView : public IParaWebView
    {
    public:
        virtual ~ParaEngineWebView();

        static ParaEngineWebView *createWebView(int x, int y, int w, int h);
        static void openWebView(const std::string &url);
        static void openExternalBrowser(const char *url);
        static ParaEngineWebView *getWebViewByIndex(int viewTag);

        virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
        virtual void setAlpha(float a) override;
        virtual void setVisible(bool bVisible) override;
        virtual void SetHideViewWhenClickBack(bool b) override;
        virtual void SetIgnoreCloseWhenClickBack(bool b) override;
        virtual void SetCloseWhenClickBackground(bool b) override;
        virtual void Refresh() override;
        virtual void hideCloseButton(bool bHide) override;
        virtual void addCloseListener(onCloseFunc fun) override;
        virtual void bringToTop() override {};
        virtual void move(int x, int y) override;
        virtual void resize(int width, int height) override;
        virtual void activate(const std::string &filepath, const std::string &msg) override;
    protected:
        ParaEngineWebView();
    private:
        UIWebViewWrapper *_uiWebViewWrapper;
        onCloseFunc _onClose;
        int viewTag = 0;
    };
    
} // end namespace


#endif /* WebView_h */

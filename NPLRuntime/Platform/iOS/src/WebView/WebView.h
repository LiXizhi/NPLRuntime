//
//  WebView.h
//  NPLRuntime
//
//  Created by apple on 2018/5/22.
//

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
        
        static ParaEngineWebView* createWebView(int x, int y, int w, int h, Boolean bSub);

        virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
        virtual void setAlpha(float a) override;
        virtual void setVisible(bool bVisible) override;
        virtual void SetHideViewWhenClickBack(bool b) override;
        virtual void Refresh() override;
        virtual void hideCloseButton(bool bHide) override;
        virtual void addCloseListener(onCloseFunc fun) override;
        virtual void bringToTop() override {};
        virtual void move(int x, int y) override;
        virtual void resize(int width, int height) override;
        
    protected:
        ParaEngineWebView();
    private:
        UIWebViewWrapper* _uiWebViewWrapper;
        onCloseFunc _onClose;
    };
    
} // end namespace


#endif /* WebView_h */

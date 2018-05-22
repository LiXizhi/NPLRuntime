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
        
        static ParaEngineWebView* createWebView(int x, int y, int w, int h);
        
        virtual IAttributeFields* GetAttributeObject() override;
        virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
        virtual void setAlpha(float a) override;
        virtual void setVisible(bool bVisible) override;
        virtual void SetHideViewWhenClickBack(bool b) override;
        virtual void Refresh() override;

        virtual void addCloseListener(onCloseFunc fun) override;
        

        virtual const char* GetAttributeClassName() override { return "ParaEngineWebView"; };
        virtual const char* GetAttributeClassDescription() override { return ""; };
        virtual int InstallFields(CAttributeClass* pClass, bool bOverride) override;
        
        ATTRIBUTE_METHOD1(ParaEngineWebView, loadUrl_s, const char*) {cls->loadUrl(p1); return S_OK;};
        ATTRIBUTE_METHOD1(ParaEngineWebView, setAlpha_s, float) {cls->setAlpha(p1); return S_OK;};
        ATTRIBUTE_METHOD1(ParaEngineWebView, setVisible_s, bool) {cls->setVisible(p1); return S_OK;};
        ATTRIBUTE_METHOD(ParaEngineWebView, Refresh_s) {cls->Refresh(); return S_OK;};
        
    protected:
        ParaEngineWebView();
    private:
        UIWebViewWrapper* _uiWebViewWrapper;
        onCloseFunc _onClose;
    };
    
} // end namespace


#endif /* WebView_h */

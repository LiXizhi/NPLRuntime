#include "ParaEngine.h"
#include "WebView.h"

namespace ParaEngine {
	IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
	{
		return nullptr;
	}

	IParaWebView* IParaWebView::createSubViewView(int x, int y, int w, int h)
	{
		return nullptr;
	}

	void ParaEngineWebView::move(int x, int y)
    {
        return;
    }

    void ParaEngineWebView::resize(int width, int height)
    {
        return;
    }
} // end namespace
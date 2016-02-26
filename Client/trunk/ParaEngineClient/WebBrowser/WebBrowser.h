#pragma once
//#include <Windows.h>
//#include <crtdbg.h>
#include "OleIdl.h"
#include <string>
#include "IBrowserMsgListener.h"
#define NOTIMPLEMENTED _ASSERT(0); return(E_NOTIMPL)

namespace ParaEngine
{
	class LauncherApp;
	class WBClientSite;

	class WebBrowser:public IDispatch
	{
	public:
		WebBrowser(HWND hParentWnd,IBrowserMsgListener* pApp);
		~WebBrowser();

		static void RegisterDefaultBrowserWnd(HINSTANCE instance);

		void Cleanup();
		void UnembedBrowser();
		void ShowHTMLPage(LPCTSTR url);
		HWND GetHWND(){return m_hWnd;}
		void ChangeWindowSize(int nWidth, int nHeight);
		void Refresh();
		void GoBack();
		void GoForward();
		void Stop();

		/** get the current location url*/
		std::string get_LocationURL();

		// The class name of our Window to host the browser. It can be anything of your choosing.
		static WCHAR* g_BrowserClassName;

		//IDispatch Members
		STDMETHODIMP Invoke (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, 
			DISPPARAMS* pDispParams,VARIANT* pvarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj)    
		{
			HRESULT ret = S_OK;
			if(riid == IID_IUnknown)
				*ppvObj = static_cast<IUnknown *>(this);
			else if(riid == IID_IDispatch)
				*ppvObj = static_cast<IDispatch *>(this);
			else
				*ppvObj = 0, ret = E_NOINTERFACE;;
			return ret;                
		}

		virtual ULONG   STDMETHODCALLTYPE AddRef(void){ return 1; }
		virtual ULONG   STDMETHODCALLTYPE Release(void){ return 1; }
		virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR* pctinfo){ return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo){ return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId){ return E_NOTIMPL; }

		/** find a given text*/
		bool FindText(const std::string& sFindText);

		/** write user specified content to web browser */
		void WriteContent(const std::string& sContent);

		/** get the current html.body content */
		void GetContent(std::string& sContent);

		static LRESULT CALLBACK browserWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	private:
		IOleObject* m_pBrowserObj;
		HWND m_hWnd;
		bool m_ignoreDownLoadComplete;
		bool m_navErr;
		IBrowserMsgListener* m_pMsgListener;
		WBClientSite* m_pClientSite;
	};
}

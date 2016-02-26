#include "ParaEngine.h"
#include <Exdisp.h>
#include <MsHTML.h>

#include "WebBrowser.h"
#include "WBStorage.h"
#include "WBClientsite.h"
#include "atlcomcli.h"
#include "atlbase.h"
#include "Exdisp.h"
#include "exdispid.h"
#include "util/StringHelper.h"

using namespace std;
using namespace ParaEngine;

WCHAR* ParaEngine::WebBrowser::g_BrowserClassName = L"Browser Object";

namespace ParaEngine
{
	
	WebBrowser::WebBrowser(HWND hParentWnd,IBrowserMsgListener *pMsgListener)
	{
		m_pBrowserObj = NULL;
		m_hWnd = 0;
		m_ignoreDownLoadComplete = true;
		m_navErr = false;
		m_pMsgListener = pMsgListener;
		m_pClientSite = NULL;

		RECT rect;
		WBStorage storage;
		m_pClientSite = new WBClientSite(hParentWnd);
		HRESULT hr = OleCreate(CLSID_WebBrowser,IID_IOleObject,OLERENDER_DRAW,0,(IOleClientSite*)m_pClientSite,
			&storage,(void**)&m_pBrowserObj);
		if(hr == S_OK)
		{
			m_pBrowserObj->SetHostNames(L"TestHost",0);
			GetClientRect(hParentWnd,&rect);

			if(!OleSetContainedObject((IUnknown*)m_pBrowserObj,TRUE))
			{
				if(!m_pBrowserObj->DoVerb(OLEIVERB_SHOW,NULL,(IOleClientSite*)m_pClientSite,-1,hParentWnd,&rect))
				{
					IWebBrowser2* pWebBrowser2;
					if(!m_pBrowserObj->QueryInterface(IID_IWebBrowser2,(void**)&pWebBrowser2))
					{
						pWebBrowser2->put_Left(0);
						pWebBrowser2->put_Top(0);
						pWebBrowser2->put_Width(rect.right);
						pWebBrowser2->put_Height(rect.bottom);
						pWebBrowser2->get_HWND((SHANDLE_PTR*)&m_hWnd);
						pWebBrowser2->put_Visible(FALSE);
						pWebBrowser2->put_Silent(VARIANT_TRUE);
						pWebBrowser2->Release();
						return;
					}
				}
			}
		}
	}

	WebBrowser::~WebBrowser()
	{
		Cleanup();
		SAFE_DELETE(m_pClientSite);
	}

	void WebBrowser::Cleanup()
	{
		if(m_pBrowserObj != NULL)
		{
			/*IWebBrowser2 *pWebBrowser2 = NULL;
			if (!m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
			{
				pWebBrowser2->Stop();
				pWebBrowser2->Quit();
			}
			if(pWebBrowser2 != 0)
			{
				pWebBrowser2->Release();
			}*/
			m_pBrowserObj->Close(OLECLOSE_NOSAVE);
			m_pBrowserObj->Release();
			m_pBrowserObj = NULL;
		}
	}

	void WebBrowser::ShowHTMLPage(LPCTSTR webPageName)
	{
		IWebBrowser2 *pWebBrowser2;
		VARIANT   varMyURL;

		//////////////////////////////////////////////////////////////////////////
		//
		// Not implemented for multiple browsing...
		//
		//IOleObject **ppBrowserHandle;
		//IOleObject *pBrowserObject;
		//
		//// Retrieve the browser object's pointer we stored in our window's GWL_USERDATA when
		//// we initially attached the browser object to this window.
		//if ((ppBrowserHandle = (IOleObject **)GetWindowLong(hwnd, GWL_USERDATA)))
		//{
		// // Unembed the browser object, and release its resources.
		// pBrowserObject = *ppBrowserHandle;
		//}
		//////////////////////////////////////////////////////////////////////////

		// We want to get the base address (ie, a pointer) to the IWebBrowser2 object embedded within the browser
		// object, so we can call some of the functions in the former's table.
		if (!m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			// Our URL (ie, web address, such as "http://www.microsoft.com" or an HTM filename on disk
			// such as "c:/myfile.htm") must be passed to the IWebBrowser2's Navigate2() function as a BSTR.
			// A BSTR is like a pascal version of a double-byte character string. In other words, the
			// first unsigned short is a count of how many characters are in the string, and then this
			// is followed by those characters, each expressed as an unsigned short (rather than a
			// char). The string is not nul-terminated. The OS function SysAllocString can allocate and
			// copy a UNICODE C string to a BSTR. Of course, we'll need to free that BSTR after we're done
			// with it. If we're not using UNICODE, we first have to convert to a UNICODE string.
			//
			// What's more, our BSTR needs to be stuffed into a VARIENT struct, and that VARIENT struct is
			// then passed to Navigate2(). Why? The VARIENT struct makes it possible to define generic
			// 'datatypes' that can be used with all languages. Not all languages support things like
			// nul-terminated C strings. So, by using a VARIENT, whose first field tells what sort of
			// data (ie, string, float, etc) is in the VARIENT, COM interfaces can be used by just about
			// any language.
			VariantInit(&varMyURL);

			varMyURL.vt = VT_BSTR;

	#ifndef UNICODE
			{
				wchar_t  *buffer;
				DWORD  size;

				size = MultiByteToWideChar(CP_ACP, 0, webPageName, -1, 0, 0);
				if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size))) goto badalloc;
				MultiByteToWideChar(CP_ACP, 0, webPageName, -1, buffer, size);
				varMyURL.bstrVal = SysAllocString(buffer);
				GlobalFree(buffer);
			}
	#else
			varMyURL.bstrVal = SysAllocString(webPageName);
	#endif
			if (!varMyURL.bstrVal)
			{
	#ifndef UNICODE
	badalloc:
	#endif
				pWebBrowser2->Release();
				return;
			}


			HRESULT hr;
			CComPtr<IConnectionPointContainer> pCPContainer;
			hr = pWebBrowser2->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPContainer);
			if(SUCCEEDED(hr))
			{
				CComPtr <IConnectionPoint> spCP;
				hr = pCPContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &spCP);
				if(SUCCEEDED(hr))
				{
					DWORD mCookie = 0;
					spCP->Advise(static_cast<IDispatch *>(this),&mCookie);
				}
			}

			// Call the Navigate2() function to actually display the page.
			pWebBrowser2->Navigate2(&varMyURL, 0, 0, 0, 0);
			pWebBrowser2->put_Visible(TRUE); // this may not be necessary.

			// Free any resources (including the BSTR we allocated above).
			VariantClear(&varMyURL);

			// We no longer need the IWebBrowser2 object (ie, we don't plan to call any more functions in it,
			// so we can release our hold on it). Note that we'll still maintain our hold on the browser
			// object.
			pWebBrowser2->Release();

			// Success
			return;
		}

		// failed
		return ;
	}

	STDMETHODIMP  WebBrowser:: Invoke (DISPID dispidMember, REFIID riid,  
		LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,  
		VARIANT* pvarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) 
	{ 
		char sID[32];
		itoa(dispidMember, sID, 10);
		
		if(dispidMember == DISPID_DOWNLOADCOMPLETE)
		{
			//WriteSimpleLogDebug(sID, "DISPID_DOWNLOADCOMPLETE", "\n");
			//ignore the first complete message which is fired by Navigate2
			if(m_ignoreDownLoadComplete)
			{
				m_ignoreDownLoadComplete = false;
				return S_OK;
			}
		}
		else if(dispidMember == DISPID_DOCUMENTCOMPLETE)
		{
			//WriteSimpleLogDebug(sID, "DISPID_DOCUMENTCOMPLETE", "\n");
			m_pMsgListener->OnWebPageLoaded();
		}
		else if(dispidMember == DISPID_BEFORENAVIGATE2)
		{
			// Is navigation to specified Url disallowed? 
			// (*pDispParams).rgvarg[5].vt = VT_BYREF | VT_BSTR;
			CComBSTR url_ = ((*pDispParams).rgvarg)[5].pvarVal->bstrVal;

			std::string url = StringHelper::WideCharToMultiByte((wchar_t*)url_, CP_UTF8);

			// added by LiXizhi:
			// cancel all "javascript:", otherwise it will be impossible to use onclick event on <a> tag
			if( url.find("javascript:") == 0)
			{
				if(url == "javascript:;" || url.find("javascript:void") == 0)
				{
					// Set Cancel parameter to TRUE to cancel the current event
					*(((*pDispParams).rgvarg)[0].pboolVal) = TRUE;

					/*
					// If so, navigate the browser frame to standard resource page 
					CComQIPtr<IWebBrowser2> spBrowser = ((*pDispParams).rgvarg)[6].pdispVal;
					if (spBrowser != NULL)
					{
						static const CComBSTR newURL = L"http://www.paraengine.com/";
						spBrowser->Navigate(newURL, NULL, NULL, NULL, NULL);
					}
					*/
				}
			}
			// WriteSimpleLogDebug(sID, "DISPID_BEFORENAVIGATE2","\n");
		}
		else if(dispidMember == DISPID_NAVIGATECOMPLETE2)
		{
			//WriteSimpleLogDebug(sID, "DISPID_NAVIGATECOMPLETE2", "\n");
		}
		else if(dispidMember == DISPID_STATUSTEXTCHANGE || dispidMember == DISPID_COMMANDSTATECHANGE)
		{
			//WriteSimpleLogDebug(sID, "DISPID_STATUSTEXTCHANGE", "\n");
		}
		else if(dispidMember == DISPID_STATUSTEXTCHANGE)
		{
			//WriteSimpleLogDebug(sID, "DISPID_STATUSTEXTCHANGE", "\n");
		}
		else if(dispidMember == DISPID_NAVIGATECOMPLETE)
		{
			//WriteSimpleLogDebug(sID, "DISPID_NAVIGATECOMPLETE", "\n");
		}
		else if(dispidMember == DISPID_BEFORENAVIGATE)
		{
			//WriteSimpleLogDebug(sID, "DISPID_BEFORENAVIGATE", "\n");
		}
		else if(dispidMember == DISPID_NAVIGATEERROR)
		{
			//WriteSimpleLogDebug(sID, "DISPID_NAVIGATEERROR", "\n");
			m_navErr = true;
		}
		else
		{
			//WriteSimpleLogDebug(sID, "DISPID_XXX  \n");
		}
		
		return S_OK;
	}

	void WebBrowser::ChangeWindowSize( int nWidth, int nHeight )
	{
		IWebBrowser2 *pWebBrowser2 = NULL;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			pWebBrowser2->put_Width(nWidth);
			pWebBrowser2->put_Height(nHeight);
			pWebBrowser2->get_HWND((SHANDLE_PTR*)&m_hWnd);
		}
		if(pWebBrowser2 != 0)
		{
			pWebBrowser2->Release();
		}
	}

	std::string WebBrowser::get_LocationURL()
	{
		std::string url;

		CComPtr<IWebBrowser2> pWebBrowser2;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			CComBSTR bstrLocationURL;
			if( pWebBrowser2->get_LocationURL(&bstrLocationURL) == S_OK)
			{
				url = StringHelper::WideCharToMultiByte((wchar_t*)bstrLocationURL, CP_UTF8);
			}
		}
		return url;
	}

	void WebBrowser::Refresh()
	{
		IWebBrowser2 *pWebBrowser2 = NULL;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			pWebBrowser2->Refresh();
		}
		SAFE_RELEASE(pWebBrowser2);
	}

	void WebBrowser::GoBack()
	{
		IWebBrowser2 *pWebBrowser2 = NULL;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			pWebBrowser2->GoBack();
		}
		SAFE_RELEASE(pWebBrowser2);
	}

	void WebBrowser::GoForward()
	{
		IWebBrowser2 *pWebBrowser2 = NULL;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			pWebBrowser2->GoForward();
		}
		SAFE_RELEASE(pWebBrowser2);
	}

	void WebBrowser::Stop()
	{
		IWebBrowser2 *pWebBrowser2 = NULL;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&pWebBrowser2))
		{
			pWebBrowser2->Stop();
		}
		SAFE_RELEASE(pWebBrowser2);
	}

	bool WebBrowser::FindText(const std::string& sFindText)
	{
		CComPtr <IWebBrowser2> _Browser;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&_Browser))
		{
			CComPtr <IHTMLDocument2> document; 
			
			if (_Browser != NULL) {

				// get browser document's dispatch interface

				IDispatch *document_dispatch = NULL;

				HRESULT hr = _Browser->get_Document(&document_dispatch);

				if (SUCCEEDED(hr) && (document_dispatch != NULL)) {

					// get the actual document interface

					hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,
						(void **)&document);

					// release dispatch interface

					document_dispatch->Release();

				}
				if (document != NULL) 
				{
					CComPtr <IHTMLElement> body; 
					if( SUCCEEDED(document->get_body(&body)) )
					{
						CComBSTR pbstrInnerHtml;
						if( SUCCEEDED(body->get_innerHTML(&pbstrInnerHtml)))
						{
							std::string sContent = StringHelper::WideCharToMultiByte((wchar_t*)pbstrInnerHtml, CP_UTF8);
							// OUTPUT_LOG(sContent.c_str());
						}
					}
				}
			}
		}
		return false;
	}

	void WebBrowser::WriteContent( const std::string& sContent )
	{
		CComPtr <IWebBrowser2> _Browser;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&_Browser))
		{
			CComPtr <IHTMLDocument2> document; 

			if (_Browser != NULL) {

				// get browser document's dispatch interface

				IDispatch *document_dispatch = NULL;

				HRESULT hr = _Browser->get_Document(&document_dispatch);

				if (SUCCEEDED(hr) && (document_dispatch != NULL)) {

					// get the actual document interface

					hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,
						(void **)&document);

					// release dispatch interface

					document_dispatch->Release();

				}
				if (document != NULL) 
				{

					// construct text to be written to browser as SAFEARRAY

					SAFEARRAY *safe_array = SafeArrayCreateVector(VT_VARIANT,0,1);

					VARIANT	*variant;

					SafeArrayAccessData(safe_array,(LPVOID *)&variant);

					variant->vt	= VT_BSTR;

#ifndef UNICODE
					{
						wchar_t  *buffer;
						DWORD  size;

						size = MultiByteToWideChar(CP_ACP, 0, sContent.c_str(), -1, 0, 0);
						if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size)))
						{
							return;
						}
						MultiByteToWideChar(CP_ACP, 0, sContent.c_str(), -1, buffer, size);
						variant->bstrVal = SysAllocString(buffer);
						GlobalFree(buffer);
					}
#else
					variant->bstrVal = SysAllocString(sContent.c_str());
#endif

					SafeArrayUnaccessData(safe_array);

					// write SAFEARRAY to browser document

					document->write(safe_array);

					::SysFreeString(variant->bstrVal);
					variant->bstrVal = NULL;

					SafeArrayDestroy(safe_array);
				}
			}
		}
	}

	void WebBrowser::GetContent( std::string& sContent )
	{
		CComPtr <IWebBrowser2> _Browser;
		if (m_pBrowserObj && !m_pBrowserObj->QueryInterface(IID_IWebBrowser2, (void**)&_Browser))
		{
			CComPtr <IHTMLDocument2> document; 

			if (_Browser != NULL) {

				// get browser document's dispatch interface

				IDispatch *document_dispatch = NULL;

				HRESULT hr = _Browser->get_Document(&document_dispatch);

				if (SUCCEEDED(hr) && (document_dispatch != NULL)) {

					// get the actual document interface

					hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,
						(void **)&document);

					// release dispatch interface

					document_dispatch->Release();

				}
				if (document != NULL) 
				{
					CComPtr <IHTMLElement> body; 
					if( SUCCEEDED(document->get_body(&body)) )
					{
						CComBSTR pbstrOutterHtml;
						if( SUCCEEDED(body->get_outerHTML(&pbstrOutterHtml)))
						{
							sContent = StringHelper::WideCharToMultiByte((wchar_t*)pbstrOutterHtml, CP_UTF8);
						}
					}
				}
			}
		}
	}

	/****************************** WindowProc() ***************************
	 * Our message handler for our window to host the browser.
	 */
	LRESULT CALLBACK WebBrowser::browserWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		switch (uMsg)
		{
		case WM_SIZE:
			{
				// Resize the browser object to fit the window
				//ResizeBrowser(hwnd, LOWORD(lParam), HIWORD(lParam));
				//return(0);
			}
		case WM_CREATE:
			{
				// Embed the browser object into our host window. We need do this only
				// once. Note that the browser object will start calling some of our
				// IOleInPlaceFrame and IOleClientSite functions as soon as we start
				// calling browser object functions in EmbedBrowserObject().
				//if (EmbedBrowserObject(hwnd)) return(-1);

				// Success
				//return(0);
			}

		case WM_DESTROY:
			{
				// Detach the browser object from this window, and free resources.
				//UnEmbedBrowserObject(hwnd);

				//return(TRUE);
			}
		}
		return(DefWindowProcW(hwnd, uMsg, wParam, lParam));
	}

	static bool s_is_browser_wnd_registered = false;
	void WebBrowser::RegisterDefaultBrowserWnd(HINSTANCE hInstance)
	{
		if(!s_is_browser_wnd_registered)
		{
			s_is_browser_wnd_registered = true;

			WNDCLASSEXW		wc;

			// Register the class of our Main window. 'windowProc' is our message handler
			// and 'ClassName' is the class name. You can choose any class name you want.
			ZeroMemory(&wc, sizeof(WNDCLASSEX));
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.hInstance = hInstance;
			// Register the class of our window to host the browser. 'browserWindowProc' is our message handler
			// and 'BrowserClassName' is the class name. You can choose any class name you want.
			wc.lpfnWndProc = WebBrowser::browserWindowProc;
			wc.lpszClassName = g_BrowserClassName;
			wc.style = CS_HREDRAW|CS_VREDRAW;
			RegisterClassExW(&wc);
		}
	}

}





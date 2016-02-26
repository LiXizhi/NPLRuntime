#include "ParaEngine.h"
#include "WBClientSite.h"
#include "WebBrowser.h"

namespace ParaEngine
{
	// Implementation of CWBClientSite, a derived class of IOleClientSite, IOleInPlaceFrame, and IOleInPlaceSite
	HRESULT STDMETHODCALLTYPE WBClientSite::QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		// query the interface of IUnknown
		if (!memcmp(&riid, &IID_IUnknown, sizeof(GUID)))
		{
			*ppvObject = reinterpret_cast<IUnknown *> (this);
		}
		// query the interface of IOleClientSite
		else if (!memcmp(&riid, &IID_IOleClientSite, sizeof(GUID)))
		{
			*ppvObject = (IOleClientSite *)this;
		}
		// query the interface of IOleInPlaceSite
		else if (!memcmp(&riid, &IID_IOleInPlaceSite, sizeof(GUID)))
		{
			*ppvObject = (IOleInPlaceSite *)this;
		}
		else if(!memcmp(&riid,&IID_IDocHostUIHandler,sizeof(GUID)))
		{
			*ppvObject = (IDocHostUIHandler*)this;
		}
		// For other types of objects the browser wants, just report that we don't have any such objects.
		// NOTE: If you want to add additional functionality to your browser hosting, you may need to
		// provide some more objects here. You'll have to investigate what the browser is asking for
		// (ie, what REFIID it is passing).
		else
		{
			*ppvObject = 0;
			return(E_NOINTERFACE);
		}

		return(S_OK);
	}

	ULONG STDMETHODCALLTYPE WBClientSite::AddRef( void)
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE WBClientSite::Release( void)
	{
		return 0;
	}

	// IOleClientSite methods
	HRESULT STDMETHODCALLTYPE WBClientSite::SaveObject( void)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::GetMoniker( 
		/* [in] */ DWORD dwAssign,
		/* [in] */ DWORD dwWhichMoniker,
		/* [out] */ IMoniker **ppmk)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::GetContainer( 
		/* [out] */ IOleContainer **ppContainer)
	{
		// Tell the browser that we are a simple object and don't support a container
		*ppContainer = 0;

		return(E_NOINTERFACE);
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::ShowObject( void)
	{
		return(NOERROR);
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::OnShowWindow( 
		/* [in] */ BOOL fShow)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::RequestNewObjectLayout( void)
	{
		NOTIMPLEMENTED;
	}

	// IOleWindow methods
	HRESULT STDMETHODCALLTYPE WBClientSite::GetWindow( 
		/* [out] */ HWND *phwnd)
	{
		*phwnd = m_hWindow;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::ContextSensitiveHelp( 
		/* [in] */ BOOL fEnterMode)
	{
		NOTIMPLEMENTED;
	}

	// IOleInPlaceUIWindow methods
	HRESULT STDMETHODCALLTYPE WBClientSite::GetBorder( 
		/* [out] */ LPRECT lprectBorder)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::RequestBorderSpace( 
		/* [unique][in] */ LPCBORDERWIDTHS pborderwidths)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::SetBorderSpace( 
		/* [unique][in] */ LPCBORDERWIDTHS pborderwidths)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::SetActiveObject( 
		/* [unique][in] */ IOleInPlaceActiveObject *pActiveObject,
		/* [unique][string][in] */ LPCOLESTR pszObjName)
	{
		return S_OK;
	}

	// IOleInPlaceFrame methods
	HRESULT STDMETHODCALLTYPE WBClientSite::InsertMenus( 
		/* [in] */ HMENU hmenuShared,
		/* [out][in] */ LPOLEMENUGROUPWIDTHS lpMenuWidths)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::SetMenu( 
		/* [in] */ HMENU hmenuShared,
		/* [in] */ HOLEMENU holemenu,
		/* [in] */ HWND hwndActiveObject)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::RemoveMenus( 
		/* [in] */ HMENU hmenuShared)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::SetStatusText( 
		/* [unique][in] */ LPCOLESTR pszStatusText)
	{
		return S_OK;
	}

	// Called from the browser object's IOleInPlaceActiveObject object's EnableModeless() function. Also
	// called when the browser displays a modal dialog box.
	HRESULT STDMETHODCALLTYPE WBClientSite::EnableModeless( 
		/* [in] */ BOOL fEnable)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::TranslateAccelerator( 
		/* [in] */ LPMSG lpmsg,
		/* [in] */ WORD wID)
	{
		NOTIMPLEMENTED;
	}

	// IOleInPlaceSite methods
	HRESULT STDMETHODCALLTYPE WBClientSite::CanInPlaceActivate( void)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::OnInPlaceActivate( void)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::OnUIActivate( void)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::GetWindowContext( 
		/* [out] */ IOleInPlaceFrame **ppFrame,
		/* [out] */ IOleInPlaceUIWindow **ppDoc,
		/* [out] */ LPRECT lprcPosRect,
		/* [out] */ LPRECT lprcClipRect,
		/* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{
		*ppFrame = (IOleInPlaceFrame *)this; // maybe incorrect

		// We have no OLEINPLACEUIWINDOW
		*ppDoc = 0;

		// Fill in some other info for the browser
		lpFrameInfo->fMDIApp = FALSE;
		lpFrameInfo->hwndFrame = m_hWindow;  // maybe incorrect
		lpFrameInfo->haccel = 0;
		lpFrameInfo->cAccelEntries = 0;

		// Give the browser the dimensions of where it can draw. We give it our entire window to fill
		GetClientRect(lpFrameInfo->hwndFrame, lprcPosRect);
		GetClientRect(lpFrameInfo->hwndFrame, lprcClipRect);

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::Scroll( 
		/* [in] */ SIZE scrollExtant)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::OnUIDeactivate( 
		/* [in] */ BOOL fUndoable)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::OnInPlaceDeactivate( void)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::DiscardUndoState( void)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::DeactivateAndUndo( void)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::OnPosRectChange( 
		/* [in] */ LPCRECT lprcPosRect)
	{
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	//IDocHostUIHandler interface
	//////////////////////////////////////////////////////////////////////////

	HRESULT STDMETHODCALLTYPE WBClientSite::ShowContextMenu( 
		/* [in] */ DWORD dwID,
		/* [in] */ POINT *ppt,
		/* [in] */ IUnknown *pcmdtReserved,
		/* [in] */ IDispatch *pdispReserved)
	{
		// We will return S_OK to tell the browser not to display its default context menu,
		// or return S_FALSE to let the browser show its default context menu. For this
		// example, we wish to disable the browser's default context menu.
		return S_OK;
	}

	// Called at initialization of the browser object UI. We can set various features of the browser object here.
	HRESULT STDMETHODCALLTYPE WBClientSite::GetHostInfo( 
		/* [out][in] */ DOCHOSTUIINFO *pInfo)
	{
		pInfo->cbSize = sizeof(DOCHOSTUIINFO);
		// Set some flags. We don't want any 3D border. You can do other things like hide
		// the scroll bar (DOCHOSTUIFLAG_SCROLL_NO), display picture display (DOCHOSTUIFLAG_NOPICS),
		// disable any script running when the page is loaded (DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE),
		// open a site in a new browser window when the user clicks on some link (DOCHOSTUIFLAG_OPENNEWWIN),
		// and lots of other things. See the MSDN docs on the DOCHOSTUIINFO struct passed to us.

		//pInfo->dwFlags = DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_SCROLL_NO |DOCHOSTUIFLAG_OPENNEWWIN;
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_FLAT_SCROLLBAR | DOCHOSTUIFLAG_OPENNEWWIN;

		// Set what happens when the user double-clicks on the object. Here we use the default.
		pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
		return S_OK;
	}

	// Called when the browser object shows its UI. This allows us to replace its menus and toolbars by creating our
	// own and displaying them here.
	HRESULT STDMETHODCALLTYPE WBClientSite::ShowUI( 
		/* [in] */ DWORD dwID,
		/* [in] */ IOleInPlaceActiveObject *pActiveObject,
		/* [in] */ IOleCommandTarget *pCommandTarget,
		/* [in] */ IOleInPlaceFrame *pFrame,
		/* [in] */ IOleInPlaceUIWindow *pDoc)
	{
		// We've already got our own UI in place so just return S_OK to tell the browser
		// not to display its menus/toolbars. Otherwise we'd return S_FALSE to let it do
		// that.
		return S_FALSE;
	}

	// Called when browser object hides its UI. This allows us to hide any menus/toolbars we created in ShowUI.
	HRESULT STDMETHODCALLTYPE WBClientSite::HideUI()
	{
		// We've already got our own UI in place so just return S_OK to tell the browser
		// not to display its menus/toolbars. Otherwise we'd return S_FALSE to let it do
		// that.
		return(S_OK);
		//return E_NOTIMPL;
	}

	// Called when the browser object wants to notify us that the command state has changed. We should update any
	// controls we have that are dependent upon our embedded object, such as "Back", "Forward", "Stop", or "Home"
	// buttons.
	HRESULT STDMETHODCALLTYPE WBClientSite::UpdateUI()
	{
		// We update our UI in our window message loop so we don't do anything here.
		return (S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's OnDocWindowActivate() function.
	// This informs off of when the object is getting/losing the focus.
	HRESULT STDMETHODCALLTYPE WBClientSite::OnDocWindowActivate( 
		/* [in] */ BOOL fActivate)
	{
		return(S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's OnFrameWindowActivate() function.
	HRESULT STDMETHODCALLTYPE WBClientSite::OnFrameWindowActivate( 
		/* [in] */ BOOL fActivate)
	{
		return(S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's ResizeBorder() function.
	HRESULT STDMETHODCALLTYPE WBClientSite::ResizeBorder( 
		/* [in] */ LPCRECT prcBorder,
		/* [in] */ IOleInPlaceUIWindow *pUIWindow,
		/* [in] */ BOOL fRameWindow)
	{
		return(S_OK);
	}

	// Called from the browser object's TranslateAccelerator routines to translate key strokes to commands.
	HRESULT STDMETHODCALLTYPE WBClientSite::TranslateAccelerator( 
		/* [in] */ LPMSG lpMsg,
		/* [in] */ const GUID *pguidCmdGroup,
		/* [in] */ DWORD nCmdID)
	{
		// We don't intercept any keystrokes, so we do nothing here. But for example, if we wanted to
		// override the TAB key, perhaps do something with it ourselves, and then tell the browser
		// not to do anything with this keystroke, we'd do:
		//
		//	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
		//	{
		//		// Here we do something as a result of a TAB key press.
		//
		//		// Tell the browser not to do anything with it.
		//		return(S_FALSE);
		//	}
		//
		//	// Otherwise, let the browser do something with this message.
		//	return(S_OK);

		// For our example, we want to make sure that the user can invoke some key to popup the context
		// menu, so we'll tell it to ignore all messages.
		// return(S_FALSE);

		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::GetOptionKeyPath( 
		/* [out] */ LPOLESTR *pchKey,
		/* [in] */ DWORD dw)
	{
		*pchKey = NULL;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::GetDropTarget( 
		/* [in] */ IDropTarget *pDropTarget,
		/* [out] */ IDropTarget **ppDropTarget)
	{
		*ppDropTarget = NULL;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::GetExternal( 
		/* [out] */ IDispatch **ppDispatch)
	{
		*ppDispatch = NULL;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::TranslateUrl( 
		/* [in] */ DWORD dwTranslate,
		/* [in] */ OLECHAR *pchURLIn,
		/* [out] */ OLECHAR **ppchURLOut)
	{
		*ppchURLOut = NULL;
		return S_FALSE;
	}

	HRESULT STDMETHODCALLTYPE WBClientSite::FilterDataObject( 
		/* [in] */ IDataObject *pDO,
		/* [out] */ IDataObject **ppDORet)
	{
		*ppDORet = NULL;
		return S_FALSE;
	}
}


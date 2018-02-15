

#pragma once
#include <oleidl.h>
#include <mshtmhst.h>
 
namespace ParaEngine
{
	class WBClientSite :public IOleClientSite, public IOleInPlaceFrame, public IOleInPlaceSite,public IDocHostUIHandler
	{
	public:
		 // constructors
		 WBClientSite()
		 {}
 
		 WBClientSite(HWND hwnd)
		 {
		  m_hWindow = hwnd;
		 }
 
		 //
		 // IUnknown methods
		 //
		 virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		  /* [in] */ REFIID riid,
		  /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
 
		 virtual ULONG STDMETHODCALLTYPE AddRef( void);
 
		 virtual ULONG STDMETHODCALLTYPE Release( void);
 
		 //
		 // IOleClientSite methods
		 //
		 virtual HRESULT STDMETHODCALLTYPE SaveObject( void);
 
		 virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
		  /* [in] */ DWORD dwAssign,
		  /* [in] */ DWORD dwWhichMoniker,
		  /* [out] */ IMoniker **ppmk);
 
		 virtual HRESULT STDMETHODCALLTYPE GetContainer( 
		  /* [out] */ IOleContainer **ppContainer);
 
		 virtual HRESULT STDMETHODCALLTYPE ShowObject( void);
 
		 virtual HRESULT STDMETHODCALLTYPE OnShowWindow( 
		  /* [in] */ BOOL fShow);
 
		 virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout( void);
 
		 //
		 // IOleWindow methods
		 //
		 virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE GetWindow( 
		  /* [out] */ HWND *phwnd);
 
		 virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp( 
		  /* [in] */ BOOL fEnterMode);
 
		 //
		 // IOleInPlaceUIWindow methods
		 //
		 virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE GetBorder( 
		  /* [out] */ LPRECT lprectBorder);
 
		 virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE RequestBorderSpace( 
		  /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);
 
		 virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE SetBorderSpace( 
		  /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);
 
		 virtual HRESULT STDMETHODCALLTYPE SetActiveObject( 
		  /* [unique][in] */ IOleInPlaceActiveObject *pActiveObject,
		  /* [unique][string][in] */ LPCOLESTR pszObjName);
 
		 //
		 // IOleInPlaceFrame methods
		 //
		 virtual HRESULT STDMETHODCALLTYPE InsertMenus( 
		  /* [in] */ HMENU hmenuShared,
		  /* [out][in] */ LPOLEMENUGROUPWIDTHS lpMenuWidths);
 
		 virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE SetMenu( 
		  /* [in] */ HMENU hmenuShared,
		  /* [in] */ HOLEMENU holemenu,
		  /* [in] */ HWND hwndActiveObject);
 
		 virtual HRESULT STDMETHODCALLTYPE RemoveMenus( 
		  /* [in] */ HMENU hmenuShared);
 
		 virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE SetStatusText( 
		  /* [unique][in] */ LPCOLESTR pszStatusText);
 
		 virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
		  /* [in] */ BOOL fEnable);
 
		 virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
		  /* [in] */ LPMSG lpmsg,
		  /* [in] */ WORD wID);
 
		 //
		 // IOleInPlaceSite methods
		 //
		 virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate( void);
 
		 virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate( void);
 
		 virtual HRESULT STDMETHODCALLTYPE OnUIActivate( void);
 
		 virtual HRESULT STDMETHODCALLTYPE GetWindowContext( 
		  /* [out] */ IOleInPlaceFrame **ppFrame,
		  /* [out] */ IOleInPlaceUIWindow **ppDoc,
		  /* [out] */ LPRECT lprcPosRect,
		  /* [out] */ LPRECT lprcClipRect,
		  /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);
 
		 virtual HRESULT STDMETHODCALLTYPE Scroll( 
		  /* [in] */ SIZE scrollExtant);
 
		 virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate( 
		  /* [in] */ BOOL fUndoable);
 
		 virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate( void);
 
		 virtual HRESULT STDMETHODCALLTYPE DiscardUndoState( void);
 
		 virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo( void);
 
		 virtual HRESULT STDMETHODCALLTYPE OnPosRectChange( /* [in] */ LPCRECT lprcPosRect);




		 //////////////////////////////////////////////////////////////////////////
		 //IDocHostUIHandler interface
		 //////////////////////////////////////////////////////////////////////////
		 virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( 
			 /* [in] */ DWORD dwID,
			 /* [in] */ POINT *ppt,
			 /* [in] */ IUnknown *pcmdtReserved,
			 /* [in] */ IDispatch *pdispReserved);
		 virtual HRESULT STDMETHODCALLTYPE GetHostInfo( 
			 /* [out][in] */ DOCHOSTUIINFO *pInfo);
		 virtual HRESULT STDMETHODCALLTYPE ShowUI( 
			 /* [in] */ DWORD dwID,
			 /* [in] */ IOleInPlaceActiveObject *pActiveObject,
			 /* [in] */ IOleCommandTarget *pCommandTarget,
			 /* [in] */ IOleInPlaceFrame *pFrame,
			 /* [in] */ IOleInPlaceUIWindow *pDoc);
		 virtual HRESULT STDMETHODCALLTYPE HideUI();
		 virtual HRESULT STDMETHODCALLTYPE UpdateUI();

		 /*
		 virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
			  BOOL fEnable);
			  */
	 

		 virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
			 /* [in] */ BOOL fActivate);
		 virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
			 /* [in] */ BOOL fActivate);
		 virtual HRESULT STDMETHODCALLTYPE ResizeBorder( 
			 /* [in] */ LPCRECT prcBorder,
			 /* [in] */ IOleInPlaceUIWindow *pUIWindow,
			 /* [in] */ BOOL fRameWindow);
		 virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
			 /* [in] */ LPMSG lpMsg,
			 /* [in] */ const GUID *pguidCmdGroup,
			 /* [in] */ DWORD nCmdID);
		 virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
			 /* [out] */ LPOLESTR *pchKey,
			 /* [in] */ DWORD dw);
		 virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
			 /* [in] */ IDropTarget *pDropTarget,
			 /* [out] */ IDropTarget **ppDropTarget);
		 virtual HRESULT STDMETHODCALLTYPE GetExternal( 
			 /* [out] */ IDispatch **ppDispatch);
		 virtual HRESULT STDMETHODCALLTYPE TranslateUrl( 
			 /* [in] */ DWORD dwTranslate,
			 /* [in] */ OLECHAR *pchURLIn,
			 /* [out] */ OLECHAR **ppchURLOut);
		 virtual HRESULT STDMETHODCALLTYPE FilterDataObject( 
			 /* [in] */ IDataObject *pDO,
			 /* [out] */ IDataObject **ppDORet);


		 /* //////////////////////////////////////////////////
		 // We don't want those functions to access global
		 // variables, because then we couldn't use more
		 // than one browser object. (ie, we couldn't have
		 // multiple windows, each with its own embedded
		 // browser object to display a different web page).
		 //
		 // So here is where I added my extra HWND that the
		 // CWBIOleInPlaceFrame function Frame_GetWindow() needs
		 // to access.
		 ////////////////////////////////////////////////// */
		 HWND    m_hWindow;
	};
}


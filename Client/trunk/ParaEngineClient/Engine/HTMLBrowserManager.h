#pragma once
#include "2dengine/EventBinding.h"
#include "IHTMLRenderer.h"
#include <string>

namespace ParaEngine
{
	using namespace std;
	class CHTMLBrowserManager;
	class IHTMLBrowserManager;
	class CEventBinding;
	class SimpleScript;

	/** an instance of HTML browser. */
	class CHTMLBrowser : IEmbeddedBrowserWindowObserver
	{
	public:
		CHTMLBrowser(CHTMLBrowserManager* manager, int nBrowserWindowId);
		~CHTMLBrowser();
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IEmbeddedBrowserWindowObserver interface
		virtual void onPageChanged( const IEmbeddedBrowserWindowObserver::EventType& eventIn );
		virtual void onNavigateBegin( const IEmbeddedBrowserWindowObserver::EventType& eventIn );
		virtual void onNavigateComplete( const IEmbeddedBrowserWindowObserver::EventType& eventIn );
		virtual void onUpdateProgress( const IEmbeddedBrowserWindowObserver::EventType& eventIn );
		virtual void onStatusTextChange( const IEmbeddedBrowserWindowObserver::EventType& eventIn );
		virtual void onLocationChange( const IEmbeddedBrowserWindowObserver::EventType& eventIn );
		virtual void onClickLinkHref( const IEmbeddedBrowserWindowObserver::EventType& eventIn );

	public:
		/** something like delete this*/
		void Release();

		bool setSize( int widthIn, int heightIn );
		bool scrollByLines( int linesIn );
		bool setBackgroundColor( const int redIn, const int greenIn, const int blueIn );
		bool setEnabled( bool enabledIn );

		// add/remove yourself as an observer on browser events - see IEmbeddedBrowserWindowObserver declaration
		bool addObserver( IEmbeddedBrowserWindowObserver* subjectIn );
		bool remObserver( IEmbeddedBrowserWindowObserver* subjectIn );

		// navigation - self explanatory
		bool navigateTo( const std::string uriIn );
		bool navigateStop();
		bool canNavigateBack();
		bool navigateBack();
		bool canNavigateForward();
		bool navigateForward();

		// access to rendered bitmap data
		const unsigned char* grabBrowserWindow();		// renders page to memory and returns pixels
		const unsigned char* getBrowserWindowPixels();	// just returns pixels - no render
		const int getBrowserWidth();						// current browser width (can vary slightly after page is rendered)
		const int getBrowserHeight();					// current height
		const int getBrowserDepth();						// depth in bytes
		const int getBrowserRowSpan();					// width in pixels * depth in bytes

		// mouse/keyboard interaction
		bool mouseDown( int xPosIn, int yPosIn );		// send a mouse down event to a browser window at given XY in browser space
		bool mouseUp( int xPosIn, int yPosIn );			// send a mouse up event to a browser window at given XY in browser space
		bool mouseMove( int xPosIn, int yPosIn );		// send a mouse move event to a browser window at given XY in browser space
		bool keyPress( int keyCodeIn );					// send a key press event to a browser window 
		bool focusBrowser( bool focusBrowserIn );		// set/remove focus to given browser window

		// accessor/mutator for scheme that browser doesn't follow - e.g. paraengine.com://
		void setNoFollowScheme( std::string schemeIn );
		std::string getNoFollowScheme();

	private:
		/** this function will return when the video is initialized and its size is known. 
		* Internally it will wait for the codec to be ready. 
		* @note: there is a timeout of 100 milliseconds, if the codec still has no idea of the media type, it will return E_UNEXPECTED.*/
		HRESULT CreateTexture(LPDIRECT3DTEXTURE9 *ppTexture);

		/** update the texture with the latest content. It is safe to call this function at higher frequency than the movie frame rate. 
		* it will internally detect if the current frame has changed since the last invocation of this function. */
		HRESULT UpdateTexture(LPDIRECT3DTEXTURE9 pTexture);

	public:
		/** get window id */
		int GetBrowserWindowID(){return m_nBrowserWindowId;}
		
		/**
		* send a windows message to the inner flash player window.
		* in most cases: we will forward mouse and key messages to the flash player using this function.
		*/
		LRESULT SendMessage( UINT Msg,WPARAM wParam,LPARAM lParam);
		/**
		* post a windows message to the inner flash player window.
		* in most cases: we will forward mouse and key messages to the flash player using this function.
		*/
		LRESULT PostMessage( UINT Msg,WPARAM wParam,LPARAM lParam);

		/** get the window name. */
		const char* GetName();

		/** release d3d texture.*/
		void InvalidateDeviceObjects();

		/// get the texture. 
		LPDIRECT3DTEXTURE9 GetTexture();

		/** the last url when navigateTo() is called. default is ""*/
		const string& GetLastNavURL(){ return m_LastNavURL;}
	public:
		/// file name that the current texture is associated with. 
		string m_filename;
		/** the last url when navigateTo() is called. default is ""*/
		string m_LastNavURL;

		int m_nBrowserWindowId;
		CHTMLBrowserManager* m_manager;
	
		/// if a flash player is never used for this number of time, it will be deleted. 
		float m_fTimeOut;
		/// last time the GetTexture() is called. 
		float m_fLastUsedTime;

		/// dynamic d3d texture
		LPDIRECT3DTEXTURE9 m_pTexture;
		D3DFORMAT m_TextureFormat;
		bool m_bUseDynamicTextures;

		/// whether the texture has been update in this rendering frame. if there are multiple calls to GetTexture()
		/// in the same rendering frame, we will only copy texture once. 
		bool m_bTextureUpdated;

		/// this flag is set to true every frame move. and set to false when the target image is saved to temp buffer
		bool m_bNeedUpdate;
	};

	/**
	* managing HTML browsers. 
	*/
	class CHTMLBrowserManager
	{
	public:
		CHTMLBrowserManager(void);
		~CHTMLBrowserManager(void);

		/** HTML browser events */
		enum HTMLBROWSER_EVENTS
		{
			EM_onPageChanged = 0,
			EM_onNavigateBegin, 
			EM_onNavigateComplete,
			EM_onUpdateProgress,
			EM_onStatusTextChange,
			EM_onLocationChange,
			EM_onClickLinkHref,
		};
	public:
		bool LoadHTMLManagerPlugin();

		HRESULT InitDeviceObjects();// device independent
		HRESULT RestoreDeviceObjects(); // device dependent
		HRESULT InvalidateDeviceObjects();
		HRESULT DeleteDeviceObjects();
		/** clean up. this function should be called just before the application exits. */
		void Cleanup();
		/** initialize all assets created so far to accelerate loading during game play. */
		void LoadAsset();
		/** uninitialize all assets created so far to save some memory */
		void UnloadAsset();
		/** Garbage Collect(free resources of) all unused entity.
		* any inactive one in the last frame will be removed.*/
		void GarbageCollectAll();

		//////////////////////////////////////////////////////////////////////////
		// house keeping
		bool init( std::string appBaseDirIn, std::string profileDirNameIn );
		bool reset();
		bool clearCache();
		int getLastError();
		const std::string getVersion();
		void setBrowserAgentId( std::string idIn );
		// how many browsers can be opened at the same time
		int GetMaxWindowNum();
		void SetMaxWindowNum(int nNum);

		//////////////////////////////////////////////////////////////////////////
		// browser window - creation/deletion, mutation etc.
		/**
		* create a new browser window with the given name and dimension in pixels.
		*/
		CHTMLBrowser* createBrowserWindow( const char* sFileName, int browserWindowWidthIn, int browserWindowHeightIn );
		bool destroyBrowserWindow( int browserWindowIdIn );
		/** it may return NULL, if the browser does not exist */
		CHTMLBrowser* GetBrowserWindow(const char* sFileName);

		/** it may return NULL, if the browser does not exist */
		CHTMLBrowser* GetBrowserWindow(int nWindowID);

		/** first get, if not exist if will create using default parameters. */
		CHTMLBrowser* CreateGetBrowserWindow(const char* sFileName);
	
		/** get the main plugin interface */
		IHTMLBrowserManager* GetInterface(){return m_pInterface;};

		//////////////////////////////////////////////////////////////////////////
		// event handlers

		/** if you want to erase an event script, use script=NULL;*/
		void				SetEventScript(int nEvent,const SimpleScript *script);
		const SimpleScript* GetEventScript(int nEvent)const;
		/** check if the object has a certain event handler 
		* @etype: script type.*/
		bool				HasEvent(int etype);
		/** call events. */
		bool ActivateScript(int etype, const string &code);

	private:
		vector <CHTMLBrowser*> m_browsers;
		IHTMLBrowserManager* m_pInterface;
		int m_nMaxWindowNum;
		EventBinding_cow_type m_pEventbinding;
	};

}

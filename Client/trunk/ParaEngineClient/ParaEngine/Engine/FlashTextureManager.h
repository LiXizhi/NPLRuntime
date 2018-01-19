#pragma once
#include <vector>
#include <string>
#include "FileManager.h"

#include "util/mutex.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace ParaEngine
{
	using namespace std;
	/**
	* for SWF rect
	*/
	struct SWFRect
	{
		int m_bitSize;
		int m_minX;
		int m_minY;
		int m_maxX;
		int m_maxY;
	public:
		int getMinX();
		int getMinY();
		int getMaxX();
		int getMaxY();

		void setMinX( int minX );
		void setMinY( int minY );
		void setMaxX( int maxX );
		void setMaxY( int maxY );

		
		SWFRect( int minX, int minY, int maxX, int maxY );
		SWFRect();

		/// Load from file
		void Init( CParaFile& in, bool bUnsigned=false);
	};
	/**
	* flash SWF header
	Signature           UI8           Signature byte always ¡®F¡¯      
	Signature           UI8           Signature byte always ¡®W¡¯      
	Signature           UI8           Signature byte always ¡®S¡¯      
	Version           UI8           Single byte file version (e.g. 0x04F for SWF 4)      
	FileLength           UI32           Length of entire file in bytes      
	FrameSize           RECT           Frame size in twips  (1/20 pixels): A twip is defined as being 1/1440 of an inch
	FrameRate           UI16           Frame delay in 8.8 fixed number of frames per second      
	FrameCount           UI16           Total number of frames in movie      
	*/
	struct SWFHeader
	{
		byte Signature[3]; // 'F', 'W', 'S' or 'C', 'W', 'S'(compressed)
		byte Version;
		DWORD fileLength;

		// rect
		SWFRect frameSize;
		short frameRate;
		short frameCount;

	public:
		SWFHeader();
		SWFHeader(CParaFile& in);
	};

	/** an instance of flash player.*/
	class CFlashPlayer
	{
	public:
		CFlashPlayer(int nFlashPlayerIndex);
		~CFlashPlayer();

		typedef boost::shared_ptr<boost::thread> Boost_Thread_ptr_type;

	public:
		/** whether the current player is free to load a new movie. */
		bool IsFree(){return m_bFree;}

		/** load a movie by file name, it can be URL, a virtual file, a file, or a resource */
		void LoadMovie(const char* filename);

		/** put flash variables. This function must be loaded before a flash movie is loaded. 
		* @param args: such as "company=http://paraengine.com"
		*/
		void PutFlashVars(const char* args);

		/**
		[ ActionScript ] 
		import flash.external.*; 
		ExternalInterface.addCallback("CallMeFromApplication", this, InternalFunction); 

		function InternalFunction(str: string): string { 
		TextArea1.text = str; 
		return "The function was called successfully"; 
		} 
		[ C++ ] 
		string res = CallFlashFunction("<invoke name=\"CallMeFromApplication\" returntype=\"xml\"><arguments><string>Some text for FlashPlayerControl</string></arguments></invoke>"); 
		@param res: result is returned as a string. 
		@note: the max return string length of the flash function can not exceed 2048 bytes.
		@remark: one can also execute NPL script from flash script. Such as below. 
		[ ActionScript ]
		on (click) {   
		_root.TextArea1.text = flash.external.ExternalInterface.call("SomeNPLFunction(parameters)");
		}

		Please note that the return value must be explicitly set in the NPL script, by calling SetFlashReturnValue() function. 
		Otherwise empty string is returned. 

		[Security alert]: must be run in sand-box environment or there is a security here. Plus this function must be called from the same thread as ParaEngine main thread.  
		*/
		const char* CallFlashFunction(const char* sFunc);

		/** This function is called from NPL script to set the return value of an NPL function which can be called by the flash external interface call. 
		* @param sResult: xml encoded string. such as "<string>Result from NPL</string>". if NULL, it is set to "<string></string>"
		*/
		void SetFlashReturnValue(const char* sResult);

		/** get flash return value */
		const char* GetFlashReturnValue();

		/** unload current movie. However d3d texture and the flash player window are not released for reuse.*/
		void UnloadMovie();
		
		/** release d3d texture.*/
		void InvalidateDeviceObjects();

		/** called each render frame move, it may unload time out movies and force flash window to update itself
		* @param fCurTime: Elapsed Time in seconds
		*/
		void RenderFrameMove(float fCurTime);

		/**
		* get texture info
		* @param width [out]
		* @param height [out]
		* @return 
		*/
		bool GetTextureInfo(int* width, int* height);

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

		/** get the movie file name. */
		const char* GetFileName();

		/** get unique index of this flash player . */
		int GetIndex() {return m_nFlashPlayerIndex;}


		/** whether this player is using a real win32 window for display. */
		bool IsWindowMode() { return m_bWindowMode;};

		/** set whether this player is using a real win32 window for display. Default to false. */
		void SetWindowMode(bool bIsWindowMode);

		/** move or resize the window. */
		void MoveWindow(int x, int y, int width, int height, bool bRepaint=true);


		/** this function is called by ParaEngineApp whenever the window size is changed. We usually needs to 
		* reposition all flash windows according to the new size. */
		void OnSizeChange();

		/** show or hide the window. */
		void SetWindowVisible(bool bVisible);

		/** repaint all window-mode flash window. in case of full screen render, 
		* we may need to repaint when the background scene finished. 
		*/
		void Repaint();

		/** this can be called from any thread*/
		void SetFocus();

		/** get the window handle if any. */
		HWND GetWindowHandle();

		/** copy the current flash window client area to a memory buffer in system memory (m_lpPixels). 
		* One may need to call Repaint() prior to this call to force flash window to update itself first. 
		*/
		void UpdatePixelsBuffer();
	private:
		/** this function will return when the video is initialized and its size is known. 
		* Internally it will wait for the codec to be ready. 
		* @note: there is a timeout of 100 milliseconds, if the codec still has no idea of the media type, it will return E_UNEXPECTED.*/
		HRESULT CreateTexture(LPDIRECT3DTEXTURE9 *ppTexture);

		/** update the texture with the latest content. It is safe to call this function at higher frequency than the movie frame rate. 
		* it will internally detect if the current frame has changed since the last invocation of this function. */
		HRESULT UpdateTexture(LPDIRECT3DTEXTURE9 pTexture);

	public:
		/** create the default win32 windows for rendering. and assigning to m_hWnd. 
		* window is created in another thread. Since windows uses single threaded apartment, the window procedure should also be in that thread. 
		*/
		HRESULT StartWindowThread();
		void DefaultWinThreadProc();

		/** process WM_USER messages*/
		bool MsgProcCustom(UINT message,WPARAM wParam,LPARAM lParam);

		/** post a WM_USER message to the window thread. */
		bool PostWinThreadMessage(UINT message,WPARAM wParam,LPARAM lParam);
		
		/** this function is called, whenever the associated main parent window changes. 
		Internally, we just call win32 function SetParent to change any already created flash windows. */
		bool SetParentWindow(HWND hWnd);

		/** if the flash window has focus. */
		bool HasFocus();
	public:
		/// get the texture. 
		LPDIRECT3DTEXTURE9 GetTexture();
		
		/// file name that the current texture is associated with. 
		string m_filename;
		/// if a flash player is never used for this number of time, it will be deleted. 
		float m_fTimeOut;
		/// last time the GetTexture() is called. 
		float m_fLastUsedTime;
		/// index of this flash player
		int m_nFlashPlayerIndex;
		
		/// dynamic d3d texture
		LPDIRECT3DTEXTURE9 m_pTexture;
		D3DFORMAT m_TextureFormat;
		bool m_bUseDynamicTextures;

		/// whether the texture has been update in this rendering frame. if there are multiple calls to GetTexture()
		/// in the same rendering frame, we will only copy texture once. 
		bool m_bTextureUpdatedThisFrame;
		/// whether the current player is free.
		bool m_bFree;
		/// There are two modes of rendering are supported:
		/// 1. Full transparent mode. In this mode, you get pixel colors with alpha component. You can create a texture with alpha channel.
		/// 2. Simple transparent mode. In this mode, real value of alpha channel is not available. But this mode faster than full transparent mode.\n\nPress Yes if you want to see how full transparent mode works.\nPress No if you want to see how simple transparent mode works.
		bool m_bTransparent;
		/// flash control handle
		HWND m_hwndFlashPlayerControl;
		
		int m_nX;
		int m_nY;
		int m_nWidth;
		int m_nHeight;
		int m_nBufWidth;// thread unsafe
		int m_nBufHeight;// thread unsafe
		int m_nTextureWidth;
		int m_nTextureHeight;
		int m_nWindowX;
		int m_nWindowY;
		int m_nWindowWidth;
		int m_nWindowHeight;

		DWORD* m_lpPixels; // thread unsafe
		LinearColor m_backgroundColor;

		/// this flag is set to true every frame move. and set to false when the flash image is saved to temp buffer
		bool m_bNeedFlashUpdate;
		bool m_bFlipVertical;

		/** if true, we use a real win32 window. */
		bool m_bWindowMode;

		// the most recent flash return value. 
		string m_sFlashReturnValue;

		//  the default window thread ptr. 
		Boost_Thread_ptr_type m_win_thread;
		ParaEngine::mutex	  m_win_thread_mutex;
		bool m_bWindowCreated;
		bool m_bWinThreadMovieLoaded;
		// the window thread id
		DWORD m_dwWinThreadID;

		string m_sCurrentFlashFunc;
		string m_sCurrentFlashFuncResponse;
		bool m_bInFlashFunc;

		bool m_bHasFocus;
	};

	/**
	* managing flash textures. 
	*/
	class CFlashTextureManager : public ISearchPathManager
	{
	public:
		CFlashTextureManager(void);
		virtual ~CFlashTextureManager(void);

	public:
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

		/** this function is called by ParaEngineApp whenever the window size is changed. We usually needs to 
		* reposition all flash windows according to the new size. */
		void OnSizeChange();

		/**
		* set the maximum number of flash players in the background. Default value is 1
		*/
		void SetMaxFlashPlayerNum(int nNum);
		/**
		* Get the maximum number of flash players in the background. Default value is 1
		*/
		int GetMaxFlashPlayerNum();

		/**
		* get flash player pointer by name. this function may return NULL if the player is created successfully. 
		* this function will try to create and load the player if it does not exist.
		*/
		CFlashPlayer* GetFlashPlayer(const char* sFileName);
		
		/**
		* get flash player pointer by index. this function does not create any player if there is no player at the given index. 
		*/
		CFlashPlayer* GetFlashPlayer(int nIndex);
		
		/**
		* get flash player index by file name. this function does not create any player if there is no player with the name. 
		* @return -1 is returned if no player is found.
		*/
		int GetFlashPlayerIndex(const char* sFileName);

		/**
		* get DirectX texture by name. this function may return NULL if the player is not loaded yet. 
		* the caller may use a traditional static texture in such cases.
		*/
		LPDIRECT3DTEXTURE9 GetTexture(const char* sFileName);
		/** get the texture info of the given flash player.*/
		bool GetTextureInfo(const char* sFileName, int* width, int* height);

		/**
		* unload the given texture, even it has not timed out yet.
		* return true if unloaded
		*/
		bool UnloadTexture(const char* sFileName);
		
		/** this function should be called each rendering frame. 
		* it will close unused flash players and mark if all flash texture to be not updated for the frame. 
		* @param fElapsedTime: fElapsedTime
		*/
		void RenderFrameMove(float fElapsedTime);

		/** check whether a file exist locally on this machine 
		* @return: return the file name. it may be "" or a file name that can be opened via CParaFile
		*/
		string FindFile(const string& filename, bool bFindInSearchPaths);

		/** repaint all window-mode flash window. in case of full screen render, we may need to repaint when the background scene finished. 
		*/
		void RepaintAll();

		/** render flash windows at the same position as the flash window using directX API. 
		* In windowed mode directX, this function is not necessary, since flash windows are rendered with GDI with the directx background. 
		* In fullscreen mode directX, we need to call this function to ensure that flash windows are rendered to screen. 
		*/
		void RenderFlashWindows(SceneState& sceneState);
	
		/** this function is called, whenever the associated main parent window changes. 
		Internally, we just call win32 function SetParent to change any already created flash windows. */
		bool SetParentWindow(HWND hWnd);
		
		/** if any of its flash window has focus, it will return true*/
		static bool HasFocus();
	public:
		vector <CFlashPlayer*> m_FlashPlayers;

		static bool m_bHasFocus;
	};
}

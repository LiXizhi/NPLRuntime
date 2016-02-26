#pragma once
#ifdef USE_DIRECTX_RENDERER
namespace ParaEngine
{
	struct DXUT_SCREEN_VERTEX;

	/** this allows us to create another device, such as in a worker thread to 
	* perform some background work while the main directX device has other work to do. */
	class CDirectXEngine
	{
	public:
		CDirectXEngine();
		~CDirectXEngine(void);
	public:
		void InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);
		void RestoreDeviceObjects();
		void InvalidateDeviceObjects();
		void DeleteDeviceObjects();

		virtual HRESULT Create();
		virtual HRESULT Destroy();

		virtual LPDIRECT3D9 GetD3DObject();
		virtual LPDIRECT3DDEVICE9 GetD3DDevice();

		virtual bool OnFrameMove();

	public:
		//some GUI functions here 

		HRESULT Clear(const Color &color);
		//these are internal graphic manipulating functions
		HRESULT DrawTriangleStrip(LPDIRECT3DTEXTURE9 pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertexData);
		HRESULT DrawTriangleList(LPDIRECT3DTEXTURE9 pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertexData);
		void SetTransform(const Matrix4 *matTransform);
		void GetTransform(Matrix4 *matTransform);

		LPDIRECT3DTEXTURE9 GetRenderTarget();

		// actuall no need to be called. since we never present to windows. We only render to render target. 
		bool BeginScene();
		void EndScene();
		/** this function may be called regular. mainly for checking device lost, etc*/
		bool PresentScene();

	protected:
		static bool CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed);
		
		HRESULT Reset3DEnvironment();
		HRESULT Initialize3DEnvironment();
		void Cleanup3DEnvironment();

		void BuildPresentParamsFromSettings(HWND * pFocusWnd=NULL);
	protected:
		bool Begin2D();
		void End2D();

	private:
		LPDIRECT3D9       m_pD3D;              // The main D3D object
		LPDIRECT3DDEVICE9 m_pd3dDevice;        // The D3D rendering device
		D3DPRESENT_PARAMETERS m_d3dpp;         // Parameters for CreateDevice/Reset

		HWND m_hwndDeviceWindow;
		bool m_bDeviceLost;

		LPDIRECT3DTEXTURE9    m_pRenderTargetTexture;
		LPDIRECT3DSURFACE9	  m_pRenderTargetSurface;
		Matrix4 m_transform;

		bool m_bDeviceObjectsRestored;
		bool m_bDeviceObjectsInited;
		bool m_bIsSceneBegin;
	};

	/** pre-stored device parameters for the directX device. */
	class DirectXEngine
	{
	public:
		DirectXEngine();
		~DirectXEngine(void);

		/** Set render target
		@param nIndex: current only 0 is supported
		@param sRenderTarget: if this is NULL, the back buffer will be used.
		*/
		HRESULT SetRenderTarget(int nIndex=0, LPDIRECT3DSURFACE9  sRenderTarget=NULL);
		/** return the current render target, without increasing the reference count of the surface */
		LPDIRECT3DSURFACE9  GetRenderTarget(int nIndex=0);

		void InitDeviceObjects(LPDIRECT3D9 pd3d, LPDIRECT3DDEVICE9 pd3dDevice, IDirect3DSwapChain9* pd3dSwapChain=NULL);
		void RestoreDeviceObjects();
		void InvalidateDeviceObjects();
		void DeleteDeviceObjects();


		/** get the vertex shader version. This can be roughly used to auto-adjust graphics settings. 
		* @return 0,1,2,3,4
		*/
		int GetVertexShaderVersion();

		/** get the pixel shader version. This can be roughly used to auto-adjust graphics settings.  
		* @return 0,1,2,3,4
		*/
		int GetPixelShaderVersion();

		void GetDisplayMode(D3DDISPLAYMODE **displayModes,int &displayModeCount,D3DFORMAT fmt=D3DFMT_X8R8G8B8);
		Vector2 GetMonitorResolution();

		LPDIRECT3DDEVICE9 GetRenderDevice();

		int GetBackBufferWidth();
		int GetBackBufferHeight();
	public:
		LPDIRECT3D9       m_pD3D;              // The main D3D object
		LPDIRECT3DDEVICE9 m_pd3dDevice;        // The D3D rendering device
		IDirect3DSwapChain9* m_pd3dSwapChain;
		D3DCAPS9          m_d3dCaps;           // Caps for the device
		D3DSURFACE_DESC   m_d3dsdBackBuffer;   // Surface desc of the back buffer
		LPDIRECT3DSURFACE9       m_backBufferRenderTarget;// the back buffer surface
		// current render target, this is a week pointer(no reference count).
		LPDIRECT3DSURFACE9       m_CurrentRenderTarget; 
	
		
	};
}
#endif
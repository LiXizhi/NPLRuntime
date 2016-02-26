//-----------------------------------------------------------------------------
// Class:	DirectXEngine
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DirectXEngine.h"
#include "ParaEngineApp.h"
#include "ParaWorldAsset.h"

using namespace ParaEngine;


//////////////////////////////////////////////////////////////////////////
//
// DirectXEngine
//
//////////////////////////////////////////////////////////////////////////

DirectXEngine::DirectXEngine()
{
	memset(this, 0, sizeof(DirectXEngine));
}

DirectXEngine::~DirectXEngine(void)
{
}


int DirectXEngine::GetVertexShaderVersion()
{
	DWORD ShaderVersion = m_d3dCaps.VertexShaderVersion;
	if(ShaderVersion < D3DVS_VERSION(1,0))
		return 0;
	else if(ShaderVersion < D3DVS_VERSION(2,0))
		return 1;
	else if(ShaderVersion < D3DVS_VERSION(3,0))
		return 2;
	else if(ShaderVersion < D3DVS_VERSION(4,0))
		return 3;
	else if(ShaderVersion < D3DVS_VERSION(5,0))
		return 4;
	else
		return 5;
}

void DirectXEngine::GetDisplayMode(D3DDISPLAYMODE **displayModes,int &displayModeCount,D3DFORMAT fmt)
{
	displayModeCount = m_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT,fmt);
	if(displayModeCount < 1)
		return;

	*displayModes = new D3DDISPLAYMODE[displayModeCount];
	D3DDISPLAYMODE *current = *displayModes;
	for(int i=0;i<displayModeCount;i++)
	{
		m_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,fmt,i,current);
		current++;
	}
}

Vector2 DirectXEngine::GetMonitorResolution()
{
	D3DDISPLAYMODE mode;
	m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&mode);
	return Vector2((float)mode.Width,(float)mode.Height);
}

int DirectXEngine::GetPixelShaderVersion()
{
	DWORD PixelVersion = m_d3dCaps.PixelShaderVersion;

	if(PixelVersion < D3DPS_VERSION(1,0))
		return 0;
	else if(PixelVersion < D3DPS_VERSION(2,0))
		return 1;
	else if(PixelVersion < D3DPS_VERSION(3,0))
		return 2;
	else if(PixelVersion < D3DPS_VERSION(4,0))
		return 3;
	else if(PixelVersion < D3DPS_VERSION(5,0))
		return 4;
	else
		return 5;
}

void DirectXEngine::InitDeviceObjects(LPDIRECT3D9 pd3d, LPDIRECT3DDEVICE9 pd3dDevice, IDirect3DSwapChain9* pd3dSwapChain)
{
	m_pD3D = pd3d;
	m_pd3dDevice = pd3dDevice;
	m_pd3dSwapChain = pd3dSwapChain;
	// Store render target surface desc
	LPDIRECT3DSURFACE9 pBackBuffer = NULL;
	if(m_pd3dSwapChain)
	{
		m_pd3dSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	}
	else if(m_pd3dDevice)
	{
		m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	}
	pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
	pBackBuffer->Release();

	// Store device Caps	
	m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
}

void DirectXEngine::RestoreDeviceObjects()
{
	// Store back buffer
	if(m_pd3dSwapChain)
	{
		m_pd3dSwapChain->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &m_backBufferRenderTarget );
	}
	else if(m_pd3dDevice)
	{
		HRESULT hr = m_pd3dDevice->GetRenderTarget( 0,&m_backBufferRenderTarget);
		assert(hr==S_OK);
	}
	
	m_backBufferRenderTarget->GetDesc( &m_d3dsdBackBuffer );
	m_CurrentRenderTarget = m_backBufferRenderTarget;
}

void DirectXEngine::InvalidateDeviceObjects()
{
	SAFE_RELEASE(m_backBufferRenderTarget);
}

void DirectXEngine::DeleteDeviceObjects()
{
	m_pd3dDevice = NULL;
}

HRESULT DirectXEngine::SetRenderTarget(int nIndex, LPDIRECT3DSURFACE9  sRenderTarget)
{
	if(sRenderTarget == m_CurrentRenderTarget)
		return S_OK;
	if(sRenderTarget==0)
		sRenderTarget = m_backBufferRenderTarget;
	
	HRESULT hr = m_pd3dDevice->SetRenderTarget(nIndex, sRenderTarget);
	if(SUCCEEDED(hr))
	{
		m_CurrentRenderTarget = sRenderTarget;
	}
	return hr;
}

LPDIRECT3DSURFACE9  DirectXEngine::GetRenderTarget(int nIndex)
{
	return m_CurrentRenderTarget;
}

LPDIRECT3DDEVICE9 ParaEngine::DirectXEngine::GetRenderDevice()
{
	return m_pd3dDevice;
}

int ParaEngine::DirectXEngine::GetBackBufferWidth()
{
	return m_d3dsdBackBuffer.Width;
}

int ParaEngine::DirectXEngine::GetBackBufferHeight()
{
	return m_d3dsdBackBuffer.Height;
}


//////////////////////////////////////////////////////////////////////////
//
// CDirectXEngine
//
//////////////////////////////////////////////////////////////////////////

CDirectXEngine::CDirectXEngine()
:m_hwndDeviceWindow(NULL), m_pD3D(NULL), m_pd3dDevice(NULL),m_bDeviceLost(false), m_pRenderTargetTexture(NULL),m_pRenderTargetSurface(NULL),
m_bDeviceObjectsRestored(false), m_bDeviceObjectsInited(false), m_bIsSceneBegin(false)
{
	ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS)); 
}

CDirectXEngine::~CDirectXEngine(void)
{
}

LPDIRECT3DTEXTURE9 CDirectXEngine::GetRenderTarget() 
{
	return m_pRenderTargetTexture;
}

bool CDirectXEngine::OnFrameMove()
{
	HRESULT hr = S_OK;

	// Test the cooperative level to see if it's ok to render
	if(m_bDeviceLost)
	{
		// Test the cooperative level to see if it's okay to render
		if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
		{
			// If the device was lost, do not render until we get it back
			if( D3DERR_DEVICELOST == hr )
				return true;
			// Check if the device needs to be reset.
			if( D3DERR_DEVICENOTRESET == hr )
			{
				if( FAILED( hr = Reset3DEnvironment() ) )
				{
					OUTPUT_LOG("error: failed to reset 3d environment in worker thread");
				}
			}
		}
		m_bDeviceLost = false;
	}
	// TODO: do the render here 

	// is it necessary if we do not render to back buffer any more
	//hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	//if( D3DERR_DEVICELOST == hr )
	//	m_bDeviceLost = true;
	return true;
}

LRESULT CALLBACK CDirectXEngine_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

HRESULT CDirectXEngine::Create()
{
	// Start clean
	Destroy();

	HRESULT hr;

	ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS)); 

	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D == NULL)
	{
		OUTPUT_LOG("error: failed creating d3d object in worker thread\n");
	}

	// Register the windows class
	WNDCLASSW wndClass = { 0, CDirectXEngine_WndProc, 0, 0, NULL,
		NULL,
		NULL,
		NULL,
		NULL, L"PE_BG_DEVICE" };
	RegisterClassW( &wndClass );


	/** Note LiXizhi: only one full-screen mode is possible now. 
	The concept of exclusive full-screen mode is retained in Microsoft DirectX 9.0, but it is kept entirely implicit 
	in the IDirect3D9::CreateDevice and IDirect3DDevice9::Reset method calls. Whenever a device is successfully reset 
	or created in full-screen operation, the Microsoft Direct3D object that created the device is marked as owning 
	all adapters on that system. This state is known as exclusive mode, and at this point the Direct3D object owns 
	exclusive mode. Exclusive mode means that devices created by any other Direct3D9 object can neither assume 
	full-screen operation nor allocate video memory. In addition, when a Direct3D9 object assumes exclusive mode, 
	all devices other than the device that went full-screen are placed into the lost state.
	*/

	// Create a device window
	m_hwndDeviceWindow = CreateWindowW(L"PE_BG_DEVICE", L"ParaEngine BG Device Window", WS_POPUP | WS_DISABLED, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
	// m_hwndDeviceWindow = CreateWindow("PE_BG_DEVICE", TEXT("ParaEngine BG Device Window"), WS_CHILD, 0, 0, 1, 1, *g_pHwndHWND, NULL, NULL, NULL);

	if(m_hwndDeviceWindow == 0)
	{
		OUTPUT_LOG("error: failed creating render device window. error code is %d\n", GetLastError());
	}

	// Initialize the 3D environment for the app
	if( FAILED( hr = Initialize3DEnvironment() ) )
	{
		SAFE_RELEASE( m_pD3D );
		OUTPUT_LOG("warning: failed to init 3d environment.");
		return E_FAIL;
	}

	OUTPUT_LOG("Background render device is successfully created\n");

	return S_OK;
}

void CDirectXEngine::BuildPresentParamsFromSettings(HWND * pFocusWnd)
{
	ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS)); 

	D3DDISPLAYMODE Mode;

	m_pD3D->GetAdapterDisplayMode(0, &Mode);

	m_d3dpp.BackBufferFormat = Mode.Format;
	m_d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD; // D3DSWAPEFFECT_COPY;
	m_d3dpp.hDeviceWindow = m_hwndDeviceWindow;
	// TODO: IsWindowedMode() is not thread-safe, but use it anyway, since it is not likely to run concurrently. 
	bool bWindowed = true; //CParaEngineApp::GetInstance()->IsWindowedMode();
	//m_d3dpp.Windowed = bWindowed ? TRUE: FALSE;
	m_d3dpp.Windowed = TRUE;

	HWND hwndFocusWnd = m_hwndDeviceWindow; 
	if( bWindowed )
	{
		m_d3dpp.BackBufferCount  = 0;
		m_d3dpp.BackBufferWidth  = 0;
		m_d3dpp.BackBufferHeight = 0;
		m_d3dpp.FullScreen_RefreshRateInHz = 0;
		// m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		// Set windowed-mode style
		// SetWindowLong( m_hwndDeviceWindow, GWL_STYLE, WS_POPUP );
	}
	else
	{
		//m_d3dpp.BackBufferCount  = 1;
		//m_d3dpp.BackBufferWidth  = Mode.Width;
		//m_d3dpp.BackBufferHeight = Mode.Height;
		m_d3dpp.FullScreen_RefreshRateInHz = Mode.RefreshRate;
		m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		// hwndFocusWnd = *g_pHwndHWND; // GetTopWindow(NULL);
		OUTPUT_LOG("DEBUG: g_pHwndHWND handle : %d\n", hwndFocusWnd);
		OUTPUT_LOG("DEBUG: top level window handle : %d\n", GetTopWindow(NULL));
		OUTPUT_LOG("DEBUG: bg wnd handle: %d\n", m_hwndDeviceWindow);
		
		// Set fullscreen-mode style
		// SetWindowLong( m_hwndDeviceWindow, GWL_STYLE, WS_POPUP );
		//SetWindowLong( m_hwndDeviceWindow, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
		//SetWindowPos( m_hwndDeviceWindow, HWND_NOTOPMOST,0,0,1,1, SWP_SHOWWINDOW);
	}
	if(pFocusWnd)
		*pFocusWnd = hwndFocusWnd;
}

HRESULT CDirectXEngine::Initialize3DEnvironment()
{
	HRESULT hr;
	HWND hwndFocusWnd = NULL; 
	BuildPresentParamsFromSettings(&hwndFocusWnd);

	if (FAILED(hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwndFocusWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE /*|D3DCREATE_MULTITHREADED*/, &m_d3dpp, &m_pd3dDevice)))
	{
		OUTPUT_LOG("error: failed creating d3d device object in worker thread\n");
		OUTPUT_LOG("width:%d, height:%d, windowed:%d\n", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, m_d3dpp.Windowed ? 1:0);
		return E_FAIL;
	}

	// Initialize the app's device-dependent objects
	InitDeviceObjects(m_pd3dDevice);
	m_bDeviceObjectsInited = true;
	RestoreDeviceObjects();
	m_bDeviceObjectsRestored = true;

	return S_OK;
}

void CDirectXEngine::Cleanup3DEnvironment()
{
	m_bDeviceLost = false;
	if( m_pd3dDevice != NULL )
	{
		if( m_bDeviceObjectsRestored )
		{
			m_bDeviceObjectsRestored = false;
			InvalidateDeviceObjects();
		}
		if( m_bDeviceObjectsInited )
		{
			m_bDeviceObjectsInited = false;
			DeleteDeviceObjects();
		}

		if( m_pd3dDevice->Release() > 0 )
			OUTPUT_LOG( "error: d3d device in worker thread exit with none-zero reference. \n" );
		m_pd3dDevice = NULL;
	}
}

HRESULT CDirectXEngine::Reset3DEnvironment()
{
	OUTPUT_LOG("background CDirectXEngine device is reset\n");

	HRESULT hr;

	// Release all vidmem objects
	if( m_bDeviceObjectsRestored )
	{
		m_bDeviceObjectsRestored = false;
		InvalidateDeviceObjects();
	}
	// Reset the device
	if( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
	{
		OUTPUT_DEFAULT_ERROR(hr);
		OUTPUT_LOG("error: failed to reset d3d device. %d\n", hr);
		return hr;
	}

	// Initialize the app's device-dependent objects
	RestoreDeviceObjects();
	
	m_bDeviceObjectsRestored = true;
	return S_OK;
}

bool CDirectXEngine::Begin2D()
{
	if(!BeginScene() || m_pd3dDevice == 0)
		return false;
	HRESULT hr = m_pd3dDevice->SetRenderTarget(0, m_pRenderTargetSurface);
	if(FAILED(hr))
	{
		return false;
	}
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
	m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,      FALSE);

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetFVF( DXUT_SCREEN_VERTEX::FVF );

	m_pd3dDevice->SetVertexShader( NULL );
	m_pd3dDevice->SetPixelShader( NULL );
	return true;
}

void CDirectXEngine::End2D()
{
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	EndScene();
}

bool CDirectXEngine::PresentScene()
{
	HRESULT hr = S_OK;
	if( m_bDeviceLost )
	{
		//OUTPUT_LOG("DEBUG: try to recover from a background device lost.\n");
		//// Test the cooperative level to see if it's okay to render
		//if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
		//{
		//	// If the device was lost, do not render until we get it back
		//	if( D3DERR_DEVICELOST == hr )
		//	{
		//		OUTPUT_LOG("DEBUG: is still lost\n");
		//		return false;
		//	}

		//	// Check if the device needs to be reset.
		//	if( D3DERR_DEVICENOTRESET == hr )
		//	{
		//		if( FAILED( hr = Reset3DEnvironment() ) )
		//		{
		//			OUTPUT_LOG("error: can not reset 3d environment\n");
		//			return false;
		//		}
		//	}
		//}
	}

	if( ! m_bDeviceLost )
	{
		if(m_pd3dDevice)
		{
			hr = m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
			if( D3DERR_DEVICELOST == hr )
			{
				m_bDeviceLost = true;

				OUTPUT_LOG("warning: background device is lost during present. try to recover it.\n");
				
				//Cleanup3DEnvironment();
				//Initialize3DEnvironment();

				BuildPresentParamsFromSettings();
				Reset3DEnvironment();
				return false;
			}
		}
	}
	return true;
}

void CDirectXEngine::InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{

}

void CDirectXEngine::RestoreDeviceObjects()
{
	if(m_pd3dDevice == 0)
		return;
	DWORD width = 256;
	DWORD height = 256;
	// Default: create the render target with alpha. this will allow some mini scene graphs to render alpha pixels. 
	D3DFORMAT format = D3DFMT_A8R8G8B8;

	if( FAILED(D3DXCreateTexture(m_pd3dDevice,width,height,1,D3DUSAGE_RENDERTARGET,
		format, D3DPOOL_DEFAULT,&m_pRenderTargetTexture)) )
	{
		if(format == D3DFMT_A8R8G8B8)
		{
			// try D3DFMT_X8R8G8B8 if FMT_A8R8G8B8 is not supported; perhaps D3DXCreateTexture already secretly does this for us. 
			if( FAILED(D3DXCreateTexture(m_pd3dDevice,width,height,1,D3DUSAGE_RENDERTARGET,
				D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,&m_pRenderTargetTexture)) )
			{
				OUTPUT_LOG("error: failed creating main render target \n");
				m_pRenderTargetTexture = NULL;
			}
		}
		else
		{
			OUTPUT_LOG("error: failed creating main render target \n");
			m_pRenderTargetTexture = NULL;
		}
	}
	if(m_pRenderTargetTexture)
	{
		m_pRenderTargetTexture->GetSurfaceLevel(0, &m_pRenderTargetSurface);
	}

	// begin by default
	Begin2D();
}

bool CDirectXEngine::BeginScene()
{
	if(!m_bIsSceneBegin)
	{
		if( m_pd3dDevice && FAILED( m_pd3dDevice->BeginScene() ) )
		{
			OUTPUT_LOG("error: failed to begin scene in dx worker engine.\n");
		}
		else
			m_bIsSceneBegin = true;
	}
	return m_bIsSceneBegin;
}

void CDirectXEngine::EndScene()
{
	if(m_pd3dDevice && m_bIsSceneBegin)
	{
		m_pd3dDevice->EndScene();
	}
}

void CDirectXEngine::InvalidateDeviceObjects()
{
	End2D();

	SAFE_RELEASE(m_pRenderTargetSurface);
	SAFE_RELEASE(m_pRenderTargetTexture);
}

void CDirectXEngine::DeleteDeviceObjects()
{

}

HRESULT CDirectXEngine::Destroy()
{
	Cleanup3DEnvironment();
	SAFE_RELEASE(m_pD3D);

	if (m_hwndDeviceWindow)
	{
		DestroyWindow(m_hwndDeviceWindow);
		m_hwndDeviceWindow = NULL;

		OUTPUT_LOG("Background render device is successfully destroyed\n");
	}

	return S_OK;
}


LPDIRECT3D9 CDirectXEngine::GetD3DObject()
{
	return m_pD3D;
}

LPDIRECT3DDEVICE9  CDirectXEngine::GetD3DDevice()
{
	return m_pd3dDevice;
}

// NOT used
bool CALLBACK CDirectXEngine::IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed )
{
	// No fallback defined by this app, so reject any device that 
	// doesn't support at least ps1.1
	//if( pCaps->PixelShaderVersion < D3DPS_VERSION(1,1) )
	//	return false;
	return true;
}


HRESULT CDirectXEngine::Clear(const Color &color)
{
	if(m_pd3dDevice == 0)
		return E_FAIL;
	LPDIRECT3DDEVICE9 pd3dDevice=m_pd3dDevice;
	return pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,color,1.0f,0);
}

void CDirectXEngine::SetTransform(const Matrix4 *matTransform)
{
	if (matTransform==NULL) {
		return;
	}
	m_transform=*matTransform;
}

void CDirectXEngine::GetTransform(Matrix4 *matTransform)
{
	if (matTransform==NULL) {
		return;
	}
	*matTransform=m_transform;
}

HRESULT CDirectXEngine::DrawTriangleStrip(LPDIRECT3DTEXTURE9 pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertices)
{
	if (pVertices==NULL || m_pd3dDevice ==0) 
		return S_OK;
	
	LPDIRECT3DDEVICE9 pd3dDevice=m_pd3dDevice;

	HRESULT hr;
	hr=pd3dDevice->SetTexture(0,pTexture);

	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	hr=pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, nNumVertices-2,pVertices, sizeof(DXUT_SCREEN_VERTEX));
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}

	return hr;
}

HRESULT CDirectXEngine::DrawTriangleList(LPDIRECT3DTEXTURE9 pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertices)
{
	if (pVertices==NULL || m_pd3dDevice ==0) 
		return S_OK;
	LPDIRECT3DDEVICE9 pd3dDevice=m_pd3dDevice;

	HRESULT hr;
	hr=pd3dDevice->SetTexture(0,pTexture);

	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	hr=pd3dDevice->SetTransform(D3DTS_TEXTURE0,m_transform.GetPointer());
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}

	hr=pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, nNumVertices/3,pVertices, sizeof(DXUT_SCREEN_VERTEX));
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	return hr;
}

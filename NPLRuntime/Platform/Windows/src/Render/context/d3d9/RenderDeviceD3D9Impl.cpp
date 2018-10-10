#include "ParaEngine.h"
#include "RenderDeviceD3D9Impl.h"
#include "RenderWindowWin32.h"
#include "TextureD3D9.h"
using namespace ParaEngine;



IRenderDevice* IRenderDevice::Create(const RenderConfiguration& cfg)
{

	RenderWindowWin32* pWin = dynamic_cast<RenderWindowWin32*>(cfg.renderWindow);
	auto pContext = Direct3DCreate9(D3D_SDK_VERSION);
	D3DDISPLAYMODE defaultAdapterDesplayMode;
	HRESULT hr = pContext->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &defaultAdapterDesplayMode);
	if (hr != D3D_OK)
	{
		OUTPUT_LOG("GetDefaultAdapterDisplayMode failed.");
		return nullptr;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.hDeviceWindow = pWin->GetHandle();
	if (cfg.isWindowed)
	{
		d3dpp.Windowed = true;
		d3dpp.BackBufferWidth = pWin->GetWidth();
		d3dpp.BackBufferHeight = pWin->GetHeight();
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}
	else {
		d3dpp.Windowed = false;
		d3dpp.BackBufferWidth = defaultAdapterDesplayMode.Width;
		d3dpp.BackBufferHeight = defaultAdapterDesplayMode.Height;
		d3dpp.BackBufferFormat = defaultAdapterDesplayMode.Format;
		d3dpp.FullScreen_RefreshRateInHz = defaultAdapterDesplayMode.RefreshRate;	
	}

	D3DCAPS9 caps; int vp = 0;
	if (pContext->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps) != D3D_OK)
	{
		OUTPUT_LOG("GetDeviceCaps failed.");
		return nullptr;
	}
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else {
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	IDirect3DDevice9* pD3DDevice = nullptr;
	hr = pContext->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pWin->GetHandle(), vp | D3DCREATE_FPU_PRESERVE, &d3dpp, &pD3DDevice);

	if (hr != D3D_OK) return nullptr;

	RenderDeviceD3D9Impl* pDevice = new RenderDeviceD3D9Impl();
	pDevice->m_pD3DDevice = pD3DDevice;
	pDevice->m_pContext = pContext;

	if (!pDevice->Initialize())
	{
		delete pDevice;
		return nullptr;
	}
	return pDevice;
}


bool RenderDeviceD3D9Impl::Reset(const RenderConfiguration& cfg)
{


	RenderWindowWin32* pWin = dynamic_cast<RenderWindowWin32*>(cfg.renderWindow);
	auto pContext = this->m_pContext;
	D3DDISPLAYMODE defaultAdapterDesplayMode;
	HRESULT hr = pContext->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &defaultAdapterDesplayMode);
	if (hr != D3D_OK)
	{
		OUTPUT_LOG("GetDefaultAdapterDisplayMode failed.");
		return nullptr;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.hDeviceWindow = pWin->GetHandle();
	if (cfg.isWindowed)
	{
		d3dpp.Windowed = true;
		d3dpp.BackBufferWidth = pWin->GetWidth();
		d3dpp.BackBufferHeight = pWin->GetHeight();
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}
	else {
		d3dpp.Windowed = false;
		d3dpp.BackBufferWidth = defaultAdapterDesplayMode.Width;
		d3dpp.BackBufferHeight = defaultAdapterDesplayMode.Height;
		d3dpp.BackBufferFormat = defaultAdapterDesplayMode.Format;
		d3dpp.FullScreen_RefreshRateInHz = defaultAdapterDesplayMode.RefreshRate;
	}

	D3DCAPS9 caps; int vp = 0;
	if (pContext->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps) != D3D_OK)
	{
		OUTPUT_LOG("GetDeviceCaps failed.");
		return nullptr;
	}
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else {
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	auto it = std::find(m_Resources.begin(), m_Resources.end(), m_backbufferRenderTarget);
	m_Resources.erase(it);

	it = std::find(m_Resources.begin(), m_Resources.end(), m_backbufferDepthStencil);
	m_Resources.erase(it);


	while (m_backbufferRenderTarget->GetRef()>0)
	{
		m_backbufferRenderTarget->Release();
	}
	delete m_backbufferRenderTarget;

	while (m_backbufferDepthStencil->GetRef() > 0)
	{
		m_backbufferDepthStencil->Release();
	}
	delete m_backbufferDepthStencil;

	hr = m_pD3DDevice->Reset(&d3dpp);

	if (hr != D3D_OK) return false;


	if (!InitBackbuffers()) return false;


	return true;
}


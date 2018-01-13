#include <d3d9.h>

#include "ParaEngine.h"
#include "D3D9/D3D9Tools.hpp"
#include "D3D9/D3D9RenderDevice.h"
#include "D3D9RenderContext.h"
#include "WindowsRenderWindow.h"
#include "PEtypes.h"


using namespace ParaEngine;

namespace ParaEngine
{
	HWND toHWND(const IRenderWindow* window)
	{
		if (window == nullptr) return NULL;
		auto winRenderWindow = dynamic_cast<const WindowsRenderWindow*>(window);
		assert(winRenderWindow);
		return winRenderWindow->GetHandle();
	}
}

ParaEngine::D3D9RenderContext* ParaEngine::D3D9RenderContext::Create()
{
	auto pContext = new D3D9RenderContext();
	auto pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D)
	{
		pContext->m_D3D = pD3D;
		return pContext;
	}
	else {
		delete pContext;
		pContext = nullptr;
	}
	return nullptr;
}

ParaEngine::IRenderDevice* ParaEngine::D3D9RenderContext::CreateDevice(const RenderConfiguration& cfg)
{
	assert(cfg.renderWindow);

	WindowsRenderWindow* pWin = dynamic_cast<WindowsRenderWindow*>(cfg.renderWindow);
	assert(pWin);



	D3DDISPLAYMODE defaultAdapterDesplayMode;
	HRESULT hr = m_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &defaultAdapterDesplayMode);
	if (hr != D3D_OK)
	{
		OUTPUT_LOG("GetDefaultAdapterDisplayMode failed.");
		return nullptr;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));


	if (cfg.isWindowed)
	{
		d3dpp.Windowed = true;
		d3dpp.BackBufferCount = 1;
		d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality = 0;
		d3dpp.BackBufferWidth = pWin->GetWidth();
		d3dpp.BackBufferHeight = pWin->GetHeight();	
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = true;
		d3dpp.AutoDepthStencilFormat = toD3DFromat(cfg.depthStencilFormat);
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.BackBufferFormat = toD3DFromat(cfg.colorFormat);
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		d3dpp.hDeviceWindow = pWin->GetHandle();
	}
	else {
		d3dpp.Windowed = false;
		d3dpp.BackBufferCount = 1;
		d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality = 0;
		d3dpp.BackBufferWidth = defaultAdapterDesplayMode.Width;
		d3dpp.BackBufferHeight = defaultAdapterDesplayMode.Height;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = true;
		d3dpp.AutoDepthStencilFormat = toD3DFromat(cfg.depthStencilFormat);
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.BackBufferFormat = defaultAdapterDesplayMode.Format;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		d3dpp.FullScreen_RefreshRateInHz = defaultAdapterDesplayMode.RefreshRate;
		d3dpp.hDeviceWindow = pWin->GetHandle();
	}

	D3DCAPS9 caps; int vp = 0;
	if (m_D3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps) != D3D_OK)
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
	hr = m_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pWin->GetHandle(), vp | D3DCREATE_FPU_PRESERVE, &d3dpp, &pD3DDevice);
	if (hr == D3D_OK)
	{
		CD3D9RenderDevice* pDevice = new CD3D9RenderDevice(pD3DDevice);
		return pDevice;
	}
	else if (hr == D3DERR_INVALIDCALL)
	{
		OUTPUT_LOG("Invalid call");
	}
	else if (hr == D3DERR_NOTAVAILABLE)
	{
		OUTPUT_LOG("Not available");
	}
	return nullptr;
}

IDirect3D9* ParaEngine::D3D9RenderContext::GetD3D() const
{
	return m_D3D;
}

bool ParaEngine::D3D9RenderContext::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
{
	assert(cfg.renderWindow);

	WindowsRenderWindow* pWin = dynamic_cast<WindowsRenderWindow*>(cfg.renderWindow);
	assert(pWin);

	assert(device);

	CD3D9RenderDevice* d3d9Device = dynamic_cast<CD3D9RenderDevice*>(device);
	assert(d3d9Device);




	D3DDISPLAYMODE defaultAdapterDesplayMode;
	HRESULT hr = m_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &defaultAdapterDesplayMode);
	if (hr != D3D_OK)
	{
		OUTPUT_LOG("GetDefaultAdapterDisplayMode failed.");
		return nullptr;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));


	if (cfg.isWindowed)
	{
		d3dpp.Windowed = true;
		d3dpp.BackBufferCount = 1;
		d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality = 0;
		d3dpp.BackBufferWidth = pWin->GetWidth();
		d3dpp.BackBufferHeight = pWin->GetHeight();
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = true;
		d3dpp.AutoDepthStencilFormat = toD3DFromat(cfg.depthStencilFormat);
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.BackBufferFormat = toD3DFromat(cfg.colorFormat);
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		d3dpp.hDeviceWindow = pWin->GetHandle();
	}
	else {
		d3dpp.Windowed = false;
		d3dpp.BackBufferCount = 1;
		d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality = 0;
		d3dpp.BackBufferWidth = defaultAdapterDesplayMode.Width;
		d3dpp.BackBufferHeight = defaultAdapterDesplayMode.Height;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.EnableAutoDepthStencil = true;
		d3dpp.AutoDepthStencilFormat = toD3DFromat(cfg.depthStencilFormat);
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		d3dpp.BackBufferFormat = defaultAdapterDesplayMode.Format;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		d3dpp.FullScreen_RefreshRateInHz = defaultAdapterDesplayMode.RefreshRate;
		d3dpp.hDeviceWindow = pWin->GetHandle();
	}

	hr = d3d9Device->GetDirect3DDevice9()->Reset(&d3dpp);

	if (hr != D3D_OK)
	{
		return false;
	}

	return true;
}

D3D9RenderContext::~D3D9RenderContext()
{
	if (m_D3D)
	{
		m_D3D->Release();
		m_D3D = nullptr;
	}
}

D3D9RenderContext::D3D9RenderContext()
	:m_D3D(nullptr)
{
	m_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	assert(m_D3D);
	if (m_D3D)
	{
		OUTPUT_LOG("Init Direct3D failed. \n");
	}
}


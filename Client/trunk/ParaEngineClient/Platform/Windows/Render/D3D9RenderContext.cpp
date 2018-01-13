#include "ParaEngine.h"
#include "D3D9/D3D9RenderDevice.h"
#include "D3D9RenderContext.h"
#include "WindowsRenderWindow.h"
#include "PEtypes.h"
#include <d3d9.h>

using namespace ParaEngine;


namespace ParaEngine
{



	D3DFORMAT toD3DFromat(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::Unkonwn:
			return D3DFMT_UNKNOWN;
		case PixelFormat::R8G8B8:
			return D3DFMT_R8G8B8;
		case PixelFormat::A8R8G8B8:
			return D3DFMT_A8R8G8B8;
		case PixelFormat::X8R8G8B8:
			return D3DFMT_X8R8G8B8;
		case PixelFormat::R5G6B5:
			return D3DFMT_R5G6B5;
		case PixelFormat::X1R5G5B5:
			return D3DFMT_X1R5G5B5;
		case PixelFormat::A1R5G5B5:
			return D3DFMT_A1R5G5B5;
		case PixelFormat::A4R4G4B4:
			return D3DFMT_A4R4G4B4;
		case PixelFormat::R3G3B2:
			return D3DFMT_R3G3B2;
		case PixelFormat::A8:
			return D3DFMT_A8;
		case PixelFormat::A8R3G3B2:
			return D3DFMT_A8R3G3B2;
		case PixelFormat::X4R4G4B4:
			return D3DFMT_X4R4G4B4;
		case PixelFormat::A2B10G10R10:
			return D3DFMT_A2B10G10R10;
		case PixelFormat::A8B8G8R8:
			return D3DFMT_A8B8G8R8;
		case PixelFormat::X8B8G8R8:
			return D3DFMT_X8B8G8R8;
		case PixelFormat::G16R16:
			return D3DFMT_G16R16;
		case PixelFormat::A2R10G10B10:
			return D3DFMT_A2R10G10B10;
		case PixelFormat::A16B16G16R16:
			return D3DFMT_A16B16G16R16;
		case PixelFormat::A8P8:
			return D3DFMT_A8P8;
		case PixelFormat::P8:
			return D3DFMT_P8;
		case PixelFormat::L8:
			return D3DFMT_L8;
		case PixelFormat::A8L8:
			return D3DFMT_A8L8;
		case PixelFormat::A4L4:
			return D3DFMT_A4L4;
		case PixelFormat::V8U8:
			return D3DFMT_V8U8;
		case PixelFormat::L6V5U5:
			return D3DFMT_L6V5U5;
		case PixelFormat::X8L8V8U8:
			return D3DFMT_X8L8V8U8;
		case PixelFormat::Q8W8V8U8:
			return D3DFMT_Q8W8V8U8;
		case PixelFormat::V16U16:
			return D3DFMT_V16U16;
		case PixelFormat::A2W10V10U10:
			return D3DFMT_A2W10V10U10;
		case PixelFormat::UYVY:
			return D3DFMT_UYVY;
		case PixelFormat::R8G8_B8G8:
			return D3DFMT_R8G8_B8G8;
		case PixelFormat::YUY2:
			return D3DFMT_YUY2;
		case PixelFormat::G8R8_G8B8:
			return D3DFMT_G8R8_G8B8;
		case PixelFormat::DXT1:
			return D3DFMT_DXT1;
		case PixelFormat::DXT2:
			return D3DFMT_DXT2;
		case PixelFormat::DXT3:
			return D3DFMT_DXT3;
		case PixelFormat::DXT4:
			return D3DFMT_DXT4;
		case PixelFormat::DXT5:
			return D3DFMT_DXT5;
		case PixelFormat::D16_LOCKABLE:
			return D3DFMT_D16_LOCKABLE;
		case PixelFormat::D32:
			return D3DFMT_D32;
		case PixelFormat::D15S1:
			return D3DFMT_D15S1;
		case PixelFormat::D24S8:
			return D3DFMT_D24S8;
		case PixelFormat::D24X8:
			return D3DFMT_D24X8;
		case PixelFormat::D24X4S4:
			return D3DFMT_D24X4S4;
		case PixelFormat::D16:
			return D3DFMT_D16;
		case PixelFormat::D32F_LOCKABLE:
			return D3DFMT_D32F_LOCKABLE;
		case PixelFormat::D24FS8:
			return D3DFMT_D24FS8;
		default:
			return D3DFMT_UNKNOWN;
		}
	}


	PixelFormat toPixelFormat(D3DFORMAT format)
	{
		static std::unordered_map<D3DFORMAT, PixelFormat > enum_map;
		if (enum_map.empty())
		{
			int count = (int)PixelFormat::COUNT;
			for (int i = 0; i < count; i++)
			{
				PixelFormat pixelFormat = (PixelFormat)(i);
				enum_map[toD3DFromat(pixelFormat)] = pixelFormat;
			}
		}

		auto it = enum_map.find(format);
		if (it != enum_map.end())
		{
			return it->second;
		}
		return PixelFormat::Unkonwn;
	}

	D3DMULTISAMPLE_TYPE toD3DMSAAType(MultiSampleType msaa)
	{
		switch (msaa)
		{
		case MultiSampleType::None:
			return D3DMULTISAMPLE_NONE;
			break;
		case MultiSampleType::NONMASKABLE:
			return D3DMULTISAMPLE_NONMASKABLE;
			break;
		case MultiSampleType::Samples_2:
			return D3DMULTISAMPLE_2_SAMPLES;
			break;
		case MultiSampleType::Samples_3:
			return D3DMULTISAMPLE_3_SAMPLES;
			break;
		case MultiSampleType::Samples_4:
			return D3DMULTISAMPLE_4_SAMPLES;
			break;
		case MultiSampleType::Samples_5:
			return D3DMULTISAMPLE_5_SAMPLES;
			break;
		case MultiSampleType::Samples_6:
			return D3DMULTISAMPLE_6_SAMPLES;
			break;
		case MultiSampleType::Samples_7:
			return D3DMULTISAMPLE_7_SAMPLES;
			break;
		case MultiSampleType::Samples_8:
			return D3DMULTISAMPLE_8_SAMPLES;
			break;
		case MultiSampleType::Samples_9:
			return D3DMULTISAMPLE_9_SAMPLES;
			break;
		case MultiSampleType::Samples_10:
			return D3DMULTISAMPLE_10_SAMPLES;
			break;
		case MultiSampleType::Samples_11:
			return D3DMULTISAMPLE_11_SAMPLES;
			break;
		case MultiSampleType::Samples_12:
			return D3DMULTISAMPLE_12_SAMPLES;
			break;
		case MultiSampleType::Samples_13:
			return D3DMULTISAMPLE_13_SAMPLES;
			break;
		case MultiSampleType::Samples_14:
			return D3DMULTISAMPLE_14_SAMPLES;
			break;
		case MultiSampleType::Samples_15:
			return D3DMULTISAMPLE_15_SAMPLES;
			break;
		case MultiSampleType::Samples_16:
			return D3DMULTISAMPLE_16_SAMPLES;
			break;
		default:
			assert(false);
			return D3DMULTISAMPLE_NONE;
			break;
		}
	}



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

ParaEngine::IRenderDevice* ParaEngine::D3D9RenderContext::CreateDevice(const RenderDeviceConfiguration& cfg)
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

bool ParaEngine::D3D9RenderContext::ResetDevice(IRenderDevice* device, const RenderDeviceConfiguration& cfg)
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


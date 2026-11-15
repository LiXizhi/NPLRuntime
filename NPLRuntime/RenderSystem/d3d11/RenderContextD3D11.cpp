#include "ParaEngine.h"
#include "RenderContextD3D11.h"
#include "RenderDeviceD3D11.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <cstring>
#include <string>

using Microsoft::WRL::ComPtr;

namespace ParaEngine
{
	namespace
	{
		bool HasCommandLineFlag(const char* flag)
		{
			const char* cmd = ::GetCommandLineA();
			if (!cmd || !flag)
				return false;
			return std::strstr(cmd, flag) != nullptr;
		}

		std::string WideToUtf8(const wchar_t* wstr)
		{
			if (!wstr || !wstr[0])
				return "Direct3D 11 Adapter";
			int len = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
			if (len <= 1)
				return "Direct3D 11 Adapter";
			std::string out(static_cast<size_t>(len - 1), '\0');
			::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &out[0], len, nullptr, nullptr);
			return out;
		}

		struct D3D11RuntimeLoader
		{
			HMODULE hD3D11 = nullptr;
			PFN_D3D11_CREATE_DEVICE pfnCreateDevice = nullptr;

			D3D11RuntimeLoader()
			{
				hD3D11 = ::LoadLibraryW(L"d3d11.dll");
				if (hD3D11)
				{
					pfnCreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(::GetProcAddress(hD3D11, "D3D11CreateDevice"));
				}
			}
		};

		const D3D11RuntimeLoader& GetD3D11Runtime()
		{
			static D3D11RuntimeLoader s_loader;
			return s_loader;
		}
	}

	IRenderContext* CreateRenderContextD3D11()
	{
		return new RenderContextD3D11();
	}

	IRenderDevice* RenderContextD3D11::CreateDevice(const RenderConfiguration& cfg)
	{
		const auto& rt = GetD3D11Runtime();
		if (!rt.pfnCreateDevice)
		{
			OUTPUT_LOG("ParaRHI: d3d11.dll not available on this system; skipping D3D11 backend\n");
			return nullptr;
		}

		if (!cfg.renderWindow)
		{
			OUTPUT_LOG("ERROR: RenderContextD3D11::CreateDevice called with null renderWindow\n");
			return nullptr;
		}

		HWND hWnd = reinterpret_cast<HWND>(cfg.renderWindow->GetNativeHandle());
		if (!hWnd)
		{
			OUTPUT_LOG("ERROR: RenderContextD3D11::CreateDevice got null HWND\n");
			return nullptr;
		}

		const bool forceWarp = HasCommandLineFlag("--d3d11-warp") || HasCommandLineFlag("-d3d11-warp");
		const bool forceFL93 = HasCommandLineFlag("--d3d11-fl93") || HasCommandLineFlag("-d3d11-fl93");

		UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		if (cfg.debugValidation || HasCommandLineFlag("--rhi-validation"))
		{
			createFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}

		const D3D_FEATURE_LEVEL allLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3
		};
		const D3D_FEATURE_LEVEL fl93Only[] = {
			D3D_FEATURE_LEVEL_9_3
		};

		const D3D_FEATURE_LEVEL* pLevels = forceFL93 ? fl93Only : allLevels;
		UINT numLevels = forceFL93 ? 1u : static_cast<UINT>(sizeof(allLevels) / sizeof(allLevels[0]));

		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		D3D_FEATURE_LEVEL achievedFL = D3D_FEATURE_LEVEL_11_0;
		bool usingWarp = false;

		auto tryCreate = [&](D3D_DRIVER_TYPE driverType, UINT flags) -> HRESULT {
			HRESULT hr = rt.pfnCreateDevice(
				nullptr,
				driverType,
				nullptr,
				flags,
				pLevels,
				numLevels,
				D3D11_SDK_VERSION,
				device.ReleaseAndGetAddressOf(),
				&achievedFL,
				context.ReleaseAndGetAddressOf());

			// Older Windows 7 systems without KB2670838 reject D3D_FEATURE_LEVEL_11_1 with E_INVALIDARG
			if (hr == E_INVALIDARG && !forceFL93 && numLevels > 1)
			{
				hr = rt.pfnCreateDevice(
					nullptr,
					driverType,
					nullptr,
					flags,
					allLevels + 1,
					numLevels - 1,
					D3D11_SDK_VERSION,
					device.ReleaseAndGetAddressOf(),
					&achievedFL,
					context.ReleaseAndGetAddressOf());
			}
			return hr;
		};

		HRESULT hr = E_FAIL;
		if (!forceWarp)
		{
			hr = tryCreate(D3D_DRIVER_TYPE_HARDWARE, createFlags);
			if (FAILED(hr) && (createFlags & D3D11_CREATE_DEVICE_DEBUG) != 0)
			{
				createFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
				hr = tryCreate(D3D_DRIVER_TYPE_HARDWARE, createFlags);
			}
		}

		if (FAILED(hr) && (forceWarp || cfg.allowSoftwareFallback))
		{
			hr = tryCreate(D3D_DRIVER_TYPE_WARP, createFlags);
			if (FAILED(hr) && (createFlags & D3D11_CREATE_DEVICE_DEBUG) != 0)
			{
				createFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
				hr = tryCreate(D3D_DRIVER_TYPE_WARP, createFlags);
			}
			if (SUCCEEDED(hr))
			{
				usingWarp = true;
			}
		}

		if (FAILED(hr) || !device || !context)
		{
			OUTPUT_LOG("ERROR: D3D11CreateDevice failed (hr=0x%08X)\n", static_cast<unsigned int>(hr));
			return nullptr;
		}

		ComPtr<IDXGIDevice> dxgiDevice;
		hr = device.As(&dxgiDevice);
		if (FAILED(hr))
			return nullptr;

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
		if (FAILED(hr))
			return nullptr;

		std::string adapterName = usingWarp ? "Microsoft Basic Render Driver (WARP)" : "Direct3D 11 Adapter";
		DXGI_ADAPTER_DESC adapterDesc = {};
		if (SUCCEEDED(dxgiAdapter->GetDesc(&adapterDesc)))
		{
			adapterName = WideToUtf8(adapterDesc.Description);
			if (usingWarp && adapterName.find("WARP") == std::string::npos)
				adapterName += " (WARP)";
		}

		ComPtr<IDXGIFactory> dxgiFactory;
		hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));
		if (FAILED(hr) || !dxgiFactory)
			return nullptr;

		uint32_t width = cfg.screenWidth > 0 ? cfg.screenWidth : cfg.renderWindow->GetWidth();
		uint32_t height = cfg.screenHeight > 0 ? cfg.screenHeight : cfg.renderWindow->GetHeight();
		if (width == 0) width = 1024;
		if (height == 0) height = 768;

		UINT sampleCount = 1;
		UINT sampleQuality = 0;
		if (cfg.msaaSamples > 1)
		{
			UINT qualityLevels = 0;
			if (SUCCEEDED(device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, cfg.msaaSamples, &qualityLevels)) && qualityLevels > 0)
			{
				sampleCount = cfg.msaaSamples;
				sampleQuality = qualityLevels - 1;
			}
		}

		DXGI_SWAP_CHAIN_DESC scDesc = {};
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Width = width;
		scDesc.BufferDesc.Height = height;
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Numerator = 60;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		scDesc.OutputWindow = hWnd;
		scDesc.SampleDesc.Count = sampleCount;
		scDesc.SampleDesc.Quality = sampleQuality;
		scDesc.Windowed = cfg.isWindowed ? TRUE : FALSE;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ComPtr<IDXGISwapChain> swapChain;
		hr = dxgiFactory->CreateSwapChain(device.Get(), &scDesc, swapChain.GetAddressOf());
		if (FAILED(hr))
		{
			// Retry without SHADER_INPUT on strict drivers / FL 9_3
			scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			hr = dxgiFactory->CreateSwapChain(device.Get(), &scDesc, swapChain.GetAddressOf());
		}
		if (FAILED(hr) || !swapChain)
		{
			OUTPUT_LOG("ERROR: IDXGIFactory::CreateSwapChain failed (hr=0x%08X)\n", static_cast<unsigned int>(hr));
			return nullptr;
		}

		dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		auto* rhiDevice = new RenderDeviceD3D11(device.Get(), context.Get(), swapChain.Get(), achievedFL, usingWarp, adapterName);
		if (!rhiDevice->ResizeSwapChain(width, height, cfg.isWindowed))
		{
			delete rhiDevice;
			return nullptr;
		}

		OUTPUT_LOG("ParaRHI: Initialized D3D11 backend (FeatureLevel=0x%04X, Adapter='%s', %ux%u)\n",
			static_cast<unsigned int>(achievedFL), adapterName.c_str(), width, height);

		return rhiDevice;
	}

	bool RenderContextD3D11::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
	{
		if (!device || device->GetBackendType() != ERHIBackend::D3D11)
			return false;
		uint32_t w = cfg.screenWidth > 0 ? cfg.screenWidth : 1u;
		uint32_t h = cfg.screenHeight > 0 ? cfg.screenHeight : 1u;
		return static_cast<RenderDeviceD3D11*>(device)->ResizeSwapChain(w, h, cfg.isWindowed);
	}
}

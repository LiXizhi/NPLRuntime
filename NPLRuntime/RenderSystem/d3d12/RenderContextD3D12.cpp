#include "ParaEngine.h"
#include "RenderContextD3D12.h"
#include "RenderDeviceD3D12.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
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
				return "Direct3D 12 Adapter";
			int len = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
			if (len <= 1)
				return "Direct3D 12 Adapter";
			std::string out(static_cast<size_t>(len - 1), '\0');
			::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &out[0], len, nullptr, nullptr);
			return out;
		}

		using PFN_CreateDXGIFactory2 = HRESULT(WINAPI*)(UINT Flags, REFIID riid, void** ppFactory);
		using PFN_CreateDXGIFactory1 = HRESULT(WINAPI*)(REFIID riid, void** ppFactory);

		struct D3D12RuntimeLoader
		{
			HMODULE hD3D12 = nullptr;
			HMODULE hDXGI = nullptr;
			PFN_D3D12_CREATE_DEVICE pfnCreateDevice = nullptr;
			PFN_D3D12_GET_DEBUG_INTERFACE pfnGetDebugInterface = nullptr;
			PFN_CreateDXGIFactory2 pfnCreateDXGIFactory2 = nullptr;
			PFN_CreateDXGIFactory1 pfnCreateDXGIFactory1 = nullptr;

			D3D12RuntimeLoader()
			{
				hD3D12 = ::LoadLibraryW(L"d3d12.dll");
				if (hD3D12)
				{
					pfnCreateDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(::GetProcAddress(hD3D12, "D3D12CreateDevice"));
					pfnGetDebugInterface = reinterpret_cast<PFN_D3D12_GET_DEBUG_INTERFACE>(::GetProcAddress(hD3D12, "D3D12GetDebugInterface"));
				}
				hDXGI = ::LoadLibraryW(L"dxgi.dll");
				if (hDXGI)
				{
					pfnCreateDXGIFactory2 = reinterpret_cast<PFN_CreateDXGIFactory2>(::GetProcAddress(hDXGI, "CreateDXGIFactory2"));
					pfnCreateDXGIFactory1 = reinterpret_cast<PFN_CreateDXGIFactory1>(::GetProcAddress(hDXGI, "CreateDXGIFactory1"));
				}
			}
		};

		const D3D12RuntimeLoader& GetD3D12Runtime()
		{
			static D3D12RuntimeLoader s_loader;
			return s_loader;
		}
	}

	IRenderContext* CreateRenderContextD3D12()
	{
		return new RenderContextD3D12();
	}

	IRenderDevice* RenderContextD3D12::CreateDevice(const RenderConfiguration& cfg)
	{
		const auto& rt = GetD3D12Runtime();
		if (!rt.pfnCreateDevice || (!rt.pfnCreateDXGIFactory2 && !rt.pfnCreateDXGIFactory1))
		{
			OUTPUT_LOG("ParaRHI: d3d12.dll or DXGI 1.4 not available on this Windows version; skipping D3D12 backend\n");
			return nullptr;
		}

		if (!cfg.renderWindow)
		{
			OUTPUT_LOG("ERROR: RenderContextD3D12::CreateDevice called with null renderWindow\n");
			return nullptr;
		}

		HWND hWnd = reinterpret_cast<HWND>(cfg.renderWindow->GetNativeHandle());
		if (!hWnd)
		{
			OUTPUT_LOG("ERROR: RenderContextD3D12::CreateDevice got null HWND\n");
			return nullptr;
		}

		const bool forceWarp = HasCommandLineFlag("--d3d12-warp") || HasCommandLineFlag("-d3d12-warp");
		const bool wantValidation = cfg.debugValidation || HasCommandLineFlag("--rhi-validation");

		UINT dxgiFactoryFlags = 0;
		if (wantValidation && rt.pfnGetDebugInterface)
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(rt.pfnGetDebugInterface(IID_PPV_ARGS(&debugController))) && debugController)
			{
				debugController->EnableDebugLayer();
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}

		ComPtr<IDXGIFactory4> factory;
		HRESULT hr = E_NOINTERFACE;
		if (rt.pfnCreateDXGIFactory2)
		{
			hr = rt.pfnCreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
			if (FAILED(hr) && dxgiFactoryFlags != 0)
			{
				hr = rt.pfnCreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
			}
		}
		else if (rt.pfnCreateDXGIFactory1)
		{
			hr = rt.pfnCreateDXGIFactory1(IID_PPV_ARGS(&factory));
		}
		if (FAILED(hr) || !factory)
		{
			OUTPUT_LOG("ERROR: CreateDXGIFactory2 failed for D3D12 (hr=0x%08X)\n", static_cast<unsigned int>(hr));
			return nullptr;
		}

		const D3D_FEATURE_LEVEL candidateLevels[] = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		ComPtr<ID3D12Device> device;
		D3D_FEATURE_LEVEL achievedFL = D3D_FEATURE_LEVEL_11_0;
		std::string adapterName = "Direct3D 12 Adapter";
		bool usingWarp = false;

		auto tryCreateOnAdapter = [&](IDXGIAdapter1* adapter) -> bool {
			for (D3D_FEATURE_LEVEL fl : candidateLevels)
			{
				ComPtr<ID3D12Device> dev;
				if (SUCCEEDED(rt.pfnCreateDevice(adapter, fl, IID_PPV_ARGS(&dev))) && dev)
				{
					device = dev;
					achievedFL = fl;
					return true;
				}
			}
			return false;
		};

		if (!forceWarp)
		{
			ComPtr<IDXGIAdapter1> bestAdapter;
			SIZE_T bestDedicatedVram = 0;

			for (UINT idx = 0;; ++idx)
			{
				ComPtr<IDXGIAdapter1> cand;
				if (factory->EnumAdapters1(idx, &cand) == DXGI_ERROR_NOT_FOUND)
					break;
				if (!cand)
					continue;

				DXGI_ADAPTER_DESC1 desc = {};
				if (FAILED(cand->GetDesc1(&desc)))
					continue;
				if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
					continue;

				if (SUCCEEDED(rt.pfnCreateDevice(cand.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					if (!bestAdapter || desc.DedicatedVideoMemory > bestDedicatedVram)
					{
						bestAdapter = cand;
						bestDedicatedVram = desc.DedicatedVideoMemory;
					}
				}
			}

			if (bestAdapter && tryCreateOnAdapter(bestAdapter.Get()))
			{
				DXGI_ADAPTER_DESC1 desc = {};
				if (SUCCEEDED(bestAdapter->GetDesc1(&desc)))
				{
					adapterName = WideToUtf8(desc.Description);
				}
			}
		}

		if (!device && (forceWarp || cfg.allowSoftwareFallback))
		{
			ComPtr<IDXGIAdapter1> warpAdapter;
			if (SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))) && warpAdapter)
			{
				if (tryCreateOnAdapter(warpAdapter.Get()))
				{
					usingWarp = true;
					DXGI_ADAPTER_DESC1 desc = {};
					if (SUCCEEDED(warpAdapter->GetDesc1(&desc)))
					{
						adapterName = WideToUtf8(desc.Description);
						if (adapterName.find("WARP") == std::string::npos)
							adapterName += " (WARP)";
					}
					else
					{
						adapterName = "Microsoft Basic Render Driver (D3D12 WARP)";
					}
				}
			}
		}

		if (!device)
		{
			OUTPUT_LOG("ERROR: D3D12CreateDevice failed on all adapters\n");
			return nullptr;
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;

		ComPtr<ID3D12CommandQueue> cmdQueue;
		hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
		if (FAILED(hr) || !cmdQueue)
		{
			OUTPUT_LOG("ERROR: CreateCommandQueue failed for D3D12 (hr=0x%08X)\n", static_cast<unsigned int>(hr));
			return nullptr;
		}

		uint32_t width = cfg.screenWidth > 0 ? cfg.screenWidth : cfg.renderWindow->GetWidth();
		uint32_t height = cfg.screenHeight > 0 ? cfg.screenHeight : cfg.renderWindow->GetHeight();
		if (width == 0) width = 1024;
		if (height == 0) height = 768;

		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Width = width;
		scDesc.Height = height;
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.Stereo = FALSE;
		scDesc.SampleDesc.Count = 1; // Flip-model swapchains require SampleCount = 1
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = RenderDeviceD3D12::FRAME_COUNT;
		scDesc.Scaling = DXGI_SCALING_STRETCH;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = {};
		fsDesc.Windowed = cfg.isWindowed ? TRUE : FALSE;

		ComPtr<IDXGISwapChain1> swapChain1;
		hr = factory->CreateSwapChainForHwnd(
			cmdQueue.Get(),
			hWnd,
			&scDesc,
			&fsDesc,
			nullptr,
			&swapChain1);
		if (FAILED(hr) || !swapChain1)
		{
			OUTPUT_LOG("ERROR: CreateSwapChainForHwnd failed for D3D12 (hr=0x%08X)\n", static_cast<unsigned int>(hr));
			return nullptr;
		}

		factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

		ComPtr<IDXGISwapChain3> swapChain3;
		hr = swapChain1.As(&swapChain3);
		if (FAILED(hr) || !swapChain3)
		{
			OUTPUT_LOG("ERROR: QueryInterface IDXGISwapChain3 failed (hr=0x%08X)\n", static_cast<unsigned int>(hr));
			return nullptr;
		}

		OUTPUT_LOG(
			"ParaRHI D3D12 initialized: Adapter='%s', FeatureLevel=0x%X, Size=%ux%u, WARP=%d\n",
			adapterName.c_str(),
			static_cast<unsigned int>(achievedFL),
			width,
			height,
			usingWarp ? 1 : 0);

		auto* renderDevice = new RenderDeviceD3D12(
			device.Get(),
			cmdQueue.Get(),
			swapChain3.Get(),
			achievedFL,
			usingWarp,
			adapterName);

		if (!renderDevice->Initialize(width, height))
		{
			OUTPUT_LOG("ERROR: RenderDeviceD3D12::Initialize failed\n");
			delete renderDevice;
			return nullptr;
		}

		return renderDevice;
	}

	bool RenderContextD3D12::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
	{
		if (!device)
			return false;
		auto* d3d12Dev = static_cast<RenderDeviceD3D12*>(device);
		uint32_t width = cfg.screenWidth > 0 ? cfg.screenWidth : (cfg.renderWindow ? cfg.renderWindow->GetWidth() : d3d12Dev->GetBackBufferWidth());
		uint32_t height = cfg.screenHeight > 0 ? cfg.screenHeight : (cfg.renderWindow ? cfg.renderWindow->GetHeight() : d3d12Dev->GetBackBufferHeight());
		return d3d12Dev->ResizeSwapChain(width, height, cfg.isWindowed);
	}
}

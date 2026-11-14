#include "ParaEngine.h"
#include "RenderContextWebGPU.h"
#include "RenderDeviceWebGPU.h"
#include "Framework/Interface/Render/IRenderWindow.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <cstring>
#include <string>
#include <vector>

namespace ParaEngine
{
	namespace
	{
		struct AdapterReqState
		{
			bool done = false;
			WGPURequestAdapterStatus status = WGPURequestAdapterStatus_Unknown;
			WGPUAdapter adapter = nullptr;
			std::string message;
		};

		struct DeviceReqState
		{
			bool done = false;
			WGPURequestDeviceStatus status = WGPURequestDeviceStatus_Unknown;
			WGPUDevice device = nullptr;
			std::string message;
		};

		void WGPU_CALL OnAdapterRequested(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata)
		{
			auto* st = static_cast<AdapterReqState*>(userdata);
			if (!st)
				return;
			st->status = status;
			st->adapter = adapter;
			if (message)
				st->message = message;
			st->done = true;
		}

		void WGPU_CALL OnDeviceRequested(WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* userdata)
		{
			auto* st = static_cast<DeviceReqState*>(userdata);
			if (!st)
				return;
			st->status = status;
			st->device = device;
			if (message)
				st->message = message;
			st->done = true;
		}

		void WGPU_CALL OnUncapturedWebGPUError(WGPUErrorType type, const char* message, void* /*userdata*/)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] Uncaptured error (type=%u): %s\n",
				static_cast<unsigned>(type),
				message ? message : "(no message)");
		}
	}

	IRenderContext* CreateRenderContextWebGPU()
	{
		return new RenderContextWebGPU();
	}

	IRenderDevice* RenderContextWebGPU::CreateDevice(const RenderConfiguration& cfg)
	{
		if (!cfg.renderWindow)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] CreateDevice called with null renderWindow\n");
			return nullptr;
		}

		WebGPUDispatch wgpu;
		if (!wgpu.Load())
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] WebGPU runtime/navigator.gpu not available; falling back to next ParaRHI backend.\n");
			return nullptr;
		}

		WGPUInstanceDescriptor instDesc = {};
		WGPUInstance instance = PARA_WGPU_CALL(wgpu, wgpuCreateInstance, &instDesc);
		if (!instance)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] wgpuCreateInstance returned null.\n");
			wgpu.Unload();
			return nullptr;
		}

		WGPUSurface surface = nullptr;
#if defined(__EMSCRIPTEN__)
		WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
		canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
		canvasDesc.selector = "#canvas";
		WGPUSurfaceDescriptor surfDesc = {};
		surfDesc.nextInChain = &canvasDesc.chain;
		surfDesc.label = "ParaEngineCanvasSurface";
		surface = PARA_WGPU_CALL(wgpu, wgpuInstanceCreateSurface, instance, &surfDesc);
#elif defined(_WIN32)
		void* hwnd = reinterpret_cast<void*>(cfg.renderWindow->GetNativeHandle());
		if (!hwnd)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] Win32 HWND is null.\n");
			PARA_WGPU_CALL(wgpu, wgpuInstanceRelease, instance);
			wgpu.Unload();
			return nullptr;
		}
		WGPUSurfaceDescriptorFromWindowsHWND hwndDesc = {};
		hwndDesc.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
		hwndDesc.hinstance = reinterpret_cast<void*>(::GetModuleHandleW(nullptr));
		hwndDesc.hwnd = hwnd;
		WGPUSurfaceDescriptor surfDesc = {};
		surfDesc.nextInChain = &hwndDesc.chain;
		surfDesc.label = "ParaEngineWin32Surface";
		surface = PARA_WGPU_CALL(wgpu, wgpuInstanceCreateSurface, instance, &surfDesc);
#endif

		if (!surface)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] Failed to create WGPUSurface.\n");
			PARA_WGPU_CALL(wgpu, wgpuInstanceRelease, instance);
			wgpu.Unload();
			return nullptr;
		}

		WGPURequestAdapterOptions adapterOpts = {};
		adapterOpts.compatibleSurface = surface;
		adapterOpts.powerPreference = WGPUPowerPreference_HighPerformance;

		AdapterReqState adapterReq;
		PARA_WGPU_CALL(wgpu, wgpuInstanceRequestAdapter, instance, &adapterOpts, &OnAdapterRequested, &adapterReq);
		for (int spin = 0; !adapterReq.done && spin < 2000; ++spin)
		{
			wgpu.ProcessEvents(instance, nullptr);
		}

		if (!adapterReq.done || adapterReq.status != WGPURequestAdapterStatus_Success || !adapterReq.adapter)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] wgpuInstanceRequestAdapter failed: %s\n", adapterReq.message.c_str());
			PARA_WGPU_CALL(wgpu, wgpuSurfaceRelease, surface);
			PARA_WGPU_CALL(wgpu, wgpuInstanceRelease, instance);
			wgpu.Unload();
			return nullptr;
		}

		WGPUAdapter adapter = adapterReq.adapter;
		bool supportsBC = false;
		std::vector<WGPUFeatureName> reqFeatures;
		if (PARA_WGPU_HAS(wgpu, wgpuAdapterHasFeature) &&
			PARA_WGPU_CALL(wgpu, wgpuAdapterHasFeature, adapter, WGPUFeatureName_TextureCompressionBC))
		{
			supportsBC = true;
			reqFeatures.push_back(WGPUFeatureName_TextureCompressionBC);
		}

		std::string adapterName = "WebGPU Adapter";
#if !defined(__EMSCRIPTEN__)
		if (PARA_WGPU_HAS(wgpu, wgpuAdapterGetProperties))
		{
			WGPUAdapterProperties props = {};
			PARA_WGPU_CALL(wgpu, wgpuAdapterGetProperties, adapter, &props);
			if (props.name && props.name[0] != '\0')
				adapterName = props.name;
		}
#else
		adapterName = "Browser WebGPU (WGSL 1.0)";
#endif

		WGPUDeviceDescriptor devDesc = {};
		devDesc.label = "ParaEngineWebGPUDevice";
		devDesc.requiredFeatureCount = reqFeatures.size();
		devDesc.requiredFeatures = reqFeatures.empty() ? nullptr : reqFeatures.data();
		devDesc.defaultQueue.label = "ParaEngineWebGPUQueue";

		DeviceReqState devReq;
		PARA_WGPU_CALL(wgpu, wgpuAdapterRequestDevice, adapter, &devDesc, &OnDeviceRequested, &devReq);
		for (int spin = 0; !devReq.done && spin < 2000; ++spin)
		{
			wgpu.ProcessEvents(instance, nullptr);
		}

		if (!devReq.done || devReq.status != WGPURequestDeviceStatus_Success || !devReq.device)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] wgpuAdapterRequestDevice failed: %s\n", devReq.message.c_str());
			PARA_WGPU_CALL(wgpu, wgpuAdapterRelease, adapter);
			PARA_WGPU_CALL(wgpu, wgpuSurfaceRelease, surface);
			PARA_WGPU_CALL(wgpu, wgpuInstanceRelease, instance);
			wgpu.Unload();
			return nullptr;
		}

		WGPUDevice device = devReq.device;
		if (PARA_WGPU_HAS(wgpu, wgpuDeviceSetUncapturedErrorCallback))
		{
			PARA_WGPU_CALL(wgpu, wgpuDeviceSetUncapturedErrorCallback, device, &OnUncapturedWebGPUError, nullptr);
		}

		WGPUQueue queue = PARA_WGPU_CALL(wgpu, wgpuDeviceGetQueue, device);
		if (!queue)
		{
			OUTPUT_LOG("[ParaRHI-WebGPU] wgpuDeviceGetQueue returned null.\n");
			PARA_WGPU_CALL(wgpu, wgpuDeviceRelease, device);
			PARA_WGPU_CALL(wgpu, wgpuAdapterRelease, adapter);
			PARA_WGPU_CALL(wgpu, wgpuSurfaceRelease, surface);
			PARA_WGPU_CALL(wgpu, wgpuInstanceRelease, instance);
			wgpu.Unload();
			return nullptr;
		}

		WGPUTextureFormat surfFmt = WGPUTextureFormat_BGRA8Unorm;
		if (PARA_WGPU_HAS(wgpu, wgpuSurfaceGetPreferredFormat))
		{
			WGPUTextureFormat pref = PARA_WGPU_CALL(wgpu, wgpuSurfaceGetPreferredFormat, surface, adapter);
			if (pref != WGPUTextureFormat_Undefined)
				surfFmt = pref;
		}

		const uint32_t initW = (cfg.screenWidth > 0) ? cfg.screenWidth : 1020u;
		const uint32_t initH = (cfg.screenHeight > 0) ? cfg.screenHeight : 680u;
		const bool vsync = true;

		auto* rhiDev = new RenderDeviceWebGPU(
			wgpu,
			instance,
			surface,
			adapter,
			device,
			queue,
			surfFmt,
			supportsBC,
			adapterName);
		if (!rhiDev->Initialize(initW, initH, vsync))
		{
			delete rhiDev;
			return nullptr;
		}

		OUTPUT_LOG("[ParaRHI-WebGPU] Initialized WebGPU device (%s, %ux%u, BC=%d)\n",
			adapterName.c_str(),
			initW,
			initH,
			supportsBC ? 1 : 0);
		return rhiDev;
	}

	bool RenderContextWebGPU::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
	{
		if (!device || device->GetBackendType() != ERHIBackend::WebGPU)
			return false;
		auto* wgpuDev = static_cast<RenderDeviceWebGPU*>(device);
		const uint32_t w = (cfg.screenWidth > 0) ? cfg.screenWidth : 1u;
		const uint32_t h = (cfg.screenHeight > 0) ? cfg.screenHeight : 1u;
		return wgpuDev->ResizeSwapChain(w, h, true);
	}
}

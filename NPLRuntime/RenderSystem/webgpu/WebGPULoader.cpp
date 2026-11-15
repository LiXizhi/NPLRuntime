#include "ParaEngine.h"
#include "WebGPULoader.h"

#if defined(_WIN32)
#include <windows.h>
#elif !defined(__EMSCRIPTEN__)
#include <dlfcn.h>
#endif

namespace ParaEngine
{
	namespace
	{
#if !defined(__EMSCRIPTEN__)
		void* LoadWebGPUSharedLibrary()
		{
#if defined(_WIN32)
			const char* kCandidates[] = {
				"webgpu_dawn.dll",
				"wgpu_native.dll",
				"dawn.dll"
			};
			for (const char* dll : kCandidates)
			{
				HMODULE h = ::LoadLibraryA(dll);
				if (h)
					return reinterpret_cast<void*>(h);
			}
			return nullptr;
#elif defined(__APPLE__)
			const char* kCandidates[] = {
				"libwebgpu_dawn.dylib",
				"libwgpu_native.dylib"
			};
			for (const char* lib : kCandidates)
			{
				void* h = ::dlopen(lib, RTLD_NOW | RTLD_LOCAL);
				if (h)
					return h;
			}
			return nullptr;
#else
			const char* kCandidates[] = {
				"libwebgpu_dawn.so",
				"libwgpu_native.so"
			};
			for (const char* lib : kCandidates)
			{
				void* h = ::dlopen(lib, RTLD_NOW | RTLD_LOCAL);
				if (h)
					return h;
			}
			return nullptr;
#endif
		}

		void* ResolveSym(void* mod, const char* name)
		{
			if (!mod || !name)
				return nullptr;
#if defined(_WIN32)
			return reinterpret_cast<void*>(::GetProcAddress(reinterpret_cast<HMODULE>(mod), name));
#else
			return ::dlsym(mod, name);
#endif
		}
#endif
	}

	bool WebGPUDispatch::Load()
	{
#if defined(__EMSCRIPTEN__)
		// Automatically enable WebGPU on Windows/macOS desktop browsers when navigator.gpu is supported
		// (or on any browser when explicitly requested via ?renderer=webgpu).
		const int allowWebGPU = EM_ASM_INT({
			if (typeof navigator === "undefined" || !navigator.gpu)
				return 0;
			try {
				if (typeof window !== "undefined" && window.location && window.location.search) {
					var params = new URLSearchParams(window.location.search);
					var r = (params.get("renderer") || "").trim().toLowerCase();
					if (r === "webgpu" || r === "wgpu")
						return 1;
					if (r === "opengl" || r === "gl" || r === "gles" || r === "webgl" || r === "webgl2")
						return 0;
				}
			} catch (e) {}
			var ua = navigator.userAgent || "";
			var plat = (navigator.userAgentData && navigator.userAgentData.platform) || navigator.platform || "";
			var isMobile = !!(navigator.userAgentData && navigator.userAgentData.mobile) ||
				/Android|iPhone|iPad|iPod|Mobile/i.test(ua) ||
				(/Macintosh|Mac OS X/i.test(ua) && navigator.maxTouchPoints > 1);
			if (isMobile)
				return 0;
			var isWinOrMac = /Win|Mac/i.test(plat) || /Windows|Macintosh|Mac OS X/i.test(ua);
			return isWinOrMac ? 1 : 0;
		});
		if (!allowWebGPU)
			return false;
		moduleHandle = reinterpret_cast<void*>(1);
		return true;
#else
		if (moduleHandle)
			return true;

		moduleHandle = LoadWebGPUSharedLibrary();
		if (!moduleHandle)
			return false;

#define LOAD_WGPU_REQ(fn) \
		do { \
			fn = reinterpret_cast<decltype(fn)>(ResolveSym(moduleHandle, #fn)); \
			if (!fn) { Unload(); return false; } \
		} while (0)

#define LOAD_WGPU_OPT(fn) \
		fn = reinterpret_cast<decltype(fn)>(ResolveSym(moduleHandle, #fn))

		LOAD_WGPU_REQ(wgpuCreateInstance);
		LOAD_WGPU_REQ(wgpuInstanceRelease);
		LOAD_WGPU_OPT(wgpuInstanceProcessEvents);
		LOAD_WGPU_REQ(wgpuInstanceCreateSurface);
		LOAD_WGPU_REQ(wgpuInstanceRequestAdapter);

		LOAD_WGPU_REQ(wgpuSurfaceRelease);
		LOAD_WGPU_REQ(wgpuSurfaceConfigure);
		LOAD_WGPU_OPT(wgpuSurfaceUnconfigure);
		LOAD_WGPU_REQ(wgpuSurfaceGetCurrentTexture);
		LOAD_WGPU_REQ(wgpuSurfacePresent);
		LOAD_WGPU_OPT(wgpuSurfaceGetPreferredFormat);

		LOAD_WGPU_REQ(wgpuAdapterRelease);
		LOAD_WGPU_OPT(wgpuAdapterHasFeature);
		LOAD_WGPU_OPT(wgpuAdapterGetProperties);
		LOAD_WGPU_REQ(wgpuAdapterRequestDevice);

		LOAD_WGPU_REQ(wgpuDeviceRelease);
		LOAD_WGPU_OPT(wgpuDeviceDestroy);
		LOAD_WGPU_OPT(wgpuDeviceTick);
		LOAD_WGPU_OPT(wgpuDevicePoll);
		LOAD_WGPU_OPT(wgpuDeviceHasFeature);
		LOAD_WGPU_OPT(wgpuDeviceSetUncapturedErrorCallback);
		LOAD_WGPU_REQ(wgpuDeviceGetQueue);

		LOAD_WGPU_REQ(wgpuDeviceCreateBuffer);
		LOAD_WGPU_REQ(wgpuDeviceCreateTexture);
		LOAD_WGPU_REQ(wgpuDeviceCreateSampler);
		LOAD_WGPU_REQ(wgpuDeviceCreateBindGroupLayout);
		LOAD_WGPU_REQ(wgpuDeviceCreatePipelineLayout);
		LOAD_WGPU_REQ(wgpuDeviceCreateBindGroup);
		LOAD_WGPU_REQ(wgpuDeviceCreateShaderModule);
		LOAD_WGPU_REQ(wgpuDeviceCreateRenderPipeline);
		LOAD_WGPU_REQ(wgpuDeviceCreateCommandEncoder);
		LOAD_WGPU_OPT(wgpuDeviceCreateQuerySet);

		LOAD_WGPU_REQ(wgpuQueueRelease);
		LOAD_WGPU_REQ(wgpuQueueSubmit);
		LOAD_WGPU_REQ(wgpuQueueWriteBuffer);
		LOAD_WGPU_REQ(wgpuQueueWriteTexture);

		LOAD_WGPU_REQ(wgpuBufferRelease);
		LOAD_WGPU_OPT(wgpuBufferDestroy);
		LOAD_WGPU_REQ(wgpuBufferMapAsync);
		LOAD_WGPU_OPT(wgpuBufferGetMappedRange);
		LOAD_WGPU_REQ(wgpuBufferGetConstMappedRange);
		LOAD_WGPU_REQ(wgpuBufferUnmap);

		LOAD_WGPU_REQ(wgpuTextureRelease);
		LOAD_WGPU_OPT(wgpuTextureDestroy);
		LOAD_WGPU_REQ(wgpuTextureCreateView);
		LOAD_WGPU_REQ(wgpuTextureViewRelease);

		LOAD_WGPU_REQ(wgpuSamplerRelease);
		LOAD_WGPU_REQ(wgpuBindGroupLayoutRelease);
		LOAD_WGPU_REQ(wgpuPipelineLayoutRelease);
		LOAD_WGPU_REQ(wgpuBindGroupRelease);
		LOAD_WGPU_REQ(wgpuShaderModuleRelease);
		LOAD_WGPU_REQ(wgpuRenderPipelineRelease);
		LOAD_WGPU_OPT(wgpuQuerySetRelease);
		LOAD_WGPU_OPT(wgpuQuerySetDestroy);

		LOAD_WGPU_REQ(wgpuCommandEncoderRelease);
		LOAD_WGPU_REQ(wgpuCommandEncoderBeginRenderPass);
		LOAD_WGPU_REQ(wgpuCommandEncoderCopyBufferToBuffer);
		LOAD_WGPU_REQ(wgpuCommandEncoderCopyBufferToTexture);
		LOAD_WGPU_REQ(wgpuCommandEncoderCopyTextureToBuffer);
		LOAD_WGPU_REQ(wgpuCommandEncoderCopyTextureToTexture);
		LOAD_WGPU_OPT(wgpuCommandEncoderResolveQuerySet);
		LOAD_WGPU_REQ(wgpuCommandEncoderFinish);
		LOAD_WGPU_REQ(wgpuCommandBufferRelease);

		LOAD_WGPU_REQ(wgpuRenderPassEncoderRelease);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderEnd);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderSetPipeline);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderSetBindGroup);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderSetVertexBuffer);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderSetIndexBuffer);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderSetViewport);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderSetScissorRect);
		LOAD_WGPU_OPT(wgpuRenderPassEncoderSetBlendConstant);
		LOAD_WGPU_OPT(wgpuRenderPassEncoderSetStencilReference);
		LOAD_WGPU_OPT(wgpuRenderPassEncoderBeginOcclusionQuery);
		LOAD_WGPU_OPT(wgpuRenderPassEncoderEndOcclusionQuery);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderDraw);
		LOAD_WGPU_REQ(wgpuRenderPassEncoderDrawIndexed);

#undef LOAD_WGPU_REQ
#undef LOAD_WGPU_OPT
		return true;
#endif
	}

	void WebGPUDispatch::Unload()
	{
#if defined(__EMSCRIPTEN__)
		moduleHandle = nullptr;
#else
		if (moduleHandle)
		{
#if defined(_WIN32)
			::FreeLibrary(reinterpret_cast<HMODULE>(moduleHandle));
#else
			::dlclose(moduleHandle);
#endif
			moduleHandle = nullptr;
		}
#endif
	}

	void WebGPUDispatch::ProcessEvents(WGPUInstance instance, WGPUDevice device) const
	{
#if defined(__EMSCRIPTEN__)
		(void)device;
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
		if (instance)
			::wgpuInstanceProcessEvents(instance);
#else
		(void)instance;
#endif
#else
		if (wgpuDeviceTick && device)
			wgpuDeviceTick(device);
		if (wgpuDevicePoll && device)
			wgpuDevicePoll(device, 1, nullptr);
		if (wgpuInstanceProcessEvents && instance)
			wgpuInstanceProcessEvents(instance);
#endif
	}

	void WebGPUDispatch::WaitAsync(WGPUInstance instance, WGPUDevice device) const
	{
#if defined(__EMSCRIPTEN__)
		emscripten_sleep(1);
		ProcessEvents(instance, device);
#else
		ProcessEvents(instance, device);
#endif
	}
}

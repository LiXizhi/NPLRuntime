#pragma once
#include <stdint.h>
#include <stddef.h>
#include "Framework/RenderSystem/RenderTypes.h"

namespace ParaEngine
{
	/**
	 * Runtime-selectable cross-platform graphics backend identifiers for ParaRHI.
	 */
	enum class ERHIBackend : uint8_t
	{
		Auto = 0,
		D3D11,      // Direct3D 11 (Feature Levels 9_3 .. 11_1 + WARP fallback)
		D3D12,      // Direct3D 12 (Windows 10/11 modern explicit API)
		Vulkan,     // Vulkan 1.1+ (Windows, Linux, Android, HarmonyOS, PicoVR)
		Metal,      // Apple Metal 2+ (macOS, iOS)
		OpenGL,     // OpenGL 3.3/2.1, OpenGL ES 3.0/2.0, WebGL 2.0/1.0
		WebGPU,     // WebGPU (Emscripten WASM / Dawn / wgpu)
		Null        // Headless server & unit-test stub backend
	};

	/**
	 * Hardware & API capability flags queried at runtime across all ParaRHI backends.
	 */
	struct RHIDeviceCaps
	{
		ERHIBackend backend = ERHIBackend::Null;
		const char* backendName = "Null";
		const char* adapterName = "Headless";
		uint32_t featureLevel = 0;               // e.g. 0x9300..0xb100 for D3D11, 330 for GL 3.3
		uint32_t maxSimultaneousRTs = 1;         // >= 4 enables Deferred Shading (Fancy Graphics MRT)
		uint32_t maxTextureStages = 8;           // >= 8 enables 8-stage GeoMipmap terrain splatting
		uint32_t maxTextureSize = 4096;
		uint32_t maxAnisotropy = 1;
		bool supportsFloatRenderTargets = false; // R32F / RGBA16F (ShadowMaps, Depth MRT, HDR)
		bool supportsDepthTextures = false;      // Hardware depth-texture shadow comparison
		bool supportsInstancing = false;         // Hardware mesh & block instancing
		bool supportsOcclusionQuery = false;     // LatentOcclusionQueryBank GPU occlusion culling
		bool supportsTwoSidedStencil = false;    // ShadowVolume stencil shadows
		bool supportsUserClipPlanes = false;     // Planar mirrors & water reflection clip plane
		bool supportsBCCompression = false;      // Native DXT1/3/5/BC7; false triggers CPU S3TC decode
		bool supportsETC2Compression = false;    // Native ETC1/ETC2
		bool supportsASTCCompression = false;    // Native ASTC
		bool supportsAsyncReadback = false;      // Non-blocking staging texture readback
		bool supportsRuntimeShaderCompile = true;// Runtime .fx compilation via ShaderTranspiler
		bool topLeftScreenOrigin = true;         // false on OpenGL FBOs (requires Y-flip)
		bool clipSpaceZeroToOne = true;          // true: D3D/VK/MTL/WGPU [0,1]; false: GL [-1,1]
		bool needsHalfPixelOffset = false;       // false on D3D11+/GL/VK/MTL/WGPU

		int GetShaderModelMajor() const
		{
			switch (backend)
			{
			case ERHIBackend::D3D11:
				if (featureLevel >= 0xb000) return 5;
				if (featureLevel >= 0xa000) return 4;
				return (featureLevel >= 0x9100) ? 2 : 0;
			case ERHIBackend::OpenGL:
				if (featureLevel >= 330) return 4;
				if (featureLevel >= 300) return 3;
				return (featureLevel >= 200) ? 2 : 0;
			case ERHIBackend::D3D12:
			case ERHIBackend::Vulkan:
			case ERHIBackend::Metal:
			case ERHIBackend::WebGPU:
				return 5;
			case ERHIBackend::Null:
			case ERHIBackend::Auto:
			default:
				return 0;
			}
		}
	};

	enum class ERHIBufferUsage : uint32_t
	{
		Static    = 0,
		Dynamic   = 1 << 0,
		Stream    = 1 << 1,
		WriteOnly = 1 << 2,
		Readback  = 1 << 3
	};

	inline constexpr ERHIBufferUsage operator|(ERHIBufferUsage a, ERHIBufferUsage b)
	{
		return static_cast<ERHIBufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline constexpr ERHIBufferUsage operator&(ERHIBufferUsage a, ERHIBufferUsage b)
	{
		return static_cast<ERHIBufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}
	inline ERHIBufferUsage& operator|=(ERHIBufferUsage& a, ERHIBufferUsage b)
	{
		a = a | b;
		return a;
	}
	inline constexpr bool HasFlag(ERHIBufferUsage flags, ERHIBufferUsage bit)
	{
		return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(bit)) != 0;
	}

	enum class ERHIMapMode : uint8_t
	{
		Read = 0,
		Write,
		ReadWrite,
		WriteDiscard,
		WriteNoOverwrite
	};

	enum class ERHITextureUsage : uint32_t
	{
		None           = 0,
		ShaderResource = 1 << 0,
		RenderTarget   = 1 << 1,
		DepthStencil   = 1 << 2,
		Dynamic        = 1 << 3,
		CubeMap        = 1 << 4,
		Readback       = 1 << 5
	};

	inline constexpr ERHITextureUsage operator|(ERHITextureUsage a, ERHITextureUsage b)
	{
		return static_cast<ERHITextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline constexpr ERHITextureUsage operator&(ERHITextureUsage a, ERHITextureUsage b)
	{
		return static_cast<ERHITextureUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}
	inline ERHITextureUsage& operator|=(ERHITextureUsage& a, ERHITextureUsage b)
	{
		a = a | b;
		return a;
	}
	inline constexpr bool HasFlag(ERHITextureUsage flags, ERHITextureUsage bit)
	{
		return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(bit)) != 0;
	}

	struct RHIMipData
	{
		const void* data = nullptr;
		uint32_t dataSize = 0;
		uint32_t rowPitch = 0;
	};

	/**
	 * Abstract GPU texture resource implemented by every ParaRHI backend.
	 * Note: GLTexture2D, D3D11Texture, D3D12Texture, VulkanTexture, MetalTexture,
	 * WebGPUTexture, and NullTexture all implement IRHITexture.
	 */
	class IRHITexture
	{
	public:
		virtual ~IRHITexture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetMipLevels() const { return 1; }
		virtual PixelFormat GetFormat() const { return PixelFormat::A8R8G8B8; }
		virtual bool IsCubeMap() const { return false; }
		virtual bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t rowPitch = 0) { return false; }
		virtual void* GetNativeHandle() const { return nullptr; }
		virtual void ReleaseRHIResource() { delete this; }
	};

	/**
	 * Abstract GPU vertex/index/constant buffer implemented by every ParaRHI backend.
	 */
	class IRHIBuffer
	{
	public:
		virtual ~IRHIBuffer() = default;
		virtual uint32_t GetSize() const = 0;
		virtual bool IsIndexBuffer() const = 0;
		virtual void* Map(uint32_t offset, uint32_t size, ERHIMapMode mode) = 0;
		virtual void Unmap() = 0;
		virtual bool UpdateData(uint32_t offset, uint32_t size, const void* data) = 0;
		virtual void* GetNativeHandle() const { return nullptr; }
		virtual void ReleaseRHIResource() { delete this; }
	};

	/**
	 * Abstract offscreen render target / depth-stencil attachment for single RT and 4-target MRT.
	 */
	class IRHIRenderTarget
	{
	public:
		virtual ~IRHIRenderTarget() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual PixelFormat GetFormat() const = 0;
		virtual IRHITexture* GetColorTexture(uint32_t index = 0) const = 0;
		virtual IRHITexture* GetDepthTexture() const { return nullptr; }
		virtual void* GetNativeHandle() const { return nullptr; }
		virtual void ReleaseRHIResource() { delete this; }
	};

	/**
	 * Abstract GPU occlusion query used by LatentOcclusionQueryBank.
	 */
	class IRHIOcclusionQuery
	{
	public:
		virtual ~IRHIOcclusionQuery() = default;
		virtual bool Begin() = 0;
		virtual void End() = 0;
		/** Returns true if query result is ready (or if flush=true blocks until ready). */
		virtual bool GetResult(uint64_t& outSamplesPassed, bool flush = false) = 0;
		virtual void ReleaseRHIResource() { delete this; }
	};

	/**
	 * Descriptor for compiling a VS+PS shader program pass on native RHI backends.
	 */
	struct RHIShaderProgramDesc
	{
		const void* vsBytecode = nullptr;
		uint32_t vsBytecodeSize = 0;
		const void* psBytecode = nullptr;
		uint32_t psBytecodeSize = 0;
		uint32_t vsGlobalCBufferSize = 0;
		uint32_t psGlobalCBufferSize = 0;
		const char* vsEntryPoint = "main";
		const char* psEntryPoint = "main";
		const char* debugName = nullptr;
	};

	/**
	 * Abstract compiled VS+PS shader program pass.
	 */
	class IRHIShaderProgram
	{
	public:
		virtual ~IRHIShaderProgram() = default;
		virtual bool IsValid() const = 0;
		virtual int32_t GetUniformLocation(const char* name) const { return -1; }
		virtual bool SetUniformRaw(int32_t location, const void* data, uint32_t byteSize) { return false; }
		virtual bool SetUniformByName(const char* name, const void* data, uint32_t byteSize) { return false; }
		virtual void* GetNativeHandle() const { return nullptr; }
		virtual void ReleaseRHIResource() { delete this; }
	};

	template <typename T>
	inline void ReleaseRHIResource(T*& ptr)
	{
		if (ptr)
		{
			ptr->ReleaseRHIResource();
			ptr = nullptr;
		}
	}
}

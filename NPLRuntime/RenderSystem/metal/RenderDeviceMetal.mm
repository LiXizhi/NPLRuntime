#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <TargetConditionals.h>
#include <dispatch/dispatch.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#include "ParaEngine.h"
#include "RenderContextMetal.h"
#include "RenderDeviceMetal.h"
#include "FxShaderTypes.h"
#include "RHISoftwareS3TC.h"
#include "renderer/VertexDeclarationOpenGL.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace ParaEngine
{
	namespace
	{
		inline uint32_t AlignUpMtl(uint32_t val, uint32_t align)
		{
			return (align <= 1u) ? val : ((val + align - 1u) & ~(align - 1u));
		}

		inline void HashCombineMtlU64(uint64_t& h, uint64_t v)
		{
			h ^= v;
			h *= 1099511628211ULL;
		}

		uint64_t ComputeMtlByteHash64(const void* data, size_t size)
		{
			uint64_t h = 14695981039346656037ULL;
			const uint8_t* p = static_cast<const uint8_t*>(data);
			for (size_t i = 0; i < size; ++i)
			{
				h ^= static_cast<uint64_t>(p[i]);
				h *= 1099511628211ULL;
			}
			return h;
		}

		bool IsDepthPixelFormatMtl(PixelFormat fmt)
		{
			switch (fmt)
			{
			case PixelFormat::D16:
			case PixelFormat::D16_LOCKABLE:
			case PixelFormat::D24S8:
			case PixelFormat::D24X8:
			case PixelFormat::D24FS8:
			case PixelFormat::D24X4S4:
			case PixelFormat::D15S1:
			case PixelFormat::D32:
			case PixelFormat::D32F_LOCKABLE:
				return true;
			default:
				return false;
			}
		}

		MTLPixelFormat MapDepthFormatToMtl(PixelFormat fmt)
		{
			switch (fmt)
			{
			case PixelFormat::D16:
			case PixelFormat::D16_LOCKABLE:
#if !TARGET_OS_IPHONE
				return MTLPixelFormatDepth16Unorm;
#else
				return MTLPixelFormatDepth32Float;
#endif
			case PixelFormat::D32:
			case PixelFormat::D32F_LOCKABLE:
				return MTLPixelFormatDepth32Float;
			default:
				return MTLPixelFormatDepth32Float_Stencil8;
			}
		}

		MTLPixelFormat MapColorFormatToMtl(PixelFormat fmt, bool supportsBC, uint32_t& outBytesPerBlock, bool& outIsBC)
		{
			outBytesPerBlock = 4;
			outIsBC = false;
			switch (fmt)
			{
			case PixelFormat::A8R8G8B8:
			case PixelFormat::X8R8G8B8:
				return MTLPixelFormatRGBA8Unorm;
			case PixelFormat::A8B8G8R8:
			case PixelFormat::X8B8G8R8:
				return MTLPixelFormatBGRA8Unorm;
			case PixelFormat::R16F:
				outBytesPerBlock = 2;
				return MTLPixelFormatR16Float;
			case PixelFormat::R32F:
				outBytesPerBlock = 4;
				return MTLPixelFormatR32Float;
			case PixelFormat::G16R16F:
				outBytesPerBlock = 4;
				return MTLPixelFormatRG16Float;
			case PixelFormat::G32R32F:
				outBytesPerBlock = 8;
				return MTLPixelFormatRG32Float;
			case PixelFormat::A16B16G16R16F:
				outBytesPerBlock = 8;
				return MTLPixelFormatRGBA16Float;
			case PixelFormat::A32B32G32R32F:
				outBytesPerBlock = 16;
				return MTLPixelFormatRGBA32Float;
			case PixelFormat::A16B16G16R16:
				outBytesPerBlock = 8;
				return MTLPixelFormatRGBA16Unorm;
			case PixelFormat::A2B10G10R10:
			case PixelFormat::A2R10G10B10:
				outBytesPerBlock = 4;
				return MTLPixelFormatRGB10A2Unorm;
			case PixelFormat::DXT1:
				if (supportsBC)
				{
					outBytesPerBlock = 8;
					outIsBC = true;
#if !TARGET_OS_IPHONE
					return MTLPixelFormatBC1_RGBA;
#else
					return static_cast<MTLPixelFormat>(130); // MTLPixelFormatBC1_RGBA on Apple Silicon iOS 16.4+
#endif
				}
				return MTLPixelFormatRGBA8Unorm;
			case PixelFormat::DXT2:
			case PixelFormat::DXT3:
				if (supportsBC)
				{
					outBytesPerBlock = 16;
					outIsBC = true;
#if !TARGET_OS_IPHONE
					return MTLPixelFormatBC2_RGBA;
#else
					return static_cast<MTLPixelFormat>(132);
#endif
				}
				return MTLPixelFormatRGBA8Unorm;
			case PixelFormat::DXT4:
			case PixelFormat::DXT5:
				if (supportsBC)
				{
					outBytesPerBlock = 16;
					outIsBC = true;
#if !TARGET_OS_IPHONE
					return MTLPixelFormatBC3_RGBA;
#else
					return static_cast<MTLPixelFormat>(134);
#endif
				}
				return MTLPixelFormatRGBA8Unorm;
			default:
				return MTLPixelFormatRGBA8Unorm;
			}
		}

		bool MapPrimitiveTopologyMtl(EPrimitiveType primType, uint32_t primCount, MTLPrimitiveType& outTopo, uint32_t& outVertCount)
		{
			if (primCount == 0)
				return false;
			switch (primType)
			{
			case EPrimitiveType::POINTLIST:
				outTopo = MTLPrimitiveTypePoint;
				outVertCount = primCount;
				return true;
			case EPrimitiveType::LINELIST:
				outTopo = MTLPrimitiveTypeLine;
				outVertCount = primCount * 2u;
				return true;
			case EPrimitiveType::LINESTRIP:
				outTopo = MTLPrimitiveTypeLineStrip;
				outVertCount = primCount + 1u;
				return true;
			case EPrimitiveType::TRIANGLELIST:
				outTopo = MTLPrimitiveTypeTriangle;
				outVertCount = primCount * 3u;
				return true;
			case EPrimitiveType::TRIANGLESTRIP:
			case EPrimitiveType::TRIANGLEFAN:
				outTopo = MTLPrimitiveTypeTriangleStrip;
				outVertCount = primCount + 2u;
				return true;
			default:
				return false;
			}
		}

		MTLCompareFunction ToMtlCompareFn(uint32_t rsvCmp)
		{
			switch (rsvCmp)
			{
			case RSV_CMP_NEVER:        return MTLCompareFunctionNever;
			case RSV_CMP_LESS:         return MTLCompareFunctionLess;
			case RSV_CMP_EQUAL:        return MTLCompareFunctionEqual;
			case RSV_CMP_LESSEQUAL:    return MTLCompareFunctionLessEqual;
			case RSV_CMP_GREATER:      return MTLCompareFunctionGreater;
			case RSV_CMP_NOTEQUAL:     return MTLCompareFunctionNotEqual;
			case RSV_CMP_GREATEREQUAL: return MTLCompareFunctionGreaterEqual;
			case RSV_CMP_ALWAYS:
			default:                   return MTLCompareFunctionAlways;
			}
		}

		MTLBlendFactor ToMtlBlendFactor(uint32_t rsvBlend, bool isAlpha)
		{
			switch (rsvBlend)
			{
			case RSV_BLEND_ZERO:           return MTLBlendFactorZero;
			case RSV_BLEND_ONE:            return MTLBlendFactorOne;
			case RSV_BLEND_SRCCOLOR:       return isAlpha ? MTLBlendFactorSourceAlpha : MTLBlendFactorSourceColor;
			case RSV_BLEND_INVSRCCOLOR:    return isAlpha ? MTLBlendFactorOneMinusSourceAlpha : MTLBlendFactorOneMinusSourceColor;
			case RSV_BLEND_SRCALPHA:       return MTLBlendFactorSourceAlpha;
			case RSV_BLEND_INVSRCALPHA:    return MTLBlendFactorOneMinusSourceAlpha;
			case RSV_BLEND_DESTALPHA:      return MTLBlendFactorDestinationAlpha;
			case RSV_BLEND_INVDESTALPHA:   return MTLBlendFactorOneMinusDestinationAlpha;
			case RSV_BLEND_DESTCOLOR:      return isAlpha ? MTLBlendFactorDestinationAlpha : MTLBlendFactorDestinationColor;
			case RSV_BLEND_INVDESTCOLOR:   return isAlpha ? MTLBlendFactorOneMinusDestinationAlpha : MTLBlendFactorOneMinusDestinationColor;
			case RSV_BLEND_SRCALPHASAT:    return MTLBlendFactorSourceAlphaSaturated;
			case RSV_BLEND_BLENDFACTOR:    return isAlpha ? MTLBlendFactorBlendAlpha : MTLBlendFactorBlendColor;
			case RSV_BLEND_INVBLENDFACTOR: return isAlpha ? MTLBlendFactorOneMinusBlendAlpha : MTLBlendFactorOneMinusBlendColor;
			default:                       return MTLBlendFactorOne;
			}
		}

		MTLBlendOperation ToMtlBlendOp(uint32_t op)
		{
			switch (op)
			{
			case 2:  return MTLBlendOperationSubtract;
			case 3:  return MTLBlendOperationReverseSubtract;
			case 4:  return MTLBlendOperationMin;
			case 5:  return MTLBlendOperationMax;
			case 1:
			default: return MTLBlendOperationAdd;
			}
		}

		MTLStencilOperation ToMtlStencilOp(uint32_t op)
		{
			switch (op)
			{
			case RSV_STENCILOP_ZERO:    return MTLStencilOperationZero;
			case RSV_STENCILOP_REPLACE: return MTLStencilOperationReplace;
			case RSV_STENCILOP_INCRSAT: return MTLStencilOperationIncrementClamp;
			case RSV_STENCILOP_DECRSAT: return MTLStencilOperationDecrementClamp;
			case RSV_STENCILOP_INVERT:  return MTLStencilOperationInvert;
			case RSV_STENCILOP_INCR:    return MTLStencilOperationIncrementWrap;
			case RSV_STENCILOP_DECR:    return MTLStencilOperationDecrementWrap;
			case RSV_STENCILOP_KEEP:
			default:                    return MTLStencilOperationKeep;
			}
		}

		MTLSamplerAddressMode ToMtlAddressMode(uint32_t mode)
		{
			switch (mode)
			{
			case 2: return MTLSamplerAddressModeMirrorRepeat;
			case 3: return MTLSamplerAddressModeClampToEdge;
			case 4:
#if !TARGET_OS_IPHONE
				return MTLSamplerAddressModeClampToBorderColor;
#else
				return MTLSamplerAddressModeClampToZero;
#endif
			case 5:
#if !TARGET_OS_IPHONE
				return MTLSamplerAddressModeMirrorClampToEdge;
#else
				return MTLSamplerAddressModeMirrorRepeat;
#endif
			case 1:
			default: return MTLSamplerAddressModeRepeat;
			}
		}

		MTLVertexFormat MapDeclTypeToMtlVertexFormat(uint8_t declType)
		{
			switch (declType)
			{
			case D3DDECLTYPE_FLOAT1:    return MTLVertexFormatFloat;
			case D3DDECLTYPE_FLOAT2:    return MTLVertexFormatFloat2;
			case D3DDECLTYPE_FLOAT3:    return MTLVertexFormatFloat3;
			case D3DDECLTYPE_FLOAT4:    return MTLVertexFormatFloat4;
			case D3DDECLTYPE_D3DCOLOR:  return MTLVertexFormatUChar4Normalized_BGRA;
			case D3DDECLTYPE_UBYTE4:    return MTLVertexFormatUChar4Normalized;
			case D3DDECLTYPE_SHORT2:    return MTLVertexFormatShort2;
			case D3DDECLTYPE_SHORT4:    return MTLVertexFormatShort4;
			case D3DDECLTYPE_UBYTE4N:   return MTLVertexFormatUChar4Normalized;
			case D3DDECLTYPE_SHORT2N:   return MTLVertexFormatShort2Normalized;
			case D3DDECLTYPE_SHORT4N:   return MTLVertexFormatShort4Normalized;
			case D3DDECLTYPE_USHORT2N:  return MTLVertexFormatUShort2Normalized;
			case D3DDECLTYPE_USHORT4N:  return MTLVertexFormatUShort4Normalized;
			case D3DDECLTYPE_FLOAT16_2: return MTLVertexFormatHalf2;
			case D3DDECLTYPE_FLOAT16_4: return MTLVertexFormatHalf4;
			default:                    return MTLVertexFormatFloat3;
			}
		}

		int32_t MapDeclUsageToLocationMtl(uint8_t usage, uint8_t usageIndex)
		{
			switch (usage)
			{
			case D3DDECLUSAGE_POSITION:
			case D3DDECLUSAGE_POSITIONT:    return 0;
			case D3DDECLUSAGE_BLENDWEIGHT:  return 1;
			case D3DDECLUSAGE_BLENDINDICES: return 2;
			case D3DDECLUSAGE_NORMAL:       return 3;
			case D3DDECLUSAGE_COLOR:        return (usageIndex == 0) ? 4 : 5;
			case D3DDECLUSAGE_TEXCOORD:     return (usageIndex < 8) ? static_cast<int32_t>(6 + usageIndex) : 13;
			case D3DDECLUSAGE_TANGENT:      return 14;
			case D3DDECLUSAGE_BINORMAL:
			case D3DDECLUSAGE_PSIZE:        return 15;
			default:                        return -1;
			}
		}
	}

	// =========================================================================
	// Metal RHI Resource Implementations
	// =========================================================================

	class MetalTexture : public IRHITexture
	{
	public:
		MetalTexture(
			RenderDeviceMetal* owner,
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			PixelFormat srcFmt,
			MTLPixelFormat mtlFmt,
			bool isCube,
			bool isBC,
			bool expandedToRGBA8,
			uint32_t bytesPerBlock,
			id<MTLTexture> tex,
			id<MTLTexture> msaaTex = nil)
			: m_owner(owner)
			, m_width(width)
			, m_height(height)
			, m_mipLevels(mipLevels)
			, m_format(srcFmt)
			, m_mtlFormat(mtlFmt)
			, m_isCube(isCube)
			, m_isBC(isBC)
			, m_expandedToRGBA8(expandedToRGBA8)
			, m_bytesPerBlock(bytesPerBlock)
			, m_texture(tex)
			, m_msaaTexture(msaaTex)
		{
		}

		~MetalTexture() override
		{
			if (m_owner)
			{
				if (m_texture) m_owner->DeferRelease(reinterpret_cast<void*>(m_texture));
				if (m_msaaTexture) m_owner->DeferRelease(reinterpret_cast<void*>(m_msaaTexture));
			}
			m_texture = nil;
			m_msaaTexture = nil;
		}

		uint32_t GetWidth() const override { return m_width; }
		uint32_t GetHeight() const override { return m_height; }
		uint32_t GetMipLevels() const override { return m_mipLevels; }
		PixelFormat GetFormat() const override { return m_format; }
		bool IsCubeMap() const override { return m_isCube; }
		void* GetNativeHandle() const override { return const_cast<MetalTexture*>(this); }

		id<MTLTexture> GetMTLTexture() const { return m_texture; }
		id<MTLTexture> GetMSAATexture() const { return m_msaaTexture; }
		MTLPixelFormat GetMTLPixelFormat() const { return m_mtlFormat; }

		bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t rowPitch = 0) override
		{
			if (!m_texture || !data || width == 0 || height == 0 || mipLevel >= m_mipLevels)
				return false;

			MTLRegion region = MTLRegionMake2D(static_cast<NSUInteger>((std::max)(0, x)), static_cast<NSUInteger>((std::max)(0, y)), width, height);

			if (RHISoftwareS3TC::IsS3TCFormat(m_format) && !m_isBC)
			{
				size_t compBytes = rowPitch ? static_cast<size_t>(rowPitch) * ((height + 3u) / 4u)
					: RHISoftwareS3TC::ComputeCompressedMipSize(m_format, width, height);
				std::vector<uint8_t> rgba;
				if (!RHISoftwareS3TC::DecodeToRGBA8(m_format, data, compBytes, width, height, rgba))
					return false;
				[m_texture replaceRegion:region mipmapLevel:mipLevel slice:arraySlice withBytes:rgba.data() bytesPerRow:width * 4u bytesPerImage:0];
				return true;
			}

			if (m_expandedToRGBA8)
			{
				std::vector<uint8_t> rgba;
				if (!RHISoftwareS3TC::ExpandToRGBA8(m_format, data, width, height, rowPitch, rgba))
					return false;
				[m_texture replaceRegion:region mipmapLevel:mipLevel slice:arraySlice withBytes:rgba.data() bytesPerRow:width * 4u bytesPerImage:0];
				return true;
			}

			uint32_t bpr = rowPitch;
			if (bpr == 0)
			{
				if (m_isBC)
					bpr = ((width + 3u) / 4u) * m_bytesPerBlock;
				else
					bpr = width * m_bytesPerBlock;
			}
			[m_texture replaceRegion:region mipmapLevel:mipLevel slice:arraySlice withBytes:data bytesPerRow:bpr bytesPerImage:0];
			return true;
		}

	private:
		RenderDeviceMetal* m_owner = nullptr;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_mipLevels = 1;
		PixelFormat m_format = PixelFormat::A8R8G8B8;
		MTLPixelFormat m_mtlFormat = MTLPixelFormatRGBA8Unorm;
		bool m_isCube = false;
		bool m_isBC = false;
		bool m_expandedToRGBA8 = false;
		uint32_t m_bytesPerBlock = 4;
		id<MTLTexture> m_texture = nil;
		id<MTLTexture> m_msaaTexture = nil;
	};

	class MetalBuffer : public IRHIBuffer
	{
	public:
		MetalBuffer(RenderDeviceMetal* owner, id<MTLDevice> dev, uint32_t byteSize, bool isIndex, ERHIBufferUsage usage, id<MTLBuffer> buf)
			: m_owner(owner)
			, m_device(dev)
			, m_byteSize(byteSize)
			, m_isIndex(isIndex)
			, m_usage(usage)
			, m_buffer(buf)
		{
		}

		~MetalBuffer() override
		{
			if (m_owner && m_buffer)
				m_owner->DeferRelease(reinterpret_cast<void*>(m_buffer));
			m_buffer = nil;
		}

		uint32_t GetSize() const override { return m_byteSize; }
		bool IsIndexBuffer() const override { return m_isIndex; }
		void* GetNativeHandle() const override { return const_cast<MetalBuffer*>(this); }
		id<MTLBuffer> GetMTLBuffer() const { return m_buffer; }

		void* Map(uint32_t offset, uint32_t size, ERHIMapMode mode) override
		{
			if (!m_buffer || offset >= m_byteSize)
				return nullptr;
			(void)size;
			if (mode == ERHIMapMode::WriteDiscard && m_device)
			{
				id<MTLBuffer> fresh = [m_device newBufferWithLength:(std::max)(m_byteSize, 16u) options:MTLResourceStorageModeShared];
				if (fresh)
				{
					if (m_owner && m_buffer)
						m_owner->DeferRelease(reinterpret_cast<void*>(m_buffer));
					m_buffer = fresh;
				}
			}
			uint8_t* base = static_cast<uint8_t*>([m_buffer contents]);
			return base ? (base + offset) : nullptr;
		}

		void Unmap() override {}

		bool UpdateData(uint32_t offset, uint32_t size, const void* data) override
		{
			if (!m_buffer || !data || size == 0 || offset + size > m_byteSize)
				return false;
			if (offset == 0 && size == m_byteSize && HasFlag(m_usage, ERHIBufferUsage::Dynamic) && m_device)
			{
				id<MTLBuffer> fresh = [m_device newBufferWithBytes:data length:(std::max)(m_byteSize, 16u) options:MTLResourceStorageModeShared];
				if (fresh)
				{
					if (m_owner && m_buffer)
						m_owner->DeferRelease(reinterpret_cast<void*>(m_buffer));
					m_buffer = fresh;
					return true;
				}
			}
			uint8_t* base = static_cast<uint8_t*>([m_buffer contents]);
			if (!base)
				return false;
			std::memcpy(base + offset, data, size);
			return true;
		}

	private:
		RenderDeviceMetal* m_owner = nullptr;
		id<MTLDevice> m_device = nil;
		uint32_t m_byteSize = 0;
		bool m_isIndex = false;
		ERHIBufferUsage m_usage = ERHIBufferUsage::Static;
		id<MTLBuffer> m_buffer = nil;
	};

	class MetalRenderTarget : public IRHIRenderTarget
	{
	public:
		MetalRenderTarget(
			uint32_t w,
			uint32_t h,
			PixelFormat fmt,
			const std::vector<MetalTexture*>& colors,
			MetalTexture* depth)
			: m_width(w)
			, m_height(h)
			, m_format(fmt)
			, m_colors(colors)
			, m_depth(depth)
		{
		}

		~MetalRenderTarget() override
		{
			for (MetalTexture* c : m_colors)
				delete c;
			m_colors.clear();
			delete m_depth;
			m_depth = nullptr;
		}

		uint32_t GetWidth() const override { return m_width; }
		uint32_t GetHeight() const override { return m_height; }
		PixelFormat GetFormat() const override { return m_format; }
		IRHITexture* GetColorTexture(uint32_t index = 0) const override
		{
			return (index < m_colors.size()) ? m_colors[index] : nullptr;
		}
		IRHITexture* GetDepthTexture() const override { return m_depth; }
		void* GetNativeHandle() const override { return const_cast<MetalRenderTarget*>(this); }

		uint32_t GetColorCount() const { return static_cast<uint32_t>(m_colors.size()); }
		MetalTexture* GetMetalColor(uint32_t idx) const { return (idx < m_colors.size()) ? m_colors[idx] : nullptr; }
		MetalTexture* GetMetalDepth() const { return m_depth; }

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		PixelFormat m_format = PixelFormat::A8R8G8B8;
		std::vector<MetalTexture*> m_colors;
		MetalTexture* m_depth = nullptr;
	};

	class MetalOcclusionQuery : public IRHIOcclusionQuery
	{
	public:
		explicit MetalOcclusionQuery(RenderDeviceMetal* owner)
			: m_owner(owner)
		{
			if (m_owner)
				m_slot = m_owner->AllocateOcclusionSlot();
		}

		~MetalOcclusionQuery() override
		{
			if (m_owner && m_slot != UINT32_MAX)
				m_owner->FreeOcclusionSlot(m_slot);
		}

		bool Begin() override
		{
			if (!m_owner || m_slot == UINT32_MAX)
				return false;
			m_owner->BeginOcclusionOnActiveEncoder(m_slot);
			return true;
		}

		void End() override
		{
			if (m_owner && m_slot != UINT32_MAX)
				m_owner->EndOcclusionOnActiveEncoder();
		}

		bool GetResult(uint64_t& outSamplesPassed, bool flush) override
		{
			if (!m_owner || m_slot == UINT32_MAX)
			{
				outSamplesPassed = 1;
				return true;
			}
			if (flush)
				m_owner->ExecuteAndSyncGPU();
			outSamplesPassed = m_owner->ReadOcclusionSlotValue(m_slot);
			return true;
		}

	private:
		RenderDeviceMetal* m_owner = nullptr;
		uint32_t m_slot = UINT32_MAX;
	};

	class MetalShaderProgram : public IRHIShaderProgram
	{
	public:
		MetalShaderProgram(
			RenderDeviceMetal* owner,
			id<MTLFunction> vsFn,
			id<MTLFunction> psFn,
			uint64_t hash,
			uint32_t vsCbSize,
			uint32_t psCbSize)
			: m_owner(owner)
			, m_vsFunc(vsFn)
			, m_psFunc(psFn)
			, m_hash(hash)
			, m_vsCbSize(vsCbSize)
			, m_psCbSize(psCbSize)
		{
		}

		~MetalShaderProgram() override
		{
			if (m_owner)
			{
				if (m_vsFunc) m_owner->DeferRelease(reinterpret_cast<void*>(m_vsFunc));
				if (m_psFunc) m_owner->DeferRelease(reinterpret_cast<void*>(m_psFunc));
			}
			m_vsFunc = nil;
			m_psFunc = nil;
		}

		bool IsValid() const override { return m_vsFunc != nil; }
		void* GetNativeHandle() const override { return const_cast<MetalShaderProgram*>(this); }

		id<MTLFunction> GetVSFunction() const { return m_vsFunc; }
		id<MTLFunction> GetPSFunction() const { return m_psFunc; }
		uint64_t GetHash() const { return m_hash; }
		uint32_t GetVSCBufferSize() const { return m_vsCbSize; }
		uint32_t GetPSCBufferSize() const { return m_psCbSize; }

	private:
		RenderDeviceMetal* m_owner = nullptr;
		id<MTLFunction> m_vsFunc = nil;
		id<MTLFunction> m_psFunc = nil;
		uint64_t m_hash = 0;
		uint32_t m_vsCbSize = 0;
		uint32_t m_psCbSize = 0;
	};

	// =========================================================================
	// RenderContextMetal
	// =========================================================================

	IRenderContext* CreateRenderContextMetal()
	{
		return new RenderContextMetal();
	}

	IRenderDevice* RenderContextMetal::CreateDevice(const RenderConfiguration& cfg)
	{
		@autoreleasepool
		{
			id<MTLDevice> mtlDevice = MTLCreateSystemDefaultDevice();
			if (!mtlDevice)
			{
				OUTPUT_LOG("ERROR: MTLCreateSystemDefaultDevice returned nil\n");
				return nullptr;
			}

			id<MTLCommandQueue> queue = [mtlDevice newCommandQueue];
			if (!queue)
			{
				[mtlDevice release];
				return nullptr;
			}

			CAMetalLayer* metalLayer = [[CAMetalLayer alloc] init];
			metalLayer.device = mtlDevice;
			metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
			metalLayer.framebufferOnly = NO;
			metalLayer.opaque = YES;

			uint32_t initW = cfg.screenWidth > 0 ? cfg.screenWidth : 1280;
			uint32_t initH = cfg.screenHeight > 0 ? cfg.screenHeight : 720;

			if (cfg.renderWindow)
			{
				intptr_t nativePtr = cfg.renderWindow->GetNativeHandle();
#if TARGET_OS_IPHONE
				UIView* view = reinterpret_cast<UIView*>(nativePtr);
				if (view)
				{
					CGFloat scale = [UIScreen mainScreen].scale;
					metalLayer.contentsScale = scale;
					metalLayer.frame = view.bounds;
					[view.layer addSublayer:metalLayer];
					initW = static_cast<uint32_t>((std::max)(1.0, view.bounds.size.width * scale));
					initH = static_cast<uint32_t>((std::max)(1.0, view.bounds.size.height * scale));
				}
#else
				NSObject* obj = reinterpret_cast<NSObject*>(nativePtr);
				NSView* targetView = nil;
				if ([obj isKindOfClass:[NSWindow class]])
				{
					NSWindow* win = static_cast<NSWindow*>(obj);
					targetView = [win contentView];
					if (!targetView)
					{
						targetView = [[[NSView alloc] initWithFrame:NSMakeRect(0, 0, initW, initH)] autorelease];
						[win setContentView:targetView];
					}
				}
				else if ([obj isKindOfClass:[NSView class]])
				{
					targetView = static_cast<NSView*>(obj);
				}
				if (targetView)
				{
					[targetView setWantsLayer:YES];
					NSScreen* scr = targetView.window ? targetView.window.screen : [NSScreen mainScreen];
					if (scr)
						metalLayer.contentsScale = scr.backingScaleFactor;
					[targetView setLayer:metalLayer];
				}
#endif
			}

			metalLayer.drawableSize = CGSizeMake(initW, initH);

			auto* rhiDev = new RenderDeviceMetal(mtlDevice, queue, metalLayer);
			if (!rhiDev->Initialize(initW, initH, true))
			{
				delete rhiDev;
				return nullptr;
			}

			[metalLayer release];
			[queue release];
			[mtlDevice release];
			return rhiDev;
		}
	}

	bool RenderContextMetal::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
	{
		if (!device)
			return false;
		auto* mtlDev = static_cast<RenderDeviceMetal*>(device);
		return mtlDev->ResizeSwapChain(
			cfg.screenWidth > 0 ? cfg.screenWidth : 1u,
			cfg.screenHeight > 0 ? cfg.screenHeight : 1u);
	}

	// =========================================================================
	// RenderDeviceMetal Lifecycle & Resource Creation
	// =========================================================================

	RenderDeviceMetal::RenderDeviceMetal(id<MTLDevice> device, id<MTLCommandQueue> queue, CAMetalLayer* layer)
	{
		m_device = reinterpret_cast<void*>([device retain]);
		m_commandQueue = reinterpret_cast<void*>([queue retain]);
		m_metalLayer = reinterpret_cast<void*>([layer retain]);
		m_frameSemaphore = reinterpret_cast<void*>(dispatch_semaphore_create(FRAME_COUNT));

		const char* nm = [[device name] UTF8String];
		m_adapterName = nm ? nm : "Apple Metal GPU";
	}

	RenderDeviceMetal::~RenderDeviceMetal()
	{
		ExecuteAndSyncGPU();
		if (m_currentDrawable)
		{
			[reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) release];
			m_currentDrawable = nullptr;
		}

		delete m_defaultWhiteTex;
		m_defaultWhiteTex = nullptr;
		delete m_defaultCubeTex;
		m_defaultCubeTex = nullptr;

		for (auto& kv : m_psoCache)
			if (kv.second) [reinterpret_cast<id<MTLRenderPipelineState>>(kv.second) release];
		m_psoCache.clear();

		for (auto& kv : m_depthStencilCache)
			if (kv.second) [reinterpret_cast<id<MTLDepthStencilState>>(kv.second) release];
		m_depthStencilCache.clear();

		for (auto& kv : m_samplerCache)
			if (kv.second) [reinterpret_cast<id<MTLSamplerState>>(kv.second) release];
		m_samplerCache.clear();

		for (auto& kv : m_blitPsoCache)
			if (kv.second) [reinterpret_cast<id<MTLRenderPipelineState>>(kv.second) release];
		m_blitPsoCache.clear();
		if (m_blitSamplerNearest) [reinterpret_cast<id<MTLSamplerState>>(m_blitSamplerNearest) release];
		if (m_blitSamplerLinear) [reinterpret_cast<id<MTLSamplerState>>(m_blitSamplerLinear) release];
		if (m_blitLibrary) [reinterpret_cast<id<MTLLibrary>>(m_blitLibrary) release];

		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
		{
			for (auto& page : m_frames[f].uploadPages)
			{
				if (page.mtlBuffer)
					[reinterpret_cast<id<MTLBuffer>>(page.mtlBuffer) release];
			}
			m_frames[f].uploadPages.clear();
		}

		ReclaimCompletedResources(UINT64_MAX);

		if (m_defaultDepthTex) [reinterpret_cast<id<MTLTexture>>(m_defaultDepthTex) release];
		if (m_fallbackVertexBuf) [reinterpret_cast<id<MTLBuffer>>(m_fallbackVertexBuf) release];
		if (m_occlusionBuf) [reinterpret_cast<id<MTLBuffer>>(m_occlusionBuf) release];
		if (m_metalLayer) [reinterpret_cast<CAMetalLayer*>(m_metalLayer) release];
		if (m_commandQueue) [reinterpret_cast<id<MTLCommandQueue>>(m_commandQueue) release];
		if (m_device) [reinterpret_cast<id<MTLDevice>>(m_device) release];
		if (m_frameSemaphore) dispatch_release(reinterpret_cast<dispatch_semaphore_t>(m_frameSemaphore));
	}

	void RenderDeviceMetal::InitCaps()
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		m_caps = RHIDeviceCaps{};
		m_caps.backend = ERHIBackend::Metal;
		m_caps.backendName = "Metal";
		m_caps.adapterName = m_adapterName.c_str();
		m_caps.featureLevel = 200; // Metal 2.0+
		m_caps.maxSimultaneousRTs = 4;
		m_caps.maxTextureStages = 16;
		m_caps.maxTextureSize = 16384;
		m_caps.maxAnisotropy = 16;
		m_caps.maxVertexUniformVectors = 4096;
		m_caps.supportsFloatRenderTargets = true;
		m_caps.supportsDepthTextures = true;
		m_caps.supportsInstancing = true;
		m_caps.supportsOcclusionQuery = true;
		m_caps.supportsTwoSidedStencil = true;
		m_caps.supportsUserClipPlanes = false;
#if !TARGET_OS_IPHONE
		if (@available(macOS 11.0, *))
			m_caps.supportsBCCompression = [dev supportsBCTextureCompression];
		else
			m_caps.supportsBCCompression = true;
		m_caps.supportsASTCCompression = false;
#else
		if (@available(iOS 16.4, *))
			m_caps.supportsBCCompression = [dev supportsBCTextureCompression];
		else
			m_caps.supportsBCCompression = false;
		m_caps.supportsASTCCompression = true;
#endif
		m_caps.supportsETC2Compression = m_caps.supportsASTCCompression;
		m_caps.supportsAsyncReadback = true;
		m_caps.supportsRuntimeShaderCompile = true;
		m_caps.topLeftScreenOrigin = true;
		m_caps.clipSpaceZeroToOne = true;
		m_caps.needsHalfPixelOffset = false;
	}

	bool RenderDeviceMetal::Initialize(uint32_t width, uint32_t height, bool vsync)
	{
		m_backBufferWidth = (std::max)(width, 1u);
		m_backBufferHeight = (std::max)(height, 1u);
		m_activeRTWidth = m_backBufferWidth;
		m_activeRTHeight = m_backBufferHeight;

#if !TARGET_OS_IPHONE
		CAMetalLayer* layer = reinterpret_cast<CAMetalLayer*>(m_metalLayer);
		if (layer && [layer respondsToSelector:@selector(setDisplaySyncEnabled:)])
			layer.displaySyncEnabled = vsync ? YES : NO;
#else
		(void)vsync;
#endif

		InitCaps();
		m_state.ResetToDefaults();

		ParaViewport vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = m_backBufferWidth;
		vp.Height = m_backBufferHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		m_state.SetViewport(vp);

		if (!CreateDefaultResources(m_backBufferWidth, m_backBufferHeight))
			return false;

		for (uint32_t i = MAX_OCCLUSION_QUERIES; i > 0; --i)
			m_freeOcclusionSlots.push_back(i - 1u);

		OUTPUT_LOG("ParaRHI Metal initialized on adapter: %s (%ux%u)\n",
			m_adapterName.c_str(), m_backBufferWidth, m_backBufferHeight);
		return true;
	}

	bool RenderDeviceMetal::CreateDefaultResources(uint32_t width, uint32_t height)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev)
			return false;

		if (m_defaultDepthTex)
		{
			[reinterpret_cast<id<MTLTexture>>(m_defaultDepthTex) release];
			m_defaultDepthTex = nullptr;
		}

		MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
																							 width:(std::max)(width, 1u)
																							height:(std::max)(height, 1u)
																						 mipmapped:NO];
		depthDesc.storageMode = MTLStorageModePrivate;
		depthDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
		m_defaultDepthTex = reinterpret_cast<void*>([dev newTextureWithDescriptor:depthDesc]);

		if (!m_fallbackVertexBuf)
		{
			const float kFallbackVert[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_fallbackVertexBuf = reinterpret_cast<void*>([dev newBufferWithBytes:kFallbackVert length:sizeof(kFallbackVert) options:MTLResourceStorageModeShared]);
		}

		if (!m_occlusionBuf)
		{
			m_occlusionBuf = reinterpret_cast<void*>([dev newBufferWithLength:MAX_OCCLUSION_QUERIES * sizeof(uint64_t) options:MTLResourceStorageModeShared]);
			if (m_occlusionBuf)
				std::memset([reinterpret_cast<id<MTLBuffer>>(m_occlusionBuf) contents], 0, MAX_OCCLUSION_QUERIES * sizeof(uint64_t));
		}

		if (!m_defaultWhiteTex)
		{
			const uint32_t whitePx = 0xFFFFFFFFu;
			RHIMipData mip{ &whitePx, 4u, 4u };
			m_defaultWhiteTex = static_cast<MetalTexture*>(CreateTexture(1, 1, PixelFormat::A8R8G8B8, 1, ERHITextureUsage::ShaderResource, &mip, 1));
		}

		if (!m_defaultCubeTex)
		{
			const uint32_t whitePx = 0xFFFFFFFFu;
			RHIMipData faces[6] = {
				{ &whitePx, 4u, 4u }, { &whitePx, 4u, 4u }, { &whitePx, 4u, 4u },
				{ &whitePx, 4u, 4u }, { &whitePx, 4u, 4u }, { &whitePx, 4u, 4u }
			};
			m_defaultCubeTex = static_cast<MetalTexture*>(CreateTexture(1, 1, PixelFormat::A8R8G8B8, 1, ERHITextureUsage::ShaderResource | ERHITextureUsage::CubeMap, faces, 6));
		}

		return m_defaultDepthTex != nullptr && m_fallbackVertexBuf != nullptr && m_defaultWhiteTex != nullptr;
	}

	bool RenderDeviceMetal::ResizeSwapChain(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return true;
		if (width == m_backBufferWidth && height == m_backBufferHeight)
			return true;

		ExecuteAndSyncGPU();
		if (m_currentDrawable)
		{
			[reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) release];
			m_currentDrawable = nullptr;
		}
		m_backBufferWidth = width;
		m_backBufferHeight = height;
		if (m_usingBackBufferRT)
		{
			m_activeRTWidth = width;
			m_activeRTHeight = height;
		}
		CAMetalLayer* layer = reinterpret_cast<CAMetalLayer*>(m_metalLayer);
		if (layer)
			layer.drawableSize = CGSizeMake(width, height);

		return CreateDefaultResources(width, height);
	}

	void RenderDeviceMetal::DeferRelease(void* retainedObjcPtr)
	{
		if (!retainedObjcPtr)
			return;
		m_deferredReleases.push_back({ m_nextSubmitSerial, retainedObjcPtr });
	}

	void RenderDeviceMetal::ReclaimCompletedResources(uint64_t completedSerial)
	{
		size_t writeIdx = 0;
		for (size_t i = 0; i < m_deferredReleases.size(); ++i)
		{
			if (m_deferredReleases[i].retireSerial <= completedSerial)
			{
				if (m_deferredReleases[i].objcObject)
					[reinterpret_cast<id<NSObject>>(m_deferredReleases[i].objcObject) release];
			}
			else
			{
				m_deferredReleases[writeIdx++] = m_deferredReleases[i];
			}
		}
		m_deferredReleases.resize(writeIdx);
	}

	void RenderDeviceMetal::AcquireFrameSlotIfNeeded()
	{
		if (m_frameSlotAcquired || !m_frameSemaphore)
			return;
		dispatch_semaphore_wait(reinterpret_cast<dispatch_semaphore_t>(m_frameSemaphore), DISPATCH_TIME_FOREVER);
		m_frameSlotAcquired = true;
		FrameSlot& slot = m_frames[m_frameIndex];
		for (auto& page : slot.uploadPages)
			page.offset = 0;
		ReclaimCompletedResources(m_completedSerial);
	}

	void RenderDeviceMetal::ExecuteAndSyncGPU()
	{
		EndActiveRenderPass();
		if (m_activeCmdBuf)
		{
			id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
			uint64_t serial = m_nextSubmitSerial++;
			m_frames[m_frameIndex].submitSerial = serial;
			[cmd commit];
			[cmd waitUntilCompleted];
			[cmd release];
			m_activeCmdBuf = nullptr;
			m_completedSerial = serial;
		}
		for (auto& page : m_frames[m_frameIndex].uploadPages)
			page.offset = 0;
		if (m_frameSlotAcquired && m_frameSemaphore)
		{
			dispatch_semaphore_signal(reinterpret_cast<dispatch_semaphore_t>(m_frameSemaphore));
			m_frameSlotAcquired = false;
		}
		ReclaimCompletedResources(m_completedSerial);
	}

	bool RenderDeviceMetal::AllocateUpload(uint32_t byteSize, uint32_t alignment, void*& outCpuPtr, void*& outMtlBuffer, uint32_t& outOffset)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || byteSize == 0)
			return false;

		AcquireFrameSlotIfNeeded();
		uint32_t align = (std::max)(alignment, 256u);
		FrameSlot& slot = m_frames[m_frameIndex];
		for (auto& page : slot.uploadPages)
		{
			uint32_t alignedOff = AlignUpMtl(page.offset, align);
			if (alignedOff + byteSize <= page.capacity)
			{
				outCpuPtr = page.cpuBase + alignedOff;
				outMtlBuffer = page.mtlBuffer;
				outOffset = alignedOff;
				page.offset = alignedOff + byteSize;
				return true;
			}
		}

		uint32_t newCap = (std::max)(AlignUpMtl(byteSize, 65536u), 1024u * 1024u);
		id<MTLBuffer> buf = [dev newBufferWithLength:newCap options:MTLResourceStorageModeShared];
		if (!buf)
			return false;

		UploadPage page;
		page.mtlBuffer = reinterpret_cast<void*>(buf);
		page.cpuBase = static_cast<uint8_t*>([buf contents]);
		page.capacity = newCap;
		page.offset = byteSize;
		slot.uploadPages.push_back(page);

		outCpuPtr = page.cpuBase;
		outMtlBuffer = page.mtlBuffer;
		outOffset = 0;
		return true;
	}

	uint32_t RenderDeviceMetal::AllocateOcclusionSlot()
	{
		if (m_freeOcclusionSlots.empty())
			return UINT32_MAX;
		uint32_t s = m_freeOcclusionSlots.back();
		m_freeOcclusionSlots.pop_back();
		return s;
	}

	void RenderDeviceMetal::FreeOcclusionSlot(uint32_t slot)
	{
		if (slot < MAX_OCCLUSION_QUERIES)
			m_freeOcclusionSlots.push_back(slot);
	}

	uint64_t RenderDeviceMetal::ReadOcclusionSlotValue(uint32_t slot) const
	{
		if (!m_occlusionBuf || slot >= MAX_OCCLUSION_QUERIES)
			return 1;
		const uint64_t* ptr = static_cast<const uint64_t*>([reinterpret_cast<id<MTLBuffer>>(m_occlusionBuf) contents]);
		return ptr ? ptr[slot] : 1;
	}

	void RenderDeviceMetal::BeginOcclusionOnActiveEncoder(uint32_t slot)
	{
		if (slot >= MAX_OCCLUSION_QUERIES || !EnsureRenderPassEncoder(false, false, false))
			return;
		id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
		[enc setVisibilityResultMode:MTLVisibilityResultModeCounting offset:slot * sizeof(uint64_t)];
	}

	void RenderDeviceMetal::EndOcclusionOnActiveEncoder()
	{
		if (!m_activeEncoder)
			return;
		id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
		[enc setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
	}

	bool RenderDeviceMetal::CheckDeviceError(const char* /*filename*/, const char* /*func*/, int /*nLine*/)
	{
		return m_device != nullptr;
	}

	IRHITexture* RenderDeviceMetal::CreateTexture(
		uint32_t width,
		uint32_t height,
		PixelFormat format,
		uint32_t mipLevels,
		ERHITextureUsage usage,
		const RHIMipData* initialMips,
		uint32_t initialMipCount)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || width == 0 || height == 0)
			return nullptr;

		const bool isCube = HasFlag(usage, ERHITextureUsage::CubeMap);
		const bool isDepth = HasFlag(usage, ERHITextureUsage::DepthStencil) || IsDepthPixelFormatMtl(format);
		const bool isRT = HasFlag(usage, ERHITextureUsage::RenderTarget);
		uint32_t mips = (std::max)(mipLevels, 1u);

		uint32_t bytesPerBlock = 4;
		bool isBC = false;
		bool expandedToRGBA8 = false;
		MTLPixelFormat mtlFmt = MTLPixelFormatRGBA8Unorm;

		if (isDepth)
		{
			mtlFmt = MapDepthFormatToMtl(format);
		}
		else if (RHISoftwareS3TC::NeedsRGBA8Expansion(format, false))
		{
			expandedToRGBA8 = true;
			mtlFmt = MTLPixelFormatRGBA8Unorm;
		}
		else
		{
			mtlFmt = MapColorFormatToMtl(format, m_caps.supportsBCCompression, bytesPerBlock, isBC);
		}

		MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
		desc.textureType = isCube ? MTLTextureTypeCube : MTLTextureType2D;
		desc.pixelFormat = mtlFmt;
		desc.width = width;
		desc.height = height;
		desc.mipmapLevelCount = mips;
		desc.storageMode = (isDepth || isRT) ? MTLStorageModePrivate : MTLStorageModeShared;
		desc.usage = MTLTextureUsageShaderRead;
		if (isRT || isDepth)
			desc.usage |= MTLTextureUsageRenderTarget;

		id<MTLTexture> tex = [dev newTextureWithDescriptor:desc];
		[desc release];
		if (!tex)
			return nullptr;

		auto* rhiTex = new MetalTexture(this, width, height, mips, format, mtlFmt, isCube, isBC, expandedToRGBA8, bytesPerBlock, tex);

		if (initialMips && initialMipCount > 0 && !isDepth && !isRT)
		{
			uint32_t slices = isCube ? 6u : 1u;
			uint32_t idx = 0;
			for (uint32_t s = 0; s < slices; ++s)
			{
				uint32_t mw = width;
				uint32_t mh = height;
				for (uint32_t m = 0; m < mips && idx < initialMipCount; ++m, ++idx)
				{
					if (initialMips[idx].data)
						rhiTex->UpdateSubresource(m, s, 0, 0, mw, mh, initialMips[idx].data, initialMips[idx].rowPitch);
					mw = (std::max)(1u, mw >> 1);
					mh = (std::max)(1u, mh >> 1);
				}
			}
		}

		return rhiTex;
	}

	IRHIBuffer* RenderDeviceMetal::CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage, const void* initialData)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || byteSize == 0)
			return nullptr;

		NSUInteger allocLen = (std::max)(byteSize, 16u);
		id<MTLBuffer> buf = nil;
		if (initialData)
		{
			buf = [dev newBufferWithLength:allocLen options:MTLResourceStorageModeShared];
			if (buf)
				std::memcpy([buf contents], initialData, byteSize);
		}
		else
		{
			buf = [dev newBufferWithLength:allocLen options:MTLResourceStorageModeShared];
		}
		if (!buf)
			return nullptr;

		return new MetalBuffer(this, dev, byteSize, isIndexBuffer, usage, buf);
	}

	IRHIRenderTarget* RenderDeviceMetal::CreateRenderTarget(
		uint32_t width,
		uint32_t height,
		PixelFormat colorFormat,
		PixelFormat depthFormat,
		uint32_t colorTargetCount,
		uint32_t msaaSamples)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || width == 0 || height == 0)
			return nullptr;

		uint32_t rtCount = (std::min)((std::max)(colorTargetCount, 1u), 4u);
		uint32_t samples = (msaaSamples > 1 && [dev supportsTextureSampleCount:msaaSamples]) ? msaaSamples : 1u;

		std::vector<MetalTexture*> colors;
		for (uint32_t i = 0; i < rtCount; ++i)
		{
			uint32_t bpb = 4;
			bool isBC = false;
			MTLPixelFormat mtlFmt = MapColorFormatToMtl(colorFormat, false, bpb, isBC);

			MTLTextureDescriptor* cd = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:mtlFmt width:width height:height mipmapped:NO];
			cd.storageMode = MTLStorageModePrivate;
			cd.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
			id<MTLTexture> resolvedTex = [dev newTextureWithDescriptor:cd];
			if (!resolvedTex)
			{
				for (MetalTexture* c : colors) delete c;
				return nullptr;
			}

			id<MTLTexture> msaaTex = nil;
			if (samples > 1)
			{
				MTLTextureDescriptor* md = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:mtlFmt width:width height:height mipmapped:NO];
				md.textureType = MTLTextureType2DMultisample;
				md.sampleCount = samples;
				md.storageMode = MTLStorageModePrivate;
				md.usage = MTLTextureUsageRenderTarget;
				msaaTex = [dev newTextureWithDescriptor:md];
			}

			colors.push_back(new MetalTexture(this, width, height, 1, colorFormat, mtlFmt, false, false, false, bpb, resolvedTex, msaaTex));
		}

		MetalTexture* depthRhi = nullptr;
		if (depthFormat != PixelFormat::Unknown)
		{
			MTLPixelFormat df = MapDepthFormatToMtl(depthFormat);
			MTLTextureDescriptor* dd = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:df width:width height:height mipmapped:NO];
			dd.storageMode = MTLStorageModePrivate;
			dd.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
			if (samples > 1)
			{
				dd.textureType = MTLTextureType2DMultisample;
				dd.sampleCount = samples;
			}
			id<MTLTexture> dt = [dev newTextureWithDescriptor:dd];
			if (dt)
				depthRhi = new MetalTexture(this, width, height, 1, depthFormat, df, false, false, false, 4, dt);
		}

		return new MetalRenderTarget(width, height, colorFormat, colors, depthRhi);
	}

	IRHIOcclusionQuery* RenderDeviceMetal::CreateOcclusionQuery()
	{
		return new MetalOcclusionQuery(this);
	}

	// =========================================================================
	// Render Targets, Encoders, Shaders & Pipeline State Cache
	// =========================================================================

	void RenderDeviceMetal::EndActiveRenderPass()
	{
		if (m_activeEncoder)
		{
			id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
			[enc endEncoding];
			[enc release];
			m_activeEncoder = nullptr;
		}
	}

	bool RenderDeviceMetal::EnsureCommandBuffer()
	{
		if (m_activeCmdBuf)
			return true;
		id<MTLCommandQueue> queue = reinterpret_cast<id<MTLCommandQueue>>(m_commandQueue);
		if (!queue)
			return false;
		id<MTLCommandBuffer> cmd = [[queue commandBuffer] retain];
		m_activeCmdBuf = reinterpret_cast<void*>(cmd);
		return m_activeCmdBuf != nullptr;
	}

	bool RenderDeviceMetal::EnsureDrawableAcquired()
	{
		if (m_currentDrawable)
			return true;
		if (m_importedColorTex)
			return true;
		CAMetalLayer* layer = reinterpret_cast<CAMetalLayer*>(m_metalLayer);
		if (!layer)
			return false;
		id<CAMetalDrawable> drawable = [[layer nextDrawable] retain];
		m_currentDrawable = reinterpret_cast<void*>(drawable);
		return m_currentDrawable != nullptr;
	}

	bool RenderDeviceMetal::EnsureRenderPassEncoder(bool clearColor, bool clearDepth, bool clearStencil)
	{
		if (m_activeEncoder && !clearColor && !clearDepth && !clearStencil)
			return true;

		EndActiveRenderPass();
		if (!EnsureCommandBuffer())
			return false;

		MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
		const Color4f& cc = m_state.GetClearColor();
		const float cd = m_state.GetClearDepth();
		const uint32_t cs = static_cast<uint32_t>(m_state.GetClearStencil() & 0xFF);

		if (m_usingBackBufferRT)
		{
			if (!EnsureDrawableAcquired())
				return false;

			id<MTLTexture> colorTex = m_importedColorTex
				? reinterpret_cast<id<MTLTexture>>(m_importedColorTex)
				: [reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) texture];
			if (!colorTex)
				return false;

			rp.colorAttachments[0].texture = colorTex;
			rp.colorAttachments[0].loadAction = clearColor ? MTLLoadActionClear : MTLLoadActionLoad;
			rp.colorAttachments[0].storeAction = MTLStoreActionStore;
			rp.colorAttachments[0].clearColor = MTLClearColorMake(cc.r, cc.g, cc.b, cc.a);

			id<MTLTexture> depthTex = reinterpret_cast<id<MTLTexture>>(m_defaultDepthTex);
			if (depthTex)
			{
				rp.depthAttachment.texture = depthTex;
				rp.depthAttachment.loadAction = clearDepth ? MTLLoadActionClear : MTLLoadActionLoad;
				rp.depthAttachment.storeAction = MTLStoreActionStore;
				rp.depthAttachment.clearDepth = cd;

				rp.stencilAttachment.texture = depthTex;
				rp.stencilAttachment.loadAction = clearStencil ? MTLLoadActionClear : MTLLoadActionLoad;
				rp.stencilAttachment.storeAction = MTLStoreActionStore;
				rp.stencilAttachment.clearStencil = cs;
			}
		}
		else
		{
			for (uint32_t i = 0; i < m_activeColorTargetCount; ++i)
			{
				MetalTexture* ct = m_activeColorTargets[i];
				if (!ct) continue;
				id<MTLTexture> msaa = ct->GetMSAATexture();
				if (msaa)
				{
					rp.colorAttachments[i].texture = msaa;
					rp.colorAttachments[i].resolveTexture = ct->GetMTLTexture();
					rp.colorAttachments[i].storeAction = MTLStoreActionStoreAndMultisampleResolve;
				}
				else
				{
					rp.colorAttachments[i].texture = ct->GetMTLTexture();
					rp.colorAttachments[i].storeAction = MTLStoreActionStore;
				}
				rp.colorAttachments[i].loadAction = clearColor ? MTLLoadActionClear : MTLLoadActionLoad;
				rp.colorAttachments[i].clearColor = MTLClearColorMake(cc.r, cc.g, cc.b, cc.a);
			}

			if (m_activeDepthTarget && m_activeDepthTarget->GetMTLTexture())
			{
				id<MTLTexture> dt = m_activeDepthTarget->GetMTLTexture();
				rp.depthAttachment.texture = dt;
				rp.depthAttachment.loadAction = clearDepth ? MTLLoadActionClear : MTLLoadActionLoad;
				rp.depthAttachment.storeAction = MTLStoreActionStore;
				rp.depthAttachment.clearDepth = cd;

				if (m_activeDepthTarget->GetMTLPixelFormat() == MTLPixelFormatDepth32Float_Stencil8)
				{
					rp.stencilAttachment.texture = dt;
					rp.stencilAttachment.loadAction = clearStencil ? MTLLoadActionClear : MTLLoadActionLoad;
					rp.stencilAttachment.storeAction = MTLStoreActionStore;
					rp.stencilAttachment.clearStencil = cs;
				}
			}
		}

		if (m_occlusionBuf)
			rp.visibilityResultBuffer = reinterpret_cast<id<MTLBuffer>>(m_occlusionBuf);

		id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
		id<MTLRenderCommandEncoder> enc = [[cmd renderCommandEncoderWithDescriptor:rp] retain];
		m_activeEncoder = reinterpret_cast<void*>(enc);
		m_vsConstDirty = true;
		m_psConstDirty = true;
		return m_activeEncoder != nullptr;
	}

	bool RenderDeviceMetal::SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride)
	{
		EndActiveRenderPass();
		NotifyRenderTargetsBound(count, targets, depthStencilOverride);

		std::memset(m_activeColorTargets, 0, sizeof(m_activeColorTargets));
		m_activeColorTargetCount = 0;
		m_activeDepthTarget = nullptr;

		if (!targets || count == 0 || !targets[0])
		{
			m_usingBackBufferRT = true;
			m_activeRTWidth = m_backBufferWidth;
			m_activeRTHeight = m_backBufferHeight;
			ParaViewport vp{ 0, 0, m_backBufferWidth, m_backBufferHeight, 0.0f, 1.0f };
			m_state.SetViewport(vp);
			return true;
		}

		m_usingBackBufferRT = false;
		uint32_t clamped = (std::min)(count, 4u);
		if (clamped == 1)
		{
			auto* rt0 = static_cast<MetalRenderTarget*>(targets[0]->GetNativeHandle());
			if (rt0)
			{
				m_activeColorTargetCount = (std::min)(rt0->GetColorCount(), 4u);
				for (uint32_t i = 0; i < m_activeColorTargetCount; ++i)
					m_activeColorTargets[i] = rt0->GetMetalColor(i);
				m_activeDepthTarget = rt0->GetMetalDepth();
				m_activeRTWidth = rt0->GetWidth();
				m_activeRTHeight = rt0->GetHeight();
			}
		}
		else
		{
			for (uint32_t i = 0; i < clamped; ++i)
			{
				if (!targets[i]) continue;
				auto* rti = static_cast<MetalRenderTarget*>(targets[i]->GetNativeHandle());
				if (!rti) continue;
				m_activeColorTargets[i] = rti->GetMetalColor(0);
				m_activeColorTargetCount = i + 1u;
				if (i == 0)
				{
					m_activeDepthTarget = rti->GetMetalDepth();
					m_activeRTWidth = rti->GetWidth();
					m_activeRTHeight = rti->GetHeight();
				}
			}
		}

		if (depthStencilOverride)
		{
			auto* dsRt = static_cast<MetalRenderTarget*>(depthStencilOverride->GetNativeHandle());
			if (dsRt && dsRt->GetMetalDepth())
				m_activeDepthTarget = dsRt->GetMetalDepth();
		}

		ParaViewport vp{ 0, 0, m_activeRTWidth, m_activeRTHeight, 0.0f, 1.0f };
		m_state.SetViewport(vp);
		return true;
	}

	void RenderDeviceMetal::BeginRenderTarget(uint32_t width, uint32_t height)
	{
		ParaViewport vp{ 0, 0, (std::max)(width, 1u), (std::max)(height, 1u), 0.0f, 1.0f };
		m_state.SetViewport(vp);
	}

	void RenderDeviceMetal::EndRenderTarget()
	{
		EndActiveRenderPass();
	}

	bool RenderDeviceMetal::BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect, const RECT* dstRect, bool linearFilter)
	{
		if (src == dst)
			return false;

		EndActiveRenderPass();
		if (!EnsureCommandBuffer())
			return false;

		id<MTLTexture> srcTex = nil;
		id<MTLTexture> dstTex = nil;
		id<MTLTexture> dstMsaaTex = nil;
		if (src)
		{
			auto* srt = static_cast<MetalRenderTarget*>(src->GetNativeHandle());
			if (srt && srt->GetMetalColor(0)) srcTex = srt->GetMetalColor(0)->GetMTLTexture();
		}
		else if (m_importedColorTex)
		{
			srcTex = reinterpret_cast<id<MTLTexture>>(m_importedColorTex);
		}
		else if (EnsureDrawableAcquired())
		{
			srcTex = [reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) texture];
		}

		if (dst)
		{
			auto* drt = static_cast<MetalRenderTarget*>(dst->GetNativeHandle());
			if (drt && drt->GetMetalColor(0))
			{
				dstTex = drt->GetMetalColor(0)->GetMTLTexture();
				dstMsaaTex = drt->GetMetalColor(0)->GetMSAATexture();
			}
		}
		else if (m_importedColorTex)
		{
			dstTex = reinterpret_cast<id<MTLTexture>>(m_importedColorTex);
		}
		else if (EnsureDrawableAcquired())
		{
			dstTex = [reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) texture];
		}

		if (!srcTex || !dstTex || srcTex == dstTex)
			return false;

		const uint32_t srcW = static_cast<uint32_t>([srcTex width]);
		const uint32_t srcH = static_cast<uint32_t>([srcTex height]);
		const uint32_t dstW = static_cast<uint32_t>([dstTex width]);
		const uint32_t dstH = static_cast<uint32_t>([dstTex height]);
		if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
			return false;

		const int sx0 = srcRect ? (std::max)(0, (std::min)(static_cast<int>(srcRect->left), static_cast<int>(srcW))) : 0;
		const int sy0 = srcRect ? (std::max)(0, (std::min)(static_cast<int>(srcRect->top), static_cast<int>(srcH))) : 0;
		const int sx1 = srcRect ? (std::max)(sx0, (std::min)(static_cast<int>(srcRect->right), static_cast<int>(srcW))) : static_cast<int>(srcW);
		const int sy1 = srcRect ? (std::max)(sy0, (std::min)(static_cast<int>(srcRect->bottom), static_cast<int>(srcH))) : static_cast<int>(srcH);

		const int dx0 = dstRect ? (std::max)(0, (std::min)(static_cast<int>(dstRect->left), static_cast<int>(dstW))) : 0;
		const int dy0 = dstRect ? (std::max)(0, (std::min)(static_cast<int>(dstRect->top), static_cast<int>(dstH))) : 0;
		const int dx1 = dstRect ? (std::max)(dx0, (std::min)(static_cast<int>(dstRect->right), static_cast<int>(dstW))) : static_cast<int>(dstW);
		const int dy1 = dstRect ? (std::max)(dy0, (std::min)(static_cast<int>(dstRect->bottom), static_cast<int>(dstH))) : static_cast<int>(dstH);

		const uint32_t sw = static_cast<uint32_t>(sx1 - sx0);
		const uint32_t sh = static_cast<uint32_t>(sy1 - sy0);
		const uint32_t dw = static_cast<uint32_t>(dx1 - dx0);
		const uint32_t dh = static_cast<uint32_t>(dy1 - dy0);
		if (sw == 0 || sh == 0 || dw == 0 || dh == 0)
			return false;

		const MTLPixelFormat srcFmt = [srcTex pixelFormat];
		const MTLPixelFormat dstFmt = [dstTex pixelFormat];

		// Fast path: raw texture copy only when pixel formats AND region dimensions match identically.
		// Note: copyFromTexture between RGBA8Unorm and BGRA8Unorm copies raw bytes without swizzling R<->B!
		if (srcFmt == dstFmt && sw == dw && sh == dh && !dstMsaaTex)
		{
			id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
			id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
			[blit copyFromTexture:srcTex
					  sourceSlice:0
					  sourceLevel:0
					 sourceOrigin:MTLOriginMake(static_cast<NSUInteger>(sx0), static_cast<NSUInteger>(sy0), 0)
					   sourceSize:MTLSizeMake(sw, sh, 1)
						toTexture:dstTex
				 destinationSlice:0
				 destinationLevel:0
				destinationOrigin:MTLOriginMake(static_cast<NSUInteger>(dx0), static_cast<NSUInteger>(dy0), 0)];
			[blit endEncoding];
			return true;
		}

		// Shader blit path: handles RGBA8Unorm <-> BGRA8Unorm format conversion, scaling, and filtering.
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev)
			return false;

		@autoreleasepool
		{
			if (!m_blitLibrary)
			{
				static NSString* const kBlitMsl =
					@"#include <metal_stdlib>\n"
					 "using namespace metal;\n"
					 "struct BlitVsOut {\n"
					 "    float4 pos [[position]];\n"
					 "    float2 uv;\n"
					 "};\n"
					 "vertex BlitVsOut para_blit_vs(uint vid [[vertex_id]], constant float4& uvRect [[buffer(0)]]) {\n"
					 "    BlitVsOut o;\n"
					 "    float x = float((vid << 1u) & 2u);\n"
					 "    float y = float(vid & 2u);\n"
					 "    o.pos = float4(x * 2.0f - 1.0f, 1.0f - y * 2.0f, 0.0f, 1.0f);\n"
					 "    o.uv  = uvRect.xy + float2(x, y) * uvRect.zw;\n"
					 "    return o;\n"
					 "}\n"
					 "fragment float4 para_blit_fs(BlitVsOut in [[stage_in]], texture2d<float> tex [[texture(0)]], sampler smp [[sampler(0)]]) {\n"
					 "    return tex.sample(smp, in.uv);\n"
					 "}\n";
				NSError* err = nil;
				id<MTLLibrary> lib = [dev newLibraryWithSource:kBlitMsl options:nil error:&err];
				if (!lib)
				{
					OUTPUT_LOG("ERROR: Metal BlitRenderTarget shader compile failed: %s\n", err ? [[err localizedDescription] UTF8String] : "unknown");
					return false;
				}
				m_blitLibrary = reinterpret_cast<void*>(lib);
			}

			if (!m_blitSamplerNearest || !m_blitSamplerLinear)
			{
				MTLSamplerDescriptor* sd = [[[MTLSamplerDescriptor alloc] init] autorelease];
				sd.sAddressMode = MTLSamplerAddressModeClampToEdge;
				sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
				sd.mipFilter = MTLSamplerMipFilterNotMipmapped;
				if (!m_blitSamplerNearest)
				{
					sd.minFilter = MTLSamplerMinMagFilterNearest;
					sd.magFilter = MTLSamplerMinMagFilterNearest;
					m_blitSamplerNearest = reinterpret_cast<void*>([dev newSamplerStateWithDescriptor:sd]);
				}
				if (!m_blitSamplerLinear)
				{
					sd.minFilter = MTLSamplerMinMagFilterLinear;
					sd.magFilter = MTLSamplerMinMagFilterLinear;
					m_blitSamplerLinear = reinterpret_cast<void*>([dev newSamplerStateWithDescriptor:sd]);
				}
			}

			const NSUInteger sampleCount = dstMsaaTex ? [dstMsaaTex sampleCount] : 1u;
			const uint64_t psoKey = (static_cast<uint64_t>(dstFmt) << 8) | static_cast<uint64_t>(sampleCount);
			id<MTLRenderPipelineState> blitPso = nil;
			auto it = m_blitPsoCache.find(psoKey);
			if (it != m_blitPsoCache.end())
			{
				blitPso = reinterpret_cast<id<MTLRenderPipelineState>>(it->second);
			}
			else
			{
				id<MTLLibrary> lib = reinterpret_cast<id<MTLLibrary>>(m_blitLibrary);
				id<MTLFunction> vsFn = [[lib newFunctionWithName:@"para_blit_vs"] autorelease];
				id<MTLFunction> fsFn = [[lib newFunctionWithName:@"para_blit_fs"] autorelease];
				if (!vsFn || !fsFn)
					return false;

				MTLRenderPipelineDescriptor* pd = [[[MTLRenderPipelineDescriptor alloc] init] autorelease];
				pd.vertexFunction = vsFn;
				pd.fragmentFunction = fsFn;
				pd.rasterSampleCount = sampleCount;
				pd.colorAttachments[0].pixelFormat = dstFmt;
				pd.colorAttachments[0].blendingEnabled = NO;

				NSError* err = nil;
				blitPso = [dev newRenderPipelineStateWithDescriptor:pd error:&err];
				if (!blitPso)
				{
					OUTPUT_LOG("ERROR: Metal BlitRenderTarget PSO failed: %s\n", err ? [[err localizedDescription] UTF8String] : "unknown");
					return false;
				}
				m_blitPsoCache[psoKey] = reinterpret_cast<void*>(blitPso);
			}

			const bool fullDst = (dx0 == 0 && dy0 == 0 && dw == dstW && dh == dstH);
			MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
			if (dstMsaaTex)
			{
				rp.colorAttachments[0].texture = dstMsaaTex;
				rp.colorAttachments[0].resolveTexture = dstTex;
				rp.colorAttachments[0].storeAction = MTLStoreActionStoreAndMultisampleResolve;
			}
			else
			{
				rp.colorAttachments[0].texture = dstTex;
				rp.colorAttachments[0].storeAction = MTLStoreActionStore;
			}
			rp.colorAttachments[0].loadAction = fullDst ? MTLLoadActionDontCare : MTLLoadActionLoad;

			id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
			id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:rp];
			if (!enc)
				return false;

			MTLViewport vp;
			vp.originX = static_cast<double>(dx0);
			vp.originY = static_cast<double>(dy0);
			vp.width = static_cast<double>(dw);
			vp.height = static_cast<double>(dh);
			vp.znear = 0.0;
			vp.zfar = 1.0;
			[enc setViewport:vp];

			MTLScissorRect sc;
			sc.x = static_cast<NSUInteger>(dx0);
			sc.y = static_cast<NSUInteger>(dy0);
			sc.width = static_cast<NSUInteger>(dw);
			sc.height = static_cast<NSUInteger>(dh);
			[enc setScissorRect:sc];

			const float uvRect[4] = {
				static_cast<float>(sx0) / static_cast<float>(srcW),
				static_cast<float>(sy0) / static_cast<float>(srcH),
				static_cast<float>(sw) / static_cast<float>(srcW),
				static_cast<float>(sh) / static_cast<float>(srcH)
			};

			id<MTLSamplerState> smp = reinterpret_cast<id<MTLSamplerState>>(linearFilter ? m_blitSamplerLinear : m_blitSamplerNearest);
			[enc setRenderPipelineState:blitPso];
			[enc setCullMode:MTLCullModeNone];
			[enc setVertexBytes:uvRect length:sizeof(uvRect) atIndex:0];
			[enc setFragmentTexture:srcTex atIndex:0];
			if (smp)
				[enc setFragmentSamplerState:smp atIndex:0];
			[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
			[enc endEncoding];
			return true;
		}
	}

	bool RenderDeviceMetal::ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height)
	{
		EndActiveRenderPass();
		m_importedColorTex = reinterpret_cast<void*>(nativeFboOrTexHandle);
		if (width > 0 && height > 0)
		{
			m_backBufferWidth = width;
			m_backBufferHeight = height;
			if (m_usingBackBufferRT)
			{
				m_activeRTWidth = width;
				m_activeRTHeight = height;
			}
		}
		return true;
	}

	IRHIShaderProgram* RenderDeviceMetal::CompileShaderProgram(const RHIShaderProgramDesc& desc)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || !desc.vsBytecode || desc.vsBytecodeSize == 0)
			return nullptr;

		@autoreleasepool
		{
			MTLCompileOptions* opts = [[[MTLCompileOptions alloc] init] autorelease];
			opts.languageVersion = MTLLanguageVersion2_0;

			std::string vsText(static_cast<const char*>(desc.vsBytecode), desc.vsBytecodeSize);
			while (!vsText.empty() && vsText.back() == '\0')
				vsText.pop_back();

			NSError* vsErr = nil;
			NSString* nsVsSrc = [NSString stringWithUTF8String:vsText.c_str()];
			id<MTLLibrary> vsLib = [dev newLibraryWithSource:nsVsSrc options:opts error:&vsErr];
			if (!vsLib)
			{
				OUTPUT_LOG("ERROR: Metal VS compile failed (%s): %s\n",
					desc.debugName ? desc.debugName : "unnamed",
					vsErr ? [[vsErr localizedDescription] UTF8String] : "unknown");
				return nullptr;
			}

			NSString* vsEntry = [NSString stringWithUTF8String:(desc.vsEntryPoint && desc.vsEntryPoint[0]) ? desc.vsEntryPoint : "vs_main"];
			id<MTLFunction> vsFn = [vsLib newFunctionWithName:vsEntry];
			[vsLib release];
			if (!vsFn)
				return nullptr;

			id<MTLFunction> psFn = nil;
			uint64_t hash = ComputeMtlByteHash64(desc.vsBytecode, desc.vsBytecodeSize);
			if (desc.psBytecode && desc.psBytecodeSize > 0)
			{
				HashCombineMtlU64(hash, ComputeMtlByteHash64(desc.psBytecode, desc.psBytecodeSize));
				std::string psText(static_cast<const char*>(desc.psBytecode), desc.psBytecodeSize);
				while (!psText.empty() && psText.back() == '\0')
					psText.pop_back();

				NSError* psErr = nil;
				NSString* nsPsSrc = [NSString stringWithUTF8String:psText.c_str()];
				id<MTLLibrary> psLib = [dev newLibraryWithSource:nsPsSrc options:opts error:&psErr];
				if (!psLib)
				{
					OUTPUT_LOG("ERROR: Metal PS compile failed (%s): %s\n",
						desc.debugName ? desc.debugName : "unnamed",
						psErr ? [[psErr localizedDescription] UTF8String] : "unknown");
					[vsFn release];
					return nullptr;
				}
				NSString* psEntry = [NSString stringWithUTF8String:(desc.psEntryPoint && desc.psEntryPoint[0]) ? desc.psEntryPoint : "fs_main"];
				psFn = [psLib newFunctionWithName:psEntry];
				[psLib release];
			}

			return new MetalShaderProgram(this, vsFn, psFn, hash, desc.vsGlobalCBufferSize, desc.psGlobalCBufferSize);
		}
	}

	bool RenderDeviceMetal::BindShaderProgram(IRHIShaderProgram* program)
	{
		m_currentProgram = program ? static_cast<MetalShaderProgram*>(program->GetNativeHandle()) : nullptr;
		m_vsConstDirty = true;
		m_psConstDirty = true;
		return true;
	}

	bool RenderDeviceMetal::SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize)
	{
		if (vsConstants && vsByteSize > 0)
		{
			uint32_t aligned = AlignUpMtl(vsByteSize, 16u);
			if (m_vsConstShadow.size() < aligned)
				m_vsConstShadow.resize(aligned, 0);
			std::memcpy(m_vsConstShadow.data(), vsConstants, vsByteSize);
			m_vsConstSize = aligned;
			m_vsConstDirty = true;
		}
		if (psConstants && psByteSize > 0)
		{
			uint32_t aligned = AlignUpMtl(psByteSize, 16u);
			if (m_psConstShadow.size() < aligned)
				m_psConstShadow.resize(aligned, 0);
			std::memcpy(m_psConstShadow.data(), psConstants, psByteSize);
			m_psConstSize = aligned;
			m_psConstDirty = true;
		}
		return true;
	}

	// =========================================================================
	// PSO / DepthStencil / Sampler Caches & Draw Submission
	// =========================================================================

	void* RenderDeviceMetal::GetOrCreatePipelineState(EPrimitiveType primitiveType)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || !m_currentProgram || !m_currentProgram->IsValid())
			return nullptr;

		uint64_t rtFmtHash = 14695981039346656037ULL;
		NSUInteger sampleCount = 1;
		if (m_usingBackBufferRT)
		{
			HashCombineMtlU64(rtFmtHash, static_cast<uint64_t>(MTLPixelFormatBGRA8Unorm));
			HashCombineMtlU64(rtFmtHash, static_cast<uint64_t>(MTLPixelFormatDepth32Float_Stencil8));
		}
		else
		{
			for (uint32_t i = 0; i < m_activeColorTargetCount; ++i)
			{
				MTLPixelFormat cf = m_activeColorTargets[i] ? m_activeColorTargets[i]->GetMTLPixelFormat() : MTLPixelFormatInvalid;
				HashCombineMtlU64(rtFmtHash, static_cast<uint64_t>(cf));
				if (i == 0 && m_activeColorTargets[0] && m_activeColorTargets[0]->GetMSAATexture())
					sampleCount = [m_activeColorTargets[0]->GetMSAATexture() sampleCount];
			}
			MTLPixelFormat df = m_activeDepthTarget ? m_activeDepthTarget->GetMTLPixelFormat() : MTLPixelFormatInvalid;
			HashCombineMtlU64(rtFmtHash, static_cast<uint64_t>(df));
		}
		HashCombineMtlU64(rtFmtHash, static_cast<uint64_t>(sampleCount));

		uint64_t vdHash = m_currentVertexDecl ? m_currentVertexDecl->GetHash() : 0ULL;
		for (uint32_t s = 0; s < MAX_VERTEX_STREAMS; ++s)
		{
			HashCombineMtlU64(rtFmtHash, m_streams[s].stride);
			HashCombineMtlU64(rtFmtHash, m_streams[s].frequencyDivider);
		}

		uint64_t psoKey = m_state.ComputePSOHash(m_currentProgram->GetHash(), vdHash, primitiveType, rtFmtHash);
		auto it = m_psoCache.find(psoKey);
		if (it != m_psoCache.end())
			return it->second;

		@autoreleasepool
		{
			MTLRenderPipelineDescriptor* pd = [[[MTLRenderPipelineDescriptor alloc] init] autorelease];
			pd.vertexFunction = m_currentProgram->GetVSFunction();
			pd.fragmentFunction = m_currentProgram->GetPSFunction();
			pd.sampleCount = sampleCount;

			MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
			bool locAssigned[16] = {};
			bool streamUsed[MAX_VERTEX_STREAMS] = {};

			if (m_currentVertexDecl)
			{
				for (const VertexElement& el : m_currentVertexDecl->GetElements())
				{
					if (el.Stream >= MAX_VERTEX_STREAMS)
						continue;
					int32_t loc = MapDeclUsageToLocationMtl(el.Usage, el.UsageIndex);
					if (loc < 0 || loc >= 16 || locAssigned[loc])
						continue;

					vd.attributes[loc].format = MapDeclTypeToMtlVertexFormat(el.Type);
					vd.attributes[loc].offset = el.Offset;
					vd.attributes[loc].bufferIndex = el.Stream;
					locAssigned[loc] = true;
					streamUsed[el.Stream] = true;
				}
			}

			for (uint32_t s = 0; s < MAX_VERTEX_STREAMS; ++s)
			{
				if (!streamUsed[s])
					continue;
				uint32_t stride = m_streams[s].stride;
				if (stride == 0 && m_currentVertexDecl)
					stride = m_currentVertexDecl->GetStride(s);
				if (stride == 0)
					stride = 32;
				vd.layouts[s].stride = stride;
				if ((m_streams[s].frequencyDivider & 0x80000000u) != 0)
				{
					uint32_t rate = (std::max)(1u, m_streams[s].frequencyDivider & 0x7FFFFFFFu);
					vd.layouts[s].stepFunction = MTLVertexStepFunctionPerInstance;
					vd.layouts[s].stepRate = rate;
				}
				else
				{
					vd.layouts[s].stepFunction = MTLVertexStepFunctionPerVertex;
					vd.layouts[s].stepRate = 1;
				}
			}

			// Map any unbound shader attributes to constant fallback vertex buffer slot 15 ([0,0,0,1])
			bool anyFallback = false;
			for (uint32_t loc = 0; loc < 16; ++loc)
			{
				if (!locAssigned[loc])
				{
					vd.attributes[loc].format = MTLVertexFormatFloat4;
					vd.attributes[loc].offset = 0;
					vd.attributes[loc].bufferIndex = FALLBACK_VB_SLOT;
					anyFallback = true;
				}
			}
			if (anyFallback)
			{
				vd.layouts[FALLBACK_VB_SLOT].stride = 16;
				vd.layouts[FALLBACK_VB_SLOT].stepFunction = MTLVertexStepFunctionConstant;
				vd.layouts[FALLBACK_VB_SLOT].stepRate = 0;
			}
			pd.vertexDescriptor = vd;

			const bool alphaBlend = m_state.GetRenderState(ERenderState::ALPHABLENDENABLE) != 0;
			const bool sepAlpha = m_state.GetRenderState(ERenderState::SEPARATEALPHABLENDENABLE) != 0;
			const uint32_t srcRgb = m_state.GetRenderState(ERenderState::SRCBLEND);
			const uint32_t dstRgb = m_state.GetRenderState(ERenderState::DESTBLEND);
			const uint32_t blendOp = m_state.GetRenderState(ERenderState::BLENDOP);
			const uint32_t srcA = sepAlpha ? m_state.GetRenderState(ERenderState::SRCBLENDALPHA) : srcRgb;
			const uint32_t dstA = sepAlpha ? m_state.GetRenderState(ERenderState::DESTBLENDALPHA) : dstRgb;
			const uint32_t blendOpA = sepAlpha ? m_state.GetRenderState(ERenderState::BLENDOPALPHA) : blendOp;
			const uint32_t cwMask = m_state.GetRenderState(ERenderState::COLORWRITEENABLE);

			MTLColorWriteMask mtlMask = MTLColorWriteMaskNone;
			if (cwMask & 0x1u) mtlMask |= MTLColorWriteMaskRed;
			if (cwMask & 0x2u) mtlMask |= MTLColorWriteMaskGreen;
			if (cwMask & 0x4u) mtlMask |= MTLColorWriteMaskBlue;
			if (cwMask & 0x8u) mtlMask |= MTLColorWriteMaskAlpha;

			uint32_t colorCount = m_usingBackBufferRT ? 1u : m_activeColorTargetCount;
			for (uint32_t i = 0; i < colorCount; ++i)
			{
				MTLPixelFormat cf = m_usingBackBufferRT
					? MTLPixelFormatBGRA8Unorm
					: (m_activeColorTargets[i] ? m_activeColorTargets[i]->GetMTLPixelFormat() : MTLPixelFormatInvalid);
				if (cf == MTLPixelFormatInvalid)
					continue;
				pd.colorAttachments[i].pixelFormat = cf;
				pd.colorAttachments[i].writeMask = mtlMask;
				pd.colorAttachments[i].blendingEnabled = alphaBlend ? YES : NO;
				if (alphaBlend)
				{
					pd.colorAttachments[i].sourceRGBBlendFactor = ToMtlBlendFactor(srcRgb, false);
					pd.colorAttachments[i].destinationRGBBlendFactor = ToMtlBlendFactor(dstRgb, false);
					pd.colorAttachments[i].rgbBlendOperation = ToMtlBlendOp(blendOp);
					pd.colorAttachments[i].sourceAlphaBlendFactor = ToMtlBlendFactor(srcA, true);
					pd.colorAttachments[i].destinationAlphaBlendFactor = ToMtlBlendFactor(dstA, true);
					pd.colorAttachments[i].alphaBlendOperation = ToMtlBlendOp(blendOpA);
				}
			}

			MTLPixelFormat df = m_usingBackBufferRT
				? (m_defaultDepthTex ? MTLPixelFormatDepth32Float_Stencil8 : MTLPixelFormatInvalid)
				: (m_activeDepthTarget ? m_activeDepthTarget->GetMTLPixelFormat() : MTLPixelFormatInvalid);
			pd.depthAttachmentPixelFormat = df;
			if (df == MTLPixelFormatDepth32Float_Stencil8)
				pd.stencilAttachmentPixelFormat = df;

			NSError* err = nil;
			id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor:pd error:&err];
			if (!pso)
			{
				OUTPUT_LOG("ERROR: Metal PSO creation failed: %s\n", err ? [[err localizedDescription] UTF8String] : "unknown");
				return nullptr;
			}

			void* rawPso = reinterpret_cast<void*>(pso);
			m_psoCache[psoKey] = rawPso;
			return rawPso;
		}
	}

	void* RenderDeviceMetal::GetOrCreateDepthStencilState()
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev)
			return nullptr;

		const bool zEnable = m_state.GetRenderState(ERenderState::ZENABLE) != 0;
		const bool zWrite = m_state.GetRenderState(ERenderState::ZWRITEENABLE) != 0;
		const uint32_t zFunc = m_state.GetRenderState(ERenderState::ZFUNC);
		const bool stEnable = m_state.GetRenderState(ERenderState::STENCILENABLE) != 0;
		const bool twoSided = m_state.GetRenderState(ERenderState::TWOSIDEDSTENCILMODE) != 0;

		uint64_t key = 14695981039346656037ULL;
		HashCombineMtlU64(key, zEnable ? 1 : 0);
		HashCombineMtlU64(key, zWrite ? 1 : 0);
		HashCombineMtlU64(key, zFunc);
		HashCombineMtlU64(key, stEnable ? 1 : 0);
		if (stEnable)
		{
			HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::STENCILFUNC));
			HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::STENCILFAIL));
			HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::STENCILZFAIL));
			HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::STENCILPASS));
			HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::STENCILMASK));
			HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::STENCILWRITEMASK));
			HashCombineMtlU64(key, twoSided ? 1 : 0);
			if (twoSided)
			{
				HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::CCW_STENCILFUNC));
				HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::CCW_STENCILFAIL));
				HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::CCW_STENCILZFAIL));
				HashCombineMtlU64(key, m_state.GetRenderState(ERenderState::CCW_STENCILPASS));
			}
		}

		auto it = m_depthStencilCache.find(key);
		if (it != m_depthStencilCache.end())
			return it->second;

		@autoreleasepool
		{
			MTLDepthStencilDescriptor* dd = [[[MTLDepthStencilDescriptor alloc] init] autorelease];
			dd.depthCompareFunction = zEnable ? ToMtlCompareFn(zFunc) : MTLCompareFunctionAlways;
			dd.depthWriteEnabled = (zEnable && zWrite) ? YES : NO;

			if (stEnable)
			{
				uint32_t readMask = m_state.GetRenderState(ERenderState::STENCILMASK) & 0xFFu;
				uint32_t writeMask = m_state.GetRenderState(ERenderState::STENCILWRITEMASK) & 0xFFu;

				MTLStencilDescriptor* front = [[[MTLStencilDescriptor alloc] init] autorelease];
				front.stencilCompareFunction = ToMtlCompareFn(m_state.GetRenderState(ERenderState::STENCILFUNC));
				front.stencilFailureOperation = ToMtlStencilOp(m_state.GetRenderState(ERenderState::STENCILFAIL));
				front.depthFailureOperation = ToMtlStencilOp(m_state.GetRenderState(ERenderState::STENCILZFAIL));
				front.depthStencilPassOperation = ToMtlStencilOp(m_state.GetRenderState(ERenderState::STENCILPASS));
				front.readMask = readMask;
				front.writeMask = writeMask;
				dd.frontFaceStencil = front;

				if (twoSided)
				{
					MTLStencilDescriptor* back = [[[MTLStencilDescriptor alloc] init] autorelease];
					back.stencilCompareFunction = ToMtlCompareFn(m_state.GetRenderState(ERenderState::CCW_STENCILFUNC));
					back.stencilFailureOperation = ToMtlStencilOp(m_state.GetRenderState(ERenderState::CCW_STENCILFAIL));
					back.depthFailureOperation = ToMtlStencilOp(m_state.GetRenderState(ERenderState::CCW_STENCILZFAIL));
					back.depthStencilPassOperation = ToMtlStencilOp(m_state.GetRenderState(ERenderState::CCW_STENCILPASS));
					back.readMask = readMask;
					back.writeMask = writeMask;
					dd.backFaceStencil = back;
				}
				else
				{
					dd.backFaceStencil = front;
				}
			}

			id<MTLDepthStencilState> dss = [dev newDepthStencilStateWithDescriptor:dd];
			void* raw = reinterpret_cast<void*>(dss);
			m_depthStencilCache[key] = raw;
			return raw;
		}
	}

	void* RenderDeviceMetal::GetOrCreateSamplerState(uint32_t stage)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || stage >= 16)
			return nullptr;

		uint32_t addrU = m_state.GetSamplerState(stage, ESamplerStateType::ADDRESSU);
		uint32_t addrV = m_state.GetSamplerState(stage, ESamplerStateType::ADDRESSV);
		uint32_t addrW = m_state.GetSamplerState(stage, ESamplerStateType::ADDRESSW);
		uint32_t minF = m_state.GetSamplerState(stage, ESamplerStateType::MINFILTER);
		uint32_t magF = m_state.GetSamplerState(stage, ESamplerStateType::MAGFILTER);
		uint32_t mipF = m_state.GetSamplerState(stage, ESamplerStateType::MIPFILTER);
		uint32_t maxAniso = m_state.GetSamplerState(stage, ESamplerStateType::MAXANISOTROPY);

		uint64_t key = (static_cast<uint64_t>(addrU) & 0xFu)
			| ((static_cast<uint64_t>(addrV) & 0xFu) << 4)
			| ((static_cast<uint64_t>(addrW) & 0xFu) << 8)
			| ((static_cast<uint64_t>(minF) & 0xFu) << 12)
			| ((static_cast<uint64_t>(magF) & 0xFu) << 16)
			| ((static_cast<uint64_t>(mipF) & 0xFu) << 20)
			| ((static_cast<uint64_t>(maxAniso) & 0xFFu) << 24);

		auto it = m_samplerCache.find(key);
		if (it != m_samplerCache.end())
			return it->second;

		@autoreleasepool
		{
			MTLSamplerDescriptor* sd = [[[MTLSamplerDescriptor alloc] init] autorelease];
			sd.sAddressMode = ToMtlAddressMode(addrU);
			sd.tAddressMode = ToMtlAddressMode(addrV);
			sd.rAddressMode = ToMtlAddressMode(addrW);
			sd.minFilter = (minF == 1) ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
			sd.magFilter = (magF == 1) ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
			if (mipF == 0) sd.mipFilter = MTLSamplerMipFilterNotMipmapped;
			else if (mipF == 1) sd.mipFilter = MTLSamplerMipFilterNearest;
			else sd.mipFilter = MTLSamplerMipFilterLinear;

			if ((minF == 3 || magF == 3) && maxAniso > 1)
				sd.maxAnisotropy = (std::min)(maxAniso, 16u);
			else
				sd.maxAnisotropy = 1;

			id<MTLSamplerState> ss = [dev newSamplerStateWithDescriptor:sd];
			void* raw = reinterpret_cast<void*>(ss);
			m_samplerCache[key] = raw;
			return raw;
		}
	}

	bool RenderDeviceMetal::PrepareForDraw(EPrimitiveType primitiveType)
	{
		if (!m_currentProgram || !m_currentProgram->IsValid())
			return false;
		if (!EnsureRenderPassEncoder(false, false, false))
			return false;

		void* rawPso = GetOrCreatePipelineState(primitiveType);
		if (!rawPso)
			return false;

		id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
		[enc setRenderPipelineState:reinterpret_cast<id<MTLRenderPipelineState>>(rawPso)];

		if (void* rawDss = GetOrCreateDepthStencilState())
			[enc setDepthStencilState:reinterpret_cast<id<MTLDepthStencilState>>(rawDss)];

		[enc setStencilReferenceValue:(m_state.GetRenderState(ERenderState::STENCILREF) & 0xFFu)];

		// Direct3D default winding: clockwise front face; RSV_CULL_CCW culls back-facing CCW triangles
		[enc setFrontFacingWinding:MTLWindingClockwise];
		uint32_t cullMode = m_state.GetRenderState(ERenderState::CULLMODE);
		if (cullMode == RSV_CULL_NONE)
			[enc setCullMode:MTLCullModeNone];
		else if (cullMode == RSV_CULL_CW)
			[enc setCullMode:MTLCullModeFront];
		else
			[enc setCullMode:MTLCullModeBack];

		uint32_t fillMode = m_state.GetRenderState(ERenderState::FILLMODE);
		[enc setTriangleFillMode:(fillMode == 2) ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];

		uint32_t rawBias = m_state.GetRenderState(ERenderState::DEPTHBIAS);
		uint32_t rawSlope = m_state.GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS);
		float depthBias = 0.0f, slopeBias = 0.0f;
		std::memcpy(&depthBias, &rawBias, sizeof(float));
		std::memcpy(&slopeBias, &rawSlope, sizeof(float));
		[enc setDepthBias:(depthBias * 65536.0f) slopeScale:slopeBias clamp:0.0f];

		const ParaViewport& vp = m_state.GetViewport();
		MTLViewport mtlVp;
		mtlVp.originX = static_cast<double>(vp.X);
		mtlVp.originY = static_cast<double>(vp.Y);
		mtlVp.width = static_cast<double>((std::max)(vp.Width, 1u));
		mtlVp.height = static_cast<double>((std::max)(vp.Height, 1u));
		mtlVp.znear = static_cast<double>(vp.MinZ);
		mtlVp.zfar = static_cast<double>(vp.MaxZ);
		[enc setViewport:mtlVp];

		MTLScissorRect sc;
		if (m_state.GetRenderState(ERenderState::SCISSORTESTENABLE) != 0)
		{
			const RECT& r = m_state.GetScissorRect();
			int32_t sx = (std::max)(0, static_cast<int32_t>(r.left));
			int32_t sy = (std::max)(0, static_cast<int32_t>(r.top));
			int32_t ex = (std::min)(static_cast<int32_t>(m_activeRTWidth), (std::max)(sx, static_cast<int32_t>(r.right)));
			int32_t ey = (std::min)(static_cast<int32_t>(m_activeRTHeight), (std::max)(sy, static_cast<int32_t>(r.bottom)));
			sc.x = static_cast<NSUInteger>(sx);
			sc.y = static_cast<NSUInteger>(sy);
			sc.width = static_cast<NSUInteger>((std::max)(1, ex - sx));
			sc.height = static_cast<NSUInteger>((std::max)(1, ey - sy));
		}
		else
		{
			sc.x = 0;
			sc.y = 0;
			sc.width = (std::max)(m_activeRTWidth, 1u);
			sc.height = (std::max)(m_activeRTHeight, 1u);
		}
		[enc setScissorRect:sc];

		// Bind VS & PS Uniform Buffers (slot 16 & slot 17)
		uint32_t reqVs = (std::max)(m_vsConstSize, m_currentProgram->GetVSCBufferSize());
		if (reqVs > 0)
		{
			reqVs = AlignUpMtl(reqVs, 16u);
			if (m_vsConstShadow.size() < reqVs)
				m_vsConstShadow.resize(reqVs, 0);
			if (reqVs <= 4096u)
			{
				[enc setVertexBytes:m_vsConstShadow.data() length:reqVs atIndex:VS_UBO_SLOT];
			}
			else
			{
				void* cpuPtr = nullptr;
				void* mtlBuf = nullptr;
				uint32_t off = 0;
				if (AllocateUpload(reqVs, 256u, cpuPtr, mtlBuf, off))
				{
					std::memcpy(cpuPtr, m_vsConstShadow.data(), reqVs);
					[enc setVertexBuffer:reinterpret_cast<id<MTLBuffer>>(mtlBuf) offset:off atIndex:VS_UBO_SLOT];
				}
			}
		}

		uint32_t reqPs = (std::max)(m_psConstSize, m_currentProgram->GetPSCBufferSize());
		if (reqPs > 0)
		{
			reqPs = AlignUpMtl(reqPs, 16u);
			if (m_psConstShadow.size() < reqPs)
				m_psConstShadow.resize(reqPs, 0);
			if (reqPs <= 4096u)
			{
				[enc setFragmentBytes:m_psConstShadow.data() length:reqPs atIndex:PS_UBO_SLOT];
			}
			else
			{
				void* cpuPtr = nullptr;
				void* mtlBuf = nullptr;
				uint32_t off = 0;
				if (AllocateUpload(reqPs, 256u, cpuPtr, mtlBuf, off))
				{
					std::memcpy(cpuPtr, m_psConstShadow.data(), reqPs);
					[enc setFragmentBuffer:reinterpret_cast<id<MTLBuffer>>(mtlBuf) offset:off atIndex:PS_UBO_SLOT];
				}
			}
		}

		// Bind Vertex Streams 0..3 + Fallback [0,0,0,1] at slot 15
		for (uint32_t s = 0; s < MAX_VERTEX_STREAMS; ++s)
		{
			if (m_streams[s].buffer && m_streams[s].buffer->GetMTLBuffer())
				[enc setVertexBuffer:m_streams[s].buffer->GetMTLBuffer() offset:m_streams[s].offsetInBytes atIndex:s];
			else if (m_fallbackVertexBuf)
				[enc setVertexBuffer:reinterpret_cast<id<MTLBuffer>>(m_fallbackVertexBuf) offset:0 atIndex:s];
		}
		if (m_fallbackVertexBuf)
			[enc setVertexBuffer:reinterpret_cast<id<MTLBuffer>>(m_fallbackVertexBuf) offset:0 atIndex:FALLBACK_VB_SLOT];

		// Bind Textures & Samplers 0..15
		for (uint32_t slot = 0; slot < 16; ++slot)
		{
			MetalTexture* tex = m_boundTextures[slot] ? m_boundTextures[slot] : m_defaultWhiteTex;
			if (tex && tex->GetMTLTexture())
			{
				[enc setVertexTexture:tex->GetMTLTexture() atIndex:slot];
				[enc setFragmentTexture:tex->GetMTLTexture() atIndex:slot];
			}
			if (void* rawSmp = GetOrCreateSamplerState(slot))
			{
				id<MTLSamplerState> smp = reinterpret_cast<id<MTLSamplerState>>(rawSmp);
				[enc setVertexSamplerState:smp atIndex:slot];
				[enc setFragmentSamplerState:smp atIndex:slot];
			}
		}

		return true;
	}

	// =========================================================================
	// Draw Commands, Readback, State & Frame Submission
	// =========================================================================

	bool RenderDeviceMetal::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		MTLPrimitiveType mtlTopo = MTLPrimitiveTypeTriangle;
		uint32_t vertCount = 0;
		if (!MapPrimitiveTopologyMtl(PrimitiveType, PrimitiveCount, mtlTopo, vertCount))
			return false;
		if (!PrepareForDraw(PrimitiveType))
			return false;

		id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			uint32_t triCount = PrimitiveCount;
			uint32_t idxBytes = triCount * 3u * sizeof(uint16_t);
			void* cpuPtr = nullptr;
			void* mtlBuf = nullptr;
			uint32_t off = 0;
			if (!AllocateUpload(idxBytes, 256u, cpuPtr, mtlBuf, off))
				return false;
			uint16_t* idx = static_cast<uint16_t*>(cpuPtr);
			for (uint32_t t = 0; t < triCount; ++t)
			{
				idx[t * 3u + 0] = static_cast<uint16_t>(StartVertex);
				idx[t * 3u + 1] = static_cast<uint16_t>(StartVertex + t + 1u);
				idx[t * 3u + 2] = static_cast<uint16_t>(StartVertex + t + 2u);
			}
			[enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
							indexCount:triCount * 3u
							 indexType:MTLIndexTypeUInt16
						   indexBuffer:reinterpret_cast<id<MTLBuffer>>(mtlBuf)
					 indexBufferOffset:off];
			return true;
		}

		uint32_t instCount = 1u;
		if ((m_streams[0].frequencyDivider & 0x40000000u) != 0)
			instCount = (std::max)(1u, m_streams[0].frequencyDivider & 0x3FFFFFFFu);

		[enc drawPrimitives:mtlTopo vertexStart:StartVertex vertexCount:vertCount instanceCount:instCount];
		return true;
	}

	bool RenderDeviceMetal::DrawIndexedPrimitive(
		EPrimitiveType Type,
		int BaseVertexIndex,
		uint32_t /*MinIndex*/,
		uint32_t /*NumVertices*/,
		uint32_t indexStart,
		uint32_t PrimitiveCount)
	{
		uint32_t instCount = 1u;
		if ((m_streams[0].frequencyDivider & 0x40000000u) != 0)
			instCount = (std::max)(1u, m_streams[0].frequencyDivider & 0x3FFFFFFFu);
		return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, instCount);
	}

	bool RenderDeviceMetal::DrawIndexedInstanced(
		EPrimitiveType primitiveType,
		int baseVertexIndex,
		uint32_t indexStart,
		uint32_t primitiveCount,
		uint32_t instanceCount)
	{
		if (!m_currentIndexBuffer || !m_currentIndexBuffer->GetMTLBuffer() || instanceCount == 0)
			return false;

		MTLPrimitiveType mtlTopo = MTLPrimitiveTypeTriangle;
		uint32_t indexCount = 0;
		if (!MapPrimitiveTopologyMtl(primitiveType, primitiveCount, mtlTopo, indexCount))
			return false;
		if (!PrepareForDraw(primitiveType))
			return false;

		id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
		[enc drawIndexedPrimitives:mtlTopo
						indexCount:indexCount
						 indexType:MTLIndexTypeUInt16
					   indexBuffer:m_currentIndexBuffer->GetMTLBuffer()
				 indexBufferOffset:indexStart * sizeof(uint16_t)
					 instanceCount:instanceCount
						baseVertex:baseVertexIndex
					  baseInstance:0];
		return true;
	}

	bool RenderDeviceMetal::DrawPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t PrimitiveCount,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!pVertexStreamZeroData || VertexStreamZeroStride == 0)
			return false;

		MTLPrimitiveType mtlTopo = MTLPrimitiveTypeTriangle;
		uint32_t vertCount = 0;
		if (!MapPrimitiveTopologyMtl(PrimitiveType, PrimitiveCount, mtlTopo, vertCount))
			return false;

		uint32_t vbBytes = vertCount * VertexStreamZeroStride;
		void* cpuPtr = nullptr;
		void* mtlBuf = nullptr;
		uint32_t off = 0;
		if (!AllocateUpload(vbBytes, 256u, cpuPtr, mtlBuf, off))
			return false;
		std::memcpy(cpuPtr, pVertexStreamZeroData, vbBytes);

		StreamBinding prev0 = m_streams[0];
		m_streams[0].buffer = nullptr;
		m_streams[0].stride = VertexStreamZeroStride;
		m_streams[0].offsetInBytes = 0;
		m_streams[0].frequencyDivider = 0;

		bool ok = PrepareForDraw(PrimitiveType);
		if (ok)
		{
			id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
			[enc setVertexBuffer:reinterpret_cast<id<MTLBuffer>>(mtlBuf) offset:off atIndex:0];
			if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
			{
				void* icpu = nullptr;
				void* ibuf = nullptr;
				uint32_t ioff = 0;
				if (AllocateUpload(PrimitiveCount * 3u * sizeof(uint16_t), 256u, icpu, ibuf, ioff))
				{
					uint16_t* idx = static_cast<uint16_t*>(icpu);
					for (uint32_t t = 0; t < PrimitiveCount; ++t)
					{
						idx[t * 3u + 0] = 0;
						idx[t * 3u + 1] = static_cast<uint16_t>(t + 1u);
						idx[t * 3u + 2] = static_cast<uint16_t>(t + 2u);
					}
					[enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
									indexCount:PrimitiveCount * 3u
									 indexType:MTLIndexTypeUInt16
								   indexBuffer:reinterpret_cast<id<MTLBuffer>>(ibuf)
							 indexBufferOffset:ioff];
				}
			}
			else
			{
				[enc drawPrimitives:mtlTopo vertexStart:0 vertexCount:vertCount];
			}
		}

		m_streams[0] = prev0;
		return ok;
	}

	bool RenderDeviceMetal::DrawIndexedPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t /*MinVertexIndex*/,
		uint32_t NumVertices,
		uint32_t PrimitiveCount,
		const void* pIndexData,
		PixelFormat IndexDataFormat,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!pIndexData || !pVertexStreamZeroData || VertexStreamZeroStride == 0 || NumVertices == 0)
			return false;

		MTLPrimitiveType mtlTopo = MTLPrimitiveTypeTriangle;
		uint32_t indexCount = 0;
		if (!MapPrimitiveTopologyMtl(PrimitiveType, PrimitiveCount, mtlTopo, indexCount))
			return false;

		const bool is32 = (IndexDataFormat == PixelFormat::INDEX32);
		uint32_t idxElemSize = is32 ? 4u : 2u;
		uint32_t vbBytes = NumVertices * VertexStreamZeroStride;
		uint32_t ibBytes = indexCount * idxElemSize;

		void* vCpu = nullptr; void* vBuf = nullptr; uint32_t vOff = 0;
		void* iCpu = nullptr; void* iBuf = nullptr; uint32_t iOff = 0;
		if (!AllocateUpload(vbBytes, 256u, vCpu, vBuf, vOff) ||
			!AllocateUpload(ibBytes, 256u, iCpu, iBuf, iOff))
			return false;

		std::memcpy(vCpu, pVertexStreamZeroData, vbBytes);
		std::memcpy(iCpu, pIndexData, ibBytes);

		StreamBinding prev0 = m_streams[0];
		m_streams[0].buffer = nullptr;
		m_streams[0].stride = VertexStreamZeroStride;
		m_streams[0].offsetInBytes = 0;
		m_streams[0].frequencyDivider = 0;

		bool ok = PrepareForDraw(PrimitiveType);
		if (ok)
		{
			id<MTLRenderCommandEncoder> enc = reinterpret_cast<id<MTLRenderCommandEncoder>>(m_activeEncoder);
			[enc setVertexBuffer:reinterpret_cast<id<MTLBuffer>>(vBuf) offset:vOff atIndex:0];
			[enc drawIndexedPrimitives:mtlTopo
							indexCount:indexCount
							 indexType:is32 ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16
						   indexBuffer:reinterpret_cast<id<MTLBuffer>>(iBuf)
					 indexBufferOffset:iOff];
		}

		m_streams[0] = prev0;
		return ok;
	}

	bool RenderDeviceMetal::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t /*nDataFormat*/, uint32_t /*nDataType*/)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || !pDataOut || nWidth <= 0 || nHeight <= 0)
			return false;

		id<MTLTexture> srcTex = nil;
		MTLPixelFormat srcFmt = MTLPixelFormatBGRA8Unorm;
		if (!m_usingBackBufferRT && m_activeColorTargets[0])
		{
			srcTex = m_activeColorTargets[0]->GetMTLTexture();
			srcFmt = m_activeColorTargets[0]->GetMTLPixelFormat();
		}
		else if (m_importedColorTex)
		{
			srcTex = reinterpret_cast<id<MTLTexture>>(m_importedColorTex);
			srcFmt = [srcTex pixelFormat];
		}
		else if (EnsureDrawableAcquired())
		{
			srcTex = [reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) texture];
			srcFmt = [srcTex pixelFormat];
		}

		if (!srcTex || nLeft < 0 || nTop < 0 ||
			static_cast<NSUInteger>(nLeft + nWidth) > [srcTex width] ||
			static_cast<NSUInteger>(nTop + nHeight) > [srcTex height])
			return false;

		EndActiveRenderPass();
		if (!EnsureCommandBuffer())
			return false;

		uint32_t rowBytes = AlignUpMtl(static_cast<uint32_t>(nWidth) * 4u, 256u);
		uint32_t totalBytes = rowBytes * static_cast<uint32_t>(nHeight);
		id<MTLBuffer> stageBuf = [dev newBufferWithLength:totalBytes options:MTLResourceStorageModeShared];
		if (!stageBuf)
			return false;

		id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
		id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
		[blit copyFromTexture:srcTex
				  sourceSlice:0
				  sourceLevel:0
				 sourceOrigin:MTLOriginMake(static_cast<NSUInteger>(nLeft), static_cast<NSUInteger>(nTop), 0)
				   sourceSize:MTLSizeMake(static_cast<NSUInteger>(nWidth), static_cast<NSUInteger>(nHeight), 1)
					 toBuffer:stageBuf
			destinationOffset:0
	   destinationBytesPerRow:rowBytes
	 destinationBytesPerImage:totalBytes];
		[blit endEncoding];

		ExecuteAndSyncGPU();

		const uint8_t* srcPtr = static_cast<const uint8_t*>([stageBuf contents]);
		uint8_t* dstPtr = static_cast<uint8_t*>(pDataOut);
		const bool isBGRA = (srcFmt == MTLPixelFormatBGRA8Unorm || srcFmt == MTLPixelFormatBGRA8Unorm_sRGB);
		for (int y = 0; y < nHeight; ++y)
		{
			const uint8_t* row = srcPtr + static_cast<size_t>(y) * rowBytes;
			uint8_t* dstRow = dstPtr + static_cast<size_t>(y) * static_cast<size_t>(nWidth) * 4u;
			if (isBGRA)
			{
				for (int x = 0; x < nWidth; ++x)
				{
					dstRow[x * 4 + 0] = row[x * 4 + 2];
					dstRow[x * 4 + 1] = row[x * 4 + 1];
					dstRow[x * 4 + 2] = row[x * 4 + 0];
					dstRow[x * 4 + 3] = row[x * 4 + 3];
				}
			}
			else
			{
				std::memcpy(dstRow, row, static_cast<size_t>(nWidth) * 4u);
			}
		}

		[stageBuf release];
		return true;
	}

	bool RenderDeviceMetal::ReadDepthPixel(int x, int y, float& outDepth)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		id<MTLTexture> depthTex = m_usingBackBufferRT
			? reinterpret_cast<id<MTLTexture>>(m_defaultDepthTex)
			: (m_activeDepthTarget ? m_activeDepthTarget->GetMTLTexture() : nil);
		if (!dev || !depthTex || x < 0 || y < 0 ||
			static_cast<NSUInteger>(x) >= [depthTex width] ||
			static_cast<NSUInteger>(y) >= [depthTex height])
			return false;

		EndActiveRenderPass();
		if (!EnsureCommandBuffer())
			return false;

		id<MTLBuffer> stageBuf = [dev newBufferWithLength:256 options:MTLResourceStorageModeShared];
		if (!stageBuf)
			return false;

		id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
		id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
		[blit copyFromTexture:depthTex
				  sourceSlice:0
				  sourceLevel:0
				 sourceOrigin:MTLOriginMake(static_cast<NSUInteger>(x), static_cast<NSUInteger>(y), 0)
				   sourceSize:MTLSizeMake(1, 1, 1)
					 toBuffer:stageBuf
			destinationOffset:0
	   destinationBytesPerRow:256
	 destinationBytesPerImage:256
					  options:MTLBlitOptionDepthFromDepthStencil];
		[blit endEncoding];

		ExecuteAndSyncGPU();

		std::memcpy(&outDepth, [stageBuf contents], sizeof(float));
		[stageBuf release];
		return true;
	}

	bool RenderDeviceMetal::ReadTexturePixels(IRHITexture* texture, void* pDataOut)
	{
		id<MTLDevice> dev = reinterpret_cast<id<MTLDevice>>(m_device);
		if (!dev || !texture || !pDataOut)
			return false;

		auto* mtlTexObj = static_cast<MetalTexture*>(texture->GetNativeHandle());
		if (!mtlTexObj || !mtlTexObj->GetMTLTexture())
			return false;

		id<MTLTexture> srcTex = mtlTexObj->GetMTLTexture();
		uint32_t w = mtlTexObj->GetWidth();
		uint32_t h = mtlTexObj->GetHeight();
		MTLPixelFormat fmt = mtlTexObj->GetMTLPixelFormat();
		if (fmt != MTLPixelFormatRGBA8Unorm && fmt != MTLPixelFormatBGRA8Unorm)
			return false;

		EndActiveRenderPass();
		if (!EnsureCommandBuffer())
			return false;

		uint32_t rowBytes = AlignUpMtl(w * 4u, 256u);
		uint32_t totalBytes = rowBytes * h;
		id<MTLBuffer> stageBuf = [dev newBufferWithLength:totalBytes options:MTLResourceStorageModeShared];
		if (!stageBuf)
			return false;

		id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
		id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
		[blit copyFromTexture:srcTex
				  sourceSlice:0
				  sourceLevel:0
				 sourceOrigin:MTLOriginMake(0, 0, 0)
				   sourceSize:MTLSizeMake(w, h, 1)
					 toBuffer:stageBuf
			destinationOffset:0
	   destinationBytesPerRow:rowBytes
	 destinationBytesPerImage:totalBytes];
		[blit endEncoding];

		ExecuteAndSyncGPU();

		const uint8_t* srcPtr = static_cast<const uint8_t*>([stageBuf contents]);
		uint8_t* dstPtr = static_cast<uint8_t*>(pDataOut);
		const bool isBGRA = (fmt == MTLPixelFormatBGRA8Unorm);
		for (uint32_t y = 0; y < h; ++y)
		{
			const uint8_t* row = srcPtr + static_cast<size_t>(y) * rowBytes;
			uint8_t* dstRow = dstPtr + static_cast<size_t>(y) * w * 4u;
			if (isBGRA)
			{
				for (uint32_t x = 0; x < w; ++x)
				{
					dstRow[x * 4 + 0] = row[x * 4 + 2];
					dstRow[x * 4 + 1] = row[x * 4 + 1];
					dstRow[x * 4 + 2] = row[x * 4 + 0];
					dstRow[x * 4 + 3] = row[x * 4 + 3];
				}
			}
			else
			{
				std::memcpy(dstRow, row, w * 4u);
			}
		}

		[stageBuf release];
		return true;
	}

	uint32_t RenderDeviceMetal::GetRenderState(const ERenderState& State) { return m_state.GetRenderState(State); }
	bool RenderDeviceMetal::SetRenderState(const ERenderState State, const uint32_t Value) { m_state.SetRenderState(State, Value); return true; }
	bool RenderDeviceMetal::SetClipPlane(uint32_t Index, const float* pPlane) { if (pPlane) m_state.SetClipPlane(Index, pPlane); return true; }
	bool RenderDeviceMetal::SetTransform(ETransformsStateType /*State*/, DeviceMatrix_ptr /*pMatrix*/) { return true; }
	bool RenderDeviceMetal::SetFVF(uint32_t /*FVF*/) { return true; }
	void RenderDeviceMetal::SetCursorPosition(int /*X*/, int /*Y*/, uint32_t /*Flags*/) {}

	bool RenderDeviceMetal::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		if (stage >= 16)
			return false;
		m_boundTextures[stage] = texture ? static_cast<MetalTexture*>(texture->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceMetal::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		m_state.SetSamplerState(stage, type, value);
		return true;
	}

	bool RenderDeviceMetal::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		if (value) *value = m_state.GetSamplerState(stage, type);
		return true;
	}

	bool RenderDeviceMetal::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
	{
		m_currentVertexDecl = pVertexDeclaration;
		return true;
	}

	bool RenderDeviceMetal::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
	{
		if (!ppDecl) return false;
		*ppDecl = new CVertexDeclaration(pVertexElements);
		return true;
	}

	bool RenderDeviceMetal::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		m_currentIndexBuffer = pIndexData ? static_cast<MetalBuffer*>(pIndexData->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceMetal::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
	{
		if (StreamNumber >= MAX_VERTEX_STREAMS)
			return false;
		m_streams[StreamNumber].buffer = pStreamData ? static_cast<MetalBuffer*>(pStreamData->GetNativeHandle()) : nullptr;
		m_streams[StreamNumber].offsetInBytes = OffsetInBytes;
		m_streams[StreamNumber].stride = Stride;
		return true;
	}

	bool RenderDeviceMetal::SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider)
	{
		if (streamNumber >= MAX_VERTEX_STREAMS)
			return false;
		m_streams[streamNumber].frequencyDivider = divider;
		return true;
	}

	ParaViewport RenderDeviceMetal::GetViewport() { return m_state.GetViewport(); }
	bool RenderDeviceMetal::SetViewport(const ParaViewport& viewport) { m_state.SetViewport(viewport); return true; }
	bool RenderDeviceMetal::SetClearColor(const Color4f& color) { m_state.SetClearColor(color); return true; }
	bool RenderDeviceMetal::SetClearDepth(const float depth) { m_state.SetClearDepth(depth); return true; }
	bool RenderDeviceMetal::SetClearStencil(const int stencil) { m_state.SetClearStencil(stencil); return true; }

	bool RenderDeviceMetal::Clear(bool color, bool depth, bool stencil)
	{
		if (!color && !depth && !stencil)
			return true;
		return EnsureRenderPassEncoder(color, depth, stencil);
	}

	bool RenderDeviceMetal::SetScissorRect(RECT* pRect) { m_state.SetScissorRect(pRect); return true; }
	bool RenderDeviceMetal::GetScissorRect(RECT* pRect) { if (pRect) *pRect = m_state.GetScissorRect(); return true; }

	bool RenderDeviceMetal::BeginScene()
	{
		auto* pApp = CGlobals::GetApp();
		auto* pWin = pApp ? pApp->GetRenderWindow() : nullptr;
		if (pWin)
		{
			uint32_t winW = pApp->IsRotateScreen() ? pWin->GetHeight() : pWin->GetWidth();
			uint32_t winH = pApp->IsRotateScreen() ? pWin->GetWidth() : pWin->GetHeight();
			if (winW > 0 && winH > 0 && (winW != m_backBufferWidth || winH != m_backBufferHeight))
			{
				ResizeSwapChain(winW, winH);
			}
		}

		AcquireFrameSlotIfNeeded();
		SetRenderTargets(0, nullptr, nullptr);
		return EnsureCommandBuffer();
	}

	bool RenderDeviceMetal::EndScene()
	{
		EndActiveRenderPass();
		return true;
	}

	bool RenderDeviceMetal::Present()
	{
		EndActiveRenderPass();
		if (!EnsureCommandBuffer())
		{
			if (m_frameSlotAcquired && m_frameSemaphore)
			{
				dispatch_semaphore_signal(reinterpret_cast<dispatch_semaphore_t>(m_frameSemaphore));
				m_frameSlotAcquired = false;
			}
			return false;
		}

		AcquireFrameSlotIfNeeded();
		id<MTLCommandBuffer> cmd = reinterpret_cast<id<MTLCommandBuffer>>(m_activeCmdBuf);
		if (m_currentDrawable)
		{
			[cmd presentDrawable:reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable)];
			[reinterpret_cast<id<CAMetalDrawable>>(m_currentDrawable) release];
			m_currentDrawable = nullptr;
		}

		uint64_t serial = m_nextSubmitSerial++;
		m_frames[m_frameIndex].submitSerial = serial;
		dispatch_semaphore_t sem = reinterpret_cast<dispatch_semaphore_t>(m_frameSemaphore);
		const bool signalSlot = m_frameSlotAcquired && (sem != nullptr);
		m_frameSlotAcquired = false;
		uint64_t* completedPtr = &m_completedSerial;
		[cmd addCompletedHandler:^(id<MTLCommandBuffer> /*buffer*/) {
			if (*completedPtr < serial)
				*completedPtr = serial;
			if (signalSlot)
				dispatch_semaphore_signal(sem);
		}];

		[cmd commit];
		[cmd release];
		m_activeCmdBuf = nullptr;
		m_frameIndex = (m_frameIndex + 1u) % FRAME_COUNT;
		return true;
	}

	void RenderDeviceMetal::Flush()
	{
		ExecuteAndSyncGPU();
	}
} // namespace ParaEngine

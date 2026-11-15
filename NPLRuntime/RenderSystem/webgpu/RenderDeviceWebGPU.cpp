#include "ParaEngine.h"
#include "RenderDeviceWebGPU.h"
#include "RHISoftwareS3TC.h"
#include "VertexDeclarationOpenGL.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

namespace ParaEngine
{
	namespace
	{
		uint32_t AlignUpU32(uint32_t val, uint32_t alignment)
		{
			return (val + alignment - 1u) & ~(alignment - 1u);
		}

		uint64_t Fnv1aBytes64(const void* data, size_t len, uint64_t seed = 14695981039346656037ull)
		{
			const auto* bytes = static_cast<const uint8_t*>(data);
			uint64_t h = seed;
			for (size_t i = 0; i < len; ++i)
			{
				h ^= static_cast<uint64_t>(bytes[i]);
				h *= 1099511628211ull;
			}
			return h;
		}

		bool IsDepthPixelFormat(PixelFormat fmt)
		{
			return fmt == PixelFormat::D24S8 || fmt == PixelFormat::D32 || fmt == PixelFormat::D16 || fmt == PixelFormat::D24X8;
		}

		bool IsBCCompressedWGPUFormat(WGPUTextureFormat fmt)
		{
			return fmt == WGPUTextureFormat_BC1RGBAUnorm ||
				fmt == WGPUTextureFormat_BC2RGBAUnorm ||
				fmt == WGPUTextureFormat_BC3RGBAUnorm;
		}

		bool IsUnblendableFloat32WGPUFormat(WGPUTextureFormat fmt)
		{
			return fmt == WGPUTextureFormat_R32Float ||
				fmt == WGPUTextureFormat_RG32Float ||
				fmt == WGPUTextureFormat_RGBA32Float;
		}

		WGPUTextureFormat ToWGPUTextureFormat(
			PixelFormat fmt,
			uint32_t width,
			uint32_t height,
			bool supportsBC,
			bool supportsFloat32Filterable,
			bool hasInitialCpuData,
			bool& outDecodedS3TC,
			bool& outExpandedRGBA8)
		{
			outDecodedS3TC = false;
			outExpandedRGBA8 = false;

			if (RHISoftwareS3TC::IsS3TCFormat(fmt))
			{
				if (supportsBC && (width % 4u) == 0u && (height % 4u) == 0u)
				{
					switch (fmt)
					{
					case PixelFormat::DXT1: return WGPUTextureFormat_BC1RGBAUnorm;
					case PixelFormat::DXT2:
					case PixelFormat::DXT3: return WGPUTextureFormat_BC2RGBAUnorm;
					case PixelFormat::DXT4:
					case PixelFormat::DXT5: return WGPUTextureFormat_BC3RGBAUnorm;
					default: break;
					}
				}
				outDecodedS3TC = true;
				return WGPUTextureFormat_RGBA8Unorm;
			}

			if (RHISoftwareS3TC::NeedsRGBA8Expansion(fmt, /*backendSupportsA8=*/false))
			{
				outExpandedRGBA8 = true;
				return WGPUTextureFormat_RGBA8Unorm;
			}

			switch (fmt)
			{
			case PixelFormat::A8R8G8B8:
			case PixelFormat::X8R8G8B8:
				return WGPUTextureFormat_RGBA8Unorm;
			case PixelFormat::A8B8G8R8:
			case PixelFormat::X8B8G8R8:
				return WGPUTextureFormat_BGRA8Unorm;
			case PixelFormat::R32F:
				return (supportsFloat32Filterable || hasInitialCpuData) ? WGPUTextureFormat_R32Float : WGPUTextureFormat_R16Float;
			case PixelFormat::G16R16F:
				return WGPUTextureFormat_RG16Float;
			case PixelFormat::G32R32F:
				return (supportsFloat32Filterable || hasInitialCpuData) ? WGPUTextureFormat_RG32Float : WGPUTextureFormat_RG16Float;
			case PixelFormat::A16B16G16R16F:
				return WGPUTextureFormat_RGBA16Float;
			case PixelFormat::A32B32G32R32F:
				return (supportsFloat32Filterable || hasInitialCpuData) ? WGPUTextureFormat_RGBA32Float : WGPUTextureFormat_RGBA16Float;
			case PixelFormat::D16:
				return WGPUTextureFormat_Depth16Unorm;
			case PixelFormat::D32:
				return WGPUTextureFormat_Depth32Float;
			case PixelFormat::D24X8:
				return WGPUTextureFormat_Depth24Plus;
			case PixelFormat::D24S8:
				return WGPUTextureFormat_Depth24PlusStencil8;
			default:
				return WGPUTextureFormat_RGBA8Unorm;
			}
		}

		uint32_t GetFormatTightRowBytes(WGPUTextureFormat fmt, uint32_t width)
		{
			width = (std::max)(1u, width);
			switch (fmt)
			{
			case WGPUTextureFormat_BC1RGBAUnorm:
				return ((width + 3u) / 4u) * 8u;
			case WGPUTextureFormat_BC2RGBAUnorm:
			case WGPUTextureFormat_BC3RGBAUnorm:
				return ((width + 3u) / 4u) * 16u;
			case WGPUTextureFormat_R8Unorm:
				return width;
			case WGPUTextureFormat_RG8Unorm:
			case WGPUTextureFormat_R16Float:
			case WGPUTextureFormat_Depth16Unorm:
				return width * 2u;
			case WGPUTextureFormat_RG32Float:
			case WGPUTextureFormat_RGBA16Float:
				return width * 8u;
			case WGPUTextureFormat_RGBA32Float:
				return width * 16u;
			default:
				return width * 4u;
			}
		}

		uint32_t GetFormatBlockRows(WGPUTextureFormat fmt, uint32_t height)
		{
			height = (std::max)(1u, height);
			switch (fmt)
			{
			case WGPUTextureFormat_BC1RGBAUnorm:
			case WGPUTextureFormat_BC2RGBAUnorm:
			case WGPUTextureFormat_BC3RGBAUnorm:
				return (height + 3u) / 4u;
			default:
				return height;
			}
		}

		// Maps D3DDECLUSAGE + UsageIndex to canonical ParaRHI VS shaderLocation 0..15 (must match MapSemanticToVSLocation in FxShaderTypes.h)
		int32_t MapVertexElementToLocation(uint8_t usage, uint8_t usageIndex)
		{
			switch (usage)
			{
			case 0: // D3DDECLUSAGE_POSITION
			case 9: // D3DDECLUSAGE_POSITIONT
				return (usageIndex == 0) ? 0 : -1;
			case 1: // D3DDECLUSAGE_BLENDWEIGHT
				return 1;
			case 2: // D3DDECLUSAGE_BLENDINDICES
				return 2;
			case 3: // D3DDECLUSAGE_NORMAL
				return (usageIndex == 0) ? 3 : -1;
			case 10: // D3DDECLUSAGE_COLOR
				return (usageIndex == 0) ? 4 : 5;
			case 5: // D3DDECLUSAGE_TEXCOORD
				return (usageIndex < 8u) ? static_cast<int32_t>(6u + usageIndex) : 13;
			case 6: // D3DDECLUSAGE_TANGENT
				return 14;
			case 7: // D3DDECLUSAGE_BINORMAL
			case 4: // D3DDECLUSAGE_PSIZE
				return 15;
			default:
				return -1;
			}
		}

		// AstTextCodeGenCommon emits all WGSL VS input @location attributes as vec4<f32>, so all vertex formats must decode to float.
		WGPUVertexFormat MapDeclTypeToWGPU(uint8_t declType)
		{
			switch (declType)
			{
			case 0:  return WGPUVertexFormat_Float32;        // D3DDECLTYPE_FLOAT1
			case 1:  return WGPUVertexFormat_Float32x2;      // D3DDECLTYPE_FLOAT2
			case 2:  return WGPUVertexFormat_Float32x3;      // D3DDECLTYPE_FLOAT3
			case 3:  return WGPUVertexFormat_Float32x4;      // D3DDECLTYPE_FLOAT4
			case 4:                                          // D3DDECLTYPE_D3DCOLOR
#if defined(__EMSCRIPTEN__) && !defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER) && !defined(WGPUVertexFormat_Unorm8x4BGRA)
				return WGPUVertexFormat_Unorm8x4;
#else
				return WGPUVertexFormat_Unorm8x4BGRA;
#endif
			case 5:  return WGPUVertexFormat_Unorm8x4;       // D3DDECLTYPE_UBYTE4
			case 6:  return WGPUVertexFormat_Snorm16x2;      // D3DDECLTYPE_SHORT2
			case 7:  return WGPUVertexFormat_Snorm16x4;      // D3DDECLTYPE_SHORT4
			case 8:  return WGPUVertexFormat_Unorm8x4;       // D3DDECLTYPE_UBYTE4N
			case 9:  return WGPUVertexFormat_Snorm16x2;      // D3DDECLTYPE_SHORT2N
			case 10: return WGPUVertexFormat_Snorm16x4;      // D3DDECLTYPE_SHORT4N
			case 11: return WGPUVertexFormat_Unorm16x2;      // D3DDECLTYPE_USHORT2N
			case 12: return WGPUVertexFormat_Unorm16x4;      // D3DDECLTYPE_USHORT4N
			case 15: return WGPUVertexFormat_Float16x2;      // D3DDECLTYPE_FLOAT16_2
			case 16: return WGPUVertexFormat_Float16x4;      // D3DDECLTYPE_FLOAT16_4
			default: return WGPUVertexFormat_Float32x4;
			}
		}

		uint32_t GetVertexFormatByteSize(WGPUVertexFormat fmt)
		{
			switch (fmt)
			{
			case WGPUVertexFormat_Float32:
			case WGPUVertexFormat_Uint8x4:
			case WGPUVertexFormat_Unorm8x4:
			case WGPUVertexFormat_Sint16x2:
			case WGPUVertexFormat_Snorm16x2:
			case WGPUVertexFormat_Unorm16x2:
			case WGPUVertexFormat_Float16x2:
#if !defined(__EMSCRIPTEN__) || defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER) || defined(WGPUVertexFormat_Unorm8x4BGRA)
			case WGPUVertexFormat_Unorm8x4BGRA:
#endif
				return 4u;
			case WGPUVertexFormat_Float32x2:
			case WGPUVertexFormat_Sint16x4:
			case WGPUVertexFormat_Snorm16x4:
			case WGPUVertexFormat_Unorm16x4:
			case WGPUVertexFormat_Float16x4:
				return 8u;
			case WGPUVertexFormat_Float32x3:
				return 12u;
			case WGPUVertexFormat_Float32x4:
			default:
				return 16u;
			}
		}

		WGPUCompareFunction MapCompareFunc(uint32_t d3dCmp)
		{
			switch (d3dCmp)
			{
			case 1: return WGPUCompareFunction_Never;
			case 2: return WGPUCompareFunction_Less;
			case 3: return WGPUCompareFunction_Equal;
			case 4: return WGPUCompareFunction_LessEqual;
			case 5: return WGPUCompareFunction_Greater;
			case 6: return WGPUCompareFunction_NotEqual;
			case 7: return WGPUCompareFunction_GreaterEqual;
			case 8: default: return WGPUCompareFunction_Always;
			}
		}

		WGPUStencilOperation MapStencilOp(uint32_t d3dStencilOp)
		{
			switch (d3dStencilOp)
			{
			case 1: default: return WGPUStencilOperation_Keep;
			case 2: return WGPUStencilOperation_Zero;
			case 3: return WGPUStencilOperation_Replace;
			case 4: return WGPUStencilOperation_IncrementClamp;
			case 5: return WGPUStencilOperation_DecrementClamp;
			case 6: return WGPUStencilOperation_Invert;
			case 7: return WGPUStencilOperation_IncrementWrap;
			case 8: return WGPUStencilOperation_DecrementWrap;
			}
		}

		WGPUBlendFactor MapBlendFactor(uint32_t d3dBlend, bool isAlpha)
		{
			switch (d3dBlend)
			{
			case 1: return WGPUBlendFactor_Zero;
			case 2: return WGPUBlendFactor_One;
			case 3: return isAlpha ? WGPUBlendFactor_SrcAlpha : WGPUBlendFactor_Src;
			case 4: return isAlpha ? WGPUBlendFactor_OneMinusSrcAlpha : WGPUBlendFactor_OneMinusSrc;
			case 5: return WGPUBlendFactor_SrcAlpha;
			case 6: return WGPUBlendFactor_OneMinusSrcAlpha;
			case 7: return WGPUBlendFactor_DstAlpha;
			case 8: return WGPUBlendFactor_OneMinusDstAlpha;
			case 9: return isAlpha ? WGPUBlendFactor_DstAlpha : WGPUBlendFactor_Dst;
			case 10: return isAlpha ? WGPUBlendFactor_OneMinusDstAlpha : WGPUBlendFactor_OneMinusDst;
			case 11: return WGPUBlendFactor_SrcAlphaSaturated;
			case 14: return WGPUBlendFactor_Constant;
			case 15: return WGPUBlendFactor_OneMinusConstant;
			default: return WGPUBlendFactor_One;
			}
		}

		WGPUBlendOperation MapBlendOp(uint32_t d3dBlendOp)
		{
			switch (d3dBlendOp)
			{
			case 2: return WGPUBlendOperation_Subtract;
			case 3: return WGPUBlendOperation_ReverseSubtract;
			case 4: return WGPUBlendOperation_Min;
			case 5: return WGPUBlendOperation_Max;
			case 1: default: return WGPUBlendOperation_Add;
			}
		}

		WGPUAddressMode MapAddressMode(uint32_t addr)
		{
			switch (addr)
			{
			case 2: return WGPUAddressMode_MirrorRepeat;
			case 3:
			case 4: return WGPUAddressMode_ClampToEdge;
			case 1: default: return WGPUAddressMode_Repeat;
			}
		}

		WGPUPrimitiveTopology MapPrimitiveTopology(EPrimitiveType pt, uint32_t primCount, uint32_t& outVertexOrIndexCount)
		{
			switch (pt)
			{
			case EPrimitiveType::POINTLIST:
				outVertexOrIndexCount = primCount;
				return WGPUPrimitiveTopology_PointList;
			case EPrimitiveType::LINELIST:
				outVertexOrIndexCount = primCount * 2u;
				return WGPUPrimitiveTopology_LineList;
			case EPrimitiveType::LINESTRIP:
				outVertexOrIndexCount = (primCount > 0) ? (primCount + 1u) : 0u;
				return WGPUPrimitiveTopology_LineStrip;
			case EPrimitiveType::TRIANGLESTRIP:
			case EPrimitiveType::TRIANGLEFAN:
				outVertexOrIndexCount = (primCount > 0) ? (primCount + 2u) : 0u;
				return WGPUPrimitiveTopology_TriangleStrip;
			case EPrimitiveType::TRIANGLELIST:
			default:
				outVertexOrIndexCount = primCount * 3u;
				return WGPUPrimitiveTopology_TriangleList;
			}
		}
	}

	class WebGPUTexture : public IRHITexture
	{
	public:
		WebGPUTexture(
			RenderDeviceWebGPU* owner,
			uint32_t w,
			uint32_t h,
			uint32_t depthOrLayers,
			uint32_t mips,
			PixelFormat logicalFmt,
			WGPUTextureFormat wgpuFmt,
			ERHITextureUsage usage,
			bool isCube,
			bool is3D,
			bool decodedS3TC,
			bool expandedRGBA8,
			WGPUTexture tex,
			WGPUTextureView srvView,
			WGPUTextureView rtvDsvView)
			: m_owner(owner)
			, m_width(w)
			, m_height(h)
			, m_depthOrLayers(depthOrLayers)
			, m_mipLevels(mips)
			, m_format(logicalFmt)
			, m_wgpuFormat(wgpuFmt)
			, m_usage(usage)
			, m_isCube(isCube)
			, m_is3D(is3D)
			, m_decodedS3TC(decodedS3TC)
			, m_expandedRGBA8(expandedRGBA8)
			, m_texture(tex)
			, m_srvView(srvView)
			, m_rtvDsvView(rtvDsvView)
		{
		}

		~WebGPUTexture() override
		{
			if (m_owner)
			{
				m_owner->NotifyTextureDestroyed(this, m_texture, m_srvView, m_rtvDsvView);
				m_srvView = nullptr;
				m_rtvDsvView = nullptr;
				m_texture = nullptr;
			}
		}

		uint32_t GetWidth() const override { return m_width; }
		uint32_t GetHeight() const override { return m_height; }
		uint32_t GetMipLevels() const override { return m_mipLevels; }
		PixelFormat GetFormat() const override { return m_format; }
		bool IsCubeMap() const override { return m_isCube; }
		void* GetNativeHandle() const override { return const_cast<WebGPUTexture*>(this); }

		WGPUTexture GetWGPUTexture() const { return m_texture; }
		WGPUTextureView GetSRVView() const { return m_srvView; }
		WGPUTextureView GetRTVDSVView() const { return m_rtvDsvView ? m_rtvDsvView : m_srvView; }
		WGPUTextureFormat GetWGPUFormat() const { return m_wgpuFormat; }
		bool Is3D() const { return m_is3D; }

		bool UpdateSubresource(
			uint32_t mipLevel,
			uint32_t arraySlice,
			int x,
			int y,
			uint32_t width,
			uint32_t height,
			const void* data,
			uint32_t rowPitch) override
		{
			if (!m_owner || !m_texture || !data || mipLevel >= m_mipLevels)
				return false;

			const uint32_t mipW = (std::max)(1u, m_width >> mipLevel);
			const uint32_t mipH = (std::max)(1u, m_height >> mipLevel);
			uint32_t ox = static_cast<uint32_t>((std::max)(0, x));
			uint32_t oy = static_cast<uint32_t>((std::max)(0, y));
			if (IsBCCompressedWGPUFormat(m_wgpuFormat))
			{
				ox &= ~3u;
				oy &= ~3u;
			}
			if (ox >= mipW || oy >= mipH)
				return true;
			const uint32_t maxW = mipW - ox;
			const uint32_t maxH = mipH - oy;
			const uint32_t upW = (width == 0) ? maxW : (std::min)(width, maxW);
			const uint32_t upH = (height == 0) ? maxH : (std::min)(height, maxH);
			if (upW == 0 || upH == 0)
				return true;

			std::vector<uint8_t> tempRGBA;
			const void* uploadPtr = data;
			uint32_t uploadRowPitch = rowPitch;

			if (m_decodedS3TC)
			{
				const size_t compSize = RHISoftwareS3TC::ComputeCompressedMipSize(m_format, upW, upH);
				if (!RHISoftwareS3TC::DecodeToRGBA8(m_format, data, compSize, upW, upH, tempRGBA))
					return false;
				uploadPtr = tempRGBA.data();
				uploadRowPitch = upW * 4u;
			}
			else if (m_expandedRGBA8)
			{
				if (!RHISoftwareS3TC::ExpandToRGBA8(m_format, data, upW, upH, rowPitch, tempRGBA))
					return false;
				uploadPtr = tempRGBA.data();
				uploadRowPitch = upW * 4u;
			}
			else if (uploadRowPitch == 0)
			{
				uploadRowPitch = GetFormatTightRowBytes(m_wgpuFormat, upW);
			}

			const uint32_t rows = GetFormatBlockRows(m_wgpuFormat, upH);
			const size_t totalBytes = (rows > 0)
				? (static_cast<size_t>(rows - 1u) * uploadRowPitch + GetFormatTightRowBytes(m_wgpuFormat, upW))
				: 0;
			if (totalBytes == 0)
				return true;

			WGPUImageCopyTexture dst = {};
			dst.texture = m_texture;
			dst.mipLevel = mipLevel;
			dst.origin.x = ox;
			dst.origin.y = oy;
			dst.origin.z = (std::min)(arraySlice, (std::max)(1u, m_depthOrLayers) - 1u);
			dst.aspect = WGPUTextureAspect_All;

			WGPUTextureDataLayout layout = {};
			layout.offset = 0;
			layout.bytesPerRow = uploadRowPitch;
			layout.rowsPerImage = rows;

			WGPUExtent3D ext = {};
			if (IsBCCompressedWGPUFormat(m_wgpuFormat))
			{
				ext.width = AlignUpU32(upW, 4u);
				ext.height = AlignUpU32(upH, 4u);
			}
			else
			{
				ext.width = upW;
				ext.height = upH;
			}
			ext.depthOrArrayLayers = 1;

			PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuQueueWriteTexture,
				m_owner->GetWGPUQueue(), &dst, uploadPtr, totalBytes, &layout, &ext);
			return true;
		}

	private:
		RenderDeviceWebGPU* m_owner = nullptr;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_depthOrLayers = 1;
		uint32_t m_mipLevels = 1;
		PixelFormat m_format = PixelFormat::A8R8G8B8;
		WGPUTextureFormat m_wgpuFormat = WGPUTextureFormat_RGBA8Unorm;
		ERHITextureUsage m_usage = ERHITextureUsage::ShaderResource;
		bool m_isCube = false;
		bool m_is3D = false;
		bool m_decodedS3TC = false;
		bool m_expandedRGBA8 = false;
		WGPUTexture m_texture = nullptr;
		WGPUTextureView m_srvView = nullptr;
		WGPUTextureView m_rtvDsvView = nullptr;
	};

	class WebGPUBuffer : public IRHIBuffer
	{
	public:
		WebGPUBuffer(WGPUBuffer borrowedGpuBuf, uint32_t alignedSize)
			: m_owner(nullptr)
			, m_byteSize(alignedSize)
			, m_alignedSize(alignedSize)
			, m_isIndex(false)
			, m_buffer(borrowedGpuBuf)
		{
		}

		WebGPUBuffer(
			RenderDeviceWebGPU* owner,
			uint32_t byteSize,
			bool isIndex,
			ERHIBufferUsage usage,
			const void* initialData)
			: m_owner(owner)
			, m_byteSize(byteSize)
			, m_isIndex(isIndex)
			, m_usage(usage)
		{
			m_alignedSize = AlignUpU32((std::max)(byteSize, 16u), 4u);
			m_cpuShadow.resize(m_alignedSize, 0);
			if (initialData && byteSize > 0)
			{
				std::memcpy(m_cpuShadow.data(), initialData, byteSize);
			}
			RecreateAndUpload();
		}

		~WebGPUBuffer() override
		{
			if (m_owner)
			{
				m_owner->NotifyBufferDestroyed(this, m_buffer);
				m_buffer = nullptr;
			}
		}

		uint32_t GetSize() const override { return m_byteSize; }
		bool IsIndexBuffer() const override { return m_isIndex; }
		void* GetNativeHandle() const override { return const_cast<WebGPUBuffer*>(this); }
		WGPUBuffer GetWGPUBuffer() const { return m_buffer; }
		uint32_t GetAlignedSize() const { return m_alignedSize; }
		const std::vector<uint8_t>& GetCpuShadow() const { return m_cpuShadow; }

		void* Map(uint32_t offset, uint32_t size, ERHIMapMode mode) override
		{
			if (offset >= m_alignedSize)
				return nullptr;
			m_mappedOffset = offset;
			m_mappedSize = (size == 0 || offset + size > m_byteSize) ? (m_byteSize - offset) : size;
			m_mappedMode = mode;
			if (mode == ERHIMapMode::WriteDiscard)
			{
				std::memset(m_cpuShadow.data() + offset, 0, m_mappedSize);
			}
			m_isMapped = true;
			return m_cpuShadow.data() + offset;
		}

		void Unmap() override
		{
			if (!m_isMapped)
				return;
			m_isMapped = false;
			if (m_mappedMode == ERHIMapMode::Read)
				return;

			if (!m_owner)
				return;
			if (m_mappedMode == ERHIMapMode::WriteDiscard)
			{
				// Allocate a fresh GPU buffer so any earlier draw commands encoded in the current pass retain their old contents
				WGPUBuffer fresh = CreateGpuBuffer();
				if (fresh)
				{
					PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuQueueWriteBuffer,
						m_owner->GetWGPUQueue(), fresh, 0, m_cpuShadow.data(), m_alignedSize);
					if (m_buffer)
						m_owner->DeferReleaseBuffer(m_buffer);
					m_buffer = fresh;
					return;
				}
			}

			if (m_buffer)
			{
				const uint32_t alignedOff = m_mappedOffset & ~3u;
				const uint32_t alignedEnd = (std::min)(m_alignedSize, AlignUpU32(m_mappedOffset + m_mappedSize, 4u));
				if (alignedEnd > alignedOff)
				{
					PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuQueueWriteBuffer,
						m_owner->GetWGPUQueue(), m_buffer, alignedOff, m_cpuShadow.data() + alignedOff, alignedEnd - alignedOff);
				}
			}
		}

		bool UpdateData(uint32_t offset, uint32_t size, const void* data) override
		{
			if (!m_owner || !data || size == 0 || offset + size > m_alignedSize)
				return false;
			std::memcpy(m_cpuShadow.data() + offset, data, size);
			if (offset == 0 && size == m_byteSize && HasFlag(m_usage, ERHIBufferUsage::Dynamic))
			{
				WGPUBuffer fresh = CreateGpuBuffer();
				if (fresh)
				{
					PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuQueueWriteBuffer,
						m_owner->GetWGPUQueue(), fresh, 0, m_cpuShadow.data(), m_alignedSize);
					if (m_buffer)
						m_owner->DeferReleaseBuffer(m_buffer);
					m_buffer = fresh;
					return true;
				}
			}
			if (!m_buffer)
				return false;
			const uint32_t alignedOff = offset & ~3u;
			const uint32_t alignedEnd = (std::min)(m_alignedSize, AlignUpU32(offset + size, 4u));
			if (alignedEnd > alignedOff)
			{
				PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuQueueWriteBuffer,
					m_owner->GetWGPUQueue(), m_buffer, alignedOff, m_cpuShadow.data() + alignedOff, alignedEnd - alignedOff);
			}
			return true;
		}

	private:
		WGPUBuffer CreateGpuBuffer() const
		{
			if (!m_owner || !m_owner->GetWGPUDevice())
				return nullptr;
			WGPUBufferDescriptor desc = {};
			desc.usage = (m_isIndex ? WGPUBufferUsage_Index : WGPUBufferUsage_Vertex) | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
			desc.size = m_alignedSize;
			desc.mappedAtCreation = 0;
			return PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuDeviceCreateBuffer, m_owner->GetWGPUDevice(), &desc);
		}

		void RecreateAndUpload()
		{
			m_buffer = CreateGpuBuffer();
			if (m_buffer && m_owner)
			{
				PARA_WGPU_CALL(m_owner->GetWGPU(), wgpuQueueWriteBuffer,
					m_owner->GetWGPUQueue(), m_buffer, 0, m_cpuShadow.data(), m_alignedSize);
			}
		}

		RenderDeviceWebGPU* m_owner = nullptr;
		uint32_t m_byteSize = 0;
		uint32_t m_alignedSize = 16;
		bool m_isIndex = false;
		ERHIBufferUsage m_usage = ERHIBufferUsage::Static;
		WGPUBuffer m_buffer = nullptr;
		std::vector<uint8_t> m_cpuShadow;
		bool m_isMapped = false;
		uint32_t m_mappedOffset = 0;
		uint32_t m_mappedSize = 0;
		ERHIMapMode m_mappedMode = ERHIMapMode::Write;
	};

	class WebGPURenderTarget : public IRHIRenderTarget
	{
	public:
		WebGPURenderTarget(
			uint32_t w,
			uint32_t h,
			PixelFormat fmt,
			const std::vector<WebGPUTexture*>& colors,
			WebGPUTexture* depth)
			: m_width(w)
			, m_height(h)
			, m_format(fmt)
			, m_colors(colors)
			, m_depth(depth)
		{
		}

		~WebGPURenderTarget() override
		{
			for (WebGPUTexture* c : m_colors)
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
		void* GetNativeHandle() const override { return const_cast<WebGPURenderTarget*>(this); }

		uint32_t GetColorCount() const { return static_cast<uint32_t>(m_colors.size()); }
		WebGPUTexture* GetWebGPUColor(uint32_t idx) const { return (idx < m_colors.size()) ? m_colors[idx] : nullptr; }
		WebGPUTexture* GetWebGPUDepth() const { return m_depth; }

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		PixelFormat m_format = PixelFormat::A8R8G8B8;
		std::vector<WebGPUTexture*> m_colors;
		WebGPUTexture* m_depth = nullptr;
	};

	class WebGPUOcclusionQuery : public IRHIOcclusionQuery
	{
	public:
		explicit WebGPUOcclusionQuery(RenderDeviceWebGPU* owner)
			: m_owner(owner)
		{
			if (m_owner)
				m_slot = m_owner->AllocateOcclusionSlot();
		}

		~WebGPUOcclusionQuery() override
		{
			if (m_owner && m_slot != UINT32_MAX)
				m_owner->FreeOcclusionSlot(m_slot);
		}

		bool Begin() override
		{
			if (!m_owner || m_slot == UINT32_MAX)
				return false;
			m_owner->BeginOcclusionOnActivePass(m_slot);
			return true;
		}

		void End() override
		{
			if (m_owner && m_slot != UINT32_MAX)
				m_owner->EndOcclusionOnActivePass();
		}

		bool GetResult(uint64_t& outSamplesPassed, bool flush) override
		{
			if (!m_owner || m_slot == UINT32_MAX)
			{
				outSamplesPassed = 1;
				return true;
			}
			return m_owner->ReadOcclusionSlotValue(m_slot, flush, outSamplesPassed);
		}

	private:
		RenderDeviceWebGPU* m_owner = nullptr;
		uint32_t m_slot = UINT32_MAX;
	};

	struct WebGPUShaderReflection
	{
		bool hasVsUbo = false;
		uint32_t vsUboMinBytes = 16;
		bool hasPsUbo = false;
		uint32_t psUboMinBytes = 16;
		WGPUShaderStageFlags texVisibility[16] = {};
		WGPUTextureViewDimension texViewDim[16] = {};
		WGPUShaderStageFlags smpVisibility[16] = {};
		uint32_t vsInputLocMask = 0;
		uint32_t psOutputLocMask = 0;
	};

	namespace
	{
		void ReflectWgslStage(const std::string& src, bool isVertex, WebGPUShaderReflection& ioRefl)
		{
			const WGPUShaderStageFlags stageFlag = isVertex ? WGPUShaderStage_Vertex : WGPUShaderStage_Fragment;

			// Scan `array<vec4<f32>, N>` inside ParaGlobalsBuffer for exact UBO size
			uint32_t uboBytes = 16u;
			size_t gbPos = src.find("struct ParaGlobalsBuffer");
			if (gbPos != std::string::npos)
			{
				size_t arrPos = src.find("array<vec4<f32>,", gbPos);
				if (arrPos != std::string::npos)
				{
					size_t numStart = arrPos + std::strlen("array<vec4<f32>,");
					while (numStart < src.size() && std::isspace(static_cast<unsigned char>(src[numStart])))
						++numStart;
					uint32_t vec4Count = 0;
					while (numStart < src.size() && std::isdigit(static_cast<unsigned char>(src[numStart])))
					{
						vec4Count = vec4Count * 10u + static_cast<uint32_t>(src[numStart] - '0');
						++numStart;
					}
					if (vec4Count > 0)
						uboBytes = vec4Count * 16u;
				}
			}

			// Scan all `@binding(N)` declarations
			size_t pos = 0;
			while ((pos = src.find("@binding(", pos)) != std::string::npos)
			{
				size_t numPos = pos + 9;
				uint32_t binding = 0;
				bool hasDigit = false;
				while (numPos < src.size() && std::isdigit(static_cast<unsigned char>(src[numPos])))
				{
					binding = binding * 10u + static_cast<uint32_t>(src[numPos] - '0');
					hasDigit = true;
					++numPos;
				}
				size_t lineEnd = src.find('\n', numPos);
				if (lineEnd == std::string::npos)
					lineEnd = src.size();
				std::string line = src.substr(pos, lineEnd - pos);

				if (hasDigit)
				{
					if (binding == 0 && isVertex)
					{
						ioRefl.hasVsUbo = true;
						ioRefl.vsUboMinBytes = (std::max)(ioRefl.vsUboMinBytes, uboBytes);
					}
					else if (binding == 1 && !isVertex)
					{
						ioRefl.hasPsUbo = true;
						ioRefl.psUboMinBytes = (std::max)(ioRefl.psUboMinBytes, uboBytes);
					}
					else if (binding >= 16u && binding < 32u)
					{
						const uint32_t slot = binding - 16u;
						ioRefl.texVisibility[slot] |= stageFlag;
						if (line.find("texture_cube<") != std::string::npos)
							ioRefl.texViewDim[slot] = WGPUTextureViewDimension_Cube;
						else if (line.find("texture_3d<") != std::string::npos)
							ioRefl.texViewDim[slot] = WGPUTextureViewDimension_3D;
						else if (ioRefl.texViewDim[slot] == WGPUTextureViewDimension_Undefined)
							ioRefl.texViewDim[slot] = WGPUTextureViewDimension_2D;
					}
					else if (binding >= 32u && binding < 48u)
					{
						const uint32_t slot = binding - 32u;
						ioRefl.smpVisibility[slot] |= stageFlag;
					}
				}
				pos = numPos;
			}

			// Scan `@location(L) attr_` in VS or `@location(L) color_` in PS
			pos = 0;
			while ((pos = src.find("@location(", pos)) != std::string::npos)
			{
				size_t numPos = pos + 10;
				uint32_t loc = 0;
				bool hasDigit = false;
				while (numPos < src.size() && std::isdigit(static_cast<unsigned char>(src[numPos])))
				{
					loc = loc * 10u + static_cast<uint32_t>(src[numPos] - '0');
					hasDigit = true;
					++numPos;
				}
				size_t lineEnd = src.find('\n', numPos);
				if (lineEnd == std::string::npos)
					lineEnd = src.size();
				std::string rest = src.substr(numPos, lineEnd - numPos);
				if (hasDigit)
				{
					if (isVertex && loc < 16u && rest.find("attr_") != std::string::npos)
						ioRefl.vsInputLocMask |= (1u << loc);
					else if (!isVertex && loc < 8u && rest.find("color_") != std::string::npos)
						ioRefl.psOutputLocMask |= (1u << loc);
				}
				pos = numPos;
			}
		}
	}

	class WebGPUShaderProgram : public IRHIShaderProgram
	{
	public:
		WebGPUShaderProgram(
			RenderDeviceWebGPU* owner,
			WGPUShaderModule vsMod,
			WGPUShaderModule psMod,
			const std::string& vsEntry,
			const std::string& psEntry,
			WGPUBindGroupLayout bgl,
			WGPUPipelineLayout pipeLayout,
			const WebGPUShaderReflection& refl,
			uint64_t hash)
			: m_owner(owner)
			, m_vsModule(vsMod)
			, m_psModule(psMod)
			, m_vsEntry(vsEntry)
			, m_psEntry(psEntry)
			, m_bindGroupLayout(bgl)
			, m_pipelineLayout(pipeLayout)
			, m_refl(refl)
			, m_hash(hash)
		{
		}

		~WebGPUShaderProgram() override
		{
			if (m_owner)
			{
				const WebGPUDispatch& wgpu = m_owner->GetWGPU();
				if (m_pipelineLayout)
					PARA_WGPU_CALL(wgpu, wgpuPipelineLayoutRelease, m_pipelineLayout);
				if (m_bindGroupLayout)
					PARA_WGPU_CALL(wgpu, wgpuBindGroupLayoutRelease, m_bindGroupLayout);
				if (m_vsModule)
					PARA_WGPU_CALL(wgpu, wgpuShaderModuleRelease, m_vsModule);
				if (m_psModule)
					PARA_WGPU_CALL(wgpu, wgpuShaderModuleRelease, m_psModule);
			}
		}

		bool IsValid() const override { return m_vsModule != nullptr && m_pipelineLayout != nullptr; }
		void* GetNativeHandle() const override { return const_cast<WebGPUShaderProgram*>(this); }

		WGPUShaderModule GetVSModule() const { return m_vsModule; }
		WGPUShaderModule GetPSModule() const { return m_psModule; }
		const char* GetVSEntry() const { return m_vsEntry.c_str(); }
		const char* GetPSEntry() const { return m_psEntry.c_str(); }
		WGPUBindGroupLayout GetBindGroupLayout() const { return m_bindGroupLayout; }
		WGPUPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
		const WebGPUShaderReflection& GetReflection() const { return m_refl; }
		uint64_t GetHash() const { return m_hash; }

	private:
		RenderDeviceWebGPU* m_owner = nullptr;
		WGPUShaderModule m_vsModule = nullptr;
		WGPUShaderModule m_psModule = nullptr;
		std::string m_vsEntry;
		std::string m_psEntry;
		WGPUBindGroupLayout m_bindGroupLayout = nullptr;
		WGPUPipelineLayout m_pipelineLayout = nullptr;
		WebGPUShaderReflection m_refl;
		uint64_t m_hash = 0;
	};

	RenderDeviceWebGPU::RenderDeviceWebGPU(
		const WebGPUDispatch& wgpu,
		WGPUInstance instance,
		WGPUSurface surface,
		WGPUAdapter adapter,
		WGPUDevice device,
		WGPUQueue queue,
		WGPUTextureFormat surfaceFormat,
		bool supportsBC,
		bool supportsFloat32Filterable,
		const std::string& adapterName)
		: m_wgpu(wgpu)
		, m_instance(instance)
		, m_surface(surface)
		, m_adapter(adapter)
		, m_device(device)
		, m_queue(queue)
		, m_surfaceFormat(surfaceFormat)
		, m_supportsBC(supportsBC)
		, m_supportsFloat32Filterable(supportsFloat32Filterable)
		, m_adapterNameStr(adapterName)
	{
		InitCaps();
	}

	RenderDeviceWebGPU::~RenderDeviceWebGPU()
	{
		EndActiveRenderPass();
		ExecuteAndSyncGPU();
		FlushFrameTransientBuffers();

		for (auto& kv : m_psoCache)
		{
			if (kv.second)
				PARA_WGPU_CALL(m_wgpu, wgpuRenderPipelineRelease, kv.second);
		}
		m_psoCache.clear();

		for (auto& kv : m_blitPipelines)
		{
			if (kv.second)
				PARA_WGPU_CALL(m_wgpu, wgpuRenderPipelineRelease, kv.second);
		}
		m_blitPipelines.clear();
		if (m_blitPipeLayout)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuPipelineLayoutRelease, m_blitPipeLayout);
			m_blitPipeLayout = nullptr;
		}
		if (m_blitBgl)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuBindGroupLayoutRelease, m_blitBgl);
			m_blitBgl = nullptr;
		}
		if (m_blitShaderModule)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuShaderModuleRelease, m_blitShaderModule);
			m_blitShaderModule = nullptr;
		}

		for (auto& kv : m_samplerCache)
		{
			if (kv.second)
				PARA_WGPU_CALL(m_wgpu, wgpuSamplerRelease, kv.second);
		}
		m_samplerCache.clear();

		delete m_defaultDepthTex;
		m_defaultDepthTex = nullptr;
		delete m_fallbackWhite2D;
		m_fallbackWhite2D = nullptr;
		delete m_fallbackWhiteCube;
		m_fallbackWhiteCube = nullptr;
		delete m_fallbackWhite3D;
		m_fallbackWhite3D = nullptr;
		FlushFrameTransientBuffers();

		auto releaseBuf = [&](WGPUBuffer& b) {
			if (b)
			{
				if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
					PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, b);
				PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, b);
				b = nullptr;
			}
		};
		releaseBuf(m_fallbackZeroVB);
		releaseBuf(m_zeroUboBuffer);
		releaseBuf(m_currentVsUbo);
		releaseBuf(m_currentPsUbo);
		releaseBuf(m_occlusionResolveBuf);

		if (m_occlusionQuerySet)
		{
			if (PARA_WGPU_HAS(m_wgpu, wgpuQuerySetDestroy))
				PARA_WGPU_CALL(m_wgpu, wgpuQuerySetDestroy, m_occlusionQuerySet);
			if (PARA_WGPU_HAS(m_wgpu, wgpuQuerySetRelease))
				PARA_WGPU_CALL(m_wgpu, wgpuQuerySetRelease, m_occlusionQuerySet);
			m_occlusionQuerySet = nullptr;
		}

		if (m_currentSurfaceView)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureViewRelease, m_currentSurfaceView);
			m_currentSurfaceView = nullptr;
		}
		if (m_currentSurfaceTex)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, m_currentSurfaceTex);
			m_currentSurfaceTex = nullptr;
		}

		if (m_surface)
		{
			if (PARA_WGPU_HAS(m_wgpu, wgpuSurfaceUnconfigure))
				PARA_WGPU_CALL(m_wgpu, wgpuSurfaceUnconfigure, m_surface);
			PARA_WGPU_CALL(m_wgpu, wgpuSurfaceRelease, m_surface);
			m_surface = nullptr;
		}
		if (m_queue)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuQueueRelease, m_queue);
			m_queue = nullptr;
		}
		if (m_device)
		{
			if (PARA_WGPU_HAS(m_wgpu, wgpuDeviceDestroy))
				PARA_WGPU_CALL(m_wgpu, wgpuDeviceDestroy, m_device);
			PARA_WGPU_CALL(m_wgpu, wgpuDeviceRelease, m_device);
			m_device = nullptr;
		}
		if (m_adapter)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuAdapterRelease, m_adapter);
			m_adapter = nullptr;
		}
		if (m_instance)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuInstanceRelease, m_instance);
			m_instance = nullptr;
		}
		m_wgpu.Unload();
	}

	void RenderDeviceWebGPU::InitCaps()
	{
		m_caps = RHIDeviceCaps{};
		m_caps.backend = ERHIBackend::WebGPU;
		m_caps.backendName = "WebGPU";
		m_caps.adapterName = m_adapterNameStr.c_str();
		m_caps.featureLevel = 0xb000;
		m_caps.maxSimultaneousRTs = 4;
		m_caps.maxTextureStages = 16;
		m_caps.maxTextureSize = 8192;
		m_caps.maxAnisotropy = 16;
		m_caps.maxVertexUniformVectors = 4096;
		m_caps.supportsFloatRenderTargets = true;
		m_caps.supportsDepthTextures = true;
		m_caps.supportsInstancing = true;
		m_caps.supportsOcclusionQuery = PARA_WGPU_HAS(m_wgpu, wgpuDeviceCreateQuerySet);
		m_caps.supportsTwoSidedStencil = true;
		m_caps.supportsUserClipPlanes = false;
		m_caps.supportsBCCompression = m_supportsBC;
		m_caps.supportsETC2Compression = false;
		m_caps.supportsASTCCompression = false;
		m_caps.supportsAsyncReadback = true;
		m_caps.supportsRuntimeShaderCompile = true;
		m_caps.topLeftScreenOrigin = true;
		m_caps.clipSpaceZeroToOne = true;
		m_caps.needsHalfPixelOffset = false;
	}

	bool RenderDeviceWebGPU::ConfigureSurface(uint32_t width, uint32_t height, bool vsync)
	{
		if (!m_surface || !m_device)
			return false;
		WGPUTextureUsageFlags desiredUsage =
			WGPUTextureUsage_RenderAttachment |
			WGPUTextureUsage_CopySrc |
			WGPUTextureUsage_CopyDst |
			WGPUTextureUsage_TextureBinding;
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
		if (m_adapter)
		{
			WGPUSurfaceCapabilities surfCaps = {};
			if (PARA_WGPU_CALL(m_wgpu, wgpuSurfaceGetCapabilities, m_surface, m_adapter, &surfCaps) == WGPUStatus_Success)
			{
				if (surfCaps.usages != 0)
					desiredUsage &= surfCaps.usages;
				PARA_WGPU_CALL(m_wgpu, wgpuSurfaceCapabilitiesFreeMembers, surfCaps);
			}
		}
#else
		desiredUsage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
#endif
		desiredUsage |= WGPUTextureUsage_RenderAttachment;
		m_surfaceUsage = desiredUsage;

		WGPUSurfaceConfiguration surfCfg = {};
		surfCfg.device = m_device;
		surfCfg.format = m_surfaceFormat;
		surfCfg.usage = desiredUsage;
		surfCfg.viewFormatCount = 0;
		surfCfg.viewFormats = nullptr;
		surfCfg.alphaMode = WGPUCompositeAlphaMode_Auto;
		surfCfg.width = (std::max)(1u, width);
		surfCfg.height = (std::max)(1u, height);
		surfCfg.presentMode = vsync ? WGPUPresentMode_Fifo : WGPUPresentMode_Immediate;
		PARA_WGPU_CALL(m_wgpu, wgpuSurfaceConfigure, m_surface, &surfCfg);
		return true;
	}

	bool RenderDeviceWebGPU::CreateDefaultResources(uint32_t width, uint32_t height)
	{
		delete m_defaultDepthTex;
		m_defaultDepthTex = static_cast<WebGPUTexture*>(CreateTexture(
			(std::max)(1u, width),
			(std::max)(1u, height),
			PixelFormat::D24S8,
			1,
			ERHITextureUsage::DepthStencil));
		if (!m_defaultDepthTex)
			return false;

		if (!m_fallbackWhite2D)
		{
			const uint32_t whitePx = 0xFFFFFFFFu;
			RHIMipData mip{ &whitePx, 4, 4 };
			m_fallbackWhite2D = static_cast<WebGPUTexture*>(CreateTexture(
				1, 1, PixelFormat::A8R8G8B8, 1, ERHITextureUsage::ShaderResource, &mip, 1));
		}

		if (!m_fallbackWhiteCube)
		{
			WGPUTextureDescriptor td = {};
			td.label = ParaWgpuLabel("ParaFallbackWhiteCube");
			td.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
			td.dimension = WGPUTextureDimension_2D;
			td.size = { 1, 1, 6 };
			td.format = WGPUTextureFormat_RGBA8Unorm;
			td.mipLevelCount = 1;
			td.sampleCount = 1;
			WGPUTexture cubeTex = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateTexture, m_device, &td);
			if (cubeTex)
			{
				const uint32_t whiteFaces[6] = {
					0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
					0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu
				};
				WGPUImageCopyTexture dst = {};
				dst.texture = cubeTex;
				dst.mipLevel = 0;
				dst.aspect = WGPUTextureAspect_All;
				WGPUTextureDataLayout lay = {};
				lay.bytesPerRow = 4;
				lay.rowsPerImage = 1;
				WGPUExtent3D ext = { 1, 1, 6 };
				PARA_WGPU_CALL(m_wgpu, wgpuQueueWriteTexture, m_queue, &dst, whiteFaces, sizeof(whiteFaces), &lay, &ext);

				WGPUTextureViewDescriptor vd = {};
				vd.format = WGPUTextureFormat_RGBA8Unorm;
				vd.dimension = WGPUTextureViewDimension_Cube;
				vd.baseMipLevel = 0;
				vd.mipLevelCount = 1;
				vd.baseArrayLayer = 0;
				vd.arrayLayerCount = 6;
				vd.aspect = WGPUTextureAspect_All;
				WGPUTextureView cubeView = PARA_WGPU_CALL(m_wgpu, wgpuTextureCreateView, cubeTex, &vd);
				m_fallbackWhiteCube = new WebGPUTexture(
					this, 1, 1, 6, 1, PixelFormat::A8R8G8B8, WGPUTextureFormat_RGBA8Unorm,
					ERHITextureUsage::ShaderResource | ERHITextureUsage::CubeMap,
					true, false, false, false, cubeTex, cubeView, nullptr);
			}
		}

		if (!m_fallbackWhite3D)
		{
			WGPUTextureDescriptor td = {};
			td.label = ParaWgpuLabel("ParaFallbackWhite3D");
			td.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
			td.dimension = WGPUTextureDimension_3D;
			td.size = { 1, 1, 1 };
			td.format = WGPUTextureFormat_RGBA8Unorm;
			td.mipLevelCount = 1;
			td.sampleCount = 1;
			WGPUTexture tex3d = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateTexture, m_device, &td);
			if (tex3d)
			{
				const uint32_t whitePx = 0xFFFFFFFFu;
				WGPUImageCopyTexture dst = {};
				dst.texture = tex3d;
				dst.mipLevel = 0;
				dst.aspect = WGPUTextureAspect_All;
				WGPUTextureDataLayout lay = {};
				lay.bytesPerRow = 4;
				lay.rowsPerImage = 1;
				WGPUExtent3D ext = { 1, 1, 1 };
				PARA_WGPU_CALL(m_wgpu, wgpuQueueWriteTexture, m_queue, &dst, &whitePx, 4, &lay, &ext);

				WGPUTextureViewDescriptor vd = {};
				vd.format = WGPUTextureFormat_RGBA8Unorm;
				vd.dimension = WGPUTextureViewDimension_3D;
				vd.baseMipLevel = 0;
				vd.mipLevelCount = 1;
				vd.baseArrayLayer = 0;
				vd.arrayLayerCount = 1;
				vd.aspect = WGPUTextureAspect_All;
				WGPUTextureView v3d = PARA_WGPU_CALL(m_wgpu, wgpuTextureCreateView, tex3d, &vd);
				m_fallbackWhite3D = new WebGPUTexture(
					this, 1, 1, 1, 1, PixelFormat::A8R8G8B8, WGPUTextureFormat_RGBA8Unorm,
					ERHITextureUsage::ShaderResource,
					false, true, false, false, tex3d, v3d, nullptr);
			}
		}

		if (!m_fallbackZeroVB)
		{
			const float defaultAttr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			WGPUBufferDescriptor bd = {};
			bd.label = ParaWgpuLabel("ParaFallbackZeroVB");
			bd.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
			bd.size = sizeof(defaultAttr);
			m_fallbackZeroVB = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &bd);
			if (m_fallbackZeroVB)
				PARA_WGPU_CALL(m_wgpu, wgpuQueueWriteBuffer, m_queue, m_fallbackZeroVB, 0, defaultAttr, sizeof(defaultAttr));
		}

		if (!m_zeroUboBuffer)
		{
			std::vector<uint8_t> zeros(4096, 0);
			WGPUBufferDescriptor bd = {};
			bd.label = ParaWgpuLabel("ParaZeroUBO");
			bd.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
			bd.size = zeros.size();
			m_zeroUboBuffer = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &bd);
			if (m_zeroUboBuffer)
				PARA_WGPU_CALL(m_wgpu, wgpuQueueWriteBuffer, m_queue, m_zeroUboBuffer, 0, zeros.data(), zeros.size());
		}

		if (!m_occlusionQuerySet && PARA_WGPU_HAS(m_wgpu, wgpuDeviceCreateQuerySet))
		{
			WGPUQuerySetDescriptor qsd = {};
			qsd.label = ParaWgpuLabel("ParaOcclusionQuerySet");
			qsd.type = WGPUQueryType_Occlusion;
			qsd.count = MAX_OCCLUSION_QUERIES;
			m_occlusionQuerySet = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateQuerySet, m_device, &qsd);
			if (m_occlusionQuerySet)
			{
				WGPUBufferDescriptor rbd = {};
				rbd.label = ParaWgpuLabel("ParaOcclusionResolveBuf");
				rbd.usage = WGPUBufferUsage_QueryResolve | WGPUBufferUsage_CopySrc;
				rbd.size = static_cast<uint64_t>(MAX_OCCLUSION_QUERIES) * sizeof(uint64_t);
				m_occlusionResolveBuf = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &rbd);

				m_freeOcclusionSlots.reserve(MAX_OCCLUSION_QUERIES);
				for (uint32_t i = MAX_OCCLUSION_QUERIES; i > 0; --i)
					m_freeOcclusionSlots.push_back(i - 1u);
			}
		}

		return true;
	}

	bool RenderDeviceWebGPU::Initialize(uint32_t width, uint32_t height, bool vsync)
	{
		m_backBufferWidth = (std::max)(1u, width);
		m_backBufferHeight = (std::max)(1u, height);
		m_activeRTWidth = m_backBufferWidth;
		m_activeRTHeight = m_backBufferHeight;
		m_vsync = vsync;

		if (!ConfigureSurface(m_backBufferWidth, m_backBufferHeight, m_vsync))
			return false;
		if (!CreateDefaultResources(m_backBufferWidth, m_backBufferHeight))
			return false;

		ParaViewport vp{};
		vp.X = 0;
		vp.Y = 0;
		vp.Width = m_backBufferWidth;
		vp.Height = m_backBufferHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		m_state.SetViewport(vp);

		RECT sc{ 0, 0, static_cast<LONG>(m_backBufferWidth), static_cast<LONG>(m_backBufferHeight) };
		m_state.SetScissorRect(&sc);
		return true;
	}

	bool RenderDeviceWebGPU::ResizeSwapChain(uint32_t width, uint32_t height, bool vsync)
	{
		EndActiveRenderPass();
		ExecuteAndSyncGPU();

		if (m_currentSurfaceView)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureViewRelease, m_currentSurfaceView);
			m_currentSurfaceView = nullptr;
		}
		if (m_currentSurfaceTex)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, m_currentSurfaceTex);
			m_currentSurfaceTex = nullptr;
		}

		const uint32_t prevBackW = m_backBufferWidth;
		const uint32_t prevBackH = m_backBufferHeight;
		m_backBufferWidth = (std::max)(1u, width);
		m_backBufferHeight = (std::max)(1u, height);
		m_vsync = vsync;
		if (m_boundRTCount == 0)
		{
			m_activeRTWidth = m_backBufferWidth;
			m_activeRTHeight = m_backBufferHeight;
			const ParaViewport& curVp = m_state.GetViewport();
			if (curVp.X == 0 && curVp.Y == 0 && curVp.Width == prevBackW && curVp.Height == prevBackH)
			{
				ParaViewport newVp = curVp;
				newVp.Width = m_backBufferWidth;
				newVp.Height = m_backBufferHeight;
				m_state.SetViewport(newVp);
			}
		}

		if (!ConfigureSurface(m_backBufferWidth, m_backBufferHeight, m_vsync))
			return false;
		return CreateDefaultResources(m_backBufferWidth, m_backBufferHeight);
	}

	void RenderDeviceWebGPU::NotifyBufferDestroyed(WebGPUBuffer* wrapper, WGPUBuffer buf)
	{
		for (uint32_t s = 0; s < MAX_VERTEX_STREAMS; ++s)
		{
			if (m_streams[s].buffer == wrapper)
				m_streams[s] = {};
		}
		if (m_boundIndexBuffer == wrapper)
			m_boundIndexBuffer = nullptr;
		if (buf)
			m_transientBuffers.push_back(buf);
	}

	void RenderDeviceWebGPU::NotifyTextureDestroyed(WebGPUTexture* wrapper, WGPUTexture tex, WGPUTextureView srvView, WGPUTextureView rtvView)
	{
		for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
		{
			if (m_boundTextures[s] == wrapper)
				m_boundTextures[s] = nullptr;
		}
		if (srvView)
			m_transientTextureViews.push_back(srvView);
		if (rtvView && rtvView != srvView)
			m_transientTextureViews.push_back(rtvView);
		if (tex)
			m_transientTextures.push_back(tex);
	}

	void RenderDeviceWebGPU::FlushFrameTransientBuffers()
	{
		for (WGPUBindGroup bg : m_transientBindGroups)
		{
			if (bg)
				PARA_WGPU_CALL(m_wgpu, wgpuBindGroupRelease, bg);
		}
		m_transientBindGroups.clear();

		for (WGPUBuffer buf : m_transientBuffers)
		{
			if (buf)
			{
				if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
					PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, buf);
				PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, buf);
			}
		}
		m_transientBuffers.clear();

		for (WGPUTextureView view : m_transientTextureViews)
		{
			if (view)
				PARA_WGPU_CALL(m_wgpu, wgpuTextureViewRelease, view);
		}
		m_transientTextureViews.clear();

		for (WGPUTexture tex : m_transientTextures)
		{
			if (tex)
			{
				if (PARA_WGPU_HAS(m_wgpu, wgpuTextureDestroy))
					PARA_WGPU_CALL(m_wgpu, wgpuTextureDestroy, tex);
				PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, tex);
			}
		}
		m_transientTextures.clear();

		m_currentVsUbo = nullptr;
		m_currentVsUboSize = 0;
		m_vsUboDirty = true;
		m_currentPsUbo = nullptr;
		m_currentPsUboSize = 0;
		m_psUboDirty = true;
	}

	WGPUBuffer RenderDeviceWebGPU::AllocateTransientBuffer(WGPUBufferUsageFlags usage, const void* data, uint32_t byteSize, uint32_t minSize)
	{
		if (usage == 0 || !m_device)
			return nullptr;
		const uint32_t allocBytes = AlignUpU32((std::max)(byteSize, (std::max)(minSize, 16u)), 4u);
		WGPUBufferDescriptor bd = {};
		bd.usage = usage | WGPUBufferUsage_CopyDst;
		bd.size = allocBytes;
		WGPUBuffer buf = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &bd);
		if (!buf)
			return nullptr;
		if (data && byteSize > 0)
		{
			if ((byteSize & 3u) == 0)
			{
				PARA_WGPU_CALL(m_wgpu, wgpuQueueWriteBuffer, m_queue, buf, 0, data, byteSize);
			}
			else
			{
				std::vector<uint8_t> padded(AlignUpU32(byteSize, 4u), 0);
				std::memcpy(padded.data(), data, byteSize);
				PARA_WGPU_CALL(m_wgpu, wgpuQueueWriteBuffer, m_queue, buf, 0, padded.data(), padded.size());
			}
		}
		m_transientBuffers.push_back(buf);
		return buf;
	}

	bool RenderDeviceWebGPU::EnsureSwapChainTexture()
	{
		if (m_currentSurfaceView && m_currentSurfaceTex)
			return true;
		if (!m_surface)
			return false;

#if defined(__EMSCRIPTEN__)
		int canvasW = 0, canvasH = 0;
		if (emscripten_get_canvas_element_size("#canvas", &canvasW, &canvasH) == EMSCRIPTEN_RESULT_SUCCESS &&
			canvasW > 0 && canvasH > 0)
		{
			const uint32_t cw = static_cast<uint32_t>(canvasW);
			const uint32_t ch = static_cast<uint32_t>(canvasH);
			if (cw != m_backBufferWidth || ch != m_backBufferHeight)
			{
				ResizeSwapChain(cw, ch, m_vsync);
			}
		}
#endif

		WGPUSurfaceTexture surfTex = {};
		PARA_WGPU_CALL(m_wgpu, wgpuSurfaceGetCurrentTexture, m_surface, &surfTex);
		if (surfTex.status == WGPUSurfaceGetCurrentTextureStatus_Outdated ||
			surfTex.status == WGPUSurfaceGetCurrentTextureStatus_Lost)
		{
			if (surfTex.texture)
				PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, surfTex.texture);
			ConfigureSurface(m_backBufferWidth, m_backBufferHeight, m_vsync);
			surfTex = {};
			PARA_WGPU_CALL(m_wgpu, wgpuSurfaceGetCurrentTexture, m_surface, &surfTex);
		}
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
		const bool surfOk = (surfTex.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal ||
			surfTex.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal);
#else
		const bool surfOk = (surfTex.status == WGPUSurfaceGetCurrentTextureStatus_Success);
#endif
		if (!surfTex.texture || !surfOk)
		{
			if (surfTex.texture)
				PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, surfTex.texture);
			return false;
		}

		m_currentSurfaceTex = surfTex.texture;
		WGPUTextureViewDescriptor vd = {};
		vd.format = m_surfaceFormat;
		vd.dimension = WGPUTextureViewDimension_2D;
		vd.baseMipLevel = 0;
		vd.mipLevelCount = 1;
		vd.baseArrayLayer = 0;
		vd.arrayLayerCount = 1;
		vd.aspect = WGPUTextureAspect_All;
		m_currentSurfaceView = PARA_WGPU_CALL(m_wgpu, wgpuTextureCreateView, m_currentSurfaceTex, &vd);
		if (!m_currentSurfaceView)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, m_currentSurfaceTex);
			m_currentSurfaceTex = nullptr;
			return false;
		}
		return true;
	}

	WGPUCommandEncoder RenderDeviceWebGPU::EnsureEncoder()
	{
		if (!m_cmdEncoder && m_device)
		{
			WGPUCommandEncoderDescriptor ed = {};
			ed.label = ParaWgpuLabel("ParaFrameCommandEncoder");
			m_cmdEncoder = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateCommandEncoder, m_device, &ed);
		}
		return m_cmdEncoder;
	}

	void RenderDeviceWebGPU::EndActiveRenderPass()
	{
		if (!m_renderPass)
			return;
		if (m_occlusionActiveInPass)
		{
			if (PARA_WGPU_HAS(m_wgpu, wgpuRenderPassEncoderEndOcclusionQuery))
				PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderEndOcclusionQuery, m_renderPass);
			m_occlusionActiveInPass = false;
		}
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderEnd, m_renderPass);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderRelease, m_renderPass);
		m_renderPass = nullptr;

		if (!m_pendingOcclusionResolveSlots.empty() &&
			m_cmdEncoder && m_occlusionQuerySet && m_occlusionResolveBuf &&
			PARA_WGPU_HAS(m_wgpu, wgpuCommandEncoderResolveQuerySet))
		{
			for (uint32_t slot : m_pendingOcclusionResolveSlots)
			{
				PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderResolveQuerySet,
					m_cmdEncoder, m_occlusionQuerySet, slot, 1,
					m_occlusionResolveBuf, static_cast<uint64_t>(slot) * sizeof(uint64_t));
			}
			m_pendingOcclusionResolveSlots.clear();
		}
	}

	void RenderDeviceWebGPU::ApplyClampedViewport(const ParaViewport& vp)
	{
		if (!m_renderPass || m_activeRTWidth == 0 || m_activeRTHeight == 0)
			return;
		const float rtW = static_cast<float>(m_activeRTWidth);
		const float rtH = static_cast<float>(m_activeRTHeight);
		const float x = (std::max)(0.0f, (std::min)(static_cast<float>(vp.X), rtW));
		const float y = (std::max)(0.0f, (std::min)(static_cast<float>(vp.Y), rtH));
		const float reqW = (vp.Width > 0) ? static_cast<float>(vp.Width) : rtW;
		const float reqH = (vp.Height > 0) ? static_cast<float>(vp.Height) : rtH;
		const float w = (std::max)(0.0f, (std::min)(reqW, rtW - x));
		const float h = (std::max)(0.0f, (std::min)(reqH, rtH - y));
		const float minZ = std::isfinite(vp.MinZ) ? (std::max)(0.0f, (std::min)(vp.MinZ, 1.0f)) : 0.0f;
		const float maxZ = std::isfinite(vp.MaxZ) ? (std::max)(0.0f, (std::min)(vp.MaxZ, 1.0f)) : 1.0f;
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetViewport, m_renderPass, x, y, w, h, minZ, maxZ);
	}

	bool RenderDeviceWebGPU::EnsureActiveRenderPass(bool clearColor, bool clearDepth, bool clearStencil)
	{
		if (m_renderPass && !clearColor && !clearDepth && !clearStencil)
			return true;

		EndActiveRenderPass();

		WGPURenderPassColorAttachment colorAtts[MAX_RENDER_TARGETS] = {};
		uint32_t colorAttCount = 0;
		WebGPUTexture* depthTex = nullptr;
		bool effClearDepth = clearDepth;
		bool effClearStencil = clearStencil;

		const Color4f& cc = m_state.GetClearColor();
		WGPUColor wgpuClearColor{ cc.r, cc.g, cc.b, cc.a };

		if (m_boundRTCount > 0)
		{
			for (uint32_t i = 0; i < m_boundRTCount && i < MAX_RENDER_TARGETS; ++i)
			{
				WebGPURenderTarget* rt = m_boundRTs[i];
				if (!rt)
					continue;
				const uint32_t subCount = rt->GetColorCount();
				if (i == 0 && subCount > 1)
				{
					for (uint32_t j = 0; j < subCount && colorAttCount < MAX_RENDER_TARGETS; ++j)
					{
						WebGPUTexture* ct = rt->GetWebGPUColor(j);
						if (!ct || !ct->GetRTVDSVView())
							continue;
						WGPURenderPassColorAttachment& ca = colorAtts[colorAttCount++];
						ca.view = ct->GetRTVDSVView();
						ca.depthSlice = 0xFFFFFFFFu;
						ca.loadOp = clearColor ? WGPULoadOp_Clear : WGPULoadOp_Load;
						ca.storeOp = WGPUStoreOp_Store;
						ca.clearValue = wgpuClearColor;
					}
					break;
				}
				WebGPUTexture* ct = rt->GetWebGPUColor(0);
				if (ct && ct->GetRTVDSVView())
				{
					WGPURenderPassColorAttachment& ca = colorAtts[colorAttCount++];
					ca.view = ct->GetRTVDSVView();
					ca.depthSlice = 0xFFFFFFFFu;
					ca.loadOp = clearColor ? WGPULoadOp_Clear : WGPULoadOp_Load;
					ca.storeOp = WGPUStoreOp_Store;
					ca.clearValue = wgpuClearColor;
				}
			}
			if (m_boundDepthOverride && m_boundDepthOverride->GetWebGPUDepth())
				depthTex = m_boundDepthOverride->GetWebGPUDepth();
			else if (m_boundRTs[0] && m_boundRTs[0]->GetWebGPUDepth())
				depthTex = m_boundRTs[0]->GetWebGPUDepth();
		}
		else
		{
			// Must resolve swapchain BEFORE EnsureEncoder(), as ResizeSwapChain() submits and releases m_cmdEncoder.
			if (!EnsureSwapChainTexture())
				return false;
			const bool firstSurfacePass = !m_surfaceClearedThisFrame;
			const bool doClear = clearColor || firstSurfacePass;
			if (firstSurfacePass)
			{
				effClearDepth = true;
				effClearStencil = true;
			}
			WGPURenderPassColorAttachment& ca = colorAtts[colorAttCount++];
			ca.view = m_currentSurfaceView;
			ca.depthSlice = 0xFFFFFFFFu;
			ca.loadOp = doClear ? WGPULoadOp_Clear : WGPULoadOp_Load;
			ca.storeOp = WGPUStoreOp_Store;
			ca.clearValue = wgpuClearColor;
			depthTex = m_defaultDepthTex;
			if (doClear)
				m_surfaceClearedThisFrame = true;
		}

		WGPURenderPassDepthStencilAttachment dsAtt = {};
		const bool hasDepth = (depthTex != nullptr && depthTex->GetRTVDSVView() != nullptr);
		if (hasDepth)
		{
			const bool fmtHasStencil = (depthTex->GetWGPUFormat() == WGPUTextureFormat_Depth24PlusStencil8 ||
				depthTex->GetWGPUFormat() == WGPUTextureFormat_Depth32FloatStencil8);
			dsAtt.view = depthTex->GetRTVDSVView();
			dsAtt.depthLoadOp = effClearDepth ? WGPULoadOp_Clear : WGPULoadOp_Load;
			dsAtt.depthStoreOp = WGPUStoreOp_Store;
			dsAtt.depthClearValue = m_state.GetClearDepth();
			dsAtt.depthReadOnly = 0;
			if (fmtHasStencil)
			{
				dsAtt.stencilLoadOp = effClearStencil ? WGPULoadOp_Clear : WGPULoadOp_Load;
				dsAtt.stencilStoreOp = WGPUStoreOp_Store;
				dsAtt.stencilClearValue = static_cast<uint32_t>(m_state.GetClearStencil() & 0xFF);
				dsAtt.stencilReadOnly = 0;
			}
		}

		WGPUCommandEncoder enc = EnsureEncoder();
		if (!enc)
			return false;

		WGPURenderPassDescriptor rpd = {};
		rpd.label = ParaWgpuLabel("ParaRenderPass");
		rpd.colorAttachmentCount = colorAttCount;
		rpd.colorAttachments = (colorAttCount > 0) ? colorAtts : nullptr;
		rpd.depthStencilAttachment = hasDepth ? &dsAtt : nullptr;
		rpd.occlusionQuerySet = m_occlusionQuerySet;

		m_renderPass = PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderBeginRenderPass, enc, &rpd);
		if (!m_renderPass)
			return false;

		// Apply initial viewport and scissor to the newly opened render pass
		ApplyClampedViewport(m_state.GetViewport());

		const bool scissorEnabled = (m_state.GetRenderState(ERenderState::SCISSORTESTENABLE) != 0);
		if (scissorEnabled)
		{
			const RECT& sc = m_state.GetScissorRect();
			const uint32_t sx = static_cast<uint32_t>((std::max)(static_cast<LONG>(0), (std::min)(sc.left, static_cast<LONG>(m_activeRTWidth))));
			const uint32_t sy = static_cast<uint32_t>((std::max)(static_cast<LONG>(0), (std::min)(sc.top, static_cast<LONG>(m_activeRTHeight))));
			const uint32_t ex = static_cast<uint32_t>((std::max)(static_cast<LONG>(sx), (std::min)(sc.right, static_cast<LONG>(m_activeRTWidth))));
			const uint32_t ey = static_cast<uint32_t>((std::max)(static_cast<LONG>(sy), (std::min)(sc.bottom, static_cast<LONG>(m_activeRTHeight))));
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetScissorRect, m_renderPass, sx, sy, ex - sx, ey - sy);
		}
		else
		{
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetScissorRect, m_renderPass, 0, 0, m_activeRTWidth, m_activeRTHeight);
		}
		return true;
	}

	void RenderDeviceWebGPU::ExecuteAndSyncGPU()
	{
		EndActiveRenderPass();
		if (m_cmdEncoder)
		{
			WGPUCommandBufferDescriptor cbd = {};
			cbd.label = ParaWgpuLabel("ParaCommandBuffer");
			WGPUCommandBuffer cmd = PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderFinish, m_cmdEncoder, &cbd);
			PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderRelease, m_cmdEncoder);
			m_cmdEncoder = nullptr;
			if (cmd)
			{
				PARA_WGPU_CALL(m_wgpu, wgpuQueueSubmit, m_queue, 1, &cmd);
				PARA_WGPU_CALL(m_wgpu, wgpuCommandBufferRelease, cmd);
			}
		}
		m_wgpu.ProcessEvents(m_instance, m_device);
		FlushFrameTransientBuffers();
	}

	bool RenderDeviceWebGPU::MapStagingBufferReadSync(WGPUBuffer staging, size_t mapSize)
	{
		if (!staging || mapSize == 0)
			return false;
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
		struct MapWaitState
		{
			bool done = false;
			WGPUMapAsyncStatus status = WGPUMapAsyncStatus_Error;
		} waitSt;
		WGPUBufferMapCallbackInfo cbInfo = {};
		cbInfo.mode = WGPUCallbackMode_AllowSpontaneous;
		cbInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView /*message*/, void* ud1, void* /*ud2*/) {
			auto* s = static_cast<MapWaitState*>(ud1);
			if (s)
			{
				s->status = status;
				s->done = true;
			}
		};
		cbInfo.userdata1 = &waitSt;
		PARA_WGPU_CALL(m_wgpu, wgpuBufferMapAsync, staging, WGPUMapMode_Read, 0, mapSize, cbInfo);
		for (int spin = 0; !waitSt.done && spin < 5000; ++spin)
		{
			m_wgpu.WaitAsync(m_instance, m_device);
		}
		return waitSt.done && waitSt.status == WGPUMapAsyncStatus_Success;
#else
		struct MapWaitState
		{
			bool done = false;
			WGPUBufferMapAsyncStatus status = WGPUBufferMapAsyncStatus_Unknown;
		} waitSt;
		auto cb = [](WGPUBufferMapAsyncStatus status, void* ud) {
			auto* s = static_cast<MapWaitState*>(ud);
			s->status = status;
			s->done = true;
		};
		PARA_WGPU_CALL(m_wgpu, wgpuBufferMapAsync, staging, WGPUMapMode_Read, 0, mapSize, cb, &waitSt);
		for (int spin = 0; !waitSt.done && spin < 2000; ++spin)
		{
			m_wgpu.WaitAsync(m_instance, m_device);
		}
		return waitSt.done && waitSt.status == WGPUBufferMapAsyncStatus_Success;
#endif
	}

	bool RenderDeviceWebGPU::ReadBufferSync(WGPUBuffer srcBuffer, uint64_t byteSize, void* dstData)
	{
		if (!srcBuffer || byteSize == 0 || !dstData || !m_device)
			return false;
		const uint64_t alignedBytes = (byteSize + 3ull) & ~3ull;

		WGPUBufferDescriptor sbd = {};
		sbd.label = ParaWgpuLabel("ParaReadbackStaging");
		sbd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
		sbd.size = alignedBytes;
		WGPUBuffer staging = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &sbd);
		if (!staging)
			return false;

		EndActiveRenderPass();
		WGPUCommandEncoder enc = EnsureEncoder();
		PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderCopyBufferToBuffer, enc, srcBuffer, 0, staging, 0, alignedBytes);
		ExecuteAndSyncGPU();

		bool ok = false;
		if (MapStagingBufferReadSync(staging, static_cast<size_t>(alignedBytes)))
		{
			const void* mapped = PARA_WGPU_CALL(m_wgpu, wgpuBufferGetConstMappedRange, staging, 0, static_cast<size_t>(alignedBytes));
			if (mapped)
			{
				std::memcpy(dstData, mapped, static_cast<size_t>(byteSize));
				ok = true;
			}
			PARA_WGPU_CALL(m_wgpu, wgpuBufferUnmap, staging);
		}

		if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
			PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, staging);
		PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, staging);
		return ok;
	}

	uint32_t RenderDeviceWebGPU::AllocateOcclusionSlot()
	{
		if (m_freeOcclusionSlots.empty())
			return UINT32_MAX;
		uint32_t s = m_freeOcclusionSlots.back();
		m_freeOcclusionSlots.pop_back();
		return s;
	}

	void RenderDeviceWebGPU::FreeOcclusionSlot(uint32_t slot)
	{
		if (slot < MAX_OCCLUSION_QUERIES)
			m_freeOcclusionSlots.push_back(slot);
	}

	void RenderDeviceWebGPU::BeginOcclusionOnActivePass(uint32_t slot)
	{
		if (!m_occlusionQuerySet || slot >= MAX_OCCLUSION_QUERIES || !PARA_WGPU_HAS(m_wgpu, wgpuRenderPassEncoderBeginOcclusionQuery))
			return;
		if (!EnsureActiveRenderPass(false, false, false))
			return;
		if (m_occlusionActiveInPass)
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderEndOcclusionQuery, m_renderPass);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderBeginOcclusionQuery, m_renderPass, slot);
		m_occlusionActiveInPass = true;
		m_pendingOcclusionResolveSlots.push_back(slot);
	}

	void RenderDeviceWebGPU::EndOcclusionOnActivePass()
	{
		if (m_renderPass && m_occlusionActiveInPass && PARA_WGPU_HAS(m_wgpu, wgpuRenderPassEncoderEndOcclusionQuery))
		{
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderEndOcclusionQuery, m_renderPass);
			m_occlusionActiveInPass = false;
		}
	}

	bool RenderDeviceWebGPU::ReadOcclusionSlotValue(uint32_t slot, bool flush, uint64_t& outSamples)
	{
		if (!m_occlusionResolveBuf || slot >= MAX_OCCLUSION_QUERIES)
		{
			outSamples = 1;
			return true;
		}
		if (!flush)
		{
			// Avoid synchronous GPU stall unless requested; conservatively report visible
			outSamples = 1;
			return true;
		}

		EndActiveRenderPass();
		WGPUCommandEncoder enc = EnsureEncoder();
		WGPUBufferDescriptor sbd = {};
		sbd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
		sbd.size = sizeof(uint64_t);
		WGPUBuffer staging = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &sbd);
		if (!staging)
		{
			outSamples = 1;
			return true;
		}
		PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderCopyBufferToBuffer,
			enc, m_occlusionResolveBuf, static_cast<uint64_t>(slot) * sizeof(uint64_t), staging, 0, sizeof(uint64_t));
		ExecuteAndSyncGPU();

		outSamples = 1;
		if (MapStagingBufferReadSync(staging, sizeof(uint64_t)))
		{
			const void* ptr = PARA_WGPU_CALL(m_wgpu, wgpuBufferGetConstMappedRange, staging, 0, sizeof(uint64_t));
			if (ptr)
				std::memcpy(&outSamples, ptr, sizeof(uint64_t));
			PARA_WGPU_CALL(m_wgpu, wgpuBufferUnmap, staging);
		}
		if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
			PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, staging);
		PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, staging);
		return true;
	}

	bool RenderDeviceWebGPU::CheckDeviceError(const char* /*filename*/, const char* /*func*/, int /*nLine*/)
	{
		return true;
	}

	IRHITexture* RenderDeviceWebGPU::CreateTexture(
		uint32_t width,
		uint32_t height,
		PixelFormat format,
		uint32_t mipLevels,
		ERHITextureUsage usage,
		const RHIMipData* initialMips,
		uint32_t initialMipCount)
	{
		if (!m_device)
			return nullptr;

		width = (std::max)(1u, width);
		height = (std::max)(1u, height);
		mipLevels = (std::max)(1u, mipLevels);

		bool decodedS3TC = false;
		bool expandedRGBA8 = false;
		const bool hasInitialCpuData = (initialMips != nullptr && initialMipCount > 0 && initialMips[0].data != nullptr);
		const WGPUTextureFormat wgpuFmt = ToWGPUTextureFormat(
			format, width, height, m_supportsBC, m_supportsFloat32Filterable, hasInitialCpuData, decodedS3TC, expandedRGBA8);
		const bool isCube = HasFlag(usage, ERHITextureUsage::CubeMap);
		const bool isDepth = IsDepthPixelFormat(format) || HasFlag(usage, ERHITextureUsage::DepthStencil);
		const bool isRT = HasFlag(usage, ERHITextureUsage::RenderTarget);

		WGPUTextureUsageFlags wgpuUsage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
		if (HasFlag(usage, ERHITextureUsage::ShaderResource) || (!isDepth && !isRT))
			wgpuUsage |= WGPUTextureUsage_TextureBinding;
		if (isRT || isDepth)
			wgpuUsage |= WGPUTextureUsage_RenderAttachment;

		WGPUTextureDescriptor td = {};
		td.usage = wgpuUsage;
		td.dimension = WGPUTextureDimension_2D;
		td.size = { width, height, isCube ? 6u : 1u };
		td.format = wgpuFmt;
		td.mipLevelCount = mipLevels;
		td.sampleCount = 1;

		WGPUTexture tex = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateTexture, m_device, &td);
		if (!tex)
			return nullptr;

		WGPUTextureView srvView = nullptr;
		if ((wgpuUsage & WGPUTextureUsage_TextureBinding) != 0)
		{
			WGPUTextureViewDescriptor vd = {};
			WGPUTextureFormat srvFmt = wgpuFmt;
			if (isDepth)
			{
				if (wgpuFmt == WGPUTextureFormat_Depth24PlusStencil8)
					srvFmt = WGPUTextureFormat_Depth24Plus;
				else if (wgpuFmt == WGPUTextureFormat_Depth32FloatStencil8)
					srvFmt = WGPUTextureFormat_Depth32Float;
			}
			vd.format = srvFmt;
			vd.dimension = isCube ? WGPUTextureViewDimension_Cube : WGPUTextureViewDimension_2D;
			vd.baseMipLevel = 0;
			vd.mipLevelCount = mipLevels;
			vd.baseArrayLayer = 0;
			vd.arrayLayerCount = isCube ? 6u : 1u;
			vd.aspect = isDepth ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All;
			srvView = PARA_WGPU_CALL(m_wgpu, wgpuTextureCreateView, tex, &vd);
		}

		WGPUTextureView rtvDsvView = nullptr;
		if ((wgpuUsage & WGPUTextureUsage_RenderAttachment) != 0)
		{
			WGPUTextureViewDescriptor rvd = {};
			rvd.format = wgpuFmt;
			rvd.dimension = WGPUTextureViewDimension_2D;
			rvd.baseMipLevel = 0;
			rvd.mipLevelCount = 1;
			rvd.baseArrayLayer = 0;
			rvd.arrayLayerCount = 1;
			rvd.aspect = WGPUTextureAspect_All;
			rtvDsvView = PARA_WGPU_CALL(m_wgpu, wgpuTextureCreateView, tex, &rvd);
		}

		auto* outTex = new WebGPUTexture(
			this,
			width,
			height,
			isCube ? 6u : 1u,
			mipLevels,
			format,
			wgpuFmt,
			usage,
			isCube,
			false,
			decodedS3TC,
			expandedRGBA8,
			tex,
			srvView,
			rtvDsvView);

		if (initialMips && initialMipCount > 0)
		{
			const uint32_t uploadCount = (std::min)(mipLevels, initialMipCount);
			for (uint32_t m = 0; m < uploadCount; ++m)
			{
				if (initialMips[m].data)
				{
					const uint32_t mw = (std::max)(1u, width >> m);
					const uint32_t mh = (std::max)(1u, height >> m);
					outTex->UpdateSubresource(m, 0, 0, 0, mw, mh, initialMips[m].data, initialMips[m].rowPitch);
				}
			}
		}

		return outTex;
	}

	IRHIBuffer* RenderDeviceWebGPU::CreateBuffer(
		uint32_t byteSize,
		bool isIndexBuffer,
		ERHIBufferUsage usage,
		const void* initialData)
	{
		if (!m_device)
			return nullptr;
		return new WebGPUBuffer(this, byteSize, isIndexBuffer, usage, initialData);
	}

	IRHIRenderTarget* RenderDeviceWebGPU::CreateRenderTarget(
		uint32_t width,
		uint32_t height,
		PixelFormat colorFormat,
		PixelFormat depthFormat,
		uint32_t colorTargetCount,
		uint32_t /*msaaSamples*/)
	{
		width = (std::max)(1u, width);
		height = (std::max)(1u, height);
		colorTargetCount = (std::min)((std::max)(1u, colorTargetCount), MAX_RENDER_TARGETS);

		std::vector<WebGPUTexture*> colors;
		colors.reserve(colorTargetCount);
		for (uint32_t i = 0; i < colorTargetCount; ++i)
		{
			auto* ct = static_cast<WebGPUTexture*>(CreateTexture(
				width, height, colorFormat, 1,
				ERHITextureUsage::RenderTarget | ERHITextureUsage::ShaderResource));
			if (!ct)
			{
				for (WebGPUTexture* prev : colors)
					delete prev;
				return nullptr;
			}
			colors.push_back(ct);
		}

		WebGPUTexture* depth = nullptr;
		if (depthFormat != PixelFormat::Unknown)
		{
			const PixelFormat effDepth = IsDepthPixelFormat(depthFormat) ? depthFormat : PixelFormat::D24S8;
			depth = static_cast<WebGPUTexture*>(CreateTexture(
				width, height, effDepth, 1,
				ERHITextureUsage::DepthStencil | ERHITextureUsage::ShaderResource));
		}

		return new WebGPURenderTarget(width, height, colorFormat, colors, depth);
	}

	IRHIOcclusionQuery* RenderDeviceWebGPU::CreateOcclusionQuery()
	{
		return new WebGPUOcclusionQuery(this);
	}

	IRHIShaderProgram* RenderDeviceWebGPU::CompileShaderProgram(const RHIShaderProgramDesc& desc)
	{
		if (!m_device || !desc.vsBytecode || desc.vsBytecodeSize == 0)
			return nullptr;

		auto extractUtf8 = [](const void* bytes, uint32_t len) -> std::string {
			if (!bytes || len == 0)
				return std::string();
			const auto* p = static_cast<const char*>(bytes);
			while (len > 0 && p[len - 1] == '\0')
				--len;
			return std::string(p, len);
		};

		const std::string vsSrc = extractUtf8(desc.vsBytecode, desc.vsBytecodeSize);
		std::string psSrc = extractUtf8(desc.psBytecode, desc.psBytecodeSize);
		if (vsSrc.empty())
			return nullptr;
		if (!psSrc.empty() && psSrc.find("derivative_uniformity") == std::string::npos)
			psSrc = "diagnostic(off, derivative_uniformity);\n" + psSrc;

		WebGPUShaderReflection refl{};
		ReflectWgslStage(vsSrc, /*isVertex=*/true, refl);
		if (!psSrc.empty())
			ReflectWgslStage(psSrc, /*isVertex=*/false, refl);

		if (desc.vsGlobalCBufferSize > 0)
			refl.vsUboMinBytes = (std::max)(refl.vsUboMinBytes, AlignUpU32(desc.vsGlobalCBufferSize, 16u));
		if (desc.psGlobalCBufferSize > 0)
			refl.psUboMinBytes = (std::max)(refl.psUboMinBytes, AlignUpU32(desc.psGlobalCBufferSize, 16u));

#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
		WGPUShaderSourceWGSL vsWgsl = {};
		vsWgsl.chain.sType = WGPUSType_ShaderSourceWGSL;
		vsWgsl.code = ParaWgpuLabel(vsSrc.c_str());
#else
		WGPUShaderModuleWGSLDescriptor vsWgsl = {};
		vsWgsl.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
		vsWgsl.code = vsSrc.c_str();
#endif
		WGPUShaderModuleDescriptor vsModDesc = {};
		vsModDesc.nextInChain = &vsWgsl.chain;
		vsModDesc.label = ParaWgpuLabel(desc.debugName ? desc.debugName : "ParaWGSL_VS");
		WGPUShaderModule vsMod = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateShaderModule, m_device, &vsModDesc);
		if (!vsMod)
			return nullptr;

		WGPUShaderModule psMod = nullptr;
		if (!psSrc.empty())
		{
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
			WGPUShaderSourceWGSL psWgsl = {};
			psWgsl.chain.sType = WGPUSType_ShaderSourceWGSL;
			psWgsl.code = ParaWgpuLabel(psSrc.c_str());
#else
			WGPUShaderModuleWGSLDescriptor psWgsl = {};
			psWgsl.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
			psWgsl.code = psSrc.c_str();
#endif
			WGPUShaderModuleDescriptor psModDesc = {};
			psModDesc.nextInChain = &psWgsl.chain;
			psModDesc.label = ParaWgpuLabel(desc.debugName ? desc.debugName : "ParaWGSL_PS");
			psMod = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateShaderModule, m_device, &psModDesc);
			if (!psMod)
			{
				PARA_WGPU_CALL(m_wgpu, wgpuShaderModuleRelease, vsMod);
				return nullptr;
			}
		}

		// Build exact per-program WGPUBindGroupLayout matching ONLY the bindings declared in VS+PS WGSL
		std::vector<WGPUBindGroupLayoutEntry> entries;
		entries.reserve(34);

		if (refl.hasVsUbo)
		{
			WGPUBindGroupLayoutEntry e = {};
			e.binding = 0;
			e.visibility = WGPUShaderStage_Vertex;
			e.buffer.type = WGPUBufferBindingType_Uniform;
			e.buffer.minBindingSize = refl.vsUboMinBytes;
			entries.push_back(e);
		}
		if (refl.hasPsUbo)
		{
			WGPUBindGroupLayoutEntry e = {};
			e.binding = 1;
			e.visibility = WGPUShaderStage_Fragment;
			e.buffer.type = WGPUBufferBindingType_Uniform;
			e.buffer.minBindingSize = refl.psUboMinBytes;
			entries.push_back(e);
		}
		for (uint32_t s = 0; s < 16u; ++s)
		{
			if (refl.texVisibility[s] != 0)
			{
				WGPUBindGroupLayoutEntry e = {};
				e.binding = 16u + s;
				e.visibility = refl.texVisibility[s];
				e.texture.sampleType = WGPUTextureSampleType_Float;
				e.texture.viewDimension = (refl.texViewDim[s] != WGPUTextureViewDimension_Undefined)
					? refl.texViewDim[s]
					: WGPUTextureViewDimension_2D;
				e.texture.multisampled = 0;
				entries.push_back(e);
			}
			if (refl.smpVisibility[s] != 0)
			{
				WGPUBindGroupLayoutEntry e = {};
				e.binding = 32u + s;
				e.visibility = refl.smpVisibility[s];
				e.sampler.type = WGPUSamplerBindingType_Filtering;
				entries.push_back(e);
			}
		}

		WGPUBindGroupLayoutDescriptor bgld = {};
		bgld.label = ParaWgpuLabel("ParaProgramBindGroupLayout");
		bgld.entryCount = entries.size();
		bgld.entries = entries.empty() ? nullptr : entries.data();
		WGPUBindGroupLayout bgl = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBindGroupLayout, m_device, &bgld);
		if (!bgl)
		{
			if (psMod) PARA_WGPU_CALL(m_wgpu, wgpuShaderModuleRelease, psMod);
			PARA_WGPU_CALL(m_wgpu, wgpuShaderModuleRelease, vsMod);
			return nullptr;
		}

		WGPUPipelineLayoutDescriptor pld = {};
		pld.label = ParaWgpuLabel("ParaProgramPipelineLayout");
		pld.bindGroupLayoutCount = 1;
		pld.bindGroupLayouts = &bgl;
		WGPUPipelineLayout pipeLayout = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreatePipelineLayout, m_device, &pld);
		if (!pipeLayout)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuBindGroupLayoutRelease, bgl);
			if (psMod) PARA_WGPU_CALL(m_wgpu, wgpuShaderModuleRelease, psMod);
			PARA_WGPU_CALL(m_wgpu, wgpuShaderModuleRelease, vsMod);
			return nullptr;
		}

		uint64_t h = Fnv1aBytes64(vsSrc.data(), vsSrc.size());
		if (!psSrc.empty())
			h = Fnv1aBytes64(psSrc.data(), psSrc.size(), h);

		const std::string vsEntry = (desc.vsEntryPoint && desc.vsEntryPoint[0]) ? desc.vsEntryPoint : "vs_main";
		const std::string psEntry = (desc.psEntryPoint && desc.psEntryPoint[0]) ? desc.psEntryPoint : "fs_main";

		return new WebGPUShaderProgram(this, vsMod, psMod, vsEntry, psEntry, bgl, pipeLayout, refl, h);
	}

	bool RenderDeviceWebGPU::BindShaderProgram(IRHIShaderProgram* program)
	{
		m_boundProgram = program ? static_cast<WebGPUShaderProgram*>(program->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceWebGPU::SetShaderConstants(
		const void* vsConstants,
		uint32_t vsByteSize,
		const void* psConstants,
		uint32_t psByteSize)
	{
		if (vsConstants && vsByteSize > 0)
		{
			if (m_vsConstantsCpu.size() < vsByteSize)
				m_vsConstantsCpu.resize(vsByteSize, 0);
			if (std::memcmp(m_vsConstantsCpu.data(), vsConstants, vsByteSize) != 0 || m_currentVsUbo == nullptr)
			{
				std::memcpy(m_vsConstantsCpu.data(), vsConstants, vsByteSize);
				m_vsUboDirty = true;
			}
		}
		if (psConstants && psByteSize > 0)
		{
			if (m_psConstantsCpu.size() < psByteSize)
				m_psConstantsCpu.resize(psByteSize, 0);
			if (std::memcmp(m_psConstantsCpu.data(), psConstants, psByteSize) != 0 || m_currentPsUbo == nullptr)
			{
				std::memcpy(m_psConstantsCpu.data(), psConstants, psByteSize);
				m_psUboDirty = true;
			}
		}
		return true;
	}

	bool RenderDeviceWebGPU::SetRenderTargets(
		uint32_t count,
		IRHIRenderTarget* const* targets,
		IRHIRenderTarget* depthStencilOverride)
	{
		EndActiveRenderPass();

		m_boundRTCount = 0;
		for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
			m_boundRTs[i] = nullptr;
		m_boundDepthOverride = depthStencilOverride
			? static_cast<WebGPURenderTarget*>(depthStencilOverride->GetNativeHandle())
			: nullptr;

		if (count > 0 && targets)
		{
			const uint32_t clampCount = (std::min)(count, MAX_RENDER_TARGETS);
			for (uint32_t i = 0; i < clampCount; ++i)
			{
				m_boundRTs[i] = targets[i] ? static_cast<WebGPURenderTarget*>(targets[i]->GetNativeHandle()) : nullptr;
				if (m_boundRTs[i])
					m_boundRTCount = i + 1u;
			}
		}

		if (m_boundRTCount > 0 && m_boundRTs[0])
		{
			m_activeRTWidth = (std::max)(1u, m_boundRTs[0]->GetWidth());
			m_activeRTHeight = (std::max)(1u, m_boundRTs[0]->GetHeight());
		}
		else
		{
			m_activeRTWidth = m_backBufferWidth;
			m_activeRTHeight = m_backBufferHeight;
		}

		NotifyRenderTargetsBound(m_boundRTCount, m_boundRTCount > 0 ? targets : nullptr, depthStencilOverride);

		ParaViewport vp{};
		vp.X = 0;
		vp.Y = 0;
		vp.Width = m_activeRTWidth;
		vp.Height = m_activeRTHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		m_state.SetViewport(vp);
		return true;
	}

	void RenderDeviceWebGPU::BeginRenderTarget(uint32_t width, uint32_t height)
	{
		m_activeRTWidth = (std::max)(1u, width);
		m_activeRTHeight = (std::max)(1u, height);
	}

	void RenderDeviceWebGPU::EndRenderTarget()
	{
		SetRenderTargets(0, nullptr, nullptr);
	}

	bool RenderDeviceWebGPU::BlitRenderTarget(
		IRHIRenderTarget* src,
		IRHIRenderTarget* dst,
		const RECT* srcRect,
		const RECT* dstRect,
		bool linearFilter)
	{
		if (src == dst)
			return false;

		EndActiveRenderPass();

		if (!src || !dst)
		{
			if (!EnsureSwapChainTexture())
				return false;
		}

		WGPUTexture srcWgpuTex = nullptr;
		WGPUTextureView srcSrvView = nullptr;
		WGPUTextureFormat srcFmt = m_surfaceFormat;
		uint32_t srcW = m_backBufferWidth;
		uint32_t srcH = m_backBufferHeight;
		bool srcSupportsCopy = (m_surfaceUsage & WGPUTextureUsage_CopySrc) != 0;

		if (src)
		{
			auto* srcRT = static_cast<WebGPURenderTarget*>(src->GetNativeHandle());
			WebGPUTexture* srcTex = srcRT ? srcRT->GetWebGPUColor(0) : nullptr;
			if (!srcTex || !srcTex->GetWGPUTexture())
				return false;
			srcWgpuTex = srcTex->GetWGPUTexture();
			srcSrvView = srcTex->GetSRVView();
			srcFmt = srcTex->GetWGPUFormat();
			srcW = srcTex->GetWidth();
			srcH = srcTex->GetHeight();
			srcSupportsCopy = true;
		}
		else
		{
			srcWgpuTex = m_currentSurfaceTex;
			srcSrvView = ((m_surfaceUsage & WGPUTextureUsage_TextureBinding) != 0) ? m_currentSurfaceView : nullptr;
		}

		WGPUTexture dstWgpuTex = nullptr;
		WGPUTextureView dstRtvView = nullptr;
		WGPUTextureFormat dstFmt = m_surfaceFormat;
		uint32_t dstW = m_backBufferWidth;
		uint32_t dstH = m_backBufferHeight;
		bool dstSupportsCopy = (m_surfaceUsage & WGPUTextureUsage_CopyDst) != 0;

		if (dst)
		{
			auto* dstRT = static_cast<WebGPURenderTarget*>(dst->GetNativeHandle());
			WebGPUTexture* dstColor = dstRT ? dstRT->GetWebGPUColor(0) : nullptr;
			if (!dstColor || !dstColor->GetWGPUTexture())
				return false;
			dstWgpuTex = dstColor->GetWGPUTexture();
			dstRtvView = dstColor->GetRTVDSVView();
			dstFmt = dstColor->GetWGPUFormat();
			dstW = dstColor->GetWidth();
			dstH = dstColor->GetHeight();
			dstSupportsCopy = true;
		}
		else
		{
			dstWgpuTex = m_currentSurfaceTex;
			dstRtvView = m_currentSurfaceView;
		}

		if (!srcWgpuTex || !dstWgpuTex || srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
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

		// Fast path: direct GPU texture copy when formats & extents match and both textures support copy usage
		if (srcFmt == dstFmt && sw == dw && sh == dh && srcSupportsCopy && dstSupportsCopy && !IsBCCompressedWGPUFormat(srcFmt))
		{
			WGPUCommandEncoder enc = EnsureEncoder();
			if (!enc)
				return false;
			WGPUImageCopyTexture s = {};
			s.texture = srcWgpuTex;
			s.origin = { static_cast<uint32_t>(sx0), static_cast<uint32_t>(sy0), 0 };
			s.aspect = WGPUTextureAspect_All;
			WGPUImageCopyTexture d = {};
			d.texture = dstWgpuTex;
			d.origin = { static_cast<uint32_t>(dx0), static_cast<uint32_t>(dy0), 0 };
			d.aspect = WGPUTextureAspect_All;
			WGPUExtent3D ext = { sw, sh, 1 };
			PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderCopyTextureToTexture, enc, &s, &d, &ext);
			if (!dst)
				m_surfaceClearedThisFrame = true;
			return true;
		}

		// Shader blit fallback (handles RGBA8Unorm <-> BGRA8Unorm format conversion and sub-rect scaling)
		if (!srcSrvView || !dstRtvView || IsDepthPixelFormat(src ? src->GetColorTexture(0)->GetFormat() : PixelFormat::A8R8G8B8))
			return false;
		const bool unfilterableSrc = !m_supportsFloat32Filterable && IsUnblendableFloat32WGPUFormat(srcFmt);
		if (unfilterableSrc)
			return false;

		if (!m_blitShaderModule)
		{
			static const char* kBlitWgsl =
				"struct BlitParams { uvRect : vec4<f32>, };\n"
				"@group(0) @binding(0) var<uniform> params : BlitParams;\n"
				"@group(0) @binding(1) var blitTex : texture_2d<f32>;\n"
				"@group(0) @binding(2) var blitSmp : sampler;\n"
				"struct VsOut { @builtin(position) pos : vec4<f32>, @location(0) uv : vec2<f32>, };\n"
				"@vertex fn vs_main(@builtin(vertex_index) vid : u32) -> VsOut {\n"
				"    var o : VsOut;\n"
				"    let x = f32((vid << 1u) & 2u);\n"
				"    let y = f32(vid & 2u);\n"
				"    o.pos = vec4<f32>(x * 2.0 - 1.0, 1.0 - y * 2.0, 0.0, 1.0);\n"
				"    o.uv = params.uvRect.xy + vec2<f32>(x, y) * params.uvRect.zw;\n"
				"    return o;\n"
				"}\n"
				"@fragment fn fs_main(in : VsOut) -> @location(0) vec4<f32> {\n"
				"    return textureSample(blitTex, blitSmp, in.uv);\n"
				"}\n";
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
			WGPUShaderSourceWGSL wgslSrc = {};
			wgslSrc.chain.sType = WGPUSType_ShaderSourceWGSL;
			wgslSrc.code = ParaWgpuLabel(kBlitWgsl);
#else
			WGPUShaderModuleWGSLDescriptor wgslSrc = {};
			wgslSrc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
			wgslSrc.code = kBlitWgsl;
#endif
			WGPUShaderModuleDescriptor smd = {};
			smd.nextInChain = &wgslSrc.chain;
			smd.label = ParaWgpuLabel("ParaBlitWGSL");
			m_blitShaderModule = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateShaderModule, m_device, &smd);
			if (!m_blitShaderModule)
				return false;
		}

		if (!m_blitBgl)
		{
			WGPUBindGroupLayoutEntry bglEntries[3] = {};
			bglEntries[0].binding = 0;
			bglEntries[0].visibility = WGPUShaderStage_Vertex;
			bglEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
			bglEntries[0].buffer.minBindingSize = 16;

			bglEntries[1].binding = 1;
			bglEntries[1].visibility = WGPUShaderStage_Fragment;
			bglEntries[1].texture.sampleType = WGPUTextureSampleType_Float;
			bglEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;

			bglEntries[2].binding = 2;
			bglEntries[2].visibility = WGPUShaderStage_Fragment;
			bglEntries[2].sampler.type = WGPUSamplerBindingType_Filtering;

			WGPUBindGroupLayoutDescriptor bgld = {};
			bgld.label = ParaWgpuLabel("ParaBlitBGL");
			bgld.entryCount = 3;
			bgld.entries = bglEntries;
			m_blitBgl = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBindGroupLayout, m_device, &bgld);
			if (!m_blitBgl)
				return false;
		}

		if (!m_blitPipeLayout)
		{
			WGPUPipelineLayoutDescriptor pld = {};
			pld.label = ParaWgpuLabel("ParaBlitPipeLayout");
			pld.bindGroupLayoutCount = 1;
			pld.bindGroupLayouts = &m_blitBgl;
			m_blitPipeLayout = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreatePipelineLayout, m_device, &pld);
			if (!m_blitPipeLayout)
				return false;
		}

		WGPURenderPipeline blitPipe = nullptr;
		auto pipeIt = m_blitPipelines.find(static_cast<uint32_t>(dstFmt));
		if (pipeIt != m_blitPipelines.end())
		{
			blitPipe = pipeIt->second;
		}
		else
		{
			WGPUColorTargetState cts = {};
			cts.format = dstFmt;
			cts.writeMask = WGPUColorWriteMask_All;

			WGPUFragmentState fs = {};
			fs.module = m_blitShaderModule;
			fs.entryPoint = ParaWgpuLabel("fs_main");
			fs.targetCount = 1;
			fs.targets = &cts;

			WGPURenderPipelineDescriptor rpd = {};
			rpd.label = ParaWgpuLabel("ParaBlitPipeline");
			rpd.layout = m_blitPipeLayout;
			rpd.vertex.module = m_blitShaderModule;
			rpd.vertex.entryPoint = ParaWgpuLabel("vs_main");
			rpd.primitive.topology = WGPUPrimitiveTopology_TriangleList;
			rpd.primitive.frontFace = WGPUFrontFace_CW;
			rpd.primitive.cullMode = WGPUCullMode_None;
			rpd.multisample.count = 1;
			rpd.multisample.mask = 0xFFFFFFFFu;
			rpd.fragment = &fs;

			blitPipe = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateRenderPipeline, m_device, &rpd);
			if (!blitPipe)
				return false;
			m_blitPipelines[static_cast<uint32_t>(dstFmt)] = blitPipe;
		}

		const float uvRect[4] = {
			static_cast<float>(sx0) / static_cast<float>(srcW),
			static_cast<float>(sy0) / static_cast<float>(srcH),
			static_cast<float>(sw) / static_cast<float>(srcW),
			static_cast<float>(sh) / static_cast<float>(srcH)
		};
		WGPUBuffer paramBuf = AllocateTransientBuffer(WGPUBufferUsage_Uniform, uvRect, sizeof(uvRect), 16u);
		if (!paramBuf)
			return false;

		WGPUSamplerDescriptor sd = {};
		sd.addressModeU = WGPUAddressMode_ClampToEdge;
		sd.addressModeV = WGPUAddressMode_ClampToEdge;
		sd.addressModeW = WGPUAddressMode_ClampToEdge;
		sd.magFilter = linearFilter ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
		sd.minFilter = linearFilter ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
		sd.mipmapFilter = WGPUMipmapFilterMode_Nearest;
		sd.lodMinClamp = 0.0f;
		sd.lodMaxClamp = 0.25f;
		sd.maxAnisotropy = 1u;
		const uint64_t smpKey = linearFilter ? 0xFFFF0002ull : 0xFFFF0001ull;
		WGPUSampler smp = nullptr;
		auto smpIt = m_samplerCache.find(smpKey);
		if (smpIt != m_samplerCache.end())
		{
			smp = smpIt->second;
		}
		else
		{
			smp = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateSampler, m_device, &sd);
			if (smp)
				m_samplerCache[smpKey] = smp;
		}
		if (!smp)
			return false;

		WGPUBindGroupEntry bgEntries[3] = {};
		bgEntries[0].binding = 0;
		bgEntries[0].buffer = paramBuf;
		bgEntries[0].offset = 0;
		bgEntries[0].size = 16;
		bgEntries[1].binding = 1;
		bgEntries[1].textureView = srcSrvView;
		bgEntries[2].binding = 2;
		bgEntries[2].sampler = smp;

		WGPUBindGroupDescriptor bgd = {};
		bgd.label = ParaWgpuLabel("ParaBlitBindGroup");
		bgd.layout = m_blitBgl;
		bgd.entryCount = 3;
		bgd.entries = bgEntries;
		WGPUBindGroup bg = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBindGroup, m_device, &bgd);
		if (!bg)
			return false;
		m_transientBindGroups.push_back(bg);

		WGPUCommandEncoder enc = EnsureEncoder();
		if (!enc)
			return false;

		const bool fullDstOverwrite = (dx0 == 0 && dy0 == 0 && dw == dstW && dh == dstH);
		WGPURenderPassColorAttachment ca = {};
		ca.view = dstRtvView;
		ca.depthSlice = 0xFFFFFFFFu;
		ca.loadOp = fullDstOverwrite ? WGPULoadOp_Clear : WGPULoadOp_Load;
		ca.storeOp = WGPUStoreOp_Store;
		ca.clearValue = { 0.0, 0.0, 0.0, 1.0 };

		WGPURenderPassDescriptor passDesc = {};
		passDesc.label = ParaWgpuLabel("ParaBlitPass");
		passDesc.colorAttachmentCount = 1;
		passDesc.colorAttachments = &ca;

		WGPURenderPassEncoder rpe = PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderBeginRenderPass, enc, &passDesc);
		if (!rpe)
			return false;
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetViewport,
			rpe, static_cast<float>(dx0), static_cast<float>(dy0), static_cast<float>(dw), static_cast<float>(dh), 0.0f, 1.0f);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetScissorRect,
			rpe, static_cast<uint32_t>(dx0), static_cast<uint32_t>(dy0), dw, dh);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetPipeline, rpe, blitPipe);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetBindGroup, rpe, 0, bg, 0, nullptr);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDraw, rpe, 3, 1, 0, 0);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderEnd, rpe);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderRelease, rpe);
		if (!dst)
			m_surfaceClearedThisFrame = true;
		return true;
	}

	bool RenderDeviceWebGPU::ImportExternalFramebuffer(uintptr_t /*nativeFboOrTexHandle*/, uint32_t width, uint32_t height)
	{
		m_activeRTWidth = (std::max)(1u, width);
		m_activeRTHeight = (std::max)(1u, height);
		return true;
	}

	bool RenderDeviceWebGPU::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t /*nDataFormat*/, uint32_t /*nDataType*/)
	{
		if (!pDataOut || nWidth <= 0 || nHeight <= 0)
			return false;

		WGPUTexture srcTex = nullptr;
		WGPUTextureFormat srcFmt = m_surfaceFormat;
		uint32_t texW = m_backBufferWidth;
		uint32_t texH = m_backBufferHeight;

		if (m_boundRTCount > 0 && m_boundRTs[0] && m_boundRTs[0]->GetWebGPUColor(0))
		{
			WebGPUTexture* ct = m_boundRTs[0]->GetWebGPUColor(0);
			srcTex = ct->GetWGPUTexture();
			srcFmt = ct->GetWGPUFormat();
			texW = ct->GetWidth();
			texH = ct->GetHeight();
		}
		else
		{
			if (!EnsureSwapChainTexture())
				return false;
			srcTex = m_currentSurfaceTex;
			srcFmt = m_surfaceFormat;
		}
		if (!srcTex)
			return false;

		const uint32_t x0 = static_cast<uint32_t>((std::max)(0, (std::min)(nLeft, static_cast<int>(texW) - 1)));
		const uint32_t y0 = static_cast<uint32_t>((std::max)(0, (std::min)(nTop, static_cast<int>(texH) - 1)));
		const uint32_t rw = (std::min)(static_cast<uint32_t>(nWidth), texW - x0);
		const uint32_t rh = (std::min)(static_cast<uint32_t>(nHeight), texH - y0);
		if (rw == 0 || rh == 0)
			return false;

		const uint32_t tightRow = rw * 4u;
		const uint32_t alignedRow = AlignUpU32(tightRow, 256u);
		const uint64_t bufBytes = static_cast<uint64_t>(alignedRow) * rh;

		WGPUBufferDescriptor bd = {};
		bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
		bd.size = bufBytes;
		WGPUBuffer readBuf = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &bd);
		if (!readBuf)
			return false;

		EndActiveRenderPass();
		WGPUCommandEncoder enc = EnsureEncoder();
		WGPUImageCopyTexture srcCopy = {};
		srcCopy.texture = srcTex;
		srcCopy.origin = { x0, y0, 0 };
		srcCopy.aspect = WGPUTextureAspect_All;

		WGPUImageCopyBuffer dstCopy = {};
		dstCopy.buffer = readBuf;
		dstCopy.layout.bytesPerRow = alignedRow;
		dstCopy.layout.rowsPerImage = rh;

		WGPUExtent3D ext = { rw, rh, 1 };
		PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderCopyTextureToBuffer, enc, &srcCopy, &dstCopy, &ext);
		ExecuteAndSyncGPU();

		bool ok = false;
		if (MapStagingBufferReadSync(readBuf, static_cast<size_t>(bufBytes)))
		{
			const auto* srcBytes = static_cast<const uint8_t*>(
				PARA_WGPU_CALL(m_wgpu, wgpuBufferGetConstMappedRange, readBuf, 0, static_cast<size_t>(bufBytes)));
			if (srcBytes)
			{
				auto* dstBytes = static_cast<uint8_t*>(pDataOut);
				const bool swapBGRA = (srcFmt == WGPUTextureFormat_BGRA8Unorm || srcFmt == WGPUTextureFormat_BGRA8UnormSrgb);
				for (uint32_t row = 0; row < rh; ++row)
				{
					const uint8_t* sRow = srcBytes + static_cast<size_t>(row) * alignedRow;
					uint8_t* dRow = dstBytes + static_cast<size_t>(row) * (static_cast<size_t>(nWidth) * 4u);
					if (swapBGRA)
					{
						for (uint32_t col = 0; col < rw; ++col)
						{
							dRow[col * 4u + 0] = sRow[col * 4u + 2];
							dRow[col * 4u + 1] = sRow[col * 4u + 1];
							dRow[col * 4u + 2] = sRow[col * 4u + 0];
							dRow[col * 4u + 3] = sRow[col * 4u + 3];
						}
					}
					else
					{
						std::memcpy(dRow, sRow, rw * 4u);
					}
				}
				ok = true;
			}
			PARA_WGPU_CALL(m_wgpu, wgpuBufferUnmap, readBuf);
		}

		if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
			PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, readBuf);
		PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, readBuf);
		return ok;
	}

	bool RenderDeviceWebGPU::ReadDepthPixel(int x, int y, float& outDepth)
	{
		outDepth = 1.0f;
		WebGPUTexture* depthTex = m_defaultDepthTex;
		if (m_boundRTCount > 0)
		{
			if (m_boundDepthOverride && m_boundDepthOverride->GetWebGPUDepth())
				depthTex = m_boundDepthOverride->GetWebGPUDepth();
			else if (m_boundRTs[0] && m_boundRTs[0]->GetWebGPUDepth())
				depthTex = m_boundRTs[0]->GetWebGPUDepth();
		}
		if (!depthTex || !depthTex->GetWGPUTexture())
			return false;

		if (x < 0 || y < 0 || static_cast<uint32_t>(x) >= depthTex->GetWidth() || static_cast<uint32_t>(y) >= depthTex->GetHeight())
			return false;

		// Depth32Float and Depth16Unorm support direct CopyTextureToBuffer; Depth24Plus cannot be copied in WebGPU 1.0
		const WGPUTextureFormat df = depthTex->GetWGPUFormat();
		if (df != WGPUTextureFormat_Depth32Float && df != WGPUTextureFormat_Depth16Unorm)
			return false;

		WGPUBufferDescriptor bd = {};
		bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
		bd.size = 256;
		WGPUBuffer readBuf = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &bd);
		if (!readBuf)
			return false;

		EndActiveRenderPass();
		WGPUCommandEncoder enc = EnsureEncoder();
		WGPUImageCopyTexture srcCopy = {};
		srcCopy.texture = depthTex->GetWGPUTexture();
		srcCopy.origin = { static_cast<uint32_t>(x), static_cast<uint32_t>(y), 0 };
		srcCopy.aspect = WGPUTextureAspect_DepthOnly;

		WGPUImageCopyBuffer dstCopy = {};
		dstCopy.buffer = readBuf;
		dstCopy.layout.bytesPerRow = 256;
		dstCopy.layout.rowsPerImage = 1;

		WGPUExtent3D ext = { 1, 1, 1 };
		PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderCopyTextureToBuffer, enc, &srcCopy, &dstCopy, &ext);
		ExecuteAndSyncGPU();

		bool ok = false;
		if (MapStagingBufferReadSync(readBuf, 256))
		{
			const void* ptr = PARA_WGPU_CALL(m_wgpu, wgpuBufferGetConstMappedRange, readBuf, 0, 16);
			if (ptr)
			{
				if (df == WGPUTextureFormat_Depth32Float)
				{
					std::memcpy(&outDepth, ptr, sizeof(float));
				}
				else
				{
					uint16_t d16 = 0;
					std::memcpy(&d16, ptr, sizeof(uint16_t));
					outDepth = static_cast<float>(d16) / 65535.0f;
				}
				ok = true;
			}
			PARA_WGPU_CALL(m_wgpu, wgpuBufferUnmap, readBuf);
		}
		if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
			PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, readBuf);
		PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, readBuf);
		return ok;
	}

	bool RenderDeviceWebGPU::ReadTexturePixels(IRHITexture* texture, void* pDataOut)
	{
		if (!texture || !pDataOut)
			return false;
		auto* wgpuTex = static_cast<WebGPUTexture*>(texture->GetNativeHandle());
		if (!wgpuTex || !wgpuTex->GetWGPUTexture())
			return false;

		const uint32_t w = wgpuTex->GetWidth();
		const uint32_t h = wgpuTex->GetHeight();
		const WGPUTextureFormat fmt = wgpuTex->GetWGPUFormat();
		const uint32_t tightRow = GetFormatTightRowBytes(fmt, w);
		const uint32_t rows = GetFormatBlockRows(fmt, h);
		const uint32_t alignedRow = AlignUpU32(tightRow, 256u);
		const uint64_t bufBytes = static_cast<uint64_t>(alignedRow) * rows;

		WGPUBufferDescriptor bd = {};
		bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
		bd.size = bufBytes;
		WGPUBuffer readBuf = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBuffer, m_device, &bd);
		if (!readBuf)
			return false;

		EndActiveRenderPass();
		WGPUCommandEncoder enc = EnsureEncoder();
		WGPUImageCopyTexture srcCopy = {};
		srcCopy.texture = wgpuTex->GetWGPUTexture();
		srcCopy.aspect = WGPUTextureAspect_All;

		WGPUImageCopyBuffer dstCopy = {};
		dstCopy.buffer = readBuf;
		dstCopy.layout.bytesPerRow = alignedRow;
		dstCopy.layout.rowsPerImage = rows;

		WGPUExtent3D ext = { w, h, 1 };
		PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderCopyTextureToBuffer, enc, &srcCopy, &dstCopy, &ext);
		ExecuteAndSyncGPU();

		bool ok = false;
		if (MapStagingBufferReadSync(readBuf, static_cast<size_t>(bufBytes)))
		{
			const auto* srcBytes = static_cast<const uint8_t*>(
				PARA_WGPU_CALL(m_wgpu, wgpuBufferGetConstMappedRange, readBuf, 0, static_cast<size_t>(bufBytes)));
			if (srcBytes)
			{
				auto* dstBytes = static_cast<uint8_t*>(pDataOut);
				const bool swapBGRA = (fmt == WGPUTextureFormat_BGRA8Unorm || fmt == WGPUTextureFormat_BGRA8UnormSrgb);
				for (uint32_t r = 0; r < rows; ++r)
				{
					const uint8_t* sRow = srcBytes + static_cast<size_t>(r) * alignedRow;
					uint8_t* dRow = dstBytes + static_cast<size_t>(r) * tightRow;
					if (swapBGRA && tightRow == w * 4u)
					{
						for (uint32_t c = 0; c < w; ++c)
						{
							dRow[c * 4u + 0] = sRow[c * 4u + 2];
							dRow[c * 4u + 1] = sRow[c * 4u + 1];
							dRow[c * 4u + 2] = sRow[c * 4u + 0];
							dRow[c * 4u + 3] = sRow[c * 4u + 3];
						}
					}
					else
					{
						std::memcpy(dRow, sRow, tightRow);
					}
				}
				ok = true;
			}
			PARA_WGPU_CALL(m_wgpu, wgpuBufferUnmap, readBuf);
		}

		if (PARA_WGPU_HAS(m_wgpu, wgpuBufferDestroy))
			PARA_WGPU_CALL(m_wgpu, wgpuBufferDestroy, readBuf);
		PARA_WGPU_CALL(m_wgpu, wgpuBufferRelease, readBuf);
		return ok;
	}

	WGPUSampler RenderDeviceWebGPU::GetOrCreateSampler(uint32_t stage)
	{
		if (stage >= RHIStateTracker::MAX_SAMPLER_STAGES)
			stage = 0;

		const uint32_t addrU = m_state.GetSamplerState(stage, ESamplerStateType::ADDRESSU);
		const uint32_t addrV = m_state.GetSamplerState(stage, ESamplerStateType::ADDRESSV);
		const uint32_t addrW = m_state.GetSamplerState(stage, ESamplerStateType::ADDRESSW);
		const uint32_t magF  = m_state.GetSamplerState(stage, ESamplerStateType::MAGFILTER);
		const uint32_t minF  = m_state.GetSamplerState(stage, ESamplerStateType::MINFILTER);
		const uint32_t mipF  = m_state.GetSamplerState(stage, ESamplerStateType::MIPFILTER);
		const uint32_t aniso = m_state.GetSamplerState(stage, ESamplerStateType::MAXANISOTROPY);

		const uint64_t key =
			(static_cast<uint64_t>(addrU & 0xF)) |
			(static_cast<uint64_t>(addrV & 0xF) << 4) |
			(static_cast<uint64_t>(addrW & 0xF) << 8) |
			(static_cast<uint64_t>(magF  & 0xF) << 12) |
			(static_cast<uint64_t>(minF  & 0xF) << 16) |
			(static_cast<uint64_t>(mipF  & 0xF) << 20) |
			(static_cast<uint64_t>(aniso & 0xFF) << 24);

		auto it = m_samplerCache.find(key);
		if (it != m_samplerCache.end())
			return it->second;

		WGPUSamplerDescriptor sd = {};
		sd.addressModeU = MapAddressMode(addrU);
		sd.addressModeV = MapAddressMode(addrV);
		sd.addressModeW = MapAddressMode(addrW);
		sd.magFilter = (magF >= 2) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
		sd.minFilter = (minF >= 2) ? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
		sd.mipmapFilter = (mipF >= 2) ? WGPUMipmapFilterMode_Linear : WGPUMipmapFilterMode_Nearest;
		sd.lodMinClamp = 0.0f;
		sd.lodMaxClamp = (mipF == 0) ? 0.25f : 32.0f;
		sd.compare = WGPUCompareFunction_Undefined;
		sd.maxAnisotropy = (sd.magFilter == WGPUFilterMode_Linear &&
			sd.minFilter == WGPUFilterMode_Linear &&
			sd.mipmapFilter == WGPUMipmapFilterMode_Linear &&
			aniso > 1)
			? static_cast<uint16_t>((std::min)(aniso, 16u))
			: 1u;

		WGPUSampler smp = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateSampler, m_device, &sd);
		if (smp)
			m_samplerCache[key] = smp;
		return smp;
	}

	WGPURenderPipeline RenderDeviceWebGPU::GetOrCreateRenderPipeline(EPrimitiveType primitiveType, WGPUIndexFormat stripIndexFormat)
	{
		if (!m_boundProgram || !m_boundProgram->IsValid())
			return nullptr;

		uint32_t dummyCount = 0;
		const WGPUPrimitiveTopology wgpuTopo = MapPrimitiveTopology(primitiveType, 1u, dummyCount);
		const bool isStripTopo = (wgpuTopo == WGPUPrimitiveTopology_TriangleStrip || wgpuTopo == WGPUPrimitiveTopology_LineStrip);
		const WGPUIndexFormat effStripIdxFmt = isStripTopo ? stripIndexFormat : WGPUIndexFormat_Undefined;

		// Compute render target format signature for PSO cache key
		WGPUTextureFormat colorFmts[MAX_RENDER_TARGETS] = {};
		uint32_t colorCount = 0;
		WGPUTextureFormat depthFmt = WGPUTextureFormat_Undefined;

		if (m_boundRTCount > 0)
		{
			for (uint32_t i = 0; i < m_boundRTCount && i < MAX_RENDER_TARGETS; ++i)
			{
				WebGPURenderTarget* rt = m_boundRTs[i];
				if (!rt)
					continue;
				const uint32_t subCount = rt->GetColorCount();
				if (i == 0 && subCount > 1)
				{
					for (uint32_t j = 0; j < subCount && colorCount < MAX_RENDER_TARGETS; ++j)
					{
						WebGPUTexture* ct = rt->GetWebGPUColor(j);
						if (ct)
							colorFmts[colorCount++] = ct->GetWGPUFormat();
					}
					break;
				}
				WebGPUTexture* ct = rt->GetWebGPUColor(0);
				if (ct)
					colorFmts[colorCount++] = ct->GetWGPUFormat();
			}
			if (m_boundDepthOverride && m_boundDepthOverride->GetWebGPUDepth())
				depthFmt = m_boundDepthOverride->GetWebGPUDepth()->GetWGPUFormat();
			else if (m_boundRTs[0] && m_boundRTs[0]->GetWebGPUDepth())
				depthFmt = m_boundRTs[0]->GetWebGPUDepth()->GetWGPUFormat();
		}
		else
		{
			colorFmts[colorCount++] = m_surfaceFormat;
			depthFmt = m_defaultDepthTex ? m_defaultDepthTex->GetWGPUFormat() : WGPUTextureFormat_Undefined;
		}

		uint64_t rtHash = Fnv1aBytes64(colorFmts, sizeof(colorFmts));
		rtHash = Fnv1aBytes64(&colorCount, sizeof(colorCount), rtHash);
		rtHash = Fnv1aBytes64(&depthFmt, sizeof(depthFmt), rtHash);
		rtHash = Fnv1aBytes64(&effStripIdxFmt, sizeof(effStripIdxFmt), rtHash);

		// Fold vertex declaration + active stream bindings + dividers + strides into vertexHash
		uint64_t vtxHash = m_vertexDecl ? m_vertexDecl->GetHash() : 0ull;
		for (uint32_t s = 0; s < MAX_VERTEX_STREAMS; ++s)
		{
			const uint32_t hasBuf = (m_streams[s].buffer != nullptr) ? 1u : 0u;
			const uint32_t effStride = (m_streams[s].stride > 0)
				? m_streams[s].stride
				: (m_vertexDecl ? m_vertexDecl->GetStride(s) : 0u);
			vtxHash = Fnv1aBytes64(&hasBuf, sizeof(hasBuf), vtxHash);
			vtxHash = Fnv1aBytes64(&effStride, sizeof(effStride), vtxHash);
			vtxHash = Fnv1aBytes64(&m_streams[s].frequencyDivider, sizeof(m_streams[s].frequencyDivider), vtxHash);
		}

		const uint64_t psoKey = m_state.ComputePSOHash(m_boundProgram->GetHash(), vtxHash, primitiveType, rtHash);
		auto it = m_psoCache.find(psoKey);
		if (it != m_psoCache.end())
			return it->second;

		const WebGPUShaderReflection& refl = m_boundProgram->GetReflection();

		// Build vertex buffer layouts (slots 0..7) ONLY for locations actually consumed by VS ParaStageIn
		std::vector<WGPUVertexAttribute> slotAttrs[FALLBACK_VB_SLOT + 1];
		uint32_t providedMask = 0;

		if (m_vertexDecl)
		{
			for (const VertexElement& el : m_vertexDecl->GetElements())
			{
				if (el.IsEndDeclare() || el.Stream >= MAX_VERTEX_STREAMS)
					continue;
				const int32_t locSigned = MapVertexElementToLocation(el.Usage, el.UsageIndex);
				if (locSigned < 0 || locSigned >= 16)
					continue;
				const uint32_t loc = static_cast<uint32_t>(locSigned);
				if ((refl.vsInputLocMask & (1u << loc)) == 0)
					continue;
				if ((providedMask & (1u << loc)) != 0)
					continue;
				if (!m_streams[el.Stream].buffer)
					continue;

				WGPUVertexAttribute va = {};
				va.format = MapDeclTypeToWGPU(el.Type);
				va.offset = el.Offset;
				va.shaderLocation = loc;
				const uint32_t boundStride = m_streams[el.Stream].stride;
				if (boundStride > 0 && va.offset + GetVertexFormatByteSize(va.format) > AlignUpU32(boundStride, 4u))
					continue;
				slotAttrs[el.Stream].push_back(va);
				providedMask |= (1u << loc);
			}
		}

		// Route any unbound VS input locations to m_fallbackZeroVB [0,0,0,1] on slot 7 (arrayStride = 0)
		for (uint32_t loc = 0; loc < 16u; ++loc)
		{
			if ((refl.vsInputLocMask & (1u << loc)) != 0 && (providedMask & (1u << loc)) == 0)
			{
				WGPUVertexAttribute va = {};
				va.format = WGPUVertexFormat_Float32x4;
				va.offset = 0;
				va.shaderLocation = loc;
				slotAttrs[FALLBACK_VB_SLOT].push_back(va);
			}
		}

		WGPUVertexBufferLayout vbLayouts[FALLBACK_VB_SLOT + 1] = {};
		uint32_t maxUsedVbSlot = 0;
		bool anyVbSlot = false;
		for (uint32_t slot = 0; slot <= FALLBACK_VB_SLOT; ++slot)
		{
			if (slotAttrs[slot].empty())
			{
				vbLayouts[slot].arrayStride = 0;
#if defined(WGPUINTERNAL_HAVE_EMDAWNWEBGPU_HEADER)
				vbLayouts[slot].stepMode = WGPUVertexStepMode_Undefined;
#else
				vbLayouts[slot].stepMode = WGPUVertexStepMode_VertexBufferNotUsed;
#endif
				vbLayouts[slot].attributeCount = 0;
				vbLayouts[slot].attributes = nullptr;
				continue;
			}
			anyVbSlot = true;
			maxUsedVbSlot = slot;
			if (slot == FALLBACK_VB_SLOT)
			{
				vbLayouts[slot].arrayStride = 0;
				vbLayouts[slot].stepMode = WGPUVertexStepMode_Vertex;
			}
			else
			{
				uint32_t stride = (m_streams[slot].stride > 0)
					? m_streams[slot].stride
					: (m_vertexDecl ? m_vertexDecl->GetStride(slot) : 16u);
				for (const WGPUVertexAttribute& attr : slotAttrs[slot])
				{
					const uint32_t reqEnd = static_cast<uint32_t>(attr.offset) + GetVertexFormatByteSize(attr.format);
					if (reqEnd > stride)
						stride = reqEnd;
				}
				vbLayouts[slot].arrayStride = AlignUpU32((std::max)(stride, 4u), 4u);
				const bool isInstanced = ((m_streams[slot].frequencyDivider & 0x80000000u) != 0);
				vbLayouts[slot].stepMode = isInstanced ? WGPUVertexStepMode_Instance : WGPUVertexStepMode_Vertex;
			}
			vbLayouts[slot].attributeCount = slotAttrs[slot].size();
			vbLayouts[slot].attributes = slotAttrs[slot].data();
		}

		const uint32_t cullMode = m_state.GetRenderState(ERenderState::CULLMODE);
		WGPUCullMode wgpuCull = WGPUCullMode_None;
		WGPUFrontFace wgpuFront = WGPUFrontFace_CW;
		// ParaEngine LHS D3D convention: RSV_CULL_CCW culls backfaces of clockwise-wound front triangles
		if (cullMode == RSV_CULL_CCW)
		{
			wgpuFront = WGPUFrontFace_CW;
			wgpuCull = WGPUCullMode_Back;
		}
		else if (cullMode == RSV_CULL_CW)
		{
			wgpuFront = WGPUFrontFace_CW;
			wgpuCull = WGPUCullMode_Front;
		}

		// Depth/Stencil state
		WGPUDepthStencilState dsState = {};
		const bool hasDepthAttachment = (depthFmt != WGPUTextureFormat_Undefined);
		if (hasDepthAttachment)
		{
			const bool zEnable = (m_state.GetRenderState(ERenderState::ZENABLE) != 0);
			const bool zWrite  = (m_state.GetRenderState(ERenderState::ZWRITEENABLE) != 0);
			dsState.format = depthFmt;
			dsState.depthWriteEnabled = ParaWgpuOptionalBool(zEnable && zWrite);
			dsState.depthCompare = zEnable
				? MapCompareFunc(m_state.GetRenderState(ERenderState::ZFUNC))
				: WGPUCompareFunction_Always;

			const bool stencilEnable = (m_state.GetRenderState(ERenderState::STENCILENABLE) != 0) &&
				(depthFmt == WGPUTextureFormat_Depth24PlusStencil8 || depthFmt == WGPUTextureFormat_Depth32FloatStencil8);
			if (stencilEnable)
			{
				dsState.stencilFront.compare     = MapCompareFunc(m_state.GetRenderState(ERenderState::STENCILFUNC));
				dsState.stencilFront.failOp      = MapStencilOp(m_state.GetRenderState(ERenderState::STENCILFAIL));
				dsState.stencilFront.depthFailOp = MapStencilOp(m_state.GetRenderState(ERenderState::STENCILZFAIL));
				dsState.stencilFront.passOp      = MapStencilOp(m_state.GetRenderState(ERenderState::STENCILPASS));

				if (m_state.GetRenderState(ERenderState::TWOSIDEDSTENCILMODE) != 0)
				{
					dsState.stencilBack.compare     = MapCompareFunc(m_state.GetRenderState(ERenderState::CCW_STENCILFUNC));
					dsState.stencilBack.failOp      = MapStencilOp(m_state.GetRenderState(ERenderState::CCW_STENCILFAIL));
					dsState.stencilBack.depthFailOp = MapStencilOp(m_state.GetRenderState(ERenderState::CCW_STENCILZFAIL));
					dsState.stencilBack.passOp      = MapStencilOp(m_state.GetRenderState(ERenderState::CCW_STENCILPASS));
				}
				else
				{
					dsState.stencilBack = dsState.stencilFront;
				}
				dsState.stencilReadMask = m_state.GetRenderState(ERenderState::STENCILMASK) & 0xFFu;
				dsState.stencilWriteMask = m_state.GetRenderState(ERenderState::STENCILWRITEMASK) & 0xFFu;
			}
			else
			{
				dsState.stencilFront = { WGPUCompareFunction_Always, WGPUStencilOperation_Keep, WGPUStencilOperation_Keep, WGPUStencilOperation_Keep };
				dsState.stencilBack  = dsState.stencilFront;
				dsState.stencilReadMask = 0xFFu;
				dsState.stencilWriteMask = 0u;
			}

			// WebGPU spec forbids non-zero depthBias / depthBiasSlopeScale on non-triangle topologies
			if (wgpuTopo == WGPUPrimitiveTopology_TriangleList || wgpuTopo == WGPUPrimitiveTopology_TriangleStrip)
			{
				auto bitsToFloat = [](uint32_t bits) -> float {
					float f = 0.0f;
					std::memcpy(&f, &bits, sizeof(float));
					return f;
				};
				const float rawDepthBias = bitsToFloat(m_state.GetRenderState(ERenderState::DEPTHBIAS));
				const float slopeBias = bitsToFloat(m_state.GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS));
				dsState.depthBias = std::isfinite(rawDepthBias) ? static_cast<int32_t>(std::lround(rawDepthBias * 16777216.0f)) : 0;
				dsState.depthBiasSlopeScale = std::isfinite(slopeBias) ? slopeBias : 0.0f;
			}
			dsState.depthBiasClamp = 0.0f;
		}

		// Blend & ColorTargetState
		WGPUBlendState blendState = {};
		const bool blendEnable = (m_state.GetRenderState(ERenderState::ALPHABLENDENABLE) != 0);
		if (blendEnable)
		{
			const uint32_t srcCol = m_state.GetRenderState(ERenderState::SRCBLEND);
			const uint32_t dstCol = m_state.GetRenderState(ERenderState::DESTBLEND);
			const uint32_t opCol  = m_state.GetRenderState(ERenderState::BLENDOP);
			const bool sepAlpha   = (m_state.GetRenderState(ERenderState::SEPARATEALPHABLENDENABLE) != 0);
			const uint32_t srcAlp = sepAlpha ? m_state.GetRenderState(ERenderState::SRCBLENDALPHA) : srcCol;
			const uint32_t dstAlp = sepAlpha ? m_state.GetRenderState(ERenderState::DESTBLENDALPHA) : dstCol;
			const uint32_t opAlp  = sepAlpha ? m_state.GetRenderState(ERenderState::BLENDOPALPHA) : opCol;

			blendState.color.operation = MapBlendOp(opCol);
			blendState.color.srcFactor = MapBlendFactor(srcCol, false);
			blendState.color.dstFactor = MapBlendFactor(dstCol, false);
			blendState.alpha.operation = MapBlendOp(opAlp);
			blendState.alpha.srcFactor = MapBlendFactor(srcAlp, true);
			blendState.alpha.dstFactor = MapBlendFactor(dstAlp, true);
		}

		const uint32_t cwEnable = m_state.GetRenderState(ERenderState::COLORWRITEENABLE);
		WGPUColorWriteMaskFlags writeMask = 0;
		if (cwEnable & 0x1u) writeMask |= WGPUColorWriteMask_Red;
		if (cwEnable & 0x2u) writeMask |= WGPUColorWriteMask_Green;
		if (cwEnable & 0x4u) writeMask |= WGPUColorWriteMask_Blue;
		if (cwEnable & 0x8u) writeMask |= WGPUColorWriteMask_Alpha;

		WGPUColorTargetState colorTargets[MAX_RENDER_TARGETS] = {};
		for (uint32_t i = 0; i < colorCount; ++i)
		{
			colorTargets[i].format = colorFmts[i];
			const bool psWritesTarget = ((refl.psOutputLocMask & (1u << i)) != 0);
			const bool fmtBlendable = !IsUnblendableFloat32WGPUFormat(colorFmts[i]);
			colorTargets[i].blend = (blendEnable && psWritesTarget && fmtBlendable) ? &blendState : nullptr;
			colorTargets[i].writeMask = psWritesTarget ? writeMask : WGPUColorWriteMask_None;
		}

		WGPUFragmentState fragState = {};
		const bool hasFragment = (m_boundProgram->GetPSModule() != nullptr);
		if (hasFragment)
		{
			fragState.module = m_boundProgram->GetPSModule();
			fragState.entryPoint = ParaWgpuLabel(m_boundProgram->GetPSEntry());
			fragState.targetCount = colorCount;
			fragState.targets = (colorCount > 0) ? colorTargets : nullptr;
		}

		WGPURenderPipelineDescriptor rpd = {};
		rpd.label = ParaWgpuLabel("ParaRenderPipeline");
		rpd.layout = m_boundProgram->GetPipelineLayout();
		rpd.vertex.module = m_boundProgram->GetVSModule();
		rpd.vertex.entryPoint = ParaWgpuLabel(m_boundProgram->GetVSEntry());
		rpd.vertex.bufferCount = anyVbSlot ? (maxUsedVbSlot + 1u) : 0u;
		rpd.vertex.buffers = anyVbSlot ? vbLayouts : nullptr;

		rpd.primitive.topology = wgpuTopo;
		rpd.primitive.stripIndexFormat = effStripIdxFmt;
		rpd.primitive.frontFace = wgpuFront;
		rpd.primitive.cullMode = wgpuCull;

		rpd.depthStencil = hasDepthAttachment ? &dsState : nullptr;
		rpd.multisample.count = 1;
		rpd.multisample.mask = 0xFFFFFFFFu;
		rpd.multisample.alphaToCoverageEnabled = 0;
		rpd.fragment = hasFragment ? &fragState : nullptr;

		WGPURenderPipeline pso = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateRenderPipeline, m_device, &rpd);
		if (pso)
			m_psoCache[psoKey] = pso;
		return pso;
	}

	WGPUBindGroup RenderDeviceWebGPU::BuildDrawBindGroup()
	{
		if (!m_boundProgram)
			return nullptr;
		const WebGPUShaderReflection& refl = m_boundProgram->GetReflection();

		if (refl.hasVsUbo && (m_vsUboDirty || !m_currentVsUbo || m_currentVsUboSize < refl.vsUboMinBytes))
		{
			const uint32_t reqBytes = AlignUpU32((std::max)(refl.vsUboMinBytes, static_cast<uint32_t>(m_vsConstantsCpu.size())), 16u);
			if (m_vsConstantsCpu.size() < reqBytes)
				m_vsConstantsCpu.resize(reqBytes, 0);
			WGPUBuffer fresh = AllocateTransientBuffer(WGPUBufferUsage_Uniform, m_vsConstantsCpu.data(), reqBytes, reqBytes);
			if (fresh)
			{
				m_currentVsUbo = fresh;
				m_currentVsUboSize = reqBytes;
				m_vsUboDirty = false;
			}
		}

		if (refl.hasPsUbo && (m_psUboDirty || !m_currentPsUbo || m_currentPsUboSize < refl.psUboMinBytes))
		{
			const uint32_t reqBytes = AlignUpU32((std::max)(refl.psUboMinBytes, static_cast<uint32_t>(m_psConstantsCpu.size())), 16u);
			if (m_psConstantsCpu.size() < reqBytes)
				m_psConstantsCpu.resize(reqBytes, 0);
			WGPUBuffer fresh = AllocateTransientBuffer(WGPUBufferUsage_Uniform, m_psConstantsCpu.data(), reqBytes, reqBytes);
			if (fresh)
			{
				m_currentPsUbo = fresh;
				m_currentPsUboSize = reqBytes;
				m_psUboDirty = false;
			}
		}

		std::vector<WGPUBindGroupEntry> entries;
		entries.reserve(34);

		if (refl.hasVsUbo)
		{
			WGPUBindGroupEntry e = {};
			e.binding = 0;
			e.buffer = m_currentVsUbo ? m_currentVsUbo : m_zeroUboBuffer;
			e.offset = 0;
			e.size = m_currentVsUbo ? m_currentVsUboSize : refl.vsUboMinBytes;
			entries.push_back(e);
		}
		if (refl.hasPsUbo)
		{
			WGPUBindGroupEntry e = {};
			e.binding = 1;
			e.buffer = m_currentPsUbo ? m_currentPsUbo : m_zeroUboBuffer;
			e.offset = 0;
			e.size = m_currentPsUbo ? m_currentPsUboSize : refl.psUboMinBytes;
			entries.push_back(e);
		}

		for (uint32_t s = 0; s < 16u; ++s)
		{
			if (refl.texVisibility[s] != 0)
			{
				WGPUTextureView view = nullptr;
				WebGPUTexture* bound = m_boundTextures[s];
				const WGPUTextureViewDimension expectedDim = refl.texViewDim[s];
				if (bound && bound->GetSRVView() && !IsDepthPixelFormat(bound->GetFormat()))
				{
					const bool unfilterableFloat = !m_supportsFloat32Filterable && IsUnblendableFloat32WGPUFormat(bound->GetWGPUFormat());
					const bool dimMatches =
						(expectedDim == WGPUTextureViewDimension_Cube && bound->IsCubeMap()) ||
						(expectedDim == WGPUTextureViewDimension_3D && bound->Is3D()) ||
						((expectedDim == WGPUTextureViewDimension_2D || expectedDim == WGPUTextureViewDimension_Undefined) && !bound->IsCubeMap() && !bound->Is3D());
					if (dimMatches && !unfilterableFloat)
						view = bound->GetSRVView();
				}
				if (!view)
				{
					if (expectedDim == WGPUTextureViewDimension_Cube && m_fallbackWhiteCube)
						view = m_fallbackWhiteCube->GetSRVView();
					else if (expectedDim == WGPUTextureViewDimension_3D && m_fallbackWhite3D)
						view = m_fallbackWhite3D->GetSRVView();
					else if (m_fallbackWhite2D)
						view = m_fallbackWhite2D->GetSRVView();
				}

				WGPUBindGroupEntry e = {};
				e.binding = 16u + s;
				e.textureView = view;
				entries.push_back(e);
			}
			if (refl.smpVisibility[s] != 0)
			{
				WGPUBindGroupEntry e = {};
				e.binding = 32u + s;
				e.sampler = GetOrCreateSampler(s);
				entries.push_back(e);
			}
		}

		WGPUBindGroupDescriptor bgd = {};
		bgd.label = ParaWgpuLabel("ParaDrawBindGroup");
		bgd.layout = m_boundProgram->GetBindGroupLayout();
		bgd.entryCount = entries.size();
		bgd.entries = entries.empty() ? nullptr : entries.data();

		WGPUBindGroup bg = PARA_WGPU_CALL(m_wgpu, wgpuDeviceCreateBindGroup, m_device, &bgd);
		if (bg)
			m_transientBindGroups.push_back(bg);
		return bg;
	}

	bool RenderDeviceWebGPU::PrepareForDraw(EPrimitiveType primitiveType, WGPUIndexFormat stripIndexFormat)
	{
		if (!m_boundProgram || !m_boundProgram->IsValid())
			return false;
		if (!EnsureActiveRenderPass(false, false, false))
			return false;

		WGPURenderPipeline pso = GetOrCreateRenderPipeline(primitiveType, stripIndexFormat);
		if (!pso)
			return false;

		WGPUBindGroup bg = BuildDrawBindGroup();
		if (!bg)
			return false;

		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetPipeline, m_renderPass, pso);
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetBindGroup, m_renderPass, 0, bg, 0, nullptr);

		if (PARA_WGPU_HAS(m_wgpu, wgpuRenderPassEncoderSetStencilReference))
		{
			const uint32_t stencilRef = m_state.GetRenderState(ERenderState::STENCILREF) & 0xFFu;
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetStencilReference, m_renderPass, stencilRef);
		}
		if (PARA_WGPU_HAS(m_wgpu, wgpuRenderPassEncoderSetBlendConstant))
		{
			const uint32_t bf = m_state.GetRenderState(ERenderState::BLENDFACTOR);
			WGPUColor bc{
				static_cast<double>((bf >> 16) & 0xFFu) / 255.0,
				static_cast<double>((bf >> 8)  & 0xFFu) / 255.0,
				static_cast<double>((bf >> 0)  & 0xFFu) / 255.0,
				static_cast<double>((bf >> 24) & 0xFFu) / 255.0
			};
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetBlendConstant, m_renderPass, &bc);
		}

		// Bind active vertex buffers (slots 0..3) and fallback zero VB (slot 7)
		for (uint32_t s = 0; s < MAX_VERTEX_STREAMS; ++s)
		{
			if (m_streams[s].buffer && m_streams[s].buffer->GetWGPUBuffer())
			{
				const uint64_t bufSize = m_streams[s].buffer->GetAlignedSize();
				const uint64_t off = (std::min)(static_cast<uint64_t>(m_streams[s].offsetInBytes), bufSize) & ~3ull;
				PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetVertexBuffer,
					m_renderPass, s, m_streams[s].buffer->GetWGPUBuffer(), off, bufSize - off);
			}
		}
		if (m_fallbackZeroVB)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetVertexBuffer,
				m_renderPass, FALLBACK_VB_SLOT, m_fallbackZeroVB, 0, 16);
		}
		return true;
	}

	bool RenderDeviceWebGPU::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		if (PrimitiveCount == 0)
			return true;

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			std::vector<uint16_t> fanIdx;
			fanIdx.reserve(PrimitiveCount * 3u);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				fanIdx.push_back(static_cast<uint16_t>(StartVertex));
				fanIdx.push_back(static_cast<uint16_t>(StartVertex + i + 1u));
				fanIdx.push_back(static_cast<uint16_t>(StartVertex + i + 2u));
			}
			WGPUBuffer ib = AllocateTransientBuffer(WGPUBufferUsage_Index, fanIdx.data(), static_cast<uint32_t>(fanIdx.size() * sizeof(uint16_t)));
			if (!ib || !PrepareForDraw(EPrimitiveType::TRIANGLELIST, WGPUIndexFormat_Undefined))
				return false;
			const uint64_t ibAligned = AlignUpU32((std::max)(static_cast<uint32_t>(fanIdx.size() * sizeof(uint16_t)), 16u), 4u);
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetIndexBuffer, m_renderPass, ib, WGPUIndexFormat_Uint16, 0, ibAligned);
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDrawIndexed, m_renderPass, static_cast<uint32_t>(fanIdx.size()), 1, 0, 0, 0);
			return true;
		}

		uint32_t vertCount = 0;
		MapPrimitiveTopology(PrimitiveType, PrimitiveCount, vertCount);
		if (vertCount == 0 || !PrepareForDraw(PrimitiveType, WGPUIndexFormat_Undefined))
			return false;

		uint32_t instanceCount = 1u;
		if ((m_streams[0].frequencyDivider & 0x40000000u) != 0)
			instanceCount = (std::max)(1u, m_streams[0].frequencyDivider & 0x3FFFFFFFu);

		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDraw, m_renderPass, vertCount, instanceCount, StartVertex, 0);
		return true;
	}

	bool RenderDeviceWebGPU::DrawIndexedPrimitive(
		EPrimitiveType Type,
		int BaseVertexIndex,
		uint32_t /*MinIndex*/,
		uint32_t /*NumVertices*/,
		uint32_t indexStart,
		uint32_t PrimitiveCount)
	{
		uint32_t instanceCount = 1u;
		if ((m_streams[0].frequencyDivider & 0x40000000u) != 0)
			instanceCount = (std::max)(1u, m_streams[0].frequencyDivider & 0x3FFFFFFFu);
		return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, instanceCount);
	}

	bool RenderDeviceWebGPU::DrawIndexedInstanced(
		EPrimitiveType primitiveType,
		int baseVertexIndex,
		uint32_t indexStart,
		uint32_t primitiveCount,
		uint32_t instanceCount)
	{
		if (primitiveCount == 0 || instanceCount == 0 || !m_boundIndexBuffer || !m_boundIndexBuffer->GetWGPUBuffer())
			return false;

		if (primitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			const std::vector<uint8_t>& shadow = m_boundIndexBuffer->GetCpuShadow();
			const size_t totalU16 = shadow.size() / sizeof(uint16_t);
			const uint32_t srcFanCount = primitiveCount + 2u;
			if (static_cast<size_t>(indexStart) + srcFanCount > totalU16)
				return false;
			const auto* srcIdx = reinterpret_cast<const uint16_t*>(shadow.data()) + indexStart;
			std::vector<uint16_t> fanIdx(primitiveCount * 3u);
			for (uint32_t i = 0; i < primitiveCount; ++i)
			{
				fanIdx[i * 3u + 0] = srcIdx[0];
				fanIdx[i * 3u + 1] = srcIdx[i + 1u];
				fanIdx[i * 3u + 2] = srcIdx[i + 2u];
			}
			const uint32_t ibBytes = static_cast<uint32_t>(fanIdx.size() * sizeof(uint16_t));
			WGPUBuffer transIb = AllocateTransientBuffer(WGPUBufferUsage_Index, fanIdx.data(), ibBytes);
			if (!transIb || !PrepareForDraw(EPrimitiveType::TRIANGLELIST, WGPUIndexFormat_Undefined))
				return false;
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetIndexBuffer,
				m_renderPass, transIb, WGPUIndexFormat_Uint16, 0, AlignUpU32((std::max)(ibBytes, 16u), 4u));
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDrawIndexed,
				m_renderPass, static_cast<uint32_t>(fanIdx.size()), instanceCount, 0, baseVertexIndex, 0);
			return true;
		}

		uint32_t idxCount = 0;
		MapPrimitiveTopology(primitiveType, primitiveCount, idxCount);
		if (idxCount == 0 || !PrepareForDraw(primitiveType, WGPUIndexFormat_Uint16))
			return false;

		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetIndexBuffer,
			m_renderPass,
			m_boundIndexBuffer->GetWGPUBuffer(),
			WGPUIndexFormat_Uint16,
			0,
			m_boundIndexBuffer->GetAlignedSize());
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDrawIndexed,
			m_renderPass, idxCount, instanceCount, indexStart, baseVertexIndex, 0);
		return true;
	}

	bool RenderDeviceWebGPU::DrawPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t PrimitiveCount,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!pVertexStreamZeroData || PrimitiveCount == 0 || VertexStreamZeroStride == 0)
			return false;

		uint32_t vertCount = 0;
		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
			vertCount = PrimitiveCount + 2u;
		else
			MapPrimitiveTopology(PrimitiveType, PrimitiveCount, vertCount);

		const uint32_t vbBytes = vertCount * VertexStreamZeroStride;
		const uint32_t vbAligned = AlignUpU32((std::max)(vbBytes, 16u), 4u);
		WGPUBuffer transVb = AllocateTransientBuffer(WGPUBufferUsage_Vertex, pVertexStreamZeroData, vbBytes);
		if (!transVb)
			return false;

		WebGPUBuffer borrowedVbWrap(transVb, vbAligned);
		const StreamBinding saved0 = m_streams[0];
		m_streams[0] = { &borrowedVbWrap, 0, VertexStreamZeroStride, 0 };

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			std::vector<uint16_t> fanIdx;
			fanIdx.reserve(PrimitiveCount * 3u);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				fanIdx.push_back(0);
				fanIdx.push_back(static_cast<uint16_t>(i + 1u));
				fanIdx.push_back(static_cast<uint16_t>(i + 2u));
			}
			const uint32_t ibBytes = static_cast<uint32_t>(fanIdx.size() * sizeof(uint16_t));
			WGPUBuffer transIb = AllocateTransientBuffer(WGPUBufferUsage_Index, fanIdx.data(), ibBytes);
			const bool prepared = transIb && PrepareForDraw(EPrimitiveType::TRIANGLELIST, WGPUIndexFormat_Undefined);
			m_streams[0] = saved0;
			if (!prepared)
				return false;

			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetIndexBuffer,
				m_renderPass, transIb, WGPUIndexFormat_Uint16, 0, AlignUpU32((std::max)(ibBytes, 16u), 4u));
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDrawIndexed,
				m_renderPass, static_cast<uint32_t>(fanIdx.size()), 1, 0, 0, 0);
			return true;
		}

		const bool prepared = PrepareForDraw(PrimitiveType, WGPUIndexFormat_Undefined);
		m_streams[0] = saved0;
		if (!prepared)
			return false;

		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDraw, m_renderPass, vertCount, 1, 0, 0);
		return true;
	}

	bool RenderDeviceWebGPU::DrawIndexedPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t MinVertexIndex,
		uint32_t NumVertices,
		uint32_t PrimitiveCount,
		const void* pIndexData,
		PixelFormat IndexDataFormat,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!pIndexData || !pVertexStreamZeroData || PrimitiveCount == 0 || NumVertices == 0 || VertexStreamZeroStride == 0)
			return false;

		const bool is32Bit = (IndexDataFormat == PixelFormat::INDEX32);
		const WGPUIndexFormat wgpuIdxFmt = is32Bit ? WGPUIndexFormat_Uint32 : WGPUIndexFormat_Uint16;
		const uint32_t vbBytes = (MinVertexIndex + NumVertices) * VertexStreamZeroStride;
		const uint32_t vbAligned = AlignUpU32((std::max)(vbBytes, 16u), 4u);

		EPrimitiveType drawPrimType = PrimitiveType;
		uint32_t idxCount = 0;
		std::vector<uint8_t> expandedFanIb;
		const void* uploadIdxPtr = pIndexData;

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			drawPrimType = EPrimitiveType::TRIANGLELIST;
			idxCount = PrimitiveCount * 3u;
			expandedFanIb.resize(idxCount * (is32Bit ? 4u : 2u));
			if (is32Bit)
			{
				const auto* src = static_cast<const uint32_t*>(pIndexData);
				auto* dst = reinterpret_cast<uint32_t*>(expandedFanIb.data());
				for (uint32_t i = 0; i < PrimitiveCount; ++i)
				{
					dst[i * 3u + 0] = src[0];
					dst[i * 3u + 1] = src[i + 1u];
					dst[i * 3u + 2] = src[i + 2u];
				}
			}
			else
			{
				const auto* src = static_cast<const uint16_t*>(pIndexData);
				auto* dst = reinterpret_cast<uint16_t*>(expandedFanIb.data());
				for (uint32_t i = 0; i < PrimitiveCount; ++i)
				{
					dst[i * 3u + 0] = src[0];
					dst[i * 3u + 1] = src[i + 1u];
					dst[i * 3u + 2] = src[i + 2u];
				}
			}
			uploadIdxPtr = expandedFanIb.data();
		}
		else
		{
			MapPrimitiveTopology(PrimitiveType, PrimitiveCount, idxCount);
		}

		const uint32_t ibBytes = idxCount * (is32Bit ? 4u : 2u);

		WGPUBuffer transVb = AllocateTransientBuffer(WGPUBufferUsage_Vertex, pVertexStreamZeroData, vbBytes);
		WGPUBuffer transIb = AllocateTransientBuffer(WGPUBufferUsage_Index, uploadIdxPtr, ibBytes);
		if (!transVb || !transIb)
			return false;

		WebGPUBuffer borrowedVbWrap(transVb, vbAligned);
		const StreamBinding saved0 = m_streams[0];
		m_streams[0] = { &borrowedVbWrap, 0, VertexStreamZeroStride, 0 };
		const bool prepared = PrepareForDraw(drawPrimType, wgpuIdxFmt);
		m_streams[0] = saved0;
		if (!prepared)
			return false;

		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetIndexBuffer,
			m_renderPass,
			transIb,
			wgpuIdxFmt,
			0,
			AlignUpU32((std::max)(ibBytes, 16u), 4u));
		PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderDrawIndexed, m_renderPass, idxCount, 1, 0, 0, 0);
		return true;
	}

	uint32_t RenderDeviceWebGPU::GetRenderState(const ERenderState& State)
	{
		return m_state.GetRenderState(State);
	}

	bool RenderDeviceWebGPU::SetRenderState(const ERenderState State, const uint32_t Value)
	{
		const bool changed = m_state.SetRenderState(State, Value);
		if (changed && State == ERenderState::SCISSORTESTENABLE && m_renderPass)
		{
			if (Value != 0)
			{
				const RECT& sc = m_state.GetScissorRect();
				const uint32_t sx = static_cast<uint32_t>((std::max)(static_cast<LONG>(0), (std::min)(sc.left, static_cast<LONG>(m_activeRTWidth))));
				const uint32_t sy = static_cast<uint32_t>((std::max)(static_cast<LONG>(0), (std::min)(sc.top, static_cast<LONG>(m_activeRTHeight))));
				const uint32_t ex = static_cast<uint32_t>((std::max)(static_cast<LONG>(sx), (std::min)(sc.right, static_cast<LONG>(m_activeRTWidth))));
				const uint32_t ey = static_cast<uint32_t>((std::max)(static_cast<LONG>(sy), (std::min)(sc.bottom, static_cast<LONG>(m_activeRTHeight))));
				PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetScissorRect, m_renderPass, sx, sy, ex - sx, ey - sy);
			}
			else
			{
				PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetScissorRect, m_renderPass, 0, 0, m_activeRTWidth, m_activeRTHeight);
			}
		}
		return true;
	}

	bool RenderDeviceWebGPU::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		m_state.SetClipPlane(Index, pPlane);
		return true;
	}

	bool RenderDeviceWebGPU::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		if (stage >= RHIStateTracker::MAX_SAMPLER_STAGES)
			return false;
		m_boundTextures[stage] = texture ? static_cast<WebGPUTexture*>(texture->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceWebGPU::SetTransform(ETransformsStateType /*State*/, DeviceMatrix_ptr /*pMatrix*/)
	{
		return true;
	}

	bool RenderDeviceWebGPU::SetFVF(uint32_t /*FVF*/)
	{
		return true;
	}

	void RenderDeviceWebGPU::SetCursorPosition(int /*X*/, int /*Y*/, uint32_t /*Flags*/)
	{
	}

	bool RenderDeviceWebGPU::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		m_state.SetSamplerState(stage, type, value);
		return true;
	}

	bool RenderDeviceWebGPU::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		if (!value)
			return false;
		*value = m_state.GetSamplerState(stage, type);
		return true;
	}

	bool RenderDeviceWebGPU::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
	{
		m_vertexDecl = pVertexDeclaration;
		return true;
	}

	bool RenderDeviceWebGPU::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
	{
		if (!pVertexElements || !ppDecl)
			return false;
		*ppDecl = new CVertexDeclaration(pVertexElements);
		return true;
	}

	bool RenderDeviceWebGPU::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		m_boundIndexBuffer = pIndexData ? static_cast<WebGPUBuffer*>(pIndexData->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceWebGPU::SetStreamSource(
		uint32_t StreamNumber,
		VertexBufferDevicePtr_type pStreamData,
		uint32_t OffsetInBytes,
		uint32_t Stride)
	{
		if (StreamNumber >= MAX_VERTEX_STREAMS)
			return false;
		m_streams[StreamNumber].buffer = pStreamData ? static_cast<WebGPUBuffer*>(pStreamData->GetNativeHandle()) : nullptr;
		m_streams[StreamNumber].offsetInBytes = OffsetInBytes;
		m_streams[StreamNumber].stride = Stride;
		return true;
	}

	bool RenderDeviceWebGPU::SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider)
	{
		if (streamNumber >= MAX_VERTEX_STREAMS)
			return false;
		m_streams[streamNumber].frequencyDivider = divider;
		return true;
	}

	ParaViewport RenderDeviceWebGPU::GetViewport()
	{
		return m_state.GetViewport();
	}

	bool RenderDeviceWebGPU::SetViewport(const ParaViewport& viewport)
	{
		m_state.SetViewport(viewport);
		if (m_renderPass)
			ApplyClampedViewport(viewport);
		return true;
	}

	bool RenderDeviceWebGPU::SetClearColor(const Color4f& color)
	{
		m_state.SetClearColor(color);
		return true;
	}

	bool RenderDeviceWebGPU::SetClearDepth(const float depth)
	{
		m_state.SetClearDepth(depth);
		return true;
	}

	bool RenderDeviceWebGPU::SetClearStencil(const int stencil)
	{
		m_state.SetClearStencil(stencil);
		return true;
	}

	bool RenderDeviceWebGPU::Clear(bool color, bool depth, bool stencil)
	{
		if (!color && !depth && !stencil)
			return true;
		return EnsureActiveRenderPass(color, depth, stencil);
	}

	bool RenderDeviceWebGPU::SetScissorRect(RECT* pRect)
	{
		m_state.SetScissorRect(pRect);
		if (m_renderPass && m_state.GetRenderState(ERenderState::SCISSORTESTENABLE) != 0)
		{
			const RECT& sc = m_state.GetScissorRect();
			const uint32_t sx = static_cast<uint32_t>((std::max)(static_cast<LONG>(0), (std::min)(sc.left, static_cast<LONG>(m_activeRTWidth))));
			const uint32_t sy = static_cast<uint32_t>((std::max)(static_cast<LONG>(0), (std::min)(sc.top, static_cast<LONG>(m_activeRTHeight))));
			const uint32_t ex = static_cast<uint32_t>((std::max)(static_cast<LONG>(sx), (std::min)(sc.right, static_cast<LONG>(m_activeRTWidth))));
			const uint32_t ey = static_cast<uint32_t>((std::max)(static_cast<LONG>(sy), (std::min)(sc.bottom, static_cast<LONG>(m_activeRTHeight))));
			PARA_WGPU_CALL(m_wgpu, wgpuRenderPassEncoderSetScissorRect, m_renderPass, sx, sy, ex - sx, ey - sy);
		}
		return true;
	}

	bool RenderDeviceWebGPU::GetScissorRect(RECT* pRect)
	{
		if (!pRect)
			return false;
		*pRect = m_state.GetScissorRect();
		return true;
	}

	bool RenderDeviceWebGPU::BeginScene()
	{
		return true;
	}

	bool RenderDeviceWebGPU::EndScene()
	{
		EndActiveRenderPass();
		return true;
	}

	bool RenderDeviceWebGPU::Present()
	{
		if (!m_surfaceClearedThisFrame && !m_currentSurfaceView)
		{
			EnsureActiveRenderPass(true, true, true);
		}
		EndActiveRenderPass();

		if (m_cmdEncoder)
		{
			WGPUCommandBufferDescriptor cbd = {};
			cbd.label = ParaWgpuLabel("ParaPresentCommandBuffer");
			WGPUCommandBuffer cmd = PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderFinish, m_cmdEncoder, &cbd);
			PARA_WGPU_CALL(m_wgpu, wgpuCommandEncoderRelease, m_cmdEncoder);
			m_cmdEncoder = nullptr;
			if (cmd)
			{
				PARA_WGPU_CALL(m_wgpu, wgpuQueueSubmit, m_queue, 1, &cmd);
				PARA_WGPU_CALL(m_wgpu, wgpuCommandBufferRelease, cmd);
			}
		}

		if (m_surface && m_currentSurfaceTex)
		{
#if !defined(__EMSCRIPTEN__)
			PARA_WGPU_CALL(m_wgpu, wgpuSurfacePresent, m_surface);
#endif
		}

		if (m_currentSurfaceView)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureViewRelease, m_currentSurfaceView);
			m_currentSurfaceView = nullptr;
		}
		if (m_currentSurfaceTex)
		{
			PARA_WGPU_CALL(m_wgpu, wgpuTextureRelease, m_currentSurfaceTex);
			m_currentSurfaceTex = nullptr;
		}

		m_surfaceClearedThisFrame = false;
		m_wgpu.ProcessEvents(m_instance, m_device);
		FlushFrameTransientBuffers();
		return true;
	}

	void RenderDeviceWebGPU::Flush()
	{
		ExecuteAndSyncGPU();
	}
}

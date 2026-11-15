#include "ParaEngine.h"
#include "RenderDeviceD3D11.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "RHISoftwareS3TC.h"

#include <d3dcompiler.h>
#include <algorithm>
#include <cstring>
#include <cmath>

using Microsoft::WRL::ComPtr;

namespace ParaEngine
{
	namespace
	{
		using PFN_D3DCompileDyn = HRESULT(WINAPI*)(
			LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
		using PFN_D3DReflectDyn = HRESULT(WINAPI*)(
			LPCVOID, SIZE_T, REFIID, void**);

		struct D3D11CompilerLoader
		{
			HMODULE hCompiler = nullptr;
			PFN_D3DCompileDyn pfnCompile = nullptr;
			PFN_D3DReflectDyn pfnReflect = nullptr;

			D3D11CompilerLoader()
			{
				const char* compilerDlls[] = { "d3dcompiler_47.dll", "d3dcompiler_46.dll", "d3dcompiler_43.dll" };
				for (const char* name : compilerDlls)
				{
					hCompiler = ::LoadLibraryA(name);
					if (hCompiler)
						break;
				}
				if (hCompiler)
				{
					pfnCompile = reinterpret_cast<PFN_D3DCompileDyn>(::GetProcAddress(hCompiler, "D3DCompile"));
					pfnReflect = reinterpret_cast<PFN_D3DReflectDyn>(::GetProcAddress(hCompiler, "D3DReflect"));
				}
			}
		};

		const D3D11CompilerLoader& GetD3D11Compiler()
		{
			static D3D11CompilerLoader s_compiler;
			return s_compiler;
		}

		inline void HashCombineU64(uint64_t& h, uint64_t v)
		{
			h ^= v;
			h *= 1099511628211ULL;
		}

		inline void HashCombineU32(uint64_t& h, uint32_t v)
		{
			h ^= static_cast<uint64_t>(v);
			h *= 1099511628211ULL;
		}

		uint64_t ComputeByteHash64(const void* data, size_t size)
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

		bool IsDepthPixelFormat(PixelFormat fmt)
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

		DXGI_FORMAT MapColorPixelFormatToDXGI(PixelFormat fmt, D3D_FEATURE_LEVEL fl, uint32_t& outBytesPerBlock, bool& outIsBlockCompressed)
		{
			outBytesPerBlock = 4;
			outIsBlockCompressed = false;
			switch (fmt)
			{
			case PixelFormat::A8R8G8B8:
			case PixelFormat::X8R8G8B8:
				outBytesPerBlock = 4;
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case PixelFormat::A8B8G8R8:
			case PixelFormat::X8B8G8R8:
				outBytesPerBlock = 4;
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			case PixelFormat::A8:
				outBytesPerBlock = 1;
				return DXGI_FORMAT_A8_UNORM;
			case PixelFormat::R16F:
				outBytesPerBlock = 2;
				return DXGI_FORMAT_R16_FLOAT;
			case PixelFormat::R32F:
				outBytesPerBlock = 4;
				return DXGI_FORMAT_R32_FLOAT;
			case PixelFormat::G16R16F:
				outBytesPerBlock = 4;
				return DXGI_FORMAT_R16G16_FLOAT;
			case PixelFormat::G32R32F:
				outBytesPerBlock = 8;
				return DXGI_FORMAT_R32G32_FLOAT;
			case PixelFormat::A16B16G16R16F:
				outBytesPerBlock = 8;
				return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case PixelFormat::A32B32G32R32F:
				outBytesPerBlock = 16;
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case PixelFormat::A16B16G16R16:
				outBytesPerBlock = 8;
				return DXGI_FORMAT_R16G16B16A16_UNORM;
			case PixelFormat::A2B10G10R10:
			case PixelFormat::A2R10G10B10:
				outBytesPerBlock = 4;
				return DXGI_FORMAT_R10G10B10A2_UNORM;
			case PixelFormat::DXT1:
				outBytesPerBlock = 8;
				outIsBlockCompressed = true;
				return DXGI_FORMAT_BC1_UNORM;
			case PixelFormat::DXT2:
			case PixelFormat::DXT3:
				outBytesPerBlock = 16;
				outIsBlockCompressed = true;
				return DXGI_FORMAT_BC2_UNORM;
			case PixelFormat::DXT4:
			case PixelFormat::DXT5:
				outBytesPerBlock = 16;
				outIsBlockCompressed = true;
				return DXGI_FORMAT_BC3_UNORM;
			case PixelFormat::BC7:
				if (fl >= D3D_FEATURE_LEVEL_11_0)
				{
					outBytesPerBlock = 16;
					outIsBlockCompressed = true;
					return DXGI_FORMAT_BC7_UNORM;
				}
				return DXGI_FORMAT_UNKNOWN;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		bool ComputePrimitiveTopologyAndCount(EPrimitiveType primType, uint32_t primCount, D3D11_PRIMITIVE_TOPOLOGY& outTopo, uint32_t& outVertexOrIndexCount)
		{
			if (primCount == 0)
				return false;
			switch (primType)
			{
			case EPrimitiveType::POINTLIST:
				outTopo = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
				outVertexOrIndexCount = primCount;
				return true;
			case EPrimitiveType::LINELIST:
				outTopo = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
				outVertexOrIndexCount = primCount * 2u;
				return true;
			case EPrimitiveType::LINESTRIP:
				outTopo = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
				outVertexOrIndexCount = primCount + 1u;
				return true;
			case EPrimitiveType::TRIANGLELIST:
				outTopo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				outVertexOrIndexCount = primCount * 3u;
				return true;
			case EPrimitiveType::TRIANGLESTRIP:
				outTopo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
				outVertexOrIndexCount = primCount + 2u;
				return true;
			case EPrimitiveType::TRIANGLEFAN:
				// Handled via index expansion into TRIANGLELIST
				outTopo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				outVertexOrIndexCount = primCount * 3u;
				return true;
			default:
				return false;
			}
		}

		D3D11_COMPARISON_FUNC ToD3D11CmpFunc(uint32_t rsvCmp)
		{
			switch (rsvCmp)
			{
			case RSV_CMP_NEVER:        return D3D11_COMPARISON_NEVER;
			case RSV_CMP_LESS:         return D3D11_COMPARISON_LESS;
			case RSV_CMP_EQUAL:        return D3D11_COMPARISON_EQUAL;
			case RSV_CMP_LESSEQUAL:    return D3D11_COMPARISON_LESS_EQUAL;
			case RSV_CMP_GREATER:      return D3D11_COMPARISON_GREATER;
			case RSV_CMP_NOTEQUAL:     return D3D11_COMPARISON_NOT_EQUAL;
			case RSV_CMP_GREATEREQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
			case RSV_CMP_ALWAYS:
			default:                   return D3D11_COMPARISON_ALWAYS;
			}
		}

		D3D11_BLEND ToD3D11Blend(uint32_t rsvBlend, bool isAlphaChannel)
		{
			switch (rsvBlend)
			{
			case RSV_BLEND_ZERO:         return D3D11_BLEND_ZERO;
			case RSV_BLEND_ONE:          return D3D11_BLEND_ONE;
			case RSV_BLEND_SRCCOLOR:     return isAlphaChannel ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_SRC_COLOR;
			case RSV_BLEND_INVSRCCOLOR:  return isAlphaChannel ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_INV_SRC_COLOR;
			case RSV_BLEND_SRCALPHA:     return D3D11_BLEND_SRC_ALPHA;
			case RSV_BLEND_INVSRCALPHA:  return D3D11_BLEND_INV_SRC_ALPHA;
			case RSV_BLEND_DESTALPHA:    return D3D11_BLEND_DEST_ALPHA;
			case RSV_BLEND_INVDESTALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
			case RSV_BLEND_DESTCOLOR:    return isAlphaChannel ? D3D11_BLEND_DEST_ALPHA : D3D11_BLEND_DEST_COLOR;
			case RSV_BLEND_INVDESTCOLOR: return isAlphaChannel ? D3D11_BLEND_INV_DEST_ALPHA : D3D11_BLEND_INV_DEST_COLOR;
			case RSV_BLEND_SRCALPHASAT:  return D3D11_BLEND_SRC_ALPHA_SAT;
			case RSV_BLEND_BLENDFACTOR:  return D3D11_BLEND_BLEND_FACTOR;
			case RSV_BLEND_INVBLENDFACTOR: return D3D11_BLEND_INV_BLEND_FACTOR;
			default:                     return D3D11_BLEND_ONE;
			}
		}

		D3D11_BLEND_OP ToD3D11BlendOp(uint32_t op)
		{
			switch (op)
			{
			case 2:  return D3D11_BLEND_OP_SUBTRACT;
			case 3:  return D3D11_BLEND_OP_REV_SUBTRACT;
			case 4:  return D3D11_BLEND_OP_MIN;
			case 5:  return D3D11_BLEND_OP_MAX;
			case 1:
			default: return D3D11_BLEND_OP_ADD;
			}
		}

		D3D11_STENCIL_OP ToD3D11StencilOp(uint32_t op)
		{
			switch (op)
			{
			case RSV_STENCILOP_ZERO:    return D3D11_STENCIL_OP_ZERO;
			case RSV_STENCILOP_REPLACE: return D3D11_STENCIL_OP_REPLACE;
			case RSV_STENCILOP_INCRSAT: return D3D11_STENCIL_OP_INCR_SAT;
			case RSV_STENCILOP_DECRSAT: return D3D11_STENCIL_OP_DECR_SAT;
			case RSV_STENCILOP_INVERT:  return D3D11_STENCIL_OP_INVERT;
			case RSV_STENCILOP_INCR:    return D3D11_STENCIL_OP_INCR;
			case RSV_STENCILOP_DECR:    return D3D11_STENCIL_OP_DECR;
			case RSV_STENCILOP_KEEP:
			default:                    return D3D11_STENCIL_OP_KEEP;
			}
		}

		D3D11_TEXTURE_ADDRESS_MODE ToD3D11AddressMode(uint32_t mode, D3D_FEATURE_LEVEL fl)
		{
			switch (mode)
			{
			case 2: return D3D11_TEXTURE_ADDRESS_MIRROR;
			case 3: return D3D11_TEXTURE_ADDRESS_CLAMP;
			case 4: return (fl >= D3D_FEATURE_LEVEL_9_3) ? D3D11_TEXTURE_ADDRESS_BORDER : D3D11_TEXTURE_ADDRESS_CLAMP;
			case 5: return (fl >= D3D_FEATURE_LEVEL_10_0) ? D3D11_TEXTURE_ADDRESS_MIRROR_ONCE : D3D11_TEXTURE_ADDRESS_MIRROR;
			case 1:
			default: return D3D11_TEXTURE_ADDRESS_WRAP;
			}
		}

		DXGI_FORMAT MapDeclTypeToDXGI(uint8_t declType, D3D_FEATURE_LEVEL fl)
		{
			switch (declType)
			{
			case D3DDECLTYPE_FLOAT1:    return DXGI_FORMAT_R32_FLOAT;
			case D3DDECLTYPE_FLOAT2:    return DXGI_FORMAT_R32G32_FLOAT;
			case D3DDECLTYPE_FLOAT3:    return DXGI_FORMAT_R32G32B32_FLOAT;
			case D3DDECLTYPE_FLOAT4:    return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case D3DDECLTYPE_D3DCOLOR:  return DXGI_FORMAT_B8G8R8A8_UNORM;
			case D3DDECLTYPE_UBYTE4:    return (fl >= D3D_FEATURE_LEVEL_10_0) ? DXGI_FORMAT_R8G8B8A8_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;
			case D3DDECLTYPE_SHORT2:    return (fl >= D3D_FEATURE_LEVEL_10_0) ? DXGI_FORMAT_R16G16_SINT : DXGI_FORMAT_R16G16_SNORM;
			case D3DDECLTYPE_SHORT4:    return (fl >= D3D_FEATURE_LEVEL_10_0) ? DXGI_FORMAT_R16G16B16A16_SINT : DXGI_FORMAT_R16G16B16A16_SNORM;
			case D3DDECLTYPE_UBYTE4N:   return DXGI_FORMAT_R8G8B8A8_UNORM;
			case D3DDECLTYPE_SHORT2N:   return DXGI_FORMAT_R16G16_SNORM;
			case D3DDECLTYPE_SHORT4N:   return DXGI_FORMAT_R16G16B16A16_SNORM;
			case D3DDECLTYPE_USHORT2N:  return DXGI_FORMAT_R16G16_UNORM;
			case D3DDECLTYPE_USHORT4N:  return DXGI_FORMAT_R16G16B16A16_UNORM;
			case D3DDECLTYPE_FLOAT16_2: return DXGI_FORMAT_R16G16_FLOAT;
			case D3DDECLTYPE_FLOAT16_4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
			default:                    return DXGI_FORMAT_R32G32B32_FLOAT;
			}
		}

		const char* MapDeclUsageToSemantic(uint8_t usage, bool useSVPosition)
		{
			switch (usage)
			{
			case D3DDECLUSAGE_POSITION:
			case D3DDECLUSAGE_POSITIONT:    return useSVPosition ? "SV_Position" : "POSITION";
			case D3DDECLUSAGE_BLENDWEIGHT:  return "BLENDWEIGHT";
			case D3DDECLUSAGE_BLENDINDICES: return "BLENDINDICES";
			case D3DDECLUSAGE_NORMAL:       return "NORMAL";
			case D3DDECLUSAGE_PSIZE:        return "PSIZE";
			case D3DDECLUSAGE_TEXCOORD:     return "TEXCOORD";
			case D3DDECLUSAGE_TANGENT:      return "TANGENT";
			case D3DDECLUSAGE_BINORMAL:     return "BINORMAL";
			case D3DDECLUSAGE_COLOR:        return "COLOR";
			default:                        return "TEXCOORD";
			}
		}
	}

	// =========================================================================
	// D3D11 Resource Implementations
	// =========================================================================

	class D3D11Texture : public IRHITexture
	{
	public:
		D3D11Texture(
			RenderDeviceD3D11* owner,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			uint32_t width,
			uint32_t height,
			PixelFormat sourceFormat,
			DXGI_FORMAT dxgiFormat,
			uint32_t mipLevels,
			bool expandedToRGBA8,
			ID3D11Texture2D* tex,
			ID3D11ShaderResourceView* srv)
			: m_owner(owner)
			, m_device(device)
			, m_context(context)
			, m_width(width)
			, m_height(height)
			, m_mipLevels(mipLevels)
			, m_sourceFormat(sourceFormat)
			, m_dxgiFormat(dxgiFormat)
			, m_expandedToRGBA8(expandedToRGBA8)
			, m_tex(tex)
			, m_srv(srv)
		{
		}

		virtual ~D3D11Texture() override
		{
			if (m_owner)
				m_owner->NotifyTextureDestroyed(this);
		}

		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual uint32_t GetMipLevels() const override { return m_mipLevels; }
		virtual PixelFormat GetFormat() const override { return m_sourceFormat; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(m_srv.Get()); }

		ID3D11Texture2D* GetD3DTexture() const { return m_tex.Get(); }
		ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
		DXGI_FORMAT GetDXGIFormat() const { return m_dxgiFormat; }

		virtual bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t rowPitch) override
		{
			if (!m_context || !m_tex || !data || width == 0 || height == 0 || mipLevel >= m_mipLevels)
				return false;

			uint32_t mipW = (std::max)(1u, m_width >> mipLevel);
			uint32_t mipH = (std::max)(1u, m_height >> mipLevel);
			if (x < 0 || y < 0 || static_cast<uint32_t>(x) + width > mipW || static_cast<uint32_t>(y) + height > mipH)
				return false;

			UINT subresource = ::D3D11CalcSubresource(mipLevel, arraySlice, m_mipLevels);
			D3D11_BOX box = {};
			box.left = static_cast<UINT>(x);
			box.top = static_cast<UINT>(y);
			box.front = 0;
			box.right = static_cast<UINT>(x) + width;
			box.bottom = static_cast<UINT>(y) + height;
			box.back = 1;

			if (m_expandedToRGBA8)
			{
				std::vector<uint8_t> expanded;
				if (!RHISoftwareS3TC::ExpandToRGBA8(m_sourceFormat, data, width, height, rowPitch, expanded))
					return false;
				m_context->UpdateSubresource(m_tex.Get(), subresource, &box, expanded.data(), width * 4u, 0);
				return true;
			}

			if (RHISoftwareS3TC::IsS3TCFormat(m_sourceFormat))
			{
				uint32_t blockBytes = (m_sourceFormat == PixelFormat::DXT1) ? 8u : 16u;
				uint32_t blocksX = (width + 3u) / 4u;
				UINT pitch = rowPitch ? rowPitch : (blocksX * blockBytes);
				m_context->UpdateSubresource(m_tex.Get(), subresource, &box, data, pitch, 0);
				return true;
			}

			uint32_t bpp = 4u;
			if (m_dxgiFormat == DXGI_FORMAT_A8_UNORM) bpp = 1u;
			else if (m_dxgiFormat == DXGI_FORMAT_R16_FLOAT) bpp = 2u;
			else if (m_dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT || m_dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM || m_dxgiFormat == DXGI_FORMAT_R32G32_FLOAT) bpp = 8u;
			else if (m_dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) bpp = 16u;

			UINT pitch = rowPitch ? rowPitch : (width * bpp);
			m_context->UpdateSubresource(m_tex.Get(), subresource, &box, data, pitch, 0);
			return true;
		}

	private:
		RenderDeviceD3D11* m_owner;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_mipLevels;
		PixelFormat m_sourceFormat;
		DXGI_FORMAT m_dxgiFormat;
		bool m_expandedToRGBA8;
		ComPtr<ID3D11Texture2D> m_tex;
		ComPtr<ID3D11ShaderResourceView> m_srv;
	};

	class D3D11Buffer : public IRHIBuffer
	{
	public:
		D3D11Buffer(
			RenderDeviceD3D11* owner,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			uint32_t byteSize,
			bool isIndexBuffer,
			ERHIBufferUsage usage,
			const void* initialData)
			: m_owner(owner)
			, m_device(device)
			, m_context(context)
			, m_byteSize(byteSize)
			, m_isIndexBuffer(isIndexBuffer)
			, m_isDynamic(HasFlag(usage, ERHIBufferUsage::Dynamic) || HasFlag(usage, ERHIBufferUsage::Stream))
			, m_mappedHardware(false)
			, m_mappedShadow(false)
			, m_mapOffset(0)
			, m_mapSize(0)
			, m_mapMode(ERHIMapMode::Write)
		{
			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = (std::max)(byteSize, 16u);
			desc.BindFlags = isIndexBuffer ? D3D11_BIND_INDEX_BUFFER : D3D11_BIND_VERTEX_BUFFER;
			if (m_isDynamic)
			{
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			else
			{
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.CPUAccessFlags = 0;
			}

			D3D11_SUBRESOURCE_DATA initSub = {};
			std::vector<uint8_t> paddedInit;
			const D3D11_SUBRESOURCE_DATA* pInit = nullptr;
			if (initialData && byteSize > 0)
			{
				if (byteSize < desc.ByteWidth)
				{
					paddedInit.assign(desc.ByteWidth, 0);
					std::memcpy(paddedInit.data(), initialData, byteSize);
					initSub.pSysMem = paddedInit.data();
				}
				else
				{
					initSub.pSysMem = initialData;
				}
				pInit = &initSub;

				if (!HasFlag(usage, ERHIBufferUsage::WriteOnly))
				{
					m_cpuShadow.resize(byteSize);
					std::memcpy(m_cpuShadow.data(), initialData, byteSize);
				}
			}

			device->CreateBuffer(&desc, pInit, m_buffer.GetAddressOf());
		}

		virtual ~D3D11Buffer() override
		{
			if (m_owner)
				m_owner->NotifyBufferDestroyed(this);
		}

		virtual uint32_t GetSize() const override { return m_byteSize; }
		virtual bool IsIndexBuffer() const override { return m_isIndexBuffer; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(m_buffer.Get()); }
		ID3D11Buffer* GetD3DBuffer() const { return m_buffer.Get(); }

		virtual void* Map(uint32_t offset, uint32_t size, ERHIMapMode mode) override
		{
			if (!m_buffer || offset >= m_byteSize)
				return nullptr;

			uint32_t actualSize = (size == 0 || offset + size > m_byteSize) ? (m_byteSize - offset) : size;
			m_mapOffset = offset;
			m_mapSize = actualSize;
			m_mapMode = mode;

			// Direct hardware map fast path for WriteDiscard (offset 0) & WriteNoOverwrite on dynamic buffers
			if (m_isDynamic && m_context &&
				((mode == ERHIMapMode::WriteDiscard && offset == 0) || mode == ERHIMapMode::WriteNoOverwrite))
			{
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				D3D11_MAP d3dMap = (mode == ERHIMapMode::WriteDiscard) ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;
				if (SUCCEEDED(m_context->Map(m_buffer.Get(), 0, d3dMap, 0, &mapped)) && mapped.pData)
				{
					m_mappedHardware = true;
					return static_cast<uint8_t*>(mapped.pData) + offset;
				}
			}

			// CPU shadow fallback for partial Write / Read / ReadWrite
			if (m_cpuShadow.size() < m_byteSize)
				m_cpuShadow.resize(m_byteSize, 0);
			m_mappedShadow = true;
			return m_cpuShadow.data() + offset;
		}

		virtual void Unmap() override
		{
			if (m_mappedHardware)
			{
				m_mappedHardware = false;
				if (m_context && m_buffer)
					m_context->Unmap(m_buffer.Get(), 0);
				return;
			}

			if (!m_mappedShadow)
				return;
			m_mappedShadow = false;

			if (m_mapMode == ERHIMapMode::Read || !m_context || !m_buffer || m_mapSize == 0)
				return;

			if (m_isDynamic)
			{
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				if (SUCCEEDED(m_context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && mapped.pData)
				{
					std::memcpy(mapped.pData, m_cpuShadow.data(), m_byteSize);
					m_context->Unmap(m_buffer.Get(), 0);
				}
			}
			else
			{
				D3D11_BOX box = { m_mapOffset, 0, 0, m_mapOffset + m_mapSize, 1, 1 };
				if (m_byteSize < 16u && m_mapOffset == 0 && m_mapSize == m_byteSize)
				{
					uint8_t padded[16] = {};
					std::memcpy(padded, m_cpuShadow.data(), m_byteSize);
					m_context->UpdateSubresource(m_buffer.Get(), 0, nullptr, padded, 0, 0);
				}
				else
				{
					m_context->UpdateSubresource(m_buffer.Get(), 0, &box, m_cpuShadow.data() + m_mapOffset, 0, 0);
				}
			}
		}

		virtual bool UpdateData(uint32_t offset, uint32_t size, const void* data) override
		{
			if (!data || size == 0 || offset + size > m_byteSize || !m_context || !m_buffer)
				return false;

			if (m_isDynamic && offset == 0 && size == m_byteSize && m_cpuShadow.empty())
			{
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				if (SUCCEEDED(m_context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && mapped.pData)
				{
					std::memcpy(mapped.pData, data, size);
					m_context->Unmap(m_buffer.Get(), 0);
					return true;
				}
			}

			void* dst = Map(offset, size, (offset == 0 && size == m_byteSize) ? ERHIMapMode::WriteDiscard : ERHIMapMode::Write);
			if (!dst)
				return false;
			std::memcpy(dst, data, size);
			Unmap();
			return true;
		}

	private:
		RenderDeviceD3D11* m_owner;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;
		ComPtr<ID3D11Buffer> m_buffer;
		std::vector<uint8_t> m_cpuShadow;
		uint32_t m_byteSize;
		bool m_isIndexBuffer;
		bool m_isDynamic;
		bool m_mappedHardware;
		bool m_mappedShadow;
		uint32_t m_mapOffset;
		uint32_t m_mapSize;
		ERHIMapMode m_mapMode;
	};

	class D3D11RenderTarget : public IRHIRenderTarget
	{
	public:
		D3D11RenderTarget(
			RenderDeviceD3D11* owner,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			D3D_FEATURE_LEVEL fl,
			uint32_t width,
			uint32_t height,
			PixelFormat colorFormat,
			PixelFormat depthFormat,
			uint32_t colorTargetCount,
			uint32_t msaaSamples)
			: m_owner(owner)
			, m_width(width)
			, m_height(height)
			, m_colorFormat(colorFormat)
			, m_colorCount((std::min)((std::max)(colorTargetCount, 1u), 4u))
			, m_valid(false)
		{
			uint32_t bpp = 4;
			bool compressed = false;
			DXGI_FORMAT dxgiColorFmt = MapColorPixelFormatToDXGI(colorFormat, fl, bpp, compressed);
			if (dxgiColorFmt == DXGI_FORMAT_UNKNOWN || compressed)
				dxgiColorFmt = DXGI_FORMAT_R8G8B8A8_UNORM;

			UINT sampleCount = 1;
			UINT sampleQuality = 0;
			if (msaaSamples > 1)
			{
				UINT levels = 0;
				if (SUCCEEDED(device->CheckMultisampleQualityLevels(dxgiColorFmt, msaaSamples, &levels)) && levels > 0)
				{
					sampleCount = msaaSamples;
					sampleQuality = levels - 1;
				}
			}
			m_sampleCount = sampleCount;

			for (uint32_t i = 0; i < m_colorCount; ++i)
			{
				D3D11_TEXTURE2D_DESC td = {};
				td.Width = width;
				td.Height = height;
				td.MipLevels = 1;
				td.ArraySize = 1;
				td.Format = dxgiColorFmt;
				td.SampleDesc.Count = sampleCount;
				td.SampleDesc.Quality = sampleQuality;
				td.Usage = D3D11_USAGE_DEFAULT;
				td.BindFlags = D3D11_BIND_RENDER_TARGET | (sampleCount == 1 ? D3D11_BIND_SHADER_RESOURCE : 0);

				ComPtr<ID3D11Texture2D> colorTex;
				if (FAILED(device->CreateTexture2D(&td, nullptr, colorTex.GetAddressOf())))
					return;

				if (FAILED(device->CreateRenderTargetView(colorTex.Get(), nullptr, m_rtvs[i].GetAddressOf())))
					return;

				ComPtr<ID3D11ShaderResourceView> srv;
				if (sampleCount == 1)
				{
					device->CreateShaderResourceView(colorTex.Get(), nullptr, srv.GetAddressOf());
				}
				m_colorTextures[i] = new D3D11Texture(owner, device, context, width, height, colorFormat, dxgiColorFmt, 1, false, colorTex.Get(), srv.Get());
			}

			if (depthFormat != PixelFormat::Unknown && IsDepthPixelFormat(depthFormat))
			{
				const bool canSampleDepth = (fl >= D3D_FEATURE_LEVEL_10_0) && (sampleCount == 1);
				DXGI_FORMAT texFmt = canSampleDepth ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
				DXGI_FORMAT dsvFmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
				DXGI_FORMAT srvFmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

				if (depthFormat == PixelFormat::D16 || depthFormat == PixelFormat::D16_LOCKABLE)
				{
					texFmt = canSampleDepth ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_D16_UNORM;
					dsvFmt = DXGI_FORMAT_D16_UNORM;
					srvFmt = DXGI_FORMAT_R16_UNORM;
				}
				else if (depthFormat == PixelFormat::D32 || depthFormat == PixelFormat::D32F_LOCKABLE)
				{
					texFmt = canSampleDepth ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D32_FLOAT;
					dsvFmt = DXGI_FORMAT_D32_FLOAT;
					srvFmt = DXGI_FORMAT_R32_FLOAT;
				}

				D3D11_TEXTURE2D_DESC dd = {};
				dd.Width = width;
				dd.Height = height;
				dd.MipLevels = 1;
				dd.ArraySize = 1;
				dd.Format = texFmt;
				dd.SampleDesc.Count = sampleCount;
				dd.SampleDesc.Quality = sampleQuality;
				dd.Usage = D3D11_USAGE_DEFAULT;
				dd.BindFlags = D3D11_BIND_DEPTH_STENCIL | (canSampleDepth ? D3D11_BIND_SHADER_RESOURCE : 0);

				ComPtr<ID3D11Texture2D> depthTex;
				if (SUCCEEDED(device->CreateTexture2D(&dd, nullptr, depthTex.GetAddressOf())))
				{
					D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
					dsvDesc.Format = dsvFmt;
					dsvDesc.ViewDimension = (sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
					device->CreateDepthStencilView(depthTex.Get(), &dsvDesc, m_dsv.GetAddressOf());

					ComPtr<ID3D11ShaderResourceView> depthSrv;
					if (canSampleDepth)
					{
						D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
						srvDesc.Format = srvFmt;
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MipLevels = 1;
						device->CreateShaderResourceView(depthTex.Get(), &srvDesc, depthSrv.GetAddressOf());
					}
					m_depthTexture = new D3D11Texture(owner, device, context, width, height, depthFormat, dsvFmt, 1, false, depthTex.Get(), depthSrv.Get());
				}
			}

			m_valid = true;
		}

		virtual ~D3D11RenderTarget() override
		{
			if (m_owner)
			{
				m_owner->NotifyRenderTargetDestroyed(this);
			}
			for (uint32_t i = 0; i < 4; ++i)
			{
				if (m_colorTextures[i])
				{
					m_colorTextures[i]->ReleaseRHIResource();
					m_colorTextures[i] = nullptr;
				}
			}
			if (m_depthTexture)
			{
				m_depthTexture->ReleaseRHIResource();
				m_depthTexture = nullptr;
			}
		}

		bool IsValid() const { return m_valid; }
		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual PixelFormat GetFormat() const override { return m_colorFormat; }
		virtual IRHITexture* GetColorTexture(uint32_t index = 0) const override { return (index < m_colorCount) ? m_colorTextures[index] : nullptr; }
		virtual IRHITexture* GetDepthTexture() const override { return m_depthTexture; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(m_rtvs[0].Get()); }

		uint32_t GetColorCount() const { return m_colorCount; }
		uint32_t GetSampleCount() const { return m_sampleCount; }
		ID3D11RenderTargetView* GetRTV(uint32_t index = 0) const { return (index < m_colorCount) ? m_rtvs[index].Get() : nullptr; }
		ID3D11DepthStencilView* GetDSV() const { return m_dsv.Get(); }
		D3D11Texture* GetD3DColorTex(uint32_t index = 0) const { return (index < m_colorCount) ? m_colorTextures[index] : nullptr; }
		D3D11Texture* GetD3DDepthTex() const { return m_depthTexture; }

	private:
		RenderDeviceD3D11* m_owner = nullptr;
		uint32_t m_width;
		uint32_t m_height;
		PixelFormat m_colorFormat;
		uint32_t m_colorCount;
		uint32_t m_sampleCount = 1;
		bool m_valid;
		D3D11Texture* m_colorTextures[4] = { nullptr, nullptr, nullptr, nullptr };
		D3D11Texture* m_depthTexture = nullptr;
		ComPtr<ID3D11RenderTargetView> m_rtvs[4];
		ComPtr<ID3D11DepthStencilView> m_dsv;
	};

	class D3D11OcclusionQuery : public IRHIOcclusionQuery
	{
	public:
		D3D11OcclusionQuery(ID3D11Device* device, ID3D11DeviceContext* context)
			: m_context(context)
			, m_active(false)
		{
			D3D11_QUERY_DESC qd = {};
			qd.Query = D3D11_QUERY_OCCLUSION;
			device->CreateQuery(&qd, m_query.GetAddressOf());
		}

		virtual ~D3D11OcclusionQuery() override = default;

		virtual bool Begin() override
		{
			if (!m_query || !m_context)
				return false;
			m_context->Begin(m_query.Get());
			m_active = true;
			return true;
		}

		virtual void End() override
		{
			if (m_query && m_context && m_active)
			{
				m_context->End(m_query.Get());
				m_active = false;
			}
		}

		virtual bool GetResult(uint64_t& outSamplesPassed, bool flush) override
		{
			outSamplesPassed = 0;
			if (!m_query || !m_context)
				return false;

			UINT64 samples = 0;
			UINT flags = flush ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH;
			for (;;)
			{
				HRESULT hr = m_context->GetData(m_query.Get(), &samples, sizeof(samples), flags);
				if (hr == S_OK)
				{
					outSamplesPassed = static_cast<uint64_t>(samples);
					return true;
				}
				if (!flush || FAILED(hr))
					return false;
				::SwitchToThread();
			}
		}

	private:
		ID3D11DeviceContext* m_context;
		ComPtr<ID3D11Query> m_query;
		bool m_active;
	};

	class D3D11ShaderProgram : public IRHIShaderProgram
	{
	public:
		struct InputSigParam
		{
			std::string semanticName;
			uint32_t semanticIndex = 0;
			D3D_REGISTER_COMPONENT_TYPE componentType = D3D_REGISTER_COMPONENT_FLOAT32;
		};

		D3D11ShaderProgram(RenderDeviceD3D11* owner, ID3D11Device* device, const RHIShaderProgramDesc& desc)
			: m_owner(owner)
			, m_valid(false)
			, m_vsHash(0)
			, m_usesSVPosition(false)
			, m_psOutputMask(0x0Fu)
			, m_debugName(desc.debugName ? desc.debugName : "")
			, m_psEntry(desc.psEntryPoint ? desc.psEntryPoint : "")
		{
			for (uint32_t i = 0; i < RHIStateTracker::MAX_SAMPLER_STAGES; ++i)
			{
				m_vsTexSlotToStage[i] = static_cast<uint8_t>(i);
				m_psTexSlotToStage[i] = static_cast<uint8_t>(i);
			}

			if (!device || !desc.vsBytecode || desc.vsBytecodeSize == 0)
				return;

			if (FAILED(device->CreateVertexShader(desc.vsBytecode, desc.vsBytecodeSize, nullptr, m_vs.GetAddressOf())))
				return;

			if (desc.psBytecode && desc.psBytecodeSize > 0)
			{
				if (FAILED(device->CreatePixelShader(desc.psBytecode, desc.psBytecodeSize, nullptr, m_ps.GetAddressOf())))
					return;
			}

			m_vsBytecode.resize(desc.vsBytecodeSize);
			std::memcpy(m_vsBytecode.data(), desc.vsBytecode, desc.vsBytecodeSize);
			m_vsHash = ComputeByteHash64(desc.vsBytecode, desc.vsBytecodeSize);

			const auto& d3dc = GetD3D11Compiler();
			ComPtr<ID3D11ShaderReflection> refl;
			if (d3dc.pfnReflect && SUCCEEDED(d3dc.pfnReflect(desc.vsBytecode, desc.vsBytecodeSize, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(refl.GetAddressOf()))) && refl)
			{
				D3D11_SHADER_DESC sd = {};
				if (SUCCEEDED(refl->GetDesc(&sd)))
				{
					for (UINT i = 0; i < sd.InputParameters; ++i)
					{
						D3D11_SIGNATURE_PARAMETER_DESC pd = {};
						if (SUCCEEDED(refl->GetInputParameterDesc(i, &pd)) && pd.SemanticName)
						{
							if (pd.SystemValueType != D3D_NAME_UNDEFINED &&
								_stricmp(pd.SemanticName, "SV_Position") != 0 &&
								_stricmp(pd.SemanticName, "POSITION") != 0)
							{
								continue; // Skip SV_InstanceID / SV_VertexID
							}
							if (_stricmp(pd.SemanticName, "SV_Position") == 0 && pd.SemanticIndex == 0)
							{
								m_usesSVPosition = true;
							}
							InputSigParam sig;
							sig.semanticName = pd.SemanticName;
							sig.semanticIndex = pd.SemanticIndex;
							sig.componentType = pd.ComponentType;
							m_inputParams.push_back(std::move(sig));
						}
					}

					ReflectTextureSlotMapping(refl.Get(), sd.BoundResources, m_vsTexSlotToStage);
				}
			}

			if (desc.psBytecode && desc.psBytecodeSize > 0)
			{
				ComPtr<ID3D11ShaderReflection> psRefl;
				if (d3dc.pfnReflect && SUCCEEDED(d3dc.pfnReflect(desc.psBytecode, desc.psBytecodeSize, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(psRefl.GetAddressOf()))) && psRefl)
				{
					D3D11_SHADER_DESC psd = {};
					if (SUCCEEDED(psRefl->GetDesc(&psd)))
					{
						uint8_t outMask = 0;
						for (UINT i = 0; i < psd.OutputParameters; ++i)
						{
							D3D11_SIGNATURE_PARAMETER_DESC opd = {};
							if (SUCCEEDED(psRefl->GetOutputParameterDesc(i, &opd)) && opd.SemanticName)
							{
								if ((opd.SystemValueType == D3D_NAME_TARGET ||
									 _stricmp(opd.SemanticName, "SV_Target") == 0 ||
									 _stricmp(opd.SemanticName, "COLOR") == 0) &&
									opd.SemanticIndex < 8)
								{
									outMask |= static_cast<uint8_t>(1u << opd.SemanticIndex);
								}
							}
						}
						m_psOutputMask = outMask;
						ReflectTextureSlotMapping(psRefl.Get(), psd.BoundResources, m_psTexSlotToStage);
					}
				}
			}
			else
			{
				m_psOutputMask = 0;
			}

			m_valid = true;
		}

		virtual ~D3D11ShaderProgram() override
		{
			if (m_owner)
				m_owner->NotifyProgramDestroyed(this);
		}

		virtual bool IsValid() const override { return m_valid; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(m_vs.Get()); }

		ID3D11VertexShader* GetVS() const { return m_vs.Get(); }
		ID3D11PixelShader* GetPS() const { return m_ps.Get(); }
		const std::vector<uint8_t>& GetVSBytecode() const { return m_vsBytecode; }
		uint64_t GetVSHash() const { return m_vsHash; }
		bool UsesSVPosition() const { return m_usesSVPosition; }
		uint8_t GetPSOutputMask() const { return m_psOutputMask; }
		const std::vector<InputSigParam>& GetInputSignature() const { return m_inputParams; }
		uint8_t MapVSTexSlot(uint32_t tSlot) const { return (tSlot < RHIStateTracker::MAX_SAMPLER_STAGES) ? m_vsTexSlotToStage[tSlot] : static_cast<uint8_t>(tSlot); }
		uint8_t MapPSTexSlot(uint32_t tSlot) const { return (tSlot < RHIStateTracker::MAX_SAMPLER_STAGES) ? m_psTexSlotToStage[tSlot] : static_cast<uint8_t>(tSlot); }
		const std::string& GetDebugName() const { return m_debugName; }
		const std::string& GetPsEntry() const { return m_psEntry; }

	private:
		// Under D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, legacy `sampler2D foo : register(sN)` pins the
		// SamplerState (`D3D_SIT_SAMPLER`) to `sN` while d3dcompiler auto-assigns the paired Texture2D
		// (`D3D_SIT_TEXTURE`, with matching Name) sequentially to `t0, t1, ...` in lexical order of first use.
		static void ReflectTextureSlotMapping(ID3D11ShaderReflection* refl, UINT boundResources, uint8_t* outSlotToStage)
		{
			std::unordered_map<std::string, uint8_t> samplerNameToStage;
			for (UINT i = 0; i < boundResources; ++i)
			{
				D3D11_SHADER_INPUT_BIND_DESC ibd = {};
				if (SUCCEEDED(refl->GetResourceBindingDesc(i, &ibd)) && ibd.Name)
				{
					if (ibd.Type == D3D_SIT_SAMPLER && ibd.BindPoint < RHIStateTracker::MAX_SAMPLER_STAGES)
					{
						samplerNameToStage[ibd.Name] = static_cast<uint8_t>(ibd.BindPoint);
					}
				}
			}

			for (UINT i = 0; i < boundResources; ++i)
			{
				D3D11_SHADER_INPUT_BIND_DESC ibd = {};
				if (SUCCEEDED(refl->GetResourceBindingDesc(i, &ibd)) && ibd.Name)
				{
					if (ibd.Type == D3D_SIT_TEXTURE && ibd.BindPoint < RHIStateTracker::MAX_SAMPLER_STAGES)
					{
						auto it = samplerNameToStage.find(ibd.Name);
						if (it != samplerNameToStage.end())
							outSlotToStage[ibd.BindPoint] = it->second;
						else
							outSlotToStage[ibd.BindPoint] = static_cast<uint8_t>(ibd.BindPoint);
					}
				}
			}
		}

		RenderDeviceD3D11* m_owner;
		bool m_valid;
		uint64_t m_vsHash;
		bool m_usesSVPosition;
		uint8_t m_psOutputMask;
		ComPtr<ID3D11VertexShader> m_vs;
		ComPtr<ID3D11PixelShader> m_ps;
		std::vector<uint8_t> m_vsBytecode;
		std::vector<InputSigParam> m_inputParams;
		std::string m_debugName;
		std::string m_psEntry;
		uint8_t m_vsTexSlotToStage[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		uint8_t m_psTexSlotToStage[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
	};

	// =========================================================================
	// RenderDeviceD3D11 Implementation
	// =========================================================================

	RenderDeviceD3D11::RenderDeviceD3D11(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		IDXGISwapChain* swapChain,
		D3D_FEATURE_LEVEL featureLevel,
		bool isWarp,
		const std::string& adapterName)
		: m_device(device)
		, m_context(context)
		, m_swapChain(swapChain)
		, m_featureLevel(featureLevel)
		, m_isWarp(isWarp)
		, m_adapterName(adapterName)
	{
		InitCaps();

		// Create 16-byte default attribute fallback vertex buffer [0, 0, 0, 1]
		const float defaultAttr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		D3D11_BUFFER_DESC bd = {};
		bd.ByteWidth = sizeof(defaultAttr);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = defaultAttr;
		m_device->CreateBuffer(&bd, &initData, m_defaultAttrVB.GetAddressOf());
	}

	RenderDeviceD3D11::~RenderDeviceD3D11()
	{
		m_isShuttingDown = true;
		for (auto& kv : m_fvfDecls)
		{
			if (kv.second)
				kv.second->Release();
		}
		m_fvfDecls.clear();

		if (m_context)
		{
			m_context->ClearState();
		}
	}

	void RenderDeviceD3D11::NotifyTextureDestroyed(D3D11Texture* tex)
	{
		if (m_isShuttingDown || !tex)
			return;

		ID3D11ShaderResourceView* srv = tex->GetSRV();
		ID3D11Texture2D* d3dTex = tex->GetD3DTexture();
		if (d3dTex)
		{
			if (m_activeColorTex0 == d3dTex)
				m_activeColorTex0 = nullptr;
			if (m_activeDepthTex == d3dTex)
				m_activeDepthTex = nullptr;
		}
		if (!srv)
			return;

		for (uint32_t i = 0; i < RHIStateTracker::MAX_SAMPLER_STAGES; ++i)
		{
			if (m_logicalSRVs[i] == srv)
			{
				m_logicalSRVs[i] = nullptr;
				m_srvBindingsDirty = true;
			}
			if (m_boundPSSRVs[i] == srv)
			{
				m_boundPSSRVs[i] = nullptr;
				if (m_context)
				{
					ID3D11ShaderResourceView* nullSrv = nullptr;
					m_context->PSSetShaderResources(i, 1, &nullSrv);
				}
				m_srvBindingsDirty = true;
			}
			if (m_boundVSSRVs[i] == srv)
			{
				m_boundVSSRVs[i] = nullptr;
				if (m_context && m_featureLevel >= D3D_FEATURE_LEVEL_10_0)
				{
					ID3D11ShaderResourceView* nullSrv = nullptr;
					m_context->VSSetShaderResources(i, 1, &nullSrv);
				}
				m_srvBindingsDirty = true;
			}
		}
	}

	void RenderDeviceD3D11::NotifyBufferDestroyed(D3D11Buffer* buf)
	{
		if (m_isShuttingDown || !buf)
			return;
		for (uint32_t s = 0; s < 4; ++s)
		{
			if (m_streams[s].buffer == buf)
				m_streams[s].buffer = nullptr;
		}
		if (m_currentIndexBuffer == buf)
			m_currentIndexBuffer = nullptr;
	}

	void RenderDeviceD3D11::NotifyProgramDestroyed(D3D11ShaderProgram* prog)
	{
		if (m_isShuttingDown || !prog)
			return;
		if (m_currentProgram == prog)
		{
			m_currentProgram = nullptr;
			m_srvBindingsDirty = true;
		}
	}

	void RenderDeviceD3D11::NotifyRenderTargetDestroyed(D3D11RenderTarget* rt)
	{
		if (m_isShuttingDown || !rt)
			return;

		bool changed = false;
		IRHIRenderTarget* newTargets[4] = {
			m_currentRTBinding.targets[0],
			m_currentRTBinding.targets[1],
			m_currentRTBinding.targets[2],
			m_currentRTBinding.targets[3]
		};
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (newTargets[i] == rt)
			{
				newTargets[i] = nullptr;
				changed = true;
			}
		}
		IRHIRenderTarget* dsOverride = (m_currentRTBinding.depthStencilOverride == rt)
			? nullptr
			: m_currentRTBinding.depthStencilOverride;
		if (dsOverride != m_currentRTBinding.depthStencilOverride)
			changed = true;
		if (changed)
		{
			uint32_t newCount = 0;
			for (uint32_t i = 0; i < 4; ++i)
			{
				if (newTargets[i] != nullptr)
					newCount = i + 1;
			}
			SetRenderTargets(newCount, newCount > 0 ? newTargets : nullptr, dsOverride);
		}
	}

	void RenderDeviceD3D11::InitCaps()
	{
		m_caps.backend = ERHIBackend::D3D11;
		m_caps.backendName = m_isWarp ? "D3D11 (WARP)" : "D3D11";
		m_caps.adapterName = m_adapterName.c_str();
		m_caps.featureLevel = static_cast<uint32_t>(m_featureLevel);
		m_caps.maxSimultaneousRTs = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) ? 8u : 4u;
		m_caps.maxTextureStages = 16u;
		m_caps.maxTextureSize = (m_featureLevel >= D3D_FEATURE_LEVEL_11_0) ? 16384u
			: ((m_featureLevel >= D3D_FEATURE_LEVEL_10_0) ? 8192u : 4096u);
		m_caps.maxAnisotropy = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) ? 16u : 4u;

		UINT r32fSupport = 0;
		if (m_device && SUCCEEDED(m_device->CheckFormatSupport(DXGI_FORMAT_R32_FLOAT, &r32fSupport)))
		{
			m_caps.supportsFloatRenderTargets = (r32fSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0;
		}
		else
		{
			m_caps.supportsFloatRenderTargets = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
		}

		m_caps.supportsDepthTextures = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
		m_caps.supportsInstancing = true;
		m_caps.supportsOcclusionQuery = true;
		m_caps.supportsTwoSidedStencil = true;
		m_caps.supportsUserClipPlanes = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0);
		m_caps.supportsBCCompression = true;
		m_caps.supportsETC2Compression = false;
		m_caps.supportsASTCCompression = false;
		m_caps.supportsAsyncReadback = true;
		m_caps.supportsRuntimeShaderCompile = true;
		m_caps.topLeftScreenOrigin = true;
		m_caps.clipSpaceZeroToOne = true;
	}

	bool RenderDeviceD3D11::ResizeSwapChain(uint32_t width, uint32_t height, bool isWindowed)
	{
		if (!m_device || !m_context || !m_swapChain)
			return false;

		if (width == 0) width = 1;
		if (height == 0) height = 1;

		m_context->OMSetRenderTargets(0, nullptr, nullptr);
		m_defaultRTV.Reset();
		m_backBufferTex.Reset();
		m_defaultDSV.Reset();
		m_defaultDepthTex.Reset();

		m_swapChain->SetFullscreenState(isWindowed ? FALSE : TRUE, nullptr);
		HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(hr))
		{
			OUTPUT_LOG("ERROR: IDXGISwapChain::ResizeBuffers(%ux%u) failed (hr=0x%08X)\n", width, height, static_cast<unsigned int>(hr));
			return false;
		}

		hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(m_backBufferTex.GetAddressOf()));
		if (FAILED(hr) || !m_backBufferTex)
			return false;

		hr = m_device->CreateRenderTargetView(m_backBufferTex.Get(), nullptr, m_defaultRTV.GetAddressOf());
		if (FAILED(hr) || !m_defaultRTV)
			return false;

		D3D11_TEXTURE2D_DESC bbDesc = {};
		m_backBufferTex->GetDesc(&bbDesc);
		m_backBufferWidth = bbDesc.Width;
		m_backBufferHeight = bbDesc.Height;
		m_renderTargetWidth = bbDesc.Width;
		m_renderTargetHeight = bbDesc.Height;

		D3D11_TEXTURE2D_DESC dsDesc = {};
		dsDesc.Width = bbDesc.Width;
		dsDesc.Height = bbDesc.Height;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsDesc.SampleDesc = bbDesc.SampleDesc;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = m_device->CreateTexture2D(&dsDesc, nullptr, m_defaultDepthTex.GetAddressOf());
		if (FAILED(hr) || !m_defaultDepthTex)
			return false;

		hr = m_device->CreateDepthStencilView(m_defaultDepthTex.Get(), nullptr, m_defaultDSV.GetAddressOf());
		if (FAILED(hr) || !m_defaultDSV)
			return false;

		m_activeRTVs[0] = m_defaultRTV.Get();
		m_activeRTVs[1] = nullptr;
		m_activeRTVs[2] = nullptr;
		m_activeRTVs[3] = nullptr;
		m_activeRTVCount = 1;
		m_activeDSV = m_defaultDSV.Get();
		m_activeColorTex0 = m_backBufferTex.Get();
		m_activeDepthTex = m_defaultDepthTex.Get();

		m_context->OMSetRenderTargets(1, m_activeRTVs, m_activeDSV);
		m_srvBindingsDirty = true;

		ParaViewport vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = m_backBufferWidth;
		vp.Height = m_backBufferHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		SetViewport(vp);

		return true;
	}

	bool RenderDeviceD3D11::CheckDeviceError(const char* /*filename*/, const char* /*func*/, int /*nLine*/)
	{
		if (!m_device)
			return false;
		HRESULT reason = m_device->GetDeviceRemovedReason();
		if (FAILED(reason))
		{
			OUTPUT_LOG("ERROR: D3D11 device removed/reset (reason=0x%08X)\n", static_cast<unsigned int>(reason));
			return false;
		}
		return true;
	}

	IRHITexture* RenderDeviceD3D11::CreateTexture(
		uint32_t width,
		uint32_t height,
		PixelFormat format,
		uint32_t mipLevels,
		ERHITextureUsage usage,
		const RHIMipData* initialMips,
		uint32_t initialMipCount)
	{
		if (!m_device || width == 0 || height == 0)
			return nullptr;

		uint32_t numMips = (mipLevels > 0) ? mipLevels : 1u;

		if (IsDepthPixelFormat(format))
		{
			const bool canSample = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) && HasFlag(usage, ERHITextureUsage::ShaderResource);
			DXGI_FORMAT texFmt = canSample ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
			DXGI_FORMAT srvFmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			if (format == PixelFormat::D16 || format == PixelFormat::D16_LOCKABLE)
			{
				texFmt = canSample ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_D16_UNORM;
				srvFmt = DXGI_FORMAT_R16_UNORM;
			}
			else if (format == PixelFormat::D32 || format == PixelFormat::D32F_LOCKABLE)
			{
				texFmt = canSample ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D32_FLOAT;
				srvFmt = DXGI_FORMAT_R32_FLOAT;
			}

			D3D11_TEXTURE2D_DESC td = {};
			td.Width = width;
			td.Height = height;
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.Format = texFmt;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DEFAULT;
			td.BindFlags = D3D11_BIND_DEPTH_STENCIL | (canSample ? D3D11_BIND_SHADER_RESOURCE : 0);

			ComPtr<ID3D11Texture2D> tex;
			if (FAILED(m_device->CreateTexture2D(&td, nullptr, tex.GetAddressOf())))
				return nullptr;

			ComPtr<ID3D11ShaderResourceView> srv;
			if (canSample)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
				sd.Format = srvFmt;
				sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				sd.Texture2D.MipLevels = 1;
				m_device->CreateShaderResourceView(tex.Get(), &sd, srv.GetAddressOf());
			}
			return new D3D11Texture(this, m_device.Get(), m_context.Get(), width, height, format, texFmt, 1, false, tex.Get(), srv.Get());
		}

		bool expandedToRGBA8 = RHISoftwareS3TC::NeedsRGBA8Expansion(format, true);
		uint32_t bytesPerBlock = 4;
		bool isBlockCompressed = false;
		DXGI_FORMAT dxgiFormat = expandedToRGBA8
			? DXGI_FORMAT_R8G8B8A8_UNORM
			: MapColorPixelFormatToDXGI(format, m_featureLevel, bytesPerBlock, isBlockCompressed);

		// Software decode fallback for unsupported compressed formats (or non-multiple-of-4 BC textures)
		bool softwareDecodeS3TC = false;
		if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
		{
			if (RHISoftwareS3TC::IsS3TCFormat(format))
			{
				softwareDecodeS3TC = true;
				dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				isBlockCompressed = false;
				bytesPerBlock = 4;
			}
			else
			{
				dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			}
		}
		else if (isBlockCompressed && ((width % 4u) != 0 || (height % 4u) != 0))
		{
			softwareDecodeS3TC = true;
			dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			isBlockCompressed = false;
			bytesPerBlock = 4;
		}

		// Clamp mip count for block-compressed textures on strict drivers if sub-4x4 mips aren't provided
		if (initialMips && initialMipCount > 0 && numMips > initialMipCount)
		{
			numMips = initialMipCount;
		}

		std::vector<D3D11_SUBRESOURCE_DATA> subData;
		std::vector<std::vector<uint8_t>> tempBuffers;
		const D3D11_SUBRESOURCE_DATA* pSubData = nullptr;

		if (initialMips && initialMipCount >= numMips)
		{
			subData.resize(numMips);
			tempBuffers.resize(numMips);
			uint32_t mipW = width;
			uint32_t mipH = height;
			bool allValid = true;

			for (uint32_t m = 0; m < numMips; ++m)
			{
				const RHIMipData& srcMip = initialMips[m];
				if (!srcMip.data)
				{
					allValid = false;
					break;
				}

				if (softwareDecodeS3TC)
				{
					size_t compBytes = srcMip.dataSize ? srcMip.dataSize : RHISoftwareS3TC::ComputeCompressedMipSize(format, mipW, mipH);
					if (!RHISoftwareS3TC::DecodeToRGBA8(format, srcMip.data, compBytes, mipW, mipH, tempBuffers[m]))
					{
						allValid = false;
						break;
					}
					subData[m].pSysMem = tempBuffers[m].data();
					subData[m].SysMemPitch = mipW * 4u;
					subData[m].SysMemSlicePitch = 0;
				}
				else if (expandedToRGBA8)
				{
					if (!RHISoftwareS3TC::ExpandToRGBA8(format, srcMip.data, mipW, mipH, srcMip.rowPitch, tempBuffers[m]))
					{
						allValid = false;
						break;
					}
					subData[m].pSysMem = tempBuffers[m].data();
					subData[m].SysMemPitch = mipW * 4u;
					subData[m].SysMemSlicePitch = 0;
				}
				else if (isBlockCompressed)
				{
					uint32_t blocksX = (std::max)(1u, (mipW + 3u) / 4u);
					subData[m].pSysMem = srcMip.data;
					subData[m].SysMemPitch = srcMip.rowPitch ? srcMip.rowPitch : (blocksX * bytesPerBlock);
					subData[m].SysMemSlicePitch = 0;
				}
				else
				{
					subData[m].pSysMem = srcMip.data;
					subData[m].SysMemPitch = srcMip.rowPitch ? srcMip.rowPitch : (mipW * bytesPerBlock);
					subData[m].SysMemSlicePitch = 0;
				}

				mipW = (std::max)(1u, mipW >> 1);
				mipH = (std::max)(1u, mipH >> 1);
			}

			if (allValid)
			{
				pSubData = subData.data();
			}
		}

		D3D11_TEXTURE2D_DESC td = {};
		td.Width = width;
		td.Height = height;
		td.MipLevels = numMips;
		td.ArraySize = 1;
		td.Format = dxgiFormat;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE | (HasFlag(usage, ERHITextureUsage::RenderTarget) ? D3D11_BIND_RENDER_TARGET : 0);

		ComPtr<ID3D11Texture2D> tex;
		HRESULT hr = m_device->CreateTexture2D(&td, pSubData, tex.GetAddressOf());
		if (FAILED(hr) && isBlockCompressed && initialMips && initialMipCount > 0 && initialMips[0].data)
		{
			// Fallback to software RGBA8 decode (e.g. NPOT BC texture on FL 9_3)
			std::vector<uint8_t> decoded0;
			size_t compBytes = initialMips[0].dataSize ? initialMips[0].dataSize : RHISoftwareS3TC::ComputeCompressedMipSize(format, width, height);
			if (RHISoftwareS3TC::DecodeToRGBA8(format, initialMips[0].data, compBytes, width, height, decoded0))
			{
				td.MipLevels = 1;
				td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				numMips = 1;
				D3D11_SUBRESOURCE_DATA sd0 = { decoded0.data(), width * 4u, 0 };
				hr = m_device->CreateTexture2D(&td, &sd0, tex.GetAddressOf());
			}
		}

		if (FAILED(hr) || !tex)
			return nullptr;

		ComPtr<ID3D11ShaderResourceView> srv;
		hr = m_device->CreateShaderResourceView(tex.Get(), nullptr, srv.GetAddressOf());
		if (FAILED(hr) || !srv)
			return nullptr;

		return new D3D11Texture(this, m_device.Get(), m_context.Get(), width, height, format, dxgiFormat, numMips, expandedToRGBA8, tex.Get(), srv.Get());
	}

	IRHIBuffer* RenderDeviceD3D11::CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage, const void* initialData)
	{
		if (!m_device || byteSize == 0)
			return nullptr;
		auto* buf = new D3D11Buffer(this, m_device.Get(), m_context.Get(), byteSize, isIndexBuffer, usage, initialData);
		if (!buf->GetD3DBuffer())
		{
			buf->ReleaseRHIResource();
			return nullptr;
		}
		return buf;
	}

	IRHIRenderTarget* RenderDeviceD3D11::CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat, PixelFormat depthFormat, uint32_t colorTargetCount, uint32_t msaaSamples)
	{
		if (!m_device || width == 0 || height == 0)
			return nullptr;
		auto* rt = new D3D11RenderTarget(this, m_device.Get(), m_context.Get(), m_featureLevel, width, height, colorFormat, depthFormat, colorTargetCount, msaaSamples);
		if (!rt->IsValid())
		{
			rt->ReleaseRHIResource();
			return nullptr;
		}
		return rt;
	}

	IRHIOcclusionQuery* RenderDeviceD3D11::CreateOcclusionQuery()
	{
		if (!m_device || !m_context)
			return nullptr;
		return new D3D11OcclusionQuery(m_device.Get(), m_context.Get());
	}

	void RenderDeviceD3D11::UnbindConflictingSRVs(ID3D11RenderTargetView* const* rtvs, uint32_t rtvCount, ID3D11DepthStencilView* dsv)
	{
		if (!m_context)
			return;

		ID3D11Resource* targetResources[5] = {};
		uint32_t targetCount = 0;
		if (rtvs)
		{
			for (uint32_t i = 0; i < rtvCount && i < 4; ++i)
			{
				if (rtvs[i])
				{
					rtvs[i]->GetResource(&targetResources[targetCount]);
					if (targetResources[targetCount])
						++targetCount;
				}
			}
		}
		if (dsv)
		{
			dsv->GetResource(&targetResources[targetCount]);
			if (targetResources[targetCount])
				++targetCount;
		}

		if (targetCount == 0)
			return;

		for (uint32_t slot = 0; slot < RHIStateTracker::MAX_SAMPLER_STAGES; ++slot)
		{
			if (m_boundPSSRVs[slot])
			{
				ComPtr<ID3D11Resource> srvRes;
				m_boundPSSRVs[slot]->GetResource(srvRes.GetAddressOf());
				for (uint32_t i = 0; i < targetCount; ++i)
				{
					if (srvRes.Get() == targetResources[i])
					{
						ID3D11ShaderResourceView* nullSrv = nullptr;
						m_context->PSSetShaderResources(slot, 1, &nullSrv);
						m_boundPSSRVs[slot] = nullptr;
						m_srvBindingsDirty = true;
						break;
					}
				}
			}
			if (m_boundVSSRVs[slot])
			{
				ComPtr<ID3D11Resource> srvRes;
				m_boundVSSRVs[slot]->GetResource(srvRes.GetAddressOf());
				for (uint32_t i = 0; i < targetCount; ++i)
				{
					if (srvRes.Get() == targetResources[i])
					{
						ID3D11ShaderResourceView* nullSrv = nullptr;
						if (m_featureLevel >= D3D_FEATURE_LEVEL_10_0)
							m_context->VSSetShaderResources(slot, 1, &nullSrv);
						m_boundVSSRVs[slot] = nullptr;
						m_srvBindingsDirty = true;
						break;
					}
				}
			}
		}

		for (uint32_t i = 0; i < targetCount; ++i)
		{
			if (targetResources[i])
				targetResources[i]->Release();
		}
	}

	bool RenderDeviceD3D11::SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride)
	{
		if (!m_context)
			return false;

		uint32_t effectiveCount = 0;
		if (targets && count > 0)
		{
			const uint32_t maxCount = (std::min)(count, 4u);
			for (uint32_t i = 0; i < maxCount; ++i)
			{
				if (targets[i] != nullptr)
					effectiveCount = i + 1;
			}
		}

		if (effectiveCount == 0)
		{
			m_activeRTVs[0] = m_defaultRTV.Get();
			m_activeRTVs[1] = nullptr;
			m_activeRTVs[2] = nullptr;
			m_activeRTVs[3] = nullptr;
			m_activeRTVCount = 1;
			m_activeDSV = depthStencilOverride ? static_cast<D3D11RenderTarget*>(depthStencilOverride)->GetDSV() : m_defaultDSV.Get();
			m_activeColorTex0 = m_backBufferTex.Get();
			m_activeDepthTex = depthStencilOverride && static_cast<D3D11RenderTarget*>(depthStencilOverride)->GetD3DDepthTex()
				? static_cast<D3D11RenderTarget*>(depthStencilOverride)->GetD3DDepthTex()->GetD3DTexture()
				: m_defaultDepthTex.Get();

			UnbindConflictingSRVs(m_activeRTVs, 1, m_activeDSV);
			m_context->OMSetRenderTargets(1, m_activeRTVs, m_activeDSV);
			m_srvBindingsDirty = true;
			m_renderTargetWidth = m_backBufferWidth;
			m_renderTargetHeight = m_backBufferHeight;
			NotifyRenderTargetsBound(0, nullptr, depthStencilOverride);
			return true;
		}

		auto* primary = static_cast<D3D11RenderTarget*>(targets[0]);
		m_activeRTVs[0] = nullptr;
		m_activeRTVs[1] = nullptr;
		m_activeRTVs[2] = nullptr;
		m_activeRTVs[3] = nullptr;

		uint32_t rtvCount = 0;
		if (effectiveCount == 1 && primary && primary->GetColorCount() > 1)
		{
			rtvCount = primary->GetColorCount();
			for (uint32_t i = 0; i < rtvCount; ++i)
				m_activeRTVs[i] = primary->GetRTV(i);
		}
		else
		{
			rtvCount = effectiveCount;
			for (uint32_t i = 0; i < rtvCount; ++i)
			{
				if (i == 0 && targets[0] == nullptr)
				{
					m_activeRTVs[0] = m_defaultRTV.Get();
				}
				else
				{
					auto* rt = static_cast<D3D11RenderTarget*>(targets[i]);
					m_activeRTVs[i] = rt ? rt->GetRTV(0) : nullptr;
				}
			}
		}

		m_activeRTVCount = rtvCount;
		if (depthStencilOverride)
		{
			auto* dsOverride = static_cast<D3D11RenderTarget*>(depthStencilOverride);
			m_activeDSV = dsOverride->GetDSV();
			m_activeDepthTex = dsOverride->GetD3DDepthTex() ? dsOverride->GetD3DDepthTex()->GetD3DTexture() : nullptr;
		}
		else if (primary && primary->GetDSV())
		{
			m_activeDSV = primary->GetDSV();
			m_activeDepthTex = primary->GetD3DDepthTex() ? primary->GetD3DDepthTex()->GetD3DTexture() : nullptr;
		}
		else if (!primary || (primary->GetWidth() == m_backBufferWidth && primary->GetHeight() == m_backBufferHeight && primary->GetSampleCount() == 1))
		{
			m_activeDSV = m_defaultDSV.Get();
			m_activeDepthTex = m_defaultDepthTex.Get();
		}
		else
		{
			m_activeDSV = nullptr;
			m_activeDepthTex = nullptr;
		}
		m_activeColorTex0 = primary
			? (primary->GetD3DColorTex(0) ? primary->GetD3DColorTex(0)->GetD3DTexture() : nullptr)
			: m_backBufferTex.Get();

		UnbindConflictingSRVs(m_activeRTVs, rtvCount, m_activeDSV);
		m_context->OMSetRenderTargets(rtvCount, m_activeRTVs, m_activeDSV);
		m_srvBindingsDirty = true;
		m_renderTargetWidth = primary ? primary->GetWidth() : m_backBufferWidth;
		m_renderTargetHeight = primary ? primary->GetHeight() : m_backBufferHeight;
		NotifyRenderTargetsBound(effectiveCount, targets, depthStencilOverride);
		return true;
	}

	void RenderDeviceD3D11::BeginRenderTarget(uint32_t width, uint32_t height)
	{
		m_rtSizeStack.push(std::make_pair(m_renderTargetWidth, m_renderTargetHeight));
		m_renderTargetWidth = width;
		m_renderTargetHeight = height;
	}

	void RenderDeviceD3D11::EndRenderTarget()
	{
		if (!m_rtSizeStack.empty())
		{
			m_renderTargetWidth = m_rtSizeStack.top().first;
			m_renderTargetHeight = m_rtSizeStack.top().second;
			m_rtSizeStack.pop();
		}
	}

	bool RenderDeviceD3D11::BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect, const RECT* dstRect, bool linearFilter)
	{
		if (!m_device || !m_context)
			return false;

		auto* srcRT = static_cast<D3D11RenderTarget*>(src);
		auto* dstRT = static_cast<D3D11RenderTarget*>(dst);

		ID3D11Texture2D* srcTex = srcRT ? (srcRT->GetD3DColorTex(0) ? srcRT->GetD3DColorTex(0)->GetD3DTexture() : nullptr) : m_backBufferTex.Get();
		ID3D11Texture2D* dstTex = dstRT ? (dstRT->GetD3DColorTex(0) ? dstRT->GetD3DColorTex(0)->GetD3DTexture() : nullptr) : m_backBufferTex.Get();
		if (!srcTex || !dstTex)
			return false;

		D3D11_TEXTURE2D_DESC sd = {}, dd = {};
		srcTex->GetDesc(&sd);
		dstTex->GetDesc(&dd);

		int sx0 = srcRect ? srcRect->left : 0;
		int sy0 = srcRect ? srcRect->top : 0;
		int sx1 = srcRect ? srcRect->right : static_cast<int>(sd.Width);
		int sy1 = srcRect ? srcRect->bottom : static_cast<int>(sd.Height);

		int dx0 = dstRect ? dstRect->left : 0;
		int dy0 = dstRect ? dstRect->top : 0;
		int dx1 = dstRect ? dstRect->right : static_cast<int>(dd.Width);
		int dy1 = dstRect ? dstRect->bottom : static_cast<int>(dd.Height);

		int sw = sx1 - sx0;
		int sh = sy1 - sy0;
		int dw = dx1 - dx0;
		int dh = dy1 - dy0;
		if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
			return false;

		// Fast path 1: MSAA resolve of identical size/format
		if (sd.SampleDesc.Count > 1 && dd.SampleDesc.Count == 1 && sd.Width == dd.Width && sd.Height == dd.Height && sd.Format == dd.Format && !srcRect && !dstRect)
		{
			m_context->ResolveSubresource(dstTex, 0, srcTex, 0, dd.Format);
			return true;
		}

		// Fast path 2: 1:1 unscaled copy with matching format
		if (sw == dw && sh == dh && sd.Format == dd.Format && sd.SampleDesc.Count == dd.SampleDesc.Count)
		{
			if (sx0 == 0 && sy0 == 0 && dx0 == 0 && dy0 == 0 &&
				static_cast<UINT>(sw) == sd.Width && static_cast<UINT>(sh) == sd.Height &&
				sd.Width == dd.Width && sd.Height == dd.Height)
			{
				m_context->CopyResource(dstTex, srcTex);
			}
			else
			{
				D3D11_BOX box = {
					static_cast<UINT>((std::max)(0, sx0)),
					static_cast<UINT>((std::max)(0, sy0)),
					0,
					static_cast<UINT>((std::min)(static_cast<int>(sd.Width), sx1)),
					static_cast<UINT>((std::min)(static_cast<int>(sd.Height), sy1)),
					1
				};
				m_context->CopySubresourceRegion(dstTex, 0, static_cast<UINT>((std::max)(0, dx0)), static_cast<UINT>((std::max)(0, dy0)), 0, srcTex, 0, &box);
			}
			return true;
		}

		// Scaled / format-converting shader quad blit path (supports FL 9_3 .. 11_1)
		ID3D11ShaderResourceView* srcSRV = srcRT && srcRT->GetD3DColorTex(0) ? srcRT->GetD3DColorTex(0)->GetSRV() : nullptr;
		ComPtr<ID3D11ShaderResourceView> tempSrcSRV;
		if (!srcSRV)
		{
			if (FAILED(m_device->CreateShaderResourceView(srcTex, nullptr, tempSrcSRV.GetAddressOf())))
				return false;
			srcSRV = tempSrcSRV.Get();
		}

		ID3D11RenderTargetView* dstRTV = dstRT ? dstRT->GetRTV(0) : m_defaultRTV.Get();
		if (!dstRTV)
			return false;

		if (!m_blitVS || !m_blitPS || !m_blitInputLayout || !m_blitVB)
		{
			static const char* kBlitHLSL =
				"Texture2D gTex : register(t0);\n"
				"SamplerState gSamp : register(s0);\n"
				"struct VSIn { float2 pos : POSITION; float2 uv : TEXCOORD0; };\n"
				"struct VSOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };\n"
				"VSOut BlitVS(VSIn i) { VSOut o; o.pos = float4(i.pos, 0.0, 1.0); o.uv = i.uv; return o; }\n"
				"float4 BlitPS(VSOut i) : SV_Target { return gTex.Sample(gSamp, i.uv); }\n";

			const auto& d3dc = GetD3D11Compiler();
			if (!d3dc.pfnCompile)
				return false;
			const char* vsProf = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) ? "vs_4_0" : "vs_4_0_level_9_3";
			const char* psProf = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) ? "ps_4_0" : "ps_4_0_level_9_3";
			ComPtr<ID3DBlob> vsBlob, psBlob;
			if (FAILED(d3dc.pfnCompile(kBlitHLSL, std::strlen(kBlitHLSL), "Blit", nullptr, nullptr, "BlitVS", vsProf, 0, 0, vsBlob.GetAddressOf(), nullptr)))
				return false;
			if (FAILED(d3dc.pfnCompile(kBlitHLSL, std::strlen(kBlitHLSL), "Blit", nullptr, nullptr, "BlitPS", psProf, 0, 0, psBlob.GetAddressOf(), nullptr)))
				return false;

			if (FAILED(m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_blitVS.GetAddressOf())))
				return false;
			if (FAILED(m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_blitPS.GetAddressOf())))
				return false;

			const D3D11_INPUT_ELEMENT_DESC elems[2] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			if (FAILED(m_device->CreateInputLayout(elems, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_blitInputLayout.GetAddressOf())))
				return false;

			D3D11_BUFFER_DESC vbd = {};
			vbd.ByteWidth = sizeof(float) * 16;
			vbd.Usage = D3D11_USAGE_DYNAMIC;
			vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			if (FAILED(m_device->CreateBuffer(&vbd, nullptr, m_blitVB.GetAddressOf())))
				return false;

			D3D11_SAMPLER_DESC samp = {};
			samp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samp.MaxLOD = D3D11_FLOAT32_MAX;
			samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			m_device->CreateSamplerState(&samp, m_blitLinearSampler.GetAddressOf());
			samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			m_device->CreateSamplerState(&samp, m_blitPointSampler.GetAddressOf());
		}

		float u0 = static_cast<float>(sx0) / static_cast<float>(sd.Width);
		float v0 = static_cast<float>(sy0) / static_cast<float>(sd.Height);
		float u1 = static_cast<float>(sx1) / static_cast<float>(sd.Width);
		float v1 = static_cast<float>(sy1) / static_cast<float>(sd.Height);

		const float quadVerts[16] = {
			-1.0f,  1.0f, u0, v0,
			 1.0f,  1.0f, u1, v0,
			-1.0f, -1.0f, u0, v1,
			 1.0f, -1.0f, u1, v1
		};
		D3D11_MAPPED_SUBRESOURCE mapped = {};
		if (SUCCEEDED(m_context->Map(m_blitVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && mapped.pData)
		{
			std::memcpy(mapped.pData, quadVerts, sizeof(quadVerts));
			m_context->Unmap(m_blitVB.Get(), 0);
		}

		UnbindConflictingSRVs(&dstRTV, 1);
		m_context->OMSetRenderTargets(1, &dstRTV, nullptr);

		D3D11_VIEWPORT vp = { static_cast<float>(dx0), static_cast<float>(dy0), static_cast<float>(dw), static_cast<float>(dh), 0.0f, 1.0f };
		m_context->RSSetViewports(1, &vp);
		m_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFFu);
		m_context->OMSetDepthStencilState(nullptr, 0);
		m_context->RSSetState(nullptr);

		UINT stride = sizeof(float) * 4;
		UINT offset = 0;
		ID3D11Buffer* vb = m_blitVB.Get();
		m_context->IASetInputLayout(m_blitInputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_context->VSSetShader(m_blitVS.Get(), nullptr, 0);
		m_context->PSSetShader(m_blitPS.Get(), nullptr, 0);
		m_context->PSSetShaderResources(0, 1, &srcSRV);
		ID3D11SamplerState* samp = linearFilter ? m_blitLinearSampler.Get() : m_blitPointSampler.Get();
		m_context->PSSetSamplers(0, 1, &samp);
		m_context->Draw(4, 0);

		ID3D11ShaderResourceView* nullSrv = nullptr;
		m_context->PSSetShaderResources(0, 1, &nullSrv);
		m_boundPSSRVs[0] = nullptr;

		// Restore active RTs, stream 0 VB, viewport, and mark state tracker dirty so next draw re-binds states
		UnbindConflictingSRVs(m_activeRTVs, m_activeRTVCount, m_activeDSV);
		m_context->OMSetRenderTargets(m_activeRTVCount, m_activeRTVs, m_activeDSV);
		if (m_streams[0].buffer)
		{
			ID3D11Buffer* s0vb = static_cast<ID3D11Buffer*>(m_streams[0].buffer->GetNativeHandle());
			UINT s0Stride = m_streams[0].stride;
			UINT s0Offset = m_streams[0].offsetInBytes;
			m_context->IASetVertexBuffers(0, 1, &s0vb, &s0Stride, &s0Offset);
		}
		m_stateTracker.MarkAllDirty();
		m_lastAppliedPsOutputMask = 0xFFu;
		m_srvBindingsDirty = true;
		SetViewport(m_currentViewport);
		if (m_currentProgram)
			BindShaderProgram(m_currentProgram);
		return true;
	}

	bool RenderDeviceD3D11::ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height)
	{
		if (!m_device || !m_context || !nativeFboOrTexHandle)
			return false;

		auto* extTex = reinterpret_cast<ID3D11Texture2D*>(nativeFboOrTexHandle);
		ComPtr<ID3D11RenderTargetView> extRTV;
		if (FAILED(m_device->CreateRenderTargetView(extTex, nullptr, extRTV.GetAddressOf())))
			return false;

		m_defaultRTV = extRTV;
		m_backBufferTex = extTex;
		m_backBufferWidth = width;
		m_backBufferHeight = height;
		m_renderTargetWidth = width;
		m_renderTargetHeight = height;
		m_activeRTVs[0] = m_defaultRTV.Get();
		m_activeColorTex0 = extTex;
		m_context->OMSetRenderTargets(1, m_activeRTVs, m_activeDSV);
		m_srvBindingsDirty = true;
		return true;
	}

	bool RenderDeviceD3D11::SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider)
	{
		if (streamNumber >= 4)
			return false;
		m_streams[streamNumber].frequencyDivider = divider;
		return true;
	}

	bool RenderDeviceD3D11::DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount)
	{
		if (!m_context || instanceCount == 0)
			return false;

		D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uint32_t indexCount = 0;
		if (!ComputePrimitiveTopologyAndCount(primitiveType, primitiveCount, topo, indexCount))
			return false;

		ApplyPipelineStatesBeforeDraw();
		m_context->IASetPrimitiveTopology(topo);
		m_context->DrawIndexedInstanced(indexCount, instanceCount, indexStart, baseVertexIndex, 0);
		return true;
	}

	bool RenderDeviceD3D11::ReadDepthPixel(int x, int y, float& outDepth)
	{
		outDepth = 1.0f;
		if (!m_device || !m_context || !m_activeDepthTex || x < 0 || y < 0)
			return false;

		D3D11_TEXTURE2D_DESC dd = {};
		m_activeDepthTex->GetDesc(&dd);
		if (static_cast<UINT>(x) >= dd.Width || static_cast<UINT>(y) >= dd.Height)
			return false;

		// Note: D3D11 requires CopyResource (full resource) for Depth-Stencil formats
		D3D11_TEXTURE2D_DESC sd = dd;
		sd.Usage = D3D11_USAGE_STAGING;
		sd.BindFlags = 0;
		sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		sd.MiscFlags = 0;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		if (dd.SampleDesc.Count > 1)
			return false;

		ComPtr<ID3D11Texture2D> staging;
		if (FAILED(m_device->CreateTexture2D(&sd, nullptr, staging.GetAddressOf())))
			return false;

		m_context->CopyResource(staging.Get(), m_activeDepthTex);

		D3D11_MAPPED_SUBRESOURCE mapped = {};
		if (FAILED(m_context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) || !mapped.pData)
			return false;

		const uint8_t* row = static_cast<const uint8_t*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
		if (dd.Format == DXGI_FORMAT_D32_FLOAT || dd.Format == DXGI_FORMAT_R32_TYPELESS)
		{
			std::memcpy(&outDepth, row + static_cast<size_t>(x) * 4u, sizeof(float));
		}
		else if (dd.Format == DXGI_FORMAT_D16_UNORM || dd.Format == DXGI_FORMAT_R16_TYPELESS)
		{
			uint16_t d16 = 0;
			std::memcpy(&d16, row + static_cast<size_t>(x) * 2u, sizeof(uint16_t));
			outDepth = static_cast<float>(d16) / 65535.0f;
		}
		else
		{
			// D24_UNORM_S8_UINT: lower 24 bits are normalized depth
			uint32_t d24s8 = 0;
			std::memcpy(&d24s8, row + static_cast<size_t>(x) * 4u, sizeof(uint32_t));
			outDepth = static_cast<float>(d24s8 & 0x00FFFFFFu) / 16777215.0f;
		}

		m_context->Unmap(staging.Get(), 0);
		return true;
	}

	bool RenderDeviceD3D11::ReadTexturePixels(IRHITexture* texture, void* pDataOut)
	{
		if (!m_device || !m_context || !texture || !pDataOut)
			return false;

		auto* srv = static_cast<ID3D11ShaderResourceView*>(texture->GetNativeHandle());
		if (!srv)
			return false;

		ComPtr<ID3D11Resource> res;
		srv->GetResource(res.GetAddressOf());
		if (!res)
			return false;

		ComPtr<ID3D11Texture2D> srcTex;
		if (FAILED(res.As(&srcTex)) || !srcTex)
			return false;

		D3D11_TEXTURE2D_DESC td = {};
		srcTex->GetDesc(&td);
		if (td.Width == 0 || td.Height == 0)
			return false;

		D3D11_TEXTURE2D_DESC sd = td;
		sd.MipLevels = 1;
		sd.ArraySize = 1;
		sd.Usage = D3D11_USAGE_STAGING;
		sd.BindFlags = 0;
		sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		sd.MiscFlags = 0;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		ComPtr<ID3D11Texture2D> staging;
		if (FAILED(m_device->CreateTexture2D(&sd, nullptr, staging.GetAddressOf())))
			return false;

		if (td.SampleDesc.Count > 1)
		{
			D3D11_TEXTURE2D_DESC rd = sd;
			rd.Usage = D3D11_USAGE_DEFAULT;
			rd.CPUAccessFlags = 0;
			ComPtr<ID3D11Texture2D> resolved;
			if (FAILED(m_device->CreateTexture2D(&rd, nullptr, resolved.GetAddressOf())))
				return false;
			m_context->ResolveSubresource(resolved.Get(), 0, srcTex.Get(), 0, td.Format);
			m_context->CopyResource(staging.Get(), resolved.Get());
		}
		else
		{
			m_context->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, srcTex.Get(), 0, nullptr);
		}

		D3D11_MAPPED_SUBRESOURCE mapped = {};
		if (FAILED(m_context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) || !mapped.pData)
			return false;

		uint8_t* dst = static_cast<uint8_t*>(pDataOut);
		const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
		const bool isBGRA = (td.Format == DXGI_FORMAT_B8G8R8A8_UNORM || td.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || td.Format == DXGI_FORMAT_B8G8R8X8_UNORM);
		const bool isA8 = (td.Format == DXGI_FORMAT_A8_UNORM || td.Format == DXGI_FORMAT_R8_UNORM);

		for (UINT y = 0; y < td.Height; ++y)
		{
			const uint8_t* srcRow = src + static_cast<size_t>(y) * mapped.RowPitch;
			uint8_t* dstRow = dst + static_cast<size_t>(y) * td.Width * 4u;
			if (isBGRA)
			{
				for (UINT x = 0; x < td.Width; ++x)
				{
					dstRow[x * 4u + 0] = srcRow[x * 4u + 2];
					dstRow[x * 4u + 1] = srcRow[x * 4u + 1];
					dstRow[x * 4u + 2] = srcRow[x * 4u + 0];
					dstRow[x * 4u + 3] = (td.Format == DXGI_FORMAT_B8G8R8X8_UNORM) ? 255u : srcRow[x * 4u + 3];
				}
			}
			else if (isA8)
			{
				for (UINT x = 0; x < td.Width; ++x)
				{
					dstRow[x * 4u + 0] = 255u;
					dstRow[x * 4u + 1] = 255u;
					dstRow[x * 4u + 2] = 255u;
					dstRow[x * 4u + 3] = srcRow[x];
				}
			}
			else
			{
				std::memcpy(dstRow, srcRow, static_cast<size_t>(td.Width) * 4u);
			}
		}

		m_context->Unmap(staging.Get(), 0);
		return true;
	}

	IRHIShaderProgram* RenderDeviceD3D11::CompileShaderProgram(const RHIShaderProgramDesc& desc)
	{
		if (!m_device)
			return nullptr;
		auto* prog = new D3D11ShaderProgram(this, m_device.Get(), desc);
		if (!prog->IsValid())
		{
			prog->ReleaseRHIResource();
			return nullptr;
		}
		return prog;
	}

	bool RenderDeviceD3D11::BindShaderProgram(IRHIShaderProgram* program)
	{
		if (!m_context)
			return false;
		auto* nextProg = static_cast<D3D11ShaderProgram*>(program);
		if (m_currentProgram != nextProg)
		{
			m_currentProgram = nextProg;
			m_srvBindingsDirty = true;
		}
		if (!m_currentProgram)
		{
			m_context->VSSetShader(nullptr, nullptr, 0);
			m_context->PSSetShader(nullptr, nullptr, 0);
			return true;
		}
		m_context->VSSetShader(m_currentProgram->GetVS(), nullptr, 0);
		m_context->PSSetShader(m_currentProgram->GetPS(), nullptr, 0);
		return true;
	}

	ID3D11Buffer* RenderDeviceD3D11::EnsureDynamicScratchBuffer(ComPtr<ID3D11Buffer>& buf, uint32_t& currentCap, uint32_t requiredBytes, UINT bindFlags)
	{
		if (!m_device || requiredBytes == 0)
			return nullptr;
		uint32_t alignedBytes = (requiredBytes + 15u) & ~15u;
		if (!buf || currentCap < alignedBytes)
		{
			uint32_t newCap = alignedBytes;
			if (bindFlags != D3D11_BIND_CONSTANT_BUFFER)
			{
				newCap = (std::max)(alignedBytes, (std::max)(currentCap * 2u, 65536u));
			}
			D3D11_BUFFER_DESC bd = {};
			bd.ByteWidth = newCap;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = bindFlags;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			buf.Reset();
			if (FAILED(m_device->CreateBuffer(&bd, nullptr, buf.GetAddressOf())))
			{
				currentCap = 0;
				return nullptr;
			}
			currentCap = newCap;
		}
		return buf.Get();
	}

	bool RenderDeviceD3D11::SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize)
	{
		if (!m_context)
			return false;

		if (vsConstants && vsByteSize > 0)
		{
			ID3D11Buffer* cb = EnsureDynamicScratchBuffer(m_vsGlobalCB, m_vsGlobalCBCapacity, vsByteSize, D3D11_BIND_CONSTANT_BUFFER);
			if (cb)
			{
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				if (SUCCEEDED(m_context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && mapped.pData)
				{
					std::memcpy(mapped.pData, vsConstants, vsByteSize);
					m_context->Unmap(cb, 0);
				}
				m_context->VSSetConstantBuffers(0, 1, &cb);
			}
		}

		if (psConstants && psByteSize > 0)
		{
			ID3D11Buffer* cb = EnsureDynamicScratchBuffer(m_psGlobalCB, m_psGlobalCBCapacity, psByteSize, D3D11_BIND_CONSTANT_BUFFER);
			if (cb)
			{
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				if (SUCCEEDED(m_context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && mapped.pData)
				{
					std::memcpy(mapped.pData, psConstants, psByteSize);
					m_context->Unmap(cb, 0);
				}
				m_context->PSSetConstantBuffers(0, 1, &cb);
			}
		}

		return true;
	}

	uint32_t RenderDeviceD3D11::GetRenderState(const ERenderState& State)
	{
		return m_stateTracker.GetRenderState(State);
	}

	bool RenderDeviceD3D11::SetRenderState(const ERenderState State, const uint32_t Value)
	{
		m_stateTracker.SetRenderState(State, Value);
		return true;
	}

	bool RenderDeviceD3D11::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		m_stateTracker.SetClipPlane(Index, pPlane);
		return true;
	}

	bool RenderDeviceD3D11::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t /*nDataFormat*/, uint32_t /*nDataType*/)
	{
		if (!m_device || !m_context || !m_activeColorTex0 || !pDataOut || nWidth <= 0 || nHeight <= 0)
			return false;

		D3D11_TEXTURE2D_DESC srcDesc = {};
		m_activeColorTex0->GetDesc(&srcDesc);

		int x0 = (std::max)(0, nLeft);
		int y0 = (std::max)(0, nTop);
		int x1 = (std::min)(static_cast<int>(srcDesc.Width), nLeft + nWidth);
		int y1 = (std::min)(static_cast<int>(srcDesc.Height), nTop + nHeight);
		if (x1 <= x0 || y1 <= y0)
			return false;

		uint32_t copyW = static_cast<uint32_t>(x1 - x0);
		uint32_t copyH = static_cast<uint32_t>(y1 - y0);

		ComPtr<ID3D11Texture2D> resolvedTex;
		ID3D11Texture2D* readSrc = m_activeColorTex0;
		if (srcDesc.SampleDesc.Count > 1)
		{
			D3D11_TEXTURE2D_DESC rd = srcDesc;
			rd.SampleDesc.Count = 1;
			rd.SampleDesc.Quality = 0;
			rd.Usage = D3D11_USAGE_DEFAULT;
			rd.BindFlags = 0;
			if (FAILED(m_device->CreateTexture2D(&rd, nullptr, resolvedTex.GetAddressOf())))
				return false;
			m_context->ResolveSubresource(resolvedTex.Get(), 0, m_activeColorTex0, 0, srcDesc.Format);
			readSrc = resolvedTex.Get();
		}

		D3D11_TEXTURE2D_DESC stageDesc = {};
		stageDesc.Width = copyW;
		stageDesc.Height = copyH;
		stageDesc.MipLevels = 1;
		stageDesc.ArraySize = 1;
		stageDesc.Format = srcDesc.Format;
		stageDesc.SampleDesc.Count = 1;
		stageDesc.Usage = D3D11_USAGE_STAGING;
		stageDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ComPtr<ID3D11Texture2D> staging;
		if (FAILED(m_device->CreateTexture2D(&stageDesc, nullptr, staging.GetAddressOf())))
			return false;

		D3D11_BOX box = { static_cast<UINT>(x0), static_cast<UINT>(y0), 0, static_cast<UINT>(x1), static_cast<UINT>(y1), 1 };
		m_context->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, readSrc, 0, &box);

		D3D11_MAPPED_SUBRESOURCE mapped = {};
		if (FAILED(m_context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) || !mapped.pData)
			return false;

		std::memset(pDataOut, 0, static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 4u);
		const bool isBGRA = (srcDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || srcDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

		for (uint32_t y = 0; y < copyH; ++y)
		{
			const uint8_t* srcRow = static_cast<const uint8_t*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
			uint8_t* dstRow = static_cast<uint8_t*>(pDataOut) + static_cast<size_t>(y) * static_cast<size_t>(nWidth) * 4u;
			if (isBGRA)
			{
				for (uint32_t x = 0; x < copyW; ++x)
				{
					dstRow[x * 4u + 0] = srcRow[x * 4u + 2];
					dstRow[x * 4u + 1] = srcRow[x * 4u + 1];
					dstRow[x * 4u + 2] = srcRow[x * 4u + 0];
					dstRow[x * 4u + 3] = srcRow[x * 4u + 3];
				}
			}
			else
			{
				std::memcpy(dstRow, srcRow, copyW * 4u);
			}
		}

		m_context->Unmap(staging.Get(), 0);
		return true;
	}

	int RenderDeviceD3D11::GetMaxSimultaneousTextures()
	{
		return static_cast<int>(m_caps.maxTextureStages);
	}

	bool RenderDeviceD3D11::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		if (!m_context || stage >= RHIStateTracker::MAX_SAMPLER_STAGES)
			return false;

		ID3D11ShaderResourceView* srv = texture ? static_cast<ID3D11ShaderResourceView*>(texture->GetNativeHandle()) : nullptr;
		if (m_logicalSRVs[stage] != srv)
		{
			m_logicalSRVs[stage] = srv;
			m_srvBindingsDirty = true;
		}
		return true;
	}

	ID3D11InputLayout* RenderDeviceD3D11::GetOrCreateInputLayout(CVertexDeclaration* decl, D3D11ShaderProgram* program)
	{
		if (!m_device || !decl || !program)
			return nullptr;

		uint64_t key = decl->GetHash();
		HashCombineU64(key, program->GetVSHash());
		for (uint32_t s = 0; s < 4; ++s)
		{
			HashCombineU32(key, m_streams[s].frequencyDivider);
		}

		auto it = m_inputLayouts.find(key);
		if (it != m_inputLayouts.end())
			return it->second.Get();

		std::vector<D3D11_INPUT_ELEMENT_DESC> elems;
		elems.reserve(decl->GetElements().size() + 4);

		for (const auto& ve : decl->GetElements())
		{
			if (ve.IsEndDeclare())
				continue;

			D3D11_INPUT_ELEMENT_DESC d = {};
			d.SemanticName = MapDeclUsageToSemantic(ve.Usage, program->UsesSVPosition() && ve.UsageIndex == 0);
			d.SemanticIndex = ve.UsageIndex;
			d.Format = MapDeclTypeToDXGI(ve.Type, m_featureLevel);
			d.InputSlot = ve.Stream;
			d.AlignedByteOffset = ve.Offset;

			uint32_t freq = (ve.Stream < 4) ? m_streams[ve.Stream].frequencyDivider : 0u;
			// Handle D3DSTREAMSOURCE_INSTANCEDATA (0x80000000 | stepRate) as well as plain stepRate >= 1
			if (ve.Stream > 0 && freq != 0)
			{
				uint32_t step = (freq & 0xC0000000u) ? (freq & 0x3FFFFFFFu) : freq;
				if (step == 0) step = 1;
				d.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				d.InstanceDataStepRate = step;
			}
			else
			{
				d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				d.InstanceDataStepRate = 0;
			}
			elems.push_back(d);
		}

		// Ensure every reflected VS input parameter has a matching input element (matching OpenGL [0,0,0,1] default)
		for (const auto& req : program->GetInputSignature())
		{
			bool found = false;
			for (const auto& existing : elems)
			{
				if (existing.SemanticIndex == req.semanticIndex &&
					(_stricmp(existing.SemanticName, req.semanticName.c_str()) == 0 ||
					 (req.semanticIndex == 0 &&
					  (_stricmp(req.semanticName.c_str(), "POSITION") == 0 || _stricmp(req.semanticName.c_str(), "SV_Position") == 0) &&
					  (_stricmp(existing.SemanticName, "POSITION") == 0 || _stricmp(existing.SemanticName, "SV_Position") == 0))))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				D3D11_INPUT_ELEMENT_DESC fallback = {};
				fallback.SemanticName = req.semanticName.c_str();
				fallback.SemanticIndex = req.semanticIndex;
				if (req.componentType == D3D_REGISTER_COMPONENT_UINT32)
					fallback.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (req.componentType == D3D_REGISTER_COMPONENT_SINT32)
					fallback.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else
					fallback.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

				if (m_featureLevel >= D3D_FEATURE_LEVEL_10_0 && m_defaultAttrVB)
				{
					fallback.InputSlot = 15;
					fallback.AlignedByteOffset = 0;
					fallback.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
					fallback.InstanceDataStepRate = 0;
				}
				else
				{
					fallback.InputSlot = 0;
					fallback.AlignedByteOffset = 0;
					fallback.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					fallback.InstanceDataStepRate = 0;
				}
				elems.push_back(fallback);
			}
		}

		ComPtr<ID3D11InputLayout> layout;
		if (!elems.empty())
		{
			const auto& bc = program->GetVSBytecode();
			HRESULT hr = m_device->CreateInputLayout(elems.data(), static_cast<UINT>(elems.size()), bc.data(), bc.size(), layout.GetAddressOf());
			if (FAILED(hr))
			{
				OUTPUT_LOG("WARN: D3D11 CreateInputLayout failed (hr=0x%08X, elems=%zu)\n", static_cast<unsigned int>(hr), elems.size());
			}
		}
		m_inputLayouts[key] = layout;
		return layout.Get();
	}

	void RenderDeviceD3D11::ApplyPipelineStatesBeforeDraw()
	{
		if (!m_device || !m_context)
			return;

		// 1. Blend state
		const uint8_t psOutMask = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0 && m_currentProgram)
			? (m_currentProgram->GetPSOutputMask() & 0x0Fu)
			: 0x0Fu;
		if (m_stateTracker.IsBlendDirty() || psOutMask != m_lastAppliedPsOutputMask)
		{
			uint64_t h = 14695981039346656037ULL;
			const uint32_t alphaBlend = m_stateTracker.GetRenderState(ERenderState::ALPHABLENDENABLE);
			const uint32_t sepAlpha   = m_stateTracker.GetRenderState(ERenderState::SEPARATEALPHABLENDENABLE);
			const uint32_t srcBlend   = m_stateTracker.GetRenderState(ERenderState::SRCBLEND);
			const uint32_t dstBlend   = m_stateTracker.GetRenderState(ERenderState::DESTBLEND);
			const uint32_t blendOp    = m_stateTracker.GetRenderState(ERenderState::BLENDOP);
			const uint32_t srcBlendA  = m_stateTracker.GetRenderState(ERenderState::SRCBLENDALPHA);
			const uint32_t dstBlendA  = m_stateTracker.GetRenderState(ERenderState::DESTBLENDALPHA);
			const uint32_t blendOpA   = m_stateTracker.GetRenderState(ERenderState::BLENDOPALPHA);
			const uint32_t cw0        = m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE) & 0x0Fu;
			const uint32_t cw1        = m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE1) & 0x0Fu;
			const uint32_t cw2        = m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE2) & 0x0Fu;
			const uint32_t cw3        = m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE3) & 0x0Fu;

			HashCombineU32(h, alphaBlend);
			HashCombineU32(h, sepAlpha);
			HashCombineU32(h, srcBlend);
			HashCombineU32(h, dstBlend);
			HashCombineU32(h, blendOp);
			HashCombineU32(h, srcBlendA);
			HashCombineU32(h, dstBlendA);
			HashCombineU32(h, blendOpA);
			HashCombineU32(h, cw0 | (cw1 << 4) | (cw2 << 8) | (cw3 << 12) | (static_cast<uint32_t>(psOutMask) << 16));

			auto it = m_blendStates.find(h);
			if (it == m_blendStates.end())
			{
				D3D11_BLEND_DESC bd = {};
				bd.IndependentBlendEnable = (m_featureLevel >= D3D_FEATURE_LEVEL_10_0) && (cw1 != cw0 || cw2 != cw0 || cw3 != cw0 || psOutMask != 0x0Fu);
				const uint8_t masks[4] = { static_cast<uint8_t>(cw0), static_cast<uint8_t>(cw1), static_cast<uint8_t>(cw2), static_cast<uint8_t>(cw3) };
				for (int i = 0; i < 4; ++i)
				{
					const bool psWritesSlot = !bd.IndependentBlendEnable || ((psOutMask & (1u << i)) != 0);
					bd.RenderTarget[i].BlendEnable = (psWritesSlot && alphaBlend) ? TRUE : FALSE;
					bd.RenderTarget[i].SrcBlend = ToD3D11Blend(srcBlend, false);
					bd.RenderTarget[i].DestBlend = ToD3D11Blend(dstBlend, false);
					bd.RenderTarget[i].BlendOp = ToD3D11BlendOp(blendOp);
					bd.RenderTarget[i].SrcBlendAlpha = sepAlpha ? ToD3D11Blend(srcBlendA, true) : ToD3D11Blend(srcBlend, true);
					bd.RenderTarget[i].DestBlendAlpha = sepAlpha ? ToD3D11Blend(dstBlendA, true) : ToD3D11Blend(dstBlend, true);
					bd.RenderTarget[i].BlendOpAlpha = sepAlpha ? ToD3D11BlendOp(blendOpA) : ToD3D11BlendOp(blendOp);
					bd.RenderTarget[i].RenderTargetWriteMask = !psWritesSlot ? 0 : (bd.IndependentBlendEnable ? masks[i] : masks[0]);
				}
				ComPtr<ID3D11BlendState> bs;
				m_device->CreateBlendState(&bd, bs.GetAddressOf());
				it = m_blendStates.emplace(h, bs).first;
			}

			uint32_t bfPacked = m_stateTracker.GetRenderState(ERenderState::BLENDFACTOR);
			float blendFactor[4] = {
				static_cast<float>((bfPacked >> 16) & 0xFFu) / 255.0f,
				static_cast<float>((bfPacked >> 8) & 0xFFu) / 255.0f,
				static_cast<float>(bfPacked & 0xFFu) / 255.0f,
				static_cast<float>((bfPacked >> 24) & 0xFFu) / 255.0f
			};
			m_context->OMSetBlendState(it->second.Get(), blendFactor, 0xFFFFFFFFu);
			m_stateTracker.ClearBlendDirty();
			m_lastAppliedPsOutputMask = psOutMask;
		}

		// 2. Rasterizer state
		if (m_stateTracker.IsRasterizerDirty())
		{
			uint64_t h = 14695981039346656037ULL;
			const uint32_t cullMode   = m_stateTracker.GetRenderState(ERenderState::CULLMODE);
			const uint32_t fillMode   = m_stateTracker.GetRenderState(ERenderState::FILLMODE);
			const uint32_t scissorEn  = m_stateTracker.GetRenderState(ERenderState::SCISSORTESTENABLE);
			const uint32_t biasBits   = m_stateTracker.GetRenderState(ERenderState::DEPTHBIAS);
			const uint32_t slopeBits  = m_stateTracker.GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS);
			const uint32_t msaaEn     = m_stateTracker.GetRenderState(ERenderState::MULTISAMPLEANTIALIAS);

			HashCombineU32(h, cullMode);
			HashCombineU32(h, fillMode);
			HashCombineU32(h, scissorEn);
			HashCombineU32(h, biasBits);
			HashCombineU32(h, slopeBits);
			HashCombineU32(h, msaaEn);

			auto it = m_rasterizerStates.find(h);
			if (it == m_rasterizerStates.end())
			{
				float fBias = 0.0f, fSlope = 0.0f;
				std::memcpy(&fBias, &biasBits, sizeof(float));
				std::memcpy(&fSlope, &slopeBits, sizeof(float));

				D3D11_RASTERIZER_DESC rd = {};
				rd.FillMode = (fillMode == RSV_FILL_WIREFRAME) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
				if (cullMode == RSV_CULL_NONE) rd.CullMode = D3D11_CULL_NONE;
				else if (cullMode == RSV_CULL_CW) rd.CullMode = D3D11_CULL_FRONT;
				else rd.CullMode = D3D11_CULL_BACK; // RSV_CULL_CCW (default D3D clockwise front-facing)
				rd.FrontCounterClockwise = FALSE;
				rd.DepthBias = static_cast<INT>(fBias * 16777215.0f);
				rd.SlopeScaledDepthBias = fSlope;
				rd.DepthClipEnable = TRUE;
				rd.ScissorEnable = scissorEn ? TRUE : FALSE;
				rd.MultisampleEnable = msaaEn ? TRUE : FALSE;
				rd.AntialiasedLineEnable = FALSE;

				ComPtr<ID3D11RasterizerState> rs;
				m_device->CreateRasterizerState(&rd, rs.GetAddressOf());
				it = m_rasterizerStates.emplace(h, rs).first;
			}
			m_context->RSSetState(it->second.Get());
			m_stateTracker.ClearRasterizerDirty();
		}

		// 3. Depth-stencil state
		if (m_stateTracker.IsDepthStencilDirty())
		{
			uint64_t h = 14695981039346656037ULL;
			const uint32_t zEnable     = m_stateTracker.GetRenderState(ERenderState::ZENABLE);
			const uint32_t zWrite      = m_stateTracker.GetRenderState(ERenderState::ZWRITEENABLE);
			const uint32_t zFunc       = m_stateTracker.GetRenderState(ERenderState::ZFUNC);
			const uint32_t stEnable    = m_stateTracker.GetRenderState(ERenderState::STENCILENABLE);
			const uint32_t stMask      = m_stateTracker.GetRenderState(ERenderState::STENCILMASK) & 0xFFu;
			const uint32_t stWriteMask = m_stateTracker.GetRenderState(ERenderState::STENCILWRITEMASK) & 0xFFu;
			const uint32_t stFunc      = m_stateTracker.GetRenderState(ERenderState::STENCILFUNC);
			const uint32_t stFail      = m_stateTracker.GetRenderState(ERenderState::STENCILFAIL);
			const uint32_t stZFail     = m_stateTracker.GetRenderState(ERenderState::STENCILZFAIL);
			const uint32_t stPass      = m_stateTracker.GetRenderState(ERenderState::STENCILPASS);
			const uint32_t twoSided    = m_stateTracker.GetRenderState(ERenderState::TWOSIDEDSTENCILMODE);
			const uint32_t ccwFunc     = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILFUNC);
			const uint32_t ccwFail     = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILFAIL);
			const uint32_t ccwZFail    = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILZFAIL);
			const uint32_t ccwPass     = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILPASS);
			const uint32_t stRef       = m_stateTracker.GetRenderState(ERenderState::STENCILREF);

			HashCombineU32(h, zEnable);
			HashCombineU32(h, zWrite);
			HashCombineU32(h, zFunc);
			HashCombineU32(h, stEnable);
			HashCombineU32(h, stMask | (stWriteMask << 8));
			HashCombineU32(h, stFunc);
			HashCombineU32(h, stFail);
			HashCombineU32(h, stZFail);
			HashCombineU32(h, stPass);
			HashCombineU32(h, twoSided);
			HashCombineU32(h, ccwFunc);
			HashCombineU32(h, ccwFail);
			HashCombineU32(h, ccwZFail);
			HashCombineU32(h, ccwPass);

			auto it = m_depthStencilStates.find(h);
			if (it == m_depthStencilStates.end())
			{
				D3D11_DEPTH_STENCIL_DESC dsd = {};
				dsd.DepthEnable = zEnable ? TRUE : FALSE;
				dsd.DepthWriteMask = zWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
				dsd.DepthFunc = ToD3D11CmpFunc(zFunc);
				dsd.StencilEnable = stEnable ? TRUE : FALSE;
				dsd.StencilReadMask = static_cast<UINT8>(stMask);
				dsd.StencilWriteMask = static_cast<UINT8>(stWriteMask);
				dsd.FrontFace.StencilFunc = ToD3D11CmpFunc(stFunc);
				dsd.FrontFace.StencilFailOp = ToD3D11StencilOp(stFail);
				dsd.FrontFace.StencilDepthFailOp = ToD3D11StencilOp(stZFail);
				dsd.FrontFace.StencilPassOp = ToD3D11StencilOp(stPass);
				if (twoSided)
				{
					dsd.BackFace.StencilFunc = ToD3D11CmpFunc(ccwFunc);
					dsd.BackFace.StencilFailOp = ToD3D11StencilOp(ccwFail);
					dsd.BackFace.StencilDepthFailOp = ToD3D11StencilOp(ccwZFail);
					dsd.BackFace.StencilPassOp = ToD3D11StencilOp(ccwPass);
				}
				else
				{
					dsd.BackFace = dsd.FrontFace;
				}

				ComPtr<ID3D11DepthStencilState> dss;
				m_device->CreateDepthStencilState(&dsd, dss.GetAddressOf());
				it = m_depthStencilStates.emplace(h, dss).first;
			}
			m_context->OMSetDepthStencilState(it->second.Get(), stRef);
			m_stateTracker.ClearDepthStencilDirty();
		}

		// 4. Dirty sampler states
		uint32_t dirtySamplers = m_stateTracker.GetDirtySamplerMask();
		if (dirtySamplers != 0)
		{
			for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
			{
				if ((dirtySamplers & (1u << s)) == 0)
					continue;

				const uint32_t addrU = m_stateTracker.GetSamplerState(s, ESamplerStateType::ADDRESSU);
				const uint32_t addrV = m_stateTracker.GetSamplerState(s, ESamplerStateType::ADDRESSV);
				const uint32_t addrW = m_stateTracker.GetSamplerState(s, ESamplerStateType::ADDRESSW);
				const uint32_t minF  = m_stateTracker.GetSamplerState(s, ESamplerStateType::MINFILTER);
				const uint32_t magF  = m_stateTracker.GetSamplerState(s, ESamplerStateType::MAGFILTER);
				const uint32_t mipF  = m_stateTracker.GetSamplerState(s, ESamplerStateType::MIPFILTER);
				const uint32_t aniso = m_stateTracker.GetSamplerState(s, ESamplerStateType::MAXANISOTROPY);
				const uint32_t border = m_stateTracker.GetSamplerState(s, ESamplerStateType::BORDERCOLOR);
				const uint32_t biasBits = m_stateTracker.GetSamplerState(s, ESamplerStateType::MIPMAPLODBIAS);

				uint64_t h = 14695981039346656037ULL;
				HashCombineU32(h, addrU | (addrV << 8) | (addrW << 16));
				HashCombineU32(h, minF | (magF << 8) | (mipF << 16));
				HashCombineU32(h, aniso);
				HashCombineU32(h, border);
				HashCombineU32(h, biasBits);

				auto it = m_samplerStates.find(h);
				if (it == m_samplerStates.end())
				{
					D3D11_SAMPLER_DESC sd = {};
					sd.AddressU = ToD3D11AddressMode(addrU, m_featureLevel);
					sd.AddressV = ToD3D11AddressMode(addrV, m_featureLevel);
					sd.AddressW = ToD3D11AddressMode(addrW, m_featureLevel);

					if (minF == 3 || magF == 3 || aniso > 1)
					{
						sd.Filter = D3D11_FILTER_ANISOTROPIC;
					}
					else
					{
						const bool minLin = (minF != 1);
						const bool magLin = (magF != 1);
						const bool mipLin = (mipF != 1 && mipF != 0);
						if (!minLin && !magLin && !mipLin) sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
						else if (!minLin && !magLin && mipLin) sd.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
						else if (!minLin && magLin && !mipLin) sd.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
						else if (!minLin && magLin && mipLin)  sd.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
						else if (minLin && !magLin && !mipLin) sd.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
						else if (minLin && !magLin && mipLin)  sd.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
						else if (minLin && magLin && !mipLin)  sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
						else                                   sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					}

					float lodBias = 0.0f;
					std::memcpy(&lodBias, &biasBits, sizeof(float));
					sd.MipLODBias = lodBias;
					sd.MaxAnisotropy = (std::min)((std::max)(aniso, 1u), m_caps.maxAnisotropy);
					sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
					sd.BorderColor[0] = static_cast<float>((border >> 16) & 0xFFu) / 255.0f;
					sd.BorderColor[1] = static_cast<float>((border >> 8) & 0xFFu) / 255.0f;
					sd.BorderColor[2] = static_cast<float>(border & 0xFFu) / 255.0f;
					sd.BorderColor[3] = static_cast<float>((border >> 24) & 0xFFu) / 255.0f;
					sd.MinLOD = 0.0f;
					sd.MaxLOD = (mipF == 0) ? 0.0f : D3D11_FLOAT32_MAX;

					ComPtr<ID3D11SamplerState> ss;
					m_device->CreateSamplerState(&sd, ss.GetAddressOf());
					it = m_samplerStates.emplace(h, ss).first;
				}

				ID3D11SamplerState* ssPtr = it->second.Get();
				m_context->PSSetSamplers(s, 1, &ssPtr);
				if (m_featureLevel >= D3D_FEATURE_LEVEL_10_0)
				{
					m_context->VSSetSamplers(s, 1, &ssPtr);
				}
				m_stateTracker.ClearSamplerDirty(s);
			}
		}

		// 5. Shader resource views (mapped from engine sampler stage sN -> shader t-register slot, filtering RTV/DSV hazards)
		if (m_srvBindingsDirty && m_currentProgram)
		{
			ID3D11Resource* activeTargetRes[5] = {};
			uint32_t activeTargetCount = 0;
			for (uint32_t i = 0; i < m_activeRTVCount && i < 4; ++i)
			{
				if (m_activeRTVs[i])
				{
					m_activeRTVs[i]->GetResource(&activeTargetRes[activeTargetCount]);
					if (activeTargetRes[activeTargetCount])
						++activeTargetCount;
				}
			}
			if (m_activeDSV)
			{
				m_activeDSV->GetResource(&activeTargetRes[activeTargetCount]);
				if (activeTargetRes[activeTargetCount])
					++activeTargetCount;
			}

			auto filterConflictingSRV = [&](ID3D11ShaderResourceView* candidate) -> ID3D11ShaderResourceView*
			{
				if (!candidate || activeTargetCount == 0)
					return candidate;
				ComPtr<ID3D11Resource> srvRes;
				candidate->GetResource(srvRes.GetAddressOf());
				for (uint32_t i = 0; i < activeTargetCount; ++i)
				{
					if (srvRes.Get() == activeTargetRes[i])
						return nullptr;
				}
				return candidate;
			};

			for (uint32_t t = 0; t < RHIStateTracker::MAX_SAMPLER_STAGES; ++t)
			{
				uint8_t psStage = m_currentProgram->MapPSTexSlot(t);
				ID3D11ShaderResourceView* wantPS = (psStage < RHIStateTracker::MAX_SAMPLER_STAGES)
					? filterConflictingSRV(m_logicalSRVs[psStage])
					: nullptr;
				if (m_boundPSSRVs[t] != wantPS)
				{
					m_boundPSSRVs[t] = wantPS;
					m_context->PSSetShaderResources(t, 1, &wantPS);
				}

				if (m_featureLevel >= D3D_FEATURE_LEVEL_10_0)
				{
					uint8_t vsStage = m_currentProgram->MapVSTexSlot(t);
					ID3D11ShaderResourceView* wantVS = (vsStage < RHIStateTracker::MAX_SAMPLER_STAGES)
						? filterConflictingSRV(m_logicalSRVs[vsStage])
						: nullptr;
					if (m_boundVSSRVs[t] != wantVS)
					{
						m_boundVSSRVs[t] = wantVS;
						m_context->VSSetShaderResources(t, 1, &wantVS);
					}
				}
			}

			for (uint32_t i = 0; i < activeTargetCount; ++i)
			{
				if (activeTargetRes[i])
					activeTargetRes[i]->Release();
			}
			m_srvBindingsDirty = false;
		}

		// 6. Input layout + fallback default attribute slot 15
		if (m_currentVertexDecl && m_currentProgram)
		{
			ID3D11InputLayout* layout = GetOrCreateInputLayout(m_currentVertexDecl, m_currentProgram);
			m_context->IASetInputLayout(layout);
			if (m_featureLevel >= D3D_FEATURE_LEVEL_10_0 && m_defaultAttrVB)
			{
				ID3D11Buffer* defVB = m_defaultAttrVB.Get();
				UINT stride = sizeof(float) * 4;
				UINT offset = 0;
				m_context->IASetVertexBuffers(15, 1, &defVB, &stride, &offset);
			}
		}
	}

	bool RenderDeviceD3D11::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		if (!m_context || PrimitiveCount == 0)
			return false;

		D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uint32_t vertCount = 0;
		if (!ComputePrimitiveTopologyAndCount(PrimitiveType, PrimitiveCount, topo, vertCount))
			return false;

		ApplyPipelineStatesBeforeDraw();
		m_context->IASetPrimitiveTopology(topo);

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			uint32_t idxBytes = PrimitiveCount * 3u * sizeof(uint16_t);
			ID3D11Buffer* ib = EnsureDynamicScratchBuffer(m_upIndexBuffer, m_upIndexCapacity, idxBytes, D3D11_BIND_INDEX_BUFFER);
			if (!ib)
				return false;
			D3D11_MAPPED_SUBRESOURCE mapped = {};
			if (FAILED(m_context->Map(ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) || !mapped.pData)
				return false;
			uint16_t* dst = static_cast<uint16_t*>(mapped.pData);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				dst[i * 3u + 0] = 0;
				dst[i * 3u + 1] = static_cast<uint16_t>(i + 1u);
				dst[i * 3u + 2] = static_cast<uint16_t>(i + 2u);
			}
			m_context->Unmap(ib, 0);
			m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);
			m_currentIndexBuffer = nullptr;
			m_context->DrawIndexed(PrimitiveCount * 3u, 0, static_cast<INT>(StartVertex));
			return true;
		}

		m_context->Draw(vertCount, StartVertex);
		return true;
	}

	bool RenderDeviceD3D11::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t /*MinIndex*/, uint32_t /*NumVertices*/, uint32_t indexStart, uint32_t PrimitiveCount)
	{
		if (!m_context || PrimitiveCount == 0)
			return false;

		if ((m_streams[0].frequencyDivider & 0x40000000u) != 0)
		{
			uint32_t instanceCount = m_streams[0].frequencyDivider & 0x3FFFFFFFu;
			if (instanceCount > 0)
				return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, instanceCount);
		}

		D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uint32_t indexCount = 0;
		if (!ComputePrimitiveTopologyAndCount(Type, PrimitiveCount, topo, indexCount))
			return false;

		ApplyPipelineStatesBeforeDraw();
		m_context->IASetPrimitiveTopology(topo);
		m_context->DrawIndexed(indexCount, indexStart, BaseVertexIndex);
		return true;
	}

	bool RenderDeviceD3D11::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		if (!m_context || !pVertexStreamZeroData || VertexStreamZeroStride == 0 || PrimitiveCount == 0)
			return false;

		D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uint32_t vertCount = 0;
		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			vertCount = PrimitiveCount + 2u;
		}
		else if (!ComputePrimitiveTopologyAndCount(PrimitiveType, PrimitiveCount, topo, vertCount))
		{
			return false;
		}

		uint32_t byteCount = vertCount * VertexStreamZeroStride;
		ID3D11Buffer* vb = EnsureDynamicScratchBuffer(m_upVertexBuffer, m_upVertexCapacity, byteCount, D3D11_BIND_VERTEX_BUFFER);
		if (!vb)
			return false;

		D3D11_MAP mapMode = D3D11_MAP_WRITE_NO_OVERWRITE;
		uint32_t alignedOffset = ((m_upVertexOffset + VertexStreamZeroStride - 1u) / VertexStreamZeroStride) * VertexStreamZeroStride;
		if (alignedOffset + byteCount > m_upVertexCapacity)
		{
			mapMode = D3D11_MAP_WRITE_DISCARD;
			alignedOffset = 0;
		}

		D3D11_MAPPED_SUBRESOURCE mapped = {};
		if (FAILED(m_context->Map(vb, 0, mapMode, 0, &mapped)) || !mapped.pData)
			return false;
		std::memcpy(static_cast<uint8_t*>(mapped.pData) + alignedOffset, pVertexStreamZeroData, byteCount);
		m_context->Unmap(vb, 0);
		m_upVertexOffset = alignedOffset + byteCount;

		UINT stride = VertexStreamZeroStride;
		UINT offset = alignedOffset;
		m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		m_streams[0].buffer = nullptr;

		ApplyPipelineStatesBeforeDraw();
		m_context->IASetPrimitiveTopology(topo);

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			uint32_t idxBytes = PrimitiveCount * 3u * sizeof(uint16_t);
			ID3D11Buffer* ib = EnsureDynamicScratchBuffer(m_upIndexBuffer, m_upIndexCapacity, idxBytes, D3D11_BIND_INDEX_BUFFER);
			if (!ib)
				return false;
			D3D11_MAPPED_SUBRESOURCE ibMap = {};
			if (FAILED(m_context->Map(ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &ibMap)) || !ibMap.pData)
				return false;
			uint16_t* dst = static_cast<uint16_t*>(ibMap.pData);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				dst[i * 3u + 0] = 0;
				dst[i * 3u + 1] = static_cast<uint16_t>(i + 1u);
				dst[i * 3u + 2] = static_cast<uint16_t>(i + 2u);
			}
			m_context->Unmap(ib, 0);
			m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);
			m_currentIndexBuffer = nullptr;
			m_context->DrawIndexed(PrimitiveCount * 3u, 0, 0);
			return true;
		}

		m_context->Draw(vertCount, 0);
		return true;
	}

	bool RenderDeviceD3D11::DrawIndexedPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t MinVertexIndex,
		uint32_t NumVertices,
		uint32_t PrimitiveCount,
		const void* pIndexData,
		PixelFormat IndexDataFormat,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!m_context || !pIndexData || !pVertexStreamZeroData || VertexStreamZeroStride == 0 || PrimitiveCount == 0)
			return false;

		D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uint32_t indexCount = 0;
		if (!ComputePrimitiveTopologyAndCount(PrimitiveType, PrimitiveCount, topo, indexCount))
			return false;

		const bool isIdx32 = (IndexDataFormat == PixelFormat::INDEX32);
		const uint32_t idxElemBytes = isIdx32 ? 4u : 2u;
		const uint32_t totalVerts = MinVertexIndex + NumVertices;
		const uint32_t vbBytes = totalVerts * VertexStreamZeroStride;

		ID3D11Buffer* vb = EnsureDynamicScratchBuffer(m_upVertexBuffer, m_upVertexCapacity, vbBytes, D3D11_BIND_VERTEX_BUFFER);
		if (!vb)
			return false;

		D3D11_MAP vbMapMode = D3D11_MAP_WRITE_NO_OVERWRITE;
		uint32_t vbAlignedOffset = ((m_upVertexOffset + VertexStreamZeroStride - 1u) / VertexStreamZeroStride) * VertexStreamZeroStride;
		if (vbAlignedOffset + vbBytes > m_upVertexCapacity)
		{
			vbMapMode = D3D11_MAP_WRITE_DISCARD;
			vbAlignedOffset = 0;
		}

		D3D11_MAPPED_SUBRESOURCE vbMapped = {};
		if (FAILED(m_context->Map(vb, 0, vbMapMode, 0, &vbMapped)) || !vbMapped.pData)
			return false;
		std::memcpy(static_cast<uint8_t*>(vbMapped.pData) + vbAlignedOffset, pVertexStreamZeroData, vbBytes);
		m_context->Unmap(vb, 0);
		m_upVertexOffset = vbAlignedOffset + vbBytes;

		const uint32_t ibBytes = indexCount * idxElemBytes;
		ID3D11Buffer* ib = EnsureDynamicScratchBuffer(m_upIndexBuffer, m_upIndexCapacity, ibBytes, D3D11_BIND_INDEX_BUFFER);
		if (!ib)
			return false;

		D3D11_MAP ibMapMode = D3D11_MAP_WRITE_NO_OVERWRITE;
		uint32_t ibAlignedOffset = ((m_upIndexOffset + idxElemBytes - 1u) / idxElemBytes) * idxElemBytes;
		if (ibAlignedOffset + ibBytes > m_upIndexCapacity)
		{
			ibMapMode = D3D11_MAP_WRITE_DISCARD;
			ibAlignedOffset = 0;
		}

		D3D11_MAPPED_SUBRESOURCE ibMapped = {};
		if (FAILED(m_context->Map(ib, 0, ibMapMode, 0, &ibMapped)) || !ibMapped.pData)
			return false;

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			if (isIdx32)
			{
				const uint32_t* src = static_cast<const uint32_t*>(pIndexData);
				uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(ibMapped.pData) + ibAlignedOffset);
				for (uint32_t i = 0; i < PrimitiveCount; ++i)
				{
					dst[i * 3u + 0] = src[0];
					dst[i * 3u + 1] = src[i + 1u];
					dst[i * 3u + 2] = src[i + 2u];
				}
			}
			else
			{
				const uint16_t* src = static_cast<const uint16_t*>(pIndexData);
				uint16_t* dst = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(ibMapped.pData) + ibAlignedOffset);
				for (uint32_t i = 0; i < PrimitiveCount; ++i)
				{
					dst[i * 3u + 0] = src[0];
					dst[i * 3u + 1] = src[i + 1u];
					dst[i * 3u + 2] = src[i + 2u];
				}
			}
		}
		else
		{
			std::memcpy(static_cast<uint8_t*>(ibMapped.pData) + ibAlignedOffset, pIndexData, ibBytes);
		}
		m_context->Unmap(ib, 0);
		m_upIndexOffset = ibAlignedOffset + ibBytes;

		UINT stride = VertexStreamZeroStride;
		UINT offset = vbAlignedOffset;
		m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		m_context->IASetIndexBuffer(ib, isIdx32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, ibAlignedOffset);
		m_streams[0].buffer = nullptr;
		m_currentIndexBuffer = nullptr;

		ApplyPipelineStatesBeforeDraw();
		m_context->IASetPrimitiveTopology(topo);
		m_context->DrawIndexed(indexCount, 0, 0);
		return true;
	}

	bool RenderDeviceD3D11::SetTransform(ETransformsStateType /*State*/, DeviceMatrix_ptr /*pMatrix*/)
	{
		return true;
	}

	bool RenderDeviceD3D11::SetFVF(uint32_t FVF)
	{
		if (FVF == 0)
			return true;

		auto it = m_fvfDecls.find(FVF);
		if (it == m_fvfDecls.end())
		{
			std::vector<VertexElement> elems;
			uint16_t offset = 0;
			uint32_t posMask = FVF & D3DFVF_POSITION_MASK;
			if (posMask == D3DFVF_XYZ)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 });
				offset += 12;
			}
			else if (posMask == D3DFVF_XYZRHW || posMask == D3DFVF_XYZW)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 });
				offset += 16;
			}
			if (FVF & D3DFVF_NORMAL)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 });
				offset += 12;
			}
			if (FVF & D3DFVF_PSIZE)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_PSIZE, 0 });
				offset += 4;
			}
			if (FVF & D3DFVF_DIFFUSE)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 });
				offset += 4;
			}
			if (FVF & D3DFVF_SPECULAR)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1 });
				offset += 4;
			}
			uint32_t texCount = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
			for (uint32_t t = 0; t < texCount && t < 8; ++t)
			{
				elems.push_back({ 0, offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, static_cast<uint8_t>(t) });
				offset += 8;
			}
			elems.push_back(D3DDECL_END());
			auto* decl = new CVertexDeclaration(elems.data());
			it = m_fvfDecls.emplace(FVF, decl).first;
		}
		return SetVertexDeclaration(it->second);
	}

	void RenderDeviceD3D11::SetCursorPosition(int /*X*/, int /*Y*/, uint32_t /*Flags*/)
	{
	}

	bool RenderDeviceD3D11::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		m_stateTracker.SetSamplerState(stage, type, value);
		return true;
	}

	bool RenderDeviceD3D11::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		if (value)
			*value = m_stateTracker.GetSamplerState(stage, type);
		return true;
	}

	bool RenderDeviceD3D11::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
	{
		m_currentVertexDecl = pVertexDeclaration;
		return true;
	}

	bool RenderDeviceD3D11::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
	{
		if (!ppDecl)
			return false;
		*ppDecl = new CVertexDeclaration(pVertexElements);
		return true;
	}

	bool RenderDeviceD3D11::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		if (!m_context)
			return false;
		if (m_currentIndexBuffer != pIndexData)
		{
			m_currentIndexBuffer = pIndexData;
			ID3D11Buffer* ib = pIndexData ? static_cast<ID3D11Buffer*>(pIndexData->GetNativeHandle()) : nullptr;
			m_context->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);
		}
		return true;
	}

	bool RenderDeviceD3D11::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
	{
		if (!m_context || StreamNumber >= 4)
			return false;

		auto& sb = m_streams[StreamNumber];
		sb.buffer = pStreamData;
		sb.offsetInBytes = OffsetInBytes;
		sb.stride = Stride;

		ID3D11Buffer* vb = pStreamData ? static_cast<ID3D11Buffer*>(pStreamData->GetNativeHandle()) : nullptr;
		UINT stride = Stride;
		UINT offset = OffsetInBytes;
		m_context->IASetVertexBuffers(StreamNumber, 1, &vb, &stride, &offset);
		return true;
	}

	ParaViewport RenderDeviceD3D11::GetViewport()
	{
		return m_currentViewport;
	}

	bool RenderDeviceD3D11::SetViewport(const ParaViewport& viewport)
	{
		m_currentViewport = viewport;
		m_stateTracker.SetViewport(viewport);
		if (!m_context)
			return false;

		D3D11_VIEWPORT vp = {};
		vp.TopLeftX = static_cast<FLOAT>(viewport.X);
		vp.TopLeftY = static_cast<FLOAT>(viewport.Y);
		vp.Width = static_cast<FLOAT>(viewport.Width);
		vp.Height = static_cast<FLOAT>(viewport.Height);
		vp.MinDepth = viewport.MinZ;
		vp.MaxDepth = viewport.MaxZ;
		m_context->RSSetViewports(1, &vp);
		return true;
	}

	bool RenderDeviceD3D11::SetClearColor(const Color4f& color)
	{
		m_stateTracker.SetClearColor(color);
		return true;
	}

	bool RenderDeviceD3D11::SetClearDepth(const float depth)
	{
		m_stateTracker.SetClearDepth(depth);
		return true;
	}

	bool RenderDeviceD3D11::SetClearStencil(const int stencil)
	{
		m_stateTracker.SetClearStencil(stencil);
		return true;
	}

	bool RenderDeviceD3D11::Clear(bool color, bool depth, bool stencil)
	{
		if (!m_context)
			return false;

		if (color)
		{
			const Color4f& c = m_stateTracker.GetClearColor();
			const FLOAT rgba[4] = { c.r, c.g, c.b, c.a };
			for (uint32_t i = 0; i < m_activeRTVCount && i < 4; ++i)
			{
				if (m_activeRTVs[i])
					m_context->ClearRenderTargetView(m_activeRTVs[i], rgba);
			}
		}

		if ((depth || stencil) && m_activeDSV)
		{
			UINT clearFlags = (depth ? D3D11_CLEAR_DEPTH : 0u) | (stencil ? D3D11_CLEAR_STENCIL : 0u);
			m_context->ClearDepthStencilView(m_activeDSV, clearFlags, m_stateTracker.GetClearDepth(), static_cast<UINT8>(m_stateTracker.GetClearStencil() & 0xFF));
		}

		return true;
	}

	bool RenderDeviceD3D11::SetScissorRect(RECT* pRect)
	{
		m_stateTracker.SetScissorRect(pRect);
		if (!m_context)
			return false;

		if (pRect)
		{
			D3D11_RECT r = { pRect->left, pRect->top, pRect->right, pRect->bottom };
			m_context->RSSetScissorRects(1, &r);
		}
		else
		{
			D3D11_RECT r = { 0, 0, static_cast<LONG>(m_renderTargetWidth), static_cast<LONG>(m_renderTargetHeight) };
			m_context->RSSetScissorRects(1, &r);
		}
		return true;
	}

	bool RenderDeviceD3D11::GetScissorRect(RECT* pRect)
	{
		if (pRect)
			*pRect = m_stateTracker.GetScissorRect();
		return true;
	}

	bool RenderDeviceD3D11::BeginScene()
	{
		auto* pApp = CGlobals::GetApp();
		auto* pWin = pApp ? pApp->GetRenderWindow() : nullptr;
		if (pWin)
		{
			uint32_t winW = pWin->GetWidth();
			uint32_t winH = pWin->GetHeight();
			if (winW > 0 && winH > 0 && (winW != m_backBufferWidth || winH != m_backBufferHeight))
			{
				ResizeSwapChain(winW, winH, true);
			}
		}

		while (!m_rtSizeStack.empty())
			m_rtSizeStack.pop();

		if (m_context)
		{
			m_activeRTVs[0] = m_defaultRTV.Get();
			m_activeRTVs[1] = nullptr;
			m_activeRTVs[2] = nullptr;
			m_activeRTVs[3] = nullptr;
			m_activeRTVCount = 1;
			m_activeDSV = m_defaultDSV.Get();
			m_activeColorTex0 = m_backBufferTex.Get();
			m_activeDepthTex = m_defaultDepthTex.Get();
			UnbindConflictingSRVs(m_activeRTVs, 1, m_activeDSV);
			m_context->OMSetRenderTargets(1, m_activeRTVs, m_activeDSV);
			m_srvBindingsDirty = true;
		}
		return true;
	}

	bool RenderDeviceD3D11::EndScene()
	{
		return true;
	}

	bool RenderDeviceD3D11::Present()
	{
		if (!m_swapChain)
			return false;
		HRESULT hr = m_swapChain->Present(m_vsyncEnabled ? 1 : 0, 0);
		return SUCCEEDED(hr);
	}

	void RenderDeviceD3D11::Flush()
	{
		if (m_context)
			m_context->Flush();
	}
}

#include "ParaEngine.h"
#include "renderer/RenderDevice.h"
#include "RenderDeviceD3D12.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "RHISoftwareS3TC.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>
#include <dxcapi.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>

using Microsoft::WRL::ComPtr;

namespace ParaEngine
{
	namespace
	{
		using PFN_D3DCompileDyn = HRESULT(WINAPI*)(
			LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
		using PFN_D3DReflectDyn = HRESULT(WINAPI*)(
			LPCVOID, SIZE_T, REFIID, void**);

		struct D3D12DeviceLibLoader
		{
			HMODULE hD3D12 = nullptr;
			HMODULE hCompiler = nullptr;
			PFN_D3D12_SERIALIZE_ROOT_SIGNATURE pfnSerializeRootSignature = nullptr;
			PFN_D3DCompileDyn pfnCompile = nullptr;
			PFN_D3DReflectDyn pfnReflect = nullptr;

			D3D12DeviceLibLoader()
			{
				hD3D12 = ::LoadLibraryW(L"d3d12.dll");
				if (hD3D12)
				{
					pfnSerializeRootSignature = reinterpret_cast<PFN_D3D12_SERIALIZE_ROOT_SIGNATURE>(
						::GetProcAddress(hD3D12, "D3D12SerializeRootSignature"));
				}
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

		const D3D12DeviceLibLoader& GetD3D12Libs()
		{
			static D3D12DeviceLibLoader s_libs;
			return s_libs;
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

		inline uint64_t AlignUpU64(uint64_t val, uint64_t alignment)
		{
			return (alignment <= 1) ? val : ((val + alignment - 1ULL) & ~(alignment - 1ULL));
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

		DXGI_FORMAT MapColorPixelFormatToDXGI(PixelFormat fmt, uint32_t& outBytesPerBlock, bool& outIsBlockCompressed)
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
				outBytesPerBlock = 16;
				outIsBlockCompressed = true;
				return DXGI_FORMAT_BC7_UNORM;
			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		bool ComputePrimitiveTopologyAndCount(
			EPrimitiveType primType,
			uint32_t primCount,
			D3D12_PRIMITIVE_TOPOLOGY& outTopo,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE& outTopoType,
			uint32_t& outVertexOrIndexCount)
		{
			if (primCount == 0)
				return false;
			switch (primType)
			{
			case EPrimitiveType::POINTLIST:
				outTopo = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
				outTopoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
				outVertexOrIndexCount = primCount;
				return true;
			case EPrimitiveType::LINELIST:
				outTopo = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
				outTopoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				outVertexOrIndexCount = primCount * 2u;
				return true;
			case EPrimitiveType::LINESTRIP:
				outTopo = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
				outTopoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				outVertexOrIndexCount = primCount + 1u;
				return true;
			case EPrimitiveType::TRIANGLELIST:
				outTopo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				outTopoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				outVertexOrIndexCount = primCount * 3u;
				return true;
			case EPrimitiveType::TRIANGLESTRIP:
				outTopo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
				outTopoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				outVertexOrIndexCount = primCount + 2u;
				return true;
			case EPrimitiveType::TRIANGLEFAN:
				outTopo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				outTopoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				outVertexOrIndexCount = primCount * 3u;
				return true;
			default:
				return false;
			}
		}

		D3D12_COMPARISON_FUNC ToD3D12CmpFunc(uint32_t rsvCmp)
		{
			switch (rsvCmp)
			{
			case RSV_CMP_NEVER:        return D3D12_COMPARISON_FUNC_NEVER;
			case RSV_CMP_LESS:         return D3D12_COMPARISON_FUNC_LESS;
			case RSV_CMP_EQUAL:        return D3D12_COMPARISON_FUNC_EQUAL;
			case RSV_CMP_LESSEQUAL:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case RSV_CMP_GREATER:      return D3D12_COMPARISON_FUNC_GREATER;
			case RSV_CMP_NOTEQUAL:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case RSV_CMP_GREATEREQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case RSV_CMP_ALWAYS:
			default:                   return D3D12_COMPARISON_FUNC_ALWAYS;
			}
		}

		D3D12_BLEND ToD3D12Blend(uint32_t rsvBlend, bool isAlphaChannel)
		{
			switch (rsvBlend)
			{
			case RSV_BLEND_ZERO:           return D3D12_BLEND_ZERO;
			case RSV_BLEND_ONE:            return D3D12_BLEND_ONE;
			case RSV_BLEND_SRCCOLOR:       return isAlphaChannel ? D3D12_BLEND_SRC_ALPHA : D3D12_BLEND_SRC_COLOR;
			case RSV_BLEND_INVSRCCOLOR:    return isAlphaChannel ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_INV_SRC_COLOR;
			case RSV_BLEND_SRCALPHA:       return D3D12_BLEND_SRC_ALPHA;
			case RSV_BLEND_INVSRCALPHA:    return D3D12_BLEND_INV_SRC_ALPHA;
			case RSV_BLEND_DESTALPHA:      return D3D12_BLEND_DEST_ALPHA;
			case RSV_BLEND_INVDESTALPHA:   return D3D12_BLEND_INV_DEST_ALPHA;
			case RSV_BLEND_DESTCOLOR:      return isAlphaChannel ? D3D12_BLEND_DEST_ALPHA : D3D12_BLEND_DEST_COLOR;
			case RSV_BLEND_INVDESTCOLOR:   return isAlphaChannel ? D3D12_BLEND_INV_DEST_ALPHA : D3D12_BLEND_INV_DEST_COLOR;
			case RSV_BLEND_SRCALPHASAT:    return D3D12_BLEND_SRC_ALPHA_SAT;
			case RSV_BLEND_BLENDFACTOR:    return D3D12_BLEND_BLEND_FACTOR;
			case RSV_BLEND_INVBLENDFACTOR: return D3D12_BLEND_INV_BLEND_FACTOR;
			default:                       return D3D12_BLEND_ONE;
			}
		}

		D3D12_BLEND_OP ToD3D12BlendOp(uint32_t op)
		{
			switch (op)
			{
			case 2:  return D3D12_BLEND_OP_SUBTRACT;
			case 3:  return D3D12_BLEND_OP_REV_SUBTRACT;
			case 4:  return D3D12_BLEND_OP_MIN;
			case 5:  return D3D12_BLEND_OP_MAX;
			case 1:
			default: return D3D12_BLEND_OP_ADD;
			}
		}

		D3D12_STENCIL_OP ToD3D12StencilOp(uint32_t op)
		{
			switch (op)
			{
			case RSV_STENCILOP_ZERO:    return D3D12_STENCIL_OP_ZERO;
			case RSV_STENCILOP_REPLACE: return D3D12_STENCIL_OP_REPLACE;
			case RSV_STENCILOP_INCRSAT: return D3D12_STENCIL_OP_INCR_SAT;
			case RSV_STENCILOP_DECRSAT: return D3D12_STENCIL_OP_DECR_SAT;
			case RSV_STENCILOP_INVERT:  return D3D12_STENCIL_OP_INVERT;
			case RSV_STENCILOP_INCR:    return D3D12_STENCIL_OP_INCR;
			case RSV_STENCILOP_DECR:    return D3D12_STENCIL_OP_DECR;
			case RSV_STENCILOP_KEEP:
			default:                    return D3D12_STENCIL_OP_KEEP;
			}
		}

		D3D12_TEXTURE_ADDRESS_MODE ToD3D12AddressMode(uint32_t mode)
		{
			switch (mode)
			{
			case 2: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			case 3: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			case 4: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			case 5: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			case 1:
			default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			}
		}

		DXGI_FORMAT MapDeclTypeToDXGI12(uint8_t declType)
		{
			switch (declType)
			{
			case D3DDECLTYPE_FLOAT1:    return DXGI_FORMAT_R32_FLOAT;
			case D3DDECLTYPE_FLOAT2:    return DXGI_FORMAT_R32G32_FLOAT;
			case D3DDECLTYPE_FLOAT3:    return DXGI_FORMAT_R32G32B32_FLOAT;
			case D3DDECLTYPE_FLOAT4:    return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case D3DDECLTYPE_D3DCOLOR:  return DXGI_FORMAT_B8G8R8A8_UNORM;
			case D3DDECLTYPE_UBYTE4:    return DXGI_FORMAT_R8G8B8A8_UINT;
			case D3DDECLTYPE_SHORT2:    return DXGI_FORMAT_R16G16_SINT;
			case D3DDECLTYPE_SHORT4:    return DXGI_FORMAT_R16G16B16A16_SINT;
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

		const char* MapDeclUsageToSemantic12(uint8_t usage, bool useSVPosition)
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
	// D3D12CpuDescriptorAllocator
	// =========================================================================

	bool D3D12CpuDescriptorAllocator::Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxDescriptors)
	{
		if (!device || maxDescriptors == 0)
			return false;

		D3D12_DESCRIPTOR_HEAP_DESC hd = {};
		hd.Type = type;
		hd.NumDescriptors = maxDescriptors;
		hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hd.NodeMask = 0;

		if (FAILED(device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&m_heap))) || !m_heap)
			return false;

		m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
		m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		m_capacity = maxDescriptors;
		m_nextLinear = 0;
		m_freeList.clear();
		return true;
	}

	uint32_t D3D12CpuDescriptorAllocator::Allocate()
	{
		if (!m_freeList.empty())
		{
			uint32_t idx = m_freeList.back();
			m_freeList.pop_back();
			return idx;
		}
		if (m_nextLinear < m_capacity)
		{
			return m_nextLinear++;
		}
		return UINT32_MAX;
	}

	void D3D12CpuDescriptorAllocator::Free(uint32_t index)
	{
		if (index < m_capacity)
		{
			m_freeList.push_back(index);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12CpuDescriptorAllocator::GetHandle(uint32_t index) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE h = m_cpuStart;
		if (index < m_capacity)
		{
			h.ptr += static_cast<SIZE_T>(index) * static_cast<SIZE_T>(m_descriptorSize);
		}
		return h;
	}

	// =========================================================================
	// D3D12 Resource Implementations
	// =========================================================================

	class D3D12Texture : public IRHITexture
	{
	public:
		D3D12Texture(
			RenderDeviceD3D12* owner,
			uint32_t width,
			uint32_t height,
			PixelFormat sourceFormat,
			DXGI_FORMAT dxgiFormat,
			uint32_t mipLevels,
			bool expandedToRGBA8,
			ID3D12Resource* resource,
			D3D12_RESOURCE_STATES initialState,
			uint32_t srvSlot)
			: m_owner(owner)
			, m_width(width)
			, m_height(height)
			, m_mipLevels(mipLevels)
			, m_sourceFormat(sourceFormat)
			, m_dxgiFormat(dxgiFormat)
			, m_expandedToRGBA8(expandedToRGBA8)
			, m_resource(resource)
			, m_state(initialState)
			, m_srvSlot(srvSlot)
		{
		}

		virtual ~D3D12Texture() override
		{
			if (m_owner)
			{
				m_owner->NotifyTextureDestroyed(this);
				if (m_srvSlot != UINT32_MAX)
				{
					m_owner->GetSrvAllocator().Free(m_srvSlot);
					m_srvSlot = UINT32_MAX;
				}
				if (m_resource)
				{
					m_owner->DeferRelease(m_resource.Get());
				}
			}
		}

		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual uint32_t GetMipLevels() const override { return m_mipLevels; }
		virtual PixelFormat GetFormat() const override { return m_sourceFormat; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<D3D12Texture*>(this)); }

		ID3D12Resource* GetResource() const { return m_resource.Get(); }
		DXGI_FORMAT GetDXGIFormat() const { return m_dxgiFormat; }
		D3D12_RESOURCE_STATES& GetStateRef() { return m_state; }
		D3D12_RESOURCE_STATES GetState() const { return m_state; }
		uint32_t GetSrvSlot() const { return m_srvSlot; }

		virtual bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t rowPitch) override
		{
			if (!m_owner || !m_resource || !data || width == 0 || height == 0 || mipLevel >= m_mipLevels)
				return false;

			uint32_t mipW = (std::max)(1u, m_width >> mipLevel);
			uint32_t mipH = (std::max)(1u, m_height >> mipLevel);
			if (x < 0 || y < 0 || static_cast<uint32_t>(x) + width > mipW || static_cast<uint32_t>(y) + height > mipH)
				return false;

			std::vector<uint8_t> expanded;
			const uint8_t* srcBytes = static_cast<const uint8_t*>(data);
			uint32_t srcRowBytes = 0;
			uint32_t numRows = height;
			uint32_t srcStride = rowPitch;

			if (m_expandedToRGBA8)
			{
				if (!RHISoftwareS3TC::ExpandToRGBA8(m_sourceFormat, data, width, height, rowPitch, expanded))
					return false;
				srcBytes = expanded.data();
				srcRowBytes = width * 4u;
				srcStride = srcRowBytes;
			}
			else if (RHISoftwareS3TC::IsS3TCFormat(m_sourceFormat) &&
				(m_dxgiFormat == DXGI_FORMAT_BC1_UNORM || m_dxgiFormat == DXGI_FORMAT_BC2_UNORM || m_dxgiFormat == DXGI_FORMAT_BC3_UNORM))
			{
				uint32_t blockBytes = (m_sourceFormat == PixelFormat::DXT1) ? 8u : 16u;
				uint32_t blocksX = (std::max)(1u, (width + 3u) / 4u);
				numRows = (std::max)(1u, (height + 3u) / 4u);
				srcRowBytes = blocksX * blockBytes;
				srcStride = rowPitch ? rowPitch : srcRowBytes;
			}
			else if (RHISoftwareS3TC::IsS3TCFormat(m_sourceFormat))
			{
				size_t compBytes = RHISoftwareS3TC::ComputeCompressedMipSize(m_sourceFormat, width, height);
				if (!RHISoftwareS3TC::DecodeToRGBA8(m_sourceFormat, data, compBytes, width, height, expanded))
					return false;
				srcBytes = expanded.data();
				srcRowBytes = width * 4u;
				srcStride = srcRowBytes;
			}
			else
			{
				uint32_t bpp = 4u;
				if (m_dxgiFormat == DXGI_FORMAT_A8_UNORM || m_dxgiFormat == DXGI_FORMAT_R8_UNORM) bpp = 1u;
				else if (m_dxgiFormat == DXGI_FORMAT_R16_FLOAT) bpp = 2u;
				else if (m_dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT || m_dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM || m_dxgiFormat == DXGI_FORMAT_R32G32_FLOAT) bpp = 8u;
				else if (m_dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) bpp = 16u;
				srcRowBytes = width * bpp;
				srcStride = rowPitch ? rowPitch : srcRowBytes;
			}

			uint32_t dstRowPitch = static_cast<uint32_t>(AlignUpU64(srcRowBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			uint32_t uploadBytes = (numRows > 1) ? ((numRows - 1u) * dstRowPitch + srcRowBytes) : srcRowBytes;
			uploadBytes = static_cast<uint32_t>(AlignUpU64(uploadBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));

			void* uploadCpu = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS uploadGpuVA = 0;
			ID3D12Resource* uploadRes = nullptr;
			uint64_t uploadOffset = 0;
			if (!m_owner->AllocateUpload(uploadBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, uploadCpu, uploadGpuVA, uploadRes, uploadOffset))
				return false;

			uint8_t* dstBase = static_cast<uint8_t*>(uploadCpu);
			for (uint32_t r = 0; r < numRows; ++r)
			{
				std::memcpy(dstBase + static_cast<size_t>(r) * dstRowPitch, srcBytes + static_cast<size_t>(r) * srcStride, srcRowBytes);
			}

			ID3D12GraphicsCommandList* cmd = m_owner->EnsureRecording();
			if (!cmd)
				return false;

			D3D12_RESOURCE_STATES prevState = m_state;
			m_owner->TransitionResource(m_resource.Get(), m_state, D3D12_RESOURCE_STATE_COPY_DEST);

			UINT subIdx = mipLevel + arraySlice * m_mipLevels;
			D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
			dstLoc.pResource = m_resource.Get();
			dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstLoc.SubresourceIndex = subIdx;

			D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
			srcLoc.pResource = uploadRes;
			srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLoc.PlacedFootprint.Offset = uploadOffset;
			srcLoc.PlacedFootprint.Footprint.Format = m_dxgiFormat;
			srcLoc.PlacedFootprint.Footprint.Width = width;
			srcLoc.PlacedFootprint.Footprint.Height = height;
			srcLoc.PlacedFootprint.Footprint.Depth = 1;
			srcLoc.PlacedFootprint.Footprint.RowPitch = dstRowPitch;

			D3D12_BOX srcBox = { 0, 0, 0, width, height, 1 };
			cmd->CopyTextureRegion(&dstLoc, static_cast<UINT>(x), static_cast<UINT>(y), 0, &srcLoc, &srcBox);

			D3D12_RESOURCE_STATES targetAfter = (prevState == D3D12_RESOURCE_STATE_COPY_DEST)
				? (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
				: prevState;
			m_owner->TransitionResource(m_resource.Get(), m_state, targetAfter);
			return true;
		}

	private:
		RenderDeviceD3D12* m_owner;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_mipLevels;
		PixelFormat m_sourceFormat;
		DXGI_FORMAT m_dxgiFormat;
		bool m_expandedToRGBA8;
		ComPtr<ID3D12Resource> m_resource;
		D3D12_RESOURCE_STATES m_state;
		uint32_t m_srvSlot;
	};

	class D3D12Buffer : public IRHIBuffer
	{
	public:
		D3D12Buffer(
			RenderDeviceD3D12* owner,
			uint32_t byteSize,
			bool isIndexBuffer,
			ERHIBufferUsage usage,
			const void* initialData)
			: m_owner(owner)
			, m_byteSize(byteSize)
			, m_allocSize((std::max)(byteSize, 16u))
			, m_isIndexBuffer(isIndexBuffer)
			, m_isDynamic(HasFlag(usage, ERHIBufferUsage::Dynamic) || HasFlag(usage, ERHIBufferUsage::Stream))
			, m_gpuVA(0)
			, m_state(D3D12_RESOURCE_STATE_COMMON)
			, m_mappedDirect(false)
			, m_mappedShadow(false)
			, m_mapOffset(0)
			, m_mapSize(0)
			, m_mapMode(ERHIMapMode::Write)
		{
			ID3D12Device* dev = owner ? owner->GetD3D12Device() : nullptr;
			if (!dev)
				return;

			if (m_isDynamic)
			{
				// Persistent upload buffer for WriteNoOverwrite / initial data, plus per-discard ring slices for WriteDiscard
				D3D12_HEAP_PROPERTIES hp = {};
				hp.Type = D3D12_HEAP_TYPE_UPLOAD;
				D3D12_RESOURCE_DESC rd = {};
				rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				rd.Width = m_allocSize;
				rd.Height = 1;
				rd.DepthOrArraySize = 1;
				rd.MipLevels = 1;
				rd.Format = DXGI_FORMAT_UNKNOWN;
				rd.SampleDesc.Count = 1;
				rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				if (SUCCEEDED(dev->CreateCommittedResource(
					&hp,
					D3D12_HEAP_FLAG_NONE,
					&rd,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&m_resource))) && m_resource)
				{
					m_state = D3D12_RESOURCE_STATE_GENERIC_READ;
					D3D12_RANGE readRange = { 0, 0 };
					void* ptr = nullptr;
					if (SUCCEEDED(m_resource->Map(0, &readRange, &ptr)) && ptr)
					{
						m_persistentCpuPtr = static_cast<uint8_t*>(ptr);
						if (initialData && byteSize > 0)
						{
							std::memcpy(m_persistentCpuPtr, initialData, byteSize);
						}
					}
					m_gpuVA = m_resource->GetGPUVirtualAddress();
				}
			}
			else
			{
				D3D12_HEAP_PROPERTIES hp = {};
				hp.Type = D3D12_HEAP_TYPE_DEFAULT;
				D3D12_RESOURCE_DESC rd = {};
				rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				rd.Width = m_allocSize;
				rd.Height = 1;
				rd.DepthOrArraySize = 1;
				rd.MipLevels = 1;
				rd.Format = DXGI_FORMAT_UNKNOWN;
				rd.SampleDesc.Count = 1;
				rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				if (SUCCEEDED(dev->CreateCommittedResource(
					&hp,
					D3D12_HEAP_FLAG_NONE,
					&rd,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&m_resource))) && m_resource)
				{
					m_gpuVA = m_resource->GetGPUVirtualAddress();
					m_state = D3D12_RESOURCE_STATE_COMMON;
					if (initialData && byteSize > 0)
					{
						if (!HasFlag(usage, ERHIBufferUsage::WriteOnly))
						{
							m_cpuShadow.resize(byteSize);
							std::memcpy(m_cpuShadow.data(), initialData, byteSize);
						}
						UploadRegionToDefaultBuffer(0, byteSize, initialData);
					}
				}
			}
		}

		virtual ~D3D12Buffer() override
		{
			if (m_owner)
			{
				m_owner->NotifyBufferDestroyed(this);
			}
			if (m_resource)
			{
				if (m_persistentCpuPtr)
				{
					m_resource->Unmap(0, nullptr);
					m_persistentCpuPtr = nullptr;
				}
				if (m_owner)
				{
					m_owner->DeferRelease(m_resource.Get());
				}
			}
		}

		virtual uint32_t GetSize() const override { return m_byteSize; }
		virtual bool IsIndexBuffer() const override { return m_isIndexBuffer; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<D3D12Buffer*>(this)); }
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVA() const { return m_gpuVA; }

		virtual void* Map(uint32_t offset, uint32_t size, ERHIMapMode mode) override
		{
			if (!m_owner || offset >= m_byteSize)
				return nullptr;

			uint32_t actualSize = (size == 0 || offset + size > m_byteSize) ? (m_byteSize - offset) : size;
			m_mapOffset = offset;
			m_mapSize = actualSize;
			m_mapMode = mode;

			if (m_isDynamic && mode == ERHIMapMode::WriteDiscard && offset == 0)
			{
				// Allocate a fresh ring-buffer upload slice so prior draws in the same command list are never overwritten!
				void* sliceCpu = nullptr;
				D3D12_GPU_VIRTUAL_ADDRESS sliceGpu = 0;
				ID3D12Resource* sliceRes = nullptr;
				uint64_t sliceOff = 0;
				if (m_owner->AllocateUpload(m_allocSize, 16, sliceCpu, sliceGpu, sliceRes, sliceOff) && sliceCpu)
				{
					m_gpuVA = sliceGpu;
					m_activeDynamicCpuBase = static_cast<uint8_t*>(sliceCpu);
					m_mappedDirect = true;
					return m_activeDynamicCpuBase;
				}
			}

			if (m_isDynamic && mode == ERHIMapMode::WriteNoOverwrite)
			{
				if (m_activeDynamicCpuBase)
				{
					m_mappedDirect = true;
					return m_activeDynamicCpuBase + offset;
				}
				if (m_persistentCpuPtr)
				{
					m_gpuVA = m_resource->GetGPUVirtualAddress();
					m_mappedDirect = true;
					return m_persistentCpuPtr + offset;
				}
			}

			if (m_cpuShadow.size() < m_byteSize)
				m_cpuShadow.resize(m_byteSize, 0);
			m_mappedShadow = true;
			return m_cpuShadow.data() + offset;
		}

		virtual void Unmap() override
		{
			if (m_mappedDirect)
			{
				m_mappedDirect = false;
				return;
			}
			if (!m_mappedShadow)
				return;
			m_mappedShadow = false;
			if (m_mapMode == ERHIMapMode::Read || m_mapSize == 0)
				return;

			if (m_isDynamic)
			{
				void* sliceCpu = nullptr;
				D3D12_GPU_VIRTUAL_ADDRESS sliceGpu = 0;
				ID3D12Resource* sliceRes = nullptr;
				uint64_t sliceOff = 0;
				if (m_owner->AllocateUpload(m_allocSize, 16, sliceCpu, sliceGpu, sliceRes, sliceOff) && sliceCpu)
				{
					std::memcpy(sliceCpu, m_cpuShadow.data(), m_byteSize);
					m_gpuVA = sliceGpu;
					m_activeDynamicCpuBase = static_cast<uint8_t*>(sliceCpu);
				}
			}
			else
			{
				UploadRegionToDefaultBuffer(m_mapOffset, m_mapSize, m_cpuShadow.data() + m_mapOffset);
			}
		}

		virtual bool UpdateData(uint32_t offset, uint32_t size, const void* data) override
		{
			if (!data || size == 0 || offset + size > m_byteSize)
				return false;

			if (m_isDynamic && offset == 0 && size == m_byteSize)
			{
				void* dst = Map(0, size, ERHIMapMode::WriteDiscard);
				if (!dst)
					return false;
				std::memcpy(dst, data, size);
				Unmap();
				return true;
			}

			if (!m_isDynamic)
			{
				if (!m_cpuShadow.empty() && m_cpuShadow.size() >= offset + size)
				{
					std::memcpy(m_cpuShadow.data() + offset, data, size);
				}
				return UploadRegionToDefaultBuffer(offset, size, data);
			}

			void* dst = Map(offset, size, ERHIMapMode::Write);
			if (!dst)
				return false;
			std::memcpy(dst, data, size);
			Unmap();
			return true;
		}

	private:
		bool UploadRegionToDefaultBuffer(uint32_t offset, uint32_t size, const void* data)
		{
			if (!m_owner || !m_resource || !data || size == 0)
				return false;

			void* upCpu = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS upGpu = 0;
			ID3D12Resource* upRes = nullptr;
			uint64_t upOff = 0;
			if (!m_owner->AllocateUpload(size, 16, upCpu, upGpu, upRes, upOff))
				return false;

			std::memcpy(upCpu, data, size);
			ID3D12GraphicsCommandList* cmd = m_owner->EnsureRecording();
			if (!cmd)
				return false;

			m_owner->TransitionResource(m_resource.Get(), m_state, D3D12_RESOURCE_STATE_COPY_DEST);
			cmd->CopyBufferRegion(m_resource.Get(), offset, upRes, upOff, size);
			D3D12_RESOURCE_STATES readState = m_isIndexBuffer
				? D3D12_RESOURCE_STATE_INDEX_BUFFER
				: D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			m_owner->TransitionResource(m_resource.Get(), m_state, readState);
			return true;
		}

	private:
		RenderDeviceD3D12* m_owner;
		ComPtr<ID3D12Resource> m_resource;
		uint8_t* m_persistentCpuPtr = nullptr;
		uint8_t* m_activeDynamicCpuBase = nullptr;
		std::vector<uint8_t> m_cpuShadow;
		uint32_t m_byteSize;
		uint32_t m_allocSize;
		bool m_isIndexBuffer;
		bool m_isDynamic;
		D3D12_GPU_VIRTUAL_ADDRESS m_gpuVA;
		D3D12_RESOURCE_STATES m_state;
		bool m_mappedDirect;
		bool m_mappedShadow;
		uint32_t m_mapOffset;
		uint32_t m_mapSize;
		ERHIMapMode m_mapMode;
	};

	class D3D12RenderTarget : public IRHIRenderTarget
	{
	public:
		D3D12RenderTarget(
			RenderDeviceD3D12* owner,
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
			, m_sampleCount(1)
			, m_dsvFormat(DXGI_FORMAT_UNKNOWN)
			, m_valid(false)
		{
			ID3D12Device* dev = owner ? owner->GetD3D12Device() : nullptr;
			if (!dev || width == 0 || height == 0)
				return;

			uint32_t bpp = 4;
			bool compressed = false;
			DXGI_FORMAT dxgiColorFmt = MapColorPixelFormatToDXGI(colorFormat, bpp, compressed);
			if (dxgiColorFmt == DXGI_FORMAT_UNKNOWN || compressed)
				dxgiColorFmt = DXGI_FORMAT_R8G8B8A8_UNORM;
			m_dxgiColorFmt = dxgiColorFmt;

			if (msaaSamples > 1)
			{
				D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msLevels = {};
				msLevels.Format = dxgiColorFmt;
				msLevels.SampleCount = msaaSamples;
				if (SUCCEEDED(dev->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msLevels, sizeof(msLevels))) &&
					msLevels.NumQualityLevels > 0)
				{
					m_sampleCount = msaaSamples;
				}
			}

			D3D12_HEAP_PROPERTIES defaultHeap = {};
			defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

			for (uint32_t i = 0; i < m_colorCount; ++i)
			{
				D3D12_RESOURCE_DESC td = {};
				td.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				td.Width = width;
				td.Height = height;
				td.DepthOrArraySize = 1;
				td.MipLevels = 1;
				td.Format = dxgiColorFmt;
				td.SampleDesc.Count = m_sampleCount;
				td.SampleDesc.Quality = 0;
				td.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				td.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

				D3D12_CLEAR_VALUE rtClear = {};
				rtClear.Format = dxgiColorFmt;
				rtClear.Color[0] = 0.0f;
				rtClear.Color[1] = 0.0f;
				rtClear.Color[2] = 0.0f;
				rtClear.Color[3] = 0.0f;

				ComPtr<ID3D12Resource> colorRes;
				if (FAILED(dev->CreateCommittedResource(
					&defaultHeap,
					D3D12_HEAP_FLAG_NONE,
					&td,
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&rtClear,
					IID_PPV_ARGS(&colorRes))) || !colorRes)
				{
					return;
				}

				m_rtvSlots[i] = owner->GetRtvAllocator().Allocate();
				if (m_rtvSlots[i] == UINT32_MAX)
					return;
				dev->CreateRenderTargetView(colorRes.Get(), nullptr, owner->GetRtvAllocator().GetHandle(m_rtvSlots[i]));

				uint32_t srvSlot = UINT32_MAX;
				if (m_sampleCount == 1)
				{
					srvSlot = owner->GetSrvAllocator().Allocate();
					if (srvSlot != UINT32_MAX)
					{
						D3D12_SHADER_RESOURCE_VIEW_DESC sd = {};
						sd.Format = dxgiColorFmt;
						sd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						sd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						sd.Texture2D.MipLevels = 1;
						dev->CreateShaderResourceView(colorRes.Get(), &sd, owner->GetSrvAllocator().GetHandle(srvSlot));
					}
				}

				m_colorTextures[i] = new D3D12Texture(
					owner,
					width,
					height,
					colorFormat,
					dxgiColorFmt,
					1,
					false,
					colorRes.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					srvSlot);
			}

			if (depthFormat != PixelFormat::Unknown && IsDepthPixelFormat(depthFormat))
			{
				const bool canSampleDepth = (m_sampleCount == 1);
				DXGI_FORMAT resFmt = canSampleDepth ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
				DXGI_FORMAT dsvFmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
				DXGI_FORMAT srvFmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

				if (depthFormat == PixelFormat::D16 || depthFormat == PixelFormat::D16_LOCKABLE)
				{
					resFmt = canSampleDepth ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_D16_UNORM;
					dsvFmt = DXGI_FORMAT_D16_UNORM;
					srvFmt = DXGI_FORMAT_R16_UNORM;
				}
				else if (depthFormat == PixelFormat::D32 || depthFormat == PixelFormat::D32F_LOCKABLE)
				{
					resFmt = canSampleDepth ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D32_FLOAT;
					dsvFmt = DXGI_FORMAT_D32_FLOAT;
					srvFmt = DXGI_FORMAT_R32_FLOAT;
				}

				D3D12_RESOURCE_DESC dd = {};
				dd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				dd.Width = width;
				dd.Height = height;
				dd.DepthOrArraySize = 1;
				dd.MipLevels = 1;
				dd.Format = resFmt;
				dd.SampleDesc.Count = m_sampleCount;
				dd.SampleDesc.Quality = 0;
				dd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				dd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | (canSampleDepth ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

				D3D12_CLEAR_VALUE dsClear = {};
				dsClear.Format = dsvFmt;
				dsClear.DepthStencil.Depth = 1.0f;
				dsClear.DepthStencil.Stencil = 0;

				ComPtr<ID3D12Resource> depthRes;
				if (SUCCEEDED(dev->CreateCommittedResource(
					&defaultHeap,
					D3D12_HEAP_FLAG_NONE,
					&dd,
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&dsClear,
					IID_PPV_ARGS(&depthRes))) && depthRes)
				{
					m_dsvSlot = owner->GetDsvAllocator().Allocate();
					if (m_dsvSlot != UINT32_MAX)
					{
						D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = {};
						dsvd.Format = dsvFmt;
						dsvd.ViewDimension = (m_sampleCount > 1) ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
						dev->CreateDepthStencilView(depthRes.Get(), &dsvd, owner->GetDsvAllocator().GetHandle(m_dsvSlot));
						m_dsvFormat = dsvFmt;
					}

					uint32_t depthSrvSlot = UINT32_MAX;
					if (canSampleDepth)
					{
						depthSrvSlot = owner->GetSrvAllocator().Allocate();
						if (depthSrvSlot != UINT32_MAX)
						{
							D3D12_SHADER_RESOURCE_VIEW_DESC sd = {};
							sd.Format = srvFmt;
							sd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							sd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
							sd.Texture2D.MipLevels = 1;
							dev->CreateShaderResourceView(depthRes.Get(), &sd, owner->GetSrvAllocator().GetHandle(depthSrvSlot));
						}
					}

					m_depthTexture = new D3D12Texture(
						owner,
						width,
						height,
						depthFormat,
						dsvFmt,
						1,
						false,
						depthRes.Get(),
						D3D12_RESOURCE_STATE_DEPTH_WRITE,
						depthSrvSlot);
				}
			}

			m_valid = true;
		}

		virtual ~D3D12RenderTarget() override
		{
			if (m_owner)
			{
				m_owner->NotifyRenderTargetDestroyed(this);
				for (uint32_t i = 0; i < 4; ++i)
				{
					if (m_rtvSlots[i] != UINT32_MAX)
					{
						m_owner->GetRtvAllocator().Free(m_rtvSlots[i]);
						m_rtvSlots[i] = UINT32_MAX;
					}
				}
				if (m_dsvSlot != UINT32_MAX)
				{
					m_owner->GetDsvAllocator().Free(m_dsvSlot);
					m_dsvSlot = UINT32_MAX;
				}
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
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<D3D12RenderTarget*>(this)); }

		uint32_t GetColorCount() const { return m_colorCount; }
		uint32_t GetSampleCount() const { return m_sampleCount; }
		DXGI_FORMAT GetDxgiColorFormat() const { return m_dxgiColorFmt; }
		DXGI_FORMAT GetDxgiDepthFormat() const { return m_dsvFormat; }
		bool HasDSV() const { return m_dsvSlot != UINT32_MAX; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(uint32_t index = 0) const
		{
			if (m_owner && index < m_colorCount && m_rtvSlots[index] != UINT32_MAX)
				return m_owner->GetRtvAllocator().GetHandle(m_rtvSlots[index]);
			return D3D12_CPU_DESCRIPTOR_HANDLE{};
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const
		{
			if (m_owner && m_dsvSlot != UINT32_MAX)
				return m_owner->GetDsvAllocator().GetHandle(m_dsvSlot);
			return D3D12_CPU_DESCRIPTOR_HANDLE{};
		}

		D3D12Texture* GetD3D12ColorTex(uint32_t index = 0) const { return (index < m_colorCount) ? m_colorTextures[index] : nullptr; }
		D3D12Texture* GetD3D12DepthTex() const { return m_depthTexture; }

	private:
		RenderDeviceD3D12* m_owner;
		uint32_t m_width;
		uint32_t m_height;
		PixelFormat m_colorFormat;
		DXGI_FORMAT m_dxgiColorFmt = DXGI_FORMAT_R8G8B8A8_UNORM;
		uint32_t m_colorCount;
		uint32_t m_sampleCount;
		DXGI_FORMAT m_dsvFormat;
		bool m_valid;
		D3D12Texture* m_colorTextures[4] = {};
		D3D12Texture* m_depthTexture = nullptr;
		uint32_t m_rtvSlots[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		uint32_t m_dsvSlot = UINT32_MAX;
	};

	class D3D12OcclusionQuery : public IRHIOcclusionQuery
	{
	public:
		explicit D3D12OcclusionQuery(RenderDeviceD3D12* owner)
			: m_owner(owner)
			, m_slot(owner ? owner->AllocateOcclusionQuerySlot() : UINT32_MAX)
			, m_active(false)
			, m_resolved(false)
		{
		}

		virtual ~D3D12OcclusionQuery() override
		{
			if (m_owner && m_slot != UINT32_MAX)
			{
				m_owner->FreeOcclusionQuerySlot(m_slot);
				m_slot = UINT32_MAX;
			}
		}

		virtual bool Begin() override
		{
			if (!m_owner || m_slot == UINT32_MAX || !m_owner->GetOcclusionQueryHeap())
				return false;
			ID3D12GraphicsCommandList* cmd = m_owner->EnsureRecording();
			if (!cmd)
				return false;
			cmd->BeginQuery(m_owner->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_OCCLUSION, m_slot);
			m_active = true;
			m_resolved = false;
			return true;
		}

		virtual void End() override
		{
			if (!m_owner || m_slot == UINT32_MAX || !m_active || !m_owner->GetOcclusionQueryHeap())
				return;
			ID3D12GraphicsCommandList* cmd = m_owner->EnsureRecording();
			if (cmd)
			{
				cmd->EndQuery(m_owner->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_OCCLUSION, m_slot);
				if (m_owner->GetOcclusionReadbackBuffer())
				{
					cmd->ResolveQueryData(
						m_owner->GetOcclusionQueryHeap(),
						D3D12_QUERY_TYPE_OCCLUSION,
						m_slot,
						1,
						m_owner->GetOcclusionReadbackBuffer(),
						static_cast<UINT64>(m_slot) * sizeof(uint64_t));
					m_resolved = true;
				}
			}
			m_active = false;
		}

		virtual bool GetResult(uint64_t& outSamplesPassed, bool flush) override
		{
			outSamplesPassed = 0;
			if (!m_owner || m_slot == UINT32_MAX || !m_resolved)
				return false;

			if (flush)
			{
				m_owner->ExecuteAndSyncGPU();
			}

			const uint64_t* results = m_owner->GetOcclusionMappedResults();
			if (!results)
				return false;

			outSamplesPassed = results[m_slot];
			return true;
		}

	private:
		RenderDeviceD3D12* m_owner;
		uint32_t m_slot;
		bool m_active;
		bool m_resolved;
	};

	class D3D12ShaderProgram : public IRHIShaderProgram
	{
	public:
		struct InputSigParam
		{
			std::string semanticName;
			uint32_t semanticIndex = 0;
			D3D_REGISTER_COMPONENT_TYPE componentType = D3D_REGISTER_COMPONENT_FLOAT32;
		};

		D3D12ShaderProgram(RenderDeviceD3D12* owner, const RHIShaderProgramDesc& desc)
			: m_owner(owner)
			, m_valid(false)
			, m_vsHash(0)
			, m_psHash(0)
			, m_usesSVPosition(false)
			, m_psOutputMask(0xFFu)
		{
			if (!desc.vsBytecode || desc.vsBytecodeSize == 0)
				return;

			m_vsBytecode.resize(desc.vsBytecodeSize);
			std::memcpy(m_vsBytecode.data(), desc.vsBytecode, desc.vsBytecodeSize);
			m_vsHash = ComputeByteHash64(desc.vsBytecode, desc.vsBytecodeSize);

			if (desc.psBytecode && desc.psBytecodeSize > 0)
			{
				m_psBytecode.resize(desc.psBytecodeSize);
				std::memcpy(m_psBytecode.data(), desc.psBytecode, desc.psBytecodeSize);
				m_psHash = ComputeByteHash64(desc.psBytecode, desc.psBytecodeSize);
				ReflectPSOutputs();
			}
			else
			{
				m_psOutputMask = 0;
			}

			ReflectVSInputs();
			m_valid = true;
		}

		virtual ~D3D12ShaderProgram() override
		{
			if (m_owner)
			{
				m_owner->NotifyProgramDestroyed(this);
			}
		}

		virtual bool IsValid() const override { return m_valid; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<D3D12ShaderProgram*>(this)); }

		const std::vector<uint8_t>& GetVSBytecode() const { return m_vsBytecode; }
		const std::vector<uint8_t>& GetPSBytecode() const { return m_psBytecode; }
		uint64_t GetVSHash() const { return m_vsHash; }
		uint64_t GetPSHash() const { return m_psHash; }
		bool UsesSVPosition() const { return m_usesSVPosition; }
		uint8_t GetPSOutputMask() const { return m_psOutputMask; }
		const std::vector<InputSigParam>& GetInputSignature() const { return m_inputParams; }

	private:
		void AddReflectedInputParam(const char* semName, uint32_t semIdx, uint32_t sysVal, uint32_t compType)
		{
			if (!semName || !semName[0])
				return;
			if (sysVal != 0 &&
				_stricmp(semName, "SV_Position") != 0 &&
				_stricmp(semName, "POSITION") != 0)
			{
				return; // Skip SV_InstanceID / SV_VertexID
			}
			if (_stricmp(semName, "SV_Position") == 0 && semIdx == 0)
			{
				m_usesSVPosition = true;
			}
			InputSigParam p;
			p.semanticName = semName;
			p.semanticIndex = semIdx;
			p.componentType = static_cast<D3D_REGISTER_COMPONENT_TYPE>(compType);
			m_inputParams.push_back(std::move(p));
		}

		bool ParseDXBCInputSignatureChunk()
		{
			// Both SM4/5 DXBC and SM6 DXIL containers share the 32-byte 'DXBC' outer container with 'ISGN' / 'ISG1' chunks!
			if (m_vsBytecode.size() < 36 || std::memcmp(m_vsBytecode.data(), "DXBC", 4) != 0)
				return false;

			uint32_t chunkCount = 0;
			std::memcpy(&chunkCount, m_vsBytecode.data() + 28, sizeof(uint32_t));
			if (32ULL + static_cast<uint64_t>(chunkCount) * 4ULL > m_vsBytecode.size())
				return false;

			for (uint32_t c = 0; c < chunkCount; ++c)
			{
				uint32_t chunkOffset = 0;
				std::memcpy(&chunkOffset, m_vsBytecode.data() + 32 + c * 4, sizeof(uint32_t));
				if (static_cast<size_t>(chunkOffset) + 16 > m_vsBytecode.size())
					continue;

				const uint8_t* hdr = m_vsBytecode.data() + chunkOffset;
				const bool isISG1 = (std::memcmp(hdr, "ISG1", 4) == 0);
				const bool isISGN = (std::memcmp(hdr, "ISGN", 4) == 0);
				if (!isISG1 && !isISGN)
					continue;

				uint32_t chunkSize = 0;
				std::memcpy(&chunkSize, hdr + 4, sizeof(uint32_t));
				const uint8_t* data = hdr + 8;
				if (static_cast<size_t>(chunkOffset) + 8 + chunkSize > m_vsBytecode.size() || chunkSize < 8)
					return false;

				uint32_t elemCount = 0;
				std::memcpy(&elemCount, data, sizeof(uint32_t));
				const size_t elemStride = isISG1 ? 32u : 24u;
				if (8ULL + static_cast<uint64_t>(elemCount) * elemStride > chunkSize)
					return false;

				for (uint32_t i = 0; i < elemCount; ++i)
				{
					const uint8_t* e = data + 8 + i * elemStride;
					uint32_t nameOffset = 0, semIndex = 0, sysValue = 0, compType = 0;
					if (isISG1)
					{
						// ISG1: [0:Stream, 4:NameOffset, 8:SemanticIndex, 12:SystemValue, 16:ComponentType, 20:Register, 24:Mask, 28:MinPrec]
						std::memcpy(&nameOffset, e + 4, 4);
						std::memcpy(&semIndex, e + 8, 4);
						std::memcpy(&sysValue, e + 12, 4);
						std::memcpy(&compType, e + 16, 4);
					}
					else
					{
						// ISGN: [0:NameOffset, 4:SemanticIndex, 8:SystemValue, 12:ComponentType, 16:Register, 20:Mask]
						std::memcpy(&nameOffset, e + 0, 4);
						std::memcpy(&semIndex, e + 4, 4);
						std::memcpy(&sysValue, e + 8, 4);
						std::memcpy(&compType, e + 12, 4);
					}
					if (nameOffset < chunkSize)
					{
						const char* semName = reinterpret_cast<const char*>(data + nameOffset);
						AddReflectedInputParam(semName, semIndex, sysValue, compType);
					}
				}
				return true;
			}
			return false;
		}

		void ReflectVSInputs()
		{
			// 1. Direct binary ISG1/ISGN chunk parse works instantaneously on BOTH DXIL SM6.0 and DXBC SM5.0 with zero DLL dependency!
			if (ParseDXBCInputSignatureChunk())
				return;

			// 2. Fallback to D3DReflect (SM4/SM5 DXBC)
			const auto& libs = GetD3D12Libs();
			ComPtr<ID3D12ShaderReflection> refl12;
			if (libs.pfnReflect && SUCCEEDED(libs.pfnReflect(m_vsBytecode.data(), m_vsBytecode.size(), IID_PPV_ARGS(&refl12))) && refl12)
			{
				D3D12_SHADER_DESC sd = {};
				if (SUCCEEDED(refl12->GetDesc(&sd)))
				{
					for (UINT i = 0; i < sd.InputParameters; ++i)
					{
						D3D12_SIGNATURE_PARAMETER_DESC pd = {};
						if (SUCCEEDED(refl12->GetInputParameterDesc(i, &pd)) && pd.SemanticName)
						{
							AddReflectedInputParam(pd.SemanticName, pd.SemanticIndex, static_cast<uint32_t>(pd.SystemValueType), static_cast<uint32_t>(pd.ComponentType));
						}
					}
				}
			}
		}

		bool ParseDXBCOutputSignatureChunk(uint8_t& outMask) const
		{
			if (m_psBytecode.size() < 36 || std::memcmp(m_psBytecode.data(), "DXBC", 4) != 0)
				return false;

			uint32_t chunkCount = 0;
			std::memcpy(&chunkCount, m_psBytecode.data() + 28, sizeof(uint32_t));
			if (32ULL + static_cast<uint64_t>(chunkCount) * 4ULL > m_psBytecode.size())
				return false;

			for (uint32_t c = 0; c < chunkCount; ++c)
			{
				uint32_t chunkOffset = 0;
				std::memcpy(&chunkOffset, m_psBytecode.data() + 32 + c * 4, sizeof(uint32_t));
				if (static_cast<size_t>(chunkOffset) + 16 > m_psBytecode.size())
					continue;

				const uint8_t* hdr = m_psBytecode.data() + chunkOffset;
				const bool isOSG1 = (std::memcmp(hdr, "OSG1", 4) == 0 || std::memcmp(hdr, "OSG5", 4) == 0);
				const bool isOSGN = (std::memcmp(hdr, "OSGN", 4) == 0);
				if (!isOSG1 && !isOSGN)
					continue;

				uint32_t chunkSize = 0;
				std::memcpy(&chunkSize, hdr + 4, sizeof(uint32_t));
				const uint8_t* data = hdr + 8;
				if (static_cast<size_t>(chunkOffset) + 8 + chunkSize > m_psBytecode.size() || chunkSize < 8)
					return false;

				uint32_t elemCount = 0;
				std::memcpy(&elemCount, data, sizeof(uint32_t));
				const size_t elemStride = isOSG1 ? 32u : 24u;
				if (8ULL + static_cast<uint64_t>(elemCount) * elemStride > chunkSize)
					return false;

				outMask = 0;
				for (uint32_t i = 0; i < elemCount; ++i)
				{
					const uint8_t* e = data + 8 + i * elemStride;
					uint32_t nameOffset = 0, semIndex = 0, sysValue = 0;
					if (isOSG1)
					{
						std::memcpy(&nameOffset, e + 4, 4);
						std::memcpy(&semIndex, e + 8, 4);
						std::memcpy(&sysValue, e + 12, 4);
					}
					else
					{
						std::memcpy(&nameOffset, e + 0, 4);
						std::memcpy(&semIndex, e + 4, 4);
						std::memcpy(&sysValue, e + 8, 4);
					}
					if (nameOffset < chunkSize)
					{
						const char* semName = reinterpret_cast<const char*>(data + nameOffset);
						if ((sysValue == D3D_NAME_TARGET ||
							 _stricmp(semName, "SV_Target") == 0 ||
							 _stricmp(semName, "COLOR") == 0) &&
							semIndex < 8)
						{
							outMask |= static_cast<uint8_t>(1u << semIndex);
						}
					}
				}
				return true;
			}
			return false;
		}

		void ReflectPSOutputs()
		{
			uint8_t mask = 0;
			if (ParseDXBCOutputSignatureChunk(mask))
			{
				m_psOutputMask = mask;
				return;
			}

			const auto& libs = GetD3D12Libs();
			ComPtr<ID3D12ShaderReflection> refl12;
			if (libs.pfnReflect && SUCCEEDED(libs.pfnReflect(m_psBytecode.data(), m_psBytecode.size(), IID_PPV_ARGS(&refl12))) && refl12)
			{
				D3D12_SHADER_DESC sd = {};
				if (SUCCEEDED(refl12->GetDesc(&sd)))
				{
					mask = 0;
					for (UINT i = 0; i < sd.OutputParameters; ++i)
					{
						D3D12_SIGNATURE_PARAMETER_DESC pd = {};
						if (SUCCEEDED(refl12->GetOutputParameterDesc(i, &pd)) && pd.SemanticName)
						{
							if ((pd.SystemValueType == D3D_NAME_TARGET ||
								 _stricmp(pd.SemanticName, "SV_Target") == 0 ||
								 _stricmp(pd.SemanticName, "COLOR") == 0) &&
								pd.SemanticIndex < 8)
							{
								mask |= static_cast<uint8_t>(1u << pd.SemanticIndex);
							}
						}
					}
					m_psOutputMask = mask;
				}
			}
		}

	private:
		RenderDeviceD3D12* m_owner = nullptr;
		bool m_valid;
		uint64_t m_vsHash;
		uint64_t m_psHash;
		bool m_usesSVPosition;
		uint8_t m_psOutputMask;
		std::vector<uint8_t> m_vsBytecode;
		std::vector<uint8_t> m_psBytecode;
		std::vector<InputSigParam> m_inputParams;
	};

	// =========================================================================
	// RenderDeviceD3D12 Core & Frame Lifecycle
	// =========================================================================

	RenderDeviceD3D12::RenderDeviceD3D12(
		ID3D12Device* device,
		ID3D12CommandQueue* commandQueue,
		IDXGISwapChain3* swapChain,
		D3D_FEATURE_LEVEL featureLevel,
		bool isWarp,
		const std::string& adapterName)
		: m_device(device)
		, m_cmdQueue(commandQueue)
		, m_swapChain(swapChain)
		, m_featureLevel(featureLevel)
		, m_isWarp(isWarp)
		, m_adapterName(adapterName)
	{
		InitCaps();
	}

	RenderDeviceD3D12::~RenderDeviceD3D12()
	{
		m_isShuttingDown = true;
		ExecuteAndSyncGPU();

		if (m_blitProgram)
		{
			m_blitProgram->ReleaseRHIResource();
			m_blitProgram = nullptr;
		}
		if (m_blitVertexDecl)
		{
			m_blitVertexDecl->Release();
			m_blitVertexDecl = nullptr;
		}
		if (m_whiteFallbackTex)
		{
			m_whiteFallbackTex->ReleaseRHIResource();
			m_whiteFallbackTex = nullptr;
		}

		for (auto& kv : m_fvfDecls)
		{
			if (kv.second)
				kv.second->Release();
		}
		m_fvfDecls.clear();

		if (m_occlusionReadback && m_occlusionMappedPtr)
		{
			m_occlusionReadback->Unmap(0, nullptr);
			m_occlusionMappedPtr = nullptr;
		}

		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
		{
			for (auto& page : m_uploadPages[f])
			{
				if (page.resource && page.cpuBase)
				{
					page.resource->Unmap(0, nullptr);
					page.cpuBase = nullptr;
				}
			}
			m_uploadPages[f].clear();
		}

		ReleaseBackBufferViews();
		m_deferredReleases.clear();

		if (m_fenceEvent)
		{
			::CloseHandle(m_fenceEvent);
			m_fenceEvent = nullptr;
		}
	}

	bool RenderDeviceD3D12::BindReflectionInputs(IRHITexture* depth, IRHITexture* normal, IRHITexture* material, IRHITexture* sceneColor, const PlayerReflectionMesh* player)
	{
		constexpr UINT descriptorCount = 4 + PlayerReflectionMesh::MaxTextures;
		if (m_gpuSrvOffset + descriptorCount > m_gpuSrvFrameBase[m_frameIndex] + 32768u)
			ExecuteAndSyncGPU();
		auto* cmd = EnsureRecording();
		if (!cmd) return false;
		const UINT base = m_gpuSrvOffset;
		auto dest = m_gpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
		dest.ptr += SIZE_T(base) * m_gpuSrvDescriptorSize;
		IRHITexture* inputs[descriptorCount] = { depth, normal, material, sceneColor };
		for (UINT i = 0; i < PlayerReflectionMesh::MaxTextures; ++i)
			inputs[4 + i] = player && i < player->textures.size() ? player->textures[i] : m_whiteFallbackTex;
		for (auto* input : inputs)
		{
			auto* texture = input ? static_cast<D3D12Texture*>(input->GetNativeHandle()) : nullptr;
			if (!texture || texture->GetSrvSlot() == UINT32_MAX) return false;
			TransitionResource(texture->GetResource(), texture->GetStateRef(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_device->CopyDescriptorsSimple(1, dest, m_cpuSrvAlloc.GetHandle(texture->GetSrvSlot()), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			dest.ptr += m_gpuSrvDescriptorSize;
		}
		m_gpuSrvOffset += descriptorCount;
		auto table = m_gpuSrvHeap->GetGPUDescriptorHandleForHeapStart();
		table.ptr += UINT64(base) * m_gpuSrvDescriptorSize;
		cmd->SetComputeRootSignature(m_reflectionRoot.Get());
		cmd->SetComputeRootDescriptorTable(1, table);
		return true;
	}

	bool RenderDeviceD3D12::CopyRayBuffer(ID3D12Resource* source, IRHITexture* atlas, UINT pitch)
	{
		auto* texture = static_cast<D3D12Texture*>(atlas);
		auto* cmd = EnsureRecording();
		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = source;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Footprint = { texture->GetDXGIFormat(), texture->GetWidth(), texture->GetHeight(), 1, pitch };
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = texture->GetResource();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		TransitionResource(dst.pResource, texture->GetStateRef(), D3D12_RESOURCE_STATE_COPY_DEST);
		cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		return true;
	}

	void RenderDeviceD3D12::InitCaps()
	{
		m_caps.backend = ERHIBackend::D3D12;
		m_caps.backendName = m_isWarp ? "D3D12 (WARP)" : "D3D12";
		m_caps.adapterName = m_adapterName.c_str();
		m_caps.featureLevel = static_cast<uint32_t>(m_featureLevel);
		m_caps.maxSimultaneousRTs = 8u;
		m_caps.maxTextureStages = 16u;
		m_caps.maxTextureSize = 16384u;
		m_caps.maxAnisotropy = 16u;
		m_caps.maxVertexUniformVectors = 4096u;
		m_caps.isSoftwareAdapter = m_isWarp;
		m_caps.supportsFloatRenderTargets = true;
		m_caps.supportsDepthTextures = true;
		m_caps.supportsInstancing = true;
		m_caps.supportsOcclusionQuery = true;
		m_caps.supportsTwoSidedStencil = true;
		m_caps.supportsUserClipPlanes = true;
		m_caps.supportsBCCompression = true;
		m_caps.supportsETC2Compression = false;
		m_caps.supportsASTCCompression = false;
		m_caps.supportsAsyncReadback = true;
		m_caps.supportsRuntimeShaderCompile = true;
		m_caps.topLeftScreenOrigin = true;
		m_caps.clipSpaceZeroToOne = true;
	}

	bool RenderDeviceD3D12::Initialize(uint32_t initialWidth, uint32_t initialHeight)
	{
		if (!m_device || !m_cmdQueue || !m_swapChain)
			return false;

		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
		{
			if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocators[f]))))
				return false;
		}

		if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_cmdList))) || !m_cmdList)
			return false;
		m_cmdList->Close();
		m_isRecording = false;

		if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))) || !m_fence)
			return false;
		m_fenceEvent = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent)
			return false;

		if (!CreateRootSignatureAndHeaps())
			return false;

		if (!CreateBackBufferViews(initialWidth, initialHeight))
			return false;

		// Create 1x1 white fallback SRV texture for unbound texture slots
		const uint32_t whitePixel = 0xFFFFFFFFu;
		RHIMipData mip0 = { &whitePixel, 4, 4 };
		m_whiteFallbackTex = static_cast<D3D12Texture*>(CreateTexture(1, 1, PixelFormat::A8R8G8B8, 1, ERHITextureUsage::ShaderResource, &mip0, 1));

		ExecuteAndSyncGPU();
		return true;
	}

	bool RenderDeviceD3D12::CreateRootSignatureAndHeaps()
	{
		// Root Signature:
		// [0] Root CBV (b0, VS)
		// [1] Root CBV (b0, PS)
		// [2] SRV Table (t0..t15, ALL)
		// [3] Sampler Table (s0..s15, ALL)
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = RHIStateTracker::MAX_SAMPLER_STAGES;
		srvRange.BaseShaderRegister = 0;
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = 0;

		D3D12_DESCRIPTOR_RANGE sampRange = {};
		sampRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		sampRange.NumDescriptors = RHIStateTracker::MAX_SAMPLER_STAGES;
		sampRange.BaseShaderRegister = 0;
		sampRange.RegisterSpace = 0;
		sampRange.OffsetInDescriptorsFromTableStart = 0;

		D3D12_ROOT_PARAMETER rootParams[4] = {};
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParams[0].Descriptor.ShaderRegister = 0;
		rootParams[0].Descriptor.RegisterSpace = 0;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParams[1].Descriptor.ShaderRegister = 0;
		rootParams[1].Descriptor.RegisterSpace = 0;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[3].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[3].DescriptorTable.pDescriptorRanges = &sampRange;
		rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = 4;
		rsDesc.pParameters = rootParams;
		rsDesc.NumStaticSamplers = 0;
		rsDesc.pStaticSamplers = nullptr;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		const auto& libs = GetD3D12Libs();
		ComPtr<ID3DBlob> sigBlob, errBlob;
		if (!libs.pfnSerializeRootSignature ||
			FAILED(libs.pfnSerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob)) || !sigBlob)
		{
			if (errBlob)
				OUTPUT_LOG("ERROR: D3D12SerializeRootSignature failed: %s\n", static_cast<const char*>(errBlob->GetBufferPointer()));
			return false;
		}

		if (FAILED(m_device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature))) || !m_rootSignature)
			return false;

		if (!m_cpuRtvAlloc.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024))
			return false;
		if (!m_cpuDsvAlloc.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 512))
			return false;
		if (!m_cpuSrvAlloc.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16384))
			return false;
		if (!m_cpuSamplerAlloc.Init(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 512))
			return false;

		// Permanent null RTV descriptor for sparse MRT bindings (e.g. SetRenderTarget(1, nullptr) while slots 2..3 remain bound)
		m_nullRtvSlot = m_cpuRtvAlloc.Allocate();
		if (m_nullRtvSlot != UINT32_MAX)
		{
			D3D12_RENDER_TARGET_VIEW_DESC nullRtvDesc = {};
			nullRtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			nullRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			nullRtvDesc.Texture2D.MipSlice = 0;
			nullRtvDesc.Texture2D.PlaneSlice = 0;
			m_device->CreateRenderTargetView(nullptr, &nullRtvDesc, m_cpuRtvAlloc.GetHandle(m_nullRtvSlot));
		}

		// Shader-visible GPU SRV heap partitioned across FRAME_COUNT (32768 descriptors per frame = 98304 total)
		m_gpuSrvCapacity = 32768u * FRAME_COUNT;
		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
			m_gpuSrvFrameBase[f] = f * 32768u;
		m_gpuSrvOffset = m_gpuSrvFrameBase[0];

		D3D12_DESCRIPTOR_HEAP_DESC gpuSrvDesc = {};
		gpuSrvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		gpuSrvDesc.NumDescriptors = m_gpuSrvCapacity;
		gpuSrvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (FAILED(m_device->CreateDescriptorHeap(&gpuSrvDesc, IID_PPV_ARGS(&m_gpuSrvHeap))) || !m_gpuSrvHeap)
			return false;
		m_gpuSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Shader-visible GPU Sampler heap (2048 hardware max)
		m_gpuSamplerCapacity = 2048u;
		m_cpuSamplerCache.clear();
		m_gpuSamplerTableCache.clear();
		m_samplerTableBase = UINT32_MAX;
		m_stateTracker.MarkAllDirty();
		m_gpuSamplerOffset = 0;
		D3D12_DESCRIPTOR_HEAP_DESC gpuSampDesc = {};
		gpuSampDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		gpuSampDesc.NumDescriptors = m_gpuSamplerCapacity;
		gpuSampDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (FAILED(m_device->CreateDescriptorHeap(&gpuSampDesc, IID_PPV_ARGS(&m_gpuSamplerHeap))) || !m_gpuSamplerHeap)
			return false;
		m_gpuSamplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		// Persistent 256-byte zeroed CBV fallback + 16-byte [0,0,0,1] fallback vertex attribute buffer
		D3D12_HEAP_PROPERTIES uploadHeap = {};
		uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = 256;
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.SampleDesc.Count = 1;
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if (FAILED(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_zeroCbvBuffer))))
			return false;
		{
			void* p = nullptr;
			D3D12_RANGE rr = { 0, 0 };
			if (SUCCEEDED(m_zeroCbvBuffer->Map(0, &rr, &p)) && p)
			{
				std::memset(p, 0, 256);
				m_zeroCbvBuffer->Unmap(0, nullptr);
			}
			m_zeroCbvGpuVA = m_zeroCbvBuffer->GetGPUVirtualAddress();
			m_currentVsCbvGpuVA = m_zeroCbvGpuVA;
			m_currentPsCbvGpuVA = m_zeroCbvGpuVA;
		}

		bufDesc.Width = 16;
		if (FAILED(m_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_defaultAttrVB))))
			return false;
		{
			const float defAttr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			void* p = nullptr;
			D3D12_RANGE rr = { 0, 0 };
			if (SUCCEEDED(m_defaultAttrVB->Map(0, &rr, &p)) && p)
			{
				std::memcpy(p, defAttr, sizeof(defAttr));
				m_defaultAttrVB->Unmap(0, nullptr);
			}
			m_defaultAttrVBV.BufferLocation = m_defaultAttrVB->GetGPUVirtualAddress();
			m_defaultAttrVBV.SizeInBytes = sizeof(defAttr);
			m_defaultAttrVBV.StrideInBytes = sizeof(defAttr);
		}

		// Occlusion query heap + persistent readback buffer
		D3D12_QUERY_HEAP_DESC qhd = {};
		qhd.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		qhd.Count = MAX_OCCLUSION_QUERIES;
		if (SUCCEEDED(m_device->CreateQueryHeap(&qhd, IID_PPV_ARGS(&m_occlusionHeap))))
		{
			D3D12_HEAP_PROPERTIES rbHeap = {};
			rbHeap.Type = D3D12_HEAP_TYPE_READBACK;
			bufDesc.Width = MAX_OCCLUSION_QUERIES * sizeof(uint64_t);
			if (SUCCEEDED(m_device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &bufDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_occlusionReadback))))
			{
				void* p = nullptr;
				if (SUCCEEDED(m_occlusionReadback->Map(0, nullptr, &p)))
				{
					m_occlusionMappedPtr = static_cast<uint64_t*>(p);
					std::memset(m_occlusionMappedPtr, 0, MAX_OCCLUSION_QUERIES * sizeof(uint64_t));
				}
			}
		}

		return true;
	}

	void RenderDeviceD3D12::ReleaseBackBufferViews()
	{
		for (uint32_t i = 0; i < FRAME_COUNT; ++i)
		{
			m_backBuffers[i].Reset();
			m_backBufferStates[i] = D3D12_RESOURCE_STATE_PRESENT;
			if (m_backBufferRtvSlots[i] != UINT32_MAX)
			{
				m_cpuRtvAlloc.Free(m_backBufferRtvSlots[i]);
				m_backBufferRtvSlots[i] = UINT32_MAX;
			}
		}
		m_defaultDepthTex.Reset();
		if (m_defaultDsvSlot != UINT32_MAX)
		{
			m_cpuDsvAlloc.Free(m_defaultDsvSlot);
			m_defaultDsvSlot = UINT32_MAX;
		}
	}

	bool RenderDeviceD3D12::CreateBackBufferViews(uint32_t width, uint32_t height)
	{
		if (!m_device || !m_swapChain)
			return false;

		ReleaseBackBufferViews();

		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		m_swapChain->GetDesc1(&scDesc);
		m_backBufferWidth = scDesc.Width > 0 ? scDesc.Width : width;
		m_backBufferHeight = scDesc.Height > 0 ? scDesc.Height : height;
		m_renderTargetWidth = m_backBufferWidth;
		m_renderTargetHeight = m_backBufferHeight;

		for (uint32_t i = 0; i < FRAME_COUNT; ++i)
		{
			if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]))) || !m_backBuffers[i])
				return false;
			m_backBufferStates[i] = D3D12_RESOURCE_STATE_PRESENT;
			m_backBufferRtvSlots[i] = m_cpuRtvAlloc.Allocate();
			m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[i]));
		}

		m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		D3D12_HEAP_PROPERTIES defaultHeap = {};
		defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
		D3D12_RESOURCE_DESC dd = {};
		dd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dd.Width = m_backBufferWidth;
		dd.Height = m_backBufferHeight;
		dd.DepthOrArraySize = 1;
		dd.MipLevels = 1;
		dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dd.SampleDesc.Count = 1;
		dd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE dsClear = {};
		dsClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsClear.DepthStencil.Depth = 1.0f;
		dsClear.DepthStencil.Stencil = 0;

		if (FAILED(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &dd, D3D12_RESOURCE_STATE_DEPTH_WRITE, &dsClear, IID_PPV_ARGS(&m_defaultDepthTex))) || !m_defaultDepthTex)
			return false;

		m_defaultDepthState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		m_defaultDsvSlot = m_cpuDsvAlloc.Allocate();
		m_device->CreateDepthStencilView(m_defaultDepthTex.Get(), nullptr, m_cpuDsvAlloc.GetHandle(m_defaultDsvSlot));

		m_usingBackBufferRTV = true;
		m_activeNullRtvMask = 0;
		for (int i = 0; i < 4; ++i)
		{
			m_activeColorTexObjs[i] = nullptr;
			m_activeRTVs[i] = {};
			m_activeRtvFormats[i] = (i == 0) ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_UNKNOWN;
		}
		m_activeRTVs[0] = m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[m_backBufferIndex]);
		m_activeRTVCount = 1;
		m_activeSampleCount = 1;

		m_hasActiveDSV = true;
		m_usingDefaultDSV = true;
		m_activeDepthTexObj = nullptr;
		m_activeDSV = m_cpuDsvAlloc.GetHandle(m_defaultDsvSlot);
		m_activeDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		ParaViewport vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = m_backBufferWidth;
		vp.Height = m_backBufferHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		m_currentViewport = vp;
		m_stateTracker.SetViewport(vp);
		return true;
	}

	bool RenderDeviceD3D12::ResizeSwapChain(uint32_t width, uint32_t height, bool isWindowed)
	{
		if (!m_device || !m_swapChain)
			return false;

		if (width == 0) width = 1;
		if (height == 0) height = 1;

		ExecuteAndSyncGPU();
		ReleaseBackBufferViews();

		m_swapChain->SetFullscreenState(isWindowed ? FALSE : TRUE, nullptr);
		HRESULT hr = m_swapChain->ResizeBuffers(FRAME_COUNT, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(hr))
		{
			OUTPUT_LOG("ERROR: D3D12 ResizeBuffers(%ux%u) failed (hr=0x%08X)\n", width, height, static_cast<unsigned int>(hr));
			return false;
		}

		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
			m_frameResetDone[f] = false;

		return CreateBackBufferViews(width, height);
	}

	void RenderDeviceD3D12::WaitForFenceValue(uint64_t fenceValue)
	{
		if (!m_fence || fenceValue == 0)
			return;
		if (m_fence->GetCompletedValue() < fenceValue)
		{
			m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			::WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void RenderDeviceD3D12::ReclaimCompletedFrameResources(uint32_t frameSlot)
	{
		const uint64_t completed = m_fence ? m_fence->GetCompletedValue() : UINT64_MAX;
		size_t writeIdx = 0;
		for (size_t i = 0; i < m_deferredReleases.size(); ++i)
		{
			if (m_deferredReleases[i].fenceValue > completed)
			{
				if (writeIdx != i)
					m_deferredReleases[writeIdx] = std::move(m_deferredReleases[i]);
				++writeIdx;
			}
		}
		m_deferredReleases.resize(writeIdx);

		// Reset upload page offsets for this frame slot (keep largest page, free extra overflow pages)
		auto& pages = m_uploadPages[frameSlot];
		if (pages.size() > 1)
		{
			UploadPage largest = std::move(pages.back());
			for (size_t i = 0; i + 1 < pages.size(); ++i)
			{
				if (pages[i].resource && pages[i].cpuBase)
					pages[i].resource->Unmap(0, nullptr);
			}
			pages.clear();
			pages.push_back(std::move(largest));
		}
		for (auto& p : pages)
			p.offset = 0;

		m_gpuSrvOffset = m_gpuSrvFrameBase[frameSlot];
		m_srvTableDirty = true;
		if (frameSlot == m_frameIndex)
		{
			m_currentVsCbvGpuVA = m_zeroCbvGpuVA;
			m_currentPsCbvGpuVA = m_zeroCbvGpuVA;
			m_cbvDirtyAfterReset = true;
		}
	}

	void RenderDeviceD3D12::DeferRelease(IUnknown* obj)
	{
		if (!obj || m_isShuttingDown)
			return;
		DeferredReleaseEntry e;
		e.fenceValue = m_nextFenceValue;
		e.object = obj;
		m_deferredReleases.push_back(std::move(e));
	}

	void RenderDeviceD3D12::NotifyTextureDestroyed(D3D12Texture* tex)
	{
		if (!tex || m_isShuttingDown)
			return;
		for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
		{
			if (m_boundTextures[s] == tex)
			{
				m_boundTextures[s] = nullptr;
				m_srvTableDirty = true;
			}
		}
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (m_activeColorTexObjs[i] == tex)
			{
				m_activeColorTexObjs[i] = nullptr;
				if (m_nullRtvSlot != UINT32_MAX)
				{
					m_activeRTVs[i] = m_cpuRtvAlloc.GetHandle(m_nullRtvSlot);
					m_activeRtvFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
					m_activeNullRtvMask |= (1u << i);
				}
			}
		}
		if (m_activeDepthTexObj == tex)
		{
			m_activeDepthTexObj = nullptr;
			if (!m_usingDefaultDSV)
			{
				m_hasActiveDSV = false;
				m_activeDSV = {};
				m_activeDsvFormat = DXGI_FORMAT_UNKNOWN;
			}
		}
	}

	void RenderDeviceD3D12::NotifyBufferDestroyed(D3D12Buffer* buf)
	{
		if (!buf || m_isShuttingDown)
			return;
		for (uint32_t s = 0; s < 4; ++s)
		{
			if (m_streams[s].buffer == buf)
				m_streams[s].buffer = nullptr;
		}
		if (m_currentIndexBuffer == buf)
			m_currentIndexBuffer = nullptr;
	}

	void RenderDeviceD3D12::NotifyProgramDestroyed(D3D12ShaderProgram* prog)
	{
		if (!prog || m_isShuttingDown)
			return;
		if (m_currentProgram == prog)
			m_currentProgram = nullptr;
	}

	void RenderDeviceD3D12::NotifyRenderTargetDestroyed(D3D12RenderTarget* rt)
	{
		if (!rt || m_isShuttingDown)
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

	ID3D12GraphicsCommandList* RenderDeviceD3D12::EnsureRecording()
	{
		if (m_isRecording)
			return m_cmdList.Get();
		if (!m_device || !m_cmdList)
			return nullptr;

		if (!m_frameResetDone[m_frameIndex])
		{
			WaitForFenceValue(m_frameFenceValues[m_frameIndex]);
			ReclaimCompletedFrameResources(m_frameIndex);
			m_cmdAllocators[m_frameIndex]->Reset();
			m_frameResetDone[m_frameIndex] = true;
		}

		if (FAILED(m_cmdList->Reset(m_cmdAllocators[m_frameIndex].Get(), nullptr)))
			return nullptr;

		m_isRecording = true;

		ID3D12DescriptorHeap* heaps[2] = { m_gpuSrvHeap.Get(), m_gpuSamplerHeap.Get() };
		m_cmdList->SetDescriptorHeaps(2, heaps);
		m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
		m_cmdList->SetGraphicsRootConstantBufferView(0, m_currentVsCbvGpuVA ? m_currentVsCbvGpuVA : m_zeroCbvGpuVA);
		m_cmdList->SetGraphicsRootConstantBufferView(1, m_currentPsCbvGpuVA ? m_currentPsCbvGpuVA : m_zeroCbvGpuVA);

		BindActiveRenderTargetsToCmdList();

		D3D12_VIEWPORT vp = {
			static_cast<FLOAT>(m_currentViewport.X),
			static_cast<FLOAT>(m_currentViewport.Y),
			static_cast<FLOAT>(m_currentViewport.Width > 0 ? m_currentViewport.Width : m_renderTargetWidth),
			static_cast<FLOAT>(m_currentViewport.Height > 0 ? m_currentViewport.Height : m_renderTargetHeight),
			m_currentViewport.MinZ,
			m_currentViewport.MaxZ
		};
		m_cmdList->RSSetViewports(1, &vp);

		if (m_stateTracker.GetRenderState(ERenderState::SCISSORTESTENABLE))
		{
			const RECT& sr = m_stateTracker.GetScissorRect();
			D3D12_RECT r = { sr.left, sr.top, sr.right, sr.bottom };
			m_cmdList->RSSetScissorRects(1, &r);
		}
		else
		{
			D3D12_RECT r = { 0, 0, static_cast<LONG>(m_renderTargetWidth), static_cast<LONG>(m_renderTargetHeight) };
			m_cmdList->RSSetScissorRects(1, &r);
		}

		m_srvTableDirty = true;
		return m_cmdList.Get();
	}

	void RenderDeviceD3D12::ExecuteAndSyncGPU()
	{
		if (!m_cmdQueue || !m_fence)
			return;

		if (m_isRecording && m_cmdList)
		{
			m_cmdList->Close();
			m_isRecording = false;
			ID3D12CommandList* lists[1] = { m_cmdList.Get() };
			m_cmdQueue->ExecuteCommandLists(1, lists);
		}

		const uint64_t sigVal = m_nextFenceValue++;
		m_cmdQueue->Signal(m_fence.Get(), sigVal);
		m_frameFenceValues[m_frameIndex] = sigVal;
		WaitForFenceValue(sigVal);

		// Safe to reclaim all frames since GPU is completely idle
		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
		{
			m_frameFenceValues[f] = sigVal;
			m_frameResetDone[f] = false;
			ReclaimCompletedFrameResources(f);
		}
	}

	bool RenderDeviceD3D12::AllocateUpload(
		uint32_t byteSize,
		uint32_t alignment,
		void*& outCpuPtr,
		D3D12_GPU_VIRTUAL_ADDRESS& outGpuVA,
		ID3D12Resource*& outResource,
		uint64_t& outOffset)
	{
		if (!m_device || byteSize == 0)
			return false;

		EnsureRecording(); // Ensure current frame slot has been reclaimed/reset before writing to its upload pages!

		auto& pages = m_uploadPages[m_frameIndex];
		if (!pages.empty())
		{
			UploadPage& cur = pages.back();
			uint64_t aligned = AlignUpU64(cur.offset, alignment);
			if (aligned + byteSize <= cur.capacity)
			{
				outCpuPtr = cur.cpuBase + aligned;
				outGpuVA = cur.gpuBase + aligned;
				outResource = cur.resource.Get();
				outOffset = aligned;
				cur.offset = aligned + byteSize;
				return true;
			}
		}

		uint64_t prevCap = pages.empty() ? (4ULL * 1024ULL * 1024ULL) : (pages.back().capacity * 2ULL);
		uint64_t newCap = (std::max)(AlignUpU64(byteSize, 65536ULL), prevCap);

		D3D12_HEAP_PROPERTIES hp = {};
		hp.Type = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC rd = {};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Width = newCap;
		rd.Height = 1;
		rd.DepthOrArraySize = 1;
		rd.MipLevels = 1;
		rd.SampleDesc.Count = 1;
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		UploadPage page;
		if (FAILED(m_device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&page.resource))) || !page.resource)
			return false;

		void* mapped = nullptr;
		D3D12_RANGE rr = { 0, 0 };
		if (FAILED(page.resource->Map(0, &rr, &mapped)) || !mapped)
			return false;

		page.cpuBase = static_cast<uint8_t*>(mapped);
		page.gpuBase = page.resource->GetGPUVirtualAddress();
		page.capacity = newCap;
		page.offset = byteSize;

		outCpuPtr = page.cpuBase;
		outGpuVA = page.gpuBase;
		outResource = page.resource.Get();
		outOffset = 0;

		pages.push_back(std::move(page));
		return true;
	}

	void RenderDeviceD3D12::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES& ioCurrentState, D3D12_RESOURCE_STATES targetState)
	{
		if (!resource || ioCurrentState == targetState)
			return;
		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd)
			return;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = ioCurrentState;
		barrier.Transition.StateAfter = targetState;
		cmd->ResourceBarrier(1, &barrier);
		ioCurrentState = targetState;
	}

	uint32_t RenderDeviceD3D12::AllocateOcclusionQuerySlot()
	{
		if (!m_freeOcclusionSlots.empty())
		{
			uint32_t s = m_freeOcclusionSlots.back();
			m_freeOcclusionSlots.pop_back();
			return s;
		}
		if (m_nextOcclusionSlot < MAX_OCCLUSION_QUERIES)
			return m_nextOcclusionSlot++;
		return UINT32_MAX;
	}

	void RenderDeviceD3D12::FreeOcclusionQuerySlot(uint32_t slot)
	{
		if (slot < MAX_OCCLUSION_QUERIES)
			m_freeOcclusionSlots.push_back(slot);
	}

	// =========================================================================
	// Resource Creation, Render Targets & Readback
	// =========================================================================

	bool RenderDeviceD3D12::CheckDeviceError(const char* /*filename*/, const char* /*func*/, int /*nLine*/)
	{
		if (!m_device)
			return false;
		HRESULT reason = m_device->GetDeviceRemovedReason();
		if (FAILED(reason))
		{
			OUTPUT_LOG("ERROR: D3D12 device removed/reset (reason=0x%08X)\n", static_cast<unsigned int>(reason));
			return false;
		}
		return true;
	}

	IRHITexture* RenderDeviceD3D12::CreateTexture(
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
		D3D12_HEAP_PROPERTIES defaultHeap = {};
		defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

		if (IsDepthPixelFormat(format))
		{
			const bool canSample = HasFlag(usage, ERHITextureUsage::ShaderResource);
			DXGI_FORMAT resFmt = canSample ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
			DXGI_FORMAT srvFmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			if (format == PixelFormat::D16 || format == PixelFormat::D16_LOCKABLE)
			{
				resFmt = canSample ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_D16_UNORM;
				srvFmt = DXGI_FORMAT_R16_UNORM;
			}
			else if (format == PixelFormat::D32 || format == PixelFormat::D32F_LOCKABLE)
			{
				resFmt = canSample ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D32_FLOAT;
				srvFmt = DXGI_FORMAT_R32_FLOAT;
			}

			D3D12_RESOURCE_DESC td = {};
			td.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			td.Width = width;
			td.Height = height;
			td.DepthOrArraySize = 1;
			td.MipLevels = 1;
			td.Format = resFmt;
			td.SampleDesc.Count = 1;
			td.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | (canSample ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

			ComPtr<ID3D12Resource> tex;
			if (FAILED(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &td, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(&tex))) || !tex)
				return nullptr;

			uint32_t srvSlot = UINT32_MAX;
			if (canSample)
			{
				srvSlot = m_cpuSrvAlloc.Allocate();
				if (srvSlot != UINT32_MAX)
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC sd = {};
					sd.Format = srvFmt;
					sd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					sd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					sd.Texture2D.MipLevels = 1;
					m_device->CreateShaderResourceView(tex.Get(), &sd, m_cpuSrvAlloc.GetHandle(srvSlot));
				}
			}
			return new D3D12Texture(this, width, height, format, resFmt, 1, false, tex.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, srvSlot);
		}

		bool expandedToRGBA8 = RHISoftwareS3TC::NeedsRGBA8Expansion(format, true);
		uint32_t bytesPerBlock = 4;
		bool isBlockCompressed = false;
		DXGI_FORMAT dxgiFormat = expandedToRGBA8
			? DXGI_FORMAT_R8G8B8A8_UNORM
			: MapColorPixelFormatToDXGI(format, bytesPerBlock, isBlockCompressed);

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

		if (initialMips && initialMipCount > 0 && numMips > initialMipCount)
		{
			numMips = initialMipCount;
		}

		D3D12_RESOURCE_DESC td = {};
		td.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		td.Width = width;
		td.Height = height;
		td.DepthOrArraySize = 1;
		td.MipLevels = static_cast<UINT16>(numMips);
		td.Format = dxgiFormat;
		td.SampleDesc.Count = 1;
		td.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		td.Flags = HasFlag(usage, ERHITextureUsage::RenderTarget) ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;

		const bool hasInitMips = (initialMips && initialMipCount >= numMips);
		D3D12_RESOURCE_STATES initState = hasInitMips
			? D3D12_RESOURCE_STATE_COPY_DEST
			: (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		ComPtr<ID3D12Resource> tex;
		if (FAILED(m_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &td, initState, nullptr, IID_PPV_ARGS(&tex))) || !tex)
			return nullptr;

		uint32_t srvSlot = m_cpuSrvAlloc.Allocate();
		if (srvSlot != UINT32_MAX)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC sd = {};
			sd.Format = dxgiFormat;
			sd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			sd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			sd.Texture2D.MipLevels = numMips;
			m_device->CreateShaderResourceView(tex.Get(), &sd, m_cpuSrvAlloc.GetHandle(srvSlot));
		}

		auto* texObj = new D3D12Texture(this, width, height, format, dxgiFormat, numMips, expandedToRGBA8, tex.Get(), initState, srvSlot);

		if (hasInitMips)
		{
			ID3D12GraphicsCommandList* cmd = EnsureRecording();
			if (cmd)
			{
				uint32_t mipW = width;
				uint32_t mipH = height;
				for (uint32_t m = 0; m < numMips; ++m)
				{
					const RHIMipData& srcMip = initialMips[m];
					if (!srcMip.data)
						break;

					std::vector<uint8_t> decoded;
					const uint8_t* srcPtr = static_cast<const uint8_t*>(srcMip.data);
					uint32_t srcStride = srcMip.rowPitch;

					if (softwareDecodeS3TC)
					{
						size_t compBytes = srcMip.dataSize ? srcMip.dataSize : RHISoftwareS3TC::ComputeCompressedMipSize(format, mipW, mipH);
						if (!RHISoftwareS3TC::DecodeToRGBA8(format, srcMip.data, compBytes, mipW, mipH, decoded))
							break;
						srcPtr = decoded.data();
						srcStride = mipW * 4u;
					}
					else if (expandedToRGBA8)
					{
						if (!RHISoftwareS3TC::ExpandToRGBA8(format, srcMip.data, mipW, mipH, srcMip.rowPitch, decoded))
							break;
						srcPtr = decoded.data();
						srcStride = mipW * 4u;
					}
					else if (isBlockCompressed)
					{
						uint32_t blocksX = (std::max)(1u, (mipW + 3u) / 4u);
						srcStride = srcMip.rowPitch ? srcMip.rowPitch : (blocksX * bytesPerBlock);
					}
					else
					{
						srcStride = srcMip.rowPitch ? srcMip.rowPitch : (mipW * bytesPerBlock);
					}

					D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = {};
					UINT numRows = 0;
					UINT64 rowSizeInBytes = 0;
					UINT64 totalBytes = 0;
					m_device->GetCopyableFootprints(&td, m, 1, 0, &fp, &numRows, &rowSizeInBytes, &totalBytes);

					void* upCpu = nullptr;
					D3D12_GPU_VIRTUAL_ADDRESS upGpu = 0;
					ID3D12Resource* upRes = nullptr;
					uint64_t upOff = 0;
					if (totalBytes > 0 && AllocateUpload(static_cast<uint32_t>(totalBytes), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, upCpu, upGpu, upRes, upOff))
					{
						uint8_t* dstBase = static_cast<uint8_t*>(upCpu);
						size_t copyBytesPerRow = static_cast<size_t>((std::min)(static_cast<UINT64>(srcStride), rowSizeInBytes));
						for (UINT r = 0; r < numRows; ++r)
						{
							std::memcpy(dstBase + static_cast<size_t>(r) * fp.Footprint.RowPitch, srcPtr + static_cast<size_t>(r) * srcStride, copyBytesPerRow);
						}

						D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
						dstLoc.pResource = tex.Get();
						dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
						dstLoc.SubresourceIndex = m;

						D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
						srcLoc.pResource = upRes;
						srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
						srcLoc.PlacedFootprint = fp;
						srcLoc.PlacedFootprint.Offset = upOff;

						cmd->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
					}

					mipW = (std::max)(1u, mipW >> 1);
					mipH = (std::max)(1u, mipH >> 1);
				}

				TransitionResource(tex.Get(), texObj->GetStateRef(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}
		}

		return texObj;
	}

	IRHIBuffer* RenderDeviceD3D12::CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage, const void* initialData)
	{
		if (!m_device || byteSize == 0)
			return nullptr;
		return new D3D12Buffer(this, byteSize, isIndexBuffer, usage, initialData);
	}

	IRHIRenderTarget* RenderDeviceD3D12::CreateRenderTarget(
		uint32_t width,
		uint32_t height,
		PixelFormat colorFormat,
		PixelFormat depthFormat,
		uint32_t colorTargetCount,
		uint32_t msaaSamples)
	{
		if (!m_device || width == 0 || height == 0)
			return nullptr;
		auto* rt = new D3D12RenderTarget(this, width, height, colorFormat, depthFormat, colorTargetCount, msaaSamples);
		if (!rt->IsValid())
		{
			rt->ReleaseRHIResource();
			return nullptr;
		}
		return rt;
	}

	IRHIOcclusionQuery* RenderDeviceD3D12::CreateOcclusionQuery()
	{
		if (!m_device || !m_occlusionHeap)
			return nullptr;
		return new D3D12OcclusionQuery(this);
	}

	void RenderDeviceD3D12::BindActiveRenderTargetsToCmdList()
	{
		if (!m_isRecording || !m_cmdList)
			return;

		if (m_usingBackBufferRTV)
		{
			if (m_backBuffers[m_backBufferIndex])
			{
				TransitionResource(m_backBuffers[m_backBufferIndex].Get(), m_backBufferStates[m_backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);
				m_activeRTVs[0] = m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[m_backBufferIndex]);
			}
		}

		for (uint32_t i = 0; i < m_activeRTVCount; ++i)
		{
			if (m_activeColorTexObjs[i] && m_activeColorTexObjs[i]->GetResource())
			{
				TransitionResource(m_activeColorTexObjs[i]->GetResource(), m_activeColorTexObjs[i]->GetStateRef(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
		}

		if (m_hasActiveDSV)
		{
			if (m_usingDefaultDSV)
			{
				if (m_defaultDepthTex)
				{
					TransitionResource(m_defaultDepthTex.Get(), m_defaultDepthState, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					m_activeDSV = m_cpuDsvAlloc.GetHandle(m_defaultDsvSlot);
				}
			}
			else if (m_activeDepthTexObj && m_activeDepthTexObj->GetResource())
			{
				TransitionResource(m_activeDepthTexObj->GetResource(), m_activeDepthTexObj->GetStateRef(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}
		}

		m_cmdList->OMSetRenderTargets(
			m_activeRTVCount,
			m_activeRTVCount > 0 ? m_activeRTVs : nullptr,
			FALSE,
			m_hasActiveDSV ? &m_activeDSV : nullptr);
	}

	bool RenderDeviceD3D12::SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride)
	{
		m_activeNullRtvMask = 0;
		for (int i = 0; i < 4; ++i)
		{
			m_activeColorTexObjs[i] = nullptr;
			m_activeRTVs[i] = {};
			m_activeRtvFormats[i] = DXGI_FORMAT_UNKNOWN;
		}

		uint32_t effectiveCount = 0;
		if (targets && count > 0)
		{
			const uint32_t clamped = (std::min)(count, 4u);
			for (uint32_t i = 0; i < clamped; ++i)
			{
				if (targets[i] != nullptr)
					effectiveCount = i + 1u;
			}
		}

		if (effectiveCount == 0)
		{
			m_usingBackBufferRTV = true;
			m_activeRTVCount = 1;
			m_activeSampleCount = 1;
			m_activeRTVs[0] = m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[m_backBufferIndex]);
			m_activeRtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

			if (depthStencilOverride)
			{
				auto* dsRT = static_cast<D3D12RenderTarget*>(depthStencilOverride);
				m_hasActiveDSV = dsRT->HasDSV();
				m_usingDefaultDSV = false;
				m_activeDepthTexObj = dsRT->GetD3D12DepthTex();
				m_activeDSV = dsRT->GetDSV();
				m_activeDsvFormat = dsRT->GetDxgiDepthFormat();
			}
			else
			{
				m_hasActiveDSV = true;
				m_usingDefaultDSV = true;
				m_activeDepthTexObj = nullptr;
				m_activeDSV = m_cpuDsvAlloc.GetHandle(m_defaultDsvSlot);
				m_activeDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			}

			m_renderTargetWidth = m_backBufferWidth;
			m_renderTargetHeight = m_backBufferHeight;
			EnsureRecording();
			BindActiveRenderTargetsToCmdList();
			m_srvTableDirty = true;
			NotifyRenderTargetsBound(0, nullptr, depthStencilOverride);
			return true;
		}

		auto* primary = static_cast<D3D12RenderTarget*>(targets[0]);
		m_usingBackBufferRTV = (primary == nullptr);
		uint32_t rtvCount = 0;

		if (effectiveCount == 1 && primary && primary->GetColorCount() > 1)
		{
			rtvCount = primary->GetColorCount();
			for (uint32_t i = 0; i < rtvCount; ++i)
			{
				m_activeColorTexObjs[i] = primary->GetD3D12ColorTex(i);
				m_activeRTVs[i] = primary->GetRTV(i);
				m_activeRtvFormats[i] = primary->GetDxgiColorFormat();
			}
		}
		else
		{
			rtvCount = effectiveCount;
			for (uint32_t i = 0; i < rtvCount; ++i)
			{
				if (i == 0 && targets[0] == nullptr)
				{
					m_activeColorTexObjs[0] = nullptr;
					m_activeRTVs[0] = m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[m_backBufferIndex]);
					m_activeRtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				}
				else
				{
					auto* rt = static_cast<D3D12RenderTarget*>(targets[i]);
					if (rt)
					{
						m_activeColorTexObjs[i] = rt->GetD3D12ColorTex(0);
						m_activeRTVs[i] = rt->GetRTV(0);
						m_activeRtvFormats[i] = rt->GetDxgiColorFormat();
					}
					else
					{
						m_activeColorTexObjs[i] = nullptr;
						m_activeRTVs[i] = (m_nullRtvSlot != UINT32_MAX) ? m_cpuRtvAlloc.GetHandle(m_nullRtvSlot) : D3D12_CPU_DESCRIPTOR_HANDLE{};
						m_activeRtvFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
						m_activeNullRtvMask |= (1u << i);
					}
				}
			}
		}

		m_activeRTVCount = rtvCount;
		m_activeSampleCount = primary ? primary->GetSampleCount() : 1u;

		if (depthStencilOverride)
		{
			auto* dsRT = static_cast<D3D12RenderTarget*>(depthStencilOverride);
			if (dsRT && dsRT->HasDSV())
			{
				m_hasActiveDSV = true;
				m_usingDefaultDSV = false;
				m_activeDepthTexObj = dsRT->GetD3D12DepthTex();
				m_activeDSV = dsRT->GetDSV();
				m_activeDsvFormat = dsRT->GetDxgiDepthFormat();
			}
			else
			{
				m_hasActiveDSV = false;
				m_usingDefaultDSV = false;
				m_activeDepthTexObj = nullptr;
				m_activeDSV = {};
				m_activeDsvFormat = DXGI_FORMAT_UNKNOWN;
			}
		}
		else if (primary && primary->HasDSV())
		{
			m_hasActiveDSV = true;
			m_usingDefaultDSV = false;
			m_activeDepthTexObj = primary->GetD3D12DepthTex();
			m_activeDSV = primary->GetDSV();
			m_activeDsvFormat = primary->GetDxgiDepthFormat();
		}
		else if (!primary || (primary->GetWidth() == m_backBufferWidth && primary->GetHeight() == m_backBufferHeight && primary->GetSampleCount() == 1 && m_defaultDsvSlot != UINT32_MAX))
		{
			// Preserve default scene depth-stencil buffer across screen-sized color render targets (matching D3D9 surface semantics)
			m_hasActiveDSV = true;
			m_usingDefaultDSV = true;
			m_activeDepthTexObj = nullptr;
			m_activeDSV = m_cpuDsvAlloc.GetHandle(m_defaultDsvSlot);
			m_activeDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		}
		else
		{
			m_hasActiveDSV = false;
			m_usingDefaultDSV = false;
			m_activeDepthTexObj = nullptr;
			m_activeDSV = {};
			m_activeDsvFormat = DXGI_FORMAT_UNKNOWN;
		}

		m_renderTargetWidth = primary ? primary->GetWidth() : m_backBufferWidth;
		m_renderTargetHeight = primary ? primary->GetHeight() : m_backBufferHeight;

		EnsureRecording();
		BindActiveRenderTargetsToCmdList();
		m_srvTableDirty = true;
		NotifyRenderTargetsBound(effectiveCount, targets, depthStencilOverride);
		return true;
	}

	void RenderDeviceD3D12::BeginRenderTarget(uint32_t width, uint32_t height)
	{
		m_rtSizeStack.push(std::make_pair(m_renderTargetWidth, m_renderTargetHeight));
		m_renderTargetWidth = width;
		m_renderTargetHeight = height;
	}

	void RenderDeviceD3D12::EndRenderTarget()
	{
		if (!m_rtSizeStack.empty())
		{
			m_renderTargetWidth = m_rtSizeStack.top().first;
			m_renderTargetHeight = m_rtSizeStack.top().second;
			m_rtSizeStack.pop();
		}
	}

	bool RenderDeviceD3D12::BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect, const RECT* dstRect, bool linearFilter)
	{
		if (!m_device)
			return false;

		auto* srcRT = static_cast<D3D12RenderTarget*>(src);
		auto* dstRT = static_cast<D3D12RenderTarget*>(dst);

		ID3D12Resource* srcRes = srcRT ? (srcRT->GetD3D12ColorTex(0) ? srcRT->GetD3D12ColorTex(0)->GetResource() : nullptr) : m_backBuffers[m_backBufferIndex].Get();
		ID3D12Resource* dstRes = dstRT ? (dstRT->GetD3D12ColorTex(0) ? dstRT->GetD3D12ColorTex(0)->GetResource() : nullptr) : m_backBuffers[m_backBufferIndex].Get();
		if (!srcRes || !dstRes || srcRes == dstRes)
			return false;

		D3D12_RESOURCE_STATES& srcState = srcRT ? srcRT->GetD3D12ColorTex(0)->GetStateRef() : m_backBufferStates[m_backBufferIndex];
		D3D12_RESOURCE_STATES& dstState = dstRT ? dstRT->GetD3D12ColorTex(0)->GetStateRef() : m_backBufferStates[m_backBufferIndex];

		D3D12_RESOURCE_DESC sd = srcRes->GetDesc();
		D3D12_RESOURCE_DESC dd = dstRes->GetDesc();

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

		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd)
			return false;

		// Fast path 1: MSAA resolve
		if (sd.SampleDesc.Count > 1 && dd.SampleDesc.Count == 1 && sd.Width == dd.Width && sd.Height == dd.Height && sd.Format == dd.Format && !srcRect && !dstRect)
		{
			TransitionResource(srcRes, srcState, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			TransitionResource(dstRes, dstState, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			cmd->ResolveSubresource(dstRes, 0, srcRes, 0, dd.Format);
			BindActiveRenderTargetsToCmdList();
			return true;
		}

		// Fast path 2: 1:1 unscaled copy
		if (sw == dw && sh == dh && sd.Format == dd.Format && sd.SampleDesc.Count == dd.SampleDesc.Count)
		{
			TransitionResource(srcRes, srcState, D3D12_RESOURCE_STATE_COPY_SOURCE);
			TransitionResource(dstRes, dstState, D3D12_RESOURCE_STATE_COPY_DEST);

			if (sx0 == 0 && sy0 == 0 && dx0 == 0 && dy0 == 0 &&
				static_cast<UINT64>(sw) == sd.Width && static_cast<UINT>(sh) == sd.Height &&
				sd.Width == dd.Width && sd.Height == dd.Height)
			{
				cmd->CopyResource(dstRes, srcRes);
			}
			else
			{
				D3D12_TEXTURE_COPY_LOCATION dstLoc = { dstRes, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
				D3D12_TEXTURE_COPY_LOCATION srcLoc = { srcRes, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
				D3D12_BOX box = {
					static_cast<UINT>((std::max)(0, sx0)),
					static_cast<UINT>((std::max)(0, sy0)),
					0,
					static_cast<UINT>((std::min)(static_cast<int>(sd.Width), sx1)),
					static_cast<UINT>((std::min)(static_cast<int>(sd.Height), sy1)),
					1
				};
				cmd->CopyTextureRegion(&dstLoc, static_cast<UINT>((std::max)(0, dx0)), static_cast<UINT>((std::max)(0, dy0)), 0, &srcLoc, &box);
			}
			BindActiveRenderTargetsToCmdList();
			return true;
		}

		// Scaled / format-converting shader quad blit path (when srcRT is a texture-backed RT)
		if (!srcRT || !srcRT->GetD3D12ColorTex(0))
			return false;

		if (!m_blitProgram || !m_blitVertexDecl)
		{
			static const char* kBlitHLSL =
				"Texture2D gTex : register(t0);\n"
				"SamplerState gSamp : register(s0);\n"
				"struct VSIn { float2 pos : POSITION; float2 uv : TEXCOORD0; };\n"
				"struct VSOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };\n"
				"VSOut BlitVS(VSIn i) { VSOut o; o.pos = float4(i.pos, 0.0, 1.0); o.uv = i.uv; return o; }\n"
				"float4 BlitPS(VSOut i) : SV_Target { return gTex.Sample(gSamp, i.uv); }\n";

			const auto& libs = GetD3D12Libs();
			if (!libs.pfnCompile)
				return false;
			ComPtr<ID3DBlob> vsBlob, psBlob;
			if (FAILED(libs.pfnCompile(kBlitHLSL, std::strlen(kBlitHLSL), "Blit12", nullptr, nullptr, "BlitVS", "vs_5_0", 0, 0, &vsBlob, nullptr)))
				return false;
			if (FAILED(libs.pfnCompile(kBlitHLSL, std::strlen(kBlitHLSL), "Blit12", nullptr, nullptr, "BlitPS", "ps_5_0", 0, 0, &psBlob, nullptr)))
				return false;

			RHIShaderProgramDesc progDesc = {};
			progDesc.vsBytecode = vsBlob->GetBufferPointer();
			progDesc.vsBytecodeSize = static_cast<uint32_t>(vsBlob->GetBufferSize());
			progDesc.psBytecode = psBlob->GetBufferPointer();
			progDesc.psBytecodeSize = static_cast<uint32_t>(psBlob->GetBufferSize());
			progDesc.debugName = "D3D12Blit";
			m_blitProgram = static_cast<D3D12ShaderProgram*>(CompileShaderProgram(progDesc));

			VertexElement ve[3] = {
				{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
				{ 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
				D3DDECL_END()
			};
			m_blitVertexDecl = new CVertexDeclaration(ve);
		}

		// Save current binding state, render quad into dst, then restore
		bool prevBackBuf = m_usingBackBufferRTV;
		D3D12Texture* prevColorObjs[4] = { m_activeColorTexObjs[0], m_activeColorTexObjs[1], m_activeColorTexObjs[2], m_activeColorTexObjs[3] };
		D3D12_CPU_DESCRIPTOR_HANDLE prevRtvs[4] = { m_activeRTVs[0], m_activeRTVs[1], m_activeRTVs[2], m_activeRTVs[3] };
		DXGI_FORMAT prevRtvFmts[4] = { m_activeRtvFormats[0], m_activeRtvFormats[1], m_activeRtvFormats[2], m_activeRtvFormats[3] };
		uint32_t prevRtvCount = m_activeRTVCount;
		uint32_t prevSamples = m_activeSampleCount;
		bool prevHasDSV = m_hasActiveDSV;
		ParaViewport prevVP = m_currentViewport;
		D3D12ShaderProgram* prevProg = m_currentProgram;
		CVertexDeclaration* prevDecl = m_currentVertexDecl;
		D3D12Texture* prevTex0 = m_boundTextures[0];
		uint32_t prevZEnable = GetRenderState(ERenderState::ZENABLE);
		uint32_t prevAlphaBlend = GetRenderState(ERenderState::ALPHABLENDENABLE);
		uint32_t prevCull = GetRenderState(ERenderState::CULLMODE);
		uint32_t prevMinF = m_stateTracker.GetSamplerState(0, ESamplerStateType::MINFILTER);
		uint32_t prevMagF = m_stateTracker.GetSamplerState(0, ESamplerStateType::MAGFILTER);

		IRHIRenderTarget* dstArr[1] = { dst };
		SetRenderTargets(dst ? 1u : 0u, dst ? dstArr : nullptr, nullptr);
		m_hasActiveDSV = false;
		BindActiveRenderTargetsToCmdList();

		ParaViewport bvp;
		bvp.X = static_cast<uint32_t>((std::max)(0, dx0));
		bvp.Y = static_cast<uint32_t>((std::max)(0, dy0));
		bvp.Width = static_cast<uint32_t>(dw);
		bvp.Height = static_cast<uint32_t>(dh);
		bvp.MinZ = 0.0f;
		bvp.MaxZ = 1.0f;
		SetViewport(bvp);
		SetRenderState(ERenderState::ZENABLE, 0);
		SetRenderState(ERenderState::ALPHABLENDENABLE, 0);
		SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
		SetSamplerState(0, ESamplerStateType::MINFILTER, linearFilter ? 2u : 1u);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, linearFilter ? 2u : 1u);
		SetTexture(0, srcRT->GetColorTexture(0));
		BindShaderProgram(m_blitProgram);
		SetVertexDeclaration(m_blitVertexDecl);

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
		DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, quadVerts, sizeof(float) * 4);

		// Restore state
		SetTexture(0, prevTex0);
		SetSamplerState(0, ESamplerStateType::MINFILTER, prevMinF);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, prevMagF);
		SetRenderState(ERenderState::ZENABLE, prevZEnable);
		SetRenderState(ERenderState::ALPHABLENDENABLE, prevAlphaBlend);
		SetRenderState(ERenderState::CULLMODE, prevCull);
		BindShaderProgram(prevProg);
		SetVertexDeclaration(prevDecl);

		m_usingBackBufferRTV = prevBackBuf;
		for (int i = 0; i < 4; ++i)
		{
			m_activeColorTexObjs[i] = prevColorObjs[i];
			m_activeRTVs[i] = prevRtvs[i];
			m_activeRtvFormats[i] = prevRtvFmts[i];
		}
		m_activeRTVCount = prevRtvCount;
		m_activeSampleCount = prevSamples;
		m_hasActiveDSV = prevHasDSV;
		BindActiveRenderTargetsToCmdList();
		SetViewport(prevVP);
		return true;
	}

	bool RenderDeviceD3D12::ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height)
	{
		if (!m_device || !nativeFboOrTexHandle)
			return false;

		auto* extRes = reinterpret_cast<ID3D12Resource*>(nativeFboOrTexHandle);
		m_backBuffers[m_backBufferIndex] = extRes;
		m_backBufferStates[m_backBufferIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_device->CreateRenderTargetView(extRes, nullptr, m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[m_backBufferIndex]));
		m_backBufferWidth = width;
		m_backBufferHeight = height;
		m_renderTargetWidth = width;
		m_renderTargetHeight = height;
		m_usingBackBufferRTV = true;
		BindActiveRenderTargetsToCmdList();
		return true;
	}

	bool RenderDeviceD3D12::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t /*nDataFormat*/, uint32_t /*nDataType*/)
	{
		if (!m_device || !pDataOut || nWidth <= 0 || nHeight <= 0)
			return false;

		ID3D12Resource* srcRes = m_usingBackBufferRTV
			? m_backBuffers[m_backBufferIndex].Get()
			: (m_activeColorTexObjs[0] ? m_activeColorTexObjs[0]->GetResource() : nullptr);
		if (!srcRes)
			return false;

		D3D12_RESOURCE_STATES& srcState = m_usingBackBufferRTV
			? m_backBufferStates[m_backBufferIndex]
			: m_activeColorTexObjs[0]->GetStateRef();

		D3D12_RESOURCE_DESC srcDesc = srcRes->GetDesc();
		int x0 = (std::max)(0, nLeft);
		int y0 = (std::max)(0, nTop);
		int x1 = (std::min)(static_cast<int>(srcDesc.Width), nLeft + nWidth);
		int y1 = (std::min)(static_cast<int>(srcDesc.Height), nTop + nHeight);
		if (x1 <= x0 || y1 <= y0)
			return false;

		uint32_t copyW = static_cast<uint32_t>(x1 - x0);
		uint32_t copyH = static_cast<uint32_t>(y1 - y0);

		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd)
			return false;

		ComPtr<ID3D12Resource> resolvedTex;
		ID3D12Resource* readSrc = srcRes;
		D3D12_RESOURCE_STATES prevState = srcState;

		if (srcDesc.SampleDesc.Count > 1)
		{
			D3D12_HEAP_PROPERTIES defHeap = {};
			defHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
			D3D12_RESOURCE_DESC rd = srcDesc;
			rd.SampleDesc.Count = 1;
			rd.SampleDesc.Quality = 0;
			rd.Flags = D3D12_RESOURCE_FLAG_NONE;
			if (FAILED(m_device->CreateCommittedResource(&defHeap, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_RESOLVE_DEST, nullptr, IID_PPV_ARGS(&resolvedTex))))
				return false;

			TransitionResource(srcRes, srcState, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			cmd->ResolveSubresource(resolvedTex.Get(), 0, srcRes, 0, srcDesc.Format);
			D3D12_RESOURCE_STATES resState = D3D12_RESOURCE_STATE_RESOLVE_DEST;
			TransitionResource(resolvedTex.Get(), resState, D3D12_RESOURCE_STATE_COPY_SOURCE);
			readSrc = resolvedTex.Get();
		}
		else
		{
			TransitionResource(srcRes, srcState, D3D12_RESOURCE_STATE_COPY_SOURCE);
		}

		uint32_t rowPitch = static_cast<uint32_t>(AlignUpU64(copyW * 4u, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		uint64_t readbackBytes = static_cast<uint64_t>(rowPitch) * static_cast<uint64_t>(copyH);

		D3D12_HEAP_PROPERTIES rbHeap = {};
		rbHeap.Type = D3D12_HEAP_TYPE_READBACK;
		D3D12_RESOURCE_DESC rbDesc = {};
		rbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rbDesc.Width = readbackBytes;
		rbDesc.Height = 1;
		rbDesc.DepthOrArraySize = 1;
		rbDesc.MipLevels = 1;
		rbDesc.SampleDesc.Count = 1;
		rbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ComPtr<ID3D12Resource> readbackBuf;
		if (FAILED(m_device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readbackBuf))))
			return false;

		D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
		dstLoc.pResource = readbackBuf.Get();
		dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLoc.PlacedFootprint.Offset = 0;
		dstLoc.PlacedFootprint.Footprint.Format = srcDesc.Format;
		dstLoc.PlacedFootprint.Footprint.Width = copyW;
		dstLoc.PlacedFootprint.Footprint.Height = copyH;
		dstLoc.PlacedFootprint.Footprint.Depth = 1;
		dstLoc.PlacedFootprint.Footprint.RowPitch = rowPitch;

		D3D12_TEXTURE_COPY_LOCATION srcLoc = { readSrc, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
		D3D12_BOX box = { static_cast<UINT>(x0), static_cast<UINT>(y0), 0, static_cast<UINT>(x1), static_cast<UINT>(y1), 1 };
		cmd->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, &box);

		TransitionResource(srcRes, srcState, prevState);
		ExecuteAndSyncGPU();

		void* mapped = nullptr;
		if (FAILED(readbackBuf->Map(0, nullptr, &mapped)) || !mapped)
			return false;

		std::memset(pDataOut, 0, static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 4u);
		const bool isBGRA = (srcDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || srcDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

		for (uint32_t y = 0; y < copyH; ++y)
		{
			const uint8_t* srcRow = static_cast<const uint8_t*>(mapped) + static_cast<size_t>(y) * rowPitch;
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

		D3D12_RANGE emptyWrite = { 0, 0 };
		readbackBuf->Unmap(0, &emptyWrite);
		return true;
	}

	bool RenderDeviceD3D12::ReadDepthPixel(int x, int y, float& outDepth)
	{
		outDepth = 1.0f;
		if (!m_device || !m_hasActiveDSV || x < 0 || y < 0)
			return false;

		ID3D12Resource* depthRes = m_usingDefaultDSV
			? m_defaultDepthTex.Get()
			: (m_activeDepthTexObj ? m_activeDepthTexObj->GetResource() : nullptr);
		if (!depthRes)
			return false;

		D3D12_RESOURCE_STATES& depthState = m_usingDefaultDSV
			? m_defaultDepthState
			: m_activeDepthTexObj->GetStateRef();

		D3D12_RESOURCE_DESC dd = depthRes->GetDesc();
		if (static_cast<UINT64>(x) >= dd.Width || static_cast<UINT>(y) >= dd.Height || dd.SampleDesc.Count > 1)
			return false;

		D3D12_RESOURCE_DESC copyDesc = dd;
		if (copyDesc.Format == DXGI_FORMAT_R24G8_TYPELESS) copyDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		else if (copyDesc.Format == DXGI_FORMAT_R32_TYPELESS) copyDesc.Format = DXGI_FORMAT_D32_FLOAT;
		else if (copyDesc.Format == DXGI_FORMAT_R16_TYPELESS) copyDesc.Format = DXGI_FORMAT_D16_UNORM;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = {};
		UINT numRows = 0;
		UINT64 rowBytes = 0, totalBytes = 0;
		m_device->GetCopyableFootprints(&copyDesc, 0, 1, 0, &fp, &numRows, &rowBytes, &totalBytes);
		if (totalBytes == 0)
			return false;

		D3D12_HEAP_PROPERTIES rbHeap = {};
		rbHeap.Type = D3D12_HEAP_TYPE_READBACK;
		D3D12_RESOURCE_DESC rbDesc = {};
		rbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rbDesc.Width = totalBytes;
		rbDesc.Height = 1;
		rbDesc.DepthOrArraySize = 1;
		rbDesc.MipLevels = 1;
		rbDesc.SampleDesc.Count = 1;
		rbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ComPtr<ID3D12Resource> rbBuf;
		if (FAILED(m_device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rbBuf))))
			return false;

		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd)
			return false;

		D3D12_RESOURCE_STATES prev = depthState;
		TransitionResource(depthRes, depthState, D3D12_RESOURCE_STATE_COPY_SOURCE);

		D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
		dstLoc.pResource = rbBuf.Get();
		dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLoc.PlacedFootprint = fp;

		D3D12_TEXTURE_COPY_LOCATION srcLoc = { depthRes, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
		cmd->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

		TransitionResource(depthRes, depthState, prev);
		ExecuteAndSyncGPU();

		void* mapped = nullptr;
		if (FAILED(rbBuf->Map(0, nullptr, &mapped)) || !mapped)
			return false;

		const uint8_t* row = static_cast<const uint8_t*>(mapped) + static_cast<size_t>(y) * fp.Footprint.RowPitch;
		if (copyDesc.Format == DXGI_FORMAT_D32_FLOAT)
		{
			std::memcpy(&outDepth, row + static_cast<size_t>(x) * 4u, sizeof(float));
		}
		else if (copyDesc.Format == DXGI_FORMAT_D16_UNORM)
		{
			uint16_t d16 = 0;
			std::memcpy(&d16, row + static_cast<size_t>(x) * 2u, sizeof(uint16_t));
			outDepth = static_cast<float>(d16) / 65535.0f;
		}
		else
		{
			uint32_t d24 = 0;
			std::memcpy(&d24, row + static_cast<size_t>(x) * 4u, sizeof(uint32_t));
			outDepth = static_cast<float>(d24 & 0x00FFFFFFu) / 16777215.0f;
		}

		D3D12_RANGE emptyWrite = { 0, 0 };
		rbBuf->Unmap(0, &emptyWrite);
		return true;
	}

	bool RenderDeviceD3D12::ReadTexturePixels(IRHITexture* texture, void* pDataOut)
	{
		if (!m_device || !texture || !pDataOut)
			return false;

		auto* d3dTex = static_cast<D3D12Texture*>(texture->GetNativeHandle());
		if (!d3dTex)
			return false;
		ID3D12Resource* srcRes = d3dTex->GetResource();
		if (!srcRes)
			return false;

		D3D12_RESOURCE_DESC td = srcRes->GetDesc();
		if (td.Width == 0 || td.Height == 0 || td.SampleDesc.Count > 1 || td.Format == DXGI_FORMAT_BC7_UNORM)
			return false;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = {};
		UINT numRows = 0;
		UINT64 rowBytes = 0, totalBytes = 0;
		m_device->GetCopyableFootprints(&td, 0, 1, 0, &fp, &numRows, &rowBytes, &totalBytes);
		if (totalBytes == 0)
			return false;

		D3D12_HEAP_PROPERTIES rbHeap = {};
		rbHeap.Type = D3D12_HEAP_TYPE_READBACK;
		D3D12_RESOURCE_DESC rbDesc = {};
		rbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rbDesc.Width = totalBytes;
		rbDesc.Height = 1;
		rbDesc.DepthOrArraySize = 1;
		rbDesc.MipLevels = 1;
		rbDesc.SampleDesc.Count = 1;
		rbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ComPtr<ID3D12Resource> rbBuf;
		if (FAILED(m_device->CreateCommittedResource(&rbHeap, D3D12_HEAP_FLAG_NONE, &rbDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rbBuf))))
			return false;

		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd)
			return false;

		D3D12_RESOURCE_STATES prev = d3dTex->GetState();
		TransitionResource(srcRes, d3dTex->GetStateRef(), D3D12_RESOURCE_STATE_COPY_SOURCE);

		D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
		dstLoc.pResource = rbBuf.Get();
		dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLoc.PlacedFootprint = fp;

		D3D12_TEXTURE_COPY_LOCATION srcLoc = { srcRes, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
		cmd->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

		TransitionResource(srcRes, d3dTex->GetStateRef(), prev);
		ExecuteAndSyncGPU();

		void* mapped = nullptr;
		if (FAILED(rbBuf->Map(0, nullptr, &mapped)) || !mapped)
			return false;

		uint8_t* dst = static_cast<uint8_t*>(pDataOut);
		const uint8_t* src = static_cast<const uint8_t*>(mapped);
		// A BC footprint contains rows of 4x4 blocks, padded to RowPitch.
		// Unpack only the block bytes before converting the image for CPU callers.
		if (td.Format == DXGI_FORMAT_BC1_UNORM || td.Format == DXGI_FORMAT_BC2_UNORM || td.Format == DXGI_FORMAT_BC3_UNORM)
		{
			std::vector<uint8_t> blocks(static_cast<size_t>(numRows * rowBytes));
			for (UINT y = 0; y < numRows; ++y)
				std::memcpy(blocks.data() + y * rowBytes, src + static_cast<size_t>(y) * fp.Footprint.RowPitch, static_cast<size_t>(rowBytes));
			D3D12_RANGE emptyWrite = {};
			rbBuf->Unmap(0, &emptyWrite);
			std::vector<uint8_t> rgba;
			if (!RHISoftwareS3TC::DecodeToRGBA8(d3dTex->GetFormat(), blocks.data(), blocks.size(), d3dTex->GetWidth(), td.Height, rgba))
				return false;
			std::memcpy(dst, rgba.data(), rgba.size());
			return true;
		}
		const bool isBGRA = (td.Format == DXGI_FORMAT_B8G8R8A8_UNORM || td.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || td.Format == DXGI_FORMAT_B8G8R8X8_UNORM);
		const bool isA8 = (td.Format == DXGI_FORMAT_A8_UNORM || td.Format == DXGI_FORMAT_R8_UNORM);

		for (UINT y = 0; y < td.Height; ++y)
		{
			const uint8_t* srcRow = src + static_cast<size_t>(y) * fp.Footprint.RowPitch;
			uint8_t* dstRow = dst + static_cast<size_t>(y) * static_cast<size_t>(td.Width) * 4u;
			if (isBGRA)
			{
				for (UINT64 x = 0; x < td.Width; ++x)
				{
					dstRow[x * 4u + 0] = srcRow[x * 4u + 2];
					dstRow[x * 4u + 1] = srcRow[x * 4u + 1];
					dstRow[x * 4u + 2] = srcRow[x * 4u + 0];
					dstRow[x * 4u + 3] = (td.Format == DXGI_FORMAT_B8G8R8X8_UNORM) ? 255u : srcRow[x * 4u + 3];
				}
			}
			else if (isA8)
			{
				for (UINT64 x = 0; x < td.Width; ++x)
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

		D3D12_RANGE emptyWrite = { 0, 0 };
		rbBuf->Unmap(0, &emptyWrite);
		return true;
	}

	// =========================================================================
	// Shaders, PSO Cache, Descriptors & Draw Commands
	// =========================================================================

	IRHIShaderProgram* RenderDeviceD3D12::CompileShaderProgram(const RHIShaderProgramDesc& desc)
	{
		if (!m_device)
			return nullptr;
		auto* prog = new D3D12ShaderProgram(this, desc);
		if (!prog->IsValid())
		{
			prog->ReleaseRHIResource();
			return nullptr;
		}
		return prog;
	}

	bool RenderDeviceD3D12::BindShaderProgram(IRHIShaderProgram* program)
	{
		m_currentProgram = static_cast<D3D12ShaderProgram*>(program);
		return true;
	}

	bool RenderDeviceD3D12::BindDrawConstants(ID3D12GraphicsCommandList* cmd, UINT slot, const void* data, uint32_t size,
		std::vector<uint8_t>& shadow, D3D12_GPU_VIRTUAL_ADDRESS& address)
	{
		if (!data || !size)
		{
			if (!m_cbvDirtyAfterReset || shadow.empty()) return true;
			data = shadow.data();
			size = static_cast<uint32_t>(shadow.size());
		}
		// The address belongs to this upload arena until the frame slot is reclaimed.
		const bool same = shadow.size() == size && (shadow.data() == data || std::memcmp(shadow.data(), data, size) == 0);
		if (!same || !address || address == m_zeroCbvGpuVA)
		{
			const uint32_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
			const uint32_t aligned = static_cast<uint32_t>(AlignUpU64(size, alignment));
			void* cpu = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS gpu = 0;
			ID3D12Resource* resource = nullptr;
			uint64_t offset = 0;
			if (!AllocateUpload(aligned, alignment, cpu, gpu, resource, offset)) return false;
			std::memcpy(cpu, data, size);
			if (aligned > size) std::memset(static_cast<uint8_t*>(cpu) + size, 0, aligned - size);
			if (!same)
			{
				const auto* bytes = static_cast<const uint8_t*>(data);
				shadow.assign(bytes, bytes + size);
			}
			address = gpu;
		}
		cmd->SetGraphicsRootConstantBufferView(slot, address);
		return true;
	}

	bool RenderDeviceD3D12::SetShaderConstants(const void* vsConstants, uint32_t vsByteSize, const void* psConstants, uint32_t psByteSize)
	{
		auto* cmd = EnsureRecording();
		if (!cmd || !BindDrawConstants(cmd, 0, vsConstants, vsByteSize, m_shadowVsCbv, m_currentVsCbvGpuVA)
			|| !BindDrawConstants(cmd, 1, psConstants, psByteSize, m_shadowPsCbv, m_currentPsCbvGpuVA))
			return false;
		m_cbvDirtyAfterReset = false;
		return true;
	}

	uint32_t RenderDeviceD3D12::GetOrCreateCpuSampler(uint32_t stage)
	{
		const uint32_t addrU = m_stateTracker.GetSamplerState(stage, ESamplerStateType::ADDRESSU);
		const uint32_t addrV = m_stateTracker.GetSamplerState(stage, ESamplerStateType::ADDRESSV);
		const uint32_t addrW = m_stateTracker.GetSamplerState(stage, ESamplerStateType::ADDRESSW);
		const uint32_t minF  = m_stateTracker.GetEffectiveSamplerState(stage, ESamplerStateType::MINFILTER);
		const uint32_t magF  = m_stateTracker.GetEffectiveSamplerState(stage, ESamplerStateType::MAGFILTER);
		const uint32_t mipF  = m_stateTracker.GetSamplerState(stage, ESamplerStateType::MIPFILTER);
		const uint32_t aniso = m_stateTracker.GetEffectiveSamplerState(stage, ESamplerStateType::MAXANISOTROPY);
		const uint32_t border = m_stateTracker.GetSamplerState(stage, ESamplerStateType::BORDERCOLOR);
		const uint32_t biasBits = m_stateTracker.GetSamplerState(stage, ESamplerStateType::MIPMAPLODBIAS);

		uint64_t h = 14695981039346656037ULL;
		HashCombineU32(h, addrU | (addrV << 8) | (addrW << 16));
		HashCombineU32(h, minF | (magF << 8) | (mipF << 16));
		HashCombineU32(h, aniso);
		HashCombineU32(h, border);
		HashCombineU32(h, biasBits);

		auto it = m_cpuSamplerCache.find(h);
		if (it != m_cpuSamplerCache.end())
			return it->second;

		D3D12_SAMPLER_DESC sd = {};
		sd.AddressU = ToD3D12AddressMode(addrU);
		sd.AddressV = ToD3D12AddressMode(addrV);
		sd.AddressW = ToD3D12AddressMode(addrW);

		if (minF == 3 || magF == 3 || aniso > 1)
		{
			sd.Filter = D3D12_FILTER_ANISOTROPIC;
		}
		else
		{
			const bool minLin = (minF != 1);
			const bool magLin = (magF != 1);
			const bool mipLin = (mipF != 1 && mipF != 0);
			if (!minLin && !magLin && !mipLin) sd.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			else if (!minLin && !magLin && mipLin) sd.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			else if (!minLin && magLin && !mipLin) sd.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			else if (!minLin && magLin && mipLin)  sd.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			else if (minLin && !magLin && !mipLin) sd.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			else if (minLin && !magLin && mipLin)  sd.Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			else if (minLin && magLin && !mipLin)  sd.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			else                                   sd.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}

		float lodBias = 0.0f;
		std::memcpy(&lodBias, &biasBits, sizeof(float));
		sd.MipLODBias = lodBias;
		sd.MaxAnisotropy = (std::min)((std::max)(aniso, 1u), m_caps.maxAnisotropy);
		sd.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sd.BorderColor[0] = static_cast<float>((border >> 16) & 0xFFu) / 255.0f;
		sd.BorderColor[1] = static_cast<float>((border >> 8) & 0xFFu) / 255.0f;
		sd.BorderColor[2] = static_cast<float>(border & 0xFFu) / 255.0f;
		sd.BorderColor[3] = static_cast<float>((border >> 24) & 0xFFu) / 255.0f;
		sd.MinLOD = 0.0f;
		sd.MaxLOD = (mipF == 0) ? 0.0f : D3D12_FLOAT32_MAX;

		uint32_t slot = m_cpuSamplerAlloc.Allocate();
		if (slot == UINT32_MAX)
			return 0;
		m_device->CreateSampler(&sd, m_cpuSamplerAlloc.GetHandle(slot));
		m_cpuSamplerCache[h] = slot;
		return slot;
	}

	ID3D12PipelineState* RenderDeviceD3D12::GetOrCreatePSO(D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType)
	{
		if (!m_device || !m_currentProgram || !m_currentVertexDecl)
			return nullptr;

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

		const uint32_t cullMode   = m_stateTracker.GetRenderState(ERenderState::CULLMODE);
		const uint32_t fillMode   = m_stateTracker.GetRenderState(ERenderState::FILLMODE);
		const uint32_t biasBits   = m_stateTracker.GetRenderState(ERenderState::DEPTHBIAS);
		const uint32_t slopeBits  = m_stateTracker.GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS);
		const uint32_t msaaEn     = m_stateTracker.GetRenderState(ERenderState::MULTISAMPLEANTIALIAS);

		const uint32_t zEnable     = m_hasActiveDSV ? m_stateTracker.GetRenderState(ERenderState::ZENABLE) : 0u;
		const uint32_t zWrite      = m_hasActiveDSV ? m_stateTracker.GetRenderState(ERenderState::ZWRITEENABLE) : 0u;
		const uint32_t zFunc       = m_stateTracker.GetRenderState(ERenderState::ZFUNC);
		const uint32_t stEnable    = m_hasActiveDSV ? m_stateTracker.GetRenderState(ERenderState::STENCILENABLE) : 0u;
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

		uint64_t h = 14695981039346656037ULL;
		HashCombineU64(h, m_currentProgram->GetVSHash());
		HashCombineU64(h, m_currentProgram->GetPSHash());
		HashCombineU64(h, m_currentVertexDecl->GetHash());
		for (uint32_t s = 0; s < 4; ++s)
			HashCombineU32(h, m_streams[s].frequencyDivider);
		HashCombineU32(h, static_cast<uint32_t>(topoType));
		HashCombineU32(h, m_activeRTVCount | (m_activeSampleCount << 8) | (m_activeNullRtvMask << 16));
		for (uint32_t i = 0; i < 4; ++i)
			HashCombineU32(h, static_cast<uint32_t>(m_activeRtvFormats[i]));
		HashCombineU32(h, static_cast<uint32_t>(m_hasActiveDSV ? m_activeDsvFormat : DXGI_FORMAT_UNKNOWN));

		HashCombineU32(h, alphaBlend | (sepAlpha << 8));
		HashCombineU32(h, srcBlend | (dstBlend << 8) | (blendOp << 16));
		HashCombineU32(h, srcBlendA | (dstBlendA << 8) | (blendOpA << 16));
		const uint8_t psOutMask = m_currentProgram->GetPSOutputMask();
		HashCombineU32(h, cw0 | (cw1 << 4) | (cw2 << 8) | (cw3 << 12) | (static_cast<uint32_t>(psOutMask) << 16));
		HashCombineU32(h, cullMode | (fillMode << 8) | (msaaEn << 16));
		HashCombineU32(h, biasBits);
		HashCombineU32(h, slopeBits);
		HashCombineU32(h, zEnable | (zWrite << 2) | (zFunc << 4) | (stEnable << 12) | (twoSided << 14));
		HashCombineU32(h, stMask | (stWriteMask << 8));
		HashCombineU32(h, stFunc | (stFail << 8) | (stZFail << 16) | (stPass << 24));
		HashCombineU32(h, ccwFunc | (ccwFail << 8) | (ccwZFail << 16) | (ccwPass << 24));

		auto it = m_psoCache.find(h);
		if (it != m_psoCache.end())
			return it->second.Get();

		std::vector<D3D12_INPUT_ELEMENT_DESC> elems;
		elems.reserve(m_currentVertexDecl->GetElements().size() + 4);
		for (const auto& ve : m_currentVertexDecl->GetElements())
		{
			if (ve.IsEndDeclare())
				continue;

			D3D12_INPUT_ELEMENT_DESC d = {};
			d.SemanticName = MapDeclUsageToSemantic12(ve.Usage, m_currentProgram->UsesSVPosition() && ve.UsageIndex == 0);
			d.SemanticIndex = ve.UsageIndex;
			d.Format = MapDeclTypeToDXGI12(ve.Type);
			d.InputSlot = ve.Stream;
			d.AlignedByteOffset = ve.Offset;

			uint32_t freq = (ve.Stream < 4) ? m_streams[ve.Stream].frequencyDivider : 0u;
			if (ve.Stream > 0 && freq != 0)
			{
				uint32_t step = (freq & 0xC0000000u) ? (freq & 0x3FFFFFFFu) : freq;
				if (step == 0) step = 1;
				d.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				d.InstanceDataStepRate = step;
			}
			else
			{
				d.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				d.InstanceDataStepRate = 0;
			}
			elems.push_back(d);
		}

		for (const auto& req : m_currentProgram->GetInputSignature())
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
				D3D12_INPUT_ELEMENT_DESC fallback = {};
				fallback.SemanticName = req.semanticName.c_str();
				fallback.SemanticIndex = req.semanticIndex;
				if (req.componentType == D3D_REGISTER_COMPONENT_UINT32)
					fallback.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (req.componentType == D3D_REGISTER_COMPONENT_SINT32)
					fallback.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else
					fallback.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				fallback.InputSlot = 15;
				fallback.AlignedByteOffset = 0;
				fallback.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				fallback.InstanceDataStepRate = 0;
				elems.push_back(fallback);
			}
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = { m_currentProgram->GetVSBytecode().data(), m_currentProgram->GetVSBytecode().size() };
		if (!m_currentProgram->GetPSBytecode().empty())
		{
			psoDesc.PS = { m_currentProgram->GetPSBytecode().data(), m_currentProgram->GetPSBytecode().size() };
		}

		const uint8_t activeTargetMask = (m_activeRTVCount >= 8) ? 0xFFu : static_cast<uint8_t>((1u << m_activeRTVCount) - 1u);
		const bool hasUnwrittenActiveTarget = ((psOutMask & activeTargetMask) != activeTargetMask);
		psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		psoDesc.BlendState.IndependentBlendEnable = (cw1 != cw0 || cw2 != cw0 || cw3 != cw0 || m_activeNullRtvMask != 0 || hasUnwrittenActiveTarget);
		const uint8_t masks[8] = {
			static_cast<uint8_t>(cw0), static_cast<uint8_t>(cw1), static_cast<uint8_t>(cw2), static_cast<uint8_t>(cw3),
			static_cast<uint8_t>(cw0), static_cast<uint8_t>(cw0), static_cast<uint8_t>(cw0), static_cast<uint8_t>(cw0)
		};
		for (int i = 0; i < 8; ++i)
		{
			const bool isNullSlot = (i < 4) && ((m_activeNullRtvMask & (1u << i)) != 0);
			const bool psWritesSlot = !psoDesc.BlendState.IndependentBlendEnable || ((psOutMask & (1u << i)) != 0);
			const bool disableSlot = isNullSlot || !psWritesSlot;
			psoDesc.BlendState.RenderTarget[i].BlendEnable = (!disableSlot && alphaBlend) ? TRUE : FALSE;
			psoDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
			psoDesc.BlendState.RenderTarget[i].SrcBlend = ToD3D12Blend(srcBlend, false);
			psoDesc.BlendState.RenderTarget[i].DestBlend = ToD3D12Blend(dstBlend, false);
			psoDesc.BlendState.RenderTarget[i].BlendOp = ToD3D12BlendOp(blendOp);
			psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = sepAlpha ? ToD3D12Blend(srcBlendA, true) : ToD3D12Blend(srcBlend, true);
			psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = sepAlpha ? ToD3D12Blend(dstBlendA, true) : ToD3D12Blend(dstBlend, true);
			psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = sepAlpha ? ToD3D12BlendOp(blendOpA) : ToD3D12BlendOp(blendOp);
			psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
			psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = disableSlot ? 0 : (psoDesc.BlendState.IndependentBlendEnable ? masks[i] : masks[0]);
		}
		psoDesc.SampleMask = UINT_MAX;

		float fBias = 0.0f, fSlope = 0.0f;
		std::memcpy(&fBias, &biasBits, sizeof(float));
		std::memcpy(&fSlope, &slopeBits, sizeof(float));
		psoDesc.RasterizerState.FillMode = (fillMode == RSV_FILL_WIREFRAME) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		if (cullMode == RSV_CULL_NONE) psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		else if (cullMode == RSV_CULL_CW) psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
		else psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
		psoDesc.RasterizerState.DepthBias = static_cast<INT>(fBias * 16777215.0f);
		psoDesc.RasterizerState.SlopeScaledDepthBias = fSlope;
		psoDesc.RasterizerState.DepthClipEnable = TRUE;
		psoDesc.RasterizerState.MultisampleEnable = (m_activeSampleCount > 1 && msaaEn) ? TRUE : FALSE;

		psoDesc.DepthStencilState.DepthEnable = zEnable ? TRUE : FALSE;
		psoDesc.DepthStencilState.DepthWriteMask = zWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.DepthFunc = ToD3D12CmpFunc(zFunc);
		psoDesc.DepthStencilState.StencilEnable = stEnable ? TRUE : FALSE;
		psoDesc.DepthStencilState.StencilReadMask = static_cast<UINT8>(stMask);
		psoDesc.DepthStencilState.StencilWriteMask = static_cast<UINT8>(stWriteMask);
		psoDesc.DepthStencilState.FrontFace.StencilFunc = ToD3D12CmpFunc(stFunc);
		psoDesc.DepthStencilState.FrontFace.StencilFailOp = ToD3D12StencilOp(stFail);
		psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = ToD3D12StencilOp(stZFail);
		psoDesc.DepthStencilState.FrontFace.StencilPassOp = ToD3D12StencilOp(stPass);
		if (twoSided)
		{
			psoDesc.DepthStencilState.BackFace.StencilFunc = ToD3D12CmpFunc(ccwFunc);
			psoDesc.DepthStencilState.BackFace.StencilFailOp = ToD3D12StencilOp(ccwFail);
			psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = ToD3D12StencilOp(ccwZFail);
			psoDesc.DepthStencilState.BackFace.StencilPassOp = ToD3D12StencilOp(ccwPass);
		}
		else
		{
			psoDesc.DepthStencilState.BackFace = psoDesc.DepthStencilState.FrontFace;
		}

		psoDesc.InputLayout = { elems.data(), static_cast<UINT>(elems.size()) };
		psoDesc.PrimitiveTopologyType = topoType;
		psoDesc.NumRenderTargets = m_activeRTVCount;
		for (uint32_t i = 0; i < m_activeRTVCount && i < 4; ++i)
			psoDesc.RTVFormats[i] = m_activeRtvFormats[i];
		psoDesc.DSVFormat = m_hasActiveDSV ? m_activeDsvFormat : DXGI_FORMAT_UNKNOWN;
		psoDesc.SampleDesc.Count = m_activeSampleCount;
		psoDesc.SampleDesc.Quality = 0;

		ComPtr<ID3D12PipelineState> pso;
		HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
		if (FAILED(hr))
		{
			const auto& vsb = m_currentProgram->GetVSBytecode();
			const auto& psb = m_currentProgram->GetPSBytecode();
			uint32_t vsSig = 0, psSig = 0;
			if (vsb.size() >= 8) std::memcpy(&vsSig, vsb.data() + 4, 4);
			if (psb.size() >= 8) std::memcpy(&psSig, psb.data() + 4, 4);
			OUTPUT_LOG("WARN: D3D12 CreateGraphicsPipelineState failed (hr=0x%08X, elems=%zu, rtvs=%u, rtv0=%d, dsv=%d, vsSz=%zu vsDig=0x%08X, psSz=%zu psDig=0x%08X)\n",
				static_cast<unsigned int>(hr), elems.size(), m_activeRTVCount,
				static_cast<int>(psoDesc.RTVFormats[0]), static_cast<int>(psoDesc.DSVFormat),
				vsb.size(), vsSig, psb.size(), psSig);
			for (size_t ei = 0; ei < elems.size(); ++ei)
			{
				OUTPUT_LOG("  PSO Elem[%zu]: sem=%s%u fmt=%d slot=%u off=%u cls=%d step=%u\n",
					ei, elems[ei].SemanticName ? elems[ei].SemanticName : "<null>",
					elems[ei].SemanticIndex, static_cast<int>(elems[ei].Format),
					elems[ei].InputSlot, elems[ei].AlignedByteOffset,
					static_cast<int>(elems[ei].InputSlotClass), elems[ei].InstanceDataStepRate);
			}
			ComPtr<ID3D12InfoQueue> infoQ;
			if (SUCCEEDED(m_device.As(&infoQ)) && infoQ)
			{
				UINT64 numMsgs = infoQ->GetNumStoredMessages();
				for (UINT64 mi = (numMsgs > 8 ? numMsgs - 8 : 0); mi < numMsgs; ++mi)
				{
					SIZE_T msgLen = 0;
					infoQ->GetMessage(mi, nullptr, &msgLen);
					if (msgLen > 0)
					{
						std::vector<uint8_t> buf(msgLen);
						auto* msg = reinterpret_cast<D3D12_MESSAGE*>(buf.data());
						if (SUCCEEDED(infoQ->GetMessage(mi, msg, &msgLen)) && msg->pDescription)
							OUTPUT_LOG("  D3D12Debug: %s\n", msg->pDescription);
					}
				}
			}
		}
		m_psoCache[h] = pso;
		return pso.Get();
	}

	bool RenderDeviceD3D12::PrepareForDraw(D3D12_PRIMITIVE_TOPOLOGY topo, D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType)
	{
		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd || !m_currentProgram || !m_currentVertexDecl)
			return false;

		ID3D12PipelineState* pso = GetOrCreatePSO(topoType);
		if (!pso)
			return false;

		// Ensure active RTVs / DSV are in renderable state
		BindActiveRenderTargetsToCmdList();

		// Transition bound SRV textures & populate 16-SRV GPU descriptor table
		const uint32_t fallbackSrvSlot = m_whiteFallbackTex ? m_whiteFallbackTex->GetSrvSlot() : 0u;
		uint32_t resolvedSrvSlots[RHIStateTracker::MAX_SAMPLER_STAGES] = {};
		for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
		{
			D3D12Texture* tex = m_boundTextures[s];
			bool isConflict = false;
			if (tex)
			{
				for (uint32_t r = 0; r < m_activeRTVCount; ++r)
				{
					if (m_activeColorTexObjs[r] == tex)
					{
						isConflict = true;
						break;
					}
				}
				if (m_hasActiveDSV && m_activeDepthTexObj == tex)
					isConflict = true;
			}

			if (tex && !isConflict && tex->GetResource() && tex->GetSrvSlot() != UINT32_MAX)
			{
				TransitionResource(tex->GetResource(), tex->GetStateRef(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				resolvedSrvSlots[s] = tex->GetSrvSlot();
			}
			else
			{
				resolvedSrvSlots[s] = fallbackSrvSlot;
			}
		}

		if (m_srvTableDirty)
		{
			const uint32_t frameEnd = m_gpuSrvFrameBase[m_frameIndex] + 32768u;
			if (m_gpuSrvOffset + RHIStateTracker::MAX_SAMPLER_STAGES > frameEnd)
			{
				ExecuteAndSyncGPU();
				cmd = EnsureRecording();
				BindActiveRenderTargetsToCmdList();
			}

			const uint32_t baseIdx = m_gpuSrvOffset;
			m_gpuSrvOffset += RHIStateTracker::MAX_SAMPLER_STAGES;

			D3D12_CPU_DESCRIPTOR_HANDLE dstCpu = m_gpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
			dstCpu.ptr += static_cast<SIZE_T>(baseIdx) * m_gpuSrvDescriptorSize;

			// Each source range is one descriptor. The destination table is contiguous.
			constexpr UINT count = RHIStateTracker::MAX_SAMPLER_STAGES;
			D3D12_CPU_DESCRIPTOR_HANDLE sources[count];
			for (UINT s = 0; s < count; ++s)
				sources[s] = m_cpuSrvAlloc.GetHandle(resolvedSrvSlots[s]);
			m_device->CopyDescriptors(1, &dstCpu, &count, count, sources, nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			m_lastGpuSrvTable = m_gpuSrvHeap->GetGPUDescriptorHandleForHeapStart();
			m_lastGpuSrvTable.ptr += static_cast<UINT64>(baseIdx) * m_gpuSrvDescriptorSize;
			m_srvTableDirty = false;
		}
		cmd->SetGraphicsRootDescriptorTable(2, m_lastGpuSrvTable);

		// Populate / lookup 16-Sampler GPU descriptor table
		const uint32_t dirtySamplers = m_stateTracker.GetDirtySamplerMask();
		if (dirtySamplers || m_samplerTableBase == UINT32_MAX)
		{
			uint64_t sampComboHash = 14695981039346656037ULL;
			for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
			{
				if (dirtySamplers & (1u << s))
				{
					m_samplerSlots[s] = GetOrCreateCpuSampler(s);
					m_stateTracker.ClearSamplerDirty(s);
				}
				HashCombineU32(sampComboHash, m_samplerSlots[s]);
			}

			auto sampIt = m_gpuSamplerTableCache.find(sampComboHash);
			if (sampIt != m_gpuSamplerTableCache.end())
			{
				m_samplerTableBase = sampIt->second;
			}
			else
			{
				if (m_gpuSamplerOffset + RHIStateTracker::MAX_SAMPLER_STAGES > m_gpuSamplerCapacity)
				{
					ExecuteAndSyncGPU();
					cmd = EnsureRecording();
					BindActiveRenderTargetsToCmdList();
					cmd->SetGraphicsRootDescriptorTable(2, m_lastGpuSrvTable);
					m_gpuSamplerTableCache.clear();
					m_gpuSamplerOffset = 0;
				}

				m_samplerTableBase = m_gpuSamplerOffset;
				m_gpuSamplerOffset += RHIStateTracker::MAX_SAMPLER_STAGES;
				D3D12_CPU_DESCRIPTOR_HANDLE dstCpu = m_gpuSamplerHeap->GetCPUDescriptorHandleForHeapStart();
				dstCpu.ptr += static_cast<SIZE_T>(m_samplerTableBase) * m_gpuSamplerDescriptorSize;
				for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE srcCpu = m_cpuSamplerAlloc.GetHandle(m_samplerSlots[s]);
					m_device->CopyDescriptorsSimple(1, dstCpu, srcCpu, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
					dstCpu.ptr += m_gpuSamplerDescriptorSize;
				}
				m_gpuSamplerTableCache[sampComboHash] = m_samplerTableBase;
			}
		}

		D3D12_GPU_DESCRIPTOR_HANDLE gpuSampHandle = m_gpuSamplerHeap->GetGPUDescriptorHandleForHeapStart();
		gpuSampHandle.ptr += static_cast<UINT64>(m_samplerTableBase) * m_gpuSamplerDescriptorSize;
		cmd->SetGraphicsRootDescriptorTable(3, gpuSampHandle);

		if (m_cbvDirtyAfterReset && !SetShaderConstants(nullptr, 0, nullptr, 0))
			return false;

		cmd->SetPipelineState(pso);
		cmd->IASetPrimitiveTopology(topo);

		uint32_t bfPacked = m_stateTracker.GetRenderState(ERenderState::BLENDFACTOR);
		const float blendFactor[4] = {
			static_cast<float>((bfPacked >> 16) & 0xFFu) / 255.0f,
			static_cast<float>((bfPacked >> 8) & 0xFFu) / 255.0f,
			static_cast<float>(bfPacked & 0xFFu) / 255.0f,
			static_cast<float>((bfPacked >> 24) & 0xFFu) / 255.0f
		};
		cmd->OMSetBlendFactor(blendFactor);
		cmd->OMSetStencilRef(m_stateTracker.GetRenderState(ERenderState::STENCILREF));

		for (uint32_t s = 0; s < 4; ++s)
		{
			if (m_streams[s].buffer)
			{
				D3D12_VERTEX_BUFFER_VIEW vbv = {};
				vbv.BufferLocation = m_streams[s].buffer->GetGpuVA() + m_streams[s].offsetInBytes;
				vbv.SizeInBytes = (m_streams[s].buffer->GetSize() > m_streams[s].offsetInBytes)
					? (m_streams[s].buffer->GetSize() - m_streams[s].offsetInBytes)
					: 0;
				vbv.StrideInBytes = m_streams[s].stride;
				cmd->IASetVertexBuffers(s, 1, &vbv);
			}
		}
		cmd->IASetVertexBuffers(15, 1, &m_defaultAttrVBV);

		if (m_currentIndexBuffer)
		{
			D3D12_INDEX_BUFFER_VIEW ibv = {};
			ibv.BufferLocation = m_currentIndexBuffer->GetGpuVA();
			ibv.SizeInBytes = m_currentIndexBuffer->GetSize();
			ibv.Format = DXGI_FORMAT_R16_UINT;
			cmd->IASetIndexBuffer(&ibv);
		}

		return true;
	}

	bool RenderDeviceD3D12::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		if (PrimitiveCount == 0)
			return false;

		D3D12_PRIMITIVE_TOPOLOGY topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		uint32_t vertCount = 0;
		if (!ComputePrimitiveTopologyAndCount(PrimitiveType, PrimitiveCount, topo, topoType, vertCount))
			return false;

		if (!PrepareForDraw(topo, topoType))
			return false;

		ID3D12GraphicsCommandList* cmd = m_cmdList.Get();
		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			uint32_t idxBytes = PrimitiveCount * 3u * sizeof(uint16_t);
			void* upCpu = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS upGpu = 0;
			ID3D12Resource* upRes = nullptr;
			uint64_t upOff = 0;
			if (!AllocateUpload(idxBytes, 4, upCpu, upGpu, upRes, upOff))
				return false;
			uint16_t* dst = static_cast<uint16_t*>(upCpu);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				dst[i * 3u + 0] = 0;
				dst[i * 3u + 1] = static_cast<uint16_t>(i + 1u);
				dst[i * 3u + 2] = static_cast<uint16_t>(i + 2u);
			}
			D3D12_INDEX_BUFFER_VIEW ibv = { upGpu, idxBytes, DXGI_FORMAT_R16_UINT };
			cmd->IASetIndexBuffer(&ibv);
			m_currentIndexBuffer = nullptr;
			cmd->DrawIndexedInstanced(PrimitiveCount * 3u, 1, 0, static_cast<INT>(StartVertex), 0);
			RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
			return true;
		}

		cmd->DrawInstanced(vertCount, 1, StartVertex, 0);
		RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
		return true;
	}

	bool RenderDeviceD3D12::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t /*MinIndex*/, uint32_t /*NumVertices*/, uint32_t indexStart, uint32_t PrimitiveCount)
	{
		if (PrimitiveCount == 0)
			return false;

		if ((m_streams[0].frequencyDivider & 0x40000000u) != 0)
		{
			uint32_t instanceCount = m_streams[0].frequencyDivider & 0x3FFFFFFFu;
			if (instanceCount > 0)
				return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, instanceCount);
		}

		return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, 1);
	}

	bool RenderDeviceD3D12::DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount)
	{
		if (primitiveCount == 0 || instanceCount == 0)
			return false;

		D3D12_PRIMITIVE_TOPOLOGY topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		uint32_t indexCount = 0;
		if (!ComputePrimitiveTopologyAndCount(primitiveType, primitiveCount, topo, topoType, indexCount))
			return false;

		if (!PrepareForDraw(topo, topoType))
			return false;

		m_cmdList->DrawIndexedInstanced(indexCount, instanceCount, indexStart, baseVertexIndex, 0);
		RenderDevice::RecordDraw(primitiveType, primitiveCount, instanceCount);
		return true;
	}

	bool RenderDeviceD3D12::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		if (!pVertexStreamZeroData || VertexStreamZeroStride == 0 || PrimitiveCount == 0)
			return false;

		D3D12_PRIMITIVE_TOPOLOGY topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		uint32_t vertCount = 0;
		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			topoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			vertCount = PrimitiveCount + 2u;
		}
		else if (!ComputePrimitiveTopologyAndCount(PrimitiveType, PrimitiveCount, topo, topoType, vertCount))
		{
			return false;
		}

		uint32_t vbBytes = vertCount * VertexStreamZeroStride;
		void* vbCpu = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS vbGpu = 0;
		ID3D12Resource* vbRes = nullptr;
		uint64_t vbOff = 0;
		if (!AllocateUpload(vbBytes, 16, vbCpu, vbGpu, vbRes, vbOff))
			return false;
		std::memcpy(vbCpu, pVertexStreamZeroData, vbBytes);

		m_streams[0].buffer = nullptr;
		if (!PrepareForDraw(topo, topoType))
			return false;

		D3D12_VERTEX_BUFFER_VIEW vbv = { vbGpu, vbBytes, VertexStreamZeroStride };
		m_cmdList->IASetVertexBuffers(0, 1, &vbv);

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			uint32_t idxBytes = PrimitiveCount * 3u * sizeof(uint16_t);
			void* ibCpu = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS ibGpu = 0;
			ID3D12Resource* ibRes = nullptr;
			uint64_t ibOff = 0;
			if (!AllocateUpload(idxBytes, 4, ibCpu, ibGpu, ibRes, ibOff))
				return false;
			uint16_t* dst = static_cast<uint16_t*>(ibCpu);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				dst[i * 3u + 0] = 0;
				dst[i * 3u + 1] = static_cast<uint16_t>(i + 1u);
				dst[i * 3u + 2] = static_cast<uint16_t>(i + 2u);
			}
			D3D12_INDEX_BUFFER_VIEW ibv = { ibGpu, idxBytes, DXGI_FORMAT_R16_UINT };
			m_cmdList->IASetIndexBuffer(&ibv);
			m_currentIndexBuffer = nullptr;
			m_cmdList->DrawIndexedInstanced(PrimitiveCount * 3u, 1, 0, 0, 0);
			RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
			return true;
		}

		m_cmdList->DrawInstanced(vertCount, 1, 0, 0);
		RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
		return true;
	}

	bool RenderDeviceD3D12::DrawIndexedPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t MinVertexIndex,
		uint32_t NumVertices,
		uint32_t PrimitiveCount,
		const void* pIndexData,
		PixelFormat IndexDataFormat,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!pIndexData || !pVertexStreamZeroData || VertexStreamZeroStride == 0 || PrimitiveCount == 0)
			return false;

		D3D12_PRIMITIVE_TOPOLOGY topo = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topoType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		uint32_t indexCount = 0;
		if (!ComputePrimitiveTopologyAndCount(PrimitiveType, PrimitiveCount, topo, topoType, indexCount))
			return false;

		const bool isIdx32 = (IndexDataFormat == PixelFormat::INDEX32);
		const uint32_t idxElemBytes = isIdx32 ? 4u : 2u;
		const uint32_t vbBytes = (MinVertexIndex + NumVertices) * VertexStreamZeroStride;
		const uint32_t ibBytes = indexCount * idxElemBytes;

		void* vbCpu = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS vbGpu = 0;
		ID3D12Resource* vbRes = nullptr;
		uint64_t vbOff = 0;
		if (!AllocateUpload(vbBytes, 16, vbCpu, vbGpu, vbRes, vbOff))
			return false;
		std::memcpy(vbCpu, pVertexStreamZeroData, vbBytes);

		void* ibCpu = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS ibGpu = 0;
		ID3D12Resource* ibRes = nullptr;
		uint64_t ibOff = 0;
		if (!AllocateUpload(ibBytes, 4, ibCpu, ibGpu, ibRes, ibOff))
			return false;

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			if (isIdx32)
			{
				const uint32_t* src = static_cast<const uint32_t*>(pIndexData);
				uint32_t* dst = static_cast<uint32_t*>(ibCpu);
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
				uint16_t* dst = static_cast<uint16_t*>(ibCpu);
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
			std::memcpy(ibCpu, pIndexData, ibBytes);
		}

		m_streams[0].buffer = nullptr;
		m_currentIndexBuffer = nullptr;
		if (!PrepareForDraw(topo, topoType))
			return false;

		D3D12_VERTEX_BUFFER_VIEW vbv = { vbGpu, vbBytes, VertexStreamZeroStride };
		D3D12_INDEX_BUFFER_VIEW ibv = { ibGpu, ibBytes, isIdx32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT };
		m_cmdList->IASetVertexBuffers(0, 1, &vbv);
		m_cmdList->IASetIndexBuffer(&ibv);
		m_cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
		RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
		return true;
	}

	uint32_t RenderDeviceD3D12::GetRenderState(const ERenderState& State)
	{
		return m_stateTracker.GetRenderState(State);
	}

	bool RenderDeviceD3D12::SetRenderState(const ERenderState State, const uint32_t Value)
	{
		m_stateTracker.SetRenderState(State, Value);
		if (State == ERenderState::SCISSORTESTENABLE)
		{
			if (m_isRecording && m_cmdList)
			{
				if (Value)
				{
					const RECT& sr = m_stateTracker.GetScissorRect();
					D3D12_RECT r = { sr.left, sr.top, sr.right, sr.bottom };
					m_cmdList->RSSetScissorRects(1, &r);
				}
				else
				{
					D3D12_RECT r = { 0, 0, static_cast<LONG>(m_renderTargetWidth), static_cast<LONG>(m_renderTargetHeight) };
					m_cmdList->RSSetScissorRects(1, &r);
				}
			}
		}
		return true;
	}

	bool RenderDeviceD3D12::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		m_stateTracker.SetClipPlane(Index, pPlane);
		return true;
	}

	int RenderDeviceD3D12::GetMaxSimultaneousTextures()
	{
		return static_cast<int>(m_caps.maxTextureStages);
	}

	bool RenderDeviceD3D12::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		if (stage >= RHIStateTracker::MAX_SAMPLER_STAGES)
			return false;
		m_stateTracker.SetSamplerTexture(stage, texture);
		auto* d3dTex = texture ? static_cast<D3D12Texture*>(texture->GetNativeHandle()) : nullptr;
		if (m_boundTextures[stage] != d3dTex)
		{
			m_boundTextures[stage] = d3dTex;
			m_srvTableDirty = true;
		}
		return true;
	}

	bool RenderDeviceD3D12::SetTransform(ETransformsStateType /*State*/, DeviceMatrix_ptr /*pMatrix*/)
	{
		return true;
	}

	bool RenderDeviceD3D12::SetFVF(uint32_t FVF)
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

	void RenderDeviceD3D12::SetCursorPosition(int /*X*/, int /*Y*/, uint32_t /*Flags*/)
	{
	}

	bool RenderDeviceD3D12::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		m_stateTracker.SetSamplerState(stage, type, value);
		return true;
	}

	bool RenderDeviceD3D12::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		if (value)
			*value = m_stateTracker.GetSamplerState(stage, type);
		return true;
	}

	bool RenderDeviceD3D12::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
	{
		m_currentVertexDecl = pVertexDeclaration;
		return true;
	}

	bool RenderDeviceD3D12::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
	{
		if (!ppDecl)
			return false;
		*ppDecl = new CVertexDeclaration(pVertexElements);
		return true;
	}

	bool RenderDeviceD3D12::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		m_currentIndexBuffer = pIndexData ? static_cast<D3D12Buffer*>(pIndexData->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceD3D12::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
	{
		if (StreamNumber >= 4)
			return false;
		m_streams[StreamNumber].buffer = pStreamData ? static_cast<D3D12Buffer*>(pStreamData->GetNativeHandle()) : nullptr;
		m_streams[StreamNumber].offsetInBytes = OffsetInBytes;
		m_streams[StreamNumber].stride = Stride;
		return true;
	}

	bool RenderDeviceD3D12::SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider)
	{
		if (streamNumber >= 4)
			return false;
		m_streams[streamNumber].frequencyDivider = divider;
		return true;
	}

	ParaViewport RenderDeviceD3D12::GetViewport()
	{
		return m_currentViewport;
	}

	bool RenderDeviceD3D12::SetViewport(const ParaViewport& viewport)
	{
		m_currentViewport = viewport;
		m_stateTracker.SetViewport(viewport);
		if (m_isRecording && m_cmdList)
		{
			D3D12_VIEWPORT vp = {
				static_cast<FLOAT>(viewport.X),
				static_cast<FLOAT>(viewport.Y),
				static_cast<FLOAT>(viewport.Width),
				static_cast<FLOAT>(viewport.Height),
				viewport.MinZ,
				viewport.MaxZ
			};
			m_cmdList->RSSetViewports(1, &vp);
			if (!m_stateTracker.GetRenderState(ERenderState::SCISSORTESTENABLE))
			{
				D3D12_RECT r = { 0, 0, static_cast<LONG>(m_renderTargetWidth), static_cast<LONG>(m_renderTargetHeight) };
				m_cmdList->RSSetScissorRects(1, &r);
			}
		}
		return true;
	}

	bool RenderDeviceD3D12::SetClearColor(const Color4f& color)
	{
		m_stateTracker.SetClearColor(color);
		return true;
	}

	bool RenderDeviceD3D12::SetClearDepth(const float depth)
	{
		m_stateTracker.SetClearDepth(depth);
		return true;
	}

	bool RenderDeviceD3D12::SetClearStencil(const int stencil)
	{
		m_stateTracker.SetClearStencil(stencil);
		return true;
	}

	bool RenderDeviceD3D12::Clear(bool color, bool depth, bool stencil)
	{
		ID3D12GraphicsCommandList* cmd = EnsureRecording();
		if (!cmd)
			return false;

		BindActiveRenderTargetsToCmdList();

		if (color)
		{
			const Color4f& c = m_stateTracker.GetClearColor();
			const FLOAT rgba[4] = { c.r, c.g, c.b, c.a };
			for (uint32_t i = 0; i < m_activeRTVCount && i < 4; ++i)
			{
				if (m_activeRTVs[i].ptr != 0 && (m_activeNullRtvMask & (1u << i)) == 0)
					cmd->ClearRenderTargetView(m_activeRTVs[i], rgba, 0, nullptr);
			}
		}

		if ((depth || stencil) && m_hasActiveDSV && m_activeDSV.ptr != 0)
		{
			D3D12_CLEAR_FLAGS flags = static_cast<D3D12_CLEAR_FLAGS>(
				(depth ? D3D12_CLEAR_FLAG_DEPTH : 0) |
				(stencil ? D3D12_CLEAR_FLAG_STENCIL : 0));
			cmd->ClearDepthStencilView(m_activeDSV, flags, m_stateTracker.GetClearDepth(), static_cast<UINT8>(m_stateTracker.GetClearStencil() & 0xFF), 0, nullptr);
		}

		return true;
	}

	bool RenderDeviceD3D12::SetScissorRect(RECT* pRect)
	{
		m_stateTracker.SetScissorRect(pRect);
		if (m_isRecording && m_cmdList)
		{
			if (pRect && m_stateTracker.GetRenderState(ERenderState::SCISSORTESTENABLE))
			{
				D3D12_RECT r = { pRect->left, pRect->top, pRect->right, pRect->bottom };
				m_cmdList->RSSetScissorRects(1, &r);
			}
			else
			{
				D3D12_RECT r = { 0, 0, static_cast<LONG>(m_renderTargetWidth), static_cast<LONG>(m_renderTargetHeight) };
				m_cmdList->RSSetScissorRects(1, &r);
			}
		}
		return true;
	}

	bool RenderDeviceD3D12::GetScissorRect(RECT* pRect)
	{
		if (pRect)
			*pRect = m_stateTracker.GetScissorRect();
		return true;
	}

	bool RenderDeviceD3D12::BeginScene()
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

		SetRenderTargets(0, nullptr, nullptr);
		return true;
	}

	bool RenderDeviceD3D12::EndScene()
	{
		return true;
	}

	bool RenderDeviceD3D12::Present()
	{
		if (!m_swapChain || !m_cmdQueue || !m_fence)
			return false;

		EnsureRecording();
		if (m_backBuffers[m_backBufferIndex])
		{
			TransitionResource(m_backBuffers[m_backBufferIndex].Get(), m_backBufferStates[m_backBufferIndex], D3D12_RESOURCE_STATE_PRESENT);
		}

		if (m_isRecording && m_cmdList)
		{
			m_cmdList->Close();
			m_isRecording = false;
			ID3D12CommandList* lists[1] = { m_cmdList.Get() };
			m_cmdQueue->ExecuteCommandLists(1, lists);
		}

		HRESULT hr = m_swapChain->Present(m_vsyncEnabled ? 1 : 0, 0);

		const uint64_t fenceVal = m_nextFenceValue++;
		m_cmdQueue->Signal(m_fence.Get(), fenceVal);
		m_frameFenceValues[m_frameIndex] = fenceVal;

		m_frameIndex = (m_frameIndex + 1) % FRAME_COUNT;
		m_frameResetDone[m_frameIndex] = false;
		m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
		if (m_usingBackBufferRTV)
		{
			m_activeRTVs[0] = m_cpuRtvAlloc.GetHandle(m_backBufferRtvSlots[m_backBufferIndex]);
		}

		return SUCCEEDED(hr);
	}

	void RenderDeviceD3D12::Flush()
	{
		ExecuteAndSyncGPU();
	}
}

#include "ParaEngine.h"
#include "renderer/RenderDevice.h"
#include "RenderDeviceVulkan.h"
#include "FxShaderTypes.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "RHISoftwareS3TC.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace ParaEngine
{
	namespace
	{
		static constexpr VkFlags VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT_LOCAL = 0x00000010u;

		inline void HashCombineVkU64(uint64_t& h, uint64_t v)
		{
			h ^= v;
			h *= 1099511628211ULL;
		}

		inline void HashCombineVkU32(uint64_t& h, uint32_t v)
		{
			h ^= static_cast<uint64_t>(v);
			h *= 1099511628211ULL;
		}

		uint64_t ComputeVkByteHash64(const void* data, size_t size)
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

		inline uint64_t AlignUpVkU64(uint64_t val, uint64_t alignment)
		{
			return (alignment <= 1) ? val : ((val + alignment - 1ULL) & ~(alignment - 1ULL));
		}

		bool IsDepthPixelFormatVk(PixelFormat fmt)
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

		bool HasStencilInVkDepthFormat(VkFormat fmt)
		{
			return fmt == VK_FORMAT_D24_UNORM_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT;
		}

		VkFlags AspectMaskForVkFormat(VkFormat fmt)
		{
			if (fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_X8_D24_UNORM_PACK32 || fmt == VK_FORMAT_D32_SFLOAT)
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			if (fmt == VK_FORMAT_D24_UNORM_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkFormat MapColorPixelFormatToVk(PixelFormat fmt, uint32_t& outBytesPerBlock, bool& outIsBlockCompressed, bool& outSwizzleAlphaFromR)
		{
			outBytesPerBlock = 4;
			outIsBlockCompressed = false;
			outSwizzleAlphaFromR = false;
			switch (fmt)
			{
			case PixelFormat::A8R8G8B8:
			case PixelFormat::X8R8G8B8:
				outBytesPerBlock = 4;
				return VK_FORMAT_R8G8B8A8_UNORM;
			case PixelFormat::A8B8G8R8:
			case PixelFormat::X8B8G8R8:
				outBytesPerBlock = 4;
				return VK_FORMAT_B8G8R8A8_UNORM;
			case PixelFormat::A8:
				outBytesPerBlock = 1;
				outSwizzleAlphaFromR = true;
				return VK_FORMAT_R8_UNORM;
			case PixelFormat::R16F:
				outBytesPerBlock = 2;
				return VK_FORMAT_R16_SFLOAT;
			case PixelFormat::R32F:
				outBytesPerBlock = 4;
				return VK_FORMAT_R32_SFLOAT;
			case PixelFormat::G16R16F:
				outBytesPerBlock = 4;
				return VK_FORMAT_R16G16_SFLOAT;
			case PixelFormat::G32R32F:
				outBytesPerBlock = 8;
				return VK_FORMAT_R32G32_SFLOAT;
			case PixelFormat::A16B16G16R16F:
				outBytesPerBlock = 8;
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case PixelFormat::A32B32G32R32F:
				outBytesPerBlock = 16;
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case PixelFormat::A16B16G16R16:
				outBytesPerBlock = 8;
				return VK_FORMAT_R16G16B16A16_UNORM;
			case PixelFormat::A2B10G10R10:
			case PixelFormat::A2R10G10B10:
				outBytesPerBlock = 4;
				return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case PixelFormat::DXT1:
				outBytesPerBlock = 8;
				outIsBlockCompressed = true;
				return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case PixelFormat::DXT2:
			case PixelFormat::DXT3:
				outBytesPerBlock = 16;
				outIsBlockCompressed = true;
				return VK_FORMAT_BC2_UNORM_BLOCK;
			case PixelFormat::DXT4:
			case PixelFormat::DXT5:
				outBytesPerBlock = 16;
				outIsBlockCompressed = true;
				return VK_FORMAT_BC3_UNORM_BLOCK;
			case PixelFormat::BC7:
				outBytesPerBlock = 16;
				outIsBlockCompressed = true;
				return VK_FORMAT_BC7_UNORM_BLOCK;
			default:
				return VK_FORMAT_UNDEFINED;
			}
		}

		bool ComputeVkTopologyAndCount(
			EPrimitiveType primType,
			uint32_t primCount,
			VkPrimitiveTopology& outTopo,
			uint32_t& outVertexOrIndexCount)
		{
			if (primCount == 0)
				return false;
			switch (primType)
			{
			case EPrimitiveType::POINTLIST:
				outTopo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
				outVertexOrIndexCount = primCount;
				return true;
			case EPrimitiveType::LINELIST:
				outTopo = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				outVertexOrIndexCount = primCount * 2u;
				return true;
			case EPrimitiveType::LINESTRIP:
				outTopo = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
				outVertexOrIndexCount = primCount + 1u;
				return true;
			case EPrimitiveType::TRIANGLELIST:
				outTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				outVertexOrIndexCount = primCount * 3u;
				return true;
			case EPrimitiveType::TRIANGLESTRIP:
				outTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				outVertexOrIndexCount = primCount + 2u;
				return true;
			case EPrimitiveType::TRIANGLEFAN:
				outTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
				outVertexOrIndexCount = primCount + 2u;
				return true;
			default:
				return false;
			}
		}

		VkCompareOp ToVkCompareOp(uint32_t rsvCmp)
		{
			switch (rsvCmp)
			{
			case RSV_CMP_NEVER:        return VK_COMPARE_OP_NEVER;
			case RSV_CMP_LESS:         return VK_COMPARE_OP_LESS;
			case RSV_CMP_EQUAL:        return VK_COMPARE_OP_EQUAL;
			case RSV_CMP_LESSEQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
			case RSV_CMP_GREATER:      return VK_COMPARE_OP_GREATER;
			case RSV_CMP_NOTEQUAL:     return VK_COMPARE_OP_NOT_EQUAL;
			case RSV_CMP_GREATEREQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case RSV_CMP_ALWAYS:
			default:                   return VK_COMPARE_OP_ALWAYS;
			}
		}

		VkBlendFactor ToVkBlendFactor(uint32_t rsvBlend, bool isAlphaChannel)
		{
			switch (rsvBlend)
			{
			case RSV_BLEND_ZERO:           return VK_BLEND_FACTOR_ZERO;
			case RSV_BLEND_ONE:            return VK_BLEND_FACTOR_ONE;
			case RSV_BLEND_SRCCOLOR:       return isAlphaChannel ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_SRC_COLOR;
			case RSV_BLEND_INVSRCCOLOR:    return isAlphaChannel ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case RSV_BLEND_SRCALPHA:       return VK_BLEND_FACTOR_SRC_ALPHA;
			case RSV_BLEND_INVSRCALPHA:    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case RSV_BLEND_DESTALPHA:      return VK_BLEND_FACTOR_DST_ALPHA;
			case RSV_BLEND_INVDESTALPHA:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case RSV_BLEND_DESTCOLOR:      return isAlphaChannel ? VK_BLEND_FACTOR_DST_ALPHA : VK_BLEND_FACTOR_DST_COLOR;
			case RSV_BLEND_INVDESTCOLOR:   return isAlphaChannel ? VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case RSV_BLEND_SRCALPHASAT:    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			case RSV_BLEND_BLENDFACTOR:    return isAlphaChannel ? VK_BLEND_FACTOR_CONSTANT_ALPHA : VK_BLEND_FACTOR_CONSTANT_COLOR;
			case RSV_BLEND_INVBLENDFACTOR: return isAlphaChannel ? VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA : VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			default:                       return VK_BLEND_FACTOR_ONE;
			}
		}

		VkBlendOp ToVkBlendOp(uint32_t op)
		{
			switch (op)
			{
			case 2:  return VK_BLEND_OP_SUBTRACT;
			case 3:  return VK_BLEND_OP_REVERSE_SUBTRACT;
			case 4:  return VK_BLEND_OP_MIN;
			case 5:  return VK_BLEND_OP_MAX;
			case 1:
			default: return VK_BLEND_OP_ADD;
			}
		}

		VkStencilOp ToVkStencilOp(uint32_t op)
		{
			switch (op)
			{
			case RSV_STENCILOP_ZERO:    return VK_STENCIL_OP_ZERO;
			case RSV_STENCILOP_REPLACE: return VK_STENCIL_OP_REPLACE;
			case RSV_STENCILOP_INCRSAT: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case RSV_STENCILOP_DECRSAT: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case RSV_STENCILOP_INVERT:  return VK_STENCIL_OP_INVERT;
			case RSV_STENCILOP_INCR:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case RSV_STENCILOP_DECR:    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
			case RSV_STENCILOP_KEEP:
			default:                    return VK_STENCIL_OP_KEEP;
			}
		}

		VkSamplerAddressMode ToVkAddressMode(uint32_t mode)
		{
			switch (mode)
			{
			case 2: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case 3: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case 4: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case 5: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			case 1:
			default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
		}

		// Note: SpirvCodeGen emits all VS Input locations as %v4float (vec4), so all vertex formats MUST decode to float!
		VkFormat MapDeclTypeToVkFormat(uint8_t declType, bool supportsUscaled)
		{
			switch (declType)
			{
			case D3DDECLTYPE_FLOAT1:    return VK_FORMAT_R32_SFLOAT;
			case D3DDECLTYPE_FLOAT2:    return VK_FORMAT_R32G32_SFLOAT;
			case D3DDECLTYPE_FLOAT3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case D3DDECLTYPE_FLOAT4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
			case D3DDECLTYPE_D3DCOLOR:  return VK_FORMAT_B8G8R8A8_UNORM;
			case D3DDECLTYPE_UBYTE4:    return supportsUscaled ? VK_FORMAT_R8G8B8A8_USCALED : VK_FORMAT_R8G8B8A8_UNORM;
			case D3DDECLTYPE_SHORT2:    return VK_FORMAT_R16G16_SSCALED;
			case D3DDECLTYPE_SHORT4:    return VK_FORMAT_R16G16B16A16_SSCALED;
			case D3DDECLTYPE_UBYTE4N:   return VK_FORMAT_R8G8B8A8_UNORM;
			case D3DDECLTYPE_SHORT2N:   return VK_FORMAT_R16G16_SNORM;
			case D3DDECLTYPE_SHORT4N:   return VK_FORMAT_R16G16B16A16_SNORM;
			case D3DDECLTYPE_USHORT2N:  return VK_FORMAT_R16G16_UNORM;
			case D3DDECLTYPE_USHORT4N:  return VK_FORMAT_R16G16B16A16_UNORM;
			case D3DDECLTYPE_FLOAT16_2: return VK_FORMAT_R16G16_SFLOAT;
			case D3DDECLTYPE_FLOAT16_4: return VK_FORMAT_R16G16B16A16_SFLOAT;
			default:                    return VK_FORMAT_R32G32B32_SFLOAT;
			}
		}

		int32_t MapDeclUsageToLocationVk(uint8_t usage, uint8_t usageIndex)
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
	// Vulkan RHI Resource Implementations
	// =========================================================================

	class VulkanTexture : public IRHITexture
	{
	public:
		VulkanTexture(
			RenderDeviceVulkan* owner,
			uint32_t width,
			uint32_t height,
			PixelFormat sourceFormat,
			VkFormat vkFormat,
			uint32_t mipLevels,
			uint32_t layerCount,
			bool isCube,
			bool expandedToRGBA8,
			bool swizzledAlphaFromR,
			VkImage image,
			VkDeviceMemory memory,
			VkImageView srvView,
			VkImageView attachmentView,
			uint64_t attachmentViewId,
			VkImageLayout initialLayout)
			: m_owner(owner)
			, m_width(width)
			, m_height(height)
			, m_mipLevels(mipLevels)
			, m_layerCount(layerCount)
			, m_isCube(isCube)
			, m_sourceFormat(sourceFormat)
			, m_vkFormat(vkFormat)
			, m_expandedToRGBA8(expandedToRGBA8)
			, m_swizzledAlphaFromR(swizzledAlphaFromR)
			, m_image(image)
			, m_memory(memory)
			, m_srvView(srvView)
			, m_attachmentView(attachmentView)
			, m_attachmentViewId(attachmentViewId)
			, m_layout(initialLayout)
		{
		}

		virtual ~VulkanTexture() override
		{
			if (m_owner)
			{
				m_owner->NotifyTextureDestroyed(this);
				if (m_attachmentViewId != 0)
					m_owner->NotifyViewDestroyed(m_attachmentViewId);

				VulkanDeferredDelete dd;
				dd.image = m_image;
				dd.memory = m_memory;
				dd.imageView = m_srvView;
				m_owner->DeferDelete(dd);

				if (m_attachmentView != VK_NULL_HANDLE && m_attachmentView != m_srvView)
				{
					VulkanDeferredDelete ddAtt;
					ddAtt.imageView = m_attachmentView;
					m_owner->DeferDelete(ddAtt);
				}
			}
		}

		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual uint32_t GetMipLevels() const override { return m_mipLevels; }
		virtual PixelFormat GetFormat() const override { return m_sourceFormat; }
		virtual bool IsCubeMap() const override { return m_isCube; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<VulkanTexture*>(this)); }

		VkImage GetVkImage() const { return m_image; }
		VkFormat GetVkFormat() const { return m_vkFormat; }
		VkImageView GetSrvView() const { return m_srvView; }
		VkImageView GetAttachmentView() const { return m_attachmentView != VK_NULL_HANDLE ? m_attachmentView : m_srvView; }
		uint64_t GetAttachmentViewId() const { return m_attachmentViewId; }
		uint32_t GetLayerCount() const { return m_layerCount; }
		bool IsSwizzledAlphaFromR() const { return m_swizzledAlphaFromR; }
		VkImageLayout& GetLayoutRef() { return m_layout; }
		VkImageLayout GetLayout() const { return m_layout; }

		virtual bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t rowPitch) override
		{
			if (!m_owner || !m_image || !data || width == 0 || height == 0 || mipLevel >= m_mipLevels || arraySlice >= m_layerCount)
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
				(m_vkFormat == VK_FORMAT_BC1_RGBA_UNORM_BLOCK || m_vkFormat == VK_FORMAT_BC2_UNORM_BLOCK || m_vkFormat == VK_FORMAT_BC3_UNORM_BLOCK))
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
				if (m_vkFormat == VK_FORMAT_R8_UNORM) bpp = 1u;
				else if (m_vkFormat == VK_FORMAT_R16_SFLOAT) bpp = 2u;
				else if (m_vkFormat == VK_FORMAT_R16G16B16A16_SFLOAT || m_vkFormat == VK_FORMAT_R16G16B16A16_UNORM || m_vkFormat == VK_FORMAT_R32G32_SFLOAT) bpp = 8u;
				else if (m_vkFormat == VK_FORMAT_R32G32B32A32_SFLOAT) bpp = 16u;
				srcRowBytes = width * bpp;
				srcStride = rowPitch ? rowPitch : srcRowBytes;
			}

			uint32_t totalBytes = numRows * srcRowBytes;
			void* upCpu = nullptr;
			VkBuffer upBuf = VK_NULL_HANDLE;
			VkDeviceSize upOff = 0;
			if (!m_owner->AllocateUpload(totalBytes, 16, upCpu, upBuf, upOff))
				return false;

			uint8_t* dstPtr = static_cast<uint8_t*>(upCpu);
			if (srcStride == srcRowBytes)
			{
				std::memcpy(dstPtr, srcBytes, totalBytes);
			}
			else
			{
				for (uint32_t r = 0; r < numRows; ++r)
					std::memcpy(dstPtr + static_cast<size_t>(r) * srcRowBytes, srcBytes + static_cast<size_t>(r) * srcStride, srcRowBytes);
			}

			VkCommandBuffer cmd = m_owner->EnsureRecording();
			if (!cmd)
				return false;
			m_owner->EndActiveRenderPass();

			VkFlags aspect = AspectMaskForVkFormat(m_vkFormat);
			VkImageLayout restoreLayout = (m_layout == VK_IMAGE_LAYOUT_UNDEFINED || m_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
				? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				: m_layout;

			m_owner->TransitionImageLayout(cmd, m_image, aspect, m_mipLevels, m_layerCount, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy region = {};
			region.bufferOffset = upOff;
			region.bufferRowLength = 0; // tightly packed
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = aspect;
			region.imageSubresource.mipLevel = mipLevel;
			region.imageSubresource.baseArrayLayer = arraySlice;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { x, y, 0 };
			region.imageExtent = { width, height, 1 };

			m_owner->GetVK().vkCmdCopyBufferToImage(cmd, upBuf, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			m_owner->TransitionImageLayout(cmd, m_image, aspect, m_mipLevels, m_layerCount, m_layout, restoreLayout);
			return true;
		}

	private:
		RenderDeviceVulkan* m_owner;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_mipLevels;
		uint32_t m_layerCount;
		bool m_isCube;
		PixelFormat m_sourceFormat;
		VkFormat m_vkFormat;
		bool m_expandedToRGBA8;
		bool m_swizzledAlphaFromR;
		VkImage m_image;
		VkDeviceMemory m_memory;
		VkImageView m_srvView;
		VkImageView m_attachmentView;
		uint64_t m_attachmentViewId;
		VkImageLayout m_layout;
	};

	class VulkanBuffer : public IRHIBuffer
	{
	public:
		VulkanBuffer(
			RenderDeviceVulkan* owner,
			uint32_t byteSize,
			bool isIndexBuffer,
			ERHIBufferUsage usage,
			const void* initialData)
			: m_owner(owner)
			, m_byteSize(byteSize)
			, m_allocSize((std::max)(byteSize, 16u))
			, m_isIndexBuffer(isIndexBuffer)
			, m_isDynamic(HasFlag(usage, ERHIBufferUsage::Dynamic) || HasFlag(usage, ERHIBufferUsage::Stream))
		{
			if (!owner)
				return;

			VkFlags vkUsage = (isIndexBuffer ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
				| VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			if (m_isDynamic)
			{
				if (owner->CreateVkBufferWithMemory(
					m_allocSize,
					vkUsage,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_buffer,
					m_memory))
				{
					void* ptr = nullptr;
					if (owner->GetVK().vkMapMemory(owner->GetVkDevice(), m_memory, 0, m_allocSize, 0, &ptr) == VK_SUCCESS && ptr)
					{
						m_persistentCpuPtr = static_cast<uint8_t*>(ptr);
						if (initialData && byteSize > 0)
							std::memcpy(m_persistentCpuPtr, initialData, byteSize);
					}
					m_activeBuffer = m_buffer;
					m_activeOffset = 0;
				}
			}
			else
			{
				if (owner->CreateVkBufferWithMemory(
					m_allocSize,
					vkUsage,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					m_buffer,
					m_memory))
				{
					m_activeBuffer = m_buffer;
					m_activeOffset = 0;
					if (initialData && byteSize > 0)
					{
						if (!HasFlag(usage, ERHIBufferUsage::WriteOnly))
						{
							m_cpuShadow.resize(byteSize);
							std::memcpy(m_cpuShadow.data(), initialData, byteSize);
						}
						UploadRegionToDeviceBuffer(0, byteSize, initialData);
					}
				}
			}
		}

		virtual ~VulkanBuffer() override
		{
			if (m_owner)
				m_owner->NotifyBufferDestroyed(this);
			if (m_owner && m_buffer != VK_NULL_HANDLE)
			{
				if (m_persistentCpuPtr && m_memory != VK_NULL_HANDLE)
				{
					m_owner->GetVK().vkUnmapMemory(m_owner->GetVkDevice(), m_memory);
					m_persistentCpuPtr = nullptr;
				}
				VulkanDeferredDelete dd;
				dd.buffer = m_buffer;
				dd.memory = m_memory;
				m_owner->DeferDelete(dd);
			}
		}

		virtual uint32_t GetSize() const override { return m_byteSize; }
		virtual bool IsIndexBuffer() const override { return m_isIndexBuffer; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<VulkanBuffer*>(this)); }

		VkBuffer GetActiveVkBuffer() const { return m_activeBuffer; }
		VkDeviceSize GetActiveOffset() const { return m_activeOffset; }

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
				void* sliceCpu = nullptr;
				VkBuffer sliceBuf = VK_NULL_HANDLE;
				VkDeviceSize sliceOff = 0;
				if (m_owner->AllocateUpload(m_allocSize, 16, sliceCpu, sliceBuf, sliceOff) && sliceCpu)
				{
					m_activeBuffer = sliceBuf;
					m_activeOffset = sliceOff;
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
					m_activeBuffer = m_buffer;
					m_activeOffset = 0;
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
				VkBuffer sliceBuf = VK_NULL_HANDLE;
				VkDeviceSize sliceOff = 0;
				if (m_owner->AllocateUpload(m_allocSize, 16, sliceCpu, sliceBuf, sliceOff) && sliceCpu)
				{
					std::memcpy(sliceCpu, m_cpuShadow.data(), m_byteSize);
					m_activeBuffer = sliceBuf;
					m_activeOffset = sliceOff;
					m_activeDynamicCpuBase = static_cast<uint8_t*>(sliceCpu);
				}
			}
			else
			{
				UploadRegionToDeviceBuffer(m_mapOffset, m_mapSize, m_cpuShadow.data() + m_mapOffset);
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
					std::memcpy(m_cpuShadow.data() + offset, data, size);
				return UploadRegionToDeviceBuffer(offset, size, data);
			}

			void* dst = Map(offset, size, ERHIMapMode::Write);
			if (!dst)
				return false;
			std::memcpy(dst, data, size);
			Unmap();
			return true;
		}

	private:
		bool UploadRegionToDeviceBuffer(uint32_t offset, uint32_t size, const void* data)
		{
			if (!m_owner || !m_buffer || !data || size == 0)
				return false;

			void* upCpu = nullptr;
			VkBuffer upBuf = VK_NULL_HANDLE;
			VkDeviceSize upOff = 0;
			if (!m_owner->AllocateUpload(size, 16, upCpu, upBuf, upOff))
				return false;

			std::memcpy(upCpu, data, size);
			VkCommandBuffer cmd = m_owner->EnsureRecording();
			if (!cmd)
				return false;
			m_owner->EndActiveRenderPass();

			VkBufferCopy copyRegion = { upOff, static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size) };
			m_owner->GetVK().vkCmdCopyBuffer(cmd, upBuf, m_buffer, 1, &copyRegion);

			// Memory barrier so subsequent vertex/index reads observe the transfer write
			m_owner->GetVK().vkCmdPipelineBarrier(
				cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
				0,
				0, nullptr,
				0, nullptr,
				0, nullptr);
			return true;
		}

	private:
		RenderDeviceVulkan* m_owner;
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;
		VkBuffer m_activeBuffer = VK_NULL_HANDLE;
		VkDeviceSize m_activeOffset = 0;
		uint8_t* m_persistentCpuPtr = nullptr;
		uint8_t* m_activeDynamicCpuBase = nullptr;
		std::vector<uint8_t> m_cpuShadow;
		uint32_t m_byteSize;
		uint32_t m_allocSize;
		bool m_isIndexBuffer;
		bool m_isDynamic;
		bool m_mappedDirect = false;
		bool m_mappedShadow = false;
		uint32_t m_mapOffset = 0;
		uint32_t m_mapSize = 0;
		ERHIMapMode m_mapMode = ERHIMapMode::Write;
	};

	class VulkanRenderTarget : public IRHIRenderTarget
	{
	public:
		VulkanRenderTarget(
			RenderDeviceVulkan* owner,
			uint32_t width,
			uint32_t height,
			PixelFormat colorFormat,
			PixelFormat depthFormat,
			uint32_t colorTargetCount,
			uint32_t /*msaaSamples*/)
			: m_owner(owner)
			, m_width(width)
			, m_height(height)
			, m_colorFormat(colorFormat)
			, m_colorCount((std::min)((std::max)(colorTargetCount, 1u), 4u))
			, m_sampleCount(1)
			, m_valid(false)
		{
			if (!owner || width == 0 || height == 0)
				return;

			uint32_t bpp = 4;
			bool compressed = false;
			bool swizzleA = false;
			VkFormat vkColorFmt = MapColorPixelFormatToVk(colorFormat, bpp, compressed, swizzleA);
			if (vkColorFmt == VK_FORMAT_UNDEFINED || compressed || swizzleA)
				vkColorFmt = VK_FORMAT_R8G8B8A8_UNORM;

			VkFormatProperties fmtProps = {};
			owner->GetVK().vkGetPhysicalDeviceFormatProperties(owner->GetVkPhysicalDevice(), vkColorFmt, &fmtProps);
			if ((fmtProps.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0)
				vkColorFmt = VK_FORMAT_R8G8B8A8_UNORM;
			m_vkColorFormat = vkColorFmt;

			for (uint32_t i = 0; i < m_colorCount; ++i)
			{
				VkImageCreateInfo imgInfo = {};
				imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imgInfo.imageType = VK_IMAGE_TYPE_2D;
				imgInfo.format = vkColorFmt;
				imgInfo.extent = { width, height, 1 };
				imgInfo.mipLevels = 1;
				imgInfo.arrayLayers = 1;
				imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imgInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
					| VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				VkImage img = VK_NULL_HANDLE;
				VkDeviceMemory mem = VK_NULL_HANDLE;
				if (!owner->CreateVkImageWithMemory(imgInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img, mem))
					return;

				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = img;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = vkColorFmt;
				viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

				VkImageView view = VK_NULL_HANDLE;
				if (owner->GetVK().vkCreateImageView(owner->GetVkDevice(), &viewInfo, nullptr, &view) != VK_SUCCESS || !view)
				{
					owner->GetVK().vkDestroyImage(owner->GetVkDevice(), img, nullptr);
					owner->GetVK().vkFreeMemory(owner->GetVkDevice(), mem, nullptr);
					return;
				}

				uint64_t viewId = owner->NextUniqueViewId();
				m_colorTextures[i] = new VulkanTexture(
					owner, width, height, colorFormat, vkColorFmt,
					1, 1, false, false, false,
					img, mem, view, view, viewId, VK_IMAGE_LAYOUT_UNDEFINED);
			}

			if (depthFormat != PixelFormat::Unknown && IsDepthPixelFormatVk(depthFormat))
			{
				VkFormat vkDepthFmt = VK_FORMAT_D24_UNORM_S8_UINT;
				if (depthFormat == PixelFormat::D16 || depthFormat == PixelFormat::D16_LOCKABLE)
					vkDepthFmt = VK_FORMAT_D16_UNORM;
				else if (depthFormat == PixelFormat::D32 || depthFormat == PixelFormat::D32F_LOCKABLE)
					vkDepthFmt = VK_FORMAT_D32_SFLOAT;

				VkFormatProperties dProps = {};
				owner->GetVK().vkGetPhysicalDeviceFormatProperties(owner->GetVkPhysicalDevice(), vkDepthFmt, &dProps);
				if ((dProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
				{
					vkDepthFmt = VK_FORMAT_D32_SFLOAT_S8_UINT;
					owner->GetVK().vkGetPhysicalDeviceFormatProperties(owner->GetVkPhysicalDevice(), vkDepthFmt, &dProps);
					if ((dProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
						vkDepthFmt = VK_FORMAT_D32_SFLOAT;
				}

				VkImageCreateInfo dInfo = {};
				dInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				dInfo.imageType = VK_IMAGE_TYPE_2D;
				dInfo.format = vkDepthFmt;
				dInfo.extent = { width, height, 1 };
				dInfo.mipLevels = 1;
				dInfo.arrayLayers = 1;
				dInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				dInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				dInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				dInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				dInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				VkImage dImg = VK_NULL_HANDLE;
				VkDeviceMemory dMem = VK_NULL_HANDLE;
				if (owner->CreateVkImageWithMemory(dInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dImg, dMem))
				{
					// SRV view MUST only include DEPTH_BIT; attachment view includes DEPTH_BIT | STENCIL_BIT
					VkImageViewCreateInfo srvVi = {};
					srvVi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					srvVi.image = dImg;
					srvVi.viewType = VK_IMAGE_VIEW_TYPE_2D;
					srvVi.format = vkDepthFmt;
					srvVi.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

					VkImageView dSrvView = VK_NULL_HANDLE;
					owner->GetVK().vkCreateImageView(owner->GetVkDevice(), &srvVi, nullptr, &dSrvView);

					VkFlags attAspect = AspectMaskForVkFormat(vkDepthFmt);
					VkImageView dAttView = dSrvView;
					if (attAspect != VK_IMAGE_ASPECT_DEPTH_BIT)
					{
						VkImageViewCreateInfo attVi = srvVi;
						attVi.subresourceRange.aspectMask = attAspect;
						owner->GetVK().vkCreateImageView(owner->GetVkDevice(), &attVi, nullptr, &dAttView);
					}

					uint64_t dViewId = owner->NextUniqueViewId();
					m_vkDepthFormat = vkDepthFmt;
					m_depthTexture = new VulkanTexture(
						owner, width, height, depthFormat, vkDepthFmt,
						1, 1, false, false, false,
						dImg, dMem, dSrvView, dAttView, dViewId, VK_IMAGE_LAYOUT_UNDEFINED);
				}
			}

			m_valid = true;
		}

		virtual ~VulkanRenderTarget() override
		{
			if (m_owner)
				m_owner->NotifyRenderTargetDestroyed(this);
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
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<VulkanRenderTarget*>(this)); }

		uint32_t GetColorCount() const { return m_colorCount; }
		uint32_t GetSampleCount() const { return m_sampleCount; }
		VkFormat GetVkColorFormat() const { return m_vkColorFormat; }
		VkFormat GetVkDepthFormat() const { return m_vkDepthFormat; }
		bool HasDepth() const { return m_depthTexture != nullptr; }
		VulkanTexture* GetVkColorTex(uint32_t index = 0) const { return (index < m_colorCount) ? m_colorTextures[index] : nullptr; }
		VulkanTexture* GetVkDepthTex() const { return m_depthTexture; }

	private:
		RenderDeviceVulkan* m_owner;
		uint32_t m_width;
		uint32_t m_height;
		PixelFormat m_colorFormat;
		VkFormat m_vkColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
		VkFormat m_vkDepthFormat = VK_FORMAT_UNDEFINED;
		uint32_t m_colorCount;
		uint32_t m_sampleCount;
		bool m_valid;
		VulkanTexture* m_colorTextures[4] = {};
		VulkanTexture* m_depthTexture = nullptr;
	};

	class VulkanOcclusionQuery : public IRHIOcclusionQuery
	{
	public:
		explicit VulkanOcclusionQuery(RenderDeviceVulkan* owner)
			: m_owner(owner)
			, m_slot(owner ? owner->AllocateOcclusionQuerySlot() : UINT32_MAX)
			, m_active(false)
			, m_issued(false)
		{
		}

		virtual ~VulkanOcclusionQuery() override
		{
			if (m_owner && m_slot != UINT32_MAX)
			{
				m_owner->FreeOcclusionQuerySlot(m_slot);
				m_slot = UINT32_MAX;
			}
		}

		virtual bool Begin() override
		{
			if (!m_owner || m_slot == UINT32_MAX || m_owner->GetOcclusionQueryPool() == VK_NULL_HANDLE)
				return false;
			VkCommandBuffer cmd = m_owner->EnsureRecording();
			if (!cmd)
				return false;

			// vkCmdResetQueryPool MUST occur outside a render pass
			m_owner->EndActiveRenderPass();
			m_owner->GetVK().vkCmdResetQueryPool(cmd, m_owner->GetOcclusionQueryPool(), m_slot, 1);

			if (!m_owner->EnsureActiveRenderPass())
				return false;

			m_owner->GetVK().vkCmdBeginQuery(cmd, m_owner->GetOcclusionQueryPool(), m_slot, 0);
			m_active = true;
			m_issued = false;
			return true;
		}

		virtual void End() override
		{
			if (!m_owner || m_slot == UINT32_MAX || !m_active || m_owner->GetOcclusionQueryPool() == VK_NULL_HANDLE)
				return;
			VkCommandBuffer cmd = m_owner->EnsureRecording();
			if (cmd)
			{
				m_owner->GetVK().vkCmdEndQuery(cmd, m_owner->GetOcclusionQueryPool(), m_slot);
				m_issued = true;
			}
			m_active = false;
		}

		virtual bool GetResult(uint64_t& outSamplesPassed, bool flush) override
		{
			outSamplesPassed = 0;
			if (!m_owner || m_slot == UINT32_MAX || !m_issued || m_owner->GetOcclusionQueryPool() == VK_NULL_HANDLE)
				return false;

			if (flush)
				m_owner->ExecuteAndSyncGPU();

			uint64_t count = 0;
			VkFlags flags = VK_QUERY_RESULT_64_BIT | (flush ? VK_QUERY_RESULT_WAIT_BIT : 0u);
			VkResult res = m_owner->GetVK().vkGetQueryPoolResults(
				m_owner->GetVkDevice(),
				m_owner->GetOcclusionQueryPool(),
				m_slot,
				1,
				sizeof(uint64_t),
				&count,
				sizeof(uint64_t),
				flags);
			if (res == VK_SUCCESS)
			{
				outSamplesPassed = count;
				return true;
			}
			return false;
		}

	private:
		RenderDeviceVulkan* m_owner;
		uint32_t m_slot;
		bool m_active;
		bool m_issued;
	};

	class VulkanShaderProgram : public IRHIShaderProgram
	{
	public:
		VulkanShaderProgram(RenderDeviceVulkan* owner, const RHIShaderProgramDesc& desc)
			: m_owner(owner)
			, m_debugName(desc.debugName ? desc.debugName : "")
		{
			if (!owner || !desc.vsBytecode || desc.vsBytecodeSize < 20 || (desc.vsBytecodeSize % 4) != 0)
				return;

			VkShaderModuleCreateInfo vsCi = {};
			vsCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			vsCi.codeSize = desc.vsBytecodeSize;
			vsCi.pCode = static_cast<const uint32_t*>(desc.vsBytecode);
			if (owner->GetVK().vkCreateShaderModule(owner->GetVkDevice(), &vsCi, nullptr, &m_vsModule) != VK_SUCCESS || !m_vsModule)
				return;

			m_vsHash = ComputeVkByteHash64(desc.vsBytecode, desc.vsBytecodeSize);
			if (desc.vsEntryPoint && desc.vsEntryPoint[0])
				m_vsEntry = desc.vsEntryPoint;

			ReflectSpirvModule(static_cast<const uint32_t*>(desc.vsBytecode), desc.vsBytecodeSize / 4, true);

			if (desc.psBytecode && desc.psBytecodeSize >= 20 && (desc.psBytecodeSize % 4) == 0)
			{
				VkShaderModuleCreateInfo psCi = {};
				psCi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				psCi.codeSize = desc.psBytecodeSize;
				psCi.pCode = static_cast<const uint32_t*>(desc.psBytecode);
				if (owner->GetVK().vkCreateShaderModule(owner->GetVkDevice(), &psCi, nullptr, &m_psModule) != VK_SUCCESS || !m_psModule)
				{
					owner->GetVK().vkDestroyShaderModule(owner->GetVkDevice(), m_vsModule, nullptr);
					m_vsModule = VK_NULL_HANDLE;
					return;
				}
				m_psHash = ComputeVkByteHash64(desc.psBytecode, desc.psBytecodeSize);
				if (desc.psEntryPoint && desc.psEntryPoint[0])
					m_psEntry = desc.psEntryPoint;

				ReflectSpirvModule(static_cast<const uint32_t*>(desc.psBytecode), desc.psBytecodeSize / 4, false);
			}

			m_valid = true;
		}

		virtual ~VulkanShaderProgram() override
		{
			if (m_owner)
			{
				m_owner->NotifyProgramDestroyed(this);
				if (m_vsModule != VK_NULL_HANDLE)
				{
					VulkanDeferredDelete dd;
					dd.shaderModule = m_vsModule;
					m_owner->DeferDelete(dd);
				}
				if (m_psModule != VK_NULL_HANDLE)
				{
					VulkanDeferredDelete dd;
					dd.shaderModule = m_psModule;
					m_owner->DeferDelete(dd);
				}
			}
		}

		virtual bool IsValid() const override { return m_valid; }
		virtual void* GetNativeHandle() const override { return static_cast<void*>(const_cast<VulkanShaderProgram*>(this)); }

		VkShaderModule GetVsModule() const { return m_vsModule; }
		VkShaderModule GetPsModule() const { return m_psModule; }
		const char* GetVsEntry() const { return m_vsEntry.c_str(); }
		const char* GetPsEntry() const { return m_psEntry.c_str(); }
		const char* GetDebugName() const { return m_debugName.c_str(); }
		uint64_t GetCombinedHash() const
		{
			uint64_t h = m_vsHash;
			HashCombineVkU64(h, m_psHash);
			return h;
		}
		uint32_t GetVsInputLocationMask() const { return m_vsInputLocationMask; }
		uint32_t GetPsOutputLocationMask() const { return m_psOutputLocationMask; }
		// 1 = 2D, 2 = 3D, 3 = Cube
		uint8_t GetExpectedSamplerDim(uint32_t slot) const { return (slot < 16) ? m_samplerSlotDims[slot] : 1; }

	private:
		void ReflectSpirvModule(const uint32_t* words, size_t wordCount, bool isVertex)
		{
			if (!words || wordCount < 5 || words[0] != 0x07230203u)
				return;

			std::unordered_map<uint32_t, uint8_t> imageTypeDim;   // typeId -> SpirvDim (1=2D, 2=3D, 3=Cube)
			std::unordered_map<uint32_t, uint32_t> ptrElemType;   // ptrTypeId -> elemTypeId
			std::unordered_map<uint32_t, uint32_t> varPtrType;    // varId -> ptrTypeId
			std::unordered_map<uint32_t, uint32_t> varStorage;    // varId -> StorageClass
			std::unordered_map<uint32_t, uint32_t> varLocation;   // varId -> Location
			std::unordered_map<uint32_t, uint32_t> varBinding;    // varId -> Binding
			std::vector<uint32_t> entryInterfaceVars;

			size_t pc = 5;
			while (pc < wordCount)
			{
				uint32_t header = words[pc];
				uint16_t op = static_cast<uint16_t>(header & 0xFFFFu);
				uint32_t wc = header >> 16;
				if (wc == 0 || pc + wc > wordCount)
					break;

				if (op == 15 && wc >= 4) // OpEntryPoint
				{
					uint32_t execModel = words[pc + 1];
					if ((isVertex && execModel == 0u) || (!isVertex && execModel == 4u))
					{
						// Skip literal string starting at words[pc + 3]
						size_t strIdx = pc + 3;
						while (strIdx < pc + wc)
						{
							uint32_t w = words[strIdx++];
							if ((w & 0xFFu) == 0 || ((w >> 8) & 0xFFu) == 0 || ((w >> 16) & 0xFFu) == 0 || ((w >> 24) & 0xFFu) == 0)
								break;
						}
						for (size_t i = strIdx; i < pc + wc; ++i)
							entryInterfaceVars.push_back(words[i]);
					}
				}
				else if (op == 25 && wc >= 4) // OpTypeImage
				{
					uint32_t resId = words[pc + 1];
					uint32_t dim = words[pc + 3]; // 0=1D, 1=2D, 2=3D, 3=Cube
					imageTypeDim[resId] = static_cast<uint8_t>(dim);
				}
				else if (op == 32 && wc >= 4) // OpTypePointer
				{
					ptrElemType[words[pc + 1]] = words[pc + 3];
				}
				else if (op == 59 && wc >= 4) // OpVariable
				{
					varPtrType[words[pc + 2]] = words[pc + 1];
					varStorage[words[pc + 2]] = words[pc + 3];
				}
				else if (op == 71 && wc >= 4) // OpDecorate
				{
					uint32_t targetId = words[pc + 1];
					uint32_t dec = words[pc + 2];
					uint32_t val = words[pc + 3];
					if (dec == 30u) // Location
						varLocation[targetId] = val;
					else if (dec == 33u) // Binding
						varBinding[targetId] = val;
				}
				else if (op == 54) // OpFunction: all declarations/decorations precede functions in SPIR-V
				{
					break;
				}

				pc += wc;
			}

			if (isVertex)
			{
				for (uint32_t vid : entryInterfaceVars)
				{
					auto sit = varStorage.find(vid);
					auto lit = varLocation.find(vid);
					if (sit != varStorage.end() && sit->second == 1u && lit != varLocation.end() && lit->second < 16u)
					{
						m_vsInputLocationMask |= (1u << lit->second);
					}
				}
			}
			else
			{
				for (uint32_t vid : entryInterfaceVars)
				{
					auto sit = varStorage.find(vid);
					auto lit = varLocation.find(vid);
					if (sit != varStorage.end() && sit->second == 3u && lit != varLocation.end() && lit->second < 8u)
					{
						m_psOutputLocationMask |= (1u << lit->second);
					}
				}
			}

			for (const auto& kv : varBinding)
			{
				uint32_t vid = kv.first;
				uint32_t binding = kv.second;
				if (binding >= 16u && binding < 32u)
				{
					uint32_t slot = binding - 16u;
					auto pit = varPtrType.find(vid);
					if (pit != varPtrType.end())
					{
						auto eit = ptrElemType.find(pit->second);
						if (eit != ptrElemType.end())
						{
							auto dit = imageTypeDim.find(eit->second);
							if (dit != imageTypeDim.end())
							{
								m_samplerSlotDims[slot] = dit->second;
							}
						}
					}
				}
			}
		}

	private:
		RenderDeviceVulkan* m_owner = nullptr;
		bool m_valid = false;
		VkShaderModule m_vsModule = VK_NULL_HANDLE;
		VkShaderModule m_psModule = VK_NULL_HANDLE;
		std::string m_vsEntry = "main";
		std::string m_psEntry = "main";
		std::string m_debugName;
		uint64_t m_vsHash = 0;
		uint64_t m_psHash = 0;
		uint32_t m_vsInputLocationMask = 0;
		uint32_t m_psOutputLocationMask = 0;
		uint8_t m_samplerSlotDims[16] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	};

	// =========================================================================
	// RenderDeviceVulkan Lifecycle, Memory, Swapchain & Initialization
	// =========================================================================

	RenderDeviceVulkan::RenderDeviceVulkan(
		const VulkanDispatch& vk,
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		VkSurfaceKHR surface,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkQueue graphicsQueue,
		uint32_t graphicsQueueFamily,
		const VkPhysicalDeviceProperties& physProps,
		const VkPhysicalDeviceFeatures& enabledFeatures,
		const VkPhysicalDeviceMemoryProperties& memProps,
		const std::string& adapterName)
		: m_vk(vk)
		, m_instance(instance)
		, m_debugMessenger(debugMessenger)
		, m_surface(surface)
		, m_physicalDevice(physicalDevice)
		, m_device(device)
		, m_graphicsQueue(graphicsQueue)
		, m_graphicsQueueFamily(graphicsQueueFamily)
		, m_physProps(physProps)
		, m_enabledFeatures(enabledFeatures)
		, m_memProps(memProps)
		, m_adapterName(adapterName)
	{
		VkFormatProperties ubyteProps = {};
		m_vk.vkGetPhysicalDeviceFormatProperties(m_physicalDevice, VK_FORMAT_R8G8B8A8_USCALED, &ubyteProps);
		m_supportsUbyte4Uscaled = (ubyteProps.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) != 0;

		InitCaps();
	}

	RenderDeviceVulkan::~RenderDeviceVulkan()
	{
		m_isShuttingDown = true;
		if (m_device != VK_NULL_HANDLE)
		{
			if (m_isRecording)
				ExecuteAndSyncGPU();
			m_vk.vkDeviceWaitIdle(m_device);
		}

		for (auto& kv : m_fvfDecls)
			delete kv.second;
		m_fvfDecls.clear();

		if (m_whiteFallback2D)
		{
			m_whiteFallback2D->ReleaseRHIResource();
			m_whiteFallback2D = nullptr;
		}

		if (m_device != VK_NULL_HANDLE)
		{
			if (m_whiteCubeView) m_vk.vkDestroyImageView(m_device, m_whiteCubeView, nullptr);
			if (m_whiteCubeImage) m_vk.vkDestroyImage(m_device, m_whiteCubeImage, nullptr);
			if (m_whiteCubeMem) m_vk.vkFreeMemory(m_device, m_whiteCubeMem, nullptr);

			if (m_white3DView) m_vk.vkDestroyImageView(m_device, m_white3DView, nullptr);
			if (m_white3DImage) m_vk.vkDestroyImage(m_device, m_white3DImage, nullptr);
			if (m_white3DMem) m_vk.vkFreeMemory(m_device, m_white3DMem, nullptr);

			if (m_zeroUboBuffer) m_vk.vkDestroyBuffer(m_device, m_zeroUboBuffer, nullptr);
			if (m_zeroUboMemory) m_vk.vkFreeMemory(m_device, m_zeroUboMemory, nullptr);

			if (m_defaultAttrVB) m_vk.vkDestroyBuffer(m_device, m_defaultAttrVB, nullptr);
			if (m_defaultAttrMemory) m_vk.vkFreeMemory(m_device, m_defaultAttrMemory, nullptr);

			if (m_occlusionPool) m_vk.vkDestroyQueryPool(m_device, m_occlusionPool, nullptr);

			for (auto& kv : m_pipelineCache)
				if (kv.second) m_vk.vkDestroyPipeline(m_device, kv.second, nullptr);
			m_pipelineCache.clear();

			for (auto& kv : m_framebufferCache)
				if (kv.second.framebuffer) m_vk.vkDestroyFramebuffer(m_device, kv.second.framebuffer, nullptr);
			m_framebufferCache.clear();

			for (auto& kv : m_renderPassCache)
				if (kv.second) m_vk.vkDestroyRenderPass(m_device, kv.second, nullptr);
			m_renderPassCache.clear();

			for (auto& kv : m_samplerCache)
				if (kv.second) m_vk.vkDestroySampler(m_device, kv.second, nullptr);
			m_samplerCache.clear();

			DestroySwapchainAndDefaultDepth();

			ReclaimCompletedResources(UINT64_MAX);

			if (m_pipelineLayout) m_vk.vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
			if (m_setLayout) m_vk.vkDestroyDescriptorSetLayout(m_device, m_setLayout, nullptr);

			for (uint32_t f = 0; f < FRAME_COUNT; ++f)
			{
				for (auto& page : m_frames[f].uploadPages)
				{
					if (page.cpuBase && page.memory)
						m_vk.vkUnmapMemory(m_device, page.memory);
					if (page.buffer)
						m_vk.vkDestroyBuffer(m_device, page.buffer, nullptr);
					if (page.memory)
						m_vk.vkFreeMemory(m_device, page.memory, nullptr);
				}
				m_frames[f].uploadPages.clear();

				for (VkDescriptorPool dp : m_frames[f].descriptorPools)
					if (dp) m_vk.vkDestroyDescriptorPool(m_device, dp, nullptr);
				m_frames[f].descriptorPools.clear();

				if (m_frames[f].inFlightFence) m_vk.vkDestroyFence(m_device, m_frames[f].inFlightFence, nullptr);
				if (m_frames[f].imageAvailableSem) m_vk.vkDestroySemaphore(m_device, m_frames[f].imageAvailableSem, nullptr);
				if (m_frames[f].cmdPool) m_vk.vkDestroyCommandPool(m_device, m_frames[f].cmdPool, nullptr);
			}

			m_vk.vkDestroyDevice(m_device, nullptr);
			m_device = VK_NULL_HANDLE;
		}

		if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE)
		{
			m_vk.vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
			m_surface = VK_NULL_HANDLE;
		}

		if (m_debugMessenger != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE && m_vk.vkDestroyDebugUtilsMessengerEXT)
		{
			m_vk.vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
			m_debugMessenger = VK_NULL_HANDLE;
		}

		if (m_instance != VK_NULL_HANDLE)
		{
			m_vk.vkDestroyInstance(m_instance, nullptr);
			m_instance = VK_NULL_HANDLE;
		}
	}

	void RenderDeviceVulkan::InitCaps()
	{
		m_caps.backend = ERHIBackend::Vulkan;
		m_caps.backendName = (m_physProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) ? "Vulkan (CPU)" : "Vulkan";
		m_caps.adapterName = m_adapterName.c_str();
		m_caps.featureLevel = 0xc000;
		m_caps.maxSimultaneousRTs = (std::min)(4u, (std::max)(1u, m_physProps.limits.maxColorAttachments));
		m_caps.maxTextureStages = 16u;
		m_caps.maxTextureSize = (std::min)(16384u, (std::max)(4096u, m_physProps.limits.maxImageDimension2D));
		m_caps.maxAnisotropy = (m_enabledFeatures.samplerAnisotropy == VK_TRUE)
			? static_cast<uint32_t>((std::min)(16.0f, m_physProps.limits.maxSamplerAnisotropy))
			: 1u;
		m_caps.maxVertexUniformVectors = (std::max)(256u, m_physProps.limits.maxUniformBufferRange / 16u);
		m_caps.isSoftwareAdapter = (m_physProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);
		m_caps.supportsFloatRenderTargets = true;
		m_caps.supportsDepthTextures = true;
		m_caps.supportsInstancing = true;
		m_caps.supportsOcclusionQuery = true;
		m_caps.supportsTwoSidedStencil = true;
		m_caps.supportsUserClipPlanes = false;
		m_caps.supportsBCCompression = (m_enabledFeatures.textureCompressionBC == VK_TRUE);
		m_caps.supportsETC2Compression = false;
		m_caps.supportsASTCCompression = false;
		m_caps.supportsAsyncReadback = true;
		m_caps.supportsRuntimeShaderCompile = true;
		m_caps.topLeftScreenOrigin = true;
		m_caps.clipSpaceZeroToOne = true;
		m_caps.needsHalfPixelOffset = false;
	}

	uint32_t RenderDeviceVulkan::FindMemoryType(uint32_t typeBits, VkFlags requiredProps) const
	{
		for (uint32_t i = 0; i < m_memProps.memoryTypeCount; ++i)
		{
			if ((typeBits & (1u << i)) != 0 &&
				(m_memProps.memoryTypes[i].propertyFlags & requiredProps) == requiredProps)
			{
				return i;
			}
		}
		return UINT32_MAX;
	}

	bool RenderDeviceVulkan::CreateVkBufferWithMemory(
		VkDeviceSize byteSize,
		VkFlags usage,
		VkFlags memProps,
		VkBuffer& outBuffer,
		VkDeviceMemory& outMemory) const
	{
		outBuffer = VK_NULL_HANDLE;
		outMemory = VK_NULL_HANDLE;

		VkBufferCreateInfo bci = {};
		bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bci.size = (std::max)(byteSize, static_cast<VkDeviceSize>(16));
		bci.usage = usage;
		bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (m_vk.vkCreateBuffer(m_device, &bci, nullptr, &outBuffer) != VK_SUCCESS || !outBuffer)
			return false;

		VkMemoryRequirements req = {};
		m_vk.vkGetBufferMemoryRequirements(m_device, outBuffer, &req);

		uint32_t memType = FindMemoryType(req.memoryTypeBits, memProps);
		if (memType == UINT32_MAX)
		{
			m_vk.vkDestroyBuffer(m_device, outBuffer, nullptr);
			outBuffer = VK_NULL_HANDLE;
			return false;
		}

		VkMemoryAllocateInfo mai = {};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = req.size;
		mai.memoryTypeIndex = memType;

		if (m_vk.vkAllocateMemory(m_device, &mai, nullptr, &outMemory) != VK_SUCCESS || !outMemory)
		{
			m_vk.vkDestroyBuffer(m_device, outBuffer, nullptr);
			outBuffer = VK_NULL_HANDLE;
			return false;
		}

		if (m_vk.vkBindBufferMemory(m_device, outBuffer, outMemory, 0) != VK_SUCCESS)
		{
			m_vk.vkDestroyBuffer(m_device, outBuffer, nullptr);
			m_vk.vkFreeMemory(m_device, outMemory, nullptr);
			outBuffer = VK_NULL_HANDLE;
			outMemory = VK_NULL_HANDLE;
			return false;
		}

		return true;
	}

	bool RenderDeviceVulkan::CreateVkImageWithMemory(
		const VkImageCreateInfo& imgInfo,
		VkFlags memProps,
		VkImage& outImage,
		VkDeviceMemory& outMemory) const
	{
		outImage = VK_NULL_HANDLE;
		outMemory = VK_NULL_HANDLE;

		if (m_vk.vkCreateImage(m_device, &imgInfo, nullptr, &outImage) != VK_SUCCESS || !outImage)
			return false;

		VkMemoryRequirements req = {};
		m_vk.vkGetImageMemoryRequirements(m_device, outImage, &req);

		uint32_t memType = FindMemoryType(req.memoryTypeBits, memProps);
		if (memType == UINT32_MAX)
		{
			m_vk.vkDestroyImage(m_device, outImage, nullptr);
			outImage = VK_NULL_HANDLE;
			return false;
		}

		VkMemoryAllocateInfo mai = {};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = req.size;
		mai.memoryTypeIndex = memType;

		if (m_vk.vkAllocateMemory(m_device, &mai, nullptr, &outMemory) != VK_SUCCESS || !outMemory)
		{
			m_vk.vkDestroyImage(m_device, outImage, nullptr);
			outImage = VK_NULL_HANDLE;
			return false;
		}

		if (m_vk.vkBindImageMemory(m_device, outImage, outMemory, 0) != VK_SUCCESS)
		{
			m_vk.vkDestroyImage(m_device, outImage, nullptr);
			m_vk.vkFreeMemory(m_device, outMemory, nullptr);
			outImage = VK_NULL_HANDLE;
			outMemory = VK_NULL_HANDLE;
			return false;
		}

		return true;
	}

	bool RenderDeviceVulkan::Initialize(uint32_t initialWidth, uint32_t initialHeight)
	{
		for (uint32_t f = 0; f < FRAME_COUNT; ++f)
		{
			VkCommandPoolCreateInfo cpci = {};
			cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			cpci.queueFamilyIndex = m_graphicsQueueFamily;
			if (m_vk.vkCreateCommandPool(m_device, &cpci, nullptr, &m_frames[f].cmdPool) != VK_SUCCESS)
				return false;

			VkCommandBufferAllocateInfo cbai = {};
			cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cbai.commandPool = m_frames[f].cmdPool;
			cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cbai.commandBufferCount = 1;
			if (m_vk.vkAllocateCommandBuffers(m_device, &cbai, &m_frames[f].cmdBuf) != VK_SUCCESS)
				return false;

			VkFenceCreateInfo fci = {};
			fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (m_vk.vkCreateFence(m_device, &fci, nullptr, &m_frames[f].inFlightFence) != VK_SUCCESS)
				return false;

			VkSemaphoreCreateInfo sci = {};
			sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			if (m_vk.vkCreateSemaphore(m_device, &sci, nullptr, &m_frames[f].imageAvailableSem) != VK_SUCCESS)
				return false;

			VkDescriptorPool dp = CreateNewDescriptorPoolChunk();
			if (!dp)
				return false;
			m_frames[f].descriptorPools.push_back(dp);
		}

		if (!CreateDescriptorLayoutsAndDefaults())
			return false;

		if (!CreateSwapchainAndDefaultDepth(initialWidth, initialHeight))
			return false;

		VkQueryPoolCreateInfo qpci = {};
		qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
		qpci.queryCount = MAX_OCCLUSION_QUERIES;
		m_vk.vkCreateQueryPool(m_device, &qpci, nullptr, &m_occlusionPool);

		ParaViewport vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = m_backBufferWidth;
		vp.Height = m_backBufferHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		SetViewport(vp);

		ExecuteAndSyncGPU();
		return true;
	}

	bool RenderDeviceVulkan::CreateDescriptorLayoutsAndDefaults()
	{
		// Set 0 matches SpirvCodeGen:
		// Binding 0: VS $Globals UBO
		// Binding 1: PS $Globals UBO
		// Bindings 16..31: SAMPLED_IMAGE (slots 0..15)
		// Bindings 32..47: SAMPLER (slots 0..15)
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.reserve(34);

		VkDescriptorSetLayoutBinding bVsUbo = {};
		bVsUbo.binding = 0;
		bVsUbo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bVsUbo.descriptorCount = 1;
		bVsUbo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings.push_back(bVsUbo);

		VkDescriptorSetLayoutBinding bPsUbo = {};
		bPsUbo.binding = 1;
		bPsUbo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bPsUbo.descriptorCount = 1;
		bPsUbo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(bPsUbo);

		for (uint32_t s = 0; s < 16; ++s)
		{
			VkDescriptorSetLayoutBinding bImg = {};
			bImg.binding = 16u + s;
			bImg.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			bImg.descriptorCount = 1;
			bImg.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			bindings.push_back(bImg);

			VkDescriptorSetLayoutBinding bSmp = {};
			bSmp.binding = 32u + s;
			bSmp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			bSmp.descriptorCount = 1;
			bSmp.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			bindings.push_back(bSmp);
		}

		VkDescriptorSetLayoutCreateInfo slci = {};
		slci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		slci.bindingCount = static_cast<uint32_t>(bindings.size());
		slci.pBindings = bindings.data();
		if (m_vk.vkCreateDescriptorSetLayout(m_device, &slci, nullptr, &m_setLayout) != VK_SUCCESS || !m_setLayout)
			return false;

		VkPipelineLayoutCreateInfo plci = {};
		plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		plci.setLayoutCount = 1;
		plci.pSetLayouts = &m_setLayout;
		if (m_vk.vkCreatePipelineLayout(m_device, &plci, nullptr, &m_pipelineLayout) != VK_SUCCESS || !m_pipelineLayout)
			return false;

		// Zero UBO fallback (4 KB)
		if (!CreateVkBufferWithMemory(
			4096,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_zeroUboBuffer,
			m_zeroUboMemory))
		{
			return false;
		}
		{
			void* p = nullptr;
			if (m_vk.vkMapMemory(m_device, m_zeroUboMemory, 0, 4096, 0, &p) == VK_SUCCESS && p)
			{
				std::memset(p, 0, 4096);
				m_vk.vkUnmapMemory(m_device, m_zeroUboMemory);
			}
		}
		m_currentVsUboBuf = m_zeroUboBuffer;
		m_currentVsUboOffset = 0;
		m_currentVsUboRange = 256;
		m_currentPsUboBuf = m_zeroUboBuffer;
		m_currentPsUboOffset = 0;
		m_currentPsUboRange = 256;

		// Default vertex attribute buffer [0, 0, 0, 1] at binding 15 (stride = 0)
		if (!CreateVkBufferWithMemory(
			64,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_defaultAttrVB,
			m_defaultAttrMemory))
		{
			return false;
		}
		{
			void* p = nullptr;
			if (m_vk.vkMapMemory(m_device, m_defaultAttrMemory, 0, 64, 0, &p) == VK_SUCCESS && p)
			{
				const float defAttr[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				std::memset(p, 0, 64);
				std::memcpy(p, defAttr, sizeof(defAttr));
				m_vk.vkUnmapMemory(m_device, m_defaultAttrMemory);
			}
		}

		// 1x1 2D white fallback texture
		const uint32_t whitePixel = 0xFFFFFFFFu;
		RHIMipData whiteMip = { &whitePixel, 4, 4 };
		m_whiteFallback2D = static_cast<VulkanTexture*>(CreateTexture(
			1, 1, PixelFormat::A8R8G8B8, 1, ERHITextureUsage::ShaderResource, &whiteMip, 1));
		if (!m_whiteFallback2D)
			return false;

		// 1x1x6 Cube white fallback & 1x1x1 3D white fallback
		{
			VkImageCreateInfo ci = {};
			ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT_LOCAL;
			ci.imageType = VK_IMAGE_TYPE_2D;
			ci.format = VK_FORMAT_R8G8B8A8_UNORM;
			ci.extent = { 1, 1, 1 };
			ci.mipLevels = 1;
			ci.arrayLayers = 6;
			ci.samples = VK_SAMPLE_COUNT_1_BIT;
			ci.tiling = VK_IMAGE_TILING_OPTIMAL;
			ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			if (CreateVkImageWithMemory(ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_whiteCubeImage, m_whiteCubeMem))
			{
				VkImageViewCreateInfo vi = {};
				vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				vi.image = m_whiteCubeImage;
				vi.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				vi.format = VK_FORMAT_R8G8B8A8_UNORM;
				vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6 };
				m_vk.vkCreateImageView(m_device, &vi, nullptr, &m_whiteCubeView);
			}

			VkImageCreateInfo i3d = {};
			i3d.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			i3d.imageType = VK_IMAGE_TYPE_3D;
			i3d.format = VK_FORMAT_R8G8B8A8_UNORM;
			i3d.extent = { 1, 1, 1 };
			i3d.mipLevels = 1;
			i3d.arrayLayers = 1;
			i3d.samples = VK_SAMPLE_COUNT_1_BIT;
			i3d.tiling = VK_IMAGE_TILING_OPTIMAL;
			i3d.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			i3d.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			i3d.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			if (CreateVkImageWithMemory(i3d, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_white3DImage, m_white3DMem))
			{
				VkImageViewCreateInfo vi = {};
				vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				vi.image = m_white3DImage;
				vi.viewType = VK_IMAGE_VIEW_TYPE_3D;
				vi.format = VK_FORMAT_R8G8B8A8_UNORM;
				vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
				m_vk.vkCreateImageView(m_device, &vi, nullptr, &m_white3DView);
			}

			const uint32_t whiteFaces[6] = {
				0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
				0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu
			};
			void* upCpu = nullptr;
			VkBuffer upBuf = VK_NULL_HANDLE;
			VkDeviceSize upOff = 0;
			VkCommandBuffer cmd = EnsureRecording();
			if (cmd && AllocateUpload(sizeof(whiteFaces), 16, upCpu, upBuf, upOff))
			{
				std::memcpy(upCpu, whiteFaces, sizeof(whiteFaces));
				if (m_whiteCubeImage)
				{
					VkImageLayout lay = VK_IMAGE_LAYOUT_UNDEFINED;
					TransitionImageLayout(cmd, m_whiteCubeImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 6, lay, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
					VkBufferImageCopy bic = {};
					bic.bufferOffset = upOff;
					bic.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 6 };
					bic.imageExtent = { 1, 1, 1 };
					m_vk.vkCmdCopyBufferToImage(cmd, upBuf, m_whiteCubeImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);
					TransitionImageLayout(cmd, m_whiteCubeImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 6, lay, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				if (m_white3DImage)
				{
					VkImageLayout lay = VK_IMAGE_LAYOUT_UNDEFINED;
					TransitionImageLayout(cmd, m_white3DImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, lay, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
					VkBufferImageCopy bic = {};
					bic.bufferOffset = upOff;
					bic.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
					bic.imageExtent = { 1, 1, 1 };
					m_vk.vkCmdCopyBufferToImage(cmd, upBuf, m_white3DImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);
					TransitionImageLayout(cmd, m_white3DImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, lay, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
			}
		}

		return true;
	}

	void RenderDeviceVulkan::DestroySwapchainAndDefaultDepth()
	{
		for (uint64_t vid : m_swapViewIds)
			if (vid != 0) NotifyViewDestroyed(vid);
		m_swapViewIds.clear();

		if (m_defaultDepthViewId != 0)
		{
			NotifyViewDestroyed(m_defaultDepthViewId);
			m_defaultDepthViewId = 0;
		}

		for (VkSemaphore s : m_presentSemaphores)
			if (s) m_vk.vkDestroySemaphore(m_device, s, nullptr);
		m_presentSemaphores.clear();

		for (VkImageView v : m_swapImageViews)
			if (v) m_vk.vkDestroyImageView(m_device, v, nullptr);
		m_swapImageViews.clear();
		m_swapImages.clear();
		m_swapImageLayouts.clear();

		if (m_defaultDepthView)
		{
			m_vk.vkDestroyImageView(m_device, m_defaultDepthView, nullptr);
			m_defaultDepthView = VK_NULL_HANDLE;
		}
		if (m_defaultDepthImage)
		{
			m_vk.vkDestroyImage(m_device, m_defaultDepthImage, nullptr);
			m_defaultDepthImage = VK_NULL_HANDLE;
		}
		if (m_defaultDepthMemory)
		{
			m_vk.vkFreeMemory(m_device, m_defaultDepthMemory, nullptr);
			m_defaultDepthMemory = VK_NULL_HANDLE;
		}

		if (m_swapchain)
		{
			m_vk.vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
			m_swapchain = VK_NULL_HANDLE;
		}

		m_hasAcquiredSwapImage = false;
		m_waitAcquireSemPending = false;
	}

	bool RenderDeviceVulkan::CreateSwapchainAndDefaultDepth(uint32_t width, uint32_t height)
	{
		VkSurfaceCapabilitiesKHR surfCaps = {};
		if (m_vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfCaps) != VK_SUCCESS)
			return false;

		uint32_t fmtCount = 0;
		m_vk.vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &fmtCount, nullptr);
		if (fmtCount == 0)
			return false;
		std::vector<VkSurfaceFormatKHR> surfFormats(fmtCount);
		m_vk.vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &fmtCount, surfFormats.data());

		VkSurfaceFormatKHR chosenFmt = surfFormats[0];
		if (fmtCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			chosenFmt = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		else
		{
			bool foundRGBA = false;
			for (const auto& sf : surfFormats)
			{
				if (sf.format == VK_FORMAT_R8G8B8A8_UNORM && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					chosenFmt = sf;
					foundRGBA = true;
					break;
				}
			}
			if (!foundRGBA)
			{
				for (const auto& sf : surfFormats)
				{
					if (sf.format == VK_FORMAT_B8G8R8A8_UNORM && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					{
						chosenFmt = sf;
						break;
					}
				}
			}
		}
		m_swapFormat = chosenFmt.format;
		m_swapColorSpace = chosenFmt.colorSpace;

		uint32_t pmCount = 0;
		m_vk.vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &pmCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(pmCount);
		if (pmCount > 0)
			m_vk.vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &pmCount, presentModes.data());

		VkPresentModeKHR chosenPM = VK_PRESENT_MODE_FIFO_KHR;
		if (!m_vsyncEnabled)
		{
			for (VkPresentModeKHR pm : presentModes)
			{
				if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { chosenPM = pm; break; }
				if (pm == VK_PRESENT_MODE_IMMEDIATE_KHR) chosenPM = pm;
			}
		}

		VkExtent2D extent = { width, height };
		if (surfCaps.currentExtent.width != 0xFFFFFFFFu && surfCaps.currentExtent.width > 0 && surfCaps.currentExtent.height > 0)
		{
			extent = surfCaps.currentExtent;
		}
		else
		{
			extent.width = (std::max)(surfCaps.minImageExtent.width, (std::min)(surfCaps.maxImageExtent.width, (std::max)(1u, width)));
			extent.height = (std::max)(surfCaps.minImageExtent.height, (std::min)(surfCaps.maxImageExtent.height, (std::max)(1u, height)));
		}

		uint32_t imageCount = (std::max)(FRAME_COUNT, surfCaps.minImageCount);
		if (surfCaps.maxImageCount > 0 && imageCount > surfCaps.maxImageCount)
			imageCount = surfCaps.maxImageCount;

		VkFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VkFlags preTransform = (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
			: surfCaps.currentTransform;

		VkSwapchainCreateInfoKHR sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		sci.surface = m_surface;
		sci.minImageCount = imageCount;
		sci.imageFormat = m_swapFormat;
		sci.imageColorSpace = m_swapColorSpace;
		sci.imageExtent = extent;
		sci.imageArrayLayers = 1;
		sci.imageUsage = usage;
		sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		sci.preTransform = preTransform;
		sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		sci.presentMode = chosenPM;
		sci.clipped = VK_TRUE;
		sci.oldSwapchain = VK_NULL_HANDLE;

		if (m_vk.vkCreateSwapchainKHR(m_device, &sci, nullptr, &m_swapchain) != VK_SUCCESS || !m_swapchain)
			return false;

		uint32_t actualImgs = 0;
		m_vk.vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImgs, nullptr);
		if (actualImgs == 0)
			return false;
		m_swapImages.resize(actualImgs);
		m_vk.vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImgs, m_swapImages.data());

		m_swapImageViews.resize(actualImgs, VK_NULL_HANDLE);
		m_swapViewIds.resize(actualImgs, 0);
		m_swapImageLayouts.assign(actualImgs, VK_IMAGE_LAYOUT_UNDEFINED);
		m_presentSemaphores.resize(actualImgs, VK_NULL_HANDLE);

		for (uint32_t i = 0; i < actualImgs; ++i)
		{
			VkImageViewCreateInfo vi = {};
			vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			vi.image = m_swapImages[i];
			vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vi.format = m_swapFormat;
			vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			if (m_vk.vkCreateImageView(m_device, &vi, nullptr, &m_swapImageViews[i]) != VK_SUCCESS)
				return false;
			m_swapViewIds[i] = NextUniqueViewId();

			VkSemaphoreCreateInfo semCi = {};
			semCi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			if (m_vk.vkCreateSemaphore(m_device, &semCi, nullptr, &m_presentSemaphores[i]) != VK_SUCCESS)
				return false;
		}

		// Default depth-stencil image
		VkFormat depthCandidates[] = {
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT
		};
		m_defaultDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		VkFlags depthOptimalFeatures = 0;
		for (VkFormat df : depthCandidates)
		{
			VkFormatProperties fp = {};
			m_vk.vkGetPhysicalDeviceFormatProperties(m_physicalDevice, df, &fp);
			if (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_defaultDepthFormat = df;
				depthOptimalFeatures = fp.optimalTilingFeatures;
				break;
			}
		}
		m_defaultDepthHasStencil = HasStencilInVkDepthFormat(m_defaultDepthFormat);

		VkImageCreateInfo dInfo = {};
		dInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		dInfo.imageType = VK_IMAGE_TYPE_2D;
		dInfo.format = m_defaultDepthFormat;
		dInfo.extent = { extent.width, extent.height, 1 };
		dInfo.mipLevels = 1;
		dInfo.arrayLayers = 1;
		dInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		dInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		dInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		dInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		dInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		(void)depthOptimalFeatures;

		if (!CreateVkImageWithMemory(dInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_defaultDepthImage, m_defaultDepthMemory))
			return false;

		VkImageViewCreateInfo dVi = {};
		dVi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		dVi.image = m_defaultDepthImage;
		dVi.viewType = VK_IMAGE_VIEW_TYPE_2D;
		dVi.format = m_defaultDepthFormat;
		dVi.subresourceRange = { AspectMaskForVkFormat(m_defaultDepthFormat), 0, 1, 0, 1 };
		if (m_vk.vkCreateImageView(m_device, &dVi, nullptr, &m_defaultDepthView) != VK_SUCCESS)
			return false;
		m_defaultDepthViewId = NextUniqueViewId();
		m_defaultDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		m_backBufferWidth = extent.width;
		m_backBufferHeight = extent.height;
		if (m_usingBackBufferRT)
		{
			m_renderTargetWidth = extent.width;
			m_renderTargetHeight = extent.height;
			m_activeColorFormats[0] = m_swapFormat;
			m_activeColorCount = 1;
			m_activeSampleCount = 1;
		}
		if (m_usingDefaultDepth)
		{
			m_hasActiveDepth = true;
			m_activeDepthFormat = m_defaultDepthFormat;
		}

		return true;
	}

	bool RenderDeviceVulkan::ResizeSwapChain(uint32_t width, uint32_t height, bool /*isWindowed*/)
	{
		if (width == 0 || height == 0)
			return true;
		if (width == m_backBufferWidth && height == m_backBufferHeight && m_swapchain != VK_NULL_HANDLE)
			return true;

		if (m_isRecording)
			ExecuteAndSyncGPU();
		m_vk.vkDeviceWaitIdle(m_device);

		DestroySwapchainAndDefaultDepth();
		if (!CreateSwapchainAndDefaultDepth(width, height))
			return false;

		if (m_usingBackBufferRT)
		{
			ParaViewport vp;
			vp.X = 0;
			vp.Y = 0;
			vp.Width = m_backBufferWidth;
			vp.Height = m_backBufferHeight;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			SetViewport(vp);
		}
		return true;
	}

	// =========================================================================
	// Command Recording, Synchronization, RenderPass, Framebuffer & PSO Cache
	// =========================================================================

	void RenderDeviceVulkan::ReclaimCompletedResources(uint64_t completedSerial)
	{
		size_t writeIdx = 0;
		for (size_t i = 0; i < m_deferredDeletes.size(); ++i)
		{
			VulkanDeferredDelete& d = m_deferredDeletes[i];
			if (d.retireSerial <= completedSerial)
			{
				if (d.viewUniqueId != 0)
					NotifyViewDestroyed(d.viewUniqueId);
				if (d.pipeline) m_vk.vkDestroyPipeline(m_device, d.pipeline, nullptr);
				if (d.framebuffer) m_vk.vkDestroyFramebuffer(m_device, d.framebuffer, nullptr);
				if (d.imageView) m_vk.vkDestroyImageView(m_device, d.imageView, nullptr);
				if (d.image) m_vk.vkDestroyImage(m_device, d.image, nullptr);
				if (d.buffer) m_vk.vkDestroyBuffer(m_device, d.buffer, nullptr);
				if (d.shaderModule) m_vk.vkDestroyShaderModule(m_device, d.shaderModule, nullptr);
				if (d.memory) m_vk.vkFreeMemory(m_device, d.memory, nullptr);
			}
			else
			{
				if (writeIdx != i)
					m_deferredDeletes[writeIdx] = d;
				++writeIdx;
			}
		}
		m_deferredDeletes.resize(writeIdx);
	}

	void RenderDeviceVulkan::DeferDelete(const VulkanDeferredDelete& entry)
	{
		if (!m_device)
			return;
		VulkanDeferredDelete copy = entry;
		copy.retireSerial = m_nextSubmitSerial;
		m_deferredDeletes.push_back(copy);
	}

	void RenderDeviceVulkan::NotifyViewDestroyed(uint64_t viewUniqueId)
	{
		if (viewUniqueId == 0 || !m_device)
			return;
		for (auto it = m_framebufferCache.begin(); it != m_framebufferCache.end(); )
		{
			bool match = false;
			for (uint32_t i = 0; i < it->second.viewCount; ++i)
			{
				if (it->second.viewIds[i] == viewUniqueId)
				{
					match = true;
					break;
				}
			}
			if (match)
			{
				VulkanDeferredDelete dd;
				dd.framebuffer = it->second.framebuffer;
				DeferDelete(dd);
				it = m_framebufferCache.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void RenderDeviceVulkan::NotifyTextureDestroyed(VulkanTexture* tex)
	{
		if (!tex || m_isShuttingDown)
			return;
		for (uint32_t i = 0; i < RHIStateTracker::MAX_SAMPLER_STAGES; ++i)
		{
			if (m_boundTextures[i] == tex)
			{
				m_boundTextures[i] = nullptr;
				m_descriptorsDirty = true;
			}
		}
		bool activeChanged = false;
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (m_activeColorTexObjs[i] == tex)
			{
				m_activeColorTexObjs[i] = nullptr;
				activeChanged = true;
			}
		}
		if (m_activeDepthTexObj == tex)
		{
			m_activeDepthTexObj = nullptr;
			activeChanged = true;
		}
		if (activeChanged)
			EndActiveRenderPass();
	}

	void RenderDeviceVulkan::NotifyBufferDestroyed(VulkanBuffer* buf)
	{
		if (!buf || m_isShuttingDown)
			return;
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (m_streams[i].buffer == buf)
			{
				m_streams[i].buffer = nullptr;
				m_streams[i].offsetInBytes = 0;
				m_streams[i].stride = 0;
			}
		}
		if (m_currentIndexBuffer == buf)
			m_currentIndexBuffer = nullptr;
	}

	void RenderDeviceVulkan::NotifyProgramDestroyed(VulkanShaderProgram* prog)
	{
		if (!prog || m_isShuttingDown)
			return;
		if (m_currentProgram == prog)
			m_currentProgram = nullptr;
	}

	void RenderDeviceVulkan::NotifyRenderTargetDestroyed(VulkanRenderTarget* rt)
	{
		if (!rt || m_isShuttingDown)
			return;
		bool activeMatch = (m_currentRTBinding.depthStencilOverride == rt);
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (m_currentRTBinding.targets[i] == rt)
				activeMatch = true;
		}
		if (activeMatch)
		{
			IRHIRenderTarget* newTargets[4] = {};
			uint32_t newCount = 0;
			for (uint32_t i = 0; i < m_currentRTBinding.count && i < 4; ++i)
			{
				newTargets[i] = (m_currentRTBinding.targets[i] == rt) ? nullptr : m_currentRTBinding.targets[i];
				if (newTargets[i])
					newCount = i + 1;
			}
			IRHIRenderTarget* newDS = (m_currentRTBinding.depthStencilOverride == rt) ? nullptr : m_currentRTBinding.depthStencilOverride;
			SetRenderTargets(newCount, newCount > 0 ? newTargets : nullptr, newDS);
		}
	}

	void RenderDeviceVulkan::BeginFrameSlotIfNeeded(uint32_t frameSlot)
	{
		FrameContext& fc = m_frames[frameSlot];
		if (fc.resetDone)
			return;

		m_vk.vkWaitForFences(m_device, 1, &fc.inFlightFence, VK_TRUE, UINT64_MAX);
		if (fc.submitSerial > m_completedSubmitSerial)
			m_completedSubmitSerial = fc.submitSerial;

		ReclaimCompletedResources(m_completedSubmitSerial);

		m_vk.vkResetCommandPool(m_device, fc.cmdPool, 0);

		for (VkDescriptorPool dp : fc.descriptorPools)
			m_vk.vkResetDescriptorPool(m_device, dp, 0);
		fc.activePoolIdx = 0;

		if (fc.uploadPages.size() > 1)
		{
			// Consolidate into the largest page so steady-state uses 1 page per frame
			VkDeviceSize maxCap = 0;
			size_t keepIdx = 0;
			for (size_t i = 0; i < fc.uploadPages.size(); ++i)
			{
				if (fc.uploadPages[i].capacity > maxCap)
				{
					maxCap = fc.uploadPages[i].capacity;
					keepIdx = i;
				}
			}
			UploadPage kept = fc.uploadPages[keepIdx];
			for (size_t i = 0; i < fc.uploadPages.size(); ++i)
			{
				if (i == keepIdx) continue;
				if (fc.uploadPages[i].cpuBase && fc.uploadPages[i].memory)
					m_vk.vkUnmapMemory(m_device, fc.uploadPages[i].memory);
				if (fc.uploadPages[i].buffer)
					m_vk.vkDestroyBuffer(m_device, fc.uploadPages[i].buffer, nullptr);
				if (fc.uploadPages[i].memory)
					m_vk.vkFreeMemory(m_device, fc.uploadPages[i].memory, nullptr);
			}
			fc.uploadPages.clear();
			kept.offset = 0;
			fc.uploadPages.push_back(kept);
		}
		else if (!fc.uploadPages.empty())
		{
			fc.uploadPages[0].offset = 0;
		}

		fc.resetDone = true;
	}

	VkCommandBuffer RenderDeviceVulkan::EnsureRecording()
	{
		if (!m_device)
			return VK_NULL_HANDLE;
		if (m_isRecording)
			return m_frames[m_frameIndex].cmdBuf;

		BeginFrameSlotIfNeeded(m_frameIndex);
		FrameContext& fc = m_frames[m_frameIndex];

		VkCommandBufferBeginInfo bi = {};
		bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		if (m_vk.vkBeginCommandBuffer(fc.cmdBuf, &bi) != VK_SUCCESS)
			return VK_NULL_HANDLE;

		m_isRecording = true;
		m_inRenderPass = false;
		m_activeVkRenderPass = VK_NULL_HANDLE;
		m_descriptorsDirty = true;
		m_lastDescriptorSet = VK_NULL_HANDLE;
		return fc.cmdBuf;
	}

	void RenderDeviceVulkan::EndActiveRenderPass()
	{
		if (m_isRecording && m_inRenderPass)
		{
			m_vk.vkCmdEndRenderPass(m_frames[m_frameIndex].cmdBuf);
			m_inRenderPass = false;
			m_activeVkRenderPass = VK_NULL_HANDLE;
		}
	}

	bool RenderDeviceVulkan::EnsureSwapchainImageAcquired()
	{
		if (m_hasAcquiredSwapImage)
			return true;
		if (!m_swapchain)
			return false;

		BeginFrameSlotIfNeeded(m_frameIndex);
		VkSemaphore acqSem = m_frames[m_frameIndex].imageAvailableSem;

		uint32_t imgIdx = 0;
		VkResult res = m_vk.vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, acqSem, VK_NULL_HANDLE, &imgIdx);
		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (!ResizeSwapChain(m_backBufferWidth, m_backBufferHeight, true))
				return false;
			BeginFrameSlotIfNeeded(m_frameIndex);
			res = m_vk.vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_frames[m_frameIndex].imageAvailableSem, VK_NULL_HANDLE, &imgIdx);
		}

		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
			return false;

		m_acquiredImageIndex = imgIdx;
		m_hasAcquiredSwapImage = true;
		m_waitAcquireSemPending = true;
		return true;
	}

	void RenderDeviceVulkan::ExecuteAndSyncGPU()
	{
		if (!m_device || !m_graphicsQueue)
			return;

		if (m_isRecording)
		{
			EndActiveRenderPass();
			FrameContext& fc = m_frames[m_frameIndex];
			m_vk.vkEndCommandBuffer(fc.cmdBuf);
			m_isRecording = false;

			m_vk.vkResetFences(m_device, 1, &fc.inFlightFence);

			VkFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo si = {};
			si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			if (m_waitAcquireSemPending)
			{
				si.waitSemaphoreCount = 1;
				si.pWaitSemaphores = &fc.imageAvailableSem;
				si.pWaitDstStageMask = &waitStage;
				m_waitAcquireSemPending = false;
			}
			si.commandBufferCount = 1;
			si.pCommandBuffers = &fc.cmdBuf;

			const uint64_t serial = m_nextSubmitSerial++;
			fc.submitSerial = serial;
			m_vk.vkQueueSubmit(m_graphicsQueue, 1, &si, fc.inFlightFence);
			m_vk.vkWaitForFences(m_device, 1, &fc.inFlightFence, VK_TRUE, UINT64_MAX);
			m_completedSubmitSerial = serial;

			// Advance to next frame slot or mark current slot ready to reset
			fc.resetDone = false;
			BeginFrameSlotIfNeeded(m_frameIndex);

			// Reset volatile upload UBO pointers to safe zero UBO so next recording starts cleanly
			m_currentVsUboBuf = m_zeroUboBuffer;
			m_currentVsUboOffset = 0;
			m_currentVsUboRange = 256;
			m_currentPsUboBuf = m_zeroUboBuffer;
			m_currentPsUboOffset = 0;
			m_currentPsUboRange = 256;
			m_descriptorsDirty = true;
		}
		else
		{
			m_vk.vkQueueWaitIdle(m_graphicsQueue);
			ReclaimCompletedResources(m_nextSubmitSerial);
		}
	}

	bool RenderDeviceVulkan::AllocateUpload(
		uint32_t byteSize,
		uint32_t alignment,
		void*& outCpuPtr,
		VkBuffer& outBuffer,
		VkDeviceSize& outOffset)
	{
		if (!m_device || byteSize == 0)
			return false;

		EnsureRecording();
		uint64_t minUboAlign = (std::max)(static_cast<uint64_t>(alignment), static_cast<uint64_t>(16));
		FrameContext& fc = m_frames[m_frameIndex];

		if (!fc.uploadPages.empty())
		{
			UploadPage& cur = fc.uploadPages.back();
			uint64_t aligned = AlignUpVkU64(cur.offset, minUboAlign);
			if (aligned + byteSize <= cur.capacity)
			{
				outCpuPtr = cur.cpuBase + aligned;
				outBuffer = cur.buffer;
				outOffset = aligned;
				cur.offset = aligned + byteSize;
				return true;
			}
		}

		uint64_t prevCap = fc.uploadPages.empty() ? (4ULL * 1024ULL * 1024ULL) : (fc.uploadPages.back().capacity * 2ULL);
		uint64_t newCap = (std::max)(AlignUpVkU64(byteSize, 65536ULL), prevCap);

		UploadPage page;
		VkFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			| VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
			| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
			| VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (!CreateVkBufferWithMemory(
			newCap,
			usage,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			page.buffer,
			page.memory))
		{
			return false;
		}

		void* mapped = nullptr;
		if (m_vk.vkMapMemory(m_device, page.memory, 0, newCap, 0, &mapped) != VK_SUCCESS || !mapped)
		{
			m_vk.vkDestroyBuffer(m_device, page.buffer, nullptr);
			m_vk.vkFreeMemory(m_device, page.memory, nullptr);
			return false;
		}

		page.cpuBase = static_cast<uint8_t*>(mapped);
		page.capacity = newCap;
		page.offset = byteSize;

		outCpuPtr = page.cpuBase;
		outBuffer = page.buffer;
		outOffset = 0;
		fc.uploadPages.push_back(page);
		return true;
	}

	void RenderDeviceVulkan::TransitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		VkFlags aspectMask,
		uint32_t mipLevels,
		uint32_t layerCount,
		VkImageLayout& ioCurrentLayout,
		VkImageLayout targetLayout)
	{
		if (!cmd || !image || ioCurrentLayout == targetLayout)
			return;

		EndActiveRenderPass();

		auto stageAndAccessFor = [](VkImageLayout lay, VkFlags& outStage, VkFlags& outAccess) {
			switch (lay)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				outStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				outAccess = 0;
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				outStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				outAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				outStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				outAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				outStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				outAccess = VK_ACCESS_SHADER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				outStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				outAccess = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				outStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				outAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				outStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				outAccess = 0;
				break;
			default:
				outStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				outAccess = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
				break;
			}
		};

		VkFlags srcStage = 0, srcAccess = 0;
		VkFlags dstStage = 0, dstAccess = 0;
		stageAndAccessFor(ioCurrentLayout, srcStage, srcAccess);
		stageAndAccessFor(targetLayout, dstStage, dstAccess);
		if (dstStage == VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
			dstAccess = 0;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = srcAccess;
		barrier.dstAccessMask = dstAccess;
		barrier.oldLayout = ioCurrentLayout;
		barrier.newLayout = targetLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = { aspectMask, 0, mipLevels, 0, layerCount };

		m_vk.vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		ioCurrentLayout = targetLayout;
	}

	VkRenderPass RenderDeviceVulkan::GetOrCreateRenderPass(
		uint32_t colorCount,
		const VkFormat* colorFormats,
		bool hasDepth,
		VkFormat depthFormat,
		uint32_t sampleCount)
	{
		uint64_t h = 14695981039346656037ULL;
		HashCombineVkU32(h, colorCount | (sampleCount << 8) | (hasDepth ? 0x10000u : 0u));
		for (uint32_t i = 0; i < colorCount; ++i)
			HashCombineVkU32(h, static_cast<uint32_t>(colorFormats[i]));
		HashCombineVkU32(h, hasDepth ? static_cast<uint32_t>(depthFormat) : 0u);

		auto it = m_renderPassCache.find(h);
		if (it != m_renderPassCache.end())
			return it->second;

		VkAttachmentDescription atts[5] = {};
		VkAttachmentReference colorRefs[4] = {};
		uint32_t attCount = 0;

		for (uint32_t i = 0; i < colorCount; ++i)
		{
			VkAttachmentDescription& a = atts[attCount];
			a.format = colorFormats[i];
			a.samples = VK_SAMPLE_COUNT_1_BIT;
			a.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			a.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			a.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			colorRefs[i] = { attCount, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			++attCount;
		}

		VkAttachmentReference depthRef = {};
		if (hasDepth)
		{
			bool hasSt = HasStencilInVkDepthFormat(depthFormat);
			VkAttachmentDescription& da = atts[attCount];
			da.format = depthFormat;
			da.samples = VK_SAMPLE_COUNT_1_BIT;
			da.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			da.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			da.stencilLoadOp = hasSt ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			da.stencilStoreOp = hasSt ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			da.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			da.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthRef = { attCount, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			++attCount;
		}

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorCount;
		subpass.pColorAttachments = (colorCount > 0) ? colorRefs : nullptr;
		subpass.pDepthStencilAttachment = hasDepth ? &depthRef : nullptr;

		VkSubpassDependency dep = {};
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo rpci = {};
		rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpci.attachmentCount = attCount;
		rpci.pAttachments = atts;
		rpci.subpassCount = 1;
		rpci.pSubpasses = &subpass;
		rpci.dependencyCount = 1;
		rpci.pDependencies = &dep;

		VkRenderPass rp = VK_NULL_HANDLE;
		if (m_vk.vkCreateRenderPass(m_device, &rpci, nullptr, &rp) != VK_SUCCESS || !rp)
			return VK_NULL_HANDLE;

		m_renderPassCache.emplace(h, rp);
		return rp;
	}

	VkFramebuffer RenderDeviceVulkan::GetOrCreateFramebuffer(
		VkRenderPass renderPass,
		uint32_t attachmentCount,
		const VkImageView* views,
		const uint64_t* viewUniqueIds,
		uint32_t width,
		uint32_t height)
	{
		uint64_t h = 14695981039346656037ULL;
		HashCombineVkU64(h, static_cast<uint64_t>(renderPass));
		HashCombineVkU32(h, width);
		HashCombineVkU32(h, height);
		HashCombineVkU32(h, attachmentCount);
		for (uint32_t i = 0; i < attachmentCount; ++i)
			HashCombineVkU64(h, viewUniqueIds[i]);

		auto it = m_framebufferCache.find(h);
		if (it != m_framebufferCache.end())
			return it->second.framebuffer;

		VkFramebufferCreateInfo fbci = {};
		fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbci.renderPass = renderPass;
		fbci.attachmentCount = attachmentCount;
		fbci.pAttachments = views;
		fbci.width = width;
		fbci.height = height;
		fbci.layers = 1;

		VkFramebuffer fb = VK_NULL_HANDLE;
		if (m_vk.vkCreateFramebuffer(m_device, &fbci, nullptr, &fb) != VK_SUCCESS || !fb)
			return VK_NULL_HANDLE;

		FramebufferEntry entry;
		entry.framebuffer = fb;
		entry.viewCount = attachmentCount;
		for (uint32_t i = 0; i < attachmentCount && i < 5; ++i)
			entry.viewIds[i] = viewUniqueIds[i];
		m_framebufferCache.emplace(h, entry);
		return fb;
	}

	void RenderDeviceVulkan::ApplyDynamicViewportAndScissor(VkCommandBuffer cmd)
	{
		if (!cmd)
			return;
		VkViewport vp = {
			static_cast<float>(m_currentViewport.X),
			static_cast<float>(m_currentViewport.Y),
			(std::max)(1.0f, static_cast<float>(m_currentViewport.Width)),
			(std::max)(1.0f, static_cast<float>(m_currentViewport.Height)),
			m_currentViewport.MinZ,
			m_currentViewport.MaxZ
		};
		m_vk.vkCmdSetViewport(cmd, 0, 1, &vp);

		VkRect2D sc = { { 0, 0 }, { m_renderTargetWidth, m_renderTargetHeight } };
		if (m_stateTracker.GetRenderState(ERenderState::SCISSORTESTENABLE))
		{
			const RECT& sr = m_stateTracker.GetScissorRect();
			int32_t x0 = (std::max)(0, static_cast<int32_t>(sr.left));
			int32_t y0 = (std::max)(0, static_cast<int32_t>(sr.top));
			int32_t x1 = (std::min)(static_cast<int32_t>(m_renderTargetWidth), static_cast<int32_t>(sr.right));
			int32_t y1 = (std::min)(static_cast<int32_t>(m_renderTargetHeight), static_cast<int32_t>(sr.bottom));
			sc.offset = { x0, y0 };
			sc.extent = {
				static_cast<uint32_t>((std::max)(0, x1 - x0)),
				static_cast<uint32_t>((std::max)(0, y1 - y0))
			};
		}
		m_vk.vkCmdSetScissor(cmd, 0, 1, &sc);
	}

	bool RenderDeviceVulkan::EnsureActiveRenderPass()
	{
		VkCommandBuffer cmd = EnsureRecording();
		if (!cmd)
			return false;
		if (m_inRenderPass)
			return true;

		VkImageView attViews[5] = {};
		uint64_t attIds[5] = {};
		uint32_t attCount = 0;

		uint32_t firstTexAttachment = 0;
		if (m_usingBackBufferRT)
		{
			if (!EnsureSwapchainImageAcquired())
				return false;
			if (m_acquiredImageIndex >= m_swapImages.size())
				return false;

			TransitionImageLayout(
				cmd,
				m_swapImages[m_acquiredImageIndex],
				VK_IMAGE_ASPECT_COLOR_BIT,
				1, 1,
				m_swapImageLayouts[m_acquiredImageIndex],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			attViews[attCount] = m_swapImageViews[m_acquiredImageIndex];
			attIds[attCount] = m_swapViewIds[m_acquiredImageIndex];
			++attCount;
			firstTexAttachment = 1;
		}

		for (uint32_t i = firstTexAttachment; i < m_activeColorCount; ++i)
		{
			VulkanTexture* ct = m_activeColorTexObjs[i];
			if (!ct)
				return false;
			TransitionImageLayout(
				cmd,
				ct->GetVkImage(),
				VK_IMAGE_ASPECT_COLOR_BIT,
				ct->GetMipLevels(),
				ct->GetLayerCount(),
				ct->GetLayoutRef(),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			attViews[attCount] = ct->GetAttachmentView();
			attIds[attCount] = ct->GetAttachmentViewId();
			++attCount;
		}

		if (m_hasActiveDepth)
		{
			if (m_usingDefaultDepth)
			{
				TransitionImageLayout(
					cmd,
					m_defaultDepthImage,
					AspectMaskForVkFormat(m_defaultDepthFormat),
					1, 1,
					m_defaultDepthLayout,
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				attViews[attCount] = m_defaultDepthView;
				attIds[attCount] = m_defaultDepthViewId;
				++attCount;
			}
			else if (m_activeDepthTexObj)
			{
				TransitionImageLayout(
					cmd,
					m_activeDepthTexObj->GetVkImage(),
					AspectMaskForVkFormat(m_activeDepthTexObj->GetVkFormat()),
					m_activeDepthTexObj->GetMipLevels(),
					m_activeDepthTexObj->GetLayerCount(),
					m_activeDepthTexObj->GetLayoutRef(),
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				attViews[attCount] = m_activeDepthTexObj->GetAttachmentView();
				attIds[attCount] = m_activeDepthTexObj->GetAttachmentViewId();
				++attCount;
			}
		}

		// Transition any bound shader textures that are NOT attached as active render targets to SHADER_READ_ONLY_OPTIMAL
		for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
		{
			VulkanTexture* bt = m_boundTextures[s];
			if (!bt)
				continue;
			bool conflict = (m_hasActiveDepth && !m_usingDefaultDepth && bt == m_activeDepthTexObj);
			for (uint32_t c = 0; c < m_activeColorCount; ++c)
			{
				if (bt == m_activeColorTexObjs[c])
					conflict = true;
			}
			if (!conflict && bt->GetLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				TransitionImageLayout(
					cmd,
					bt->GetVkImage(),
					AspectMaskForVkFormat(bt->GetVkFormat()),
					bt->GetMipLevels(),
					bt->GetLayerCount(),
					bt->GetLayoutRef(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}

		VkRenderPass rp = GetOrCreateRenderPass(
			m_activeColorCount,
			m_activeColorFormats,
			m_hasActiveDepth,
			m_activeDepthFormat,
			m_activeSampleCount);
		if (!rp)
			return false;

		VkFramebuffer fb = GetOrCreateFramebuffer(
			rp, attCount, attViews, attIds, m_renderTargetWidth, m_renderTargetHeight);
		if (!fb)
			return false;

		VkRenderPassBeginInfo rpbi = {};
		rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpbi.renderPass = rp;
		rpbi.framebuffer = fb;
		rpbi.renderArea = { { 0, 0 }, { m_renderTargetWidth, m_renderTargetHeight } };
		rpbi.clearValueCount = 0;
		rpbi.pClearValues = nullptr;

		m_vk.vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
		m_inRenderPass = true;
		m_activeVkRenderPass = rp;
		ApplyDynamicViewportAndScissor(cmd);
		return true;
	}

	VkDescriptorPool RenderDeviceVulkan::CreateNewDescriptorPoolChunk()
	{
		static constexpr uint32_t SETS_PER_CHUNK = 1024;
		VkDescriptorPoolSize sizes[3] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SETS_PER_CHUNK * 2u },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  SETS_PER_CHUNK * 16u },
			{ VK_DESCRIPTOR_TYPE_SAMPLER,        SETS_PER_CHUNK * 16u }
		};

		VkDescriptorPoolCreateInfo dpci = {};
		dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.maxSets = SETS_PER_CHUNK;
		dpci.poolSizeCount = 3;
		dpci.pPoolSizes = sizes;

		VkDescriptorPool pool = VK_NULL_HANDLE;
		if (m_vk.vkCreateDescriptorPool(m_device, &dpci, nullptr, &pool) != VK_SUCCESS)
			return VK_NULL_HANDLE;
		return pool;
	}

	VkDescriptorSet RenderDeviceVulkan::AllocateFrameDescriptorSet()
	{
		FrameContext& fc = m_frames[m_frameIndex];
		while (fc.activePoolIdx < fc.descriptorPools.size())
		{
			VkDescriptorSetAllocateInfo dsai = {};
			dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			dsai.descriptorPool = fc.descriptorPools[fc.activePoolIdx];
			dsai.descriptorSetCount = 1;
			dsai.pSetLayouts = &m_setLayout;

			VkDescriptorSet set = VK_NULL_HANDLE;
			VkResult res = m_vk.vkAllocateDescriptorSets(m_device, &dsai, &set);
			if (res == VK_SUCCESS && set != VK_NULL_HANDLE)
				return set;

			++fc.activePoolIdx;
		}

		VkDescriptorPool newPool = CreateNewDescriptorPoolChunk();
		if (!newPool)
			return VK_NULL_HANDLE;
		fc.descriptorPools.push_back(newPool);

		VkDescriptorSetAllocateInfo dsai = {};
		dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsai.descriptorPool = newPool;
		dsai.descriptorSetCount = 1;
		dsai.pSetLayouts = &m_setLayout;

		VkDescriptorSet set = VK_NULL_HANDLE;
		if (m_vk.vkAllocateDescriptorSets(m_device, &dsai, &set) != VK_SUCCESS)
			return VK_NULL_HANDLE;
		return set;
	}

	VkSampler RenderDeviceVulkan::GetOrCreateSampler(uint32_t stage)
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
		HashCombineVkU32(h, addrU | (addrV << 8) | (addrW << 16));
		HashCombineVkU32(h, minF | (magF << 8) | (mipF << 16) | (aniso << 24));
		HashCombineVkU32(h, border);
		HashCombineVkU32(h, biasBits);

		auto it = m_samplerCache.find(h);
		if (it != m_samplerCache.end())
			return it->second;

		VkSamplerCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sci.magFilter = (magF == 1) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		sci.minFilter = (minF == 1) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
		sci.mipmapMode = (mipF == 1) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sci.addressModeU = ToVkAddressMode(addrU);
		sci.addressModeV = ToVkAddressMode(addrV);
		sci.addressModeW = ToVkAddressMode(addrW);

		float bias = 0.0f;
		std::memcpy(&bias, &biasBits, sizeof(float));
		if (std::isfinite(bias))
			sci.mipLodBias = bias;

		if (m_enabledFeatures.samplerAnisotropy == VK_TRUE && (minF == 3 || magF == 3) && aniso > 1)
		{
			sci.anisotropyEnable = VK_TRUE;
			sci.maxAnisotropy = (std::min)(static_cast<float>(aniso), m_physProps.limits.maxSamplerAnisotropy);
		}

		sci.minLod = 0.0f;
		sci.maxLod = (mipF == 0) ? 0.0f : VK_LOD_CLAMP_NONE;

		uint32_t a = (border >> 24) & 0xFFu;
		uint32_t r = (border >> 16) & 0xFFu;
		uint32_t g = (border >> 8) & 0xFFu;
		uint32_t b = border & 0xFFu;
		if (a > 127 && r > 127 && g > 127 && b > 127)
			sci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		else if (a > 127)
			sci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		else
			sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

		VkSampler smp = VK_NULL_HANDLE;
		if (m_vk.vkCreateSampler(m_device, &sci, nullptr, &smp) != VK_SUCCESS || !smp)
			return VK_NULL_HANDLE;

		m_samplerCache.emplace(h, smp);
		return smp;
	}

	VkPipeline RenderDeviceVulkan::GetOrCreateGraphicsPipeline(VkPrimitiveTopology vkTopo, VkRenderPass renderPass)
	{
		if (!m_currentProgram || !m_currentProgram->IsValid() || !renderPass)
			return VK_NULL_HANDLE;

		const uint32_t alphaBlend = m_stateTracker.GetRenderState(ERenderState::ALPHABLENDENABLE) ? 1u : 0u;
		const uint32_t sepAlpha   = m_stateTracker.GetRenderState(ERenderState::SEPARATEALPHABLENDENABLE) ? 1u : 0u;
		const uint32_t srcBlend   = m_stateTracker.GetRenderState(ERenderState::SRCBLEND);
		const uint32_t dstBlend   = m_stateTracker.GetRenderState(ERenderState::DESTBLEND);
		const uint32_t blendOp    = m_stateTracker.GetRenderState(ERenderState::BLENDOP);
		const uint32_t srcBlendA  = sepAlpha ? m_stateTracker.GetRenderState(ERenderState::SRCBLENDALPHA) : srcBlend;
		const uint32_t dstBlendA  = sepAlpha ? m_stateTracker.GetRenderState(ERenderState::DESTBLENDALPHA) : dstBlend;
		const uint32_t blendOpA   = sepAlpha ? m_stateTracker.GetRenderState(ERenderState::BLENDOPALPHA) : blendOp;

		const uint32_t cw[4] = {
			m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE) & 0x0Fu,
			m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE1) & 0x0Fu,
			m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE2) & 0x0Fu,
			m_stateTracker.GetRenderState(ERenderState::COLORWRITEENABLE3) & 0x0Fu
		};

		const uint32_t cullMode  = m_stateTracker.GetRenderState(ERenderState::CULLMODE);
		const uint32_t fillMode  = m_stateTracker.GetRenderState(ERenderState::FILLMODE);
		const uint32_t biasBits  = m_stateTracker.GetRenderState(ERenderState::DEPTHBIAS);
		const uint32_t slopeBits = m_stateTracker.GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS);

		const uint32_t zEnable     = m_hasActiveDepth ? (m_stateTracker.GetRenderState(ERenderState::ZENABLE) ? 1u : 0u) : 0u;
		const uint32_t zWrite      = m_hasActiveDepth ? (m_stateTracker.GetRenderState(ERenderState::ZWRITEENABLE) ? 1u : 0u) : 0u;
		const uint32_t zFunc       = m_stateTracker.GetRenderState(ERenderState::ZFUNC);
		const bool depthHasStencil = m_hasActiveDepth && HasStencilInVkDepthFormat(m_activeDepthFormat);
		const uint32_t stEnable    = depthHasStencil ? (m_stateTracker.GetRenderState(ERenderState::STENCILENABLE) ? 1u : 0u) : 0u;
		const uint32_t stMask      = m_stateTracker.GetRenderState(ERenderState::STENCILMASK) & 0xFFu;
		const uint32_t stWriteMask = m_stateTracker.GetRenderState(ERenderState::STENCILWRITEMASK) & 0xFFu;
		const uint32_t stFunc      = m_stateTracker.GetRenderState(ERenderState::STENCILFUNC);
		const uint32_t stFail      = m_stateTracker.GetRenderState(ERenderState::STENCILFAIL);
		const uint32_t stZFail     = m_stateTracker.GetRenderState(ERenderState::STENCILZFAIL);
		const uint32_t stPass      = m_stateTracker.GetRenderState(ERenderState::STENCILPASS);
		const uint32_t twoSided    = m_stateTracker.GetRenderState(ERenderState::TWOSIDEDSTENCILMODE) ? 1u : 0u;
		const uint32_t ccwFunc     = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILFUNC);
		const uint32_t ccwFail     = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILFAIL);
		const uint32_t ccwZFail    = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILZFAIL);
		const uint32_t ccwPass     = m_stateTracker.GetRenderState(ERenderState::CCW_STENCILPASS);

		uint64_t h = 14695981039346656037ULL;
		HashCombineVkU64(h, m_currentProgram->GetCombinedHash());
		HashCombineVkU64(h, m_currentVertexDecl ? m_currentVertexDecl->GetHash() : 0ULL);
		HashCombineVkU64(h, static_cast<uint64_t>(renderPass));
		for (uint32_t s = 0; s < 4; ++s)
		{
			HashCombineVkU32(h, m_streams[s].stride);
			HashCombineVkU32(h, m_streams[s].frequencyDivider);
		}
		HashCombineVkU32(h, static_cast<uint32_t>(vkTopo));
		HashCombineVkU32(h, alphaBlend | (sepAlpha << 8));
		HashCombineVkU32(h, srcBlend | (dstBlend << 8) | (blendOp << 16));
		HashCombineVkU32(h, srcBlendA | (dstBlendA << 8) | (blendOpA << 16));
		HashCombineVkU32(h, cw[0] | (cw[1] << 4) | (cw[2] << 8) | (cw[3] << 12));
		HashCombineVkU32(h, cullMode | (fillMode << 8));
		HashCombineVkU32(h, biasBits);
		HashCombineVkU32(h, slopeBits);
		HashCombineVkU32(h, zEnable | (zWrite << 2) | (zFunc << 4) | (stEnable << 12) | (twoSided << 14));
		HashCombineVkU32(h, stMask | (stWriteMask << 8));
		HashCombineVkU32(h, stFunc | (stFail << 8) | (stZFail << 16) | (stPass << 24));
		HashCombineVkU32(h, ccwFunc | (ccwFail << 8) | (ccwZFail << 16) | (ccwPass << 24));

		auto it = m_pipelineCache.find(h);
		if (it != m_pipelineCache.end())
			return it->second;

		// Build Vertex Input bindings & attributes
		std::vector<VkVertexInputBindingDescription> vBindings;
		std::vector<VkVertexInputAttributeDescription> vAttrs;
		uint32_t providedLocMask = 0;
		bool streamUsed[4] = {};

		if (m_currentVertexDecl)
		{
			for (const auto& ve : m_currentVertexDecl->GetElements())
			{
				if (ve.IsEndDeclare() || ve.Stream >= 4)
					continue;
				int32_t loc = MapDeclUsageToLocationVk(ve.Usage, ve.UsageIndex);
				if (loc < 0 || loc >= 16)
					continue;
				if (providedLocMask & (1u << loc))
					continue;

				providedLocMask |= (1u << loc);
				streamUsed[ve.Stream] = true;

				VkVertexInputAttributeDescription attr = {};
				attr.location = static_cast<uint32_t>(loc);
				attr.binding = ve.Stream;
				attr.format = MapDeclTypeToVkFormat(ve.Type, m_supportsUbyte4Uscaled);
				attr.offset = ve.Offset;
				vAttrs.push_back(attr);
			}
		}

		for (uint32_t s = 0; s < 4; ++s)
		{
			if (!streamUsed[s])
				continue;
			VkVertexInputBindingDescription bd = {};
			bd.binding = s;
			bd.stride = m_streams[s].stride;
			uint32_t freq = m_streams[s].frequencyDivider;
			bd.inputRate = (s > 0 && freq != 0) ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
			vBindings.push_back(bd);
		}

		// Any VS input location required by SPIR-V but absent from CVertexDeclaration gets binding 15 (stride=0, [0,0,0,1])
		uint32_t missingMask = m_currentProgram->GetVsInputLocationMask() & ~providedLocMask;
		if (missingMask != 0)
		{
			VkVertexInputBindingDescription fallbackBd = {};
			fallbackBd.binding = 15;
			fallbackBd.stride = 0;
			fallbackBd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vBindings.push_back(fallbackBd);

			for (uint32_t loc = 0; loc < 16; ++loc)
			{
				if (missingMask & (1u << loc))
				{
					VkVertexInputAttributeDescription fa = {};
					fa.location = loc;
					fa.binding = 15;
					fa.format = VK_FORMAT_R32G32B32A32_SFLOAT;
					fa.offset = 0;
					vAttrs.push_back(fa);
				}
			}
		}

		VkPipelineShaderStageCreateInfo stages[2] = {};
		uint32_t stageCount = 0;
		stages[stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[stageCount].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[stageCount].module = m_currentProgram->GetVsModule();
		stages[stageCount].pName = m_currentProgram->GetVsEntry();
		++stageCount;

		if (m_currentProgram->GetPsModule() != VK_NULL_HANDLE)
		{
			stages[stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stages[stageCount].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			stages[stageCount].module = m_currentProgram->GetPsModule();
			stages[stageCount].pName = m_currentProgram->GetPsEntry();
			++stageCount;
		}

		VkPipelineVertexInputStateCreateInfo viState = {};
		viState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		viState.vertexBindingDescriptionCount = static_cast<uint32_t>(vBindings.size());
		viState.pVertexBindingDescriptions = vBindings.empty() ? nullptr : vBindings.data();
		viState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vAttrs.size());
		viState.pVertexAttributeDescriptions = vAttrs.empty() ? nullptr : vAttrs.data();

		VkPipelineInputAssemblyStateCreateInfo iaState = {};
		iaState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		iaState.topology = vkTopo;
		iaState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo vpState = {};
		vpState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vpState.viewportCount = 1;
		vpState.scissorCount = 1;

		// Algebraic Winding Rule: SpirvCodeGen flips clip-space Y (pos.y = -pos.y), which matches D3D screen coordinates
		// identically in Vulkan framebuffer space; therefore D3D clockwise front-facing maps to VK_FRONT_FACE_CLOCKWISE!
		VkPipelineRasterizationStateCreateInfo rsState = {};
		rsState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rsState.depthClampEnable = VK_FALSE;
		rsState.rasterizerDiscardEnable = VK_FALSE;
		rsState.polygonMode = (fillMode == 2 && m_enabledFeatures.fillModeNonSolid) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rsState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		if (cullMode == RSV_CULL_NONE)
			rsState.cullMode = VK_CULL_MODE_NONE;
		else if (cullMode == RSV_CULL_CW)
			rsState.cullMode = VK_CULL_MODE_FRONT_BIT;
		else
			rsState.cullMode = VK_CULL_MODE_BACK_BIT; // RSV_CULL_CCW (default D3D backface cull)

		float depthBias = 0.0f, slopeBias = 0.0f;
		std::memcpy(&depthBias, &biasBits, sizeof(float));
		std::memcpy(&slopeBias, &slopeBits, sizeof(float));
		if ((depthBias != 0.0f || slopeBias != 0.0f) && std::isfinite(depthBias) && std::isfinite(slopeBias))
		{
			rsState.depthBiasEnable = VK_TRUE;
			rsState.depthBiasConstantFactor = depthBias * static_cast<float>(1 << 24);
			rsState.depthBiasSlopeFactor = slopeBias;
		}
		rsState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo msState = {};
		msState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		msState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo dsState = {};
		dsState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		dsState.depthTestEnable = zEnable ? VK_TRUE : VK_FALSE;
		dsState.depthWriteEnable = zWrite ? VK_TRUE : VK_FALSE;
		dsState.depthCompareOp = ToVkCompareOp(zFunc);
		dsState.stencilTestEnable = stEnable ? VK_TRUE : VK_FALSE;
		dsState.front.failOp = ToVkStencilOp(stFail);
		dsState.front.depthFailOp = ToVkStencilOp(stZFail);
		dsState.front.passOp = ToVkStencilOp(stPass);
		dsState.front.compareOp = ToVkCompareOp(stFunc);
		dsState.front.compareMask = stMask;
		dsState.front.writeMask = stWriteMask;
		dsState.front.reference = 0;
		if (twoSided)
		{
			dsState.back.failOp = ToVkStencilOp(ccwFail);
			dsState.back.depthFailOp = ToVkStencilOp(ccwZFail);
			dsState.back.passOp = ToVkStencilOp(ccwPass);
			dsState.back.compareOp = ToVkCompareOp(ccwFunc);
			dsState.back.compareMask = stMask;
			dsState.back.writeMask = stWriteMask;
			dsState.back.reference = 0;
		}
		else
		{
			dsState.back = dsState.front;
		}

		VkPipelineColorBlendAttachmentState cbAtts[4] = {};
		const uint32_t psOutMask = (m_currentProgram && m_currentProgram->GetPsModule() != VK_NULL_HANDLE)
			? m_currentProgram->GetPsOutputLocationMask()
			: 0u;
		for (uint32_t i = 0; i < m_activeColorCount; ++i)
		{
			uint32_t maskIdx = (m_enabledFeatures.independentBlend == VK_TRUE) ? i : 0u;
			uint32_t d3dMask = cw[maskIdx];
			VkFlags vkMask = 0;
			if ((psOutMask & (1u << i)) != 0u)
			{
				if (d3dMask & 0x1u) vkMask |= VK_COLOR_COMPONENT_R_BIT;
				if (d3dMask & 0x2u) vkMask |= VK_COLOR_COMPONENT_G_BIT;
				if (d3dMask & 0x4u) vkMask |= VK_COLOR_COMPONENT_B_BIT;
				if (d3dMask & 0x8u) vkMask |= VK_COLOR_COMPONENT_A_BIT;
			}

			cbAtts[i].blendEnable = (alphaBlend && vkMask != 0) ? VK_TRUE : VK_FALSE;
			cbAtts[i].srcColorBlendFactor = ToVkBlendFactor(srcBlend, false);
			cbAtts[i].dstColorBlendFactor = ToVkBlendFactor(dstBlend, false);
			cbAtts[i].colorBlendOp = ToVkBlendOp(blendOp);
			cbAtts[i].srcAlphaBlendFactor = ToVkBlendFactor(srcBlendA, true);
			cbAtts[i].dstAlphaBlendFactor = ToVkBlendFactor(dstBlendA, true);
			cbAtts[i].alphaBlendOp = ToVkBlendOp(blendOpA);
			cbAtts[i].colorWriteMask = vkMask;
		}

		VkPipelineColorBlendStateCreateInfo cbState = {};
		cbState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cbState.attachmentCount = m_activeColorCount;
		cbState.pAttachments = (m_activeColorCount > 0) ? cbAtts : nullptr;

		const VkDynamicState dynStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_BLEND_CONSTANTS,
			VK_DYNAMIC_STATE_STENCIL_REFERENCE
		};
		VkPipelineDynamicStateCreateInfo dynInfo = {};
		dynInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynInfo.dynamicStateCount = 4;
		dynInfo.pDynamicStates = dynStates;

		VkGraphicsPipelineCreateInfo gpci = {};
		gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		gpci.stageCount = stageCount;
		gpci.pStages = stages;
		gpci.pVertexInputState = &viState;
		gpci.pInputAssemblyState = &iaState;
		gpci.pViewportState = &vpState;
		gpci.pRasterizationState = &rsState;
		gpci.pMultisampleState = &msState;
		gpci.pDepthStencilState = &dsState;
		gpci.pColorBlendState = &cbState;
		gpci.pDynamicState = &dynInfo;
		gpci.layout = m_pipelineLayout;
		gpci.renderPass = renderPass;
		gpci.subpass = 0;

		VkPipeline pipeline = VK_NULL_HANDLE;
		if (m_vk.vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gpci, nullptr, &pipeline) != VK_SUCCESS || !pipeline)
			return VK_NULL_HANDLE;

		m_pipelineCache.emplace(h, pipeline);
		return pipeline;
	}

	bool RenderDeviceVulkan::PrepareForDraw(VkPrimitiveTopology vkTopo)
	{
		if (!m_currentProgram || !m_currentProgram->IsValid())
			return false;

		// Ensure all bound non-conflicting textures are in SHADER_READ_ONLY_OPTIMAL BEFORE starting render pass
		VkCommandBuffer cmd = EnsureRecording();
		if (!cmd)
			return false;

		for (uint32_t s = 0; s < RHIStateTracker::MAX_SAMPLER_STAGES; ++s)
		{
			VulkanTexture* bt = m_boundTextures[s];
			if (!bt)
				continue;
			bool conflict = (m_hasActiveDepth && !m_usingDefaultDepth && bt == m_activeDepthTexObj);
			for (uint32_t c = 0; c < m_activeColorCount; ++c)
			{
				if (bt == m_activeColorTexObjs[c])
					conflict = true;
			}
			if (!conflict && bt->GetLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				EndActiveRenderPass();
				TransitionImageLayout(
					cmd,
					bt->GetVkImage(),
					AspectMaskForVkFormat(bt->GetVkFormat()),
					bt->GetMipLevels(),
					bt->GetLayerCount(),
					bt->GetLayoutRef(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}

		if (!EnsureActiveRenderPass())
			return false;

		VkPipeline pipeline = GetOrCreateGraphicsPipeline(vkTopo, m_activeVkRenderPass);
		if (!pipeline)
			return false;

		m_vk.vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		ApplyDynamicViewportAndScissor(cmd);

		// Dynamic blend factor & stencil reference
		uint32_t bf = m_stateTracker.GetRenderState(ERenderState::BLENDFACTOR);
		float blendConsts[4] = {
			((bf >> 16) & 0xFFu) / 255.0f,
			((bf >> 8) & 0xFFu) / 255.0f,
			(bf & 0xFFu) / 255.0f,
			((bf >> 24) & 0xFFu) / 255.0f
		};
		m_vk.vkCmdSetBlendConstants(cmd, blendConsts);

		uint32_t stRef = m_stateTracker.GetRenderState(ERenderState::STENCILREF) & 0xFFu;
		m_vk.vkCmdSetStencilReference(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, stRef);

		if (m_descriptorsDirty || !m_lastDescriptorSet || m_stateTracker.GetDirtySamplerMask() != 0)
		{
			VkDescriptorSet ds = AllocateFrameDescriptorSet();
			if (!ds)
				return false;

			VkDescriptorBufferInfo bufInfos[2] = {
				{ m_currentVsUboBuf ? m_currentVsUboBuf : m_zeroUboBuffer, m_currentVsUboOffset, m_currentVsUboRange },
				{ m_currentPsUboBuf ? m_currentPsUboBuf : m_zeroUboBuffer, m_currentPsUboOffset, m_currentPsUboRange }
			};

			VkDescriptorImageInfo imgInfos[16] = {};
			VkDescriptorImageInfo smpInfos[16] = {};
			VkWriteDescriptorSet writes[34] = {};
			uint32_t writeCount = 0;

			writes[writeCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[writeCount].dstSet = ds;
			writes[writeCount].dstBinding = 0;
			writes[writeCount].descriptorCount = 1;
			writes[writeCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writes[writeCount].pBufferInfo = &bufInfos[0];
			++writeCount;

			writes[writeCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[writeCount].dstSet = ds;
			writes[writeCount].dstBinding = 1;
			writes[writeCount].descriptorCount = 1;
			writes[writeCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writes[writeCount].pBufferInfo = &bufInfos[1];
			++writeCount;

			for (uint32_t s = 0; s < 16; ++s)
			{
				uint8_t expectedDim = m_currentProgram->GetExpectedSamplerDim(s);
				VulkanTexture* bt = m_boundTextures[s];
				bool conflict = (bt != nullptr) && (m_hasActiveDepth && !m_usingDefaultDepth && bt == m_activeDepthTexObj);
				if (bt)
				{
					for (uint32_t c = 0; c < m_activeColorCount; ++c)
					{
						if (bt == m_activeColorTexObjs[c])
							conflict = true;
					}
				}

				VkImageView chosenView = m_whiteFallback2D->GetSrvView();
				if (expectedDim == 3) // Cube
				{
					if (bt && !conflict && bt->IsCubeMap() && bt->GetSrvView())
						chosenView = bt->GetSrvView();
					else if (m_whiteCubeView)
						chosenView = m_whiteCubeView;
				}
				else if (expectedDim == 2) // 3D
				{
					if (m_white3DView)
						chosenView = m_white3DView;
				}
				else
				{
					if (bt && !conflict && !bt->IsCubeMap() && bt->GetSrvView())
						chosenView = bt->GetSrvView();
				}

				imgInfos[s].sampler = VK_NULL_HANDLE;
				imgInfos[s].imageView = chosenView;
				imgInfos[s].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				writes[writeCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writes[writeCount].dstSet = ds;
				writes[writeCount].dstBinding = 16u + s;
				writes[writeCount].descriptorCount = 1;
				writes[writeCount].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				writes[writeCount].pImageInfo = &imgInfos[s];
				++writeCount;

				VkSampler smp = GetOrCreateSampler(s);
				smpInfos[s].sampler = smp;
				smpInfos[s].imageView = VK_NULL_HANDLE;
				smpInfos[s].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				writes[writeCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writes[writeCount].dstSet = ds;
				writes[writeCount].dstBinding = 32u + s;
				writes[writeCount].descriptorCount = 1;
				writes[writeCount].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				writes[writeCount].pImageInfo = &smpInfos[s];
				++writeCount;

				m_stateTracker.ClearSamplerDirty(s);
			}

			m_vk.vkUpdateDescriptorSets(m_device, writeCount, writes, 0, nullptr);
			m_lastDescriptorSet = ds;
			m_descriptorsDirty = false;
		}

		m_vk.vkCmdBindDescriptorSets(
			cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_lastDescriptorSet, 0, nullptr);

		// Bind default [0,0,0,1] vertex buffer at slot 15 and any active streams 0..3
		VkDeviceSize zeroOffset = 0;
		m_vk.vkCmdBindVertexBuffers(cmd, 15, 1, &m_defaultAttrVB, &zeroOffset);

		for (uint32_t s = 0; s < 4; ++s)
		{
			if (m_streams[s].buffer && m_streams[s].buffer->GetActiveVkBuffer())
			{
				VkBuffer vb = m_streams[s].buffer->GetActiveVkBuffer();
				VkDeviceSize off = m_streams[s].buffer->GetActiveOffset() + static_cast<VkDeviceSize>(m_streams[s].offsetInBytes);
				m_vk.vkCmdBindVertexBuffers(cmd, s, 1, &vb, &off);
			}
			else
			{
				m_vk.vkCmdBindVertexBuffers(cmd, s, 1, &m_defaultAttrVB, &zeroOffset);
			}
		}

		return true;
	}

	// =========================================================================
	// Resource Creation, Render Targets, Blit & Readback
	// =========================================================================

	uint32_t RenderDeviceVulkan::AllocateOcclusionQuerySlot()
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

	void RenderDeviceVulkan::FreeOcclusionQuerySlot(uint32_t slot)
	{
		if (slot < MAX_OCCLUSION_QUERIES)
			m_freeOcclusionSlots.push_back(slot);
	}

	bool RenderDeviceVulkan::CheckDeviceError(const char* /*filename*/, const char* /*func*/, int /*nLine*/)
	{
		return true;
	}

	IRHITexture* RenderDeviceVulkan::CreateTexture(
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

		const bool isCube = HasFlag(usage, ERHITextureUsage::CubeMap);
		const uint32_t layerCount = isCube ? 6u : 1u;

		uint32_t maxMips = 1;
		{
			uint32_t dim = (std::max)(width, height);
			while (dim > 1) { dim >>= 1; ++maxMips; }
		}
		uint32_t numMips = (mipLevels == 0) ? maxMips : (std::min)(mipLevels, maxMips);
		if (initialMips && initialMipCount > 0 && numMips > initialMipCount)
			numMips = initialMipCount;

		if (HasFlag(usage, ERHITextureUsage::DepthStencil) || IsDepthPixelFormatVk(format))
		{
			VkFormat dFmt = VK_FORMAT_D24_UNORM_S8_UINT;
			if (format == PixelFormat::D16 || format == PixelFormat::D16_LOCKABLE)
				dFmt = VK_FORMAT_D16_UNORM;
			else if (format == PixelFormat::D32 || format == PixelFormat::D32F_LOCKABLE)
				dFmt = VK_FORMAT_D32_SFLOAT;

			VkFormatProperties fp = {};
			m_vk.vkGetPhysicalDeviceFormatProperties(m_physicalDevice, dFmt, &fp);
			if ((fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
				dFmt = m_defaultDepthFormat;

			VkImageCreateInfo ii = {};
			ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			ii.imageType = VK_IMAGE_TYPE_2D;
			ii.format = dFmt;
			ii.extent = { width, height, 1 };
			ii.mipLevels = 1;
			ii.arrayLayers = 1;
			ii.samples = VK_SAMPLE_COUNT_1_BIT;
			ii.tiling = VK_IMAGE_TILING_OPTIMAL;
			ii.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkImage img = VK_NULL_HANDLE;
			VkDeviceMemory mem = VK_NULL_HANDLE;
			if (!CreateVkImageWithMemory(ii, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img, mem))
				return nullptr;

			VkImageViewCreateInfo srvVi = {};
			srvVi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			srvVi.image = img;
			srvVi.viewType = VK_IMAGE_VIEW_TYPE_2D;
			srvVi.format = dFmt;
			srvVi.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

			VkImageView srvView = VK_NULL_HANDLE;
			m_vk.vkCreateImageView(m_device, &srvVi, nullptr, &srvView);

			VkFlags attAspect = AspectMaskForVkFormat(dFmt);
			VkImageView attView = srvView;
			if (attAspect != VK_IMAGE_ASPECT_DEPTH_BIT)
			{
				VkImageViewCreateInfo attVi = srvVi;
				attVi.subresourceRange.aspectMask = attAspect;
				m_vk.vkCreateImageView(m_device, &attVi, nullptr, &attView);
			}

			return new VulkanTexture(
				this, width, height, format, dFmt, 1, 1, false, false, false,
				img, mem, srvView, attView, NextUniqueViewId(), VK_IMAGE_LAYOUT_UNDEFINED);
		}

		bool expandedToRGBA8 = RHISoftwareS3TC::NeedsRGBA8Expansion(format, true);
		uint32_t bytesPerBlock = 4;
		bool isBlockCompressed = false;
		bool swizzleAlphaFromR = false;
		VkFormat vkFmt = expandedToRGBA8
			? VK_FORMAT_R8G8B8A8_UNORM
			: MapColorPixelFormatToVk(format, bytesPerBlock, isBlockCompressed, swizzleAlphaFromR);

		bool softwareDecodeS3TC = false;
		if (vkFmt == VK_FORMAT_UNDEFINED)
		{
			if (RHISoftwareS3TC::IsS3TCFormat(format))
				softwareDecodeS3TC = true;
			vkFmt = VK_FORMAT_R8G8B8A8_UNORM;
			isBlockCompressed = false;
			bytesPerBlock = 4;
		}
		else if (isBlockCompressed && (!m_caps.supportsBCCompression || (width % 4u) != 0 || (height % 4u) != 0))
		{
			softwareDecodeS3TC = true;
			vkFmt = VK_FORMAT_R8G8B8A8_UNORM;
			isBlockCompressed = false;
			bytesPerBlock = 4;
		}

		VkImageCreateInfo ii = {};
		ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ii.flags = isCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT_LOCAL : 0u;
		ii.imageType = VK_IMAGE_TYPE_2D;
		ii.format = vkFmt;
		ii.extent = { width, height, 1 };
		ii.mipLevels = numMips;
		ii.arrayLayers = layerCount;
		ii.samples = VK_SAMPLE_COUNT_1_BIT;
		ii.tiling = VK_IMAGE_TILING_OPTIMAL;
		ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (HasFlag(usage, ERHITextureUsage::RenderTarget))
			ii.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage img = VK_NULL_HANDLE;
		VkDeviceMemory mem = VK_NULL_HANDLE;
		if (!CreateVkImageWithMemory(ii, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img, mem))
			return nullptr;

		VkImageViewCreateInfo vi = {};
		vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vi.image = img;
		vi.viewType = isCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
		vi.format = vkFmt;
		if (swizzleAlphaFromR)
		{
			vi.components = {
				VK_COMPONENT_SWIZZLE_ZERO,
				VK_COMPONENT_SWIZZLE_ZERO,
				VK_COMPONENT_SWIZZLE_ZERO,
				VK_COMPONENT_SWIZZLE_R
			};
		}
		vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, numMips, 0, layerCount };

		VkImageView srvView = VK_NULL_HANDLE;
		if (m_vk.vkCreateImageView(m_device, &vi, nullptr, &srvView) != VK_SUCCESS || !srvView)
		{
			m_vk.vkDestroyImage(m_device, img, nullptr);
			m_vk.vkFreeMemory(m_device, mem, nullptr);
			return nullptr;
		}

		auto* texObj = new VulkanTexture(
			this, width, height, format, vkFmt, numMips, layerCount, isCube,
			expandedToRGBA8 || softwareDecodeS3TC, swizzleAlphaFromR,
			img, mem, srvView, srvView, NextUniqueViewId(), VK_IMAGE_LAYOUT_UNDEFINED);

		VkCommandBuffer cmd = EnsureRecording();
		if (cmd)
		{
			EndActiveRenderPass();
			if (initialMips && initialMipCount > 0)
			{
				TransitionImageLayout(cmd, img, VK_IMAGE_ASPECT_COLOR_BIT, numMips, layerCount, texObj->GetLayoutRef(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

				for (uint32_t layer = 0; layer < layerCount; ++layer)
				{
					uint32_t mipW = width;
					uint32_t mipH = height;
					for (uint32_t m = 0; m < numMips; ++m)
					{
						uint32_t mipIdx = layer * numMips + m;
						if (mipIdx >= initialMipCount || !initialMips[mipIdx].data)
							break;

						const RHIMipData& srcMip = initialMips[mipIdx];
						std::vector<uint8_t> decoded;
						const uint8_t* srcBytes = static_cast<const uint8_t*>(srcMip.data);
						uint32_t srcRowBytes = 0;
						uint32_t numRows = mipH;
						uint32_t srcStride = srcMip.rowPitch;

						if (softwareDecodeS3TC)
						{
							size_t compBytes = srcMip.dataSize > 0
								? srcMip.dataSize
								: RHISoftwareS3TC::ComputeCompressedMipSize(format, mipW, mipH);
							if (!RHISoftwareS3TC::DecodeToRGBA8(format, srcMip.data, compBytes, mipW, mipH, decoded))
								break;
							srcBytes = decoded.data();
							srcRowBytes = mipW * 4u;
							srcStride = srcRowBytes;
						}
						else if (expandedToRGBA8)
						{
							if (!RHISoftwareS3TC::ExpandToRGBA8(format, srcMip.data, mipW, mipH, srcMip.rowPitch, decoded))
								break;
							srcBytes = decoded.data();
							srcRowBytes = mipW * 4u;
							srcStride = srcRowBytes;
						}
						else if (isBlockCompressed)
						{
							uint32_t blocksX = (std::max)(1u, (mipW + 3u) / 4u);
							numRows = (std::max)(1u, (mipH + 3u) / 4u);
							srcRowBytes = blocksX * bytesPerBlock;
							srcStride = srcMip.rowPitch ? srcMip.rowPitch : srcRowBytes;
						}
						else
						{
							srcRowBytes = mipW * bytesPerBlock;
							srcStride = srcMip.rowPitch ? srcMip.rowPitch : srcRowBytes;
						}

						uint32_t totalBytes = numRows * srcRowBytes;
						void* upCpu = nullptr;
						VkBuffer upBuf = VK_NULL_HANDLE;
						VkDeviceSize upOff = 0;
						if (AllocateUpload(totalBytes, 16, upCpu, upBuf, upOff))
						{
							uint8_t* dstPtr = static_cast<uint8_t*>(upCpu);
							if (srcStride == srcRowBytes)
							{
								std::memcpy(dstPtr, srcBytes, totalBytes);
							}
							else
							{
								for (uint32_t r = 0; r < numRows; ++r)
									std::memcpy(dstPtr + static_cast<size_t>(r) * srcRowBytes, srcBytes + static_cast<size_t>(r) * srcStride, srcRowBytes);
							}

							VkBufferImageCopy bic = {};
							bic.bufferOffset = upOff;
							bic.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, m, layer, 1 };
							bic.imageExtent = { mipW, mipH, 1 };
							m_vk.vkCmdCopyBufferToImage(cmd, upBuf, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);
						}

						mipW = (std::max)(1u, mipW >> 1);
						mipH = (std::max)(1u, mipH >> 1);
					}
				}
			}

			TransitionImageLayout(cmd, img, VK_IMAGE_ASPECT_COLOR_BIT, numMips, layerCount, texObj->GetLayoutRef(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		return texObj;
	}

	IRHIBuffer* RenderDeviceVulkan::CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage, const void* initialData)
	{
		if (!m_device || byteSize == 0)
			return nullptr;
		auto* buf = new VulkanBuffer(this, byteSize, isIndexBuffer, usage, initialData);
		if (!buf->GetActiveVkBuffer())
		{
			buf->ReleaseRHIResource();
			return nullptr;
		}
		return buf;
	}

	IRHIRenderTarget* RenderDeviceVulkan::CreateRenderTarget(
		uint32_t width,
		uint32_t height,
		PixelFormat colorFormat,
		PixelFormat depthFormat,
		uint32_t colorTargetCount,
		uint32_t msaaSamples)
	{
		if (!m_device || width == 0 || height == 0)
			return nullptr;
		auto* rt = new VulkanRenderTarget(this, width, height, colorFormat, depthFormat, colorTargetCount, msaaSamples);
		if (!rt->IsValid())
		{
			rt->ReleaseRHIResource();
			return nullptr;
		}
		return rt;
	}

	IRHIOcclusionQuery* RenderDeviceVulkan::CreateOcclusionQuery()
	{
		if (!m_device || !m_occlusionPool)
			return nullptr;
		return new VulkanOcclusionQuery(this);
	}

	bool RenderDeviceVulkan::SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride)
	{
		const bool prevUsingBackBuffer = m_usingBackBufferRT;
		VulkanTexture* const prevColor0 = m_activeColorTexObjs[0];

		EndActiveRenderPass();

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

		for (uint32_t i = 0; i < 4; ++i)
		{
			m_activeColorTexObjs[i] = nullptr;
			m_activeColorFormats[i] = VK_FORMAT_UNDEFINED;
		}

		if (effectiveCount == 0)
		{
			m_usingBackBufferRT = true;
			m_activeColorFormats[0] = m_swapFormat;
			m_activeColorCount = 1;
			m_activeSampleCount = 1;
			m_renderTargetWidth = m_backBufferWidth;
			m_renderTargetHeight = m_backBufferHeight;

			if (depthStencilOverride)
			{
				auto* dsRT = static_cast<VulkanRenderTarget*>(depthStencilOverride);
				if (dsRT && dsRT->HasDepth())
				{
					m_hasActiveDepth = true;
					m_usingDefaultDepth = false;
					m_activeDepthTexObj = dsRT->GetVkDepthTex();
					m_activeDepthFormat = dsRT->GetVkDepthFormat();
				}
				else
				{
					m_hasActiveDepth = false;
					m_usingDefaultDepth = false;
					m_activeDepthTexObj = nullptr;
				}
			}
			else
			{
				m_hasActiveDepth = true;
				m_usingDefaultDepth = true;
				m_activeDepthTexObj = nullptr;
				m_activeDepthFormat = m_defaultDepthFormat;
			}
		}
		else
		{
			auto* rt0 = static_cast<VulkanRenderTarget*>(targets[0]);
			uint32_t rtvCount = 0;

			if (!rt0)
			{
				// Swapchain backbuffer at slot 0 + offscreen MRT targets at slots 1..effectiveCount-1
				m_usingBackBufferRT = true;
				m_renderTargetWidth = m_backBufferWidth;
				m_renderTargetHeight = m_backBufferHeight;
				m_activeSampleCount = 1;
				m_activeColorTexObjs[0] = nullptr;
				m_activeColorFormats[0] = m_swapFormat;
				rtvCount = effectiveCount;
				for (uint32_t i = 1; i < rtvCount; ++i)
				{
					auto* rti = static_cast<VulkanRenderTarget*>(targets[i]);
					if (rti && rti->GetVkColorTex(0))
					{
						m_activeColorTexObjs[i] = rti->GetVkColorTex(0);
						m_activeColorFormats[i] = rti->GetVkColorFormat();
					}
					else
					{
						m_activeColorTexObjs[i] = nullptr;
						m_activeColorFormats[i] = VK_FORMAT_R8G8B8A8_UNORM;
					}
				}
			}
			else
			{
				m_usingBackBufferRT = false;
				m_renderTargetWidth = rt0->GetWidth();
				m_renderTargetHeight = rt0->GetHeight();
				m_activeSampleCount = rt0->GetSampleCount();

				if (effectiveCount == 1 && rt0->GetColorCount() > 1)
				{
					rtvCount = (std::min)(rt0->GetColorCount(), 4u);
					for (uint32_t i = 0; i < rtvCount; ++i)
					{
						m_activeColorTexObjs[i] = rt0->GetVkColorTex(i);
						m_activeColorFormats[i] = rt0->GetVkColorFormat();
					}
				}
				else
				{
					rtvCount = effectiveCount;
					for (uint32_t i = 0; i < rtvCount; ++i)
					{
						auto* rti = static_cast<VulkanRenderTarget*>(targets[i]);
						if (rti && rti->GetVkColorTex(0))
						{
							m_activeColorTexObjs[i] = rti->GetVkColorTex(0);
							m_activeColorFormats[i] = rti->GetVkColorFormat();
						}
						else
						{
							m_activeColorTexObjs[i] = nullptr;
							m_activeColorFormats[i] = VK_FORMAT_R8G8B8A8_UNORM;
						}
					}
				}
			}
			m_activeColorCount = (std::max)(rtvCount, 1u);

			if (depthStencilOverride)
			{
				auto* dsRT = static_cast<VulkanRenderTarget*>(depthStencilOverride);
				if (dsRT && dsRT->HasDepth())
				{
					m_hasActiveDepth = true;
					m_usingDefaultDepth = false;
					m_activeDepthTexObj = dsRT->GetVkDepthTex();
					m_activeDepthFormat = dsRT->GetVkDepthFormat();
				}
				else
				{
					m_hasActiveDepth = false;
					m_usingDefaultDepth = false;
					m_activeDepthTexObj = nullptr;
				}
			}
			else if (rt0 && rt0->HasDepth())
			{
				m_hasActiveDepth = true;
				m_usingDefaultDepth = false;
				m_activeDepthTexObj = rt0->GetVkDepthTex();
				m_activeDepthFormat = rt0->GetVkDepthFormat();
			}
			else if (!rt0 || (rt0->GetWidth() == m_backBufferWidth && rt0->GetHeight() == m_backBufferHeight && rt0->GetSampleCount() == 1))
			{
				m_hasActiveDepth = true;
				m_usingDefaultDepth = true;
				m_activeDepthTexObj = nullptr;
				m_activeDepthFormat = m_defaultDepthFormat;
			}
			else
			{
				m_hasActiveDepth = false;
				m_usingDefaultDepth = false;
				m_activeDepthTexObj = nullptr;
			}
		}

		m_descriptorsDirty = true;

		if (prevUsingBackBuffer != m_usingBackBufferRT || prevColor0 != m_activeColorTexObjs[0])
		{
			ParaViewport vp;
			vp.X = 0;
			vp.Y = 0;
			vp.Width = m_renderTargetWidth;
			vp.Height = m_renderTargetHeight;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			SetViewport(vp);
		}
		NotifyRenderTargetsBound(effectiveCount, effectiveCount > 0 ? targets : nullptr, depthStencilOverride);
		return true;
	}

	void RenderDeviceVulkan::BeginRenderTarget(uint32_t width, uint32_t height)
	{
		m_rtSizeStack.push({ m_renderTargetWidth, m_renderTargetHeight });
		m_renderTargetWidth = width;
		m_renderTargetHeight = height;
	}

	void RenderDeviceVulkan::EndRenderTarget()
	{
		if (!m_rtSizeStack.empty())
		{
			auto prev = m_rtSizeStack.top();
			m_rtSizeStack.pop();
			m_renderTargetWidth = prev.first;
			m_renderTargetHeight = prev.second;
		}
		else
		{
			m_renderTargetWidth = m_backBufferWidth;
			m_renderTargetHeight = m_backBufferHeight;
		}
	}

	bool RenderDeviceVulkan::BlitRenderTarget(
		IRHIRenderTarget* src,
		IRHIRenderTarget* dst,
		const RECT* srcRect,
		const RECT* dstRect,
		bool linearFilter)
	{
		VkCommandBuffer cmd = EnsureRecording();
		if (!cmd)
			return false;
		EndActiveRenderPass();

		auto* vkSrc = static_cast<VulkanRenderTarget*>(src);
		auto* vkDst = static_cast<VulkanRenderTarget*>(dst);

		VkImage srcImg = VK_NULL_HANDLE;
		VkImageLayout* srcLayoutPtr = nullptr;
		uint32_t srcW = m_backBufferWidth, srcH = m_backBufferHeight;
		if (vkSrc && vkSrc->GetVkColorTex(0))
		{
			VulkanTexture* t = vkSrc->GetVkColorTex(0);
			srcImg = t->GetVkImage();
			srcLayoutPtr = &t->GetLayoutRef();
			srcW = t->GetWidth();
			srcH = t->GetHeight();
		}
		else
		{
			if (!EnsureSwapchainImageAcquired())
				return false;
			srcImg = m_swapImages[m_acquiredImageIndex];
			srcLayoutPtr = &m_swapImageLayouts[m_acquiredImageIndex];
		}

		VkImage dstImg = VK_NULL_HANDLE;
		VkImageLayout* dstLayoutPtr = nullptr;
		uint32_t dstW = m_backBufferWidth, dstH = m_backBufferHeight;
		if (vkDst && vkDst->GetVkColorTex(0))
		{
			VulkanTexture* t = vkDst->GetVkColorTex(0);
			dstImg = t->GetVkImage();
			dstLayoutPtr = &t->GetLayoutRef();
			dstW = t->GetWidth();
			dstH = t->GetHeight();
		}
		else
		{
			if (!EnsureSwapchainImageAcquired())
				return false;
			dstImg = m_swapImages[m_acquiredImageIndex];
			dstLayoutPtr = &m_swapImageLayouts[m_acquiredImageIndex];
		}

		if (!srcImg || !dstImg || srcImg == dstImg)
			return false;

		VkImageLayout prevSrcLayout = *srcLayoutPtr;
		VkImageLayout prevDstLayout = *dstLayoutPtr;
		TransitionImageLayout(cmd, srcImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *srcLayoutPtr, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		TransitionImageLayout(cmd, dstImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *dstLayoutPtr, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageBlit blit = {};
		blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		blit.srcOffsets[0] = {
			srcRect ? static_cast<int32_t>(srcRect->left) : 0,
			srcRect ? static_cast<int32_t>(srcRect->top) : 0,
			0
		};
		blit.srcOffsets[1] = {
			srcRect ? static_cast<int32_t>(srcRect->right) : static_cast<int32_t>(srcW),
			srcRect ? static_cast<int32_t>(srcRect->bottom) : static_cast<int32_t>(srcH),
			1
		};
		blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		blit.dstOffsets[0] = {
			dstRect ? static_cast<int32_t>(dstRect->left) : 0,
			dstRect ? static_cast<int32_t>(dstRect->top) : 0,
			0
		};
		blit.dstOffsets[1] = {
			dstRect ? static_cast<int32_t>(dstRect->right) : static_cast<int32_t>(dstW),
			dstRect ? static_cast<int32_t>(dstRect->bottom) : static_cast<int32_t>(dstH),
			1
		};

		m_vk.vkCmdBlitImage(
			cmd,
			srcImg, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			linearFilter ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);

		if (prevSrcLayout != VK_IMAGE_LAYOUT_UNDEFINED && prevSrcLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			TransitionImageLayout(cmd, srcImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *srcLayoutPtr, prevSrcLayout);
		else
			TransitionImageLayout(cmd, srcImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *srcLayoutPtr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (prevDstLayout != VK_IMAGE_LAYOUT_UNDEFINED && prevDstLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			TransitionImageLayout(cmd, dstImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *dstLayoutPtr, prevDstLayout);
		else
			TransitionImageLayout(cmd, dstImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *dstLayoutPtr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		return true;
	}

	bool RenderDeviceVulkan::ImportExternalFramebuffer(uintptr_t /*nativeFboOrTexHandle*/, uint32_t width, uint32_t height)
	{
		if (width > 0 && height > 0)
		{
			m_renderTargetWidth = width;
			m_renderTargetHeight = height;
		}
		return true;
	}

	bool RenderDeviceVulkan::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t /*nDataFormat*/, uint32_t /*nDataType*/)
	{
		if (!m_device || !pDataOut || nWidth <= 0 || nHeight <= 0)
			return false;

		VkImage srcImg = VK_NULL_HANDLE;
		VkFormat srcFmt = m_swapFormat;
		VkImageLayout* srcLayoutPtr = nullptr;
		uint32_t fullW = m_backBufferWidth, fullH = m_backBufferHeight;

		if (!m_usingBackBufferRT && m_activeColorTexObjs[0])
		{
			VulkanTexture* ct = m_activeColorTexObjs[0];
			srcImg = ct->GetVkImage();
			srcFmt = ct->GetVkFormat();
			srcLayoutPtr = &ct->GetLayoutRef();
			fullW = ct->GetWidth();
			fullH = ct->GetHeight();
		}
		else
		{
			if (!EnsureSwapchainImageAcquired())
				return false;
			srcImg = m_swapImages[m_acquiredImageIndex];
			srcFmt = m_swapFormat;
			srcLayoutPtr = &m_swapImageLayouts[m_acquiredImageIndex];
		}

		if (!srcImg || nLeft < 0 || nTop < 0 ||
			static_cast<uint32_t>(nLeft + nWidth) > fullW ||
			static_cast<uint32_t>(nTop + nHeight) > fullH)
		{
			return false;
		}

		VkDeviceSize rbBytes = static_cast<VkDeviceSize>(nWidth) * static_cast<VkDeviceSize>(nHeight) * 4ULL;
		VkBuffer rbBuf = VK_NULL_HANDLE;
		VkDeviceMemory rbMem = VK_NULL_HANDLE;
		if (!CreateVkBufferWithMemory(
			rbBytes,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			rbBuf, rbMem))
		{
			return false;
		}

		VkCommandBuffer cmd = EnsureRecording();
		EndActiveRenderPass();

		VkImageLayout prevLay = *srcLayoutPtr;
		TransitionImageLayout(cmd, srcImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *srcLayoutPtr, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkBufferImageCopy bic = {};
		bic.bufferOffset = 0;
		bic.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		bic.imageOffset = { nLeft, nTop, 0 };
		bic.imageExtent = { static_cast<uint32_t>(nWidth), static_cast<uint32_t>(nHeight), 1 };
		m_vk.vkCmdCopyImageToBuffer(cmd, srcImg, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rbBuf, 1, &bic);

		VkImageLayout restoreLay = (prevLay == VK_IMAGE_LAYOUT_UNDEFINED || prevLay == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			: prevLay;
		TransitionImageLayout(cmd, srcImg, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, *srcLayoutPtr, restoreLay);

		ExecuteAndSyncGPU();

		void* mapped = nullptr;
		bool ok = false;
		if (m_vk.vkMapMemory(m_device, rbMem, 0, rbBytes, 0, &mapped) == VK_SUCCESS && mapped)
		{
			const uint8_t* srcBytes = static_cast<const uint8_t*>(mapped);
			uint8_t* dstBytes = static_cast<uint8_t*>(pDataOut);
			const bool isBGRA = (srcFmt == VK_FORMAT_B8G8R8A8_UNORM || srcFmt == VK_FORMAT_B8G8R8A8_SRGB);
			const size_t pixelCount = static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight);
			if (isBGRA)
			{
				for (size_t p = 0; p < pixelCount; ++p)
				{
					dstBytes[p * 4 + 0] = srcBytes[p * 4 + 2];
					dstBytes[p * 4 + 1] = srcBytes[p * 4 + 1];
					dstBytes[p * 4 + 2] = srcBytes[p * 4 + 0];
					dstBytes[p * 4 + 3] = srcBytes[p * 4 + 3];
				}
			}
			else
			{
				std::memcpy(dstBytes, srcBytes, rbBytes);
			}
			m_vk.vkUnmapMemory(m_device, rbMem);
			ok = true;
		}

		m_vk.vkDestroyBuffer(m_device, rbBuf, nullptr);
		m_vk.vkFreeMemory(m_device, rbMem, nullptr);
		return ok;
	}

	bool RenderDeviceVulkan::ReadTexturePixels(IRHITexture* texture, void* pDataOut)
	{
		if (!m_device || !texture || !pDataOut)
			return false;

		auto* vkTex = static_cast<VulkanTexture*>(texture->GetNativeHandle());
		if (!vkTex)
			return false;
		VkFormat fmt = vkTex->GetVkFormat();
		const bool isRGBA8 = (fmt == VK_FORMAT_R8G8B8A8_UNORM || fmt == VK_FORMAT_R8G8B8A8_SRGB);
		const bool isBGRA8 = (fmt == VK_FORMAT_B8G8R8A8_UNORM || fmt == VK_FORMAT_B8G8R8A8_SRGB);
		const bool isR8    = (fmt == VK_FORMAT_R8_UNORM);
		if (!isRGBA8 && !isBGRA8 && !isR8)
			return false;

		uint32_t w = vkTex->GetWidth();
		uint32_t h = vkTex->GetHeight();
		uint32_t srcBpp = isR8 ? 1u : 4u;
		VkDeviceSize rbBytes = static_cast<VkDeviceSize>(w) * static_cast<VkDeviceSize>(h) * srcBpp;

		VkBuffer rbBuf = VK_NULL_HANDLE;
		VkDeviceMemory rbMem = VK_NULL_HANDLE;
		if (!CreateVkBufferWithMemory(
			rbBytes,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			rbBuf, rbMem))
		{
			return false;
		}

		VkCommandBuffer cmd = EnsureRecording();
		EndActiveRenderPass();

		VkImageLayout prevLay = vkTex->GetLayout();
		TransitionImageLayout(cmd, vkTex->GetVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, vkTex->GetMipLevels(), vkTex->GetLayerCount(), vkTex->GetLayoutRef(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkBufferImageCopy bic = {};
		bic.bufferOffset = 0;
		bic.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		bic.imageExtent = { w, h, 1 };
		m_vk.vkCmdCopyImageToBuffer(cmd, vkTex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rbBuf, 1, &bic);

		VkImageLayout restoreLay = (prevLay == VK_IMAGE_LAYOUT_UNDEFINED || prevLay == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			: prevLay;
		TransitionImageLayout(cmd, vkTex->GetVkImage(), VK_IMAGE_ASPECT_COLOR_BIT, vkTex->GetMipLevels(), vkTex->GetLayerCount(), vkTex->GetLayoutRef(), restoreLay);

		ExecuteAndSyncGPU();

		void* mapped = nullptr;
		bool ok = false;
		if (m_vk.vkMapMemory(m_device, rbMem, 0, rbBytes, 0, &mapped) == VK_SUCCESS && mapped)
		{
			const uint8_t* srcBytes = static_cast<const uint8_t*>(mapped);
			uint8_t* dstBytes = static_cast<uint8_t*>(pDataOut);
			size_t pixelCount = static_cast<size_t>(w) * static_cast<size_t>(h);
			if (isBGRA8)
			{
				for (size_t p = 0; p < pixelCount; ++p)
				{
					dstBytes[p * 4 + 0] = srcBytes[p * 4 + 2];
					dstBytes[p * 4 + 1] = srcBytes[p * 4 + 1];
					dstBytes[p * 4 + 2] = srcBytes[p * 4 + 0];
					dstBytes[p * 4 + 3] = srcBytes[p * 4 + 3];
				}
			}
			else if (isR8)
			{
				for (size_t p = 0; p < pixelCount; ++p)
				{
					dstBytes[p * 4 + 0] = 255u;
					dstBytes[p * 4 + 1] = 255u;
					dstBytes[p * 4 + 2] = 255u;
					dstBytes[p * 4 + 3] = srcBytes[p];
				}
			}
			else
			{
				std::memcpy(dstBytes, srcBytes, pixelCount * 4);
			}
			m_vk.vkUnmapMemory(m_device, rbMem);
			ok = true;
		}

		m_vk.vkDestroyBuffer(m_device, rbBuf, nullptr);
		m_vk.vkFreeMemory(m_device, rbMem, nullptr);
		return ok;
	}

	bool RenderDeviceVulkan::ReadDepthPixel(int x, int y, float& outDepth)
	{
		outDepth = 1.0f;
		if (!m_device || !m_hasActiveDepth)
			return false;

		VkImage depthImg = m_usingDefaultDepth ? m_defaultDepthImage : (m_activeDepthTexObj ? m_activeDepthTexObj->GetVkImage() : VK_NULL_HANDLE);
		VkFormat depthFmt = m_activeDepthFormat;
		VkImageLayout* layPtr = m_usingDefaultDepth ? &m_defaultDepthLayout : (m_activeDepthTexObj ? &m_activeDepthTexObj->GetLayoutRef() : nullptr);
		uint32_t fullW = m_usingDefaultDepth ? m_backBufferWidth : (m_activeDepthTexObj ? m_activeDepthTexObj->GetWidth() : 0);
		uint32_t fullH = m_usingDefaultDepth ? m_backBufferHeight : (m_activeDepthTexObj ? m_activeDepthTexObj->GetHeight() : 0);

		if (!depthImg || !layPtr || x < 0 || y < 0 || static_cast<uint32_t>(x) >= fullW || static_cast<uint32_t>(y) >= fullH)
			return false;

		VkBuffer rbBuf = VK_NULL_HANDLE;
		VkDeviceMemory rbMem = VK_NULL_HANDLE;
		if (!CreateVkBufferWithMemory(
			16,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			rbBuf, rbMem))
		{
			return false;
		}

		VkCommandBuffer cmd = EnsureRecording();
		EndActiveRenderPass();

		VkImageLayout prevLay = *layPtr;
		VkFlags fullAspect = AspectMaskForVkFormat(depthFmt);
		TransitionImageLayout(cmd, depthImg, fullAspect, 1, 1, *layPtr, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkBufferImageCopy bic = {};
		bic.bufferOffset = 0;
		bic.imageSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
		bic.imageOffset = { x, y, 0 };
		bic.imageExtent = { 1, 1, 1 };
		m_vk.vkCmdCopyImageToBuffer(cmd, depthImg, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rbBuf, 1, &bic);

		VkImageLayout restoreLay = (prevLay == VK_IMAGE_LAYOUT_UNDEFINED || prevLay == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			: prevLay;
		TransitionImageLayout(cmd, depthImg, fullAspect, 1, 1, *layPtr, restoreLay);

		ExecuteAndSyncGPU();

		void* mapped = nullptr;
		bool ok = false;
		if (m_vk.vkMapMemory(m_device, rbMem, 0, 16, 0, &mapped) == VK_SUCCESS && mapped)
		{
			if (depthFmt == VK_FORMAT_D32_SFLOAT || depthFmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
			{
				std::memcpy(&outDepth, mapped, sizeof(float));
			}
			else if (depthFmt == VK_FORMAT_D16_UNORM)
			{
				uint16_t u16 = 0;
				std::memcpy(&u16, mapped, sizeof(uint16_t));
				outDepth = static_cast<float>(u16) / 65535.0f;
			}
			else // D24_UNORM_S8_UINT or X8_D24_UNORM_PACK32
			{
				uint32_t raw = 0;
				std::memcpy(&raw, mapped, sizeof(uint32_t));
				outDepth = static_cast<float>(raw & 0x00FFFFFFu) / 16777215.0f;
			}
			m_vk.vkUnmapMemory(m_device, rbMem);
			ok = true;
		}

		m_vk.vkDestroyBuffer(m_device, rbBuf, nullptr);
		m_vk.vkFreeMemory(m_device, rbMem, nullptr);
		return ok;
	}

	// =========================================================================
	// Shaders, Constants, Draw Calls, States, Clear & Present
	// =========================================================================

	IRHIShaderProgram* RenderDeviceVulkan::CompileShaderProgram(const RHIShaderProgramDesc& desc)
	{
		if (!m_device)
			return nullptr;
		auto* prog = new VulkanShaderProgram(this, desc);
		if (!prog->IsValid())
		{
			prog->ReleaseRHIResource();
			return nullptr;
		}
		return prog;
	}

	bool RenderDeviceVulkan::BindShaderProgram(IRHIShaderProgram* program)
	{
		auto* vkProg = static_cast<VulkanShaderProgram*>(program);
		if (m_currentProgram != vkProg)
		{
			m_currentProgram = vkProg;
			m_descriptorsDirty = true;
		}
		return true;
	}

	bool RenderDeviceVulkan::SetShaderConstants(
		const void* vsConstants,
		uint32_t vsByteSize,
		const void* psConstants,
		uint32_t psByteSize)
	{
		if (!m_device)
			return false;

		const uint32_t uboAlign = static_cast<uint32_t>((std::max)(
			static_cast<VkDeviceSize>(16),
			m_physProps.limits.minUniformBufferOffsetAlignment));

		if (vsConstants && vsByteSize > 0)
		{
			uint32_t padded = static_cast<uint32_t>(AlignUpVkU64(vsByteSize, 16));
			void* upCpu = nullptr;
			VkBuffer upBuf = VK_NULL_HANDLE;
			VkDeviceSize upOff = 0;
			if (AllocateUpload(padded, uboAlign, upCpu, upBuf, upOff))
			{
				std::memcpy(upCpu, vsConstants, vsByteSize);
				if (padded > vsByteSize)
					std::memset(static_cast<uint8_t*>(upCpu) + vsByteSize, 0, padded - vsByteSize);
				m_currentVsUboBuf = upBuf;
				m_currentVsUboOffset = upOff;
				m_currentVsUboRange = padded;
				m_descriptorsDirty = true;
			}
		}

		if (psConstants && psByteSize > 0)
		{
			uint32_t padded = static_cast<uint32_t>(AlignUpVkU64(psByteSize, 16));
			void* upCpu = nullptr;
			VkBuffer upBuf = VK_NULL_HANDLE;
			VkDeviceSize upOff = 0;
			if (AllocateUpload(padded, uboAlign, upCpu, upBuf, upOff))
			{
				std::memcpy(upCpu, psConstants, psByteSize);
				if (padded > psByteSize)
					std::memset(static_cast<uint8_t*>(upCpu) + psByteSize, 0, padded - psByteSize);
				m_currentPsUboBuf = upBuf;
				m_currentPsUboOffset = upOff;
				m_currentPsUboRange = padded;
				m_descriptorsDirty = true;
			}
		}

		return true;
	}

	bool RenderDeviceVulkan::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		VkPrimitiveTopology vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		uint32_t vertexCount = 0;
		if (!ComputeVkTopologyAndCount(PrimitiveType, PrimitiveCount, vkTopo, vertexCount))
			return false;

		if (!PrepareForDraw(vkTopo))
			return false;

		m_vk.vkCmdDraw(m_frames[m_frameIndex].cmdBuf, vertexCount, 1, StartVertex, 0);
		RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
		return true;
	}

	bool RenderDeviceVulkan::DrawIndexedPrimitive(
		EPrimitiveType Type,
		int BaseVertexIndex,
		uint32_t /*MinIndex*/,
		uint32_t /*NumVertices*/,
		uint32_t indexStart,
		uint32_t PrimitiveCount)
	{
		return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, 1);
	}

	bool RenderDeviceVulkan::DrawIndexedInstanced(
		EPrimitiveType primitiveType,
		int baseVertexIndex,
		uint32_t indexStart,
		uint32_t primitiveCount,
		uint32_t instanceCount)
	{
		if (!m_currentIndexBuffer || !m_currentIndexBuffer->GetActiveVkBuffer() || instanceCount == 0)
			return false;

		VkPrimitiveTopology vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		uint32_t indexCount = 0;
		if (!ComputeVkTopologyAndCount(primitiveType, primitiveCount, vkTopo, indexCount))
			return false;

		if (!PrepareForDraw(vkTopo))
			return false;

		VkCommandBuffer cmd = m_frames[m_frameIndex].cmdBuf;
		m_vk.vkCmdBindIndexBuffer(
			cmd,
			m_currentIndexBuffer->GetActiveVkBuffer(),
			m_currentIndexBuffer->GetActiveOffset(),
			VK_INDEX_TYPE_UINT16);
		m_vk.vkCmdDrawIndexed(cmd, indexCount, instanceCount, indexStart, baseVertexIndex, 0);
		RenderDevice::RecordDraw(primitiveType, primitiveCount, instanceCount);
		return true;
	}

	bool RenderDeviceVulkan::DrawPrimitiveUP(
		EPrimitiveType PrimitiveType,
		uint32_t PrimitiveCount,
		const void* pVertexStreamZeroData,
		uint32_t VertexStreamZeroStride)
	{
		if (!pVertexStreamZeroData || VertexStreamZeroStride == 0 || PrimitiveCount == 0)
			return false;

		VkPrimitiveTopology vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		uint32_t srcVertexCount = 0;
		if (!ComputeVkTopologyAndCount(PrimitiveType, PrimitiveCount, vkTopo, srcVertexCount))
			return false;

		uint32_t drawVertexCount = srcVertexCount;
		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			drawVertexCount = PrimitiveCount * 3u;
		}

		uint32_t uploadBytes = drawVertexCount * VertexStreamZeroStride;
		void* vbCpu = nullptr;
		VkBuffer vbBuf = VK_NULL_HANDLE;
		VkDeviceSize vbOff = 0;
		if (!AllocateUpload(uploadBytes, 16, vbCpu, vbBuf, vbOff))
			return false;

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			const uint8_t* src = static_cast<const uint8_t*>(pVertexStreamZeroData);
			uint8_t* dst = static_cast<uint8_t*>(vbCpu);
			for (uint32_t i = 0; i < PrimitiveCount; ++i)
			{
				std::memcpy(dst + (i * 3u + 0) * VertexStreamZeroStride, src + 0, VertexStreamZeroStride);
				std::memcpy(dst + (i * 3u + 1) * VertexStreamZeroStride, src + (i + 1u) * VertexStreamZeroStride, VertexStreamZeroStride);
				std::memcpy(dst + (i * 3u + 2) * VertexStreamZeroStride, src + (i + 2u) * VertexStreamZeroStride, VertexStreamZeroStride);
			}
		}
		else
		{
			std::memcpy(vbCpu, pVertexStreamZeroData, uploadBytes);
		}

		m_streams[0].buffer = nullptr;
		m_streams[0].offsetInBytes = 0;
		m_streams[0].stride = VertexStreamZeroStride;
		if (!PrepareForDraw(vkTopo))
			return false;

		VkCommandBuffer cmd = m_frames[m_frameIndex].cmdBuf;
		m_vk.vkCmdBindVertexBuffers(cmd, 0, 1, &vbBuf, &vbOff);
		m_vk.vkCmdDraw(cmd, drawVertexCount, 1, 0, 0);
		RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
		return true;
	}

	bool RenderDeviceVulkan::DrawIndexedPrimitiveUP(
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

		VkPrimitiveTopology vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		uint32_t indexCount = 0;
		if (!ComputeVkTopologyAndCount(PrimitiveType, PrimitiveCount, vkTopo, indexCount))
			return false;

		if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		{
			vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			indexCount = PrimitiveCount * 3u;
		}

		const bool isIdx32 = (IndexDataFormat == PixelFormat::INDEX32);
		const uint32_t idxElemBytes = isIdx32 ? 4u : 2u;
		const uint32_t vbBytes = (MinVertexIndex + NumVertices) * VertexStreamZeroStride;
		const uint32_t ibBytes = indexCount * idxElemBytes;

		void* vbCpu = nullptr;
		VkBuffer vbBuf = VK_NULL_HANDLE;
		VkDeviceSize vbOff = 0;
		if (!AllocateUpload(vbBytes, 16, vbCpu, vbBuf, vbOff))
			return false;
		std::memcpy(vbCpu, pVertexStreamZeroData, vbBytes);

		void* ibCpu = nullptr;
		VkBuffer ibBuf = VK_NULL_HANDLE;
		VkDeviceSize ibOff = 0;
		if (!AllocateUpload(ibBytes, 4, ibCpu, ibBuf, ibOff))
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
		m_streams[0].offsetInBytes = 0;
		m_streams[0].stride = VertexStreamZeroStride;
		m_currentIndexBuffer = nullptr;
		if (!PrepareForDraw(vkTopo))
			return false;

		VkCommandBuffer cmd = m_frames[m_frameIndex].cmdBuf;
		m_vk.vkCmdBindVertexBuffers(cmd, 0, 1, &vbBuf, &vbOff);
		m_vk.vkCmdBindIndexBuffer(cmd, ibBuf, ibOff, isIdx32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		m_vk.vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
		RenderDevice::RecordDraw(PrimitiveType, PrimitiveCount);
		return true;
	}

	uint32_t RenderDeviceVulkan::GetRenderState(const ERenderState& State)
	{
		return m_stateTracker.GetRenderState(State);
	}

	bool RenderDeviceVulkan::SetRenderState(const ERenderState State, const uint32_t Value)
	{
		m_stateTracker.SetRenderState(State, Value);
		if (State == ERenderState::SCISSORTESTENABLE && m_isRecording && m_inRenderPass)
		{
			ApplyDynamicViewportAndScissor(m_frames[m_frameIndex].cmdBuf);
		}
		return true;
	}

	bool RenderDeviceVulkan::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		m_stateTracker.SetClipPlane(Index, pPlane);
		return true;
	}

	int RenderDeviceVulkan::GetMaxSimultaneousTextures()
	{
		return static_cast<int>(m_caps.maxTextureStages);
	}

	bool RenderDeviceVulkan::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		if (stage >= RHIStateTracker::MAX_SAMPLER_STAGES)
			return false;
		if (m_stateTracker.SetSamplerTexture(stage, texture))
			m_descriptorsDirty = true;
		auto* vkTex = texture ? static_cast<VulkanTexture*>(texture->GetNativeHandle()) : nullptr;
		if (m_boundTextures[stage] != vkTex)
		{
			m_boundTextures[stage] = vkTex;
			m_descriptorsDirty = true;
		}
		return true;
	}

	bool RenderDeviceVulkan::SetTransform(ETransformsStateType /*State*/, DeviceMatrix_ptr /*pMatrix*/)
	{
		return true;
	}

	bool RenderDeviceVulkan::SetFVF(uint32_t FVF)
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

	void RenderDeviceVulkan::SetCursorPosition(int /*X*/, int /*Y*/, uint32_t /*Flags*/)
	{
	}

	bool RenderDeviceVulkan::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		if (m_stateTracker.SetSamplerState(stage, type, value))
			m_descriptorsDirty = true;
		return true;
	}

	bool RenderDeviceVulkan::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		if (value)
			*value = m_stateTracker.GetSamplerState(stage, type);
		return true;
	}

	bool RenderDeviceVulkan::SetVertexDeclaration(VertexDeclarationPtr pVertexDeclaration)
	{
		m_currentVertexDecl = pVertexDeclaration;
		return true;
	}

	bool RenderDeviceVulkan::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
	{
		if (!ppDecl)
			return false;
		*ppDecl = new CVertexDeclaration(pVertexElements);
		return true;
	}

	bool RenderDeviceVulkan::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		m_currentIndexBuffer = pIndexData ? static_cast<VulkanBuffer*>(pIndexData->GetNativeHandle()) : nullptr;
		return true;
	}

	bool RenderDeviceVulkan::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
	{
		if (StreamNumber >= 4)
			return false;
		m_streams[StreamNumber].buffer = pStreamData ? static_cast<VulkanBuffer*>(pStreamData->GetNativeHandle()) : nullptr;
		m_streams[StreamNumber].offsetInBytes = OffsetInBytes;
		m_streams[StreamNumber].stride = Stride;
		return true;
	}

	bool RenderDeviceVulkan::SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider)
	{
		if (streamNumber >= 4)
			return false;
		m_streams[streamNumber].frequencyDivider = divider;
		return true;
	}

	ParaViewport RenderDeviceVulkan::GetViewport()
	{
		return m_currentViewport;
	}

	bool RenderDeviceVulkan::SetViewport(const ParaViewport& viewport)
	{
		m_currentViewport = viewport;
		m_stateTracker.SetViewport(viewport);
		if (m_isRecording && m_inRenderPass)
		{
			ApplyDynamicViewportAndScissor(m_frames[m_frameIndex].cmdBuf);
		}
		return true;
	}

	bool RenderDeviceVulkan::SetClearColor(const Color4f& color)
	{
		m_stateTracker.SetClearColor(color);
		return true;
	}

	bool RenderDeviceVulkan::SetClearDepth(const float depth)
	{
		m_stateTracker.SetClearDepth(depth);
		return true;
	}

	bool RenderDeviceVulkan::SetClearStencil(const int stencil)
	{
		m_stateTracker.SetClearStencil(stencil);
		return true;
	}

	bool RenderDeviceVulkan::Clear(bool color, bool depth, bool stencil)
	{
		if (!color && !depth && !stencil)
			return true;
		if (!EnsureActiveRenderPass())
			return false;

		VkClearAttachment atts[5] = {};
		uint32_t attCount = 0;

		if (color)
		{
			const Color4f& c = m_stateTracker.GetClearColor();
			for (uint32_t i = 0; i < m_activeColorCount && i < 4; ++i)
			{
				atts[attCount].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				atts[attCount].colorAttachment = i;
				atts[attCount].clearValue.color.float32[0] = c.r;
				atts[attCount].clearValue.color.float32[1] = c.g;
				atts[attCount].clearValue.color.float32[2] = c.b;
				atts[attCount].clearValue.color.float32[3] = c.a;
				++attCount;
			}
		}

		if ((depth || stencil) && m_hasActiveDepth)
		{
			VkFlags dsAspect = 0;
			if (depth)
				dsAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
			if (stencil && HasStencilInVkDepthFormat(m_activeDepthFormat))
				dsAspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

			if (dsAspect != 0)
			{
				atts[attCount].aspectMask = dsAspect;
				atts[attCount].colorAttachment = 0;
				atts[attCount].clearValue.depthStencil.depth = m_stateTracker.GetClearDepth();
				atts[attCount].clearValue.depthStencil.stencil = static_cast<uint32_t>(m_stateTracker.GetClearStencil() & 0xFF);
				++attCount;
			}
		}

		if (attCount > 0 && m_renderTargetWidth > 0 && m_renderTargetHeight > 0)
		{
			VkClearRect rect = {};
			rect.rect = { { 0, 0 }, { m_renderTargetWidth, m_renderTargetHeight } };
			rect.baseArrayLayer = 0;
			rect.layerCount = 1;
			m_vk.vkCmdClearAttachments(m_frames[m_frameIndex].cmdBuf, attCount, atts, 1, &rect);
		}

		return true;
	}

	bool RenderDeviceVulkan::SetScissorRect(RECT* pRect)
	{
		m_stateTracker.SetScissorRect(pRect);
		if (m_isRecording && m_inRenderPass)
		{
			ApplyDynamicViewportAndScissor(m_frames[m_frameIndex].cmdBuf);
		}
		return true;
	}

	bool RenderDeviceVulkan::GetScissorRect(RECT* pRect)
	{
		if (pRect)
			*pRect = m_stateTracker.GetScissorRect();
		return true;
	}

	bool RenderDeviceVulkan::BeginScene()
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

	bool RenderDeviceVulkan::EndScene()
	{
		return true;
	}

	bool RenderDeviceVulkan::Present()
	{
		if (!m_swapchain || !m_graphicsQueue)
			return false;

		if (!EnsureSwapchainImageAcquired())
			return false;

		VkCommandBuffer cmd = EnsureRecording();
		if (!cmd)
			return false;

		EndActiveRenderPass();
		TransitionImageLayout(
			cmd,
			m_swapImages[m_acquiredImageIndex],
			VK_IMAGE_ASPECT_COLOR_BIT,
			1, 1,
			m_swapImageLayouts[m_acquiredImageIndex],
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		FrameContext& fc = m_frames[m_frameIndex];
		m_vk.vkEndCommandBuffer(fc.cmdBuf);
		m_isRecording = false;

		m_vk.vkResetFences(m_device, 1, &fc.inFlightFence);

		VkFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSemaphore sigSem = m_presentSemaphores[m_acquiredImageIndex];

		VkSubmitInfo si = {};
		si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if (m_waitAcquireSemPending)
		{
			si.waitSemaphoreCount = 1;
			si.pWaitSemaphores = &fc.imageAvailableSem;
			si.pWaitDstStageMask = &waitStage;
			m_waitAcquireSemPending = false;
		}
		si.commandBufferCount = 1;
		si.pCommandBuffers = &fc.cmdBuf;
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = &sigSem;

		fc.submitSerial = m_nextSubmitSerial++;
		if (m_vk.vkQueueSubmit(m_graphicsQueue, 1, &si, fc.inFlightFence) != VK_SUCCESS)
			return false;

		VkPresentInfoKHR pi = {};
		pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pi.waitSemaphoreCount = 1;
		pi.pWaitSemaphores = &sigSem;
		pi.swapchainCount = 1;
		pi.pSwapchains = &m_swapchain;
		pi.pImageIndices = &m_acquiredImageIndex;

		VkResult pres = m_vk.vkQueuePresentKHR(m_graphicsQueue, &pi);
		m_hasAcquiredSwapImage = false;

		m_frameIndex = (m_frameIndex + 1) % FRAME_COUNT;
		m_frames[m_frameIndex].resetDone = false;

		m_currentVsUboBuf = m_zeroUboBuffer;
		m_currentVsUboOffset = 0;
		m_currentVsUboRange = 256;
		m_currentPsUboBuf = m_zeroUboBuffer;
		m_currentPsUboOffset = 0;
		m_currentPsUboRange = 256;
		m_descriptorsDirty = true;

		if (pres == VK_ERROR_OUT_OF_DATE_KHR)
		{
			ResizeSwapChain(m_backBufferWidth, m_backBufferHeight, true);
			return true;
		}
		return (pres == VK_SUCCESS || pres == VK_SUBOPTIMAL_KHR);
	}

	void RenderDeviceVulkan::Flush()
	{
		ExecuteAndSyncGPU();
	}
}

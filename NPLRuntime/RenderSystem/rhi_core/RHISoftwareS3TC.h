#pragma once
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include "Framework/RenderSystem/RenderTypes.h"

namespace ParaEngine
{
	/**
	 * Bounds-safe CPU S3TC (DXT1 / DXT3 / DXT5) to RGBA8 software decoder fallback
	 * for GPUs/APIs without hardware BC compression (mobile GLES/Vulkan, WebGL/WebGPU without texture-compression-bc).
	 */
	class RHISoftwareS3TC
	{
	public:
		static bool IsS3TCFormat(PixelFormat format);
		static size_t ComputeCompressedMipSize(PixelFormat format, uint32_t width, uint32_t height);

		/**
		 * Decodes DXT1/DXT3/DXT5 blocks into tightly-packed RGBA8 bytes (R, G, B, A byte order).
		 */
		static bool DecodeToRGBA8(PixelFormat format, const void* srcData, size_t srcByteSize, uint32_t width, uint32_t height, std::vector<uint8_t>& outRGBA);

		/**
		 * Returns true if a legacy uncompressed format (R8G8B8 24bpp, L8, A8L8, R5G6B5, A4R4G4B4, A1R5G5B5,
		 * or A8 when backendSupportsA8 == false) lacks direct modern RHI hardware support and must be expanded to RGBA8.
		 */
		static bool NeedsRGBA8Expansion(PixelFormat format, bool backendSupportsA8 = true);

		/**
		 * Expands legacy uncompressed formats (R8G8B8, L8, A8L8, A8, R5G6B5, A4R4G4B4, A1R5G5B5) into RGBA8 (R, G, B, A).
		 */
		static bool ExpandToRGBA8(PixelFormat format, const void* srcData, uint32_t width, uint32_t height, uint32_t srcRowPitch, std::vector<uint8_t>& outRGBA);
	};
}

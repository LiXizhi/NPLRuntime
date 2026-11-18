#pragma once
#include <cstdint>
#include <vector>

namespace ParaEngine
{
	class IRHITexture;

	// One contiguous row of opaque cubes in volume-local block coordinates.
	struct BlockSkySpan
	{
		uint8_t x, y, z, length;
		uint32_t color; // RGB from the block map color or its per-block color data.
		bool operator==(const BlockSkySpan& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z && length == rhs.length && color == rhs.color;
		}
	};

	struct BlockReflectionView
	{
		float inverseView[16];
		float volume[4];   // render-space corner and inverse block size
		float viewport[4]; // normalized scale, offset
		float lens[2];     // horizontal, vertical tangent of half FOV
		float daylight, padding;
		float sun[3];      // direction toward the celestial light
		float reserved;
	};

	class IBlockSkyRayTracer
	{
	public:
		static constexpr int Side = 32;
		static constexpr int AtlasWidth = 256;
		static constexpr int AtlasHeight = Side * Side * Side / AtlasWidth;
		virtual bool SupportsBlockSkyTracing() = 0;
		virtual bool TraceBlockSky(const std::vector<BlockSkySpan>& spans, IRHITexture* atlas) = 0;
		virtual bool TraceBlockReflections(const BlockReflectionView& view, IRHITexture* depth, IRHITexture* normal,
			IRHITexture* material, IRHITexture* sceneColor, IRHITexture* output) = 0;
	protected:
		virtual ~IBlockSkyRayTracer() = default;
	};
}

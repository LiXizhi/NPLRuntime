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
		bool operator==(const BlockSkySpan& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z && length == rhs.length;
		}
	};

	class IBlockSkyRayTracer
	{
	public:
		static constexpr int Side = 32;
		static constexpr int AtlasWidth = 256;
		static constexpr int AtlasHeight = Side * Side * Side / AtlasWidth;
		virtual bool SupportsBlockSkyTracing() = 0;
		virtual bool TraceBlockSky(const std::vector<BlockSkySpan>& spans, IRHITexture* atlas) = 0;
	protected:
		virtual ~IBlockSkyRayTracer() = default;
	};
}

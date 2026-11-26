#pragma once

#include <cstdint>

namespace ParaEngine
{
	namespace WorldGen
	{
		/**
		 * Fast, deterministic, allocation-free procedural noise primitives for
		 * cross-platform C++20 / WebAssembly terrain synthesis.
		 * Thread-safe and stateless: safe to call from main or worker threads.
		 */
		class ProceduralNoise
		{
		public:
			static inline uint64_t SplitMix64(uint64_t x) noexcept
			{
				x += 0x9e3779b97f4a7c15ULL;
				x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
				x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
				return x ^ (x >> 31);
			}

			static inline uint32_t Hash2D(int32_t seed, int32_t x, int32_t z) noexcept
			{
				uint64_t h = static_cast<uint64_t>(static_cast<uint32_t>(seed)) * 0x9e3779b97f4a7c15ULL;
				h ^= static_cast<uint64_t>(static_cast<uint32_t>(x)) * 0xbf58476d1ce4e5b9ULL;
				h ^= static_cast<uint64_t>(static_cast<uint32_t>(z)) * 0x94d049bb133111ebULL;
				h = (h ^ (h >> 29)) * 0xff51afd7ed558ccdULL;
				return static_cast<uint32_t>((h ^ (h >> 32)) & 0xffffffffULL);
			}

			static inline uint32_t Hash3D(int32_t seed, int32_t x, int32_t y, int32_t z) noexcept
			{
				uint64_t h = static_cast<uint64_t>(static_cast<uint32_t>(seed)) * 0x9e3779b97f4a7c15ULL;
				h ^= static_cast<uint64_t>(static_cast<uint32_t>(x)) * 0xbf58476d1ce4e5b9ULL;
				h ^= static_cast<uint64_t>(static_cast<uint32_t>(y)) * 0x94d049bb133111ebULL;
				h ^= static_cast<uint64_t>(static_cast<uint32_t>(z)) * 0x6c62272e07bb0142ULL;
				h = (h ^ (h >> 29)) * 0xff51afd7ed558ccdULL;
				return static_cast<uint32_t>((h ^ (h >> 32)) & 0xffffffffULL);
			}

			static inline float HashToUnitFloat(uint32_t h) noexcept
			{
				return static_cast<float>(h & 0x00ffffffu) * (1.0f / 16777216.0f);
			}

			/** Single-octave 2D gradient noise in [-1, 1]. */
			static float Noise2D(int32_t seed, double x, double z) noexcept;

			/** Single-octave 3D gradient noise in [-1, 1]. */
			static float Noise3D(int32_t seed, double x, double y, double z) noexcept;

			/** Normalized multi-octave 2D fBm in roughly [-1, 1]. */
			static float Fbm2D(int32_t seed, double x, double z, int octaves = 4, double lacunarity = 2.0, double gain = 0.5) noexcept;

			/** Normalized multi-octave 3D fBm in roughly [-1, 1]. */
			static float Fbm3D(int32_t seed, double x, double y, double z, int octaves = 3, double lacunarity = 2.0, double gain = 0.5) noexcept;

			/** Normalized 2D ridged multifractal in [0, 1] for sharp peaks and karst ridges. */
			static float Ridged2D(int32_t seed, double x, double z, int octaves = 4, double lacunarity = 2.05, double gain = 0.5, float offset = 1.0f) noexcept;

			/** Normalized 3D ridged multifractal in [0, 1]. */
			static float Ridged3D(int32_t seed, double x, double y, double z, int octaves = 3, double lacunarity = 2.05, double gain = 0.5, float offset = 1.0f) noexcept;

			/** Domain-warped 2D fBm in roughly [-1, 1] for meandering rivers and canyon walls. */
			static float Warped2D(int32_t seed, double x, double z, double warpAmp, double warpFreq, int octaves = 4) noexcept;

			/**
			 * 2D Worley / Cellular distance field.
			 * @param outF1: Euclidean distance to nearest feature point (in cell units, ~[0, 1.4])
			 * @param outF2: Euclidean distance to second-nearest feature point
			 * @param outCellHash: deterministic 32-bit hash of the winning cell
			 */
			static void Cellular2D(int32_t seed, double x, double z, float jitter, float* outF1, float* outF2, uint32_t* outCellHash = nullptr) noexcept;
		};
	}
}

#include "ParaEngine.h"
#include "ProceduralNoise.h"

#include <algorithm>
#include <cmath>

namespace ParaEngine
{
	namespace WorldGen
	{
		namespace
		{
			inline float Fade(float t) noexcept
			{
				return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
			}

			inline float Lerp(float t, float a, float b) noexcept
			{
				return a + t * (b - a);
			}

			// 16-direction 2D unit-ish gradients without table lookups
			inline float Grad2D(uint32_t hash, float x, float z) noexcept
			{
				switch (hash & 7u)
				{
				case 0: return  x + z;
				case 1: return -x + z;
				case 2: return  x - z;
				case 3: return -x - z;
				case 4: return  x * 1.41421356f;
				case 5: return -x * 1.41421356f;
				case 6: return  z * 1.41421356f;
				default: return -z * 1.41421356f;
				}
			}

			// Perlin's 12-edge 3D gradient evaluation
			inline float Grad3D(uint32_t hash, float x, float y, float z) noexcept
			{
				const uint32_t h = hash & 15u;
				const float u = h < 8u ? x : y;
				const float v = h < 4u ? y : ((h == 12u || h == 14u) ? x : z);
				return ((h & 1u) == 0u ? u : -u) + ((h & 2u) == 0u ? v : -v);
			}
		}

		float ProceduralNoise::Noise2D(int32_t seed, double x, double z) noexcept
		{
			const double fx = std::floor(x);
			const double fz = std::floor(z);
			const int32_t ix = static_cast<int32_t>(fx);
			const int32_t iz = static_cast<int32_t>(fz);

			const float dx = static_cast<float>(x - fx);
			const float dz = static_cast<float>(z - fz);
			const float u = Fade(dx);
			const float w = Fade(dz);

			const float n00 = Grad2D(Hash2D(seed, ix,     iz),     dx,        dz);
			const float n10 = Grad2D(Hash2D(seed, ix + 1, iz),     dx - 1.0f, dz);
			const float n01 = Grad2D(Hash2D(seed, ix,     iz + 1), dx,        dz - 1.0f);
			const float n11 = Grad2D(Hash2D(seed, ix + 1, iz + 1), dx - 1.0f, dz - 1.0f);

			const float nx0 = Lerp(u, n00, n10);
			const float nx1 = Lerp(u, n01, n11);
			// Scale factor ~0.707 normalizes maximal diagonal contribution into [-1, 1]
			return std::clamp(Lerp(w, nx0, nx1) * 0.70710678f, -1.0f, 1.0f);
		}

		float ProceduralNoise::Noise3D(int32_t seed, double x, double y, double z) noexcept
		{
			const double fx = std::floor(x);
			const double fy = std::floor(y);
			const double fz = std::floor(z);
			const int32_t ix = static_cast<int32_t>(fx);
			const int32_t iy = static_cast<int32_t>(fy);
			const int32_t iz = static_cast<int32_t>(fz);

			const float dx = static_cast<float>(x - fx);
			const float dy = static_cast<float>(y - fy);
			const float dz = static_cast<float>(z - fz);

			const float u = Fade(dx);
			const float v = Fade(dy);
			const float w = Fade(dz);

			const float n000 = Grad3D(Hash3D(seed, ix,     iy,     iz),     dx,        dy,        dz);
			const float n100 = Grad3D(Hash3D(seed, ix + 1, iy,     iz),     dx - 1.0f, dy,        dz);
			const float n010 = Grad3D(Hash3D(seed, ix,     iy + 1, iz),     dx,        dy - 1.0f, dz);
			const float n110 = Grad3D(Hash3D(seed, ix + 1, iy + 1, iz),     dx - 1.0f, dy - 1.0f, dz);
			const float n001 = Grad3D(Hash3D(seed, ix,     iy,     iz + 1), dx,        dy,        dz - 1.0f);
			const float n101 = Grad3D(Hash3D(seed, ix + 1, iy,     iz + 1), dx - 1.0f, dy,        dz - 1.0f);
			const float n011 = Grad3D(Hash3D(seed, ix,     iy + 1, iz + 1), dx,        dy - 1.0f, dz - 1.0f);
			const float n111 = Grad3D(Hash3D(seed, ix + 1, iy + 1, iz + 1), dx - 1.0f, dy - 1.0f, dz - 1.0f);

			const float nx00 = Lerp(u, n000, n100);
			const float nx10 = Lerp(u, n010, n110);
			const float nx01 = Lerp(u, n001, n101);
			const float nx11 = Lerp(u, n011, n111);

			const float nxy0 = Lerp(v, nx00, nx10);
			const float nxy1 = Lerp(v, nx01, nx11);

			return std::clamp(Lerp(w, nxy0, nxy1) * 0.85f, -1.0f, 1.0f);
		}

		float ProceduralNoise::Fbm2D(int32_t seed, double x, double z, int octaves, double lacunarity, double gain) noexcept
		{
			octaves = std::clamp(octaves, 1, 10);
			float sum = 0.0f;
			float amp = 1.0f;
			float ampSum = 0.0f;
			const float fGain = static_cast<float>(gain);

			for (int i = 0; i < octaves; ++i)
			{
				sum += Noise2D(seed + i * 1013, x, z) * amp;
				ampSum += amp;
				x *= lacunarity;
				z *= lacunarity;
				amp *= fGain;
			}
			return ampSum > 1e-6f ? (sum / ampSum) : 0.0f;
		}

		float ProceduralNoise::Fbm3D(int32_t seed, double x, double y, double z, int octaves, double lacunarity, double gain) noexcept
		{
			octaves = std::clamp(octaves, 1, 8);
			float sum = 0.0f;
			float amp = 1.0f;
			float ampSum = 0.0f;
			const float fGain = static_cast<float>(gain);

			for (int i = 0; i < octaves; ++i)
			{
				sum += Noise3D(seed + i * 1013, x, y, z) * amp;
				ampSum += amp;
				x *= lacunarity;
				y *= lacunarity;
				z *= lacunarity;
				amp *= fGain;
			}
			return ampSum > 1e-6f ? (sum / ampSum) : 0.0f;
		}

		float ProceduralNoise::Ridged2D(int32_t seed, double x, double z, int octaves, double lacunarity, double gain, float offset) noexcept
		{
			octaves = std::clamp(octaves, 1, 8);
			float sum = 0.0f;
			float amp = 0.5f;
			float ampSum = 0.0f;
			float weight = 1.0f;
			const float fGain = static_cast<float>(gain);

			for (int i = 0; i < octaves; ++i)
			{
				float signal = offset - std::abs(Noise2D(seed + i * 1619, x, z));
				signal = std::max(0.0f, signal);
				signal *= signal;
				signal *= weight;
				weight = std::clamp(signal * 1.8f, 0.0f, 1.0f);
				sum += signal * amp;
				ampSum += amp * (offset * offset);
				x *= lacunarity;
				z *= lacunarity;
				amp *= fGain;
			}
			return ampSum > 1e-6f ? std::clamp(sum / ampSum, 0.0f, 1.0f) : 0.0f;
		}

		float ProceduralNoise::Ridged3D(int32_t seed, double x, double y, double z, int octaves, double lacunarity, double gain, float offset) noexcept
		{
			octaves = std::clamp(octaves, 1, 6);
			float sum = 0.0f;
			float amp = 0.5f;
			float ampSum = 0.0f;
			float weight = 1.0f;
			const float fGain = static_cast<float>(gain);

			for (int i = 0; i < octaves; ++i)
			{
				float signal = offset - std::abs(Noise3D(seed + i * 1619, x, y, z));
				signal = std::max(0.0f, signal);
				signal *= signal;
				signal *= weight;
				weight = std::clamp(signal * 1.8f, 0.0f, 1.0f);
				sum += signal * amp;
				ampSum += amp * (offset * offset);
				x *= lacunarity;
				y *= lacunarity;
				z *= lacunarity;
				amp *= fGain;
			}
			return ampSum > 1e-6f ? std::clamp(sum / ampSum, 0.0f, 1.0f) : 0.0f;
		}

		float ProceduralNoise::Warped2D(int32_t seed, double x, double z, double warpAmp, double warpFreq, int octaves) noexcept
		{
			const float wx = Fbm2D(seed + 7919, x * warpFreq, z * warpFreq, 2, 2.0, 0.5);
			const float wz = Fbm2D(seed + 15401, (x + 37.2) * warpFreq, (z - 19.8) * warpFreq, 2, 2.0, 0.5);
			return Fbm2D(seed, x + wx * warpAmp, z + wz * warpAmp, octaves, 2.03, 0.5);
		}

		void ProceduralNoise::Cellular2D(int32_t seed, double x, double z, float jitter, float* outF1, float* outF2, uint32_t* outCellHash) noexcept
		{
			const int32_t baseX = static_cast<int32_t>(std::floor(x));
			const int32_t baseZ = static_cast<int32_t>(std::floor(z));

			float f1 = 1e9f;
			float f2 = 1e9f;
			uint32_t bestHash = 0u;
			jitter = std::clamp(jitter, 0.0f, 1.0f);

			for (int32_t dz = -1; dz <= 1; ++dz)
			{
				for (int32_t dx = -1; dx <= 1; ++dx)
				{
					const int32_t cx = baseX + dx;
					const int32_t cz = baseZ + dz;
					const uint32_t h = Hash2D(seed, cx, cz);
					const uint32_t h2 = Hash2D(seed ^ 0x5bd1e995, cx, cz);

					const double px = static_cast<double>(cx) + 0.5 + (static_cast<double>(HashToUnitFloat(h)) - 0.5) * jitter;
					const double pz = static_cast<double>(cz) + 0.5 + (static_cast<double>(HashToUnitFloat(h2)) - 0.5) * jitter;

					const float distX = static_cast<float>(px - x);
					const float distZ = static_cast<float>(pz - z);
					const float d2 = distX * distX + distZ * distZ;

					if (d2 < f1)
					{
						f2 = f1;
						f1 = d2;
						bestHash = h;
					}
					else if (d2 < f2)
					{
						f2 = d2;
					}
				}
			}

			if (outF1) *outF1 = std::sqrt(f1);
			if (outF2) *outF2 = std::sqrt(f2);
			if (outCellHash) *outCellHash = bestHash;
		}
	}
}

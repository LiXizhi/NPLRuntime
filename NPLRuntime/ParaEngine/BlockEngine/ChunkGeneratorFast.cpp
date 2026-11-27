#include "ParaEngine.h"
#include "ChunkGeneratorFast.h"
#include "ProceduralNoise.h"

#include "BlockChunk.h"
#include "BlockCommon.h"
#include "util/StringBuilder.h"
#include "BlockDataCodec.h"
#include "BlockIndex.h"
#include "BlockLightGridBase.h"
#include "BlockRegion.h"
#include "BlockTemplate.h"
#include "BlockWorld.h"
#include "ChunkMaxHeight.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <mutex>
#endif

namespace ParaEngine
{
	namespace WorldGen
	{
		namespace
		{
			inline float Clamp01(float v) noexcept
			{
				return std::clamp(v, 0.0f, 1.0f);
			}

			inline float SmoothStep(float edge0, float edge1, float x) noexcept
			{
				if (edge1 <= edge0) return x >= edge1 ? 1.0f : 0.0f;
				const float t = Clamp01((x - edge0) / (edge1 - edge0));
				return t * t * (3.0f - 2.0f * t);
			}

			inline float LerpF(float t, float a, float b) noexcept
			{
				return a + t * (b - a);
			}

			inline float TriLerp(float tx, float ty, float tz,
				float c000, float c100, float c010, float c110,
				float c001, float c101, float c011, float c111) noexcept
			{
				const float x00 = LerpF(tx, c000, c100);
				const float x10 = LerpF(tx, c010, c110);
				const float x01 = LerpF(tx, c001, c101);
				const float x11 = LerpF(tx, c011, c111);
				const float y0 = LerpF(ty, x00, x10);
				const float y1 = LerpF(ty, x01, x11);
				return LerpF(tz, y0, y1);
			}

			struct ColumnMacro2D
			{
				float targetY = 64.0f;
				float temp = 0.5f;
				float moist = 0.5f;
				float cont = 0.0f;
				float erosion = 0.5f;
				float ridge = 0.0f;
				float riverFactor = 0.0f;  // 0 = dry, 1 = river center
				float karstFactor = 0.0f;  // 0..1 karst pillar strength
				float canyonFactor = 0.0f; // 0..1 danxia/canyon mesa strength
				float sinkholeDepth = 0.0f;// >0 inside a Tiankeng shaft
				float skyMaskUpper = 0.0f; // >0 inside upper floating island
				float skyMaskLower = 0.0f; // >0 inside lower floating island
				int16_t skyBaseUpper = 0;
				int16_t skyBaseLower = 0;
				bool skyWaterfallUpper = false;
				bool skyWaterfallLower = false;
				uint8_t biomeIdx = 0;
			};

			uint8_t SelectBiomeIndex(const WorldGenProfile& profile, float temp, float moist, float targetY,
				float cont, float karstFactor, float canyonFactor) noexcept
			{
				if (profile.biomes.empty())
					return 0;

				const float sea = static_cast<float>(profile.seaLevel);
				const bool isOcean = (targetY < sea - 1.5f) && (cont < -0.16f);

				int bestIdx = 0;
				float bestScore = -1e9f;

				for (size_t i = 0; i < profile.biomes.size(); ++i)
				{
					const BiomeDef& b = profile.biomes[i];
					if (b.specialFlag == 1) // ocean biome
					{
						if (!isOcean) continue;
						return static_cast<uint8_t>(i);
					}
					if (isOcean && b.specialFlag != 1)
						continue;

					if (b.specialFlag == 2 && karstFactor < 0.32f)
						continue;
					if (b.specialFlag == 3 && canyonFactor < 0.32f)
						continue;
					if (b.specialFlag == 4 && targetY < static_cast<float>(profile.snowLineY - 25))
						continue;

					float score = 0.0f;
					const float midT = 0.5f * (b.minTemp + b.maxTemp);
					const float halfT = std::max(0.08f, 0.5f * (b.maxTemp - b.minTemp));
					const float midM = 0.5f * (b.minMoist + b.maxMoist);
					const float halfM = std::max(0.08f, 0.5f * (b.maxMoist - b.minMoist));

					const float dT = std::abs(temp - midT) / halfT;
					const float dM = std::abs(moist - midM) / halfM;
					score -= (dT * dT + dM * dM);

					if (b.specialFlag == 2) score += karstFactor * 2.2f;
					if (b.specialFlag == 3) score += canyonFactor * 2.2f;
					if (b.specialFlag == 4 && targetY >= static_cast<float>(profile.snowLineY)) score += 2.0f;

					if (score > bestScore)
					{
						bestScore = score;
						bestIdx = static_cast<uint8_t>(i);
					}
				}
				return static_cast<uint8_t>(bestIdx);
			}

			void EvaluateMacro2D(const WorldGenProfile& profile, int32_t wx, int32_t wz, ColumnMacro2D& out) noexcept
			{
				const int32_t seed = profile.seed;
				const double dx = static_cast<double>(wx);
				const double dz = static_cast<double>(wz);
				const double cFreq = std::max(0.0001, static_cast<double>(profile.continentalFreq));

				out.temp = Clamp01(0.5f + 0.58f * ProceduralNoise::Fbm2D(seed + 101, dx * 0.00065, dz * 0.00065, 3, 2.0, 0.5));
				out.moist = Clamp01(0.5f + 0.58f * ProceduralNoise::Fbm2D(seed + 211, dx * 0.00075 + 11.3, dz * 0.00075 - 7.9, 3, 2.0, 0.5));

				out.cont = ProceduralNoise::Warped2D(seed + 307, dx * cFreq, dz * cFreq, 0.45, 0.7, 4);
				out.erosion = Clamp01(0.5f + 0.55f * ProceduralNoise::Fbm2D(seed + 401, dx * (cFreq * 1.6), dz * (cFreq * 1.6), 3, 2.0, 0.5));
				out.ridge = ProceduralNoise::Ridged2D(seed + 503, dx * 0.0016, dz * 0.0016, 4, 2.05, 0.52, 1.0f);
				const float detail = ProceduralNoise::Fbm2D(seed + 607, dx * 0.011, dz * 0.011, 3, 2.05, 0.5);

				const float sea = static_cast<float>(profile.seaLevel);
				float h = profile.baseHeight;

				if (profile.presetMode == Preset_SkyIslands)
				{
					// Scenic ocean floor below with occasional dramatic sea stacks reaching above the waves
					h = sea - 26.0f + out.cont * 22.0f + detail * 6.0f;
					if (out.ridge > 0.72f && out.cont > 0.1f)
					{
						const float spire = SmoothStep(0.72f, 0.96f, out.ridge);
						h = sea + spire * 95.0f;
					}
				}
				else if (profile.presetMode == Preset_LushCavern)
				{
					// Rolling highland roof above vast underground cathedrals, pierced by skylight sinkholes
					const float mtn = SmoothStep(0.25f, 0.85f, out.ridge) * (1.0f - 0.5f * out.erosion);
					h = profile.baseHeight + 92.0f + out.cont * 35.0f + mtn * 145.0f + detail * 10.0f;
				}
				else
				{
					// Epic Overworld & Karst/Canyon presets
					if (out.cont < -0.18f)
					{
						// Continental shelf -> deep ocean trench
						const float oceanT = SmoothStep(-0.18f, -0.78f, out.cont);
						h = LerpF(oceanT, sea - 4.0f, sea - 58.0f) + detail * 4.0f;
					}
					else
					{
						const float landT = SmoothStep(-0.18f, 0.65f, out.cont);
						const float mtnMask = SmoothStep(0.10f, 0.68f, landT) * (1.0f - 0.68f * out.erosion);
						const float ridgeShape = std::pow(out.ridge, 1.35f);

						h = sea + 3.0f + landT * 36.0f + detail * (6.0f + 12.0f * (1.0f - out.erosion));
						h += mtnMask * ridgeShape * profile.mountainHeight;

						// Dramatic alpine / Himalayan peak boost on tectonic ridge intersections
						if (out.ridge > 0.62f && mtnMask > 0.35f)
						{
							const float peakT = SmoothStep(0.62f, 0.96f, out.ridge) * SmoothStep(0.35f, 0.85f, mtnMask);
							h += peakT * profile.peakBoost;
						}
					}
				}

				// Zhangjiajie Karst Pillar Forest zones
				if (profile.karstStrength > 0.01f && out.cont > -0.05f && profile.presetMode != Preset_SkyIslands)
				{
					const float karstZone = (profile.presetMode == Preset_KarstCanyon)
						? SmoothStep(0.25f, 0.62f, out.moist)
						: (SmoothStep(0.56f, 0.82f, out.moist) * SmoothStep(0.05f, 0.45f, out.cont));
					if (karstZone > 0.02f)
					{
						float f1 = 1.0f, f2 = 1.0f;
						uint32_t cellH = 0;
						ProceduralNoise::Cellular2D(seed + 709, dx * 0.026, dz * 0.026, 0.85f, &f1, &f2, &cellH);
						const float pillarCore = 1.0f - SmoothStep(0.14f, 0.44f, f1);
						const float pillarHeightVar = 0.45f + 0.55f * ProceduralNoise::HashToUnitFloat(cellH);
						const float pillarBoost = karstZone * pillarCore * pillarHeightVar * 195.0f * profile.karstStrength;
						h += pillarBoost;
						out.karstFactor = Clamp01(karstZone * (0.35f + 0.65f * pillarCore));
					}
				}

				// Danxia Rainbow Mesa & Terraced Canyon zones
				if (profile.canyonStrength > 0.01f && out.cont > 0.0f && profile.presetMode != Preset_SkyIslands)
				{
					const float canyonZone = (profile.presetMode == Preset_KarstCanyon)
						? SmoothStep(0.65f, 0.32f, out.moist)
						: (SmoothStep(0.62f, 0.86f, out.temp) * SmoothStep(0.45f, 0.18f, out.moist) * SmoothStep(0.05f, 0.40f, out.cont));
					if (canyonZone > 0.02f)
					{
						const float plateauRaw = ProceduralNoise::Ridged2D(seed + 811, dx * 0.0028, dz * 0.0028, 3, 2.0, 0.5, 1.0f);
						const float stepSize = 14.0f;
						const float rawMesa = sea + 24.0f + plateauRaw * 165.0f * profile.canyonStrength;
						const float stairIdx = std::floor(rawMesa / stepSize);
						const float stairFrac = (rawMesa / stepSize) - stairIdx;
						const float terraced = (stairIdx + SmoothStep(0.25f, 0.75f, stairFrac)) * stepSize;
						h = LerpF(canyonZone, h, terraced);
						out.canyonFactor = canyonZone;
					}
				}

				// Winding River & Canyon Gorge carving
				if (profile.riverStrength > 0.01f && out.cont > -0.16f && profile.presetMode != Preset_SkyIslands)
				{
					const float rVal = std::abs(ProceduralNoise::Warped2D(seed + 919, dx * 0.0014, dz * 0.0014, 0.65, 0.85, 3));
					const float riverHalfWidth = 0.038f * profile.riverStrength;
					if (rVal < riverHalfWidth * 2.6f)
					{
						const float bankT = 1.0f - SmoothStep(0.0f, riverHalfWidth * 2.6f, rVal);
						const float channelT = 1.0f - SmoothStep(0.0f, riverHalfWidth, rVal);
						const float riverBedY = sea - 5.0f - 2.0f * channelT;
						// Smoothly carve high mountains down into dramatic navigable river gorges
						h = LerpF(bankT * 0.92f, h, LerpF(channelT, sea + 1.5f, riverBedY));
						out.riverFactor = channelT;
					}
				}

				// Tiankeng (giant circular sinkhole shafts plunging into deep lush caverns)
				if (profile.sinkholeChance > 0.001f && h > sea + 18.0f && out.riverFactor < 0.1f)
				{
					float sf1 = 1.0f, sf2 = 1.0f;
					uint32_t sHash = 0;
					ProceduralNoise::Cellular2D(seed + 1103, dx * 0.0038, dz * 0.0038, 0.75f, &sf1, &sf2, &sHash);
					const float cellRoll = ProceduralNoise::HashToUnitFloat(sHash);
					if (cellRoll < profile.sinkholeChance * 0.14f && sf1 < 0.16f)
					{
						const float rimT = 1.0f - SmoothStep(0.07f, 0.16f, sf1);
						out.sinkholeDepth = rimT;
					}
				}

				// Floating Sky Islands (sparse upper & lower tiers)
				if (profile.skyIslandChance > 0.001f)
				{
					// Upper Sky Tier
					float uf1 = 1.0f, uf2 = 1.0f;
					uint32_t uHash = 0;
					ProceduralNoise::Cellular2D(seed + 1301, dx * 0.0052, dz * 0.0052, 0.8f, &uf1, &uf2, &uHash);
					if (ProceduralNoise::HashToUnitFloat(uHash) < profile.skyIslandChance)
					{
						const float warp = ProceduralNoise::Fbm2D(seed + 1307, dx * 0.022, dz * 0.022, 3, 2.0, 0.5) * 0.10f;
						const float radius = 0.24f + 0.14f * ProceduralNoise::HashToUnitFloat(uHash >> 5);
						const float d = uf1 + warp;
						if (d < radius)
						{
							out.skyMaskUpper = Clamp01(1.0f - (d / radius));
							const float altT = ProceduralNoise::HashToUnitFloat(uHash ^ 0x9e3779b9u);
							out.skyBaseUpper = static_cast<int16_t>(LerpF(altT,
								static_cast<float>(profile.skyIslandMinY),
								static_cast<float>(profile.skyIslandMaxY)));
							// Deterministic rim spring point for plunging waterfall
							if (profile.skyWaterfallChance > 0.01f && out.skyMaskUpper > 0.06f && out.skyMaskUpper < 0.16f)
							{
								const uint32_t wfH = ProceduralNoise::Hash2D(seed + 1319, wx >> 1, wz >> 1);
								out.skyWaterfallUpper = (wfH & 63u) == 0u &&
									(ProceduralNoise::HashToUnitFloat(uHash ^ 0x12345u) < profile.skyWaterfallChance);
							}
						}
					}

					// Lower Sky Tier (active primarily in SkyIslands preset for multi-altitude traversal)
					if (profile.presetMode == Preset_SkyIslands)
					{
						float lf1 = 1.0f, lf2 = 1.0f;
						uint32_t lHash = 0;
						ProceduralNoise::Cellular2D(seed + 1409, dx * 0.0075 + 19.4, dz * 0.0075 - 13.1, 0.82f, &lf1, &lf2, &lHash);
						if (ProceduralNoise::HashToUnitFloat(lHash) < profile.skyIslandChance * 0.9f)
						{
							const float warp = ProceduralNoise::Fbm2D(seed + 1423, dx * 0.028, dz * 0.028, 2, 2.0, 0.5) * 0.09f;
							const float radius = 0.22f + 0.13f * ProceduralNoise::HashToUnitFloat(lHash >> 4);
							const float d = lf1 + warp;
							if (d < radius)
							{
								out.skyMaskLower = Clamp01(1.0f - (d / radius));
								const float altT = ProceduralNoise::HashToUnitFloat(lHash ^ 0x7f4a7c15u);
								out.skyBaseLower = static_cast<int16_t>(LerpF(altT, 118.0f,
									static_cast<float>(std::max<int>(180, profile.skyIslandMinY - 20))));
								if (profile.skyWaterfallChance > 0.01f && out.skyMaskLower > 0.07f && out.skyMaskLower < 0.16f)
								{
									const uint32_t wfH = ProceduralNoise::Hash2D(seed + 1429, wx >> 1, wz >> 1);
									out.skyWaterfallLower = (wfH & 63u) == 0u;
								}
							}
						}
					}
				}

				h = std::clamp(h, static_cast<float>(profile.bedrockY + 8), static_cast<float>(BlockConfig::MaxBlockY - 64));
				out.targetY = h;
				out.biomeIdx = SelectBiomeIndex(profile, out.temp, out.moist, h, out.cont, out.karstFactor, out.canyonFactor);
			}

#ifndef EMSCRIPTEN_SINGLE_THREAD
			std::mutex g_profileMutex;
#endif
			std::unordered_map<int, std::shared_ptr<const WorldGenProfile>>& GetProfileMap()
			{
				static std::unordered_map<int, std::shared_ptr<const WorldGenProfile>> s_profiles;
				return s_profiles;
			}
		}

		// =========================================================================
		// ChunkColumnBuffer
		// =========================================================================

		ChunkColumnBuffer::ChunkColumnBuffer()
		{
			m_sectionLookup.fill(-1);
			m_sectionPool.reserve(32);
			Reset(0, 0);
		}

		void ChunkColumnBuffer::Reset(int32_t chunkX, int32_t chunkZ)
		{
			m_chunkX = chunkX;
			m_chunkZ = chunkZ;
			m_activeSectionCount = 0;
			m_sectionLookup.fill(-1);

			m_surfaceY.fill(kEmptyY);
			m_surfaceBlockId.fill(0);
			m_biomeId.fill(1);
			m_slopeGrad.fill(0);
			m_skyTopY.fill(kEmptyY);
			m_skyBottomY.fill(kEmptyY);
			m_caveFloorY.fill(kEmptyY);
			m_caveCeilY.fill(kEmptyY);
			m_waterDepth.fill(0);
			m_solidHeight.fill(kEmptyY);
			m_maxHeight.fill(kEmptyY);
		}

		bool ChunkColumnBuffer::IsDefaultTransparentBlock(uint16_t blockId) noexcept
		{
			switch (blockId)
			{
			case 0:
			case 17:  // Ice
			case 26:  // Cactus (legacy alias)
			case 73:  // TransparentColorBlock
			case 74:  // Glass_Carpet
			case 75:  // Water
			case 76:  // Still_Water
			case 85:  // Jungle_Leaves
			case 86:  // Oak_Leaves
			case 91:  // Spruce_Leaves
			case 92:  // Cherry_Blossoms
			case 95:  // Glass
			case 100: // Torch
			case 101: // Fence
			case 102: // GlassPane
			case 113: // TallGrass
			case 114: // Fern
			case 115: // Red_Rose
			case 116: // Yellow_Flower
			case 117: // Red_Mushroom
			case 118: // Cobweb
			case 119: // Oak_Sapling
			case 120: // Spruce_Sapling
			case 121: // Birch_Sapling
			case 122: // Jungle_Sapling
			case 129: // Birch_Leaves
			case 132: // DeadBush
			case 141: // Brown_Mushroom
			case 160: // Oak_Wood_Slab
			case 161: // Reed
			case 162: // Vines
			case 165: // Cactus
			case 176: // Stone_Slab
			case 177: // Sandstone_Slab
			case 178: // Cobblestone_Slab
			case 180: // StoneBrick_Slab
			case 222: // LilyPad
			case 256: // Stone_Slope
			case 257: // Sand_Slope
			case 258: // Oak_Wood_Slope
			case 259: // Cobblestone_Slope
			case 281: // ColorBlock_Slab
			case 282: // ColorBlock_Slope
			case 284: // TransparentColorBlock_Slab
			case 285: // TransparentColorBlock_Slope
				return true;
			default:
				return false;
			}
		}

		ChunkColumnBuffer::SectionData* ChunkColumnBuffer::GetOrCreateSection(int16_t sectionY)
		{
			if (!BlockConfig::IsSectionY(sectionY))
				return nullptr;
			const int slot = sectionY - BlockConfig::MinSectionY;
			int16_t poolIdx = m_sectionLookup[slot];
			if (poolIdx >= 0)
				return m_sectionPool[poolIdx].get();

			poolIdx = static_cast<int16_t>(m_activeSectionCount++);
			if (static_cast<size_t>(poolIdx) >= m_sectionPool.size())
				m_sectionPool.push_back(std::make_unique<SectionData>());

			SectionData* sec = m_sectionPool[poolIdx].get();
			sec->Clear(sectionY);
			m_sectionLookup[slot] = poolIdx;
			return sec;
		}

		const ChunkColumnBuffer::SectionData* ChunkColumnBuffer::FindSection(int16_t sectionY) const noexcept
		{
			if (!BlockConfig::IsSectionY(sectionY))
				return nullptr;
			const int16_t poolIdx = m_sectionLookup[sectionY - BlockConfig::MinSectionY];
			return poolIdx >= 0 ? m_sectionPool[poolIdx].get() : nullptr;
		}

		void ChunkColumnBuffer::SetBlock(int lx, int wy, int lz, uint16_t blockId, uint32_t blockData, bool onlyIfAir, bool isTransparentHint)
		{
			if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || !BlockConfig::IsBlockY(wy))
				return;

			const int16_t sectionY = static_cast<int16_t>(BlockConfig::SectionY(wy));
			if (blockId == 0)
			{
				const int slot = sectionY - BlockConfig::MinSectionY;
				const int16_t poolIdx = m_sectionLookup[slot];
				if (poolIdx < 0)
					return;
				SectionData* sec = m_sectionPool[poolIdx].get();
				const int ly = wy - sectionY * 16;
				const uint16_t idx = PackBlockIndex(static_cast<uint16_t>(lx), static_cast<uint16_t>(ly), static_cast<uint16_t>(lz));
				if (sec->ids[idx] != 0)
				{
					sec->ids[idx] = 0;
					sec->data[idx] = 0;
					if (sec->nonAirCount > 0) --sec->nonAirCount;
				}
				return;
			}

			SectionData* sec = GetOrCreateSection(sectionY);
			if (!sec)
				return;

			const int ly = wy - sectionY * 16;
			const uint16_t idx = PackBlockIndex(static_cast<uint16_t>(lx), static_cast<uint16_t>(ly), static_cast<uint16_t>(lz));
			const uint16_t prevId = sec->ids[idx];
			if (onlyIfAir && prevId != 0)
				return;

			if (prevId == 0)
				++sec->nonAirCount;
			sec->ids[idx] = blockId;
			sec->data[idx] = blockData;

			const int colIdx = (lz << 4) + lx;
			const int16_t y16 = static_cast<int16_t>(wy);
			if (y16 > m_maxHeight[colIdx])
				m_maxHeight[colIdx] = y16;
			if (!isTransparentHint && !IsDefaultTransparentBlock(blockId) && y16 > m_solidHeight[colIdx])
				m_solidHeight[colIdx] = y16;
		}

		uint16_t ChunkColumnBuffer::GetBlockId(int lx, int wy, int lz) const noexcept
		{
			if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || !BlockConfig::IsBlockY(wy))
				return 0;
			const int16_t sectionY = static_cast<int16_t>(BlockConfig::SectionY(wy));
			const SectionData* sec = FindSection(sectionY);
			if (!sec)
				return 0;
			const int ly = wy - sectionY * 16;
			return sec->ids[PackBlockIndex(static_cast<uint16_t>(lx), static_cast<uint16_t>(ly), static_cast<uint16_t>(lz))];
		}

		uint32_t ChunkColumnBuffer::GetBlockData(int lx, int wy, int lz) const noexcept
		{
			if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || !BlockConfig::IsBlockY(wy))
				return 0;
			const int16_t sectionY = static_cast<int16_t>(BlockConfig::SectionY(wy));
			const SectionData* sec = FindSection(sectionY);
			if (!sec)
				return 0;
			const int ly = wy - sectionY * 16;
			return sec->data[PackBlockIndex(static_cast<uint16_t>(lx), static_cast<uint16_t>(ly), static_cast<uint16_t>(lz))];
		}

		void ChunkColumnBuffer::FillColumnRange(int lx, int lz, int yStart, int yEnd, uint16_t blockId, uint32_t blockData, bool isTransparentHint)
		{
			if (lx < 0 || lx > 15 || lz < 0 || lz > 15 || yStart > yEnd)
				return;
			yStart = std::max<int>(yStart, BlockConfig::MinBlockY);
			yEnd = std::min<int>(yEnd, BlockConfig::MaxBlockY);
			if (yStart > yEnd)
				return;

			int y = yStart;
			while (y <= yEnd)
			{
				const int16_t sy = static_cast<int16_t>(BlockConfig::SectionY(y));
				const int secBaseY = sy * 16;
				const int lyStart = y - secBaseY;
				const int lyEnd = std::min<int>(15, yEnd - secBaseY);

				if (blockId == 0)
				{
					const int16_t poolIdx = m_sectionLookup[sy - BlockConfig::MinSectionY];
					if (poolIdx >= 0)
					{
						SectionData* sec = m_sectionPool[poolIdx].get();
						for (int ly = lyStart; ly <= lyEnd; ++ly)
						{
							const uint16_t idx = PackBlockIndex(static_cast<uint16_t>(lx), static_cast<uint16_t>(ly), static_cast<uint16_t>(lz));
							if (sec->ids[idx] != 0)
							{
								sec->ids[idx] = 0;
								sec->data[idx] = 0;
								if (sec->nonAirCount > 0) --sec->nonAirCount;
							}
						}
					}
				}
				else
				{
					SectionData* sec = GetOrCreateSection(sy);
					if (sec)
					{
						for (int ly = lyStart; ly <= lyEnd; ++ly)
						{
							const uint16_t idx = PackBlockIndex(static_cast<uint16_t>(lx), static_cast<uint16_t>(ly), static_cast<uint16_t>(lz));
							if (sec->ids[idx] == 0) ++sec->nonAirCount;
							sec->ids[idx] = blockId;
							sec->data[idx] = blockData;
						}
					}
				}
				y = secBaseY + lyEnd + 1;
			}

			if (blockId != 0)
			{
				const int colIdx = (lz << 4) + lx;
				const int16_t top = static_cast<int16_t>(yEnd);
				if (top > m_maxHeight[colIdx]) m_maxHeight[colIdx] = top;
				if (!isTransparentHint && !IsDefaultTransparentBlock(blockId) && top > m_solidHeight[colIdx])
					m_solidHeight[colIdx] = top;
			}
		}

		void ChunkColumnBuffer::FillBox(int x1, int y1, int z1, int x2, int y2, int z2, uint16_t blockId, uint32_t blockData, int mode)
		{
			if (x1 > x2) std::swap(x1, x2);
			if (y1 > y2) std::swap(y1, y2);
			if (z1 > z2) std::swap(z1, z2);

			const int cx1 = std::max(0, x1), cx2 = std::min(15, x2);
			const int cz1 = std::max(0, z1), cz2 = std::min(15, z2);
			const int cy1 = std::max<int>(BlockConfig::MinBlockY, y1);
			const int cy2 = std::min<int>(BlockConfig::MaxBlockY, y2);
			if (cx1 > cx2 || cz1 > cz2 || cy1 > cy2)
				return;

			const bool onlyIfAir = (mode == 1);
			const bool hollow = (mode == 2);

			for (int z = cz1; z <= cz2; ++z)
			{
				for (int x = cx1; x <= cx2; ++x)
				{
					if (!onlyIfAir && !hollow)
					{
						FillColumnRange(x, z, cy1, cy2, blockId, blockData);
					}
					else
					{
						for (int y = cy1; y <= cy2; ++y)
						{
							if (hollow && x != x1 && x != x2 && y != y1 && y != y2 && z != z1 && z != z2)
								continue;
							SetBlock(x, y, z, blockId, blockData, onlyIfAir);
						}
					}
				}
			}
		}

		void ChunkColumnBuffer::FillEllipsoid(float cx, float cy, float cz, float rx, float ry, float rz, uint16_t blockId, uint32_t blockData, bool onlyIfAir)
		{
			if (rx <= 0.1f || ry <= 0.1f || rz <= 0.1f)
				return;
			const int x1 = std::max(0, static_cast<int>(std::floor(cx - rx)));
			const int x2 = std::min(15, static_cast<int>(std::ceil(cx + rx)));
			const int z1 = std::max(0, static_cast<int>(std::floor(cz - rz)));
			const int z2 = std::min(15, static_cast<int>(std::ceil(cz + rz)));
			const int y1 = std::max<int>(BlockConfig::MinBlockY, static_cast<int>(std::floor(cy - ry)));
			const int y2 = std::min<int>(BlockConfig::MaxBlockY, static_cast<int>(std::ceil(cy + ry)));
			if (x1 > x2 || z1 > z2 || y1 > y2)
				return;

			const float invRx2 = 1.0f / (rx * rx);
			const float invRy2 = 1.0f / (ry * ry);
			const float invRz2 = 1.0f / (rz * rz);

			for (int z = z1; z <= z2; ++z)
			{
				const float dz2 = (z - cz) * (z - cz) * invRz2;
				for (int x = x1; x <= x2; ++x)
				{
					const float dxz2 = (x - cx) * (x - cx) * invRx2 + dz2;
					if (dxz2 > 1.0f) continue;
					for (int y = y1; y <= y2; ++y)
					{
						const float d2 = dxz2 + (y - cy) * (y - cy) * invRy2;
						if (d2 <= 1.0f)
							SetBlock(x, y, z, blockId, blockData, onlyIfAir);
					}
				}
			}
		}

		void ChunkColumnBuffer::StampTree(int lx, int wy, int lz, int style, uint16_t trunkId, uint32_t trunkData,
			uint16_t leafId, uint32_t leafData, uint16_t extraId, uint32_t extraData, int height, int radius, uint32_t seed)
		{
			height = std::clamp(height, 3, 36);
			radius = std::clamp(radius, 1, 9);

			if (lx + radius + 2 < 0 || lx - radius - 2 > 15 || lz + radius + 2 < 0 || lz - radius - 2 > 15)
				return;

			switch (style)
			{
			case Tree_SprucePine:
			{
				FillColumnRange(lx, lz, wy, wy + height - 1, trunkId, trunkData);
				const int crownStart = wy + std::max(2, height / 3);
				for (int y = crownStart; y <= wy + height + 1; ++y)
				{
					const float progress = static_cast<float>(y - crownStart) / static_cast<float>(std::max(1, wy + height + 1 - crownStart));
					const int layerR = ((y - crownStart) & 1) == 0
						? std::max(1, static_cast<int>(std::round((1.0f - progress * 0.82f) * radius)))
						: std::max(0, static_cast<int>(std::round((1.0f - progress * 0.9f) * (radius - 1))));
					for (int dz = -layerR; dz <= layerR; ++dz)
					{
						for (int dx = -layerR; dx <= layerR; ++dx)
						{
							if (dx * dx + dz * dz <= layerR * layerR + 1)
							{
								SetBlock(lx + dx, y, lz + dz, leafId, leafData, true, true);
								if (extraId != 0 && y == wy + height + 1 && dx == 0 && dz == 0)
									SetBlock(lx, y + 1, lz, extraId, extraData, true);
							}
						}
					}
				}
				break;
			}
			case Tree_CherryUmbrella:
			{
				const int splitY = wy + std::max(2, (height * 3) / 5);
				FillColumnRange(lx, lz, wy, splitY, trunkId, trunkData);
				// Graceful forked branches + wide umbrella blossom canopy + hanging petals
				for (int b = 0; b < 3; ++b)
				{
					const uint32_t bh = ProceduralNoise::Hash2D(static_cast<int32_t>(seed), b, height);
					const int bx = lx + static_cast<int>(bh % 3u) - 1;
					const int bz = lz + static_cast<int>((bh >> 3) % 3u) - 1;
					FillColumnRange(bx, bz, splitY, wy + height - 1, trunkId, trunkData);
				}
				FillEllipsoid(static_cast<float>(lx), static_cast<float>(wy + height), static_cast<float>(lz),
					static_cast<float>(radius) + 0.8f, 2.3f, static_cast<float>(radius) + 0.8f, leafId, leafData, true);
				for (int dz = -radius; dz <= radius; ++dz)
				{
					for (int dx = -radius; dx <= radius; ++dx)
					{
						const int d2 = dx * dx + dz * dz;
						if (d2 >= (radius - 1) * (radius - 1) && d2 <= radius * radius + 1)
						{
							if ((ProceduralNoise::Hash2D(static_cast<int32_t>(seed), dx + 17, dz + 31) & 3u) == 0u)
								SetBlock(lx + dx, wy + height - 2, lz + dz, leafId, leafData, true, true);
						}
					}
				}
				break;
			}
			case Tree_GiantJungle:
			{
				// 2x2 or thick buttressed jungle giant with tiered canopy and hanging vines
				FillBox(lx, wy, lz, lx + (height > 12 ? 1 : 0), wy + height - 1, lz + (height > 12 ? 1 : 0), trunkId, trunkData, 0);
				FillEllipsoid(static_cast<float>(lx) + 0.5f, static_cast<float>(wy + height), static_cast<float>(lz) + 0.5f,
					static_cast<float>(radius) + 1.2f, 2.6f, static_cast<float>(radius) + 1.2f, leafId, leafData, true);
				// Mid-story canopy pod
				const int podSideX = ((seed & 1u) ? 2 : -2);
				const int podSideZ = ((seed & 2u) ? 2 : -2);
				FillEllipsoid(static_cast<float>(lx + podSideX), static_cast<float>(wy + (height * 2) / 3),
					static_cast<float>(lz + podSideZ), static_cast<float>(std::max(2, radius - 1)), 1.8f,
					static_cast<float>(std::max(2, radius - 1)), leafId, leafData, true);
				if (extraId != 0)
				{
					const int r = radius + 1;
					for (int a = -r; a <= r; a += 2)
					{
						const int drop = 2 + static_cast<int>(ProceduralNoise::Hash2D(static_cast<int32_t>(seed), a, 99) % 4u);
						for (int dy = 0; dy < drop; ++dy)
						{
							SetBlock(lx + a, wy + height - 1 - dy, lz - r, extraId, extraData, true, true);
							SetBlock(lx + a, wy + height - 1 - dy, lz + r, extraId, extraData, true, true);
						}
					}
				}
				break;
			}
			case Tree_SlenderMaple:
			{
				FillColumnRange(lx, lz, wy, wy + height - 1, trunkId, trunkData);
				FillEllipsoid(static_cast<float>(lx), static_cast<float>(wy + height - 1), static_cast<float>(lz),
					static_cast<float>(radius) + 0.3f, static_cast<float>(std::max(2, height / 3)) + 0.6f,
					static_cast<float>(radius) + 0.3f, leafId, leafData, true);
				break;
			}
			case Tree_OasisPalm:
			{
				const int leanX = ((seed & 1u) ? 1 : -1) * ((seed & 4u) ? 1 : 0);
				const int leanZ = ((seed & 2u) ? 1 : -1) * ((leanX == 0) ? 1 : 0);
				int curX = lx, curZ = lz;
				for (int dy = 0; dy < height; ++dy)
				{
					if (dy == height / 2 || dy == (height * 4) / 5)
					{
						curX += leanX;
						curZ += leanZ;
					}
					SetBlock(curX, wy + dy, curZ, trunkId, trunkData);
				}
				const int topY = wy + height;
				SetBlock(curX, topY, curZ, leafId, leafData, true, true);
				const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
				for (const auto& d : dirs)
				{
					for (int r = 1; r <= radius; ++r)
					{
						const int drop = (r == radius) ? 1 : 0;
						SetBlock(curX + d[0] * r, topY - drop, curZ + d[1] * r, leafId, leafData, true, true);
					}
				}
				break;
			}
			case Tree_GiantMushroom:
			{
				FillColumnRange(lx, lz, wy, wy + height - 1, trunkId, trunkData);
				const int capY = wy + height;
				for (int dz = -radius; dz <= radius; ++dz)
				{
					for (int dx = -radius; dx <= radius; ++dx)
					{
						const int d2 = dx * dx + dz * dz;
						if (d2 <= radius * radius + 1)
						{
							const bool rim = d2 >= (radius - 1) * (radius - 1) && radius >= 2;
							SetBlock(lx + dx, capY - (rim ? 1 : 0), lz + dz, leafId, leafData, true);
						}
					}
				}
				if (extraId != 0)
					SetBlock(lx, capY - 1, lz, extraId, extraData, false);
				break;
			}
			default: // Tree_Oak
			{
				FillColumnRange(lx, lz, wy, wy + height - 1, trunkId, trunkData);
				FillEllipsoid(static_cast<float>(lx), static_cast<float>(wy + height - 1), static_cast<float>(lz),
					static_cast<float>(radius) + 0.6f, 2.4f, static_cast<float>(radius) + 0.6f, leafId, leafData, true);
				break;
			}
			}
		}

		void ChunkColumnBuffer::StampCrystalCluster(int lx, int wy, int lz, uint16_t coreId, uint16_t shellId, uint32_t shellData,
			int height, int radius, bool pointUp, uint32_t seed)
		{
			height = std::clamp(height, 2, 18);
			radius = std::clamp(radius, 1, 4);
			if (lx + radius + 2 < 0 || lx - radius - 2 > 15 || lz + radius + 2 < 0 || lz - radius - 2 > 15)
				return;

			const int dir = pointUp ? 1 : -1;
			// Central glowing core + translucent outer spire
			const int coreH = std::max(1, height / 2);
			for (int i = 0; i < height; ++i)
			{
				const int y = wy + i * dir;
				if (i < coreH && coreId != 0)
					SetBlock(lx, y, lz, coreId, 0);
				else
					SetBlock(lx, y, lz, shellId, shellData, false, true);

				if (i < (height * 2) / 3 && radius >= 2)
				{
					SetBlock(lx + 1, y, lz, shellId, shellData, true, true);
					SetBlock(lx - 1, y, lz, shellId, shellData, true, true);
					SetBlock(lx, y, lz + 1, shellId, shellData, true, true);
					SetBlock(lx, y, lz - 1, shellId, shellData, true, true);
				}
			}

			// Angled side shards
			const int offsets[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
			for (int s = 0; s < 4; ++s)
			{
				if (((seed >> s) & 1u) == 0u) continue;
				const int shardH = std::max(2, (height * 3) / 5);
				for (int i = 0; i < shardH; ++i)
				{
					const int step = (i >= shardH / 2) ? 2 : 1;
					SetBlock(lx + offsets[s][0] * step, wy + i * dir, lz + offsets[s][1] * step, shellId, shellData, true, true);
				}
			}
		}

		void ChunkColumnBuffer::FinalizeHeights(CBlockWorld* world)
		{
			m_solidHeight.fill(kEmptyY);
			m_maxHeight.fill(kEmptyY);

			if (m_activeSectionCount <= 0)
				return;

			// Scan active sections from highest sectionY down so each column stops early
			std::vector<const SectionData*> sorted;
			sorted.reserve(m_activeSectionCount);
			for (int i = 0; i < m_activeSectionCount; ++i)
			{
				const SectionData* sec = m_sectionPool[i].get();
				if (sec && sec->nonAirCount > 0)
					sorted.push_back(sec);
			}
			std::sort(sorted.begin(), sorted.end(), [](const SectionData* a, const SectionData* b) {
				return a->sectionY > b->sectionY;
			});

			int resolvedSolidCount = 0;
			std::array<uint8_t, 256> doneSolid{};

			for (const SectionData* sec : sorted)
			{
				const int baseY = sec->sectionY * 16;
				for (int ly = 15; ly >= 0; --ly)
				{
					const int wy = baseY + ly;
					const int layerOffset = ly << 8;
					for (int lz = 0; lz < 16; ++lz)
					{
						for (int lx = 0; lx < 16; ++lx)
						{
							const int col = (lz << 4) + lx;
							if (doneSolid[col])
								continue;

							const uint16_t id = sec->ids[layerOffset + (lz << 4) + lx];
							if (id == 0)
								continue;

							if (wy > m_maxHeight[col])
								m_maxHeight[col] = static_cast<int16_t>(wy);

							bool transparent = IsDefaultTransparentBlock(id);
							if (world)
							{
								if (BlockTemplate* tpl = world->GetBlockTemplate(id))
									transparent = tpl->IsTransparent();
							}
							if (!transparent)
							{
								m_solidHeight[col] = static_cast<int16_t>(wy);
								if (m_maxHeight[col] < m_solidHeight[col])
									m_maxHeight[col] = m_solidHeight[col];
								doneSolid[col] = 1;
								if (++resolvedSolidCount == 256)
									return;
							}
						}
					}
				}
			}

			for (int col = 0; col < 256; ++col)
			{
				if (m_maxHeight[col] < m_solidHeight[col])
					m_maxHeight[col] = m_solidHeight[col];
			}
		}

		void ChunkColumnBuffer::EncodeChunkV2(std::string& outStream)
		{
			FinalizeHeights(nullptr);

			std::vector<const SectionData*> active;
			active.reserve(m_activeSectionCount);
			for (int i = 0; i < m_activeSectionCount; ++i)
			{
				const SectionData* sec = m_sectionPool[i].get();
				if (sec && sec->nonAirCount > 0)
					active.push_back(sec);
			}
			std::sort(active.begin(), active.end(), [](const SectionData* a, const SectionData* b) {
				return a->sectionY < b->sectionY;
			});

			StringBuilder builder(4096);
			builder.append("chunkV2", 7);
			const int nChunkSizeLocation = builder.size();
			builder.appendBinary(static_cast<uint32>(0));

			for (int z = 0; z < 16; ++z)
			{
				for (int x = 0; x < 16; ++x)
				{
					const int col = (z << 4) + x;
					builder.appendBinary(static_cast<uint16_t>(m_solidHeight[col]));
					builder.appendBinary(static_cast<uint16_t>(m_maxHeight[col]));
				}
			}

			CSameIntegerEncoder<uint16_t> idEncoder(&builder);
			CSameIntegerEncoder<uint32_t> dataEncoder(&builder);

			for (const SectionData* sec : active)
			{
				builder.appendBinary(static_cast<uint32>(static_cast<int32_t>(sec->sectionY)));
				builder.appendBinary(static_cast<uint32>(sec->nonAirCount));

				idEncoder.Reset();
				for (int i = 0; i < 4096; ++i)
					idEncoder.Append(sec->ids[i]);
				idEncoder.Finalize();

				dataEncoder.Reset();
				for (int i = 0; i < 4096; ++i)
					dataEncoder.Append(sec->data[i]);
				dataEncoder.Finalize();
			}

			builder.WriteAt(nChunkSizeLocation, static_cast<uint32_t>(builder.length() - nChunkSizeLocation - 4));
			outStream.assign(builder.c_str(), builder.length());
		}

		bool ChunkColumnBuffer::CommitToRegion(CBlockWorld* world, BlockRegion* region, uint16_t chunkX, uint16_t chunkZ)
		{
			if (!world || !region || region->IsLocked())
				return false;

			FinalizeHeights(world);

			const uint16_t chunkX_rs = chunkX & 0x1fu;
			const uint16_t chunkZ_rs = chunkZ & 0x1fu;

			std::vector<const SectionData*> active;
			active.reserve(m_activeSectionCount);
			for (int i = 0; i < m_activeSectionCount; ++i)
			{
				const SectionData* sec = m_sectionPool[i].get();
				if (sec && sec->nonAirCount > 0)
					active.push_back(sec);
			}
			std::sort(active.begin(), active.end(), [](const SectionData* a, const SectionData* b) {
				return a->sectionY < b->sectionY;
			});

			const bool wasLightSuspended = world->IsLightUpdateSuspended();
			if (!wasLightSuspended)
				world->SuspendLightUpdate();

			int modifiedCount = 0;
			for (const SectionData* sec : active)
			{
				const int32_t packedChunkId_rs = PackChunkIndex(chunkX_rs, sec->sectionY, chunkZ_rs);
				BlockChunk* pChunk = region->GetChunk(packedChunkId_rs, true);
				if (!pChunk)
					continue;

				if (!pChunk->HasBlocks())
				{
					pChunk->ReserveBlocks(sec->nonAirCount);
					for (uint32_t i = 0; i < 4096; ++i)
					{
						const uint16_t id = sec->ids[i];
						if (id == 0) continue;
						BlockTemplate* pTemplate = world->GetBlockTemplate(id);
						if (!pTemplate) continue;
						pChunk->LoadBlock(static_cast<uint16_t>(i), pTemplate);
						if (sec->data[i] != 0)
							pChunk->SetBlockData(static_cast<uint16_t>(i), sec->data[i]);
						++modifiedCount;
					}
				}
				else
				{
					const uint16_t baseRx = chunkX_rs << 4;
					const int32_t baseRy = static_cast<int32_t>(sec->sectionY) * 16;
					const uint16_t baseRz = chunkZ_rs << 4;
					for (uint32_t i = 0; i < 4096; ++i)
					{
						const uint16_t id = sec->ids[i];
						if (id == 0) continue;
						BlockTemplate* pTemplate = world->GetBlockTemplate(id);
						if (!pTemplate) continue;
						uint16_t lx, ly, lz;
						UnpackBlockIndex(static_cast<uint16_t>(i), lx, ly, lz);
						region->SetBlockTemplateByIndex(baseRx + lx, baseRy + ly, baseRz + lz, pTemplate);
						if (sec->data[i] != 0)
							region->SetBlockUserDataByIndex(baseRx + lx, baseRy + ly, baseRz + lz, sec->data[i]);
						++modifiedCount;
					}
				}
			}

			for (int z = 0; z < 16; ++z)
			{
				for (int x = 0; x < 16; ++x)
				{
					const int col = (z << 4) + x;
					region->SetColumnHeight(chunkX_rs * 16 + x, chunkZ_rs * 16 + z,
						ChunkMaxHeight(m_solidHeight[col], m_maxHeight[col]));
				}
			}

			if (modifiedCount > 0 || region->GetChunkColumnTimeStamp(chunkX_rs << 4, chunkZ_rs << 4) <= 1)
			{
				region->SetChunkColumnTimeStamp(chunkX_rs << 4, chunkZ_rs << 4, 2);
				region->SetModified(true);
				if (!world->RefreshChunkColumn(chunkX, chunkZ))
					world->GetLightGrid().SetColumnUnloaded(chunkX, chunkZ);
			}

			if (!wasLightSuspended)
				world->ResumeLightUpdate();

			return true;
		}

		// =========================================================================
		// ChunkGeneratorFast
		// =========================================================================

		void ChunkGeneratorFast::SetProfile(int profileId, std::shared_ptr<const WorldGenProfile> profile)
		{
#ifndef EMSCRIPTEN_SINGLE_THREAD
			std::lock_guard<std::mutex> lock(g_profileMutex);
#endif
			GetProfileMap()[profileId] = std::move(profile);
		}

		std::shared_ptr<const WorldGenProfile> ChunkGeneratorFast::GetProfile(int profileId)
		{
#ifndef EMSCRIPTEN_SINGLE_THREAD
			std::lock_guard<std::mutex> lock(g_profileMutex);
#endif
			auto& map = GetProfileMap();
			auto it = map.find(profileId);
			return it != map.end() ? it->second : nullptr;
		}

		void ChunkGeneratorFast::ClearProfiles()
		{
#ifndef EMSCRIPTEN_SINGLE_THREAD
			std::lock_guard<std::mutex> lock(g_profileMutex);
#endif
			GetProfileMap().clear();
		}

		ChunkColumnBuffer& ChunkGeneratorFast::GetThreadBuffer()
		{
			thread_local ChunkColumnBuffer s_threadBuffer;
			return s_threadBuffer;
		}

		WorldColumnSample ChunkGeneratorFast::SampleWorldColumn(const WorldGenProfile& profile, int32_t wx, int32_t wz)
		{
			ColumnMacro2D m;
			EvaluateMacro2D(profile, wx, wz, m);

			ColumnMacro2D mx, mz;
			EvaluateMacro2D(profile, wx + 2, wz, mx);
			EvaluateMacro2D(profile, wx, wz + 2, mz);

			WorldColumnSample sample;
			sample.surfaceY = static_cast<int16_t>(std::round(m.targetY));
			sample.slopeGrad = static_cast<uint8_t>(std::min<int>(15, static_cast<int>(
				std::round(0.5f * (std::abs(mx.targetY - m.targetY) + std::abs(mz.targetY - m.targetY))))));

			if (!profile.biomes.empty())
			{
				const BiomeDef& b = profile.biomes[m.biomeIdx];
				sample.biomeId = b.id;
				sample.surfaceBlockId = (sample.surfaceY < profile.seaLevel) ? b.underwaterBlockId : b.topBlockId;
			}
			else
			{
				sample.biomeId = 1;
				sample.surfaceBlockId = (sample.surfaceY < profile.seaLevel) ? profile.blocks.sand : profile.blocks.grass;
			}

			sample.waterDepth = std::max<int16_t>(0, static_cast<int16_t>(profile.seaLevel - sample.surfaceY));
			sample.isRiver = m.riverFactor > 0.35f;
			sample.isSinkhole = m.sinkholeDepth > 0.25f;

			if (m.skyMaskUpper > 0.02f)
			{
				const int top = m.skyBaseUpper + static_cast<int>(std::round(6.0f * m.skyMaskUpper));
				const int bot = m.skyBaseUpper - static_cast<int>(std::round(28.0f * std::pow(m.skyMaskUpper, 1.5f)));
				sample.skyTopY = static_cast<int16_t>(top);
				sample.skyBottomY = static_cast<int16_t>(bot);
			}
			else if (m.skyMaskLower > 0.02f)
			{
				const int top = m.skyBaseLower + static_cast<int>(std::round(5.0f * m.skyMaskLower));
				const int bot = m.skyBaseLower - static_cast<int>(std::round(22.0f * std::pow(m.skyMaskLower, 1.5f)));
				sample.skyTopY = static_cast<int16_t>(top);
				sample.skyBottomY = static_cast<int16_t>(bot);
			}

			if (profile.presetMode == Preset_LushCavern)
			{
				const float floorN = ProceduralNoise::Fbm2D(profile.seed + 2203, wx * 0.008, wz * 0.008, 3, 2.0, 0.5);
				sample.caveFloorY = static_cast<int16_t>(std::round(-64.0f + floorN * 26.0f));
				sample.caveCeilY = static_cast<int16_t>(sample.caveFloorY + 68);
			}

			return sample;
		}

		void ChunkGeneratorFast::GenerateBaseColumn(const WorldGenProfile& profile, int32_t chunkX, int32_t chunkZ, ChunkColumnBuffer& outBuffer)
		{
			outBuffer.Reset(chunkX, chunkZ);

			const int32_t baseWx = chunkX * 16;
			const int32_t baseWz = chunkZ * 16;
			const int32_t seed = profile.seed;
			const int16_t seaLevel = profile.seaLevel;
			const int16_t bedrockY = std::max<int16_t>(BlockConfig::MinBlockY, profile.bedrockY);
			const int16_t lavaLevel = std::max<int16_t>(static_cast<int16_t>(bedrockY + 2), profile.lavaLevel);

			// 1. Evaluate 18x18 (with 1-block border) 2D macro grid for exact surface heights & slope gradients
			std::array<ColumnMacro2D, 18 * 18> macroGrid{};
			int minTargetY = 30000;
			int maxTargetY = -30000;
			bool hasUpperSky = false;
			bool hasLowerSky = false;
			int minUpperSkyY = 30000, maxUpperSkyY = -30000;
			int minLowerSkyY = 30000, maxLowerSkyY = -30000;

			for (int gz = -1; gz <= 16; ++gz)
			{
				for (int gx = -1; gx <= 16; ++gx)
				{
					ColumnMacro2D& cell = macroGrid[(gz + 1) * 18 + (gx + 1)];
					EvaluateMacro2D(profile, baseWx + gx, baseWz + gz, cell);
					if (gx >= 0 && gx < 16 && gz >= 0 && gz < 16)
					{
						const int iy = static_cast<int>(std::floor(cell.targetY));
						minTargetY = std::min(minTargetY, iy);
						maxTargetY = std::max(maxTargetY, iy);

						if (cell.skyMaskUpper > 0.01f)
						{
							hasUpperSky = true;
							minUpperSkyY = std::min<int>(minUpperSkyY, cell.skyBaseUpper - 48);
							maxUpperSkyY = std::max<int>(maxUpperSkyY, cell.skyBaseUpper + 18);
						}
						if (cell.skyMaskLower > 0.01f)
						{
							hasLowerSky = true;
							minLowerSkyY = std::min<int>(minLowerSkyY, cell.skyBaseLower - 40);
							maxLowerSkyY = std::max<int>(maxLowerSkyY, cell.skyBaseLower + 16);
						}
					}
				}
			}

			for (int lz = 0; lz < 16; ++lz)
			{
				for (int lx = 0; lx < 16; ++lx)
				{
					const ColumnMacro2D& c = macroGrid[(lz + 1) * 18 + (lx + 1)];
					const ColumnMacro2D& cxP = macroGrid[(lz + 1) * 18 + (lx + 2)];
					const ColumnMacro2D& cxN = macroGrid[(lz + 1) * 18 + lx];
					const ColumnMacro2D& czP = macroGrid[(lz + 2) * 18 + (lx + 1)];
					const ColumnMacro2D& czN = macroGrid[lz * 18 + (lx + 1)];
					const float grad = 0.5f * (std::max(std::abs(cxP.targetY - cxN.targetY), std::abs(czP.targetY - czN.targetY)));
					const int col = (lz << 4) + lx;
					outBuffer.m_slopeGrad[col] = static_cast<uint8_t>(std::min<int>(15, static_cast<int>(std::round(grad))));
					outBuffer.m_biomeId[col] = !profile.biomes.empty() ? profile.biomes[c.biomeIdx].id : 1;
				}
			}

			// 2. Active Surface + 3D Overhang/Arch Band (coarse 5 x Ny x 5 grid, step 4 x 8 x 4)
			const int overhangPad = (profile.overhangStrength > 0.05f) ? 32 : 8;
			const int surfBandMinY = std::max<int>(bedrockY + 4, ((minTargetY - 24) / 8) * 8);
			const int surfBandMaxY = std::min<int>(BlockConfig::MaxBlockY - 16, ((maxTargetY + overhangPad + 15) / 8) * 8);
			const int surfStepsY = std::max(1, (surfBandMaxY - surfBandMinY) / 8);

			std::vector<float> surfDensity((surfStepsY + 1) * 25, 0.0f);
			if (profile.overhangStrength > 0.01f)
			{
				for (int gy = 0; gy <= surfStepsY; ++gy)
				{
					const double wy = static_cast<double>(surfBandMinY + gy * 8);
					for (int gz = 0; gz <= 4; ++gz)
					{
						const int sampleZ = std::min(15, gz * 4);
						const double wz = static_cast<double>(baseWz + gz * 4);
						for (int gx = 0; gx <= 4; ++gx)
						{
							const int sampleX = std::min(15, gx * 4);
							const double wx = static_cast<double>(baseWx + gx * 4);
							const ColumnMacro2D& mc = macroGrid[(sampleZ + 1) * 18 + (sampleX + 1)];

							float n3 = ProceduralNoise::Fbm3D(seed + 1601, wx * 0.021, wy * 0.024, wz * 0.021, 3, 2.0, 0.5);
							const float amp = (8.0f + 22.0f * (mc.karstFactor + 0.5f * mc.ridge)) * profile.overhangStrength * (1.0f - mc.riverFactor);

							// Natural karst/mountain arches when high ridge & specific 3D ribbon align
							float archBonus = 0.0f;
							if ((mc.karstFactor > 0.35f || mc.canyonFactor > 0.45f) && wy > seaLevel + 16 && wy < mc.targetY + 26.0f)
							{
								const float archWave = ProceduralNoise::Noise2D(seed + 1613, wx * 0.018, wz * 0.018);
								if (archWave > 0.52f)
								{
									const float archCenterY = mc.targetY + 14.0f;
									const float distFromSpan = std::abs(static_cast<float>(wy) - archCenterY);
									if (distFromSpan < 5.0f)
										archBonus = (5.0f - distFromSpan) * 4.5f;
									else if (wy < archCenterY - 5.0f && wy > archCenterY - 22.0f)
										archBonus = -26.0f; // hollow under the stone arch!
								}
							}

							surfDensity[(gy * 25) + gz * 5 + gx] = n3 * amp + archBonus;
						}
					}
				}
			}

			// Fill solid foundation below surfBandMinY down to bedrockY first, then carve caves cleanly
			for (int lz = 0; lz < 16; ++lz)
			{
				for (int lx = 0; lx < 16; ++lx)
				{
					const int wx = baseWx + lx;
					const int wz = baseWz + lz;
					outBuffer.SetBlock(lx, bedrockY, lz, profile.blocks.bedrock);
					if (surfBandMinY - 1 >= bedrockY + 1)
						outBuffer.FillColumnRange(lx, lz, bedrockY + 1, surfBandMinY - 1, profile.blocks.stone, 0);

					// Bedrock roughness (2-3 blocks)
					const uint32_t brH = ProceduralNoise::Hash2D(seed + 17, wx, wz);
					if ((brH & 1u) != 0u) outBuffer.SetBlock(lx, bedrockY + 1, lz, profile.blocks.bedrock);
					if ((brH & 6u) == 0u) outBuffer.SetBlock(lx, bedrockY + 2, lz, profile.blocks.bedrock);
				}
			}

			// Rasterize Surface + Overhang Band [surfBandMinY .. surfBandMaxY] top-down per column
			BiomeDef defaultBiome{};
			const size_t strataCount = profile.strata.size();

			for (int lz = 0; lz < 16; ++lz)
			{
				const int gz = lz >> 2;
				const float tz = (lz & 3) * 0.25f;

				for (int lx = 0; lx < 16; ++lx)
				{
					const int gx = lx >> 2;
					const float tx = (lx & 3) * 0.25f;
					const int col = (lz << 4) + lx;
					const int wx = baseWx + lx;
					const int wz = baseWz + lz;

					const ColumnMacro2D& mc = macroGrid[(lz + 1) * 18 + (lx + 1)];
					const BiomeDef& biome = !profile.biomes.empty() ? profile.biomes[mc.biomeIdx] : defaultBiome;
					const uint8_t slope = outBuffer.m_slopeGrad[col];
					const int strataOffset = static_cast<int>(std::round(
						ProceduralNoise::Noise2D(seed + 1709, wx * 0.015, wz * 0.015) * 4.0f));

					int runDepth = 0;
					int16_t topSolidY = surfBandMinY - 1;

					for (int y = surfBandMaxY; y >= surfBandMinY; --y)
					{
						float densityOffset = 0.0f;
						if (profile.overhangStrength > 0.01f)
						{
							const int gy = std::min(surfStepsY - 1, (y - surfBandMinY) >> 3);
							const float ty = ((y - surfBandMinY) - gy * 8) * 0.125f;
							const int b0 = gy * 25 + gz * 5 + gx;
							const int b1 = (gy + 1) * 25 + gz * 5 + gx;
							densityOffset = TriLerp(tx, ty, tz,
								surfDensity[b0],     surfDensity[b0 + 1],     surfDensity[b1],     surfDensity[b1 + 1],
								surfDensity[b0 + 5], surfDensity[b0 + 6],     surfDensity[b1 + 5], surfDensity[b1 + 6]);
						}

						const float dens = (mc.targetY - static_cast<float>(y)) + densityOffset;
						if (dens > 0.0f)
						{
							if (topSolidY < y)
								topSolidY = static_cast<int16_t>(y);

							uint16_t blkId = profile.blocks.stone;
							uint32_t blkData = 0;

							const bool isSnowPeak = (y >= profile.snowLineY + static_cast<int>((ProceduralNoise::Hash2D(seed + 31, wx, wz) & 7u)) - 3)
								&& biome.specialFlag != 5 && !biome.useStrata;
							const bool isSteepCliff = slope >= 3 && runDepth <= 5;

							if ((biome.useStrata || mc.canyonFactor > 0.45f) && strataCount > 0 && runDepth <= 28)
							{
								const int idx = ((y + strataOffset) % static_cast<int>(strataCount) + static_cast<int>(strataCount)) % static_cast<int>(strataCount);
								blkId = profile.strata[idx].blockId;
								blkData = profile.strata[idx].blockData;
							}
							else if (runDepth == 0)
							{
								if (y < seaLevel - 1)
								{
									blkId = biome.underwaterBlockId;
									blkData = biome.underwaterBlockData;
								}
								else if (y <= seaLevel + 1 && mc.cont < 0.08f && biome.specialFlag != 2 && biome.specialFlag != 4)
								{
									blkId = profile.blocks.sand;
									blkData = 0;
								}
								else if (isSnowPeak)
								{
									blkId = (slope >= 4) ? profile.blocks.stone : ((biome.specialFlag == 4 && y > profile.snowLineY + 28) ? profile.blocks.ice : profile.blocks.snow);
								}
								else if (isSteepCliff)
								{
									blkId = biome.cliffBlockId;
									blkData = biome.cliffBlockData;
								}
								else
								{
									blkId = biome.topBlockId;
									blkData = biome.topBlockData;
								}
							}
							else if (runDepth <= biome.fillerDepth)
							{
								if (isSnowPeak && slope < 3)
								{
									blkId = (biome.specialFlag == 4) ? profile.blocks.brokenIce : profile.blocks.snowBlock;
								}
								else if (isSteepCliff)
								{
									blkId = biome.cliffBlockId;
									blkData = biome.cliffBlockData;
								}
								else
								{
									blkId = biome.fillerBlockId;
									blkData = biome.fillerBlockData;
								}
							}

							outBuffer.SetBlock(lx, y, lz, blkId, blkData);
							++runDepth;
						}
						else
						{
							runDepth = 0;
						}
					}

					outBuffer.m_surfaceY[col] = topSolidY;
					outBuffer.m_surfaceBlockId[col] = topSolidY >= bedrockY ? outBuffer.GetBlockId(lx, topSolidY, lz) : 0;

					// Fill Ocean / River Water up to seaLevel
					if (topSolidY < seaLevel && mc.sinkholeDepth < 0.35f)
					{
						outBuffer.FillColumnRange(lx, lz, topSolidY + 1, seaLevel, profile.blocks.water, 0, true);
						if (mc.temp < 0.18f && mc.riverFactor < 0.5f)
							outBuffer.SetBlock(lx, seaLevel, lz, profile.blocks.ice, 0, false, true);
						outBuffer.m_waterDepth[col] = static_cast<int16_t>(seaLevel - topSolidY);
					}
				}
			}

			// 3. Multi-Tier Cave & Abyssal Lush/Crystal Cavern Carving (4x4x4 coarse grid)
			if (profile.caveStrength > 0.01f)
			{
				const int caveMinY = bedrockY + 4;
				const int caveMaxY = (profile.presetMode == Preset_LushCavern)
					? std::min<int>(maxTargetY + 8, 220)
					: std::min<int>(maxTargetY - 4, 240);

				if (caveMaxY > caveMinY + 8)
				{
					const int cBaseY = (caveMinY / 4) * 4;
					const int cStepsY = (caveMaxY - cBaseY) / 4 + 1;
					std::vector<float> caveGrid((cStepsY + 1) * 25, -1.0f);

					for (int gy = 0; gy <= cStepsY; ++gy)
					{
						const int wy = cBaseY + gy * 4;
						for (int gz = 0; gz <= 4; ++gz)
						{
							const int sampleZ = std::min(15, gz * 4);
							const double wz = static_cast<double>(baseWz + gz * 4);
							for (int gx = 0; gx <= 4; ++gx)
							{
								const int sampleX = std::min(15, gx * 4);
								const double wx = static_cast<double>(baseWx + gx * 4);
								const ColumnMacro2D& mc = macroGrid[(sampleZ + 1) * 18 + (sampleX + 1)];

								float carve = -1.0f;

								if (profile.presetMode == Preset_LushCavern)
								{
									// Cathedral multi-hundred-meter vault between Y = -180 and Y = +130
									const float floorY = -72.0f + ProceduralNoise::Fbm2D(seed + 2203, wx * 0.006, wz * 0.006, 3, 2.0, 0.5) * 34.0f;
									const float ceilY = 64.0f + ProceduralNoise::Fbm2D(seed + 2207, wx * 0.006, wz * 0.006, 3, 2.0, 0.5) * 42.0f;
									const float pNoise = ProceduralNoise::Fbm3D(seed + 2221, wx * 0.018, wy * 0.016, wz * 0.018, 2, 2.0, 0.5);
									float f1 = 1.0f;
									ProceduralNoise::Cellular2D(seed + 2213, wx * 0.011, wz * 0.011, 0.85f, &f1, nullptr, nullptr);
									const float isSupportPillar = (f1 < 0.14f) ? 1.0f : 0.0f;

									if (wy > floorY && wy < ceilY && isSupportPillar < 0.5f)
									{
										const float midDist = std::min(static_cast<float>(wy) - floorY, ceilY - static_cast<float>(wy));
										carve = (midDist * 0.08f + pNoise * 0.45f) * profile.caveStrength;
									}
								}

								// Winding spaghetti tunnels connecting surface to abyss
								const float s1 = ProceduralNoise::Noise3D(seed + 2003, wx * 0.022, wy * 0.026, wz * 0.022);
								const float s2 = ProceduralNoise::Noise3D(seed + 2011, wx * 0.022 + 31.7, wy * 0.026 - 14.3, wz * 0.022 + 9.2);
								const float spaghetti = 0.048f * profile.caveStrength - (s1 * s1 + s2 * s2);
								carve = std::max(carve, spaghetti * 8.0f);

								// Deep Abyssal Cheese Caverns (Y in [deepCavernMinY .. deepCavernMaxY])
								if (wy >= profile.deepCavernMinY && wy <= profile.deepCavernMaxY + 24)
								{
									const float bandEnv = SmoothStep(static_cast<float>(profile.deepCavernMinY), static_cast<float>(profile.deepCavernMinY + 28), static_cast<float>(wy))
										* SmoothStep(static_cast<float>(profile.deepCavernMaxY + 24), static_cast<float>(profile.deepCavernMaxY - 12), static_cast<float>(wy));
									const float cheese = ProceduralNoise::Fbm3D(seed + 2111, wx * 0.014, wy * 0.020, wz * 0.014, 3, 2.0, 0.5);
									const float cavernCarve = (cheese - 0.24f) * bandEnv * 2.5f * profile.caveStrength;
									carve = std::max(carve, cavernCarve);
								}

								// Tiankeng vertical skylight shaft plunging straight down to Y = -110!
								if (mc.sinkholeDepth > 0.05f && wy > -120)
								{
									const float shaftFade = SmoothStep(-128.0f, -64.0f, static_cast<float>(wy));
									carve = std::max(carve, mc.sinkholeDepth * shaftFade * 2.0f);
								}

								// Protect ocean/river beds from draining unless inside an explicit Tiankeng shaft
								if (mc.sinkholeDepth <= 0.05f && (mc.targetY <= seaLevel + 2.0f || mc.riverFactor > 0.2f))
								{
									if (wy > mc.targetY - 9.0f && wy <= seaLevel + 2)
										carve = -1.0f;
								}

								caveGrid[gy * 25 + gz * 5 + gx] = carve;
							}
						}
					}

					for (int lz = 0; lz < 16; ++lz)
					{
						const int gz = lz >> 2;
						const float tz = (lz & 3) * 0.25f;
						for (int lx = 0; lx < 16; ++lx)
						{
							const int gx = lx >> 2;
							const float tx = (lx & 3) * 0.25f;
							const int col = (lz << 4) + lx;
							const ColumnMacro2D& mc = macroGrid[(lz + 1) * 18 + (lx + 1)];

							const int colMaxY = (mc.sinkholeDepth > 0.2f)
								? static_cast<int>(outBuffer.m_surfaceY[col])
								: std::min<int>(caveMaxY, outBuffer.m_surfaceY[col] - (outBuffer.m_waterDepth[col] > 0 ? 8 : 0));

							bool prevWasAir = false;
							int16_t bestFloorY = ChunkColumnBuffer::kEmptyY;
							int16_t bestCeilY = ChunkColumnBuffer::kEmptyY;
							int currentSpanCeil = ChunkColumnBuffer::kEmptyY;

							for (int y = colMaxY; y >= caveMinY; --y)
							{
								const int gy = std::clamp((y - cBaseY) >> 2, 0, cStepsY - 1);
								const float ty = ((y - cBaseY) - gy * 4) * 0.25f;
								const int b0 = gy * 25 + gz * 5 + gx;
								const int b1 = (gy + 1) * 25 + gz * 5 + gx;
								const float cVal = TriLerp(tx, ty, tz,
									caveGrid[b0],     caveGrid[b0 + 1],     caveGrid[b1],     caveGrid[b1 + 1],
									caveGrid[b0 + 5], caveGrid[b0 + 6],     caveGrid[b1 + 5], caveGrid[b1 + 6]);

								if (cVal > 0.0f)
								{
									if (!prevWasAir)
										currentSpanCeil = y + 1;
									prevWasAir = true;

									if (y <= lavaLevel)
									{
										outBuffer.SetBlock(lx, y, lz, (y == lavaLevel && ((lx + lz) & 7) == 0)
											? profile.blocks.obsidian : profile.blocks.lava);
									}
									else
									{
										outBuffer.SetBlock(lx, y, lz, 0);
									}
								}
								else
								{
									if (prevWasAir && y > lavaLevel)
									{
										const int spanH = currentSpanCeil - y;
										if (spanH >= 5 && (bestFloorY == ChunkColumnBuffer::kEmptyY || y < -20))
										{
											bestFloorY = static_cast<int16_t>(y);
											bestCeilY = static_cast<int16_t>(currentSpanCeil);
										}
									}
									prevWasAir = false;
								}
							}

							if (bestFloorY > ChunkColumnBuffer::kEmptyY)
							{
								outBuffer.m_caveFloorY[col] = bestFloorY;
								outBuffer.m_caveCeilY[col] = bestCeilY;

								// Lush / Bioluminescent Cavern Floor dressing
								const uint32_t patchH = ProceduralNoise::Hash2D(seed + 2309, baseWx + lx, baseWz + lz);
								if (mc.moist > 0.48f || profile.presetMode == Preset_LushCavern)
								{
									const uint16_t floorBlock = (mc.temp < 0.35f) ? profile.blocks.mycelium
										: (((patchH & 3u) == 0u) ? profile.blocks.mossStone : profile.blocks.grass);
									outBuffer.SetBlock(lx, bestFloorY, lz, floorBlock, 0);
								}
								if (mc.sinkholeDepth > 0.25f)
								{
									outBuffer.m_surfaceY[col] = bestFloorY;
									outBuffer.m_surfaceBlockId[col] = outBuffer.GetBlockId(lx, bestFloorY, lz);
								}
							}
						}
					}
				}
			}

			// 4. Underground Ore Veins (sampled only on sparse 2x2x2 lattice inside stone)
			if (!profile.ores.empty())
			{
				for (size_t oi = 0; oi < profile.ores.size(); ++oi)
				{
					const OrePocketDef& ore = profile.ores[oi];
					const int oMinY = std::max<int>(bedrockY + 2, ore.minY);
					const int oMaxY = std::min<int>(maxTargetY - 3, ore.maxY);
					if (oMaxY <= oMinY) continue;

					for (int y = oMinY; y <= oMaxY; y += 3)
					{
						for (int lz = 1; lz < 15; lz += 3)
						{
							for (int lx = 1; lx < 15; lx += 3)
							{
								const uint32_t h = ProceduralNoise::Hash3D(seed + static_cast<int32_t>(oi * 797 + 3109),
									baseWx + lx, y, baseWz + lz);
								if (ProceduralNoise::HashToUnitFloat(h) > ore.threshold)
								{
									if (outBuffer.GetBlockId(lx, y, lz) == profile.blocks.stone)
									{
										outBuffer.SetBlock(lx, y, lz, ore.blockId, ore.blockData);
										if ((h & 16u) && outBuffer.GetBlockId(lx + 1, y, lz) == profile.blocks.stone)
											outBuffer.SetBlock(lx + 1, y, lz, ore.blockId, ore.blockData);
										if ((h & 32u) && outBuffer.GetBlockId(lx, y + 1, lz) == profile.blocks.stone)
											outBuffer.SetBlock(lx, y + 1, lz, ore.blockId, ore.blockData);
										if ((h & 64u) && outBuffer.GetBlockId(lx, y, lz + 1) == profile.blocks.stone)
											outBuffer.SetBlock(lx, y, lz + 1, ore.blockId, ore.blockData);
									}
								}
							}
						}
					}
				}
			}

			// 5. Floating Sky Islands & Plunging Waterfalls (zero cost when chunk has no sky islands!)
			auto buildSkyLayer = [&](bool upperLayer)
			{
				for (int lz = 0; lz < 16; ++lz)
				{
					for (int lx = 0; lx < 16; ++lx)
					{
						const int col = (lz << 4) + lx;
						const int wx = baseWx + lx;
						const int wz = baseWz + lz;
						const ColumnMacro2D& mc = macroGrid[(lz + 1) * 18 + (lx + 1)];

						const float mask = upperLayer ? mc.skyMaskUpper : mc.skyMaskLower;
						if (mask <= 0.01f)
							continue;

						const int baseY = upperLayer ? mc.skyBaseUpper : mc.skyBaseLower;
						const bool waterfall = upperLayer ? mc.skyWaterfallUpper : mc.skyWaterfallLower;

						const float topUndul = ProceduralNoise::Fbm2D(seed + (upperLayer ? 3301 : 3407), wx * 0.045, wz * 0.045, 2, 2.0, 0.5);
						const float cragUndul = ProceduralNoise::Ridged2D(seed + (upperLayer ? 3313 : 3413), wx * 0.065, wz * 0.065, 2, 2.0, 0.5, 1.0f);

						const int topY = baseY + static_cast<int>(std::round(mask * 7.0f + topUndul * 3.0f));
						const float maxThick = upperLayer ? 34.0f : 26.0f;
						const int botY = baseY - static_cast<int>(std::round(std::pow(mask, 1.45f) * maxThick + cragUndul * 6.0f * mask));

						if (topY <= botY)
							continue;

						// Stone crag underside + dirt + lush grass/snow top + rare glowing sky-crystal root tip!
						if (topY - 4 >= botY)
							outBuffer.FillColumnRange(lx, lz, botY, topY - 4, profile.blocks.stone, 0);
						outBuffer.FillColumnRange(lx, lz, std::max(botY, topY - 3), topY - 1, profile.blocks.dirt, 0);

						const uint16_t skySurfaceBlock = (topY > profile.snowLineY + 260)
							? profile.blocks.snow : profile.blocks.grass;
						outBuffer.SetBlock(lx, topY, lz, skySurfaceBlock, 0);

						if (mask > 0.72f && ((ProceduralNoise::Hash2D(seed + 3359, wx, wz) & 15u) == 0u))
						{
							outBuffer.SetBlock(lx, botY - 1, lz, profile.blocks.glowstone, 0);
						}

						if (topY > outBuffer.m_skyTopY[col])
						{
							outBuffer.m_skyTopY[col] = static_cast<int16_t>(topY);
							outBuffer.m_skyBottomY[col] = static_cast<int16_t>(botY);
						}

						// Plunging sky waterfall straight down to surface/sea so players can ride/swim up!
						if (waterfall)
						{
							const int landingY = std::max<int>(seaLevel, outBuffer.m_surfaceY[col] + 1);
							if (botY > landingY + 6)
							{
								outBuffer.SetBlock(lx, topY, lz, profile.blocks.water, 0, false, true);
								outBuffer.SetBlock(lx, topY - 1, lz, profile.blocks.glowstone, 0);
								outBuffer.FillColumnRange(lx, lz, landingY, botY - 1, profile.blocks.water, 0, true);
							}
						}
					}
				}
			};

			if (hasLowerSky) buildSkyLayer(false);
			if (hasUpperSky) buildSkyLayer(true);
		}
	}
}

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "BlockConfig.h"

namespace ParaEngine
{
	class CBlockWorld;
	class BlockRegion;

	namespace WorldGen
	{
		enum PresetMode : int32_t
		{
			Preset_EpicOverworld = 0,
			Preset_SkyIslands = 1,
			Preset_KarstCanyon = 2,
			Preset_LushCavern = 3,
		};

		enum TreeStampStyle : int32_t
		{
			Tree_Oak = 0,
			Tree_SprucePine = 1,
			Tree_CherryUmbrella = 2,
			Tree_GiantJungle = 3,
			Tree_SlenderMaple = 4,
			Tree_OasisPalm = 5,
			Tree_GiantMushroom = 6,
		};

		struct BiomeDef
		{
			uint8_t id = 1;
			uint8_t specialFlag = 0; // 0=standard, 1=ocean, 2=karst, 3=danxia_canyon, 4=glacier, 5=desert, 6=lush_swamp
			bool useStrata = false;
			float minTemp = 0.0f;
			float maxTemp = 1.0f;
			float minMoist = 0.0f;
			float maxMoist = 1.0f;
			float heightBias = 0.0f;
			float heightScale = 1.0f;
			uint16_t topBlockId = 62;       // Grass
			uint32_t topBlockData = 0;
			uint16_t fillerBlockId = 55;    // Dirt
			uint32_t fillerBlockData = 0;
			uint16_t cliffBlockId = 56;     // Stone
			uint32_t cliffBlockData = 0;
			uint16_t underwaterBlockId = 51;// Sand
			uint32_t underwaterBlockData = 0;
			int16_t fillerDepth = 4;
		};

		struct StrataBand
		{
			uint16_t blockId = 4; // Sandstone / ColorBlock
			uint32_t blockData = 0;
		};

		struct OrePocketDef
		{
			uint16_t blockId = 125;
			uint32_t blockData = 0;
			int16_t minY = -240;
			int16_t maxY = 128;
			float frequency = 0.09f;
			float threshold = 0.72f;
		};

		struct WorldGenBlockPalette
		{
			uint16_t bedrock = 123;
			uint16_t stone = 56;
			uint16_t dirt = 55;
			uint16_t grass = 62;
			uint16_t sand = 51;
			uint16_t sandstone = 4;
			uint16_t clay = 53;
			uint16_t gravel = 12;
			uint16_t snow = 52;
			uint16_t snowBlock = 5;
			uint16_t ice = 17;
			uint16_t brokenIce = 28;
			uint16_t mossStone = 145;
			uint16_t obsidian = 146;
			uint16_t netherrack = 150;
			uint16_t glowstone = 87;
			uint16_t water = 76; // Still_Water
			uint16_t lava = 82;
			uint16_t mycelium = 153;
			uint16_t colorBlock = 10;
			uint16_t transparentColorBlock = 73;
		};

		struct WorldGenProfile
		{
			int32_t seed = 12345;
			int32_t presetMode = Preset_EpicOverworld;
			int16_t seaLevel = 64;
			int16_t bedrockY = -240;
			int16_t lavaLevel = -216;
			int16_t snowLineY = 230;

			float baseHeight = 68.0f;
			float continentalFreq = 0.00075f;
			float mountainHeight = 310.0f;
			float peakBoost = 210.0f;
			float karstStrength = 1.0f;
			float canyonStrength = 1.0f;
			float riverStrength = 1.0f;
			float overhangStrength = 1.0f;

			float skyIslandChance = 0.22f;
			int16_t skyIslandMinY = 260;
			int16_t skyIslandMaxY = 640;
			float skyWaterfallChance = 0.35f;

			float caveStrength = 1.0f;
			float sinkholeChance = 0.25f;
			int16_t deepCavernMinY = -210;
			int16_t deepCavernMaxY = -36;

			WorldGenBlockPalette blocks;
			std::vector<BiomeDef> biomes;
			std::vector<StrataBand> strata;
			std::vector<OrePocketDef> ores;
		};

		struct WorldColumnSample
		{
			int16_t surfaceY = 64;
			uint16_t surfaceBlockId = 62;
			uint8_t biomeId = 1;
			uint8_t slopeGrad = 0;
			int16_t skyTopY = BlockConfig::MinBlockY - 1;
			int16_t skyBottomY = BlockConfig::MinBlockY - 1;
			int16_t caveFloorY = BlockConfig::MinBlockY - 1;
			int16_t caveCeilY = BlockConfig::MinBlockY - 1;
			int16_t waterDepth = 0;
			bool isRiver = false;
			bool isSinkhole = false;
		};

		/**
		 * Reusable per-thread sparse column buffer spanning Y in [MinBlockY, MaxBlockY].
		 * Avoids heap churn across chunks and supports zero-copy commit or fast chunkV2 encoding.
		 */
		class ChunkColumnBuffer
		{
		public:
			static constexpr int16_t kEmptyY = BlockConfig::MinBlockY - 1;
			static constexpr int kSectionCount = BlockConfig::MaxSectionY - BlockConfig::MinSectionY + 1;

			struct SectionData
			{
				int16_t sectionY = 0;
				uint16_t nonAirCount = 0;
				std::array<uint16_t, 4096> ids{};
				std::array<uint32_t, 4096> data{};

				void Clear(int16_t sy) noexcept
				{
					sectionY = sy;
					nonAirCount = 0;
					ids.fill(0);
					data.fill(0);
				}
			};

			ChunkColumnBuffer();

			void Reset(int32_t chunkX, int32_t chunkZ);

			int32_t GetChunkX() const noexcept { return m_chunkX; }
			int32_t GetChunkZ() const noexcept { return m_chunkZ; }

			/** Safe clipped voxel write in chunk-local XZ [0..15] and world Y [MinBlockY..MaxBlockY]. */
			void SetBlock(int lx, int wy, int lz, uint16_t blockId, uint32_t blockData = 0, bool onlyIfAir = false, bool isTransparentHint = false);

			uint16_t GetBlockId(int lx, int wy, int lz) const noexcept;
			uint32_t GetBlockData(int lx, int wy, int lz) const noexcept;

			void FillColumnRange(int lx, int lz, int yStart, int yEnd, uint16_t blockId, uint32_t blockData = 0, bool isTransparentHint = false);
			void FillBox(int x1, int y1, int z1, int x2, int y2, int z2, uint16_t blockId, uint32_t blockData = 0, int mode = 0);
			void FillEllipsoid(float cx, float cy, float cz, float rx, float ry, float rz, uint16_t blockId, uint32_t blockData = 0, bool onlyIfAir = false);

			void StampTree(int lx, int wy, int lz, int style, uint16_t trunkId, uint32_t trunkData,
				uint16_t leafId, uint32_t leafData, uint16_t extraId, uint32_t extraData, int height, int radius, uint32_t seed);

			void StampCrystalCluster(int lx, int wy, int lz, uint16_t coreId, uint16_t shellId, uint32_t shellData,
				int height, int radius, bool pointUp, uint32_t seed);

			/** Recompute exact solidHeight / maxHeight across all 256 columns before encoding/commit. */
			void FinalizeHeights(CBlockWorld* world = nullptr);

			/** Encode non-empty sparse sections + 256 ChunkMaxHeight pairs into 'chunkV2' binary string. */
			void EncodeChunkV2(std::string& outStream);

			/** Direct main-thread commit into a BlockRegion without binary serialization. */
			bool CommitToRegion(CBlockWorld* world, BlockRegion* region, uint16_t chunkX, uint16_t chunkZ);

			int GetActiveSectionCount() const noexcept { return m_activeSectionCount; }

		public:
			std::array<int16_t, 256> m_surfaceY{};
			std::array<uint16_t, 256> m_surfaceBlockId{};
			std::array<uint8_t, 256> m_biomeId{};
			std::array<uint8_t, 256> m_slopeGrad{};
			std::array<int16_t, 256> m_skyTopY{};
			std::array<int16_t, 256> m_skyBottomY{};
			std::array<int16_t, 256> m_caveFloorY{};
			std::array<int16_t, 256> m_caveCeilY{};
			std::array<int16_t, 256> m_waterDepth{};
			std::array<int16_t, 256> m_solidHeight{};
			std::array<int16_t, 256> m_maxHeight{};

		private:
			SectionData* GetOrCreateSection(int16_t sectionY);
			const SectionData* FindSection(int16_t sectionY) const noexcept;
			static bool IsDefaultTransparentBlock(uint16_t blockId) noexcept;

			int32_t m_chunkX = 0;
			int32_t m_chunkZ = 0;
			int m_activeSectionCount = 0;
			std::array<int16_t, kSectionCount> m_sectionLookup{};
			std::vector<std::unique_ptr<SectionData>> m_sectionPool;
		};

		class ChunkGeneratorFast
		{
		public:
			static void SetProfile(int profileId, std::shared_ptr<const WorldGenProfile> profile);
			static std::shared_ptr<const WorldGenProfile> GetProfile(int profileId);
			static void ClearProfiles();

			/** Access the current OS thread's reusable ChunkColumnBuffer. */
			static ChunkColumnBuffer& GetThreadBuffer();

			/** Deterministic O(1) column surface/biome/sky-island probe at any world block (wx, wz). */
			static WorldColumnSample SampleWorldColumn(const WorldGenProfile& profile, int32_t wx, int32_t wz);

			/** Generate base terrain, overhangs, rivers, caves, sinkholes, ores, and sky islands into outBuffer. */
			static void GenerateBaseColumn(const WorldGenProfile& profile, int32_t chunkX, int32_t chunkZ, ChunkColumnBuffer& outBuffer);
		};
	}
}

#include "ParaEngine.h"
#include "Core/ParaEngineAppBase.h"
#include "BlockEngine/BlockLightGridClient.h"
#include "Framework/Interface/Render/RHITypes.h"
#include "3dengine/SoftwareOcclusionBuffer.h"
#include "BlockEngine/BlockChunk.h"
#include "BlockEngine/BlockIndex.h"
#include "BlockEngine/BlockRegion.h"
#include "BlockEngine/BlockWorld.h"
#include "BlockEngine/BlockTemplate.h"
#include "BlockEngine/BlockDirection.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/RenderableChunk.h"
#include "renderer/ParaVertexBuffer.h"
#include "renderer/RenderDevice.h"
#include "renderer/ParaVertexBufferPool.h"
#include "renderer/VertexFVF.h"
#include "ShapeAABB.h"
#include "3dengine/ParaEngineSettings.h"
#include "IParaWebXR.h"
#include "2dengine/GUIEdit.h"
#include "util/NetworkAdapter.h"
#include "util/StringHelper.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "Framework/Common/PlatformBridge/PlatformBridge.h"
#include "Framework/FileSystem/ParaFileUtils.h"
#include "NPL/NPLScriptingState.h"
#include "ParaScriptBindings/ParaScriptingGlobal.h"
#include "IParaWebView.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/particle.h"
#include "Core/AttributeClass.h"
#include "RHIStateTracker.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <boost/filesystem.hpp>

using namespace ParaEngine;

namespace
{
	int g_passed = 0;
	int g_failed = 0;

	void Check(bool cond, const char* expr, const char* file, int line)
	{
		if (cond)
		{
			++g_passed;
		}
		else
		{
			++g_failed;
			std::fprintf(stderr, "[FAIL] %s:%d: %s\n", file, line, expr);
		}
	}

#define TEST_CHECK(expr) Check((expr), #expr, __FILE__, __LINE__)

	Matrix4 BuildTestPerspectiveLH(float fovY, float aspect, float zn, float zf, bool zeroToOneClipZ)
	{
		const float yScale = 1.0f / std::tan(fovY * 0.5f);
		const float xScale = yScale / aspect;
		Matrix4 m;
		m.identity();
		m._11 = xScale;
		m._22 = yScale;
		if (zeroToOneClipZ)
		{
			m._33 = zf / (zf - zn);
			m._34 = 1.0f;
			m._43 = -zn * zf / (zf - zn);
			m._44 = 0.0f;
		}
		else
		{
			// OpenGL [-1, 1] clip Z with +w row-vector convention
			m._33 = (zf + zn) / (zf - zn);
			m._34 = 1.0f;
			m._43 = -2.0f * zn * zf / (zf - zn);
			m._44 = 0.0f;
		}
		return m;
	}

	void TestAnimationKeyIntervals()
	{
		Animated<float> rotationWeight;
		rotationWeight.times = {0, 4, 8, 12, 16, 20, 24, 28};
		rotationWeight.data = {0, 8, 16, 24, 32, 40, 48, 56};
		rotationWeight.ranges = {{0, 7}, {3, 5}};
		Animated<float> opacity;
		opacity.times = {0, 1, 2, 3, 4, 4, 8};
		opacity.data = {0, 1, 2, 3, 4, 12, 20};
		opacity.ranges = {{0, 6}};

		// Seek backwards, switch tracks with different time grids and select a subrange.
		TEST_CHECK(rotationWeight.getValue(0, 26) == 52.f);
		TEST_CHECK(opacity.getValue(0, 4) == 12.f);
		TEST_CHECK(rotationWeight.getValue(1, 14) == 28.f);
		TEST_CHECK(opacity.getValue(0, 6) == 16.f);
		TEST_CHECK(rotationWeight.getValue(1, 0) == 24.f);
		TEST_CHECK(rotationWeight.getValue(1, 30) == 40.f);
	}

	void TestLongRenderDistance()
	{
		CBlockWorld world;
		const auto originalCacheLimit = world.GetMaxCacheRegionCount();
		world.SetRenderDist(2000);
		TEST_CHECK(world.GetRenderDist() == 2000);
		world.SetRenderDist(100000);
		TEST_CHECK(world.GetRenderDist() == 2048);
		world.SetRenderDist(32);
		TEST_CHECK(world.GetActiveChunkDim() < 16);
		TEST_CHECK(world.GetMaxCacheRegionCount() == originalCacheLimit);

		BlockRegion region(0, 0, &world);
		const int emptyBytes = region.GetTotalBytes();
		const ChunkMaxHeight roof(12, 14);
		region.SetColumnHeight(15, 15, roof);
		region.SetColumnHeight(16, 16, ChunkMaxHeight());
		TEST_CHECK(region.GetHighestBlock(15, 15)->GetMaxHeight() == 14);
		TEST_CHECK(region.GetHighestBlock(31, 31)->GetMaxHeight() == BlockConfig::MinBlockY - 1);
		TEST_CHECK(region.GetTotalBytes() == emptyBytes + 256 * sizeof(ChunkMaxHeight));
		region.DeleteAllBlocks();
		TEST_CHECK(region.GetTotalBytes() == emptyBytes);
	}

	void TestBlockHeightRange()
	{
		CBlockWorld world;
		BlockRegion region(0, 0, &world);
		auto* stone = world.RegisterTemplate(1, BlockTemplate::batt_solid | BlockTemplate::batt_cubeModel, 0);
		const int levels[] = {-256, -1, 0, 255, 256, 9000};
		for (int y : levels)
		{
			region.SetBlockTemplateByIndex(2, y, 3, stone);
			TEST_CHECK(region.GetBlockTemplateIdByIndex(2, y, 3) == 1);
			region.SetBlockUserDataByIndex(2, y, 3, y + 257);
		}
		for (int y : levels)
			TEST_CHECK(region.GetBlockUserDataByIndex(2, y, 3) == y + 257);
		TEST_CHECK(region.GetColumnSections(0, 0).size() == 6);
		region.SetBlockTemplateByIndex(2, 9001, 3, stone);
		region.SetBlockTemplateByIndex(2, -257, 3, stone);
		TEST_CHECK(region.GetBlock(2, 9001, 3) == nullptr);
		TEST_CHECK(region.GetBlock(2, -257, 3) == nullptr);
		TEST_CHECK(region.GetHighestBlock(2, 3)->GetMaxSolidHeight() == 9000);

		world.SaveLightMap(true);
		const auto file = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("block-height-%%%%-%%%%.raw");

		region.SetBlockUserDataByIndex(2, 9000, 3, 0xaff0ff01u);
		auto* bottom = region.GetChunk(CalcPackedChunkID(2, -256, 3), false);

		bottom->SetLightingInitialized(true);
		bottom->GetLightData(10)->LoadBrightness(6, 12);

		region.SetBlockMaterial(2, 9000, 3, 0, 12345);
		region.SaveToFile(file.generic_string().c_str());
		TEST_CHECK(!region.IsModified());
		{
			BlockRegion restored(0, 0, &world);
			BlockRegion::LoadFromFile_s(&restored, file.generic_string().c_str());

			for (int y : levels) TEST_CHECK(restored.GetBlockTemplateIdByIndex(2, y, 3) == 1);
			TEST_CHECK(restored.GetBlockUserDataByIndex(2, 9000, 3) == 0xaff0ff01u);
			TEST_CHECK(restored.GetHighestBlock(2, 3)->GetMaxSolidHeight() == 9000);
			TEST_CHECK(restored.GetBlockMaterial(2, 9000, 3, 0) == 12345);
			auto* section = restored.GetChunk(CalcPackedChunkID(2, -256, 3), false);
			TEST_CHECK(section && section->IsLightingInitialized() && section->GetLightData(10)->GetBrightness(false) == 12);
			TEST_CHECK(restored.GetColumnSections(0, 0).size() == 6);
		}
		{
			BlockRegion legacy(0, 0, &world);
			legacy.SetBlockTemplateByIndex(2, 255, 3, stone);
			legacy.SaveToFile(file.generic_string().c_str());
			BlockRegion restored(0, 0, &world);
			BlockRegion::LoadFromFile_s(&restored, file.generic_string().c_str());

			TEST_CHECK(restored.GetBlockTemplateIdByIndex(2, 255, 3) == 1);
			TEST_CHECK(restored.GetColumnSections(0, 0).size() == 1);
		}
		boost::filesystem::remove(file);
		lua_State* L = luaL_newstate();
		{
			auto output = luabind::newtable(L);
			BlockRegion replica(0, 0, &world);
			std::vector<int16_t> sections{-16, 562};
			replica.ApplyMapChunkData(0, 0, 0, region.GetMapChunkData(0, 0, true, 0, &sections), output, true);
			TEST_CHECK(replica.GetBlockTemplateIdByIndex(2, -256, 3) == 1);
			TEST_CHECK(replica.GetBlockUserDataByIndex(2, 9000, 3) == 0xaff0ff01u);
			TEST_CHECK(replica.GetBlockTemplateIdByIndex(2, 255, 3) == 0);
			BlockRegion partial(0, 0, &world);
			sections.pop_back();
			partial.ApplyMapChunkData(0, 0, 0, region.GetMapChunkData(0, 0, true, 0, &sections), output, true);
			TEST_CHECK(partial.GetHighestBlock(2, 3)->GetMaxSolidHeight() == 9000);
			TEST_CHECK(partial.GetColumnSections(0, 0).size() == 1);
			BlockRegion legacyPeer(0, 0, &world);
			legacyPeer.ApplyMapChunkData(0, 0, 0xffff, region.GetMapChunkData(0, 0, true, 0xffff), output);
			TEST_CHECK(legacyPeer.GetBlockTemplateIdByIndex(2, 255, 3) == 1);
			TEST_CHECK(legacyPeer.GetBlockTemplateIdByIndex(2, -256, 3) == 0);
		}
		lua_close(L);
		region.SetBlockTemplateByIndex(2, 9000, 3, nullptr);
		TEST_CHECK(region.GetHighestBlock(2, 3)->GetMaxSolidHeight() == 256);
		for (int y = -256; y <= 9000; ++y)
		{
			uint16_t x, z;
			int16_t section;
			UnpackChunkIndex(CalcPackedChunkID(511, y, 510), x, section, z);
			TEST_CHECK(x == 31 && z == 31 && section * 16 + (y & 15) == y);
		}
	}

	class LightingTestWorld : public CBlockWorld
	{
	public:
		LightingTestWorld() : region(0, 0, this)
		{
			delete m_pLightGrid;
			m_pLightGrid = new CBlockLightGridClient(8, this);
			m_pRegions[0] = &region;
			m_isInWorld = true;
			GetLightGrid().OnEnterWorld();
		}
		~LightingTestWorld() { m_isInWorld = false; m_pRegions[0] = nullptr; }
		void CheckUnloadedRoof()
		{
			auto* roof = new BlockRegion(1, 0, this);
			m_pRegions[roof->GetPackedRegionIndex()] = roof;
			roof->SetBlockTemplateByIndex(24, 9000, 24, GetBlockTemplate(1));
			SetIsRemote(true);
			UnloadRegion(roof, true);
			auto* summary = GetHighestBlock(536, 24);
			TEST_CHECK(summary && summary->GetMaxSolidHeight() == 9000);
			TEST_CHECK(!GetChunk(536, 9000, 24, false));
			SetIsRemote(false);
		}
		BlockRegion region;
	};

	void TestSparseSkyLight()
	{
		size_t referenceSeeds = 0, referenceSections = 0;
		uint64_t referenceVisits = 0;
		// The first pair aligns section boundaries. The second leaves precisely 256 and 9,000 air cells.
		for (int roof : {8, 9000, 1, 8745})
		{
			LightingTestWorld world;
			auto& grid = static_cast<CBlockLightGridClient&>(world.GetLightGrid());
			auto* stone = world.RegisterTemplate(1, BlockTemplate::batt_solid | BlockTemplate::batt_cubeModel, 0);
			auto* torch = world.RegisterTemplate(2, BlockTemplate::batt_light | BlockTemplate::batt_transparent, 0);
			torch->SetTorchLight(15);
			torch->SetLightOpacity(1);
			world.SetBlockId(24, -256, 24, 1);
			grid.ProcessPendingLightCells(100000);
			Uint16x3 sample(24, -255, 24);
			uint8_t light[2] = {};
			grid.GetBrightness(sample, light, 1, 2);
			TEST_CHECK(light[1] == 15);
			PickResult picked{};
			TEST_CHECK(world.Pick(BlockCommon::ConvertToRealPosition(24, -254, 24), Vector3(0, -1, 0), 10, picked) && picked.BlockY == -256);
			auto before = grid.GetLightCellsVisited();
			world.SetBlockId(24, roof, 24, 1);
			TEST_CHECK(world.Pick(BlockCommon::ConvertToRealPosition(24, roof + 2, 24), Vector3(0, -1, 0), 10, picked) && picked.BlockY == roof);
			auto seeds = grid.GetPendingLightCellCount();
			TEST_CHECK(seeds > 0 && seeds < 64);
			grid.ProcessPendingLightCells(100000);
			grid.GetBrightness(sample, light, 1, 2);
			TEST_CHECK(light[1] == 14);
			auto sections = world.region.GetColumnSections(1, 1);
			for (int y : sections) TEST_CHECK(y <= -14 || y >= BlockConfig::SectionY(roof) - 2);
			auto count = BlockChunk::GetTotalChunksInMemory();
			for (int y = -220; y < roof - 20; y += 16)
			{
				Uint16x3 air(24, y, 24);
				grid.GetBrightness(air, light, 1, 2);
				TEST_CHECK(light[1] == 14);
			}
			TEST_CHECK(BlockChunk::GetTotalChunksInMemory() == count);
			std::printf("roof Y %d: %zu seeds, %llu light queue visits, %zu stored sections in column\n", roof, seeds,
				static_cast<unsigned long long>(grid.GetLightCellsVisited() - before), sections.size());
			const auto visits = grid.GetLightCellsVisited() - before;
			TEST_CHECK(visits < 128 && sections.size() <= 6);
			if (roof == 9000)
			{
				world.CheckUnloadedRoof();
				TEST_CHECK(seeds == referenceSeeds);
				TEST_CHECK(visits == referenceVisits);
				TEST_CHECK(sections.size() == referenceSections);
			}
			referenceSeeds = seeds;
			referenceVisits = visits;
			referenceSections = sections.size();
			world.SetBlockId(24, roof, 24, 0);
			grid.ProcessPendingLightCells(100000);
			grid.GetBrightness(sample, light, 1, 2);
			TEST_CHECK(light[1] == 15);
			auto* glass = world.RegisterTemplate(3, BlockTemplate::batt_transparent, 0);
			glass->SetLightOpacity(1);
			auto* water = world.RegisterTemplate(4, BlockTemplate::batt_liquid, 0);
			water->SetLightOpacity(3);
			world.SetBlockId(24, roof, 24, 3);
			grid.ProcessPendingLightCells(100000);
			grid.GetBrightness(sample, light, 1, 2);
			TEST_CHECK(light[1] == 15);
			world.SetBlockId(24, roof, 24, 4);
			grid.ProcessPendingLightCells(100000);
			grid.GetBrightness(sample, light, 1, 2);
			TEST_CHECK(light[1] == 14);
			world.SetBlockId(24, -241, 24, 2);
			world.SetBlockId(26, -239, 24, 2);
			grid.ProcessPendingLightCells(100000);
			Uint16x3 seam(24, -240, 24);
			grid.GetBrightness(seam, light, 1, 2);
			TEST_CHECK(light[0] == 14);
			world.SetBlockId(24, -241, 24, 0);
			grid.ProcessPendingLightCells(100000);
			grid.GetBrightness(seam, light, 1, 2);
			TEST_CHECK(light[0] == 12);
			if (roof == 9000)
			{
				// Tessellation batches both channels across signed and partial sections.
				world.SetBlockId(31, -1, 31, 2);
				grid.ProcessPendingLightCells(100000);
				for (int level : {-256, -241, -1, 0, 9000, 9001})
				{
					Uint16x3 center(31, level, 31);
					uint8_t batch[81], single[3];
					grid.GetBrightness(center, batch, 27, 3);
					for (int i = 0; i < 27; ++i)
					{
						Uint16x3 neighbor = center + BlockCommon::NeighborOfsTable[i];
						grid.GetBrightness(neighbor, single, 1, 3);
						for (int channel = 0; channel < 3; ++channel)
							TEST_CHECK(batch[27 * channel + i] == single[channel]);
					}
				}
			}
		}
	}

	void TestSavedSectionValidity()
	{
		const auto file = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("block-light-%%%%-%%%%.raw");
		LightingTestWorld world;
		world.SaveLightMap(true);
		auto& grid = static_cast<CBlockLightGridClient&>(world.GetLightGrid());
		world.RegisterTemplate(1, BlockTemplate::batt_solid | BlockTemplate::batt_cubeModel, 0);
		grid.SetColumnPreloaded(1, 1);
		world.SetBlockId(24, -256, 24, 1);
		world.SetBlockId(24, 9000, 24, 1);
		grid.ProcessPendingLightCells(100000);
		world.region.SaveToFile(file.generic_string().c_str());
		{
			LightingTestWorld loaded;
			loaded.RegisterTemplate(1, BlockTemplate::batt_solid | BlockTemplate::batt_cubeModel, 0);
			BlockRegion::LoadFromFile_s(&loaded.region, file.generic_string().c_str());
			loaded.region.OnLoadWorldFinished();
			auto& warm = static_cast<CBlockLightGridClient&>(loaded.GetLightGrid());
			TEST_CHECK(warm.IsChunkColumnLoaded(1, 1));
			TEST_CHECK(warm.GetPendingLightCellCount() == 0);
		}
		world.SetBlockId(24, 8999, 24, 1);
		TEST_CHECK(grid.IsSectionLightPending(1, 562, 1));
		TEST_CHECK(!grid.IsSectionLightPending(1, -16, 1));
		world.region.SaveToFile(file.generic_string().c_str());
		LightingTestWorld restored;
		restored.RegisterTemplate(1, BlockTemplate::batt_solid | BlockTemplate::batt_cubeModel, 0);
		BlockRegion::LoadFromFile_s(&restored.region, file.generic_string().c_str());
		restored.region.OnLoadWorldFinished();
		TEST_CHECK(restored.GetBlockTemplateIdByIdx(24, 8999, 24) == 1);
		TEST_CHECK(!restored.region.GetChunk(CalcPackedChunkID(24, 8999, 24), false)->IsLightingInitialized());
		TEST_CHECK(!restored.GetLightGrid().IsChunkColumnLoaded(1, 1));
		world.GetLightGrid().SetSkyHeight(-255);
		uint8_t light[2]{};
		Uint16x3 shadow(24, -255, 24);
		grid.GetBrightness(shadow, light, 1, 2);
		TEST_CHECK(grid.GetSkyHeight() == -255 && light[1] == 15);
		CBlockWorld::SetLegacyHeightSession_s(&world, true);
		world.SetBlockId(30, -256, 30, 1);
		world.SetBlockId(30, 9000, 30, 1);
		TEST_CHECK(!world.region.GetBlock(30, -256, 30) && !world.region.GetBlock(30, 9000, 30));
		world.SetBlockId(30, 255, 30, 1);
		TEST_CHECK(world.GetBlockId(30, 255, 30) == 1);
		boost::filesystem::remove(file);
	}

	void TestFrameDrawStatistics()
	{
		auto& settings = CGlobals::GetSettings();
		RenderDevice::ClearAllPerfCount();
		RenderDevice::RecordDraw(EPrimitiveType::LINELIST, 20);
		RenderDevice::RecordDraw(EPrimitiveType::POINTLIST, 8);
		RenderDevice::RecordDraw(EPrimitiveType::TRIANGLESTRIP, 2);
		RenderDevice::RecordDraw(EPrimitiveType::TRIANGLEFAN, 5);
		RenderDevice::RecordDraw(EPrimitiveType::TRIANGLELIST, 10, 3);
		RenderDevice::PublishFramePerfCounts();
		TEST_CHECK(settings.GetDrawCallCount() == 5);
		TEST_CHECK(settings.GetTriangleCount() == 37);

		// Scripts reading during the next frame still see the completed frame.
		RenderDevice::ClearAllPerfCount();
		TEST_CHECK(settings.GetDrawCallCount() == 5);
		TEST_CHECK(settings.GetTriangleCount() == 37);
		RenderDevice::PublishFramePerfCounts();
		TEST_CHECK(settings.GetDrawCallCount() == 0);
		TEST_CHECK(settings.GetTriangleCount() == 0);
	}

	void TestLowEndAndSkinningCaps()
	{
		// 1. DirectX 9-class feature levels (9_1, 9_2, 9_3)
		for (uint32_t fl : { 0x9100u, 0x9200u, 0x9300u })
		{
			RHIDeviceCaps caps{};
			caps.backend = ERHIBackend::D3D11;
			caps.featureLevel = fl;
			caps.maxSimultaneousRTs = (fl == 0x9300u) ? 4u : 1u;
			caps.supportsDepthTextures = false;
			caps.supportsInstancing = (fl == 0x9300u);
			caps.supports32BitIndices = (fl != 0x9100u);
			caps.maxVertexUniformVectors = 256;

			TEST_CHECK(caps.GetShaderModelMajor() == 2);
			TEST_CHECK(caps.IsDX9ClassDevice());
			TEST_CHECK(caps.IsLowEndDevice());
		}

		// 2. Software rasterizer adapter (e.g., WARP / SwiftShader fallback)
		{
			RHIDeviceCaps warpCaps{};
			warpCaps.backend = ERHIBackend::D3D12;
			warpCaps.featureLevel = 0xB000u;
			warpCaps.maxSimultaneousRTs = 8;
			warpCaps.supportsDepthTextures = true;
			warpCaps.supportsOcclusionQuery = true;
			warpCaps.supportsInstancing = true;
			warpCaps.isSoftwareAdapter = true;
			warpCaps.maxVertexUniformVectors = 4096;

			TEST_CHECK(warpCaps.GetShaderModelMajor() == 5);
			TEST_CHECK(!warpCaps.IsDX9ClassDevice());
			TEST_CHECK(warpCaps.IsLowEndDevice());
		}

		// 3. Mobile GLES2 / constrained uniform budget (< 192 vec4 uniforms)
		{
			RHIDeviceCaps gles2Caps{};
			gles2Caps.backend = ERHIBackend::OpenGL;
			gles2Caps.featureLevel = 200u;
			gles2Caps.maxSimultaneousRTs = 1;
			gles2Caps.supportsDepthTextures = false;
			gles2Caps.maxVertexUniformVectors = 128;

			TEST_CHECK(gles2Caps.GetShaderModelMajor() == 2);
			TEST_CHECK(gles2Caps.IsDX9ClassDevice());
			TEST_CHECK(gles2Caps.IsLowEndDevice());
			TEST_CHECK(gles2Caps.GetMaxGpuSkinningBones() == 0);
		}

		// 4. Modern SM5 desktop GPU
		{
			RHIDeviceCaps modernCaps{};
			modernCaps.backend = ERHIBackend::D3D11;
			modernCaps.featureLevel = 0xB000u;
			modernCaps.maxSimultaneousRTs = 8;
			modernCaps.supportsDepthTextures = true;
			modernCaps.supportsOcclusionQuery = true;
			modernCaps.supportsInstancing = true;
			modernCaps.supports32BitIndices = true;
			modernCaps.maxVertexUniformVectors = 4096;

			TEST_CHECK(modernCaps.GetShaderModelMajor() == 5);
			TEST_CHECK(!modernCaps.IsDX9ClassDevice());
			TEST_CHECK(!modernCaps.IsLowEndDevice());
			TEST_CHECK(modernCaps.GetMaxGpuSkinningBones() == 58);
		}

		// 5. GPU skinning vertex struct byte strides match VertexDeclaration & HLSL
		TEST_CHECK(sizeof(mesh_vertex_normal_skinned) == 40);
		TEST_CHECK(sizeof(bmax_vertex_skinned) == 36);
	}

	void TestBlockChunkVisibilityGraph()
	{
		BlockTemplate solidStone(1, BlockTemplate::batt_solid | BlockTemplate::batt_cubeModel | BlockTemplate::batt_obstruction, 0);
		BlockTemplate clearGlass(2, BlockTemplate::batt_transparent | BlockTemplate::batt_cubeModel, 0);

		// 1. Empty chunk: all 15 face pairs mutually visible, 0 opaque layers
		{
			BlockChunk chunk(0, nullptr);
			chunk.UpdateVisibilityGraph();
			const uint16_t vis = chunk.GetVisibilityGraph();
			TEST_CHECK(vis == BlockChunk::kVisGraphAllConnected);
			TEST_CHECK(chunk.GetOpaqueLayerMask() == 0);

			int minY = -1, maxY = -1;
			TEST_CHECK(!BlockChunk::GetBestOpaqueLayerSpan(0, minY, maxY));

			for (int a = 0; a < 6; ++a)
			{
				for (int b = a + 1; b < 6; ++b)
				{
					TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, a, b));
				}
			}
		}

		// 2. Fully solid 16x16x16 chunk: 0 face pairs visible, kVisGraphFullyOpaque set, 16 opaque layers
		{
			BlockChunk chunk(0, nullptr);
			for (uint16_t x = 0; x < 16; ++x)
			{
				for (uint16_t y = 0; y < 16; ++y)
				{
					for (uint16_t z = 0; z < 16; ++z)
					{
						chunk.SetBlockTemplate(CalcPackedBlockID(x, y, z), &solidStone);
					}
				}
			}
			chunk.UpdateVisibilityGraph();
			const uint16_t vis = chunk.GetVisibilityGraph();
			TEST_CHECK(vis == BlockChunk::kVisGraphFullyOpaque);
			TEST_CHECK(chunk.GetOpaqueLayerMask() == 0xFFFFu);

			int minY = -1, maxY = -1;
			TEST_CHECK(BlockChunk::GetBestOpaqueLayerSpan(chunk.GetOpaqueLayerMask(), minY, maxY));
			TEST_CHECK(minY == 0 && maxY == 15);

			for (int a = 0; a < 6; ++a)
			{
				for (int b = a + 1; b < 6; ++b)
				{
					TEST_CHECK(!BlockChunk::CanTraverseVisGraph(vis, a, b));
				}
			}
		}

		// 3. Horizontal solid slab at y = 6..8 (3 layers): blocks top<->bottom, preserves lateral visibility
		{
			BlockChunk chunk(0, nullptr);
			for (uint16_t y = 6; y <= 8; ++y)
			{
				for (uint16_t x = 0; x < 16; ++x)
				{
					for (uint16_t z = 0; z < 16; ++z)
					{
						chunk.SetBlockTemplate(CalcPackedBlockID(x, y, z), &solidStone);
					}
				}
			}
			chunk.UpdateVisibilityGraph();
			const uint16_t vis = chunk.GetVisibilityGraph();
			const uint16_t layers = chunk.GetOpaqueLayerMask();
			TEST_CHECK(layers == ((1u << 6) | (1u << 7) | (1u << 8)));

			int minY = -1, maxY = -1;
			TEST_CHECK(BlockChunk::GetBestOpaqueLayerSpan(layers, minY, maxY));
			TEST_CHECK(minY == 6 && maxY == 8);

			TEST_CHECK(!BlockChunk::CanTraverseVisGraph(vis, BlockDirection::top, BlockDirection::bottom));
			TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, BlockDirection::left, BlockDirection::right));
			TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, BlockDirection::front, BlockDirection::back));
			TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, BlockDirection::top, BlockDirection::left));
			TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, BlockDirection::bottom, BlockDirection::right));
		}

		// 4. L-shaped cave tunnel inside a solid chunk:
		//    cx == 0 is BlockDirection::right (-X face), cy == 15 is BlockDirection::top (+Y face).
		{
			BlockChunk chunk(0, nullptr);
			for (uint16_t x = 0; x < 16; ++x)
			{
				for (uint16_t y = 0; y < 16; ++y)
				{
					for (uint16_t z = 0; z < 16; ++z)
					{
						const bool isTunnel = (y == 8 && z == 8 && x <= 8) || (x == 8 && z == 8 && y >= 8);
						if (!isTunnel)
							chunk.SetBlockTemplate(CalcPackedBlockID(x, y, z), &solidStone);
					}
				}
			}

			chunk.UpdateVisibilityGraph();
			const uint16_t vis = chunk.GetVisibilityGraph();

			TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, BlockDirection::right, BlockDirection::top));
			TEST_CHECK(!BlockChunk::CanTraverseVisGraph(vis, BlockDirection::right, BlockDirection::left));
			TEST_CHECK(!BlockChunk::CanTraverseVisGraph(vis, BlockDirection::right, BlockDirection::bottom));
			TEST_CHECK(!BlockChunk::CanTraverseVisGraph(vis, BlockDirection::top, BlockDirection::bottom));
			TEST_CHECK(!BlockChunk::CanTraverseVisGraph(vis, BlockDirection::front, BlockDirection::back));
		}

		// 5. Transparent glass wall does NOT occlude visibility or opaque layer mask
		{
			BlockChunk chunk(0, nullptr);
			for (uint16_t x = 0; x < 16; ++x)
				for (uint16_t z = 0; z < 16; ++z)
					chunk.SetBlockTemplate(CalcPackedBlockID(x, 8, z), &clearGlass);

			chunk.UpdateVisibilityGraph();
			const uint16_t vis = chunk.GetVisibilityGraph();
			TEST_CHECK(chunk.GetOpaqueLayerMask() == 0);
			TEST_CHECK(BlockChunk::CanTraverseVisGraph(vis, BlockDirection::top, BlockDirection::bottom));
		}
	}

	void TestSoftwareOcclusionBufferMode(bool zeroToOneClipZ)
	{
		CSoftwareOcclusionBuffer occluder;
		occluder.SetEnabled(true);
		TEST_CHECK(!occluder.IsFrameValid());

		Vector3 eye(0.0f, 5.0f, -20.0f);
		Vector3 at(0.0f, 5.0f, 0.0f);
		Vector3 up(0.0f, 1.0f, 0.0f);

		Matrix4 matView;
		ParaMatrixLookAtLH(&matView, &eye, &at, &up);
		Matrix4 matProj = BuildTestPerspectiveLH(3.14159265f * 0.4f, 16.0f / 9.0f, 0.5f, 500.0f, zeroToOneClipZ);
		Matrix4 matViewProj = matView * matProj;

		occluder.BeginFrame(matViewProj, eye, Vector3::ZERO, zeroToOneClipZ);
		TEST_CHECK(occluder.IsFrameValid());
		TEST_CHECK(occluder.GetOccluderCount() == 0);

		CShapeBox hiddenBehindWall(Vector3(-2.0f, 2.0f, 8.0f), Vector3(2.0f, 6.0f, 12.0f));
		// Empty buffer must never report occluded
		TEST_CHECK(!occluder.IsWorldAABBOccluded(hiddenBehindWall));

		// Rasterize opaque wall spanning [-10..10, 0..12, 0..2]
		CShapeBox wall(Vector3(-10.0f, 0.0f, 0.0f), Vector3(10.0f, 12.0f, 2.0f));
		occluder.RasterizeOccluderWorldAABB(wall.m_Min, wall.m_Max);
		TEST_CHECK(occluder.GetOccluderCount() == 1);
		occluder.FinalizeHierarchy();

		// 1. Completely behind wall -> occluded (both AABB and point-light sphere)
		TEST_CHECK(occluder.IsWorldAABBOccluded(hiddenBehindWall));
		TEST_CHECK(occluder.IsWorldSphereOccluded(Vector3(0.0f, 4.0f, 10.0f), 2.0f));

		// 2. Tower peeking above wall top (y up to 16 > wall y=12) -> visible (NOT occluded)
		CShapeBox tallTower(Vector3(-1.0f, 2.0f, 8.0f), Vector3(1.0f, 16.0f, 10.0f));
		TEST_CHECK(!occluder.IsWorldAABBOccluded(tallTower));

		// 3. Object in front of wall (z = -8..-5 < wall z=0) -> visible (NOT occluded)
		CShapeBox inFront(Vector3(-1.0f, 3.0f, -8.0f), Vector3(1.0f, 6.0f, -5.0f));
		TEST_CHECK(!occluder.IsWorldAABBOccluded(inFront));

		// 4. Box containing the camera -> visible (NOT occluded)
		CShapeBox aroundCamera(Vector3(-5.0f, 0.0f, -25.0f), Vector3(5.0f, 10.0f, -15.0f));
		TEST_CHECK(!occluder.IsWorldAABBOccluded(aroundCamera));

		// 5. Two adjacent 16x16x16 chunk walls sharing exact seam x = 0:
		//    Conservative interior rule must leave zero cracks along x = 0!
		occluder.BeginFrame(matViewProj, eye, Vector3::ZERO, zeroToOneClipZ);
		CShapeBox leftChunk(Vector3(-16.0f, 0.0f, 10.0f), Vector3(0.0f, 16.0f, 26.0f));
		CShapeBox rightChunk(Vector3(0.0f, 0.0f, 10.0f), Vector3(16.0f, 16.0f, 26.0f));
		occluder.RasterizeOccluderWorldAABB(leftChunk.m_Min, leftChunk.m_Max);
		occluder.RasterizeOccluderWorldAABB(rightChunk.m_Min, rightChunk.m_Max);
		occluder.FinalizeHierarchy();

		CShapeBox straddlingSeamBehind(Vector3(-3.0f, 3.0f, 32.0f), Vector3(3.0f, 9.0f, 36.0f));
		TEST_CHECK(occluder.IsWorldAABBOccluded(straddlingSeamBehind));

		occluder.Reset();
		TEST_CHECK(!occluder.IsFrameValid());
		TEST_CHECK(!occluder.IsWorldAABBOccluded(straddlingSeamBehind));
	}

	void TestRigidBodyFastPathAndPsoStateHashCache()
	{
		// 1. All-rigid-body multi-pass ParaX model (like Bumi robot links: 100% weight on 1 bone per pass)
		{
			ParaXHeaderDef hdr{};
			hdr.IsAnimated = 1;
			CParaXModel rigidModel(hdr);
			rigidModel.animated = true;
			rigidModel.animBones = true;
			rigidModel.animGeometry = false;
			rigidModel.m_objNum.nBones = 26;
			rigidModel.bones = new Bone[26];

			ModelVertex verts[6] = {};
			for (int i = 0; i < 3; ++i)
			{
				verts[i].pos = Vector3(static_cast<float>(i), 0.0f, 0.0f);
				verts[i].bones[0] = 5;
				verts[i].weights[0] = 0xff;
			}
			for (int i = 3; i < 6; ++i)
			{
				verts[i].pos = Vector3(static_cast<float>(i), 1.0f, 0.0f);
				verts[i].bones[0] = 18;
				verts[i].weights[0] = 0xff;
			}
			uint16 indices[6] = { 0, 1, 2, 0, 1, 2 };

			rigidModel.SetRenderMethod(CParaXModel::SOFT_ANIM);
			rigidModel.initVertices(6, verts);
			rigidModel.initIndices(6, indices);

			ModelGeoset g0{}, g1{};
			g0.SetVertexStart(0);
			g1.SetVertexStart(3);
			rigidModel.geosets.push_back(g0);
			rigidModel.geosets.push_back(g1);

			ModelRenderPass p0{}, p1{};
			p0.geoset = 0;
			p0.SetStartIndex(0);
			p0.indexCount = 3;
			p1.geoset = 1;
			p1.SetStartIndex(3);
			p1.indexCount = 3;
			rigidModel.passes.push_back(p0);
			rigidModel.passes.push_back(p1);

			rigidModel.DetectRigidBodyPasses();
			TEST_CHECK(rigidModel.passes[0].is_rigid_body);
			TEST_CHECK(rigidModel.passes[1].is_rigid_body);

			// Now make pass 1 have a 2-bone blended vertex and verify mixed model does NOT falsely report rigid
			rigidModel.passes[1].is_rigid_body = false;
			rigidModel.m_origVertices[4].weights[0] = 128;
			rigidModel.m_origVertices[4].weights[1] = 127;
			rigidModel.m_origVertices[4].bones[1] = 19;
			rigidModel.DetectRigidBodyPasses();
			TEST_CHECK(rigidModel.passes[0].is_rigid_body);
			TEST_CHECK(!rigidModel.passes[1].is_rigid_body);
			TEST_CHECK(!rigidModel.CanUseRigidBodyGpuBuffers());
		}

		// 2. RHIStateTracker cached PSO hash consistency and invalidation
		{
			RHIStateTracker tracker;
			const uint64_t h0 = tracker.ComputePSOHash(0x1111ULL, 0x2222ULL, EPrimitiveType::TRIANGLELIST, 0x3333ULL);
			const uint64_t h0Repeat = tracker.ComputePSOHash(0x1111ULL, 0x2222ULL, EPrimitiveType::TRIANGLELIST, 0x3333ULL);
			TEST_CHECK(h0 == h0Repeat);

			// Changing CULLMODE (e.g. doubleSided robot pass) must invalidate cached render-state hash
			tracker.SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
			const uint64_t hNoCull = tracker.ComputePSOHash(0x1111ULL, 0x2222ULL, EPrimitiveType::TRIANGLELIST, 0x3333ULL);
			TEST_CHECK(hNoCull != h0);

			// Restoring CULLMODE must produce the exact original deterministic PSO hash
			tracker.SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
			const uint64_t hRestored = tracker.ComputePSOHash(0x1111ULL, 0x2222ULL, EPrimitiveType::TRIANGLELIST, 0x3333ULL);
			TEST_CHECK(hRestored == h0);
		}
	}

	class TestPoolableBuffer : public ParaVertexBuffer
	{
	public:
		void InitFakeValid(uint32 sz)
		{
			m_bufferType = BufferType_MemoryBuffer;
			m_nBufferSize = sz;
			m_buffer = new char[sz];
		}
	};

	void TestChunkCompactMemoryAndBucketPool()
	{
		// 1. ParaMemoryBuffer tracks exact post-culling byte count separately from pre-culling capacity
		{
			ParaMemoryBuffer memBuf;
			const uint32 faceBytes = 4u * (uint32)sizeof(BlockVertexCompressed);
			TEST_CHECK(memBuf.CreateBuffer(6u * faceBytes));
			TEST_CHECK(memBuf.GetBufferSize() == 6u * faceBytes);
			TEST_CHECK(memBuf.GetUsedSize() == 0u);
			memBuf.SetUsedSize(2u * faceBytes);
			TEST_CHECK(memBuf.GetUsedSize() == 2u * faceBytes);
			memBuf.ReleaseBuffer();
		}

		// 2. ParaVertexBufferPool size-bucketed reuse and unused byte budget eviction
		{
			class TestBucketPool : public ParaVertexBufferPool
			{
			public:
				uint32 TestRoundUp(uint32 sz) const { return RoundUpBucketSize(sz); }
				void InjectFakeActive(ParaVertexBuffer* buf)
				{
					m_activeBuffers.insert(buf);
					m_nTotalBufferBytes += buf->GetBufferSize();
				}
			};

			TestBucketPool pool;
			pool.SetFullSizedBufferSize(720000u);
			pool.SetMaxPooledCount(1u); // 720 KB unused budget

			TEST_CHECK(pool.TestRoundUp(80u) == 2560u);
			TEST_CHECK(pool.TestRoundUp(2560u) == 2560u);
			TEST_CHECK(pool.TestRoundUp(2561u) == 5120u);
			TEST_CHECK(pool.TestRoundUp(700000u) == 720000u);

			auto* fakeSmall = new TestPoolableBuffer();
			fakeSmall->InitFakeValid(5120u);
			pool.InjectFakeActive(fakeSmall);
			TEST_CHECK(pool.GetActiveBufferCount() == 1);
			pool.ReleaseBuffer(fakeSmall);
			TEST_CHECK(pool.GetActiveBufferCount() == 0);
			TEST_CHECK(pool.GetTotalBufferCount() == 1);
			TEST_CHECK(pool.GetTotalBufferBytes() == 5120u);

			// Requesting 3200 bytes rounds up to 5120 bucket and reuses fakeSmall in O(1)
			ParaVertexBuffer* reused = pool.CreateBuffer(3200u);
			TEST_CHECK(reused == fakeSmall);
			TEST_CHECK(pool.GetActiveBufferCount() == 1);
			TEST_CHECK(pool.GetTotalBufferCount() == 1);
			pool.ReleaseBuffer(reused);

			// Exceeding unused byte budget evicts largest unused buffers in TickCache
			auto* fakeLarge = new TestPoolableBuffer();
			fakeLarge->InitFakeValid(720000u);
			pool.InjectFakeActive(fakeLarge);
			pool.ReleaseBuffer(fakeLarge);
			TEST_CHECK(pool.GetTotalBufferBytes() == 725120u);
			pool.TickCache();
			TEST_CHECK(pool.GetTotalBufferBytes() == 5120u);
			TEST_CHECK(pool.GetTotalBufferBytes(true) == 5120u);
			pool.Cleanup();
			TEST_CHECK(pool.GetTotalBufferBytes() == 0u);
		}
	}

	void TestWaveBRuntimeOptimizations()
	{
		// 1. Animated::CompactConstantRanges collapses constant per-animation ranges while preserving varying ranges
		{
			Animated<Quaternion> rotTrack;
			rotTrack.used = true;
			rotTrack.seq = -1;
			rotTrack.type = INTERPOLATION_LINEAR;
			Quaternion qIdle(0.f, 0.70710678f, 0.f, 0.70710678f);
			Quaternion qWalk0(0.f, 0.f, 0.f, 1.f);
			Quaternion qWalk1(0.f, 1.f, 0.f, 0.f);
			for (int k = 0; k < 5; ++k)
				rotTrack.AppendKey(k * 100, qIdle);
			rotTrack.AppendKey(500, qWalk0);
			rotTrack.AppendKey(600, qWalk1);
			rotTrack.SetRangeByAnimIndex(0, AnimRange(0, 4)); // constant across 5 keys
			rotTrack.SetRangeByAnimIndex(1, AnimRange(5, 6)); // varying across 2 keys

			rotTrack.CompactConstantRanges();
			TEST_CHECK(rotTrack.ranges[0].first == 0 && rotTrack.ranges[0].second == 0);
			TEST_CHECK(rotTrack.ranges[1].first == 5 && rotTrack.ranges[1].second == 6);
			TEST_CHECK(rotTrack.getValue(0, 250) == qIdle);
		}

		// 2. Matrix4::RemoveScaling fast-path preserves orthonormal matrices and normalizes scaled matrices
		{
			Quaternion q(0.f, 0.70710678f, 0.f, 0.70710678f);
			Matrix4 mUnit(q);
			Matrix4 mBefore = mUnit;
			mUnit.RemoveScaling();
			TEST_CHECK(std::fabs(mUnit._11 - mBefore._11) < 1e-6f && std::fabs(mUnit._13 - mBefore._13) < 1e-6f);

			Matrix4 mScaled;
			mScaled.makeScale(Vector3(2.f, 3.f, 4.f));
			mScaled.RemoveScaling();
			TEST_CHECK(std::fabs(mScaled._11 - 1.f) < 1e-4f);
			TEST_CHECK(std::fabs(mScaled._22 - 1.f) < 1e-4f);
			TEST_CHECK(std::fabs(mScaled._33 - 1.f) < 1e-4f);
		}

		// 3. CAttributeClass::GetFieldIndex zero-alloc string_view lookup and <=2 attribute sort stability
		{
			CAttributeClass smallCls(999901, "WaveBSmallAttr", "test");
			smallCls.AddField("Alpha", FieldType_Int, nullptr, nullptr, nullptr, nullptr, true);
			smallCls.AddField("Beta", FieldType_Float, nullptr, nullptr, nullptr, nullptr, true);
			const int idxA = smallCls.GetFieldIndex("Alpha");
			const int idxB = smallCls.GetFieldIndex("Beta");
			TEST_CHECK(idxA >= 0 && idxB >= 0 && idxA != idxB);
			TEST_CHECK(smallCls.GetFieldIndex("MissingFieldWithLongNameExceedingSSO") == -1);
			TEST_CHECK(smallCls.GetFieldIndex(static_cast<const char*>(nullptr)) == -1);
			TEST_CHECK(smallCls.GetField("Beta") != nullptr && smallCls.GetField("Beta")->GetFieldname() == "Beta");
		}

		// 4. ParticleSystem std::vector FIFO compaction preserves particle order when expiring middle particles
		{
			ParticleSystem ps;
			ps.gravity.AppendKey(0, 0.f);
			ps.slowdown = 0.f;
			ps.mid = 0.5f;
			ps.sizes[0] = ps.sizes[1] = ps.sizes[2] = 1.f;
			ps.colors[0] = ps.colors[1] = ps.colors[2] = Vector4(1, 1, 1, 1);
			ps.billboard = true;

			ParticleList list;
			list.m_bUseAbsCord = true;
			for (int i = 0; i < 4; ++i)
			{
				Particle p{};
				p.pos = Vector3(static_cast<float>(i + 1), 0.f, 0.f);
				p.speed = Vector3::ZERO;
				p.down = Vector3::ZERO;
				p.origin = p.pos;
				p.life = (i == 1) ? 0.95f : 0.1f; // particle #1 expires on dt=0.1
				p.maxlife = 1.0f;
				p.tile = i + 10;
				list.particles.push_back(p);
			}
			TEST_CHECK(ps.AnimateExistingParticles(0.1f, Vector3::ZERO, &list));
			TEST_CHECK(list.particles.size() == 3);
			TEST_CHECK(list.particles[0].tile == 10 && list.particles[1].tile == 12 && list.particles[2].tile == 13);
			TEST_CHECK(std::fabs(list.m_positionSum.x - (1.f + 3.f + 4.f)) < 1e-4f);
		}
	}
}

int main()
{
	std::printf("=== Running World & Block Rendering Optimization Unit Tests ===\n");

	CParaEngineAppBase application;
	TestAnimationKeyIntervals();
	TestLongRenderDistance();
	TestBlockHeightRange();
	TestSparseSkyLight();
	TestSavedSectionValidity();
	TestFrameDrawStatistics();
	TestLowEndAndSkinningCaps();
	TestBlockChunkVisibilityGraph();
	TestSoftwareOcclusionBufferMode(true);   // D3D11 / D3D12 / Vulkan / Metal / WebGPU [0, 1] clip Z
	TestSoftwareOcclusionBufferMode(false);  // OpenGL [-1, 1] clip Z
	TestRigidBodyFastPathAndPsoStateHashCache();
	TestChunkCompactMemoryAndBucketPool();
	TestWaveBRuntimeOptimizations();

	std::printf("=== Summary: %d passed, %d failed ===\n", g_passed, g_failed);
	return (g_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

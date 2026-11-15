#include "ParaEngine.h"
#include "Framework/Interface/Render/RHITypes.h"
#include "3dengine/SoftwareOcclusionBuffer.h"
#include "BlockEngine/BlockChunk.h"
#include "BlockEngine/BlockTemplate.h"
#include "BlockEngine/BlockDirection.h"
#include "BlockEngine/BlockCommon.h"
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

#include <cstdio>
#include <cstdlib>
#include <cmath>

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
}

int main()
{
	std::printf("=== Running World & Block Rendering Optimization Unit Tests ===\n");

	TestLowEndAndSkinningCaps();
	TestBlockChunkVisibilityGraph();
	TestSoftwareOcclusionBufferMode(true);   // D3D11 / D3D12 / Vulkan / Metal / WebGPU [0, 1] clip Z
	TestSoftwareOcclusionBufferMode(false);  // OpenGL [-1, 1] clip Z

	std::printf("=== Summary: %d passed, %d failed ===\n", g_passed, g_failed);
	return (g_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

// Minimal headless platform stubs for linking ParaEngine static library without a GUI application target
#ifdef CreateProcess
#undef CreateProcess
#endif
#ifdef ShellExecute
#undef ShellExecute
#endif

ParaEngine::IParaEngineApp* CreateParaEngineApp() { return nullptr; }
ParaEngine::IRenderWindow* CreateParaRenderWindow(const int, const int) { return nullptr; }

namespace ParaEngine
{
	void NetworkAdapter::Init() { m_bInitialized = true; }
	const std::string& NetworkAdapter::GetMaxMacAddress() { static const std::string s; return s; }
	const std::string& NetworkAdapter::GetMaxIPAddress() { static const std::string s; return s; }
	const std::string& ParaEngineSettings::GetMachineID() { static const std::string s = "headless_test"; return s; }

	bool CEditorHelper::OpenWithDefaultEditor(const char*, bool) { return false; }
	bool CEditorHelper::CreateEmptyCharacterEventFile(const char*, const char*) { return false; }
	bool CEditorHelper::CreateProcess(const char*, const char*, bool) { return false; }
	bool CEditorHelper::ShellExecute(const char*, const char*, const char*, const char*, int) { return false; }

	CParaFileUtils* CParaFileUtils::GetInstance() { return nullptr; }

	void CGUIEditBox::CopyToClipboard() {}
	void CGUIEditBox::PasteFromClipboard() {}
	bool StringHelper::CopyTextToClipboard(const std::string&) { return false; }
	const char* StringHelper::GetTextFromClipboard() { return ""; }

	IParaWebView* IParaWebView::createWebView(int, int, int, int) { return nullptr; }
	IParaWebView* IParaWebView::createSubViewView(int, int, int, int) { return nullptr; }

	std::string PlatformBridge::call_native(std::string, const std::string) { return {}; }
	void PlatformBridge::call_native_withCB(std::string, LuaCB*, const std::string) {}

#if (defined(EMSCRIPTEN) || defined(WIN32) || defined(PICO_VR)) && !defined(NPLRUNTIME_SERVER)
	class CParaWebXR : public IParaWebXR
	{
	public:
		virtual void SetIsXR(bool isXR) override;
		virtual bool GetIsXR() override;
		virtual bool IsPressLeftHandKey(int keyCode) override;
		virtual bool IsPressRightHandKey(int keyCode) override;
		virtual void UpdateWebXRView(
			int time,
			Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
			Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation,
			int viewCount,
			Vector3 leftHandPosition, Vector3 rightHandPosition,
			Vector4 leftHandOrientation, Vector4 rightHandOrientation,
			int leftHandKey[7], int rightHandKey[7], Vector4 leftAxes, Vector4 rightAxes) override;
	};
	void CParaWebXR::SetIsXR(bool isXR) { m_isXR = isXR; }
	bool CParaWebXR::GetIsXR() { return m_isXR; }
	bool CParaWebXR::IsPressLeftHandKey(int) { return false; }
	bool CParaWebXR::IsPressRightHandKey(int) { return false; }
	void CParaWebXR::UpdateWebXRView(
		int,
		Vector4, Vector3, Vector4,
		Vector4, Vector3, Vector4,
		int,
		Vector3, Vector3,
		Vector4, Vector4,
		int[7], int[7], Vector4, Vector4) {}
#endif
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const luabind::adl::object&) { return false; }
void ParaScripting::CNPLScriptingState::LoadHAPI_Platform() {}

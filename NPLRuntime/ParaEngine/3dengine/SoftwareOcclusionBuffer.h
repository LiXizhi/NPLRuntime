#pragma once

#include <cstdint>
#include "ParaVector3.h"
#include "ParaVector4.h"
#include "ParaMatrix4.h"
#include "ShapeAABB.h"

namespace ParaEngine
{
	/**
	 * Conservative CPU Software Hierarchical-Z (Hi-Z) Occlusion Buffer.
	 *
	 * Rasterizes solid opaque world/block AABBs onto a 256x128 float depth grid
	 * with a 64x32 coarse 4x4 max-depth Hi-Z pyramid (lazily maintained via dirty bitmasks).
	 * Works identically across all ParaRHI backends, including DirectX 9-class / low-end GPUs
	 * that lack hardware occlusion queries, with zero GPU readback stalls.
	 */
	class CSoftwareOcclusionBuffer
	{
	public:
		static constexpr int WIDTH = 256;
		static constexpr int HEIGHT = 128;
		static constexpr int TILE_SIZE = 4;
		static constexpr int COARSE_WIDTH = WIDTH / TILE_SIZE;   // 64
		static constexpr int COARSE_HEIGHT = HEIGHT / TILE_SIZE; // 32

		CSoftwareOcclusionBuffer();
		~CSoftwareOcclusionBuffer() = default;

		/** Clear buffer to 1.0f and latch frame ViewProj + camera state.
		 * @param matViewProj: Render-space View * Projection matrix.
		 * @param vEyeWorld: Camera eye position in world space.
		 * @param vRenderOrigin: Scene render origin subtracted from world positions before matViewProj.
		 * @param clipSpaceZeroToOne: true for D3D/Vulkan/Metal/WebGPU [0,1], false for OpenGL [-1,1].
		 */
		void BeginFrame(const Matrix4& matViewProj, const Vector3& vEyeWorld, const Vector3& vRenderOrigin, bool clipSpaceZeroToOne);

		/** Enable/disable software occlusion culling globally. */
		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
		bool IsEnabled() const { return m_bEnabled; }

		/** Rasterize a solid opaque AABB given in world space. */
		void RasterizeOccluderWorldAABB(const Vector3& worldMin, const Vector3& worldMax);

		/** Rasterize a solid opaque AABB already shifted into render space (world - vRenderOrigin). */
		void RasterizeOccluderRenderAABB(const Vector3& renderMin, const Vector3& renderMax);

		/** Flush any remaining dirty coarse 4x4 Hi-Z tiles. */
		void FinalizeHierarchy();

		/** Conservative visibility query for a world-space AABB. Returns true ONLY if 100% occluded. */
		bool IsWorldAABBOccluded(const Vector3& worldMin, const Vector3& worldMax) const;
		bool IsWorldAABBOccluded(const CShapeAABB& worldAABB) const;
		inline bool IsWorldAABBOccluded(const CShapeBox& worldBox) const { return IsWorldAABBOccluded(worldBox.m_Min, worldBox.m_Max); }

		/** Conservative visibility query for a render-space AABB (world - vRenderOrigin). */
		bool IsRenderAABBOccluded(const Vector3& renderMin, const Vector3& renderMax) const;
		bool IsRenderAABBOccluded(const CShapeAABB& renderAABB) const;
		inline bool IsRenderAABBOccluded(const CShapeBox& renderBox) const { return IsRenderAABBOccluded(renderBox.m_Min, renderBox.m_Max); }

		/**Conservative visibility query for a world-space sphere (e.g., point/spot light volume). */
		bool IsWorldSphereOccluded(const Vector3& worldCenter, float radius) const;

		uint32_t GetOccluderCount() const { return m_occluderCount; }
		uint32_t GetCulledCount() const { return m_culledCount; }
		void ResetCulledCount() { m_culledCount = 0; }
		bool IsFrameValid() const { return m_bFrameValid; }
		void Reset()
		{
			m_bFrameValid = false;
			m_occluderCount = 0;
			m_culledCount = 0;
		}

	private:
		struct ScreenVertex
		{
			float sx;
			float sy;
			float sz;
		};

		Vector4 TransformClip(const Vector3& renderPos) const;
		ScreenVertex ClipToScreen(const Vector4& clip) const;
		void RasterizeClippedQuad(const Vector4 quadClip[4]);
		void RasterizeScreenTriangle(ScreenVertex v0, ScreenVertex v1, ScreenVertex v2);
		void RefreshCoarseTile(int tx, int ty) const;

	private:
		alignas(16) float m_depth[WIDTH * HEIGHT];
		alignas(16) mutable float m_coarseMaxDepth[COARSE_WIDTH * COARSE_HEIGHT];
		mutable uint64_t m_dirtyTileMask[COARSE_HEIGHT];

		Matrix4 m_matViewProj;
		Vector3 m_eyeRenderSpace;
		Vector3 m_renderOrigin;
		bool m_clipSpaceZeroToOne = true;
		bool m_bEnabled = true;
		bool m_bFrameValid = false;
		uint32_t m_occluderCount = 0;
		mutable uint32_t m_culledCount = 0;
	};
}

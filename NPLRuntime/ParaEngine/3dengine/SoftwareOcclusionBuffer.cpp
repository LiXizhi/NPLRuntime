//-----------------------------------------------------------------------------
// Class:	CSoftwareOcclusionBuffer
// Desc:	Conservative CPU Software Hi-Z Occlusion Buffer (SSE2 + scalar)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SoftwareOcclusionBuffer.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#if defined(_M_IX86) || defined(_M_X64) || defined(__SSE2__)
#include <emmintrin.h>
#define PARA_SOC_HAS_SSE2 1
#else
#define PARA_SOC_HAS_SSE2 0
#endif

namespace ParaEngine
{
	namespace
	{
		constexpr float kNearClipW = 0.05f;
		constexpr float kOccluderDepthEpsilon = 1.5e-4f;
		constexpr float kQueryDepthBias = 2.0e-4f;

		int ClipPolygonAgainstPlane(
			const Vector4* inVerts,
			int inCount,
			Vector4* outVerts,
			bool clipZeroToOne,
			bool clipNearZ)
		{
			if (inCount < 3)
				return 0;

			auto evalDist = [clipZeroToOne, clipNearZ](const Vector4& v) -> float {
				if (!clipNearZ)
					return v.w - kNearClipW;
				return clipZeroToOne ? v.z : (v.z + v.w);
			};

			int outCount = 0;
			Vector4 prev = inVerts[inCount - 1];
			float prevDist = evalDist(prev);

			for (int i = 0; i < inCount; ++i)
			{
				const Vector4& curr = inVerts[i];
				const float currDist = evalDist(curr);

				const bool prevInside = (prevDist >= 0.0f);
				const bool currInside = (currDist >= 0.0f);

				if (prevInside != currInside)
				{
					const float denom = prevDist - currDist;
					const float t = (std::fabs(denom) > 1e-8f) ? (prevDist / denom) : 0.0f;
					outVerts[outCount++] = Vector4(
						prev.x + (curr.x - prev.x) * t,
						prev.y + (curr.y - prev.y) * t,
						prev.z + (curr.z - prev.z) * t,
						prev.w + (curr.w - prev.w) * t);
				}
				if (currInside)
				{
					outVerts[outCount++] = curr;
				}

				prev = curr;
				prevDist = currDist;
			}
			return outCount;
		}
	}

	CSoftwareOcclusionBuffer::CSoftwareOcclusionBuffer()
	{
		std::fill_n(m_depth, WIDTH * HEIGHT, 1.0f);
		std::fill_n(m_coarseMaxDepth, COARSE_WIDTH * COARSE_HEIGHT, 1.0f);
		std::memset(m_dirtyTileMask, 0, sizeof(m_dirtyTileMask));
		m_matViewProj = Matrix4::IDENTITY;
		m_eyeRenderSpace = Vector3::ZERO;
		m_renderOrigin = Vector3::ZERO;
	}

	void CSoftwareOcclusionBuffer::BeginFrame(
		const Matrix4& matViewProj,
		const Vector3& vEyeWorld,
		const Vector3& vRenderOrigin,
		bool clipSpaceZeroToOne)
	{
		m_matViewProj = matViewProj;
		m_renderOrigin = vRenderOrigin;
		m_eyeRenderSpace = vEyeWorld - vRenderOrigin;
		m_clipSpaceZeroToOne = clipSpaceZeroToOne;
		m_occluderCount = 0;
		m_culledCount = 0;
		m_bFrameValid = m_bEnabled;

		if (!m_bEnabled)
			return;

#if PARA_SOC_HAS_SSE2
		const __m128 ones = _mm_set1_ps(1.0f);
		for (int i = 0; i < WIDTH * HEIGHT; i += 4)
			_mm_store_ps(&m_depth[i], ones);
		for (int i = 0; i < COARSE_WIDTH * COARSE_HEIGHT; i += 4)
			_mm_store_ps(&m_coarseMaxDepth[i], ones);
#else
		std::fill_n(m_depth, WIDTH * HEIGHT, 1.0f);
		std::fill_n(m_coarseMaxDepth, COARSE_WIDTH * COARSE_HEIGHT, 1.0f);
#endif
		std::memset(m_dirtyTileMask, 0, sizeof(m_dirtyTileMask));
	}

	Vector4 CSoftwareOcclusionBuffer::TransformClip(const Vector3& p) const
	{
		return Vector4(
			p.x * m_matViewProj.m[0][0] + p.y * m_matViewProj.m[1][0] + p.z * m_matViewProj.m[2][0] + m_matViewProj.m[3][0],
			p.x * m_matViewProj.m[0][1] + p.y * m_matViewProj.m[1][1] + p.z * m_matViewProj.m[2][1] + m_matViewProj.m[3][1],
			p.x * m_matViewProj.m[0][2] + p.y * m_matViewProj.m[1][2] + p.z * m_matViewProj.m[2][2] + m_matViewProj.m[3][2],
			p.x * m_matViewProj.m[0][3] + p.y * m_matViewProj.m[1][3] + p.z * m_matViewProj.m[2][3] + m_matViewProj.m[3][3]);
	}

	CSoftwareOcclusionBuffer::ScreenVertex CSoftwareOcclusionBuffer::ClipToScreen(const Vector4& clip) const
	{
		const float invW = 1.0f / clip.w;
		const float ndcX = clip.x * invW;
		const float ndcY = clip.y * invW;
		const float ndcZ = clip.z * invW;
		const float normZ = m_clipSpaceZeroToOne ? ndcZ : (ndcZ * 0.5f + 0.5f);

		ScreenVertex sv;
		sv.sx = (ndcX * 0.5f + 0.5f) * static_cast<float>(WIDTH);
		sv.sy = (0.5f - ndcY * 0.5f) * static_cast<float>(HEIGHT);
		sv.sz = (std::max)(0.0f, (std::min)(1.0f, normZ));
		return sv;
	}

	void CSoftwareOcclusionBuffer::RasterizeOccluderWorldAABB(const Vector3& worldMin, const Vector3& worldMax)
	{
		RasterizeOccluderRenderAABB(worldMin - m_renderOrigin, worldMax - m_renderOrigin);
	}

	void CSoftwareOcclusionBuffer::RasterizeOccluderRenderAABB(const Vector3& renderMin, const Vector3& renderMax)
	{
		if (!m_bFrameValid || !m_bEnabled)
			return;
		if (renderMax.x <= renderMin.x || renderMax.y <= renderMin.y || renderMax.z <= renderMin.z)
			return;

		// Skip occluder if camera eye is inside or grazing the box
		constexpr float kEyePad = 0.05f;
		if (m_eyeRenderSpace.x >= renderMin.x - kEyePad && m_eyeRenderSpace.x <= renderMax.x + kEyePad &&
			m_eyeRenderSpace.y >= renderMin.y - kEyePad && m_eyeRenderSpace.y <= renderMax.y + kEyePad &&
			m_eyeRenderSpace.z >= renderMin.z - kEyePad && m_eyeRenderSpace.z <= renderMax.z + kEyePad)
		{
			return;
		}

		Vector4 corners[8];
		corners[0] = TransformClip(Vector3(renderMin.x, renderMin.y, renderMin.z));
		corners[1] = TransformClip(Vector3(renderMax.x, renderMin.y, renderMin.z));
		corners[2] = TransformClip(Vector3(renderMin.x, renderMax.y, renderMin.z));
		corners[3] = TransformClip(Vector3(renderMax.x, renderMax.y, renderMin.z));
		corners[4] = TransformClip(Vector3(renderMin.x, renderMin.y, renderMax.z));
		corners[5] = TransformClip(Vector3(renderMax.x, renderMin.y, renderMax.z));
		corners[6] = TransformClip(Vector3(renderMin.x, renderMax.y, renderMax.z));
		corners[7] = TransformClip(Vector3(renderMax.x, renderMax.y, renderMax.z));

		// Up to 3 front-facing quads of the AABB relative to camera eye
		Vector4 quad[4];
		if (m_eyeRenderSpace.x < renderMin.x)
		{
			quad[0] = corners[0]; quad[1] = corners[2]; quad[2] = corners[6]; quad[3] = corners[4];
			RasterizeClippedQuad(quad);
		}
		else if (m_eyeRenderSpace.x > renderMax.x)
		{
			quad[0] = corners[1]; quad[1] = corners[5]; quad[2] = corners[7]; quad[3] = corners[3];
			RasterizeClippedQuad(quad);
		}

		if (m_eyeRenderSpace.y < renderMin.y)
		{
			quad[0] = corners[0]; quad[1] = corners[4]; quad[2] = corners[5]; quad[3] = corners[1];
			RasterizeClippedQuad(quad);
		}
		else if (m_eyeRenderSpace.y > renderMax.y)
		{
			quad[0] = corners[2]; quad[1] = corners[3]; quad[2] = corners[7]; quad[3] = corners[6];
			RasterizeClippedQuad(quad);
		}

		if (m_eyeRenderSpace.z < renderMin.z)
		{
			quad[0] = corners[0]; quad[1] = corners[1]; quad[2] = corners[3]; quad[3] = corners[2];
			RasterizeClippedQuad(quad);
		}
		else if (m_eyeRenderSpace.z > renderMax.z)
		{
			quad[0] = corners[4]; quad[1] = corners[6]; quad[2] = corners[7]; quad[3] = corners[5];
			RasterizeClippedQuad(quad);
		}

		++m_occluderCount;
	}

	void CSoftwareOcclusionBuffer::RasterizeClippedQuad(const Vector4 quadClip[4])
	{
		bool needsClip = false;
		bool allBehind = true;
		for (int i = 0; i < 4; ++i)
		{
			const float nearZ = m_clipSpaceZeroToOne ? quadClip[i].z : (quadClip[i].z + quadClip[i].w);
			if (quadClip[i].w > kNearClipW && nearZ >= 0.0f)
				allBehind = false;
			else
				needsClip = true;
		}
		if (allBehind)
			return;

		if (!needsClip)
		{
			const ScreenVertex s0 = ClipToScreen(quadClip[0]);
			const ScreenVertex s1 = ClipToScreen(quadClip[1]);
			const ScreenVertex s2 = ClipToScreen(quadClip[2]);
			const ScreenVertex s3 = ClipToScreen(quadClip[3]);
			RasterizeScreenTriangle(s0, s1, s2);
			RasterizeScreenTriangle(s0, s2, s3);
			return;
		}

		Vector4 bufA[8];
		Vector4 bufB[8];
		int count = ClipPolygonAgainstPlane(quadClip, 4, bufA, m_clipSpaceZeroToOne, false);
		if (count < 3)
			return;
		count = ClipPolygonAgainstPlane(bufA, count, bufB, m_clipSpaceZeroToOne, true);
		if (count < 3)
			return;

		ScreenVertex screenPoly[8];
		for (int i = 0; i < count; ++i)
			screenPoly[i] = ClipToScreen(bufB[i]);

		for (int i = 1; i + 1 < count; ++i)
			RasterizeScreenTriangle(screenPoly[0], screenPoly[i], screenPoly[i + 1]);
	}

	void CSoftwareOcclusionBuffer::RasterizeScreenTriangle(ScreenVertex v0, ScreenVertex v1, ScreenVertex v2)
	{
		float fArea = (v1.sx - v0.sx) * (v2.sy - v0.sy) - (v1.sy - v0.sy) * (v2.sx - v0.sx);
		if (std::fabs(fArea) < 1e-3f)
			return;
		if (fArea < 0.0f)
		{
			std::swap(v1, v2);
			fArea = -fArea;
		}

		const float minFX = (std::min)({ v0.sx, v1.sx, v2.sx });
		const float maxFX = (std::max)({ v0.sx, v1.sx, v2.sx });
		const float minFY = (std::min)({ v0.sy, v1.sy, v2.sy });
		const float maxFY = (std::max)({ v0.sy, v1.sy, v2.sy });

		if (maxFX < 0.0f || minFX >= static_cast<float>(WIDTH) ||
			maxFY < 0.0f || minFY >= static_cast<float>(HEIGHT))
		{
			return;
		}

		// Clamp extreme off-screen vertices to prevent 28.4 fixed-point overflow
		constexpr float kGuardLimit = 16384.0f;
		if (minFX < -kGuardLimit || maxFX > kGuardLimit || minFY < -kGuardLimit || maxFY > kGuardLimit)
			return;

		// 28.4 fixed-point coordinates for water-tight top-left edge evaluation across shared chunk seams
		const int32_t X0 = static_cast<int32_t>(std::floor(v0.sx * 16.0f + 0.5f));
		const int32_t Y0 = static_cast<int32_t>(std::floor(v0.sy * 16.0f + 0.5f));
		const int32_t X1 = static_cast<int32_t>(std::floor(v1.sx * 16.0f + 0.5f));
		const int32_t Y1 = static_cast<int32_t>(std::floor(v1.sy * 16.0f + 0.5f));
		const int32_t X2 = static_cast<int32_t>(std::floor(v2.sx * 16.0f + 0.5f));
		const int32_t Y2 = static_cast<int32_t>(std::floor(v2.sy * 16.0f + 0.5f));

		const int64_t iArea = static_cast<int64_t>(X1 - X0) * (Y2 - Y0) - static_cast<int64_t>(Y1 - Y0) * (X2 - X0);
		if (iArea <= 0)
			return;

		const int minPX = (std::max)(0, static_cast<int>(( (std::min)({ X0, X1, X2 }) - 8 ) >> 4));
		const int maxPX = (std::min)(WIDTH - 1, static_cast<int>(( (std::max)({ X0, X1, X2 }) + 8 ) >> 4));
		const int minPY = (std::max)(0, static_cast<int>(( (std::min)({ Y0, Y1, Y2 }) - 8 ) >> 4));
		const int maxPY = (std::min)(HEIGHT - 1, static_cast<int>(( (std::max)({ Y0, Y1, Y2 }) + 8 ) >> 4));
		if (minPX > maxPX || minPY > maxPY)
			return;

		// Depth plane equation z(sx, sy) = v0.sz + dzdx*(sx - v0.sx) + dzdy*(sy - v0.sy)
		const float invArea = 1.0f / fArea;
		const float dzdx = ((v1.sz - v0.sz) * (v2.sy - v0.sy) - (v2.sz - v0.sz) * (v1.sy - v0.sy)) * invArea;
		const float dzdy = ((v1.sx - v0.sx) * (v2.sz - v0.sz) - (v2.sx - v0.sx) * (v1.sz - v0.sz)) * invArea;
		// Conservative max-depth bias over a 1x1 pixel square around pixel center (px+0.5, py+0.5)
		const float conservativeBias = 0.5f * (std::fabs(dzdx) + std::fabs(dzdy)) + kOccluderDepthEpsilon;

		const int64_t dx01 = X1 - X0, dy01 = Y1 - Y0;
		const int64_t dx12 = X2 - X1, dy12 = Y2 - Y1;
		const int64_t dx20 = X0 - X2, dy20 = Y0 - Y2;

		const int64_t bias01 = ((dy01 < 0) || (dy01 == 0 && dx01 > 0)) ? 0 : -1;
		const int64_t bias12 = ((dy12 < 0) || (dy12 == 0 && dx12 > 0)) ? 0 : -1;
		const int64_t bias20 = ((dy20 < 0) || (dy20 == 0 && dx20 > 0)) ? 0 : -1;

		const int64_t startSampleX = static_cast<int64_t>(minPX) * 16 + 8;
		const int64_t startSampleY = static_cast<int64_t>(minPY) * 16 + 8;

		int64_t rowE01 = dx01 * (startSampleY - Y0) - dy01 * (startSampleX - X0) + bias01;
		int64_t rowE12 = dx12 * (startSampleY - Y1) - dy12 * (startSampleX - X1) + bias12;
		int64_t rowE20 = dx20 * (startSampleY - Y2) - dy20 * (startSampleX - X2) + bias20;

		const int64_t stepXE01 = -dy01 * 16, stepYE01 = dx01 * 16;
		const int64_t stepXE12 = -dy12 * 16, stepYE12 = dx12 * 16;
		const int64_t stepXE20 = -dy20 * 16, stepYE20 = dx20 * 16;

		float rowZ = v0.sz
			+ dzdx * ((static_cast<float>(minPX) + 0.5f) - v0.sx)
			+ dzdy * ((static_cast<float>(minPY) + 0.5f) - v0.sy)
			+ conservativeBias;

		for (int py = minPY; py <= maxPY; ++py)
		{
			int64_t e01 = rowE01;
			int64_t e12 = rowE12;
			int64_t e20 = rowE20;
			float z = rowZ;
			float* dstRow = &m_depth[py * WIDTH];
			int touchedMinX = WIDTH;
			int touchedMaxX = -1;

			for (int px = minPX; px <= maxPX; ++px)
			{
				if ((e01 | e12 | e20) >= 0)
				{
					const float clampedZ = (z > 1.0f) ? 1.0f : ((z < 0.0f) ? 0.0f : z);
					if (clampedZ < dstRow[px])
					{
						dstRow[px] = clampedZ;
						if (px < touchedMinX) touchedMinX = px;
						if (px > touchedMaxX) touchedMaxX = px;
					}
				}
				e01 += stepXE01;
				e12 += stepXE12;
				e20 += stepXE20;
				z += dzdx;
			}

			if (touchedMinX <= touchedMaxX)
			{
				const int tx0 = touchedMinX >> 2;
				const int tx1 = touchedMaxX >> 2;
				const uint64_t spanBits = (tx1 - tx0 == 63)
					? ~uint64_t(0)
					: (((uint64_t(1) << (tx1 - tx0 + 1)) - 1u) << tx0);
				m_dirtyTileMask[py >> 2] |= spanBits;
			}

			rowE01 += stepYE01;
			rowE12 += stepYE12;
			rowE20 += stepYE20;
			rowZ += dzdy;
		}
	}

	void CSoftwareOcclusionBuffer::RefreshCoarseTile(int tx, int ty) const
	{
		const int baseX = tx * TILE_SIZE;
		const int baseY = ty * TILE_SIZE;
#if PARA_SOC_HAS_SSE2
		const __m128 r0 = _mm_load_ps(&m_depth[(baseY + 0) * WIDTH + baseX]);
		const __m128 r1 = _mm_load_ps(&m_depth[(baseY + 1) * WIDTH + baseX]);
		const __m128 r2 = _mm_load_ps(&m_depth[(baseY + 2) * WIDTH + baseX]);
		const __m128 r3 = _mm_load_ps(&m_depth[(baseY + 3) * WIDTH + baseX]);
		__m128 m = _mm_max_ps(_mm_max_ps(r0, r1), _mm_max_ps(r2, r3));
		m = _mm_max_ps(m, _mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 3, 0, 1)));
		m = _mm_max_ps(m, _mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 0, 3, 2)));
		m_coarseMaxDepth[ty * COARSE_WIDTH + tx] = _mm_cvtss_f32(m);
#else
		float maxD = 0.0f;
		for (int dy = 0; dy < TILE_SIZE; ++dy)
		{
			const float* row = &m_depth[(baseY + dy) * WIDTH + baseX];
			for (int dx = 0; dx < TILE_SIZE; ++dx)
			{
				if (row[dx] > maxD)
					maxD = row[dx];
			}
		}
		m_coarseMaxDepth[ty * COARSE_WIDTH + tx] = maxD;
#endif
		m_dirtyTileMask[ty] &= ~(uint64_t(1) << tx);
	}

	void CSoftwareOcclusionBuffer::FinalizeHierarchy()
	{
		if (!m_bFrameValid)
			return;
		for (int ty = 0; ty < COARSE_HEIGHT; ++ty)
		{
			uint64_t mask = m_dirtyTileMask[ty];
			if (mask == 0)
				continue;
			for (int tx = 0; tx < COARSE_WIDTH && mask != 0; ++tx)
			{
				if (mask & (uint64_t(1) << tx))
				{
					RefreshCoarseTile(tx, ty);
					mask &= ~(uint64_t(1) << tx);
				}
			}
		}
	}

	bool CSoftwareOcclusionBuffer::IsWorldAABBOccluded(const Vector3& worldMin, const Vector3& worldMax) const
	{
		return IsRenderAABBOccluded(worldMin - m_renderOrigin, worldMax - m_renderOrigin);
	}

	bool CSoftwareOcclusionBuffer::IsWorldAABBOccluded(const CShapeAABB& worldAABB) const
	{
		Vector3 mn, mx;
		worldAABB.GetMin(mn);
		worldAABB.GetMax(mx);
		return IsWorldAABBOccluded(mn, mx);
	}

	bool CSoftwareOcclusionBuffer::IsRenderAABBOccluded(const CShapeAABB& renderAABB) const
	{
		Vector3 mn, mx;
		renderAABB.GetMin(mn);
		renderAABB.GetMax(mx);
		return IsRenderAABBOccluded(mn, mx);
	}

	bool CSoftwareOcclusionBuffer::IsWorldSphereOccluded(const Vector3& worldCenter, float radius) const
	{
		if (radius <= 0.0f)
			return false;
		const Vector3 ext(radius, radius, radius);
		return IsWorldAABBOccluded(worldCenter - ext, worldCenter + ext);
	}

	bool CSoftwareOcclusionBuffer::IsRenderAABBOccluded(const Vector3& renderMin, const Vector3& renderMax) const
	{
		if (!m_bFrameValid || !m_bEnabled || m_occluderCount == 0)
			return false;
		if (renderMax.x < renderMin.x || renderMax.y < renderMin.y || renderMax.z < renderMin.z)
			return false;

		// Never occlude an AABB containing or touching the camera eye
		constexpr float kEyePad = 0.1f;
		if (m_eyeRenderSpace.x >= renderMin.x - kEyePad && m_eyeRenderSpace.x <= renderMax.x + kEyePad &&
			m_eyeRenderSpace.y >= renderMin.y - kEyePad && m_eyeRenderSpace.y <= renderMax.y + kEyePad &&
			m_eyeRenderSpace.z >= renderMin.z - kEyePad && m_eyeRenderSpace.z <= renderMax.z + kEyePad)
		{
			return false;
		}

		float minSX = static_cast<float>(WIDTH);
		float maxSX = 0.0f;
		float minSY = static_cast<float>(HEIGHT);
		float maxSY = 0.0f;
		float minSZ = 1.0f;

		for (int i = 0; i < 8; ++i)
		{
			const Vector3 corner(
				(i & 1) ? renderMax.x : renderMin.x,
				(i & 2) ? renderMax.y : renderMin.y,
				(i & 4) ? renderMax.z : renderMin.z);
			const Vector4 clip = TransformClip(corner);
			const float nearZ = m_clipSpaceZeroToOne ? clip.z : (clip.z + clip.w);
			if (clip.w <= kNearClipW || nearZ < 0.0f)
			{
				// Intersects or extends behind near plane -> conservatively treat as visible
				return false;
			}
			const ScreenVertex sv = ClipToScreen(clip);
			if (sv.sx < minSX) minSX = sv.sx;
			if (sv.sx > maxSX) maxSX = sv.sx;
			if (sv.sy < minSY) minSY = sv.sy;
			if (sv.sy > maxSY) maxSY = sv.sy;
			if (sv.sz < minSZ) minSZ = sv.sz;
		}

		if (maxSX < 0.0f || minSX >= static_cast<float>(WIDTH) ||
			maxSY < 0.0f || minSY >= static_cast<float>(HEIGHT))
		{
			return false;
		}

		const float testMinZ = minSZ - kQueryDepthBias;
		if (testMinZ <= 0.0f)
			return false;

		// Conservative 1-pixel outward guard band on all 4 sides
		const int px0 = (std::max)(0, static_cast<int>(std::floor(minSX)) - 1);
		const int px1 = (std::min)(WIDTH - 1, static_cast<int>(std::floor(maxSX)) + 1);
		const int py0 = (std::max)(0, static_cast<int>(std::floor(minSY)) - 1);
		const int py1 = (std::min)(HEIGHT - 1, static_cast<int>(std::floor(maxSY)) + 1);
		if (px0 > px1 || py0 > py1)
			return false;

		const int tx0 = px0 >> 2;
		const int tx1 = px1 >> 2;
		const int ty0 = py0 >> 2;
		const int ty1 = py1 >> 2;

		for (int ty = ty0; ty <= ty1; ++ty)
		{
			const uint64_t dirtyRow = m_dirtyTileMask[ty];
			const int tileMinY = ty * TILE_SIZE;
			const int tileMaxY = tileMinY + (TILE_SIZE - 1);
			const bool fullTileY = (tileMinY >= py0 && tileMaxY <= py1);

			for (int tx = tx0; tx <= tx1; ++tx)
			{
				if (dirtyRow & (uint64_t(1) << tx))
					RefreshCoarseTile(tx, ty);

				const float tileMaxZ = m_coarseMaxDepth[ty * COARSE_WIDTH + tx];
				if (tileMaxZ < testMinZ)
				{
					// Entire 4x4 tile is strictly closer than testMinZ -> occluded!
					continue;
				}

				const int tileMinX = tx * TILE_SIZE;
				const int tileMaxX = tileMinX + (TILE_SIZE - 1);
				if (fullTileY && tileMinX >= px0 && tileMaxX <= px1)
				{
					// Entire 4x4 tile lies inside query rect, and at least one pixel in it has depth >= testMinZ!
					return false;
				}

				// Border tile: inspect only the overlapping fine pixels
				const int subX0 = (std::max)(px0, tileMinX);
				const int subX1 = (std::min)(px1, tileMaxX);
				const int subY0 = (std::max)(py0, tileMinY);
				const int subY1 = (std::min)(py1, tileMaxY);
				for (int py = subY0; py <= subY1; ++py)
				{
					const float* row = &m_depth[py * WIDTH];
					for (int px = subX0; px <= subX1; ++px)
					{
						if (row[px] >= testMinZ)
							return false;
					}
				}
			}
		}

		++m_culledCount;
		return true;
	}
}

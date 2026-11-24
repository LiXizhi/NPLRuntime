// World-aligned surface LOD for the interactive super render distance.
#include "ParaEngine.h"
#include "BlockWorldClient.h"
#include "BlockRegion.h"
#include "EffectManager.h"
#include "SceneObject.h"
#include "MultiFrameBlockWorldRenderer.h"

using namespace ParaEngine;

namespace
{
	// Region indices are six bits, each region is 512 blocks wide.
	constexpr int WorldWidth = 64 * 512;
	constexpr int MinCell = 4;
	constexpr int CellsPerViewRadius = 24;
	constexpr int BatchQuads = 256;
	constexpr int Empty = BlockConfig::MinBlockY - 1;

	class TerrainPass
	{
	public:
		TerrainPass(BlockWorldClient& world, CBaseCamera& camera, int distance, bool hasRetainedHeights)
			: world(world), camera(camera), radius(distance), hasRetainedHeights(hasRetainedHeights), origin(CGlobals::GetScene()->GetRenderOrigin()),
			eye(camera.GetEyePosition()), ahead(camera.GetWorldAhead()), perspective(camera.IsPerspectiveView()),
			fog(CGlobals::GetScene()->GetFogColor()), fogEnabled(CGlobals::GetScene()->IsFogEnabled()),
			light(0.25f + 0.75f * Math::Clamp(world.GetSunIntensity(), 0.f, 1.f))
		{
			eye.y -= world.GetVerticalOffset();
			eye *= float(BlockConfig::g_dBlockSizeInverse);
			// Overlap one chunk at the handoff. The detailed terrain already owns the depth buffer.
			nearRadius = (std::max)(0, world.GetRenderDist() - 16);
		}

		void Draw()
		{
			Visit(0, 0, WorldWidth);
			Flush();
		}

	private:
		float DistanceXZ(int x, int z, int size) const
		{
			return (std::max)(std::abs(eye.x - (x + size * 0.5f)),
				std::abs(eye.z - (z + size * 0.5f))) - size * 0.5f;
		}

		bool Split(int x, int z, int size) const
		{
			return size > MinCell && DistanceXZ(x, z, size) < size * CellsPerViewRadius;
		}

		bool Visible(int x, int z, int size) const
		{
			const auto& frustum = *camera.GetFrustum();
			const float scale = BlockConfig::g_blockSize;
			// Keep the scene's near and side planes. Its far plane belongs to the detailed renderer.
			for (int i = 0; i < 5; ++i)
			{
				const int mask = frustum.nVertexLUT[i];
				Vector3 corner(((mask & 1) ? x : x + size) * scale,
					world.GetVerticalOffset() + ((mask & 2) ? BlockConfig::MinBlockY : BlockConfig::MaxBlockY + 1) * scale,
					((mask & 4) ? z : z + size) * scale);
				if (frustum.planeFrustum[i].PlaneDotCoord(corner - origin) < 0)
					return false;
			}
			return true;
		}

		// Missing regions may still have the world's compact, retained column heights.
		// Never load a region, initialize lighting or touch the detailed chunk mesh pool.
		int Sample(int x, int z, DWORD* color = nullptr) const
		{
			if (x < 0 || z < 0 || x >= WorldWidth || z >= WorldWidth) return Empty;
			auto* height = world.GetHighestBlock(x, z);
			if (!height || height->m_nMaxHeight <= Empty) return Empty;
			int y = height->m_nMaxHeight;
			if (auto* region = world.GetRegion(x >> 9, z >> 9))
			{
				if (!region->IsLocked())
				{
					auto* block = region->GetBlock(x & 511, y, z & 511);
					if (block && block->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_invisible))
					{
						y = height->m_nSolidHeight;
						block = region->GetBlock(x & 511, y, z & 511);
					}
					if (!block || block->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_invisible)) return Empty;
					if (color) *color = block->GetTemplate()->GetBlockColor(block->GetUserData());
					return y;
				}
			}
			if (color) *color = 0xff858677; // Only the silhouette survives region eviction.
			return y;
		}

		void Visit(int x, int z, int size)
		{
			if (DistanceXZ(x, z, size) > radius || !Visible(x, z, size)) return;
			if (size == BlockConfig::g_regionBlockDimX && !hasRetainedHeights && !world.GetRegion(x >> 9, z >> 9))
				return;
			const int half = size / 2;
			const bool fromLeft = perspective ? eye.x < x + half : ahead.x > 0;
			const bool fromFront = perspective ? eye.z < z + half : ahead.z > 0;
			if (Split(x, z, size))
			{
				// Each split plane separates the nearer pair of children from the farther pair.
				// Painter order lets all far geometry share the sky depth, with no extra depth target.
				const int farX = fromLeft ? x + half : x, nearX = fromLeft ? x : x + half;
				const int farZ = fromFront ? z + half : z, nearZ = fromFront ? z : z + half;
				Visit(farX, farZ, half);
				Visit(nearX, farZ, half);
				Visit(farX, nearZ, half);
				Visit(nearX, nearZ, half);
				return;
			}

			DWORD material;
			const int height = Sample(x + half, z + half, &material);
			if (height == Empty) return;
			const int top = height + 1;
			const Vector3 offset(x + half - eye.x, top - eye.y, z + half - eye.z);
			const float distance = offset.length();
			if (distance + size < nearRadius || distance - size > radius) return;

			const float haze = fogEnabled ? Math::Clamp((distance / radius - 0.5f) * 1.8f, 0.f, 1.f) : 0.f;
			LinearColor tint(material);
			auto shade = [&](float brightness) -> DWORD {
				return (tint * (light * brightness * (1 - haze)) + fog * haze).CopyWithNewOpacity(1.f);
			};
			const Vector3 a = BlockCommon::ConvertToRealPosition(x, top, z, 7) - origin;
			const float width = size * BlockConfig::g_blockSize;
			const Vector3 b(a.x + width, a.y, a.z), c(b.x, a.y, a.z + width), d(a.x, a.y, c.z);
			const bool above = perspective ? eye.y >= top : ahead.y <= 0;
			Quad(a, b, c, d, shade(above ? 1.f : 0.76f));
			const Uint16x3 surface(x + half, height, z + half);

			if (!perspective || eye.x <= x || eye.x >= x + size)
				Side(x + (fromLeft ? -size : size), z, size, true, fromLeft, surface,
					fromLeft ? a : b, fromLeft ? d : c, shade(0.76f));
			if (!perspective || eye.z <= z || eye.z >= z + size)
				Side(x, z + (fromFront ? -size : size), size, false, fromFront, surface,
					fromFront ? a : d, fromFront ? b : c, shade(0.88f));
		}

		void Side(int x, int z, int size, bool alongZ, bool negativeSide, const Uint16x3& surface,
			const Vector3& a, const Vector3& b, DWORD color)
		{
			// Look up the actual neighboring leaf. At a transition its edge is two half-edges.
			if (Split(x, z, size))
			{
				const int half = size / 2;
				if (negativeSide)
				{
					if (alongZ) x += half;
					else z += half;
				}
				const Vector3 mid = (a + b) * 0.5f;
				Side(x, z, half, alongZ, negativeSide, surface, a, mid, color);
				Side(x + (alongZ ? 0 : half), z + (alongZ ? half : 0), half, alongZ, negativeSide, surface, mid, b, color);
				return;
			}
			while (size < WorldWidth && x >= 0 && z >= 0 && x < WorldWidth && z < WorldWidth)
			{
				const int parent = size * 2;
				const int px = x & -parent, pz = z & -parent;
				if (Split(px, pz, parent)) break;
				x = px;
				z = pz;
				size = parent;
			}
			const int neighbor = Sample(x + size / 2, z + size / 2);
			const int top = surface.y + 1;
			int bottom = neighbor + 1;
			if (neighbor == Empty)
			{
				// At a void edge show the measured side of the column. Limit probes to the
				// current detail level so even very tall or suspended structures are cheap.
				bottom = surface.y;
				auto* region = world.GetRegion(surface.x >> 9, surface.z >> 9);
				if (region && !region->IsLocked())
				{
					const int limit = (std::min)(top - BlockConfig::MinBlockY, (std::min)(256, size * 4));
					const int air = world.FindFirstBlock(surface.x, surface.y, surface.z, 5, limit, 0);
					bottom = top - (air < 0 ? limit : air);
				}
			}
			if (bottom >= top) return;
			const Vector3 drop(0, (top - bottom) * BlockConfig::g_blockSize, 0);
			Quad(a, b, b - drop, a - drop, color);
		}

		void Quad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, DWORD color)
		{
			vertices[count++] = {a, color};
			vertices[count++] = {b, color};
			vertices[count++] = {c, color};
			vertices[count++] = {d, color};
			if (count == vertices.size()) Flush();
		}

		void Flush()
		{
			if (!count) return;
			static const auto indices = [] {
				std::array<uint16_t, 6 * BatchQuads> buffer;
				for (int i = 0; i < BatchQuads; ++i)
				{
					const uint16_t base = i * 4;
					const uint16_t quad[] = {base, uint16_t(base + 1), uint16_t(base + 2),
						base, uint16_t(base + 2), uint16_t(base + 3)};
					std::copy(quad, quad + 6, buffer.begin() + i * 6);
				}
				return buffer;
			}();
			CGlobals::GetRenderDevice()->DrawIndexedPrimitiveUP(EPrimitiveType::TRIANGLELIST, 0,
				count, count / 2, indices.data(), PixelFormat::INDEX16, vertices.data(), sizeof(LINEVERTEX));
			count = 0;
		}

		BlockWorldClient& world;
		CBaseCamera& camera;
		float radius, nearRadius;
		bool hasRetainedHeights;
		Vector3 origin, eye, ahead;
		bool perspective;
		LinearColor fog;
		bool fogEnabled;
		float light;
		std::array<LINEVERTEX, 4 * BatchQuads> vertices;
		unsigned count = 0;
	};
}

bool CMultiFrameBlockWorldRenderer::DrawLiveTerrain()
{
	auto* scene = CGlobals::GetScene();
	auto* camera = scene->GetCurrentCamera();
	auto* effects = CGlobals::GetEffectManager();
	auto* device = CGlobals::GetRenderDevice();
	CEffectFile* effect = nullptr;
	if (!camera || !device || !effects->BeginEffect(TECH_SINGLE_COLOR, &effect) || !effect) return false;
	if (!effect->begin())
	{
		effects->EndEffect();
		return false;
	}
	const bool ready = effect->BeginPass(0);
	if (ready)
	{
		Matrix4 matrix = effects->GetViewTransform() * effects->GetProjTransform();
		// Preserve the active viewport's perspective, stereo offsets and orthographic scale.
		// Project to sky depth so all foreground geometry retains its original depth precision.
		matrix._13 = matrix._14;
		matrix._23 = matrix._24;
		matrix._33 = matrix._34;
		matrix._43 = matrix._44;
		effect->setMatrix(CEffectFileBase::k_worldViewProjMatrix, &matrix);
		effect->CommitChanges();
		const auto cull = device->GetRenderState(ERenderState::CULLMODE);
		const auto compare = device->GetRenderState(ERenderState::ZFUNC);
		const auto write = device->GetRenderState(ERenderState::ZWRITEENABLE);
		device->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
		device->SetRenderState(ERenderState::ZFUNC, RSV_CMP_LESSEQUAL);
		device->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		{
			Scoped_ReadLock<BlockReadWriteLock> lock(m_pWorld->GetReadWriteLock());
			TerrainPass pass(*m_pWorld, *camera, m_nRenderDistance, !m_pWorld->m_unloadedColumnHeights.empty());
			pass.Draw();
		}
		device->SetRenderState(ERenderState::ZWRITEENABLE, write);
		device->SetRenderState(ERenderState::ZFUNC, compare);
		device->SetRenderState(ERenderState::CULLMODE, cull);
		effect->EndPass();
	}
	effect->end();
	effects->EndEffect();
	return ready;
}

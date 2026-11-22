#include "3dengine/CharCapeCore.h"
#include <cmath>
#include <cstdio>
#include <vector>

using namespace ParaEngine;

#define CHECK(condition) \
	do { \
		if (!(condition)) { \
			std::fprintf(stderr, "CapePhysicsTest FAILED at line %d: %s\n", __LINE__, #condition); \
			return 1; \
		} \
	} while (0)

static bool IsFiniteVec(const CapeVec3& v)
{
	return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

int main()
{
	CHECK(CAPE_SYSTEM_VERSION >= 1);

	const CapeVec3 neck = { 0.0f, 1.32f, 0.0f };
	const CapeVec3 leftSh = { 0.0f, 1.28f, 0.16f };
	const CapeVec3 rightSh = { 0.0f, 1.28f, -0.16f };
	const CapeVec3 fwd = { 1.0f, 0.0f, 0.0f };
	const CapeVec3 up = { 0.0f, 1.0f, 0.0f };
	const CapeVec3 right = { 0.0f, 0.0f, -1.0f };

	// 1. Verify all 3 collar styles x 6 hem styles generate valid, finite, outward-facing meshes
	const CapeCollarStyle collars[] = {
		CapeCollarStyle::WrapPoncho,
		CapeCollarStyle::ClassicBack,
		CapeCollarStyle::Scarf
	};
	const CapeHemStyle hems[] = {
		CapeHemStyle::Rounded,
		CapeHemStyle::Scalloped,
		CapeHemStyle::StarPointed,
		CapeHemStyle::Swallowtail,
		CapeHemStyle::Pointed,
		CapeHemStyle::Asymmetric
	};

	for (CapeCollarStyle cs : collars)
	{
		for (CapeHemStyle hs : hems)
		{
			CapeStyleParams style;
			style.collarStyle = cs;
			style.hemStyle = hs;
			style.gridCols = 9;
			style.gridRows = 7;

			CapeClothMesh mesh;
			mesh.BuildProcedural(style, neck, leftSh, rightSh, fwd, up, right, 1.0f);
			CHECK(!mesh.particles.empty());
			CHECK(!mesh.constraints.empty());
			CHECK(!mesh.triIndices.empty());

			std::vector<CapeSurfaceVertex> outerTris, innerTris;
			mesh.BuildSurfaceTriangles(0.0f, neck, neck, outerTris, &innerTris);
			CHECK(outerTris.size() == mesh.triIndices.size());
			CHECK(innerTris.size() == mesh.triIndices.size());

			for (size_t i = 0; i < outerTris.size(); ++i)
			{
				CHECK(IsFiniteVec(outerTris[i].pos));
				CHECK(IsFiniteVec(outerTris[i].normal));
				CHECK(outerTris[i].uv.x >= -1e-4f && outerTris[i].uv.x <= 1.0001f);
				CHECK(outerTris[i].uv.y >= -1e-4f && outerTris[i].uv.y <= 1.0001f);
				const float nLen = outerTris[i].normal.Length();
				CHECK(nLen > 0.98f && nLen < 1.02f);
			}

			// Center back quad normal (middle row, middle column) must point toward -X (away from character back)
			const size_t midRow = static_cast<size_t>((mesh.rows - 1) / 2);
			const size_t midCol = static_cast<size_t>((mesh.cols - 1) / 2);
			const size_t centerQuadVertIdx = (midRow * static_cast<size_t>(mesh.cols - 1) + midCol) * 6;
			const CapeSurfaceVertex& midVert = outerTris[centerQuadVertIdx];
			CHECK(midVert.normal.x < -0.2f);
		}
	}

	// 2. Verify XPBD simulation: idle settle, running billow, flight lift, tether limit, and capsule non-penetration
	{
		CapeStyleParams style;
		style.collarStyle = CapeCollarStyle::WrapPoncho;
		style.hemStyle = CapeHemStyle::Scalloped;
		CapeSimTuning tuning;
		CapeClothMesh cloth;
		cloth.BuildProcedural(style, neck, leftSh, rightSh, fwd, up, right, 1.0f);

		CapeCapsule capsules[3] = {
			{ { 0.0f, 0.90f, 0.0f }, { 0.0f, 1.28f, 0.0f }, 0.16f }, // Chest
			{ { 0.0f, 0.45f, 0.09f }, { 0.0f, 0.88f, 0.09f }, 0.11f }, // Left leg
			{ { 0.0f, 0.45f, -0.09f }, { 0.0f, 0.88f, -0.09f }, 0.11f }, // Right leg
		};

		CapeMotionInput idleMotion;
		for (int f = 0; f < 60; ++f)
		{
			idleMotion.timeSeconds = f * (1.0f / 60.0f);
			cloth.StepSimulation(1.0f / 60.0f, tuning, idleMotion, neck, leftSh, rightSh, neck, capsules, 3, 4);
		}

		const size_t hemCenterIdx = static_cast<size_t>((cloth.rows - 1) * cloth.cols + cloth.cols / 2);
		const CapeVec3 idleHemPos = cloth.particles[hemCenterIdx].pos;
		CHECK(IsFiniteVec(idleHemPos));

		// Verify zero capsule penetration across all free particles
		for (size_t i = static_cast<size_t>(cloth.cols); i < cloth.particles.size(); ++i)
		{
			for (const CapeCapsule& cap : capsules)
			{
				const CapeVec3 ab = cap.b - cap.a;
				const float t = (std::max)(0.0f, (std::min)(1.0f, (cloth.particles[i].pos - cap.a).Dot(ab) / ab.LengthSq()));
				const float dist = (cloth.particles[i].pos - (cap.a + ab * t)).Length();
				CHECK(dist >= cap.radius - 1e-3f);
			}
		}

		// Running forward (+X) + flying should stream the hem further backward (-X) and higher (+Y) than idle
		CapeMotionInput flyMotion;
		flyMotion.isFlying = true;
		flyMotion.localVelocity = { 6.5f, 0.0f, 0.0f };
		for (int f = 0; f < 60; ++f)
		{
			flyMotion.timeSeconds = 1.0f + f * (1.0f / 60.0f);
			cloth.StepSimulation(1.0f / 60.0f, tuning, flyMotion, neck, leftSh, rightSh, neck, capsules, 3, 4);
		}
		const CapeVec3 flyHemPos = cloth.particles[hemCenterIdx].pos;
		CHECK(IsFiniteVec(flyHemPos));
		CHECK(flyHemPos.x < idleHemPos.x - 0.08f);
		CHECK(flyHemPos.y > idleHemPos.y + 0.08f);

		// Verify tether max-stretch clamp holds even under extreme sprint impulse
		const CapeParticle& hemPt = cloth.particles[hemCenterIdx];
		const CapeVec3& topAnchor = cloth.particles[static_cast<size_t>(hemPt.anchorIndex)].pos;
		CHECK((hemPt.pos - topAnchor).Length() <= hemPt.tetherMaxDist * tuning.stretchLimit + 0.02f);

		// Verify teleport flag resets cleanly without NaN or stretch explosion
		CapeMotionInput teleportMotion;
		teleportMotion.teleported = true;
		const CapeVec3 shiftedNeck = { 100.0f, 1.32f, -50.0f };
		cloth.StepSimulation(1.0f / 60.0f, tuning, teleportMotion, shiftedNeck,
			shiftedNeck + CapeVec3{ 0.0f, -0.04f, 0.16f },
			shiftedNeck + CapeVec3{ 0.0f, -0.04f, -0.16f },
			neck, capsules, 3, 4);
		CHECK(IsFiniteVec(cloth.particles[hemCenterIdx].pos));
		CHECK((cloth.particles[hemCenterIdx].pos - shiftedNeck).Length() < 2.0f);
	}

	// 3. Verify Bone Swing-Twist Shortest-Arc Rotation Matrix
	{
		const CapeVec3 from = { 0.0f, -1.0f, 0.0f };
		const CapeVec3 to = CapeVec3{ -0.6f, -0.8f, 0.15f }.Normalized();
		const CapeMat3 rot = CapeMat3::FromTwoVectors(from, to);
		const CapeVec3 mapped = rot.Transform(from);
		CHECK((mapped - to).Length() < 1e-4f);
	}

	// 4. Verify CapeSimulationBudget distance hysteresis, Top-N crowd cap, and main-player priority
	{
		CapeSimulationBudget& budget = CapeSimulationBudget::GetInstance();
		budget.ResetForTest();
		budget.SetMaxActiveCharacters(4);
		budget.SetActivateDistance(18.0f);
		budget.SetDeactivateDistance(21.0f);

		bool activeA = false;
		float blendA = 0.0f;

		// Distant character (25m) stays completely static (0 blend)
		budget.BeginFrame(60.0, 1.0 / 60.0);
		CHECK(!budget.EvaluateInstance(101, 25.0f, false, 0.05f, activeA, blendA));
		CHECK(blendA == 0.0f);

		// Nearby character (10m) activates and ramps blend smoothly
		for (int i = 0; i < 10; ++i)
		{
			budget.BeginFrame(60.0, 0.05);
			CHECK(budget.EvaluateInstance(101, 10.0f, false, 0.05f, activeA, blendA));
		}
		CHECK(activeA);
		CHECK(blendA > 0.99f);

		// Hysteresis band: at 19.5m (between 18m and 21m), an already-active cape stays active!
		budget.BeginFrame(60.0, 0.05);
		CHECK(budget.EvaluateInstance(101, 19.5f, false, 0.05f, activeA, blendA));
		CHECK(activeA);

		// Beyond 21m (e.g. 22.5m), it smoothly blends out to 0
		for (int i = 0; i < 10; ++i)
		{
			budget.BeginFrame(60.0, 0.05);
			budget.EvaluateInstance(101, 22.5f, false, 0.05f, activeA, blendA);
		}
		CHECK(!activeA);
		CHECK(blendA == 0.0f);

		// Crowd test: 20 nearby characters + 1 high-priority main player -> strictly caps active targets to 4, keeping main player
		bool states[21] = {};
		float blends[21] = {};
		for (int step = 0; step < 8; ++step)
		{
			budget.BeginFrame(60.0, 0.1);
			for (int id = 0; id < 20; ++id)
			{
				budget.EvaluateInstance(1000 + static_cast<uintptr_t>(id), 3.0f + id * 0.5f, false, 0.1f, states[id], blends[id]);
			}
			// Main player at index 20 (even at 12m, beats closer NPCs due to highPriority)
			budget.EvaluateInstance(9999, 12.0f, true, 0.1f, states[20], blends[20]);
		}
		budget.BeginFrame(60.0, 0.1);
		int activeTargets = 0;
		for (int i = 0; i < 21; ++i)
		{
			if (states[i]) ++activeTargets;
		}
		CHECK(states[20]); // Main player guaranteed active
		CHECK(activeTargets <= 4);

		// Verify RGBA pattern synthesizer produces opaque, valid pixels with glowing emblem contrast
		std::vector<uint8_t> rgba;
		SynthesizeCapePatternRGBA(64, 64, 0xffe85d3f, 0xffc9184a, 0xfffff3b0, 0.06f, true, 0xffffffff, 0.22f, rgba);
		CHECK(rgba.size() == 64 * 64 * 4);
		CHECK(rgba[3] == 255); // Opaque alpha
		budget.ResetForTest();
	}

	std::puts("Cape physics & simulation budget checks passed");
	return 0;
}

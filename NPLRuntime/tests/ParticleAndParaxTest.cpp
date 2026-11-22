//-----------------------------------------------------------------------------
// File:	ParticleAndParaxTest.cpp
// Desc:	Unit & round-trip regression tests for v2 ParticleSystem, RibbonEmitter,
//			procedural builtin:// textures, binary ParaX (.x), and GLB (extras.parax).
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Core/ParaEngineAppBase.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/particle.h"
#include "ParaXModel/XFileCharModelParser.h"
#include "ParaXModel/XFileCharModelExporter.h"
#include "ParaXModel/GltfModel.h"
#include "ParaXModel/ParaXGltfExtras.h"

#include <boost/filesystem.hpp>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace fs = boost::filesystem;
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

	template <typename T>
	void InitConstAnim(Animated<T>& anim, const T& val, size_t nAnims = 1)
	{
		anim.used = false;
		anim.type = INTERPOLATION_NONE;
		anim.seq = -1;
		anim.times.assign(1, 0);
		anim.data.assign(1, val);
		anim.ranges.assign(nAnims, AnimRange(0, 0));
	}

	void InitConstAnimShort(AnimatedShort& anim, float val, size_t nAnims = 1)
	{
		anim.used = false;
		anim.type = INTERPOLATION_NONE;
		anim.seq = -1;
		anim.times.assign(1, 0);
		anim.data.assign(1, val);
		anim.ranges.assign(nAnims, AnimRange(0, 0));
	}

	bool ReadFileBytes(const std::string& path, std::vector<char>& outBytes)
	{
		std::ifstream ifs(path, std::ios::binary | std::ios::ate);
		if (!ifs.is_open())
			return false;
		std::streamsize sz = ifs.tellg();
		if (sz <= 0)
			return false;
		outBytes.resize(static_cast<size_t>(sz) + 1);
		ifs.seekg(0, std::ios::beg);
		ifs.read(outBytes.data(), sz);
		outBytes[static_cast<size_t>(sz)] = '\0';
		return ifs.good() || ifs.eof();
	}

	void TestProceduralBuiltinTextures()
	{
		TEST_CHECK(IsBuiltinParticleTextureName("builtin://glow"));
		TEST_CHECK(IsBuiltinParticleTextureName("builtin://spark"));
		TEST_CHECK(IsBuiltinParticleTextureName("builtin://star"));
		TEST_CHECK(IsBuiltinParticleTextureName("builtin://ribbon"));
		TEST_CHECK(!IsBuiltinParticleTextureName("Texture/Aries/Particle.png"));

		const char* kinds[] = {
			"builtin://glow",
			"builtin://spark",
			"builtin://star",
			"builtin://flare",
			"builtin://ring",
			"builtin://smoke",
			"builtin://ribbon",
		};
		const unsigned char pngMagic[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };

		for (const char* kind : kinds)
		{
			std::vector<char> pngBytes;
			TEST_CHECK(BuildProceduralParticlePNG(kind, pngBytes));
			TEST_CHECK(pngBytes.size() > 64);
			if (pngBytes.size() >= 26)
			{
				TEST_CHECK(std::memcmp(pngBytes.data(), pngMagic, 8) == 0);
				TEST_CHECK(std::memcmp(pngBytes.data() + 12, "IHDR", 4) == 0);
				// Width = 64, Height = 64 (big-endian)
				TEST_CHECK(static_cast<unsigned char>(pngBytes[19]) == 64);
				TEST_CHECK(static_cast<unsigned char>(pngBytes[23]) == 64);
				// Bit depth = 8, Color type = 6 (RGBA)
				TEST_CHECK(static_cast<unsigned char>(pngBytes[24]) == 8);
				TEST_CHECK(static_cast<unsigned char>(pngBytes[25]) == 6);
			}
		}

		auto tex1 = LoadOrCreateBuiltinParticleTexture("builtin://glow");
		TEST_CHECK(tex1.get() != nullptr);
		if (tex1)
		{
			TEST_CHECK(tex1->GetRawData() != nullptr);
			TEST_CHECK(tex1->GetRawDataSize() > 64);
			TEST_CHECK(!tex1->IsEmbeddedTexture());
		}

		// Verify destroying a temporary CParaXModel referencing builtin://glow keeps shared raw data intact
		{
			ParaXHeaderDef hdr{};
			std::memcpy(hdr.id, "PARAX", 4);
			hdr.version[0] = 1;
			CParaXModel tmpModel(hdr);
			tmpModel.m_objNum.nTextures = 1;
			tmpModel.textures = new asset_ptr<TextureEntity>[1];
			tmpModel.textures[0] = tex1;
		}
		TEST_CHECK(tex1 && tex1->GetRawData() != nullptr && tex1->GetRawDataSize() > 64);
	}

	void TestZeroRateSilenceAndEmitterShapes()
	{
		SceneState sceneState;
		char dummyObjA = 0;
		sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&dummyObjA));

		// 1. Regression check: rate == 0 with empty particles must spawn ZERO particles
		ParticleSystem silentSys;
		silentSys.type = 0;
		silentSys.SetTextureRowsCols(1, 1);
		InitConstAnim(silentSys.rate, 0.0f);
		InitConstAnim(silentSys.lifespan, 1.0f);
		InitConstAnim(silentSys.speed, 3.0f);
		InitConstAnim(silentSys.variation, 0.0f);
		InitConstAnim(silentSys.spread, 0.2f);
		InitConstAnim(silentSys.lat, 0.2f);
		InitConstAnim(silentSys.gravity, 0.0f);
		InitConstAnim(silentSys.areal, 0.0f);
		InitConstAnim(silentSys.areaw, 0.0f);
		InitConstAnim(silentSys.grav2, 0.0f);
		silentSys.emitter = ParticleSystem::CreateEmitterByType(&silentSys, ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER);

		for (int frame = 0; frame < 10; ++frame)
		{
			ParticleList* inst = silentSys.GetCurrentInstance(&sceneState, true);
			inst->m_bUpdated = false;
			silentSys.setup(&sceneState, 0, frame * 16);
			silentSys.update(&sceneState, 0.016f);
			TEST_CHECK(inst->particles.empty());
		}

		// 2. SphereShellParticleEmitter: thin crisp shell (shellThickness == 0 -> exact speed magnitude)
		ParticleSystem shellSys;
		shellSys.SetTextureRowsCols(1, 1);
		InitConstAnim(shellSys.speed, 10.0f);
		InitConstAnim(shellSys.variation, 0.0f);
		InitConstAnim(shellSys.lifespan, 1.5f);
		InitConstAnim(shellSys.rate, 0.0f);
		InitConstAnim(shellSys.gravity, 0.0f);
		InitConstAnim(shellSys.areal, 0.0f);
		InitConstAnim(shellSys.areaw, 0.0f);
		InitConstAnim(shellSys.spread, 0.0f);
		InitConstAnim(shellSys.lat, 0.0f);
		InitConstAnim(shellSys.grav2, 0.0f);
		shellSys.shellThickness = 0.0f;
		shellSys.minPolar = 0.0f;
		shellSys.maxPolar = 3.14159265f;
		shellSys.emitter = ParticleSystem::CreateEmitterByType(&shellSys, ParticleEmitter::TYPE_SPHERE_SHELL_EMITTER);
		TEST_CHECK(shellSys.emitter != nullptr);
		for (int i = 0; i < 16; ++i)
		{
			Particle p = shellSys.emitter->newParticle(0, 0);
			float spd = p.speed.length();
			TEST_CHECK(std::fabs(spd - 10.0f) < 0.05f);
		}

		// 3. PatternParticleEmitter: ring, heart, star5, double_ring, spiral
		ParticleSystem patSys;
		patSys.SetTextureRowsCols(1, 1);
		InitConstAnim(patSys.speed, 6.0f);
		InitConstAnim(patSys.variation, 0.0f);
		InitConstAnim(patSys.lifespan, 1.2f);
		InitConstAnim(patSys.rate, 0.0f);
		InitConstAnim(patSys.gravity, 0.0f);
		InitConstAnim(patSys.areal, 0.0f);
		InitConstAnim(patSys.areaw, 0.0f);
		InitConstAnim(patSys.spread, 0.0f);
		InitConstAnim(patSys.lat, 0.0f);
		InitConstAnim(patSys.grav2, 0.0f);
		patSys.emitDir = Vector3(0.0f, 0.0f, 1.0f);
		patSys.emitter = ParticleSystem::CreateEmitterByType(&patSys, ParticleEmitter::TYPE_PATTERN_PARTICLE_EMITTER);
		TEST_CHECK(patSys.emitter != nullptr);
		for (int sub = 0; sub <= ParticleEmitter::PATTERN_SPIRAL; ++sub)
		{
			patSys.patternType = static_cast<int16>(sub);
			Particle p = patSys.emitter->newParticle(0, 0);
			TEST_CHECK(p.speed.length() > 0.5f);
			TEST_CHECK(std::isfinite(p.speed.x) && std::isfinite(p.speed.y) && std::isfinite(p.speed.z));
		}

		// 4. ConeJetParticleEmitter: narrow vertical jet along +Y
		ParticleSystem coneSys;
		coneSys.SetTextureRowsCols(1, 1);
		InitConstAnim(coneSys.speed, 12.0f);
		InitConstAnim(coneSys.variation, 0.0f);
		InitConstAnim(coneSys.lifespan, 1.0f);
		InitConstAnim(coneSys.rate, 0.0f);
		InitConstAnim(coneSys.gravity, 0.0f);
		InitConstAnim(coneSys.areal, 0.0f);
		InitConstAnim(coneSys.areaw, 0.0f);
		InitConstAnim(coneSys.spread, 0.1f);
		InitConstAnim(coneSys.lat, 0.0f);
		InitConstAnim(coneSys.grav2, 0.0f);
		coneSys.emitDir = Vector3(0.0f, 1.0f, 0.0f);
		coneSys.minPolar = 0.0f;
		coneSys.maxPolar = 0.1f;
		coneSys.emitter = ParticleSystem::CreateEmitterByType(&coneSys, ParticleEmitter::TYPE_CONE_JET_EMITTER);
		TEST_CHECK(coneSys.emitter != nullptr);
		for (int i = 0; i < 8; ++i)
		{
			Particle p = coneSys.emitter->newParticle(0, 0);
			TEST_CHECK(p.speed.y > 11.5f);
		}
	}

	CParaXModel* CreateSampleFireworkAndRibbonModel()
	{
		ParaXHeaderDef hdr{};
		std::memcpy(hdr.id, "PARAX", 4);
		hdr.version[0] = 1;
		hdr.type = PARAX_MODEL_ANIMATED;
		hdr.IsAnimated = (1 << 2);
		hdr.minExtent = Vector3(-4.0f, 0.0f, -4.0f);
		hdr.maxExtent = Vector3(4.0f, 12.0f, 4.0f);

		CParaXModel* m = new CParaXModel(hdr);

		// 2 builtin textures: glow + ribbon
		m->m_objNum.nTextures = 2;
		m->textures = new asset_ptr<TextureEntity>[2];
		m->textures[0] = LoadOrCreateBuiltinParticleTexture("builtin://glow");
		m->textures[1] = LoadOrCreateBuiltinParticleTexture("builtin://ribbon");

		// 1 animation [0 .. 2000ms]
		m->m_objNum.nAnimations = 1;
		m->anims = new ModelAnimation[1];
		std::memset(&m->anims[0], 0, sizeof(ModelAnimation));
		m->anims[0].animID = 0;
		m->anims[0].timeStart = 0;
		m->anims[0].timeEnd = 2000;
		m->anims[0].moveSpeed = 0.0f;
		m->anims[0].loopType = 0;

		// 1 root bone
		m->m_objNum.nBones = 1;
		m->bones = new Bone[1];
		m->bones[0].parent = -1;
		m->bones[0].pivot = Vector3(0.0f, 0.0f, 0.0f);
		m->bones[0].mat = Matrix4::IDENTITY;
		m->bones[0].mrot = Matrix4::IDENTITY;
		m->bones[0].bUsePivot = true;
		InitConstAnim(m->bones[0].trans, Vector3(0.0f, 0.0f, 0.0f));
		InitConstAnim(m->bones[0].rot, Quaternion::IDENTITY);
		InitConstAnim(m->bones[0].scale, Vector3(1.0f, 1.0f, 1.0f));

		// 3 particle emitters: [0] Rocket -> [1] Trail + [2] Peony Shell on death
		m->m_objNum.nParticleEmitters = 3;
		m->particleSystems = new ParticleSystem[3];

		auto initDefaults = [&](ParticleSystem& ps, int emitterType, float spd, float life, int texIdx) {
			ps.model = m;
			ps.parent = &m->bones[0];
			ps.m_texture_index = texIdx;
			ps.blend = 1; // ADDITIVE
			ps.type = 0;
			ps.billboard = true;
			ps.m_bExtendedV2 = true;
			ps.useWorldGravity = true;
			ps.SetTextureRowsCols(1, 1);
			InitConstAnim(ps.speed, spd);
			InitConstAnim(ps.variation, 0.0f);
			InitConstAnim(ps.spread, 0.05f);
			InitConstAnim(ps.lat, 0.0f);
			InitConstAnim(ps.gravity, 2.0f);
			InitConstAnim(ps.lifespan, life);
			InitConstAnim(ps.rate, 0.0f);
			InitConstAnim(ps.areal, 0.0f);
			InitConstAnim(ps.areaw, 0.0f);
			InitConstAnim(ps.grav2, 0.0f);
			ps.sizes[0] = 0.25f;
			ps.sizes[1] = 0.35f;
			ps.sizes[2] = 0.05f;
			ps.mid = 0.5f;
			ps.colors[0] = Vector4(1.0f, 0.95f, 0.7f, 1.0f);
			ps.colors[1] = Vector4(1.0f, 0.6f, 0.2f, 0.9f);
			ps.colors[2] = Vector4(1.0f, 0.2f, 0.1f, 0.0f);
			ps.emitter = ParticleSystem::CreateEmitterByType(&ps, emitterType);
		};

		// [0] Ascending rocket
		initDefaults(m->particleSystems[0], ParticleEmitter::TYPE_CONE_JET_EMITTER, 10.0f, 0.20f, 0);
		m->particleSystems[0].emitDir = Vector3(0.0f, 1.0f, 0.0f);
		m->particleSystems[0].minPolar = 0.0f;
		m->particleSystems[0].maxPolar = 0.02f;
		m->particleSystems[0].bursts.push_back({ 0, 0, 1, 0 });
		m->particleSystems[0].trailEmitterIndex = 1;
		m->particleSystems[0].trailInterval = 0.04f;
		m->particleSystems[0].deathEmitterIndex = 2;
		m->particleSystems[0].deathCount = 12;
		m->particleSystems[0].deathCountVar = 0;

		// [1] Rising gold spark trail
		initDefaults(m->particleSystems[1], ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER, 0.6f, 0.30f, 0);
		m->particleSystems[1].inheritVelocity = 0.2f;

		// [2] Peony spherical burst with palette + strobe twinkle
		initDefaults(m->particleSystems[2], ParticleEmitter::TYPE_SPHERE_SHELL_EMITTER, 8.0f, 0.60f, 0);
		m->particleSystems[2].shellThickness = 0.1f;
		m->particleSystems[2].twinkleFreq = 14.0f;
		m->particleSystems[2].twinkleAmount = 0.65f;
		m->particleSystems[2].palette = {
			Vector4(1.0f, 0.25f, 0.35f, 1.0f),
			Vector4(0.25f, 0.85f, 1.0f, 1.0f),
			Vector4(1.0f, 0.9f, 0.25f, 1.0f),
		};

		// 1 Ribbon emitter
		m->m_objNum.nRibbonEmitters = 1;
		m->ribbons = new RibbonEmitter[1];
		RibbonEmitter& rb = m->ribbons[0];
		rb.model = m;
		rb.parent = &m->bones[0];
		rb.pos = Vector3(0.0f, 0.5f, 0.0f);
		rb.m_texture_index = 1;
		rb.numsegs = 20;
		rb.seglen = 0.12f;
		rb.length = rb.numsegs * rb.seglen;
		rb.m_bExtendedV2 = true;
		rb.m_bUseAbsCord = true;
		rb.orientCamera = true;
		rb.hasEndColor = true;
		rb.colorEnd = Vector4(0.2f, 0.5f, 1.0f, 0.0f);
		rb.maxLife = 0.45f;
		rb.blend = 1;
		rb.widthEndScale = 0.15f;
		rb.maxStepDist = 6.0f;
		InitConstAnim(rb.color, Vector3(1.0f, 0.4f, 0.8f));
		InitConstAnimShort(rb.opacity, 0.95f);
		InitConstAnim(rb.above, 0.22f);
		InitConstAnim(rb.below, 0.22f);

		return m;
	}

	void TestFireworkCascadeAndRibbonSimulation()
	{
		ref_ptr<CParaXModel> m(CreateSampleFireworkAndRibbonModel());
		SceneState sceneState;
		char ownerA = 0;
		char ownerB = 0;

		sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&ownerA));
		ParticleSystem& rocketSys = m->particleSystems[0];
		ParticleSystem& trailSys = m->particleSystems[1];
		ParticleSystem& shellSys = m->particleSystems[2];

		bool sawRocket = false;
		bool sawTrail = false;
		bool sawShellBurst = false;

		for (int step = 0; step < 18; ++step)
		{
			int timeMs = step * 20;
			for (int e = 0; e < 3; ++e)
			{
				ParticleList* inst = m->particleSystems[e].GetCurrentInstance(&sceneState, true);
				inst->m_bUpdated = false;
				m->particleSystems[e].setup(&sceneState, 0, timeMs);
				m->particleSystems[e].update(&sceneState, 0.02f);
			}

			ParticleList* rInst = rocketSys.GetCurrentInstance(&sceneState, false);
			ParticleList* tInst = trailSys.GetCurrentInstance(&sceneState, false);
			ParticleList* sInst = shellSys.GetCurrentInstance(&sceneState, false);

			if (step == 0)
			{
				// Exactly 1 rocket spawned from burst at t=0ms
				TEST_CHECK(rInst && rInst->particles.size() == 1);
				sawRocket = (rInst && rInst->particles.size() == 1);
			}
			if (tInst && !tInst->particles.empty())
				sawTrail = true;
			if (sInst && sInst->particles.size() >= 12)
			{
				sawShellBurst = true;
				// Shell should detonate well above launch origin
				TEST_CHECK(sInst->particles[0].pos.y > 1.0f);
			}
		}

		TEST_CHECK(sawRocket);
		TEST_CHECK(sawTrail);
		TEST_CHECK(sawShellBurst);

		// Test RibbonEmitter per-instance isolation, width, teleport break, and stationary decay
		RibbonEmitter& rb = m->ribbons[0];
		for (int step = 0; step <= 10; ++step)
		{
			// Owner A moves along +X
			sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&ownerA));
			m->bones[0].mat = Matrix4::IDENTITY;
			m->bones[0].mat.setTrans(Vector3(step * 0.2f, 0.0f, 0.0f));
			RibbonInstance* instA = rb.GetCurrentInstance(&sceneState, true);
			instA->m_bUpdated = false;
			rb.setup(&sceneState, 0, step * 20);
			rb.update(&sceneState, 0.02f);

			// Owner B stays stationary at (0, 5, 0)
			sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&ownerB));
			m->bones[0].mat = Matrix4::IDENTITY;
			m->bones[0].mat.setTrans(Vector3(0.0f, 5.0f, 0.0f));
			RibbonInstance* instB = rb.GetCurrentInstance(&sceneState, true);
			instB->m_bUpdated = false;
			rb.setup(&sceneState, 0, step * 20);
			rb.update(&sceneState, 0.02f);
		}

		sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&ownerA));
		RibbonInstance* instA = rb.GetCurrentInstance(&sceneState, false);
		sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&ownerB));
		RibbonInstance* instB = rb.GetCurrentInstance(&sceneState, false);

		TEST_CHECK(instA != nullptr && instB != nullptr && instA != instB);
		TEST_CHECK(instA->segs.size() >= 8);
		TEST_CHECK(instB->segs.size() == 1);
		TEST_CHECK(rb.tabove > 0.2f && rb.tbelow > 0.2f);
		// Verify distinct top/bottom edge vertices (fixes legacy zero-width ribbon bug)
		Vector3 topEdge = instA->segs[0].pos + instA->segs[0].up * rb.tabove;
		Vector3 botEdge = instA->segs[0].pos - instA->segs[0].up * rb.tbelow;
		TEST_CHECK((topEdge - botEdge).length() > 0.4f);

		// Teleport Owner A beyond maxStepDist (6.0f) -> ribbon breaks cleanly
		sceneState.SetCurrentSceneObject(reinterpret_cast<CBaseObject*>(&ownerA));
		m->bones[0].mat = Matrix4::IDENTITY;
		m->bones[0].mat.setTrans(Vector3(25.0f, 0.0f, 0.0f));
		instA->m_bUpdated = false;
		rb.update(&sceneState, 0.02f);
		TEST_CHECK(instA->segs.size() == 1);

		// Build 3 segments after teleport, then verify stationary decay clears the trail
		for (int k = 1; k <= 4; ++k)
		{
			m->bones[0].mat.setTrans(Vector3(25.0f + k * 0.2f, 0.0f, 0.0f));
			instA->m_bUpdated = false;
			rb.update(&sceneState, 0.02f);
		}
		TEST_CHECK(instA->segs.size() >= 3);
		bool alive = true;
		for (int k = 0; k < 5; ++k)
			alive = rb.AnimateExistingSegments(0.15f, Vector3(0, 0, 0), instA);
		TEST_CHECK(!alive);
		TEST_CHECK(instA->segs.size() < 2);
	}

	void VerifyRoundTrippedModel(CParaXModel* loaded, const char* label)
	{
		TEST_CHECK(loaded != nullptr);
		if (!loaded)
			return;

		TEST_CHECK(loaded->GetObjectNum().nParticleEmitters == 3);
		TEST_CHECK(loaded->GetObjectNum().nRibbonEmitters == 1);
		TEST_CHECK(loaded->GetObjectNum().nCameras == 0);
		TEST_CHECK(loaded->GetObjectNum().nBones >= 1);
		TEST_CHECK(loaded->GetObjectNum().nAnimations >= 1);

		if (loaded->GetObjectNum().nParticleEmitters == 3)
		{
			const ParticleSystem& ps0 = loaded->particleSystems[0];
			const ParticleSystem& ps2 = loaded->particleSystems[2];
			TEST_CHECK(ps0.emitter && ps0.emitter->GetEmitterType() == ParticleEmitter::TYPE_CONE_JET_EMITTER);
			TEST_CHECK(ps0.useWorldGravity);
			TEST_CHECK(ps0.bursts.size() == 1 && ps0.bursts[0].count == 1);
			TEST_CHECK(ps0.trailEmitterIndex == 1);
			TEST_CHECK(ps0.deathEmitterIndex == 2);
			TEST_CHECK(ps0.deathCount == 12);

			TEST_CHECK(ps2.emitter && ps2.emitter->GetEmitterType() == ParticleEmitter::TYPE_SPHERE_SHELL_EMITTER);
			TEST_CHECK(ps2.palette.size() == 3);
			TEST_CHECK(std::fabs(ps2.twinkleFreq - 14.0f) < 0.05f);
			TEST_CHECK(std::fabs(ps2.twinkleAmount - 0.65f) < 0.02f);
		}

		if (loaded->GetObjectNum().nRibbonEmitters == 1)
		{
			const RibbonEmitter& rb = loaded->ribbons[0];
			TEST_CHECK(rb.orientCamera);
			TEST_CHECK(rb.hasEndColor);
			TEST_CHECK(rb.blend == 1);
			TEST_CHECK(std::fabs(rb.maxLife - 0.45f) < 0.02f);
			TEST_CHECK(std::fabs(rb.widthEndScale - 0.15f) < 0.02f);
			TEST_CHECK(std::fabs(rb.maxStepDist - 6.0f) < 0.05f);
		}
		std::printf("[OK] Round-trip verified (%s)\n", label);
	}

	void TestParaxAndGlbRoundTrip()
	{
		const fs::path tempDir = fs::temp_directory_path() / fs::unique_path("npl-particle-parax-%%%%-%%%%");
		fs::create_directories(tempDir);

		CParaXModel* src = CreateSampleFireworkAndRibbonModel();

		// 1. Binary .x round-trip
		const std::string xPath = (tempDir / "sample_firework.x").string();
		TEST_CHECK(XFileCharModelExporter::Export(xPath, src));
		std::vector<char> xBytes;
		TEST_CHECK(ReadFileBytes(xPath, xBytes));
		if (!xBytes.empty())
		{
			XFileCharModelParser parser(xBytes.data(), static_cast<int32>(xBytes.size() - 1));
			CParaXModel* loadedX = parser.ParseParaXModel();
			VerifyRoundTrippedModel(loadedX, "binary .x v2");
			delete loadedX;
		}

		// 2. Binary .glb (extras.parax) round-trip
		const std::string glbPath = (tempDir / "sample_firework.glb").string();
		GltfModel::ExportParaXModel(src, glbPath.c_str());
		std::vector<char> glbBytes;
		TEST_CHECK(ReadFileBytes(glbPath, glbBytes));
		if (glbBytes.size() >= 20)
		{
			uint32 magic = 0, version = 0;
			std::memcpy(&magic, glbBytes.data(), 4);
			std::memcpy(&version, glbBytes.data() + 4, 4);
			TEST_CHECK(magic == 0x46546C67u); // "glTF"
			TEST_CHECK(version == 2u);

			CParaXModel* loadedGlb = ParaXGltfExtras::ParseStandaloneModelFromBuffer(
				glbBytes.data(), static_cast<int>(glbBytes.size() - 1));
			VerifyRoundTrippedModel(loadedGlb, "binary .glb extras.parax");
			delete loadedGlb;
		}

		delete src;
		boost::system::error_code ec;
		fs::remove_all(tempDir, ec);
	}

	bool ValidateExternalModelFile(const std::string& path)
	{
		std::vector<char> bytes;
		if (!ReadFileBytes(path, bytes))
		{
			std::fprintf(stderr, "[FAIL] Cannot read external model file: %s\n", path.c_str());
			return false;
		}
		CParaXModel* m = nullptr;
		if (path.size() >= 4 && (path.substr(path.size() - 4) == ".glb" || path.substr(path.size() - 5) == ".gltf"))
		{
			m = ParaXGltfExtras::ParseStandaloneModelFromBuffer(bytes.data(), static_cast<int>(bytes.size() - 1));
		}
		else
		{
			XFileCharModelParser parser(bytes.data(), static_cast<int32>(bytes.size() - 1));
			m = parser.ParseParaXModel();
		}
		if (!m)
		{
			std::fprintf(stderr, "[FAIL] Parser returned nullptr for: %s\n", path.c_str());
			return false;
		}
		bool hasEmitters = (m->GetObjectNum().nParticleEmitters > 0 || m->GetObjectNum().nRibbonEmitters > 0);
		std::printf("[OK] Loaded %s (particles=%u, ribbons=%u, bones=%u, anims=%u)\n",
			path.c_str(),
			m->GetObjectNum().nParticleEmitters,
			m->GetObjectNum().nRibbonEmitters,
			m->GetObjectNum().nBones,
			m->GetObjectNum().nAnimations);
		delete m;
		return hasEmitters;
	}
}

int main(int argc, char** argv)
{
	CParaEngineAppBase application;
	CParaWorldAsset assetManager;

	if (argc > 2 && std::strcmp(argv[1], "--validate-models") == 0)
	{
		bool allOk = true;
		for (int i = 2; i < argc; ++i)
		{
			if (!ValidateExternalModelFile(argv[i]))
				allOk = false;
		}
		return allOk ? 0 : 1;
	}

	TestProceduralBuiltinTextures();
	TestZeroRateSilenceAndEmitterShapes();
	TestFireworkCascadeAndRibbonSimulation();
	TestParaxAndGlbRoundTrip();

	std::printf("ParticleAndParaxTest summary: %d passed, %d failed\n", g_passed, g_failed);
	return (g_failed == 0) ? 0 : 1;
}

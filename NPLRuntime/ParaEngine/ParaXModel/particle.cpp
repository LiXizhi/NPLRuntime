//-----------------------------------------------------------------------------
// Class:	particle & ribbon system for ParaX Model
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.11
// Revised: 2007.3, 2026.11 (RibbonEmitter overhaul, fireworks shells, bursts & sub-emitters)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel.h"
#include "ParaWorldAsset.h"
#include "EffectManager.h"
#include "SceneObject.h"
#include "ParaXBone.h"
#include "particle.h"
#include "zlib.h"
#include <algorithm>
#include <cmath>
#include "memdebug.h"

using namespace ParaEngine;

#define DEFAULT_TO_GLOBAL_OBJECT true
#define MAX_PARTICLES 10000

Vector4 fromARGB(uint32 color)
{
	const float a = ((color & 0xFF000000) >> 24) / 255.0f;
	const float r = ((color & 0x00FF0000) >> 16) / 255.0f;
	const float g = ((color & 0x0000FF00) >> 8) / 255.0f;
	const float b = ((color & 0x000000FF)) / 255.0f;
	return Vector4(r, g, b, a);
}

template<class T>
T lifeRamp(float life, float mid, const T& a, const T& b, const T& c)
{
	if (mid <= 0.0001f)
		return interpolate<T>(life, b, c);
	if (mid >= 0.9999f)
		return interpolate<T>(life, a, b);
	if (life <= mid)
		return interpolate<T>(life / mid, a, b);
	else
		return interpolate<T>((life - mid) / (1.0f - mid), b, c);
}

namespace
{
	void WriteBE32(std::vector<char>& out, uint32_t v)
	{
		out.push_back(static_cast<char>((v >> 24) & 0xFF));
		out.push_back(static_cast<char>((v >> 16) & 0xFF));
		out.push_back(static_cast<char>((v >> 8) & 0xFF));
		out.push_back(static_cast<char>(v & 0xFF));
	}

	void WritePNGChunk(std::vector<char>& out, const char type[4], const uint8_t* data, uint32_t len)
	{
		WriteBE32(out, len);
		out.insert(out.end(), type, type + 4);
		if (len > 0 && data)
			out.insert(out.end(), reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
		uLong crc = crc32(0L, Z_NULL, 0);
		crc = crc32(crc, reinterpret_cast<const Bytef*>(type), 4);
		if (len > 0 && data)
			crc = crc32(crc, reinterpret_cast<const Bytef*>(data), len);
		WriteBE32(out, static_cast<uint32_t>(crc));
	}

	bool EncodeRGBA8ToPNG(int width, int height, const std::vector<uint8_t>& rgba, std::vector<char>& outPng)
	{
		if (width <= 0 || height <= 0 || rgba.size() < static_cast<size_t>(width * height * 4))
			return false;

		std::vector<uint8_t> rawScanlines;
		rawScanlines.reserve(static_cast<size_t>(height) * (static_cast<size_t>(width) * 4 + 1));
		for (int y = 0; y < height; ++y)
		{
			rawScanlines.push_back(0); // filter type None
			const uint8_t* row = &rgba[static_cast<size_t>(y * width * 4)];
			rawScanlines.insert(rawScanlines.end(), row, row + width * 4);
		}

		uLongf compBound = compressBound(static_cast<uLong>(rawScanlines.size()));
		std::vector<uint8_t> compressed(compBound);
		if (compress2(compressed.data(), &compBound, rawScanlines.data(), static_cast<uLong>(rawScanlines.size()), Z_BEST_SPEED) != Z_OK)
			return false;

		static const uint8_t kSig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
		outPng.clear();
		outPng.insert(outPng.end(), reinterpret_cast<const char*>(kSig), reinterpret_cast<const char*>(kSig) + 8);

		uint8_t ihdr[13] = {
			static_cast<uint8_t>((width >> 24) & 0xFF), static_cast<uint8_t>((width >> 16) & 0xFF),
			static_cast<uint8_t>((width >> 8) & 0xFF), static_cast<uint8_t>(width & 0xFF),
			static_cast<uint8_t>((height >> 24) & 0xFF), static_cast<uint8_t>((height >> 16) & 0xFF),
			static_cast<uint8_t>((height >> 8) & 0xFF), static_cast<uint8_t>(height & 0xFF),
			8, // bit depth
			6, // RGBA
			0, 0, 0
		};
		WritePNGChunk(outPng, "IHDR", ihdr, 13);
		WritePNGChunk(outPng, "IDAT", compressed.data(), static_cast<uint32_t>(compBound));
		WritePNGChunk(outPng, "IEND", nullptr, 0);
		return true;
	}

	void BuildBasisAroundDir(const Vector3& dir, Vector3& outAxis, Vector3& outTangent1, Vector3& outTangent2)
	{
		outAxis = dir;
		if (outAxis.squaredLength() < 1e-6f)
			outAxis = Vector3(0.0f, 1.0f, 0.0f);
		else
			outAxis.normalise();

		Vector3 helper = (std::fabs(outAxis.y) < 0.9f) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
		outTangent1 = helper.crossProduct(outAxis);
		outTangent1.normalise();
		outTangent2 = outAxis.crossProduct(outTangent1);
		outTangent2.normalise();
	}

	void ApplyParticleBlendState(RenderDevicePtr& pd3dDevice, CEffectFile* pEffect, int blendMode)
	{
		switch (blendMode) {
		case 0: // OPAQUE
			pEffect->EnableAlphaBlending(false);
			pEffect->EnableAlphaTesting(false);
			break;
		case 1: // ADDITIVE BLEND
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			break;
		case 2: // ALPHA_BLEND
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case 3: // TRANSPARENT
			pEffect->EnableAlphaTesting(true);
			pEffect->EnableAlphaBlending(false);
			break;
		case 4: // ADDITIVE ALPHA
		default:
			pEffect->EnableAlphaTesting(false);
			pEffect->EnableAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
			break;
		}
	}

	void RestoreParticleBlendState(RenderDevicePtr& pd3dDevice, int blendMode)
	{
		switch (blendMode)
		{
		case 0:
		case 2:
		case 3:
			break;
		case 4:
		default:
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		}
	}
}

namespace ParaEngine
{
	bool IsBuiltinParticleTextureName(const std::string& name)
	{
		return name.compare(0, 10, "builtin://") == 0;
	}

	bool BuildProceduralParticlePNG(const std::string& rawKind, std::vector<char>& outPngBytes)
	{
		std::string kind = rawKind;
		if (IsBuiltinParticleTextureName(kind))
			kind = kind.substr(10);

		const int kSize = 64;
		std::vector<uint8_t> rgba(kSize * kSize * 4, 255);
		for (int y = 0; y < kSize; ++y)
		{
			float v = (y + 0.5f) / (float)kSize;
			float ny = v * 2.0f - 1.0f;
			for (int x = 0; x < kSize; ++x)
			{
				float u = (x + 0.5f) / (float)kSize;
				float nx = u * 2.0f - 1.0f;
				float r = std::sqrt(nx * nx + ny * ny);
				float alpha = 0.0f;

				if (kind == "ribbon")
				{
					// Smooth luminous core across V [0..1] with soft edges so ribbon strips never show hard seams
					float edge = std::sin(std::min(1.0f, std::max(0.0f, v)) * MATH_PI);
					float core = std::exp(-10.0f * ny * ny);
					alpha = std::min(1.0f, 0.55f * std::pow(edge, 1.4f) + 0.65f * core);
				}
				else if (kind == "spark")
				{
					if (r < 1.0f)
					{
						float core = std::exp(-12.0f * r * r);
						float halo = std::pow(1.0f - r, 2.5f);
						alpha = std::min(1.0f, core + 0.55f * halo);
					}
				}
				else if (kind == "star" || kind == "flare")
				{
					if (r < 1.0f)
					{
						float core = std::exp(-14.0f * r * r);
						float rayHV = std::exp(-28.0f * std::min(nx * nx, ny * ny)) * std::pow(1.0f - r, 1.6f);
						float diag1 = (nx + ny) * 0.70710678f;
						float diag2 = (nx - ny) * 0.70710678f;
						float rayDiag = (kind == "flare")
							? (std::exp(-36.0f * std::min(diag1 * diag1, diag2 * diag2)) * std::pow(1.0f - r, 2.0f) * 0.65f)
							: 0.0f;
						alpha = std::min(1.0f, core + 0.85f * rayHV + rayDiag);
					}
				}
				else if (kind == "ring")
				{
					if (r < 1.0f)
					{
						float ringDiff = (r - 0.68f) / 0.14f;
						alpha = std::exp(-ringDiff * ringDiff * 2.0f) * (1.0f - std::pow(r, 6.0f));
					}
				}
				else if (kind == "smoke")
				{
					if (r < 1.0f)
					{
						float soft = 1.0f - r * r;
						alpha = soft * soft * 0.72f;
					}
				}
				else // default "glow"
				{
					if (r < 1.0f)
					{
						float falloff = 1.0f - r;
						alpha = std::min(1.0f, std::exp(-4.2f * r * r) * (falloff * (2.0f - falloff)));
					}
				}

				size_t idx = static_cast<size_t>((y * kSize + x) * 4);
				rgba[idx + 0] = 255;
				rgba[idx + 1] = 255;
				rgba[idx + 2] = 255;
				rgba[idx + 3] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, alpha * 255.0f + 0.5f)));
			}
		}
		return EncodeRGBA8ToPNG(kSize, kSize, rgba, outPngBytes);
	}

	asset_ptr<TextureEntity> LoadOrCreateBuiltinParticleTexture(const std::string& name)
	{
		std::string key = IsBuiltinParticleTextureName(name) ? name : ("builtin://" + name);
		auto* pAssetMgr = CGlobals::GetAssetManager();
		if (!pAssetMgr)
			return asset_ptr<TextureEntity>();

		TextureEntity* existing = pAssetMgr->GetTexture(key);
		if (existing && (existing->GetRawData() || existing->IsLoaded()))
			return asset_ptr<TextureEntity>(existing);

		asset_ptr<TextureEntity> tex(pAssetMgr->LoadTexture("", key.c_str(), TextureEntity::StaticTexture));
		if (tex && !tex->GetRawData())
		{
			std::vector<char> pngBytes;
			if (BuildProceduralParticlePNG(key, pngBytes) && !pngBytes.empty())
			{
				char* buf = new char[pngBytes.size()];
				memcpy(buf, pngBytes.data(), pngBytes.size());
				// Keep m_bEmbeddedTexture = false so shared builtin://* textures in TextureManager
				// are not invalidated when an individual CParaXModel unloads, and stay compact in .x exports.
				tex->SetRawData(buf, static_cast<int32>(pngBytes.size()));
			}
		}
		return tex;
	}
}

ParticleSystem::ParticleSystem()
	: emitter(nullptr), m_texture_index(0), rotate2SpeedDirection(false)
{
	colors[0] = Vector4(1, 1, 1, 1);
	colors[1] = Vector4(1, 1, 1, 0.8f);
	colors[2] = Vector4(1, 1, 1, 0);
}

ParticleSystem::~ParticleSystem()
{
	SAFE_DELETE(emitter);
	ClearAllInstances();
}

ParticleEmitter* ParticleSystem::CreateEmitterByType(ParticleSystem* sys, int emitterType)
{
	switch (emitterType)
	{
	case ParticleEmitter::TYPE_PLANE_PARTICLE_EMITTER:
		return new PlaneParticleEmitter(sys);
	case ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER:
		return new SphereParticleEmitter(sys);
	case ParticleEmitter::TYPE_SPHERE_SHELL_EMITTER:
		return new SphereShellParticleEmitter(sys);
	case ParticleEmitter::TYPE_PATTERN_PARTICLE_EMITTER:
		return new PatternParticleEmitter(sys);
	case ParticleEmitter::TYPE_CONE_JET_EMITTER:
		return new ConeJetParticleEmitter(sys);
	default:
		return nullptr;
	}
}

void ParticleSystem::SetTextureRowsCols(int nRows, int nCols)
{
	rows = (nRows > 0) ? nRows : 1;
	cols = (nCols > 0) ? nCols : 1;

	int nCount = rows * cols;
	tiles.clear();
	tiles.reserve(nCount);

	for (int k = 0; k < nCount; ++k)
	{
		tiles.push_back(TexCoordSet());
		TexCoordSet& tc = tiles[k];

		Vector2 otc[4];
		Vector2 a, b;
		int x = k % cols;
		int y = k / cols;
		a.x = x * (1.0f / cols);
		a.y = y * (1.0f / rows);
		b.x = (x + 1) * (1.0f / cols);
		b.y = (y + 1) * (1.0f / rows);

		otc[0] = a;
		otc[2] = b;
		otc[1].x = b.x;
		otc[1].y = a.y;
		otc[3].x = a.x;
		otc[3].y = b.y;

		for (int i = 0; i < 4; i++) {
			tc.tc[(i + 4 - order) & 3] = otc[i];
		}
	}
}

void ParticleSystem::update(SceneState* pSceneState, float dt)
{
	ParticleList* instancePS = GetCurrentInstance(pSceneState);
	if (instancePS == NULL || instancePS->m_bUpdated)
		return;

	pSceneState->AddParticleSystem(this);
	instancePS->m_bUpdated = true;

	ParticleListType& particles = instancePS->particles;

	Vector3 vRenderOriginOffset(0, 0, 0);
	Matrix4 mWorld = Matrix4::IDENTITY;
	Vector3 vRenderOrigin(0, 0, 0);

	if (instancePS->m_bUseAbsCord)
	{
		mWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
		if (pSceneState->GetScene())
			vRenderOrigin = pSceneState->GetScene()->GetRenderOrigin();

		vRenderOriginOffset = instancePS->m_vLastRenderOrigin - vRenderOrigin;
		instancePS->m_vLastRenderOrigin = vRenderOrigin;
	}

	AnimateExistingParticles(dt, vRenderOriginOffset, instancePS);

	if (emitter)
	{
		int manim = instancePS->m_anim;
		int mtime = instancePS->m_time;

		// 1. Evaluate deterministic bursts across animation timestamps
		int burstToSpawn = 0;
		if (!bursts.empty())
		{
			const int prevAnim = instancePS->m_lastAnim;
			const int prevTime = instancePS->m_lastTime;
			for (const auto& b : bursts)
			{
				if (b.count <= 0)
					continue;
				if (b.animIndex >= 0 && b.animIndex != manim)
					continue;

				bool triggered = false;
				if (prevAnim != manim || prevTime < 0)
				{
					// Initial frame of this animation: fire bursts scheduled right at the start window
					if (mtime >= b.timeMs && mtime <= b.timeMs + 100)
						triggered = true;
				}
				else if (mtime >= prevTime)
				{
					if (b.timeMs > prevTime && b.timeMs <= mtime)
						triggered = true;
				}
				else
				{
					// Animation looped around
					if (b.timeMs > prevTime || b.timeMs <= mtime)
						triggered = true;
				}

				if (triggered)
				{
					int delta = (b.countVar > 0) ? randint(-b.countVar, b.countVar) : 0;
					burstToSpawn += std::max(0, b.count + delta);
				}
			}
		}
		instancePS->m_lastAnim = manim;
		instancePS->m_lastTime = mtime;

		// 2. Continuous emission rate (skipping continuous spawn on extreme frame stalls > 0.1s)
		int rateToSpawn = 0;
		float frate = rate.used || !rate.data.empty() ? rate.getValue(manim, mtime) : 0.0f;
		if (frate > 0.0f && dt <= 0.1f)
		{
			float ftospawn = (dt * frate) + instancePS->m_rem;
			if (ftospawn < 1.0f && !particles.empty())
			{
				instancePS->m_rem = std::max(0.0f, ftospawn);
			}
			else
			{
				if (ftospawn < 1.0f)
					ftospawn = 1.0f; // immediate first particle only when frate > 0
				rateToSpawn = (int)ftospawn;
				instancePS->m_rem = ftospawn - (float)rateToSpawn;
			}
		}
		else if (frate <= 0.0f)
		{
			instancePS->m_rem = 0.0f;
		}

		int totalToSpawn = burstToSpawn + rateToSpawn;
		if (totalToSpawn > 0)
		{
			const size_t targetCap = std::min<size_t>(MAX_PARTICLES, particles.size() + static_cast<size_t>(totalToSpawn));
			if (particles.capacity() < targetCap)
				particles.reserve(targetCap);
			for (int i = 0; i < totalToSpawn; i++)
			{
				if (particles.size() >= MAX_PARTICLES)
					break;
				particles.push_back(emitter->newParticle(manim, mtime));
				if (instancePS->m_bUseAbsCord)
				{
					Particle& p = particles.back();
					p.pos = p.pos * mWorld;
					if (billboard)
						instancePS->m_positionSum += p.pos;
					p.origin = p.origin * mWorld;
					p.speed = p.speed.TransformNormal(mWorld);
				}
			}
		}
	}
}

void ParticleSystem::SpawnSubParticles(int targetEmitterIdx, void* ownerKey, SceneState* pSceneState,
	const Vector3& basePos, const Vector3& parentVel, int count,
	const Vector3& lastRenderOrigin, int anim, int time)
{
	if (!model || count <= 0 || targetEmitterIdx < 0 || targetEmitterIdx >= (int)model->GetObjectNum().nParticleEmitters)
		return;

	ParticleSystem* targetSys = &model->particleSystems[targetEmitterIdx];
	if (targetSys == this || !targetSys->emitter)
		return;

	ParticleList* targetInst = nullptr;
	auto it = targetSys->m_instances.find(ownerKey);
	if (it == targetSys->m_instances.end())
	{
		targetInst = new ParticleList();
		targetInst->m_pSceneState = pSceneState;
		targetInst->m_ownerKey = ownerKey;
		targetInst->m_bUseAbsCord = !targetSys->localSpace;
		targetInst->m_vLastRenderOrigin = lastRenderOrigin;
		targetInst->m_anim = anim;
		targetInst->m_time = time;
		targetSys->m_instances[ownerKey] = targetInst;
	}
	else
	{
		targetInst = it->second;
	}

	if (pSceneState)
	{
		pSceneState->AddParticleSystem(targetSys);
		targetInst->m_bRender = true;
	}

	ParticleListType& outParticles = targetInst->particles;
	const size_t targetCap = std::min<size_t>(MAX_PARTICLES, outParticles.size() + static_cast<size_t>(count));
	if (outParticles.capacity() < targetCap)
		outParticles.reserve(targetCap);

	for (int i = 0; i < count; ++i)
	{
		if (outParticles.size() >= MAX_PARTICLES)
			break;
		Particle child = targetSys->emitter->newParticle(anim, time);
		// Re-anchor child particle at the parent particle's live position + velocity inheritance
		child.pos = basePos;
		child.origin = basePos;
		if (targetSys->inheritVelocity != 0.0f)
			child.speed += parentVel * targetSys->inheritVelocity;
		if (targetSys->billboard && targetInst->m_bUseAbsCord)
			targetInst->m_positionSum += child.pos;
		outParticles.push_back(child);
	}
}

bool ParticleSystem::AnimateExistingParticles(float dt, const Vector3& vOffset, ParticleList* instancePS)
{
	if (instancePS == NULL)
		return false;
	ParticleListType& particles = instancePS->particles;

	int manim = instancePS->m_anim;
	int mtime = instancePS->m_time;
	float grav = (gravity.used || !gravity.data.empty()) ? gravity.getValue(manim, mtime) : 0.0f;
	float mspeed = 1.0f;

	struct PendingSpawn {
		Vector3 pos;
		Vector3 vel;
		int emitterIdx;
		int count;
	};
	std::vector<PendingSpawn> pendingSpawns;

	Vector3 positionSum(0, 0, 0);
	const bool sumPositions = billboard && instancePS->m_bUseAbsCord;
	size_t writeIdx = 0;
	const size_t nCount = particles.size();
	for (size_t readIdx = 0; readIdx < nCount; ++readIdx)
	{
		Particle& p = particles[readIdx];

		p.speed += p.down * (grav * dt);

		if (p.random_force > 0.f)
		{
			Vector3 vec3RandomForce(randfloat(-p.random_force, p.random_force), randfloat(-p.random_force, p.random_force), randfloat(-p.random_force, p.random_force));
			p.speed += vec3RandomForce * dt;
		}

		if (slowdown > 0)
		{
			mspeed = expf(-1.0f * slowdown * p.life);
		}

		p.pos += p.speed * (mspeed * dt);

		if (p.rotSpeed > 0)
		{
			float rotAtt = 1.0f - p.life / std::max(0.001f, p.maxlife);
			p.rotation += p.rotSpeed * dt * rotAtt;
		}

		if (p.trail_length > 0.f)
		{
			p.origin = p.pos - p.speed * (mspeed * p.trail_length);
		}

		if (instancePS->m_bUseAbsCord)
		{
			p.pos += vOffset;
			p.origin += vOffset;
		}

		p.life += dt;
		float rlife = (p.maxlife > 0.0001f) ? (p.life / p.maxlife) : 1.0f;
		float clampedLife = std::min(1.0f, std::max(0.0f, rlife));
		p.size = lifeRamp<float>(clampedLife, mid, sizes[0], sizes[1], sizes[2]);
		Vector4 baseCol = lifeRamp<Vector4>(clampedLife, mid, colors[0], colors[1], colors[2]);
		p.color = Vector4(
			baseCol.x * p.colorTint.x,
			baseCol.y * p.colorTint.y,
			baseCol.z * p.colorTint.z,
			baseCol.w * p.colorTint.w
		);

		if (twinkleFreq > 0.0f && twinkleAmount > 0.0f)
		{
			float wave = 0.5f + 0.5f * std::sin(p.life * twinkleFreq * 2.0f * MATH_PI + p.twinklePhase);
			p.color.w *= (1.0f - twinkleAmount * (1.0f - wave));
		}

		if (rlife < 1.0f)
		{
			if (trailEmitterIndex >= 0 && trailInterval > 0.002f && dt > 0.0f)
			{
				p.trailTimer += dt;
				int nTrail = 0;
				while (p.trailTimer >= trailInterval && nTrail < 4)
				{
					p.trailTimer -= trailInterval;
					++nTrail;
				}
				if (nTrail > 0)
				{
					pendingSpawns.push_back({p.pos, p.speed * mspeed, trailEmitterIndex, nTrail});
				}
			}

			if (sumPositions)
				positionSum += p.pos;
			if (writeIdx != readIdx)
				particles[writeIdx] = p;
			++writeIdx;
		}
		else if (deathEmitterIndex >= 0 && deathCount > 0)
		{
			int delta = (deathCountVar > 0) ? randint(-deathCountVar, deathCountVar) : 0;
			int nDeath = std::max(0, (int)deathCount + delta);
			if (nDeath > 0)
			{
				pendingSpawns.push_back({p.pos, p.speed * mspeed, deathEmitterIndex, nDeath});
			}
		}
	}
	if (writeIdx != nCount)
		particles.resize(writeIdx);
	instancePS->m_positionSum = positionSum;

	if (!pendingSpawns.empty())
	{
		SceneState* pSS = static_cast<SceneState*>(instancePS->m_pSceneState);
		for (const auto& sp : pendingSpawns)
		{
			SpawnSubParticles(sp.emitterIdx, instancePS->m_ownerKey, pSS, sp.pos, sp.vel,
				sp.count, instancePS->m_vLastRenderOrigin, manim, mtime);
		}
	}

	return (!particles.empty());
}

ParticleList* ParticleSystem::GetCurrentInstance(SceneState* pSceneState, bool bForceCreate)
{
	void* owner = pSceneState ? (void*)(pSceneState->GetCurrentSceneObject()) : nullptr;
	auto iter = m_instances.find(owner);
	ParticleList* instancePS = NULL;
	if (iter == m_instances.end())
	{
		if (bForceCreate)
		{
			instancePS = new ParticleList();
			instancePS->m_pSceneState = pSceneState;
			instancePS->m_ownerKey = owner;
			instancePS->m_bUseAbsCord = !localSpace;
			m_instances[owner] = instancePS;
		}
	}
	else
	{
		instancePS = iter->second;
	}
	return instancePS;
}

bool ParticleSystem::DeleteInstance(void* owner)
{
	auto iter = m_instances.find(owner);
	if (iter != m_instances.end())
	{
		ParticleList* instancePS = iter->second;
		SAFE_DELETE(instancePS);
		m_instances.erase(iter);
	}
	return true;
}

void ParticleSystem::setup(SceneState* pSceneState, int anim, int time)
{
	ParticleList* instancePS = GetCurrentInstance(pSceneState);
	if (instancePS == NULL)
		return;
	instancePS->m_anim = anim;
	instancePS->m_time = time;
}

void ParticleSystem::draw(SceneState* pSceneState)
{
	ParticleList* instancePS = GetCurrentInstance(pSceneState, false);
	if (instancePS == NULL)
		return;
	instancePS->m_bRender = true;
	if (!instancePS->m_bUseAbsCord)
		drawInstance(instancePS);
}

void ParticleSystem::drawAllAbsInstances()
{
	for (auto& kv : m_instances)
	{
		ParticleList* instancePS = kv.second;
		if (instancePS && instancePS->m_bUseAbsCord)
		{
			drawInstance(instancePS);
		}
	}
}

void ParticleSystem::drawInstance(ParticleList* instancePS)
{
	if (instancePS == NULL || instancePS->particles.empty())
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice)
		return;

	if (instancePS->m_bUseAbsCord)
	{
		CGlobals::GetWorldMatrixStack().push(*CGlobals::GetIdentityMatrix());
	}
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();

	if (pEffect != 0)
	{
		ApplyParticleBlendState(pd3dDevice, pEffect, blend);

		DeviceTexturePtr_type pTex = GetDeviceTexture();
		if (pTex)
		{
			if (pEffect->begin())
			{
				if (pEffect->BeginPass(0))
				{
					pEffect->setTexture(0, pTex);
					pEffect->CommitChanges();

					DrawInstanceSub(instancePS);
				}
				pEffect->EndPass(0);
			}
			pEffect->end();
		}
	}

	if (instancePS->m_bUseAbsCord)
		CGlobals::GetWorldMatrixStack().pop();

	RestoreParticleBlendState(pd3dDevice, blend);
}

void ParticleSystem::DrawInstanceSub(ParticleList* instancePS)
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	ParticleListType& particles = instancePS->particles;
	if (particles.empty())
		return;
	Vector3 bv0, bv1, bv2, bv3;
	Matrix4 mbb;
	mbb.identity();
	Vector3 look(0, 1, 0);
	if (billboard) {
		Matrix4 mtrans = CGlobals::GetViewMatrixStack().SafeGetTop();
		if (!instancePS->m_bUseAbsCord)
		{
			const Matrix4& mtransWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
			mtrans = mtransWorld * mtrans;
		}
		mtrans = mtrans.inverse();
		Vector3 camera = mtrans.getTrans();
		if (instancePS->m_bUseAbsCord)
		{
			Vector3 basePos = instancePS->m_positionSum;
			float fM = (float)(particles.size());
			basePos.x /= fM;
			basePos.y /= fM;
			basePos.z /= fM;
			Vector3 diff = camera - basePos;
			look = (diff.squaredLength() > 1e-6f) ? diff.normalisedCopy() : Vector3(0, 0, -1);
		}
		else
		{
			Vector3 diff = camera - pos;
			look = (diff.squaredLength() > 1e-6f) ? diff.normalisedCopy() : Vector3(0, 0, -1);
		}

		Vector3 up = ((Vector3(0, 1, 0) * mtrans) - camera).normalisedCopy();
		Vector3 right = (up % look).normalisedCopy();
		up = (look % right).normalisedCopy();

		mbb.m[1][0] = right.x;
		mbb.m[1][1] = right.y;
		mbb.m[1][2] = right.z;
		mbb.m[2][0] = up.x;
		mbb.m[2][1] = up.y;
		mbb.m[2][2] = up.z;
		mbb.m[0][0] = look.x;
		mbb.m[0][1] = look.y;
		mbb.m[0][2] = look.z;
	}

	if (tiles.empty())
		SetTextureRowsCols(rows, cols);

	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, pBufEntity->m_nUnitSize);

	SPRITEVERTEX* vb_vertices;
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;

	if (type == 0 || type == 2) {
		float f = 0.707106781f; // sqrt(2)/2
		if (billboard) {
			bv0 = Vector3(0, -f, +f) * mbb;
			bv1 = Vector3(0, +f, +f) * mbb;
			bv2 = Vector3(0, +f, -f) * mbb;
			bv3 = Vector3(0, -f, -f) * mbb;
		}
		else {
			bv0 = Vector3(-f, 0, +f);
			bv1 = Vector3(+f, 0, +f);
			bv2 = Vector3(+f, 0, -f);
			bv3 = Vector3(-f, 0, -f);
		}

		int nTotalVertices = (int)particles.size() * 6;
		ParticleListType::const_iterator iterCur = particles.begin();
		ParticleListType::const_iterator iterEnd = particles.end();
		int nTriID = 0;
		do
		{
			if ((nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice),
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice / 3;

				for (int i = 0; i < nLockedNum && (iterCur != iterEnd); ++i)
				{
					float fSize = iterCur->size;
					int nTile = std::min((int)tiles.size() - 1, std::max(0, iterCur->tile));
					DWORD nColor = LinearColor((const float*)&(iterCur->color));
					int nVB = 3 * i;

					if (rotate2SpeedDirection && iterCur->speed.squaredLength() > 1e-6f)
					{
						Vector3 camForward(mbb.m[0][0], mbb.m[0][1], mbb.m[0][2]);
						Vector3 rotAxis = camForward;
						float dot = iterCur->speed.dotProduct(camForward);
						Vector3 projectDir = iterCur->speed - dot * camForward;
						if (projectDir.squaredLength() > 1e-6f)
						{
							projectDir.normalise();
							Vector3 upAxis(mbb.m[2][0], mbb.m[2][1], mbb.m[2][2]);
							float cosAng = std::min(1.0f, std::max(-1.0f, projectDir.dotProduct(upAxis)));
							float rotAngle = std::acos(cosAng);
							Vector3 referenceAxis = projectDir.crossProduct(upAxis);
							if (referenceAxis.dotProduct(rotAxis) < 0)
								rotAngle = -rotAngle;

							Quaternion rot(rotAxis, -rotAngle);
							Matrix4 rotMat(rot);
							rotMat = mbb * rotMat;
							bv0 = Vector3(0, -f, +f) * rotMat;
							bv1 = Vector3(0, +f, +f) * rotMat;
							bv2 = Vector3(0, +f, -f) * rotMat;
							bv3 = Vector3(0, -f, -f) * rotMat;
						}
					}

					if (iterCur->rotation > 0)
					{
						float c = cos(iterCur->rotation);
						float s = sin(iterCur->rotation);
						if (billboard)
						{
							bv0 = Vector3(0, -f * c - f * s, f * c - f * s) * mbb;
							bv1 = Vector3(0, f * c - f * s, f * c + f * s) * mbb;
							bv2 = Vector3(0, f * c + f * s, -f * c + f * s) * mbb;
							bv3 = Vector3(0, -f * c + f * s, -f * c - f * s) * mbb;
						}
						else
						{
							bv0 = Vector3(-f * c - f * s, 0, f * c - f * s);
							bv1 = Vector3(f * c - f * s, 0, f * c + f * s);
							bv2 = Vector3(f * c + f * s, 0, -f * c + f * s);
							bv3 = Vector3(-f * c + f * s, 0, -f * c - f * s);
						}
					}

					if (nTriID == 0)
					{
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;

						++nVB;
						vb_vertices[nVB].p = (iterCur->pos + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[1].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[1].y;
						vb_vertices[nVB].color = nColor;
						++nVB;

						vb_vertices[nVB].p = (iterCur->trail_length == 0.f) ? (iterCur->pos + bv2 * fSize) : (iterCur->origin + bv2 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
					}
					else
					{
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;
						++nVB;

						vb_vertices[nVB].p = (iterCur->trail_length == 0.f) ? (iterCur->pos + bv2 * fSize) : (iterCur->origin + bv2 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
						++nVB;

						vb_vertices[nVB].p = (iterCur->trail_length == 0.f) ? (iterCur->pos + bv3 * fSize) : (iterCur->origin + bv3 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[3].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[3].y;
						vb_vertices[nVB].color = nColor;
					}

					if (++nTriID >= 2)
					{
						nTriID = 0;
						iterCur++;
					}
				}
				pBufEntity->Unlock();
				if (pBufEntity->IsMemoryBuffer())
					RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

				if ((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
					nNumFinishedVertice += (nLockedNum * 3);
				else
					break;
			}
			else
				break;
		} while (1);
	}
	else if (type == 1) {
		bv0 = Vector3(0, -1.0f, 0) * mbb;
		bv1 = Vector3(0, +1.0f, 0) * mbb;

		int nTotalVertices = (int)particles.size() * 6;
		ParticleListType::const_iterator iterCur = particles.begin();
		ParticleListType::const_iterator iterEnd = particles.end();
		int nTriID = 0;
		do
		{
			if ((nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice),
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice / 3;

				for (int i = 0; i < nLockedNum && (iterCur != iterEnd); ++i)
				{
					float fSize = iterCur->size;
					int nTile = std::min((int)tiles.size() - 1, std::max(0, iterCur->tile));
					DWORD nColor = LinearColor((const float*)&(iterCur->color));
					int nVB = 3 * i;

					if (nTriID == 0)
					{
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->pos + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[1].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[1].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->origin + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
					}
					else
					{
						vb_vertices[nVB].p = (iterCur->pos + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[0].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[0].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->origin + bv1 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[2].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[2].y;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = (iterCur->origin + bv0 * fSize);
						vb_vertices[nVB].tu = tiles[nTile].tc[3].x;
						vb_vertices[nVB].tv = tiles[nTile].tc[3].y;
						vb_vertices[nVB].color = nColor;
					}
					if (++nTriID >= 2)
					{
						nTriID = 0;
						iterCur++;
					}
				}
				pBufEntity->Unlock();
				if (pBufEntity->IsMemoryBuffer())
					RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

				if ((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
					nNumFinishedVertice += (nLockedNum * 3);
				else
					break;
			}
			else
				break;
		} while (1);
	}
}

void ParticleSystem::InitCommonParticleFields(Particle& p, int anim, int time) const
{
	p.life = 0.0f;
	p.maxlife = std::max(0.01f, lifespan.data.empty() ? 1.0f : lifespan.getValue(anim, time));

	// Legacy exporters can leave a placeholder key on an absent optional track.
	// V2 emitters also support explicit constant values without animation ranges.
	const auto hasOptionalKeys = [this](const Animated<float>& track) {
		return !track.data.empty() && (m_bExtendedV2 || !track.ranges.empty());
	};
	p.random_force = 0.f;
	if (hasOptionalKeys(spread))
		p.random_force = spread.getValue(anim, time);

	p.trail_length = 0.f;
	if ((type == 0 || type == 2) && hasOptionalKeys(lat))
		p.trail_length = lat.getValue(anim, time);

	p.size = sizes[0];
	p.colorTint = palette.empty() ? Vector4(1, 1, 1, 1) : palette[randint(0, (int)palette.size() - 1)];
	p.color = Vector4(
		colors[0].x * p.colorTint.x,
		colors[0].y * p.colorTint.y,
		colors[0].z * p.colorTint.z,
		colors[0].w * p.colorTint.w
	);
	p.origin = p.pos;
	p.tile = randint(0, std::max(0, rows * cols - 1));
	p.trailTimer = 0.0f;
	p.twinklePhase = randfloat(0.0f, 2.0f * MATH_PI);

	p.rotSpeed = 0.0f;
	if (hasOptionalKeys(grav2))
	{
		float radValue = randfloat(-0.2f, 0.2f);
		float rs = grav2.getValue(0, 0);
		p.rotSpeed = rs * (1.0f + radValue);
	}
	p.rotation = p.rotSpeed * 2.0f;
}

Particle PlaneParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	float w = sys->areal.getValue(anim, time) * 0.5f;
	float l = sys->areaw.getValue(anim, time) * 0.5f;
	float spd = sys->speed.getValue(anim, time);
	float var = sys->variation.getValue(anim, time);

	p.pos = sys->pos + Vector3(randfloat(-l, l), 0, randfloat(-w, w));
	if (sys->parent)
		p.pos = p.pos * sys->parent->mat;

	Vector3 baseDir = sys->m_bExtendedV2 ? sys->emitDir : Vector3(0, -1.f, 0);
	Vector3 dir = sys->parent ? (baseDir * sys->parent->mrot) : baseDir;
	p.down = Vector3(0, -1.0f, 0);
	p.speed = dir.normalisedCopy() * spd * (1.0f + randfloat(-var, var));

	sys->InitCommonParticleFields(p, anim, time);
	return p;
}

Particle SphereParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	float l = sys->areal.getValue(anim, time);
	float w = sys->areaw.getValue(anim, time);
	float spd = sys->speed.getValue(anim, time);
	float var = sys->variation.getValue(anim, time);

	float alpha = randfloat(0, 2 * MATH_PI);
	float cosAlpha = cosf(alpha);
	float sinAlpha = sinf(alpha);
	float cosTheta = cosf(sys->rotation);
	float sinTheta = sinf(sys->rotation);

	Vector3 born_pos(l * cosAlpha, 0, w * sinAlpha);
	p.pos = sys->pos + born_pos;
	if (sys->parent)
		p.pos = p.pos * sys->parent->mat;

	Vector3 born_dir(sinTheta * cosAlpha, -cosTheta, sinTheta * sinAlpha);
	Vector3 dir = sys->parent ? (born_dir * sys->parent->mrot) : born_dir;
	p.speed = dir * spd * (1.0f + randfloat(-var, var));

	p.down = (sys->useWorldGravity || !sys->parent) ? Vector3(0, -1.0f, 0) : (Vector3(0, -1.0f, 0) * sys->parent->mrot);

	sys->InitCommonParticleFields(p, anim, time);
	return p;
}

Particle SphereShellParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	float radiusL = (!sys->areal.data.empty()) ? sys->areal.getValue(anim, time) : 0.0f;
	float radiusW = (!sys->areaw.data.empty()) ? sys->areaw.getValue(anim, time) : 0.0f;
	float spd = (!sys->speed.data.empty()) ? sys->speed.getValue(anim, time) : 1.0f;
	float var = (!sys->variation.data.empty()) ? sys->variation.getValue(anim, time) : 0.0f;

	Vector3 axis, t1, t2;
	BuildBasisAroundDir(sys->emitDir, axis, t1, t2);

	float cosMax = std::cos(std::min((float)MATH_PI, std::max(0.0f, sys->maxPolar)));
	float cosMin = std::cos(std::min((float)MATH_PI, std::max(0.0f, sys->minPolar)));
	if (cosMin < cosMax)
		std::swap(cosMin, cosMax);

	float cosTheta = randfloat(cosMax, cosMin);
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float phi = randfloat(0.0f, 2.0f * MATH_PI);

	Vector3 localDir = axis * cosTheta + (t1 * std::cos(phi) + t2 * std::sin(phi)) * sinTheta;
	float shellFactor = (sys->shellThickness > 0.0f)
		? (1.0f - sys->shellThickness * (1.0f - std::cbrt(randfloat(0.0f, 1.0f))))
		: 1.0f;

	Vector3 bornOffset(localDir.x * radiusL * shellFactor, localDir.y * std::max(radiusL, radiusW) * shellFactor, localDir.z * radiusW * shellFactor);
	p.pos = sys->pos + bornOffset;
	if (sys->parent)
		p.pos = p.pos * sys->parent->mat;

	Vector3 worldDir = sys->parent ? (localDir * sys->parent->mrot) : localDir;
	p.speed = worldDir * (spd * shellFactor * (1.0f + randfloat(-var, var)));
	p.down = (sys->useWorldGravity || !sys->parent) ? Vector3(0, -1.0f, 0) : (Vector3(0, -1.0f, 0) * sys->parent->mrot);

	sys->InitCommonParticleFields(p, anim, time);
	return p;
}

Particle PatternParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	float spd = (!sys->speed.data.empty()) ? sys->speed.getValue(anim, time) : 1.0f;
	float var = (!sys->variation.data.empty()) ? sys->variation.getValue(anim, time) : 0.0f;
	float radiusScale = (!sys->areal.data.empty()) ? sys->areal.getValue(anim, time) : 0.0f;

	Vector3 axis, t1, t2;
	BuildBasisAroundDir(sys->emitDir, axis, t1, t2);

	Vector3 patternVec(0, 0, 0);
	const int pat = sys->patternType;
	if (pat == PATTERN_HEART || pat == PATTERN_STAR5)
	{
		// Keep upright silhouettes standing vertically (+Y = top of heart/star), with emitDir as the facing normal
		Vector3 depthVec = (std::fabs(axis.y) > 0.9f) ? Vector3(1, 0, 0) : axis;
		Vector3 upVec = Vector3(0, 1, 0) - depthVec * depthVec.y;
		if (upVec.squaredLength() < 1e-6f)
			upVec = Vector3(0, 1, 0);
		else
			upVec.normalise();
		Vector3 rightVec = upVec.crossProduct(depthVec).normalisedCopy();

		if (pat == PATTERN_HEART)
		{
			float t = randfloat(0.0f, 2.0f * MATH_PI);
			float s = std::sin(t);
			float hx = (16.0f * s * s * s) / 16.0f;
			float hy = (13.0f * std::cos(t) - 5.0f * std::cos(2.0f * t) - 2.0f * std::cos(3.0f * t) - std::cos(4.0f * t)) / 16.0f;
			patternVec = rightVec * hx + upVec * hy + depthVec * randfloat(-0.06f, 0.06f);
		}
		else
		{
			int edge = randint(0, 9);
			float frac = randfloat(0.0f, 1.0f);
			float a0 = (edge * MATH_PI / 5.0f) + (MATH_PI * 0.5f);
			float a1 = ((edge + 1) * MATH_PI / 5.0f) + (MATH_PI * 0.5f);
			float r0 = (edge % 2 == 0) ? 1.0f : 0.382f;
			float r1 = (edge % 2 == 0) ? 0.382f : 1.0f;
			float px = interpolate<float>(frac, r0 * std::cos(a0), r1 * std::cos(a1));
			float py = interpolate<float>(frac, r0 * std::sin(a0), r1 * std::sin(a1));
			patternVec = rightVec * px + upVec * py + depthVec * randfloat(-0.05f, 0.05f);
		}
	}
	else if (pat == PATTERN_DOUBLE_RING)
	{
		// Saturn shell: outer tilted ring (60%) + inner spherical core (40%)
		if (randfloat(0.0f, 1.0f) < 0.6f)
		{
			float phi = randfloat(0.0f, 2.0f * MATH_PI);
			float tilt = (sys->rotation != 0.0f) ? sys->rotation : 0.42f;
			Vector3 ringDir = t1 * std::cos(phi) + (t2 * std::cos(tilt) + axis * std::sin(tilt)) * std::sin(phi);
			patternVec = ringDir * 1.15f;
		}
		else
		{
			float cosT = randfloat(-1.0f, 1.0f);
			float sinT = std::sqrt(std::max(0.0f, 1.0f - cosT * cosT));
			float phi = randfloat(0.0f, 2.0f * MATH_PI);
			patternVec = (axis * cosT + (t1 * std::cos(phi) + t2 * std::sin(phi)) * sinT) * 0.52f;
		}
	}
	else if (pat == PATTERN_SPIRAL)
	{
		float arm = (float)randint(0, 2) * (2.0f * MATH_PI / 3.0f);
		float u = randfloat(0.15f, 1.0f);
		float phi = arm + u * 4.5f;
		patternVec = (t1 * std::cos(phi) + t2 * std::sin(phi)) * u + axis * ((1.0f - u) * 0.3f);
	}
	else // PATTERN_RING (default)
	{
		float phi = randfloat(0.0f, 2.0f * MATH_PI);
		float tilt = sys->rotation;
		patternVec = t1 * std::cos(phi) + (t2 * std::cos(tilt) + axis * std::sin(tilt)) * std::sin(phi);
	}

	p.pos = sys->pos + patternVec * radiusScale;
	if (sys->parent)
		p.pos = p.pos * sys->parent->mat;

	Vector3 worldVec = sys->parent ? (patternVec * sys->parent->mrot) : patternVec;
	p.speed = worldVec * (spd * (1.0f + randfloat(-var, var)));
	p.down = (sys->useWorldGravity || !sys->parent) ? Vector3(0, -1.0f, 0) : (Vector3(0, -1.0f, 0) * sys->parent->mrot);

	sys->InitCommonParticleFields(p, anim, time);
	return p;
}

Particle ConeJetParticleEmitter::newParticle(int anim, int time)
{
	Particle p;
	float w = (!sys->areal.data.empty()) ? sys->areal.getValue(anim, time) * 0.5f : 0.0f;
	float l = (!sys->areaw.data.empty()) ? sys->areaw.getValue(anim, time) * 0.5f : 0.0f;
	float spd = (!sys->speed.data.empty()) ? sys->speed.getValue(anim, time) : 1.0f;
	float var = (!sys->variation.data.empty()) ? sys->variation.getValue(anim, time) : 0.0f;

	Vector3 axis, t1, t2;
	BuildBasisAroundDir(sys->emitDir, axis, t1, t2);

	float minAng = std::min((float)MATH_PI, std::max(0.0f, sys->minPolar));
	float maxAng = minAng;
	if (sys->maxPolar > 0.0f && sys->maxPolar < (float)MATH_PI - 1e-4f)
	{
		maxAng = std::min((float)MATH_PI, sys->maxPolar);
	}
	else if (minAng > 0.0f)
	{
		// Legacy / unit-test shorthand where only minPolar was set as cone half-angle
		minAng = 0.0f;
	}
	float cosMax = std::cos(std::max(minAng, maxAng));
	float cosMin = std::cos(std::min(minAng, maxAng));
	float cosTheta = randfloat(cosMax, cosMin);
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float phi = randfloat(0.0f, 2.0f * MATH_PI);

	Vector3 localDir = axis * cosTheta + (t1 * std::cos(phi) + t2 * std::sin(phi)) * sinTheta;
	p.pos = sys->pos + t1 * randfloat(-l, l) + t2 * randfloat(-w, w);
	if (sys->parent)
		p.pos = p.pos * sys->parent->mat;

	Vector3 worldDir = sys->parent ? (localDir * sys->parent->mrot) : localDir;
	p.speed = worldDir * (spd * (1.0f + randfloat(-var, var)));
	p.down = (sys->useWorldGravity || !sys->parent) ? Vector3(0, -1.0f, 0) : (Vector3(0, -1.0f, 0) * sys->parent->mrot);

	sys->InitCommonParticleFields(p, anim, time);
	return p;
}

//////////////////////////////////////////////////////////////////////////
// RibbonEmitter
//////////////////////////////////////////////////////////////////////////

RibbonEmitter::~RibbonEmitter()
{
	ClearAllInstances();
}

TextureEntity* ParaEngine::RibbonEmitter::GetTexture()
{
	if (model && m_texture_index >= 0 && m_texture_index < (int32)model->GetObjectNum().nTextures)
	{
		return model->textures[m_texture_index].get();
	}
	return NULL;
}

ParaEngine::DeviceTexturePtr_type ParaEngine::RibbonEmitter::GetDeviceTexture()
{
	auto pTexture = GetTexture();
	return pTexture != 0 ? pTexture->GetTexture() : 0;
}

RibbonInstance* RibbonEmitter::GetCurrentInstance(SceneState* pSceneState, bool bForceCreate)
{
	void* owner = pSceneState ? (void*)(pSceneState->GetCurrentSceneObject()) : nullptr;
	auto iter = m_instances.find(owner);
	if (iter == m_instances.end())
	{
		if (!bForceCreate)
			return nullptr;
		RibbonInstance* inst = new RibbonInstance();
		inst->m_pSceneState = pSceneState;
		inst->m_bUseAbsCord = m_bExtendedV2 ? m_bUseAbsCord : true;
		m_instances[owner] = inst;
		return inst;
	}
	return iter->second;
}

void RibbonEmitter::ClearAllInstances()
{
	for (auto& kv : m_instances)
	{
		SAFE_DELETE(kv.second);
	}
	m_instances.clear();
}

bool RibbonEmitter::DeleteInstance(void* owner)
{
	auto iter = m_instances.find(owner);
	if (iter != m_instances.end())
	{
		SAFE_DELETE(iter->second);
		m_instances.erase(iter);
	}
	return true;
}

void RibbonEmitter::setup(SceneState* pSceneState, int anim, int time)
{
	manim = anim;
	mtime = time;
	RibbonInstance* inst = GetCurrentInstance(pSceneState, true);
	if (inst)
	{
		inst->m_anim = anim;
		inst->m_time = time;
	}
}

bool RibbonEmitter::AnimateExistingSegments(float dt, const Vector3& vOffset, RibbonInstance* inst)
{
	if (!inst || inst->segs.empty())
		return false;

	const float effectiveMaxLife = (maxLife > 0.01f) ? maxLife : std::max(0.15f, (float)std::max(2, numsegs) * 0.04f);
	const float maxTotalLength = (length > 0.001f && !m_bExtendedV2)
		? length
		: (std::max(0.02f, seglen) * (float)std::max(2, numsegs));

	float accumLen = 0.0f;
	for (size_t i = 0; i < inst->segs.size(); )
	{
		RibbonSegment& seg = inst->segs[i];
		if (inst->m_bUseAbsCord)
			seg.pos += vOffset;
		seg.age += dt;
		if (i > 0)
			accumLen += (inst->segs[i - 1].pos - seg.pos).length();

		if (seg.age >= effectiveMaxLife || (i > 1 && accumLen > maxTotalLength))
		{
			inst->segs.resize(i);
			break;
		}
		++i;
	}
	return inst->segs.size() >= 2;
}

void RibbonEmitter::update(SceneState* pSceneState, float dt)
{
	RibbonInstance* inst = GetCurrentInstance(pSceneState, true);
	if (!inst || inst->m_bUpdated)
		return;

	pSceneState->AddRibbonEmitter(this);
	inst->m_bUpdated = true;

	Vector3 vRenderOriginOffset(0, 0, 0);
	Matrix4 mWorld = Matrix4::IDENTITY;
	if (inst->m_bUseAbsCord)
	{
		mWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
		Vector3 vRenderOrigin(0, 0, 0);
		if (pSceneState->GetScene())
			vRenderOrigin = pSceneState->GetScene()->GetRenderOrigin();
		if (inst->m_bInitialized)
		{
			vRenderOriginOffset = inst->m_vLastRenderOrigin - vRenderOrigin;
			inst->m_vLastPos += vRenderOriginOffset;
		}
		inst->m_vLastRenderOrigin = vRenderOrigin;
	}

	AnimateExistingSegments(dt, vRenderOriginOffset, inst);

	int anim = inst->m_anim;
	int time = inst->m_time;

	Vector3 localPos = parent ? (pos * parent->mat) : pos;
	Vector3 localUp = parent ? (((pos + Vector3(0, 0, 1)) * parent->mat) - localPos) : Vector3(0, 1, 0);
	Vector3 ntpos = inst->m_bUseAbsCord ? (localPos * mWorld) : localPos;
	Vector3 ntup = inst->m_bUseAbsCord ? localUp.TransformNormal(mWorld) : localUp;
	if (ntup.squaredLength() > 1e-6f)
		ntup.normalise();
	else
		ntup = Vector3(0, 1, 0);

	Vector3 col = (!color.data.empty()) ? color.getValue(anim, time) : Vector3(1, 1, 1);
	float opac = (!opacity.data.empty()) ? opacity.getValue(anim, time) : 1.0f;
	tcolor = Vector4(col.x, col.y, col.z, opac);
	tabove = (!above.data.empty()) ? above.getValue(anim, time) : 0.1f;
	tbelow = (!below.data.empty()) ? below.getValue(anim, time) : 0.1f;

	const float stepThreshold = std::max(0.01f, seglen);
	const int maxSegments = std::max(2, numsegs);

	if (!inst->m_bInitialized)
	{
		inst->m_bInitialized = true;
		inst->m_vLastPos = ntpos;
		if (opac > 0.001f)
		{
			RibbonSegment seg;
			seg.pos = ntpos;
			seg.up = ntup;
			seg.age = 0.0f;
			seg.headColor = tcolor;
			inst->segs.push_front(seg);
		}
		return;
	}

	float dlen = (ntpos - inst->m_vLastPos).length();
	if (maxStepDist > 0.0f && dlen > maxStepDist)
	{
		// Break ribbon cleanly across teleports or animation loop resets
		inst->segs.clear();
		inst->m_vLastPos = ntpos;
		if (opac > 0.001f)
		{
			RibbonSegment seg;
			seg.pos = ntpos;
			seg.up = ntup;
			seg.age = 0.0f;
			seg.headColor = tcolor;
			inst->segs.push_front(seg);
		}
		return;
	}

	inst->m_vLastPos = ntpos;
	if (opac <= 0.001f)
		return;

	if (inst->segs.empty())
	{
		RibbonSegment seg;
		seg.pos = ntpos;
		seg.up = ntup;
		seg.age = 0.0f;
		seg.headColor = tcolor;
		inst->segs.push_front(seg);
	}
	else if (inst->segs.size() == 1)
	{
		if ((ntpos - inst->segs.front().pos).length() > 0.001f)
		{
			RibbonSegment head;
			head.pos = ntpos;
			head.up = ntup;
			head.age = 0.0f;
			head.headColor = tcolor;
			inst->segs.push_front(head);
		}
		else
		{
			inst->segs.front().age = 0.0f;
			inst->segs.front().headColor = tcolor;
		}
	}
	else
	{
		// Keep front vertex anchored to live bone tip; commit a new knot whenever distance from segs[1] >= stepThreshold
		float distFromPrevKnot = (ntpos - inst->segs[1].pos).length();
		if (distFromPrevKnot >= stepThreshold)
		{
			RibbonSegment head;
			head.pos = ntpos;
			head.up = ntup;
			head.age = 0.0f;
			head.headColor = tcolor;
			inst->segs.push_front(head);
			if ((int)inst->segs.size() > maxSegments + 1)
				inst->segs.resize(maxSegments + 1);
		}
		else
		{
			inst->segs.front().pos = ntpos;
			inst->segs.front().up = ntup;
			inst->segs.front().age = 0.0f;
			inst->segs.front().headColor = tcolor;
		}
	}
}

void RibbonEmitter::draw(SceneState* pSceneState)
{
	RibbonInstance* inst = GetCurrentInstance(pSceneState, false);
	if (!inst)
		return;
	inst->m_bRender = true;
	if (!inst->m_bUseAbsCord)
		drawInstance(inst);
}

void RibbonEmitter::drawInstance(RibbonInstance* inst)
{
	if (!inst || inst->segs.size() < 2)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice)
		return;

	if (inst->m_bUseAbsCord)
		CGlobals::GetWorldMatrixStack().push(*CGlobals::GetIdentityMatrix());

	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	if (pEffect != nullptr)
	{
		ApplyParticleBlendState(pd3dDevice, pEffect, blend);
		DeviceTexturePtr_type pTex = GetDeviceTexture();
		if (pTex)
		{
			if (pEffect->begin())
			{
				if (pEffect->BeginPass(0))
				{
					pEffect->setTexture(0, pTex);
					pEffect->CommitChanges();
					DrawInstanceSub(inst);
				}
				pEffect->EndPass(0);
			}
			pEffect->end();
		}
	}

	if (inst->m_bUseAbsCord)
		CGlobals::GetWorldMatrixStack().pop();

	RestoreParticleBlendState(pd3dDevice, blend);
}

void RibbonEmitter::DrawInstanceSub(RibbonInstance* inst)
{
	if (!inst || inst->segs.size() < 2)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	const size_t nPoints = inst->segs.size();
	const size_t nQuads = nPoints - 1;

	Matrix4 mtrans = CGlobals::GetViewMatrixStack().SafeGetTop();
	if (!inst->m_bUseAbsCord)
		mtrans = CGlobals::GetWorldMatrixStack().SafeGetTop() * mtrans;
	mtrans = mtrans.inverse();
	Vector3 camPos = mtrans.getTrans();

	const float effectiveMaxLife = (maxLife > 0.01f) ? maxLife : std::max(0.15f, (float)std::max(2, numsegs) * 0.04f);

	// Precompute top/bottom ribbon edge vertices & colors along the chain
	struct EdgePair {
		Vector3 top, bottom;
		float u;
		DWORD color;
	};
	std::vector<EdgePair> edges(nPoints);
	for (size_t i = 0; i < nPoints; ++i)
	{
		const RibbonSegment& seg = inst->segs[i];
		float frac = (float)i / (float)(nPoints - 1);
		float lifeFrac = std::min(1.0f, std::max(0.0f, seg.age / effectiveMaxLife));
		float fade = std::max(frac, lifeFrac);

		Vector3 side = seg.up;
		if (orientCamera)
		{
			Vector3 tangent = (i + 1 < nPoints) ? (inst->segs[i].pos - inst->segs[i + 1].pos) : (inst->segs[i - 1].pos - inst->segs[i].pos);
			Vector3 toCam = camPos - seg.pos;
			Vector3 cross = tangent.crossProduct(toCam);
			if (cross.squaredLength() > 1e-8f)
				side = cross.normalisedCopy();
		}

		float wScale = interpolate<float>(fade, 1.0f, widthEndScale);
		edges[i].top = seg.pos + side * (tabove * wScale);
		edges[i].bottom = seg.pos - side * (tbelow * wScale);
		edges[i].u = frac;

		Vector4 tailTarget = hasEndColor ? colorEnd : Vector4(seg.headColor.x, seg.headColor.y, seg.headColor.z, 0.0f);
		Vector4 c = interpolate<Vector4>(fade, seg.headColor, tailTarget);
		edges[i].color = LinearColor((const float*)&c);
	}

	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, pBufEntity->m_nUnitSize);

	SPRITEVERTEX* vb_vertices = nullptr;
	int nTotalVertices = (int)(nQuads * 6);
	int nNumFinishedVertice = 0;
	size_t quadIdx = 0;
	int nTriID = 0;

	while (quadIdx < nQuads)
	{
		int nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice), (void**)(&vb_vertices));
		if (nNumLockedVertice <= 0)
			break;

		int nLockedTris = nNumLockedVertice / 3;
		int trisWritten = 0;
		for (int t = 0; t < nLockedTris && quadIdx < nQuads; ++t)
		{
			const EdgePair& e0 = edges[quadIdx];
			const EdgePair& e1 = edges[quadIdx + 1];
			int nVB = 3 * t;

			if (nTriID == 0)
			{
				vb_vertices[nVB].p = e0.top;    vb_vertices[nVB].tu = e0.u; vb_vertices[nVB].tv = 0.0f; vb_vertices[nVB].color = e0.color;
				++nVB;
				vb_vertices[nVB].p = e0.bottom; vb_vertices[nVB].tu = e0.u; vb_vertices[nVB].tv = 1.0f; vb_vertices[nVB].color = e0.color;
				++nVB;
				vb_vertices[nVB].p = e1.top;    vb_vertices[nVB].tu = e1.u; vb_vertices[nVB].tv = 0.0f; vb_vertices[nVB].color = e1.color;
			}
			else
			{
				vb_vertices[nVB].p = e0.bottom; vb_vertices[nVB].tu = e0.u; vb_vertices[nVB].tv = 1.0f; vb_vertices[nVB].color = e0.color;
				++nVB;
				vb_vertices[nVB].p = e1.bottom; vb_vertices[nVB].tu = e1.u; vb_vertices[nVB].tv = 1.0f; vb_vertices[nVB].color = e1.color;
				++nVB;
				vb_vertices[nVB].p = e1.top;    vb_vertices[nVB].tu = e1.u; vb_vertices[nVB].tv = 0.0f; vb_vertices[nVB].color = e1.color;
			}
			++trisWritten;
			if (++nTriID >= 2)
			{
				nTriID = 0;
				++quadIdx;
			}
		}

		pBufEntity->Unlock();
		if (trisWritten > 0)
		{
			if (pBufEntity->IsMemoryBuffer())
				RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, trisWritten, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
			else
				RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), trisWritten);
		}
		nNumFinishedVertice += trisWritten * 3;
		if (nNumFinishedVertice >= nTotalVertices)
			break;
	}
}

void ParaEngine::ParticleSystem::ClearAllInstances()
{
	for (auto& kv : m_instances)
	{
		SAFE_DELETE(kv.second);
	}
	m_instances.clear();
}

TextureEntity* ParaEngine::ParticleSystem::GetTexture()
{
	if (model && m_texture_index >= 0 && m_texture_index < (int32)model->GetObjectNum().nTextures)
	{
		return model->textures[m_texture_index].get();
	}
	return NULL;
}

ParaEngine::DeviceTexturePtr_type ParaEngine::ParticleSystem::GetDeviceTexture()
{
	auto pTexture = GetTexture();
	return pTexture != 0 ? pTexture->GetTexture() : 0;
}

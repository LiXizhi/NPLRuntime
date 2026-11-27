//-----------------------------------------------------------------------------
// Class: weather particle 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.12.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseCamera.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "WeatherParticleSpawner.h"
#include "BatchedElementDraw.h"
#include "BlockEngine/BlockWorldClient.h"
#include "WeatherEffect.h"
#include "WeatherParticle.h"

using namespace ParaEngine;
/** define this if we pre-calculate ground height. */
#define PRE_PRECIPITATION_HEIGHT_CALCULATION

ParaEngine::WeatherParticle::WeatherParticle(WeatherParticleSpawner* pParent)
	: m_pWeatherSpawner(pParent), x(0.f), y(0.f), z(0.f), speed_x(0.f), speed_y(0.f), speed_z(0.f),
	lifetime(3.f), minY(0.f),
	width(1.f), height(1.f), gravity(9.18f), m_isDead(false)
{
	if (m_pWeatherSpawner)
		m_pWeatherSpawner->addref();
}


ParaEngine::WeatherParticle::~WeatherParticle()
{
	SAFE_RELEASE(m_pWeatherSpawner);
}

WeatherParticleSpawner* ParaEngine::WeatherParticle::GetWeatherSpawner()
{
	return m_pWeatherSpawner;
}

void ParaEngine::WeatherParticle::Init(float x_, float y_, float z_)
{
	auto pSpawner = GetWeatherSpawner();
	
	m_isDead = false;
	detailed = pSpawner->GetWeatherEffect()->IsDetailedPrecipitation();
	opacity = 1.f;
	x = x_;
	y = y_;
	z = z_;

	speed_x = 0;
	speed_y = -(pSpawner->GetSpeedRand().random() * 2.f + 0.3f);
	speed_z = 0;

	lifetime = 3.f;

	width = height = 0.1f;

	SetTexture(pSpawner->GetTexture());
	tileIndex = pSpawner->GetRandomTileIndex();
}


void ParaEngine::WeatherParticle::Reset()
{
	speed_x = 0;
	speed_y = 0;
	speed_z = 0;
	m_isDead = false;
}

void ParaEngine::WeatherParticle::SetDead()
{
	m_isDead = true;
}

void ParaEngine::WeatherParticle::SetFallOnGround()
{
	if (detailed || GetWeatherSpawner()->GetSpeedRand().randomBoolean())
	{
		SetDead();
	}
}

void ParaEngine::WeatherParticle::FrameMove(float deltaTime)
{
	if (detailed)
	{
		const float nextX = x + speed_x * deltaTime;
		const float nextZ = z + speed_z * deltaTime;
		uint16 bx, bz, nextBx, nextBz;
		int16_t by;
		BlockCommon::ConvertToBlockIndex(x, y, z, bx, by, bz);
		BlockCommon::ConvertToBlockIndex(nextX, y, nextZ, nextBx, by, nextBz);
		if (bx != nextBx || bz != nextBz)
		{
			minY = CWeatherEffect::GetPrecipitationSurface(nextBx, nextBz);
			// Wind can carry a particle into the side of an awning below its roof.
			if (y < minY)
			{
				SetDead();
				return;
			}
		}
		x = nextX;
		z = nextZ;
		speed_y -= gravity * deltaTime;
		y += speed_y * deltaTime;
		lifetime -= deltaTime;
		if (y <= minY)
		{
			y = minY;
			SetFallOnGround();
		}
		else if (lifetime <= 0.f)
			SetDead();
		return;
	}
	speed_y = speed_y - gravity * deltaTime;
	y = y + speed_y * deltaTime;

	// air friction
	speed_y *= 0.98f;

#ifdef PRE_PRECIPITATION_HEIGHT_CALCULATION
	if (y < GetMinY())
	{
		y = GetMinY();
		SetFallOnGround();
	}

#else
	uint16 bx, by, bz;
	BlockCommon::ConvertToBlockIndex(x, y, z, bx, by, bz);
	auto block_class = BlockWorldClient::GetInstance()->GetBlockTemplate(bx, by, bz);
	if (block_class && block_class->IsMatchAttribute(BlockTemplate::batt_obstruction | BlockTemplate::batt_solid | BlockTemplate::batt_liquid))
	{
		float surface_height = BlockCommon::ConvertToRealY(by) + (float)BlockConfig::g_dBlockSize;
		if (y < surface_height)
			SetFallOnGround();
	}
#endif
	if (lifetime >= 0)
		lifetime -= deltaTime;
	if (lifetime < 0.f)
		SetDead();
}

void ParaEngine::WeatherParticle::RecollectMe()
{
	GetWeatherSpawner()->RecollectEntity(this);
}

void ParaEngine::WeatherParticle::Draw(SceneState * pSceneState, CCameraFrustum* pFrustum, const Vector3& vRenderOffset)
{
	Vector3 v[2] = {
		{ x + vRenderOffset.x, y + vRenderOffset.y - height, z + vRenderOffset.z },
		{ x + vRenderOffset.x, y + vRenderOffset.y + height, z + vRenderOffset.z }
	};
	if (pFrustum->CullPointsWithFrustum(v, 2))
	{

		//#define DEBUG_PARTICLE_DRAW
#ifdef DEBUG_PARTICLE_DRAW
		Vector3 vTo(vFrom.x, vFrom.y + 1.f, vFrom.z);
		PARAVECTOR4 color(1.f, 0.f, 0.f, 1.f);
		pSceneState->GetBatchedElementDrawer()->AddThickLine(reinterpret_cast<const PARAVECTOR3&>(vFrom), reinterpret_cast<const PARAVECTOR3&>(vTo), color, 0.1f);
#else
		SetRenderOffset(vRenderOffset);
		pSceneState->GetBatchedElementDrawer()->AddParticle(this);
#endif
	}
}

int ParaEngine::WeatherParticle::RenderParticle(SPRITEVERTEX** ppVertexBuffer, SceneState* pSceneState)
{
	// the number of particles generated (1 particle has 2 triangles with 6 vertices).
	int nNumOfParticles = 0;
	if (!IsDead())
	{
		float vX = x + m_vRenderOffset.x;
		float vY = y + m_vRenderOffset.y;
		float vZ = z + m_vRenderOffset.z;
		ParticleRectUV& uv = GetWeatherSpawner()->GetTileUV(tileIndex);

		uint8 brightness;
		Uint16x3 blockPos;
		BlockCommon::ConvertToBlockIndex(x, y + height + 0.01f, z, blockPos.x, blockPos.y, blockPos.z);
		BlockWorldClient::GetInstance()->GetBlockBrightness(blockPos, &brightness, 1, -1);
		brightness *= 16;
		const float fade = detailed ? Math::Min(1.f, Math::Max(0.f, lifetime) * 5.f) : 1.f;
		DWORD color = Color(brightness, brightness, brightness, (uint8)(255.f * opacity * fade));

		SPRITEVERTEX* pVertexBuffer = *ppVertexBuffer;
		// 2 triangle list with 6 vertices
		auto& bb = pSceneState->BillBoardInfo();
		Vector3 right = bb.TransformVertexWithoutY(Vector3(width, 0.f, 0.f));
		Vector3 up(0.f, height, 0.f);
		if (detailed && GetType() == WeatherType_Rain)
		{
			// The top of the streak trails behind the falling drop.
			up.x = height * speed_x / speed_y;
			up.z = height * speed_z / speed_y;
		}
		else if (detailed && GetType() == WeatherType_Snow)
		{
			auto* camera = pSceneState->GetScene()->GetCurrentCamera();
			const Vector3 across = camera->GetWorldRight();
			const Vector3 vertical = camera->GetWorldUp();
			const float tilt = 0.7f * sinf(phase);
			const float c = cosf(tilt), s = sinf(tilt);
			right = (across * c + vertical * s) * (width * (0.55f + 0.45f * fabsf(cosf(phase))));
			up = (vertical * c - across * s) * height;
		}
		const Vector3 corners[] = {-right - up, up - right, right + up, right - up};
		for (int i = 0; i < 4; ++i)
			SetParticleVertex(pVertexBuffer[i], vX + corners[i].x, vY + corners[i].y, vZ + corners[i].z, uv.tc[i].x, uv.tc[i].y, color);

		pVertexBuffer[4] = pVertexBuffer[0];
		pVertexBuffer[5] = pVertexBuffer[2];
		nNumOfParticles = 1;
		(*ppVertexBuffer) += 6;
	}
	return nNumOfParticles;
}

void ParaEngine::SnowParticle::Init(float x, float y, float z)
{
	isFloating = GetWeatherSpawner()->GetSpeedRand().randomDouble() > 0.1 ? true : false;
	float_speed = (float)((GetWeatherSpawner()->GetSpeedRand().randomDouble() - GetWeatherSpawner()->GetSpeedRand().randomDouble())*0.5);
	gravity = 9.81f*0.03f * GetWeatherSpawner()->GetSpeed();
	WeatherParticle::Init(x, y, z);
	height = width = 0.03f;
	if (detailed)
	{
		auto& rand = GetWeatherSpawner()->GetSpeedRand();
		const float size = rand.random();
		height = width = 0.018f + size * size * 0.075f;
		speed_y = -(0.55f + 0.85f * size) * Math::Max(0.f, GetWeatherSpawner()->GetSpeed());
		gravity = 0.f;
		phase = rand.random() * 2.f * MATH_PI;
		flutter = 1.4f + rand.random() * 2.0f;
		lifetime = 4.f + rand.random() * 2.f;
		opacity = 0.6f + 0.35f * size;
	}
}

void ParaEngine::SnowParticle::FrameMove(float deltaTime)
{
	if (detailed)
	{
		phase += flutter * deltaTime;
		const float wind = GetWeatherSpawner()->GetWeatherEffect()->GetWind();
		speed_x = 0.72f * wind + 0.43f * sinf(phase);
		speed_z = 0.24f * wind + 0.38f * cosf(phase * 0.86f);
	}
	else if (isFloating)
	{
		x = x + float_speed*deltaTime;
		z = z + float_speed*deltaTime;
	}
	WeatherParticle::FrameMove(deltaTime);
}

void ParaEngine::RainParticle::Init(float x, float y, float z)
{
	WeatherParticle::Init(x, y, z);
	height = 0.16f;	width = 0.04f;
	if (detailed)
	{
		auto& rand = GetWeatherSpawner()->GetSpeedRand();
		const float size = rand.random();
		height = 0.10f + 0.20f * size;
		width = 0.018f + 0.020f * size;
		speed_y = -(6.f + size * 4.f);
		const float wind = GetWeatherSpawner()->GetWeatherEffect()->GetWind();
		speed_x = wind * (1.6f + 0.12f * GetWeatherSpawner()->GetStrength());
		speed_z = 0.45f * wind;
		gravity = 0.f;
		opacity = 0.42f + 0.34f * size;
	}
}

void ParaEngine::RainParticle::SetDead()
{
	WeatherParticle::SetDead();
	if (!detailed)
		SpawnSplash();
}

void ParaEngine::RainParticle::SetFallOnGround()
{
	if (detailed)
	{
		SpawnSplash();
		SetDead();
	}
	else
		WeatherParticle::SetFallOnGround();
}

void ParaEngine::RainParticle::SpawnSplash()
{
	auto* effect = GetWeatherSpawner()->GetWeatherEffect();
	auto* world = BlockWorldClient::GetInstance();
	if (detailed && world->WaterRipplesEnabled())
	{
		const bool water = world->RecordRainImpact(x, GetMinY(), z);
		auto& rand = GetWeatherSpawner()->GetSpeedRand();
		const int drops = height > 0.21f ? 3 : 2;
		const float angle = rand.random() * 2.f * MATH_PI;
		for (int i = 0; i < drops; ++i)
		{
			auto* spray = static_cast<RainSplashParticle*>(effect->AddParticle(x, GetMinY(), z, WeatherType_RainSplash));
			if (!spray) break;
			spray->SetMinY(GetMinY());
			spray->SetSprayVelocity(angle + (i + 0.25f * rand.random()) * (2.f * MATH_PI / drops),
				(water ? 0.80f : 1.f) * (0.65f + 0.65f * rand.random()));
		}
		return;
	}
	WeatherParticle* entity = effect->AddParticle(x, GetMinY(), z, WeatherType_RainSplash);
	if (entity)
	{
		entity->SetMinY(detailed ? GetMinY() : GetMinY() - height);
	}
}

void ParaEngine::RainSplashParticle::Init(float x, float y, float z)
{
	WeatherParticle::Init(x, y, z);
	height = width = 0.1f;
	speed_y = 3.0f;
	speed_x = (float)(GetWeatherSpawner()->GetSpeedRand().randomDouble() - 0.5);
	speed_z = (float)(GetWeatherSpawner()->GetSpeedRand().randomDouble() - 0.5);
	gravity = 9.18f*2.0f;
	if (detailed)
	{
		auto& rand = GetWeatherSpawner()->GetSpeedRand();
		width = height = 0.024f + 0.040f * rand.random();
		speed_x *= 2.6f;
		speed_z *= 2.6f;
		speed_y = 0.9f + rand.random() * 1.4f;
		opacity = 0.68f;
		lifetime = 2.f * speed_y / gravity;
	}
}

void ParaEngine::RainSplashParticle::SetSprayVelocity(float angle, float energy)
{
	speed_x = cosf(angle) * energy;
	speed_z = sinf(angle) * energy;
	speed_y = 2.25f * energy;
	width = 0.022f + 0.014f * energy;
	height = width * 1.35f;
	opacity = 0.88f;
	lifetime = 2.f * speed_y / gravity;
}

void ParaEngine::RainSplashParticle::FrameMove(float deltaTime)
{
	if (!detailed)
	{
		x = x + speed_x*deltaTime;
		z = z + speed_z*deltaTime;
	}
	WeatherParticle::FrameMove(deltaTime);
}

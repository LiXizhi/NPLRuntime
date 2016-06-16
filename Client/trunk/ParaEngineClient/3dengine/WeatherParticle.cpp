//-----------------------------------------------------------------------------
// Class: weather particle 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.12.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseCamera.h"
#include "SceneState.h"
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
	if (GetWeatherSpawner()->GetSpeedRand().randomBoolean())
	{
		SetDead();
	}
}

void ParaEngine::WeatherParticle::FrameMove(float deltaTime)
{
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
		DWORD color = Color(brightness, brightness, brightness, 255);

		SPRITEVERTEX* pVertexBuffer = *ppVertexBuffer;
		// 2 triangle list with 6 vertices
		Vector3 v;

		BillBoardViewInfo& bbInfo = pSceneState->BillBoardInfo();
		v.x = -width; v.y = -height; v.z = 0;
		v = bbInfo.TransformVertexWithoutY(v);
		SetParticleVertex(pVertexBuffer[0], vX + v.x, vY + v.y, vZ + v.z, uv.tc[0].x, uv.tc[0].y, color);
		v.x = -width; v.y = height; v.z = 0;
		v = bbInfo.TransformVertexWithoutY(v);
		SetParticleVertex(pVertexBuffer[1], vX + v.x, vY + v.y, vZ + v.z, uv.tc[1].x, uv.tc[1].y, color);
		v.x = width; v.y = height; v.z = 0;
		v = bbInfo.TransformVertexWithoutY(v);
		SetParticleVertex(pVertexBuffer[2], vX + v.x, vY + v.y, vZ + v.z, uv.tc[2].x, uv.tc[2].y, color);
		v.x = width; v.y = -height; v.z = 0;
		v = bbInfo.TransformVertexWithoutY(v);
		SetParticleVertex(pVertexBuffer[3], vX + v.x, vY + v.y, vZ + v.z, uv.tc[3].x, uv.tc[3].y, color);

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
	gravity = 9.81f*0.03f;
	WeatherParticle::Init(x, y, z);
	height = width = 0.03f;
}

void ParaEngine::SnowParticle::FrameMove(float deltaTime)
{
	if (isFloating)
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
}

void ParaEngine::RainParticle::SetDead()
{
	WeatherParticle::SetDead();
	WeatherParticle* entity = GetWeatherSpawner()->GetWeatherEffect()->AddParticle(x, GetMinY(), z, WeatherType_RainSplash);
	if (entity)
	{
		entity->SetMinY(GetMinY() - height);
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
}

void ParaEngine::RainSplashParticle::FrameMove(float deltaTime)
{
	x = x + speed_x*deltaTime;
	z = z + speed_z*deltaTime;
	WeatherParticle::FrameMove(deltaTime);
}

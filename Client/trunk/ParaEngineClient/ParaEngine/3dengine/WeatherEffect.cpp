//-----------------------------------------------------------------------------
// Class: weather system 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.12.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockEngine/BlockWorldClient.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "BatchedElementDraw.h"
#include "WeatherParticleSpawner.h"
#include "WeatherEffect.h"

using namespace ParaEngine;

ParaEngine::CWeatherEffect::CWeatherEffect()
	:m_default_strength(4.f), m_updateTick(1/20.f)
{
	SetIdentifier("WeatherEffect");
	WeatherParticleSpawner* pRainSpawner = new WeatherParticleSpawner(WeatherType_Rain);
	pRainSpawner->SetIdentifier("rain");
	pRainSpawner->SetTextureFilename("Texture/blocks/particle_rain.png");
	pRainSpawner->SetTextureRowsCols(1, 4);
	pRainSpawner->SetWeatherEffect(this);
	pRainSpawner->SetSpawnRadius(20.f);
	AddChild(pRainSpawner);
	m_particle_spawners[WeatherType_Rain] = pRainSpawner;
	
	WeatherParticleSpawner* pSnowSpawner = new WeatherParticleSpawner(WeatherType_Snow);
	pSnowSpawner->SetIdentifier("snow");
	pSnowSpawner->SetTextureFilename("Texture/blocks/particle_snow.png");
	pSnowSpawner->SetTextureRowsCols(2, 2);
	pSnowSpawner->SetWeatherEffect(this);
	pSnowSpawner->SetSpawnRadius(20.f);
	AddChild(pSnowSpawner);
	m_particle_spawners[WeatherType_Snow] = pSnowSpawner;

	WeatherParticleSpawner* pRainSplashSpawner = new WeatherParticleSpawner(WeatherType_RainSplash);
	pRainSplashSpawner->SetIdentifier("rain_splash");
	pRainSplashSpawner->SetTextureFilename("Texture/blocks/particle_rain_splash.png");
	pRainSplashSpawner->SetTextureRowsCols(2, 2);
	pRainSplashSpawner->SetWeatherEffect(this);
	AddChild(pRainSplashSpawner);
	m_particle_spawners[WeatherType_RainSplash] = pRainSplashSpawner;
}

ParaEngine::CWeatherEffect::~CWeatherEffect()
{
	Cleanup();
}

bool ParaEngine::CWeatherEffect::HasActiveSpawners()
{
	for (const auto spawner_info : m_particle_spawners)
	{
		if (spawner_info->IsEnabled())
			return true;
	}
	return false;
}

ParaEngine::WeatherParticleSpawner& ParaEngine::CWeatherEffect::GetParticleSpawner(WeatherType weather_type)
{
	return *(m_particle_spawners[weather_type]);
}

void ParaEngine::CWeatherEffect::EnableWeatherItem(WeatherType weather_type, bool bEnabled)
{
	GetParticleSpawner(weather_type).SetEnabled(bEnabled);
}

void ParaEngine::CWeatherEffect::Cleanup()
{
	for (WeatherParticle* particle : m_particles)
	{
		particle->RecollectMe();
	}
	m_particles.clear();

	for (auto spawner_info : m_particle_spawners)
	{
		spawner_info->SetEnabled(false);
		spawner_info->Clear();
	}
}

bool ParaEngine::CWeatherEffect::IsWeatherItemEnabled(WeatherType weather_type)
{
	return GetParticleSpawner(weather_type).IsEnabled();
}

int32 ParaEngine::CWeatherEffect::GetTerrainHighestBlock(int32 x, int32 z, int32& outY)
{
	int16 y = BlockConfig::g_regionBlockDimY;
	int dist = BlockWorldClient::GetInstance()->FindFirstBlock((uint16)x, (uint16)y, (uint16)z, 5, y, BlockTemplate::batt_obstruction | BlockTemplate::batt_solid | BlockTemplate::batt_liquid);
	if (dist >= 0)
	{
		y = y - dist;
		int block_id = BlockWorldClient::GetInstance()->GetBlockTemplateIdByIdx((uint16)x, (uint16)y, (uint16)z);
		outY = y;
		return block_id;
	}
	return 0;
}


void ParaEngine::CWeatherEffect::Animate(double dTimeDelta, int nRenderNumber /*= 0*/)
{
	if (m_updateTick.IsTick((float)dTimeDelta) && HasActiveSpawners())
	{
		SpawnAllParticles();

		for (int i = 0; i < (int)m_particles.size();)
		{
			WeatherParticle* particle = m_particles[i];
			particle->FrameMove(m_updateTick.GetInverval());
			if (particle->IsDead())
			{
				particle->RecollectMe();
				m_particles.erase(i);
			}
			else
				i++;
		}
	}
}

HRESULT ParaEngine::CWeatherEffect::Draw(SceneState * pSceneState)
{
	if (HasActiveSpawners())
	{
		Vector3 vOffset = -pSceneState->GetScene()->GetRenderOrigin();
		CCameraFrustum* pFrustum = pSceneState->GetScene()->GetCurrentCamera()->GetObjectFrustum();
		for (WeatherParticle* particle : m_particles)
		{
			if (!particle->IsDead())
			{
				particle->Draw(pSceneState, pFrustum, vOffset);
			}
		}
	}
	return S_OK;
}

void ParaEngine::CWeatherEffect::SpawnAllParticles()
{
	for (auto spawner_info : m_particle_spawners)
	{
		if (spawner_info->IsEnabled() && spawner_info->GetStrength() > 0)
		{
			SpawnParticles(*spawner_info, spawner_info->GetStrength(), 20.f);
		}
	}
}

void ParaEngine::CWeatherEffect::SpawnParticles(WeatherParticleSpawner& weather_type, float strength, float spawn_tick_fps)
{
	FastRandom& r = GetSpawnRand();

	float spawnRadius = weather_type.GetSpawnRadius();
	float soundOffsetX = 0;
	float soundOffsetY = 0;
	float soundOffsetZ = 0;
	float count = 0;
	int HowManyDropsPerTick = (int)Math::Ceil(Math::Min(300.f, 3.f * strength * strength));
	
	Vector3 eye_pos = CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();
	
	float camera_x = eye_pos.x;
	float camera_y = eye_pos.y;
	float camera_z = eye_pos.z;		

	for (int rainTimes = 0; rainTimes < HowManyDropsPerTick; rainTimes ++)
	{
		float particle_pos_x = camera_x + (r.random() - r.random())*spawnRadius;
		float particle_pos_y = camera_y + (r.random() - r.random())*spawnRadius;
		float particle_pos_z = camera_z + (r.random() - r.random())*spawnRadius;

		uint16 particle_pos_bx, particle_pos_by, particle_pos_bz;
		BlockCommon::ConvertToBlockIndex(particle_pos_x, particle_pos_y, particle_pos_z, particle_pos_bx, particle_pos_by, particle_pos_bz);

		int32 terrainHeight = 0;
		int terrainBlockID = GetTerrainHighestBlock(particle_pos_bx, particle_pos_bz, terrainHeight);
		if (terrainBlockID > 0 && terrainHeight >= particle_pos_by){
			// TODO: add effect which the rain hit the ground block
		}
		else
		{
			// OUTPUT_LOG("seed %d: %d %d %d\n", r.m_seed, particle_pos_bx, particle_pos_by, particle_pos_bz);
			WeatherParticle* entity = AddParticle(particle_pos_x, particle_pos_y, particle_pos_z, weather_type.GetWeatherType());
			if (entity)
			{
				float surface_height = BlockCommon::ConvertToRealY((uint16)terrainHeight) + (float)BlockConfig::g_dBlockSize;
				entity->SetMinY(surface_height);
			}
		}
	}
}

WeatherParticle* ParaEngine::CWeatherEffect::AddParticle(float x, float y, float z, WeatherType weather_type)
{
	WeatherParticle* entity = GetParticleSpawner(weather_type).CreateEntity();
	if (entity != NULL)
	{
		entity->Init(x, y, z);
		m_particles.push_back(entity);
	}
	return entity;
}

void ParaEngine::CWeatherEffect::EnableRain(bool bEnabled)
{
	GetParticleSpawner(WeatherType_Rain).SetEnabled(bEnabled);
}

void ParaEngine::CWeatherEffect::EnableSnow(bool bEnabled)
{
	GetParticleSpawner(WeatherType_Snow).SetEnabled(bEnabled);
}

void ParaEngine::CWeatherEffect::SetRainStrength(float fStrength)
{
	GetParticleSpawner(WeatherType_Rain).SetStrength(fStrength);
}

void ParaEngine::CWeatherEffect::SetSnowStrength(float fStrength)
{
	GetParticleSpawner(WeatherType_Snow).SetStrength(fStrength);
}

int ParaEngine::CWeatherEffect::PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState)
{
	Animate(pSceneState->dTimeDelta);
	Draw(pSceneState);
	return 0;
}

int ParaEngine::CWeatherEffect::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);

	pClass->AddField("EnableSnow", FieldType_Bool, (void*)EnableSnow_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("EnableRain", FieldType_Bool, (void*)EnableRain_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("RainStrength", FieldType_Float, (void*)SetRainStrength_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("SnowStrength", FieldType_Float, (void*)SetSnowStrength_s, NULL, NULL, NULL, bOverride);
	return S_OK;
}


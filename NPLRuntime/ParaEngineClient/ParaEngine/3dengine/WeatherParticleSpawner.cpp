//-----------------------------------------------------------------------------
// Class: weather particle spawner
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.12.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseCamera.h"
#include "SceneState.h"
#include "ParaWorldAsset.h"
#include "WeatherEffect.h"
#include "WeatherParticleSpawner.h"

using namespace ParaEngine;

ParaEngine::WeatherParticleSpawner::WeatherParticleSpawner(WeatherType nType)
	: m_type(nType), m_isEnabled(false), m_fStrength(30.f), m_fSpawnRadius(20.f),
	m_totalCount(0), m_maxPoolSize(5000), m_maxPoolIndex(0), m_nextPoolIndex(0)
{
}

ParaEngine::WeatherParticleSpawner::~WeatherParticleSpawner()
{
	Cleanup();
}

void ParaEngine::WeatherParticleSpawner::Cleanup()
{
	Clear();
}

WeatherParticle* ParaEngine::WeatherParticleSpawner::CreateEntity()
{
	if (IsSpawnPoolFull())
		return NULL;

	WeatherParticle* entity = NULL;
	if (m_unusedEntities.empty()) 
	{
		if (m_type == WeatherType_Rain)
		{
			entity = new RainParticle(this);
		}
		else if(m_type == WeatherType_Snow)
		{
			entity = new SnowParticle(this);
		}
		else if (m_type == WeatherType_RainSplash)
		{
			entity = new RainSplashParticle(this);
		}
		else
			return NULL;

		entity->addref();
		m_totalCount ++;
	}
	else
	{
		entity = m_unusedEntities.back();
		m_unusedEntities.pop_back();
	}
	return entity;
}

bool ParaEngine::WeatherParticleSpawner::RecollectEntity(WeatherParticle* entity)
{
	if (entity)
	{
		if ((int)m_unusedEntities.size() < m_maxPoolSize)
		{
			entity->Reset();
			m_unusedEntities.push_back(entity);
			return true;
		}
		else
		{
			OUTPUT_LOG("WeatherParticleSpawner exceeded pool size %d.\n", m_maxPoolSize);
			m_totalCount--;
			entity->Release();
		}
		return true;
	}
	return false;
}

int ParaEngine::WeatherParticleSpawner::GetReusableCount()
{
	return (int)m_unusedEntities.size();
}

int ParaEngine::WeatherParticleSpawner::GetTotalCount()
{
	return m_totalCount;
}

void ParaEngine::WeatherParticleSpawner::Clear()
{
	for (WeatherParticle* entity : m_unusedEntities)
	{
		entity->Release();
	}
	m_unusedEntities.clear();
	m_totalCount = 0;
}

void ParaEngine::WeatherParticleSpawner::SetTextureRowsCols(int nRows, int nCols, int nFromIndex, int nToIndex)
{
	m_texRows = nRows;
	m_texCols = nCols;
	int nCount = m_texRows*m_texCols;

	m_nFromTileIndex = nFromIndex;
	m_nToTileIndex = nToIndex < 0 ? (nCount - 1) : nToIndex;
	
	m_tiles.resize(nCount);

	// the triangle winding order of the quad: whether it is CCW(-1) or CW(0). It does not matter if particle is transparent.
	int order = -1;

	for (int k = 0; k<nCount; ++k)
	{
		ParticleRectUV& tc = m_tiles[k];

		Vector2 otc[4];
		Vector2 a, b;
		int x = k % m_texCols;
		int y = k / m_texCols;
		a.x = x * (1.0f / m_texCols);
		a.y = y * (1.0f / m_texRows);
		b.x = (x + 1) * (1.0f / m_texCols);
		b.y = (y + 1) * (1.0f / m_texRows);

		otc[0] = a;
		otc[2] = b;
		otc[1].x = b.x;
		otc[1].y = a.y;
		otc[3].x = a.x;
		otc[3].y = b.y;


		for (int i = 0; i<4; i++) {
			tc.tc[(i + 4 - order) & 3] = otc[i];
		}
	}
}

ParticleRectUV& ParaEngine::WeatherParticleSpawner::GetTileUV(int nTileIndex)
{
	return m_tiles[nTileIndex];
}

int ParaEngine::WeatherParticleSpawner::GetRandomTileIndex()
{
	return (m_nFromTileIndex != m_nToTileIndex) ? GetSpeedRand().random(m_nFromTileIndex, m_nToTileIndex) : m_nFromTileIndex;
}

void ParaEngine::WeatherParticleSpawner::SetTextureFilename(const std::string& sFilename)
{
	SetTexture(CGlobals::GetAssetManager()->LoadTexture("", sFilename, TextureEntity::StaticTexture));
}

ParaEngine::Vector2 ParaEngine::WeatherParticleSpawner::GetTextureRowsCols()
{
	return Vector2((float)m_texRows, (float)m_texCols);
}

void ParaEngine::WeatherParticleSpawner::SetEnabled(bool val)
{
	m_isEnabled = val;
}

bool ParaEngine::WeatherParticleSpawner::IsEnabled() const
{
	return m_isEnabled;
}

bool ParaEngine::WeatherParticleSpawner::IsSpawnPoolFull()
{
	return (GetTotalCount() - (int)m_unusedEntities.size()) >= m_maxPoolSize;
}

int ParaEngine::WeatherParticleSpawner::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);

	pClass->AddField("Enable", FieldType_Bool, (void*)SetEnable_s, (void*)IsEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("Strength", FieldType_Float, (void*)SetStrength_s, (void*)GetStrength_s, NULL, NULL, bOverride);
	pClass->AddField("SpawnRadius", FieldType_Float, (void*)SetSpawnRadius_s, (void*)GetSpawnRadius_s, NULL, NULL, bOverride);
	pClass->AddField("TextureRowsCols", FieldType_Vector2, (void*)SetTextureRowsCols_s, (void*)GetTextureRowsCols_s, NULL, NULL, bOverride);
	pClass->AddField("Texture", FieldType_String, (void*)SetTexture_s, NULL, NULL, NULL, bOverride);
	return S_OK;
}


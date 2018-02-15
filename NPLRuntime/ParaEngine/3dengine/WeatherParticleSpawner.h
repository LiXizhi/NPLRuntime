#pragma once
#include "TextureEntity.h"
#include "util/FastRandom.h"
#include "BaseObject.h"

#include <vector>
#include "WeatherParticle.h"

namespace ParaEngine
{
	struct SceneState;
	class WeatherParticleSpawner;
	class CCameraFrustum;
	class CWeatherEffect;

	class WeatherParticleSpawner : public CBaseObject
	{
	public:
		WeatherParticleSpawner(WeatherType nType);
		virtual ~WeatherParticleSpawner();

		ATTRIBUTE_DEFINE_CLASS(WeatherParticleSpawner);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(WeatherParticleSpawner, SetEnable_s, bool)	{ cls->SetEnabled(p1); return S_OK; }
		ATTRIBUTE_METHOD1(WeatherParticleSpawner, IsEnabled_s, bool*)	{ *p1 = cls->IsEnabled(); return S_OK; }

		ATTRIBUTE_METHOD1(WeatherParticleSpawner, SetStrength_s, float)	{ cls->SetStrength(p1); return S_OK; }
		ATTRIBUTE_METHOD1(WeatherParticleSpawner, GetStrength_s, float*)	{ *p1 = cls->GetStrength(); return S_OK; }

		ATTRIBUTE_METHOD1(WeatherParticleSpawner, SetSpawnRadius_s, float)	{ cls->SetSpawnRadius(p1); return S_OK; }
		ATTRIBUTE_METHOD1(WeatherParticleSpawner, GetSpawnRadius_s, float*)	{ *p1 = cls->GetSpawnRadius(); return S_OK; }

		ATTRIBUTE_METHOD1(WeatherParticleSpawner, SetTexture_s, const char*)	{ cls->SetTextureFilename(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(WeatherParticleSpawner, SetTextureRowsCols_s, Vector2)	{ cls->SetTextureRowsCols((int)p1.x, (int)p1.y); return S_OK; }
		ATTRIBUTE_METHOD1(WeatherParticleSpawner, GetTextureRowsCols_s, Vector2*)	{ *p1 = cls->GetTextureRowsCols(); return S_OK; }
		
	public:

		bool IsEnabled() const;
		void SetEnabled(bool val);
		
		float GetStrength() const { return m_fStrength; }
		void SetStrength(float val) { m_fStrength = val; }
		
		float GetSpawnRadius() const { return m_fSpawnRadius; }
		void SetSpawnRadius(float val) { m_fSpawnRadius = val; }
		
		TextureEntity* GetTexture() const { return m_pTexture.get(); }
		void SetTexture(TextureEntity* val) { m_pTexture = val; }

		ParaEngine::WeatherType GetWeatherType() const { return m_type; }
		FastRandom& GetSpeedRand() { return m_randomSpeed; }

		CWeatherEffect* GetWeatherEffect() { return m_pWeatherEffect; }
		void SetWeatherEffect(CWeatherEffect* val) { m_pWeatherEffect = val; }
		/**
		* set how many rows and cols that the texture associated with this particle system is evenly divided into.
		* @param nToIndex : if -1, it defaults to (nRows*nCols -1)
		*/
		void SetTextureRowsCols(int nRows, int nCols, int nFromIndex = 0, int nToIndex = -1);
		Vector2 GetTextureRowsCols();
		void SetTextureFilename(const std::string& sFilename);

		void Cleanup();

		/** return true if we have reached the max number of particles allowed at a time. */
		bool IsSpawnPoolFull();
	public:
		WeatherType m_type;
		bool m_isEnabled;
		float m_fStrength;
		float m_fSpawnRadius;
		FastRandom m_randomSpeed;
	public:
		/** Creates a new entity, or reuses one that's no longer in use. 
		@return may return nil, if max pool pooled entity is reached. */ 
		WeatherParticle* CreateEntity();

		/** this function should and can only be called after the entity is detached or destroyed. */
		bool RecollectEntity(WeatherParticle* entity);

		int GetReusableCount();
			
		int GetTotalCount();

		void Clear();

		ParticleRectUV& GetTileUV(int nTileIndex);
		int GetRandomTileIndex();
	protected:

		// total number of pooled object created. 
		int m_totalCount;
		int m_maxPoolSize;
		// Next index to use when adding a Pool Entry.
		int m_nextPoolIndex;
		// Largest index reached by this Pool since last CleanPool operation. 
		int m_maxPoolIndex;
		// List of entity stored in this Pool
		std::vector <WeatherParticle*> m_unusedEntities;
		asset_ptr<TextureEntity> m_pTexture;
		/** 
		* a new particle will use a random image at one of the rows*cols regions in the texture accociated with the particle system.
		* if rows=1, cols=1, the entire texture is used.
		*/
		int m_texCols;
		int m_texRows;

		int m_nFromTileIndex;
		int m_nToTileIndex;
		// this is automatically generated from rows, cols
		std::vector<ParticleRectUV> m_tiles;
		CWeatherEffect* m_pWeatherEffect;
	};
}
#pragma once
#include "BaseObject.h"
#include "util/FastRandom.h"
#include "util/Tick.h"
#include "util/unordered_array.hpp"
#include "WeatherParticle.h"

namespace ParaEngine
{
	class WeatherParticleSpawner;

	/** either rain or snow. the sky object can have multiple weather effect object attached. 
	each weather effect will automatically spawn particles of its own kind. */
	class CWeatherEffect : public CBaseObject
	{
	public:
		CWeatherEffect();
		virtual ~CWeatherEffect();
		
		ATTRIBUTE_DEFINE_CLASS(CWeatherEffect);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CWeatherEffect);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CWeatherEffect, EnableRain_s, bool)	{ cls->EnableRain(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CWeatherEffect, EnableSnow_s, bool)	{ cls->EnableSnow(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CWeatherEffect, SetRainStrength_s, float)	{ cls->SetRainStrength(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CWeatherEffect, SetSnowStrength_s, float)	{ cls->SetSnowStrength(p1); return S_OK; }

	public:
		WeatherParticle* AddParticle(float x, float y, float z, WeatherType weather_type);

		void SpawnAllParticles();

		/*spawn particles for the given weather item
		 @param weather_item : can be snow or rain item info.
		 */
		void SpawnParticles(WeatherParticleSpawner& weather_type, float strength, float spawn_tick_fps);

		virtual void Cleanup();

		FastRandom& GetSpawnRand() { return m_spawnRand; }
		FastRandom& GetSpeedRand() { return m_speedRand; }

		/** if any spawner is enabled. */
		bool HasActiveSpawners();

		WeatherParticleSpawner& GetParticleSpawner(WeatherType weather_type);

		void EnableWeatherItem(WeatherType weather_type, bool bEnabled);
		bool IsWeatherItemEnabled(WeatherType weather_type);
		
		/** get the highest solid block information in coordinate x, z
		* @param outY: output y
		* @return block_id;
		*/
		int32 GetTerrainHighestBlock(int32 x, int32 z, int32& outY);

		int PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState);

		/** animate the model by a given delta time.
		* @param dTimeDelta: delta time in seconds
		* @param nRenderNumber: if 0 it means that it is always animated, otherwise we should only animate
		* if previous call of this function has a different render frame number than this one.
		*/
		virtual void Animate(double dTimeDelta, int nRenderNumber = 0);

		/// only for drawable objects
		virtual HRESULT Draw(SceneState * sceneState);

		void EnableSnow(bool bEnabled);
		void EnableRain(bool bEnabled);
		/**
		* @param fStrength: [0,1]
		*/
		void SetRainStrength(float fStrength);
		void SetSnowStrength(float fStrength);

	protected:
		// keep a weak reference for fast access
		WeatherParticleSpawner* m_particle_spawners[WeatherType_Last];
	
		float m_default_strength;
		FastRandom m_spawnRand;
		FastRandom m_speedRand;
		
		unordered_array<WeatherParticle*> m_particles;
		Tick m_updateTick;
	};
}

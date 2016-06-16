#pragma once
#include "IBatchedElementDraw.h"

namespace ParaEngine
{
	class WeatherParticleSpawner;
	struct SceneState;
	class CCameraFrustum;

	enum WeatherType {
		WeatherType_Rain = 0,
		WeatherType_Snow,
		WeatherType_RainSplash,
		WeatherType_Last,
	};

	
	/**
	* base class for weather particle
	*/
	class WeatherParticle : public CParticleElement
	{
	public:
		WeatherParticle(WeatherParticleSpawner* pParent);
		virtual ~WeatherParticle();

		virtual WeatherType GetType() = 0;
		virtual void Init(float x, float y, float z);
		virtual bool IsPointTexture() { return true; }
		virtual void Reset();

		virtual TextureEntity* GetTexture() { return m_pTexture; }
		virtual void SetTexture(TextureEntity* val) { m_pTexture = val; }

		/** generate particle sprite vertex into the vertex buffer based on current camera setting (bill boarding).
		* @return the number of particles generated (1 particle has 2 triangles with 6 vertices).
		*/
		virtual int RenderParticle(SPRITEVERTEX** pVertexBuffer, SceneState* pSceneState);

		virtual void FrameMove(float deltaTime);
		/// only for drawable objects
		virtual void Draw(SceneState * sceneState, CCameraFrustum* pFrustum, const Vector3& vRenderOffset);

		virtual void SetFallOnGround();
		virtual void SetDead();
		bool IsDead() const { return m_isDead; }

		/** recollect to spawner */
		virtual void RecollectMe();

		float GetMinY() const { return minY; }
		void SetMinY(float val) { minY = val; }

	protected:
		WeatherParticleSpawner* GetWeatherSpawner();
		ParaEngine::Vector3 GetRenderOffset() const { return m_vRenderOffset; }
		void SetRenderOffset(ParaEngine::Vector3 val) { m_vRenderOffset = val; }

	protected:
		float x;
		float y;
		float z;
		float speed_x;
		float speed_y;
		float speed_z;
		float width;
		float height;
		float lifetime;
		float minY;
		int tileIndex;

		bool m_isDead;
		Vector3 m_vRenderOffset;
		float gravity;
		WeatherParticleSpawner* m_pWeatherSpawner;
		TextureEntity* m_pTexture;
	};

	class RainParticle : public WeatherParticle
	{
	public:
		RainParticle(WeatherParticleSpawner* pParent) : WeatherParticle(pParent) {};
		virtual ~RainParticle(){};
		virtual WeatherType GetType() { return WeatherType_Rain; };
		virtual void Init(float x, float y, float z);
		virtual void SetDead();
	};

	class RainSplashParticle : public WeatherParticle
	{
	public:
		RainSplashParticle(WeatherParticleSpawner* pParent) : WeatherParticle(pParent) {};
		virtual ~RainSplashParticle(){};
		virtual WeatherType GetType() { return WeatherType_RainSplash; };
		virtual void Init(float x, float y, float z);
		virtual void FrameMove(float deltaTime);
	};

	class SnowParticle : public WeatherParticle
	{
	public:
		SnowParticle(WeatherParticleSpawner* pParent) : WeatherParticle(pParent) {};
		virtual ~SnowParticle(){};
		virtual WeatherType GetType() { return WeatherType_Snow; };

		virtual void Init(float x, float y, float z);
		virtual void FrameMove(float deltaTime);

		bool isFloating;
		float float_speed;
	};
}
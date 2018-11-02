#pragma once

namespace ParaEngine
{
	class CSunLight;
	class CBaseCamera;
	struct SceneState;

	/**
	* Basic scene interface: implemented by the CSceneObject and CMiniScenegraph class
	*/
	class IScene
	{
	public:
		
		/** whether rendering the sky */
		virtual bool IsRenderSky() = 0;
		virtual void EnableSkyRendering(bool bEnable) = 0;

		/** set the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		virtual void SetBackGroundColor(const LinearColor& bgColor) = 0;
		/** Get the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		virtual LinearColor GetBackGroundColor() = 0;

		/** fog parameters */
		virtual void SetFogColorFactor(const LinearColor& fogColorFactor) = 0;
		virtual const LinearColor& GetFogColorFactor() = 0;
		virtual void SetFogColor(const LinearColor& fogColor) = 0;
		virtual LinearColor GetFogColor() = 0;
		virtual void SetFogStart(float fFogStart) = 0;
		virtual float GetFogStart() = 0;
		virtual void SetFogEnd(float fFogEnd) = 0;
		virtual float GetFogEnd() = 0;
		virtual void SetFogDensity(float fFogDensity) = 0;
		virtual float GetFogDensity() = 0;
		virtual void EnableFog(bool bEnableFog) = 0;
		virtual bool IsFogEnabled() = 0;

		/** get light enable state */
		virtual bool IsLightEnabled() = 0;
		/** whether use light */
		virtual void EnableLight(bool bEnable) = 0;

		/** get sun light object*/
		virtual CSunLight& GetSunLight() = 0;

		virtual CBaseCamera*	GetCurrentCamera() = 0;

		virtual Vector3 GetRenderOrigin() = 0;

		/** get the scene state*/
		virtual SceneState* GetSceneState()=0;

		/** get light enable state */
		virtual bool IsSunLightEnabled()=0;

		/** whether use light */
		virtual void EnableSunLight(bool bEnable) = 0;

		/** whether render shadow */
		virtual void SetShadow(bool bRenderShadow) {};

		/** whether render shadow */
		virtual float GetShadowRadius() {return 0.f;};

		/** whether render using shadow map */
		virtual bool IsShadowMapEnabled(){return false;};

		/** automatically prepare the node for rendering by placing it to the current render pipeline. 
		* Most IScene implementation does following: 
		*   decide whether pObj is visible by the pCamera, if so, it will add it to the proper render queue in sceneState.
		*/
		virtual bool PrepareRenderObject(CBaseObject* pObj, CBaseCamera* pCamera, SceneState& sceneState) { return true; };

		/** get the global object by its name. If there have been several objects with the same name,
		* the most recently attached object is returned.
		* @param sName: exact name of the object
		*/
		virtual CBaseObject* GetGlobalObject(const std::string& sName) { return NULL; };
	};

}

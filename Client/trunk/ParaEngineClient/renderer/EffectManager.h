#pragma once

#include "AssetManager.h"
#include "effect_file.h"
#include "LightManager.h"

namespace ParaEngine
{
	class CBaseObject;
	class CShadowMap;
	class CGlowEffect;
	class IScene;
	class WaveEffect;
	
	/** manager all effects file used by the game engine.*/
	class EffectManager: public AssetManager<CEffectFile>
	{
	public:
		EffectManager();
		~EffectManager();

		/** Return the effect object by its handle. The game engine may dynamically change the mapping 
		* from effect handle to effect objects.
		@param nHandle: the effect handle.
		@return: the pointer to the effect object is returned. This may be NULL, if the handle is not 
		bound to any object. Also the effect returned may not be supported by the hardware. Better to check before use.
		*/
		CEffectFile* GetEffectByHandle(int nHandle);

		/** Get the integer handle to this asset. if there are multiple handles, the first (smallest) handle is returned. 
		* if handle is not available. it will return -1 (INVALID handle). */
		int GetEffectHandle(CEffectFile* effectFile);

		/**
		* the returned effect may not be valid. 
		* @param sName : any of the following known name:
		*	- simple_mesh_normal_low
		*	- simple_mesh_normal
		*	- simple_mesh_normal_instanced
		*	- simple_mesh
		*	- simple_mesh_normal_ctor
		*	- simple_mesh_normal_unlit
		*	- simple_mesh_normal_selected
		*	- simple_mesh_normal_border
		*	- simple_mesh_normal_shadow
		*	- simple_mesh_normal_tex2
		*	- occlusion_test
		*	- ocean_fft
		*	- ocean_fft_high
		*	- waterripple
		*	- sky
		*	- fullscreen_glow
		*	- terrain_normal
		* @return the returned effect may not be valid or initialized. Call loadasset and check validity.
		*/
		CEffectFile* GetEffectByName(const string& sName);

		/** Map a nHandle to effect object. 
		@param nHandle: the effect handle.
		@param pNewEffect: the pointer to the effect object is returned. This may be NULL, in which case the mapping is erased.
		@return: the old effect object, if any, is returned, otherwise NULL is returned. 
		*/
		CEffectFile* MapHandleToEffect(int nHandle, CEffectFile* pNewEffect);

		virtual bool DeleteEntity(AssetEntity* entity);
		virtual void InitDeviceObjects();
		virtual void DeleteDeviceObjects();
		virtual void RestoreDeviceObjects();
		virtual void InvalidateDeviceObjects();
		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		virtual void RendererRecreated();

		void Cleanup();

		/** get the shadow map object. */
		CShadowMap* GetShadowMap();
		/** get the glow effect object. */
		CGlowEffect* GetGlowEffect();

		/** Start effect by handle. It set the proper technique of the effect.
		the vertex declaration will also be set or created. 
		@param nHandle: the effect handle.
		@param pOutEffect: pointer to retrieve the effect object which is currently selected.
		@return: return true if succeeded.
		*/
		bool BeginEffect(int nHandle, CEffectFile** pOutEffect = NULL);
		/** end the effect,this function actually does nothing. */
		void EndEffect();
		/** get a valid effect. but it does not begin it. this function can be used to check if an effect file is supported on the current system */
		CEffectFile* CheckLoadEffect(int nHandle);

		/** predefined vertex declaration. */
		enum VERTEX_DECLARATION
		{
			S0_POS_TEX0, // all data in stream 0: position and tex0
			S0_POS_NORM_TEX0, // all data in stream 0: position, normal and tex0
			S0_POS_NORM_TEX0_INSTANCED, // all data in stream 0: position, normal and tex0, stream1:instanced data
			S0_POS_TEX0_COLOR, // all data in stream 0: position, tex0 and color
			S0_POS_NORM_TEX0_COLOR, // all data in stream 0: position, normal, tex0 and color
			S0_S1_S2_OCEAN_FFT, // for FFT ocean
			S0_S1_S2_S3_OCEAN_FFT, // for FFT ocean with terrain height field
			S0_POS_NORM_TEX0_TEX1, // all data in stream 0: position, normal tex0 and tex1
			S0_POS_TEX0_TEX1, // all data in stream 0: position, tex0 and tex1
			S0_S1_OCEAN_CLOUD, // stream 0: position; stream 1, height field
			S0_POS_COLOR, // all data in stream 0: position, and color
			S0_POS_NORM_COLOR, // all data in stream 0: position, normal, and color
			S0_POS_NORM_TEX0_COLOR0_COLOR1, // all data in stream 0: position, normal, tex0,color0 and color1
			S0_POS, // all data in stream 0: position
			MAX_DECLARATIONS_NUM,
		};


		/** Get declaration by id
		@param nIndex: value is in @see VERTEX_DECLARATION 
		@return: may return NULL.*/
		VertexDeclarationPtr GetVertexDeclaration(int nIndex);
		/** Set declaration by id
		@param nIndex: value is in @see VERTEX_DECLARATION 
		@return: return S_OK if successful. .*/
		HRESULT SetVertexDeclaration(int nIndex);

		/** load the default handle to effect file mapping.
		@param nLevel: the higher this value, the more sophisticated shader will be used. 
		The default value is 0, which is the fixed programming pipeline without shaders.
		10, VS PS shader version 1
		20, VS PS shader version 2
		30, VS PS shader version 3
		please note that any user defined mapping will be cleared. 
		*/
		void SetDefaultEffectMapping(int nLevel);
		int GetDefaultEffectMapping();

		enum EffectTechniques
		{
			/// normal rendering
			EFFECT_DEFAULT = 0, 
			/// shadow map generation technique
			EFFECT_GEN_SHADOWMAP,
			/// rendering the model with shadow map
			EFFECT_RENDER_WITH_SHADOWMAP,
			/// fixed function pipeline
			EFFECT_FIXED_FUNCTION,
		};
		/** set all effect files to a specified technique.If the effect does not have the specified technique
		nothing will be changed. 
		@param nTech: the technique handle. */
		void SetAllEffectsTechnique(EffectTechniques nTech);
		/** current technique in the effect file*/
		EffectTechniques GetCurrentEffectTechniqueType();


		int GetCurrentTechHandle();
		CEffectFile* GetCurrentEffectFile();

		//////////////////////////////////////////////////////////////////////////
		//
		// The following functions set or retrieve global effect states which are shared by all effect files.
		// They are designed to look like the fixed pipeline programming interface of DirectX9
		//
		//////////////////////////////////////////////////////////////////////////
		
		HRESULT SetMaterial(ParaMaterial *pMaterial);
		HRESULT SetLight(DWORD Index, const Para3DLight *pLight);
		HRESULT LightEnable(DWORD Index, BOOL Enable);
		HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
		HRESULT SetTexture(DWORD Stage, DeviceTexturePtr_type pTexture);
		/** internally it will cache texture stage to prevent direct d3d calls */
		HRESULT SetSamplerState(int nStage, DWORD dwType, DWORD dwValue, bool bForceSet = false);
		/** internally it will cache texture stage to prevent direct d3d calls */
		HRESULT GetSamplerState(int nStage, DWORD dwType, DWORD* pValue, bool bForceGet = false);
		/** default sampler state. in most cases, this will define whether the point filtering or linear filtering is used by default for all meshes and characters. */
		void SetDefaultSamplerState(int nStage, DWORD dwType, DWORD dwValue);
		/** default sampler state. in most cases, this will define whether the point filtering or linear filtering is used by default for all meshes and characters. */
		DWORD GetDefaultSamplerState(int nStage, DWORD dwType);

		HRESULT SetTransform(D3DTRANSFORMSTATETYPE State, const Matrix4 *pMatrix);
		HRESULT GetTransform(D3DTRANSFORMSTATETYPE State,Matrix4 * pMatrix);

		/** Get the current world transformation matrix which is used by the effect.
		@see GetTransform()*/
		const Matrix4& GetWorldTransform();
		const Matrix4& GetViewTransform();
		const Matrix4& GetProjTransform();

		/** update the transformation for both the fixed and programmable pipeline. 
		@param pWorld: world transformation, only set if it is true, 
		@param pView: camera view transformation, only set if it is true, 
		@param pProjection: camera projection transformation, only set if it is true, */
		void UpdateD3DPipelineTransform(bool pWorld, bool pView,bool pProjection);

		/** enable or disable fog.*/
		void EnableFog(bool bEnabled);

		/** clip plane state */
		enum ClipPlaneState{
			ClipPlane_Disabled,
				ClipPlane_Enabled_WorldSpace,
				ClipPlane_Enabled_ClipSpace,
		};

		/** enable or disable user clip planes.
		* the effect manager keeps two copies of the clip planes, one in world space; the other is in clip space. 
		* this function will disable clip plane immediately on the GPU; however, it will not enable clip plane immediately,
		* instead it enable the current version of the clip planes in the BeginEffect() function. 
		* @param bEnable: Enable or disable clip plane states.
		*/
		void EnableClipPlane(bool bEnable);

		/** immediately change the device's clip plane settings if and only if it is different from the current setting. This function is used mostly internally in BeginEffect() */
		void EnableClipPlaneImmediate(EffectManager::ClipPlaneState newState, int nIndex=0);
		/** whether the clip plane is enabled */
		bool IsClipPlaneEnabled();
		
		/** Sets the coefficients of a user-defined clipping plane for the device.
		when rendering using fixed pipeline, clip plane is in world space; when rendering with shaders, clip plane is in clip space. 
		this 
		@param Index:[in] Index of the clipping plane for which the plane equation coefficients are to be set. 
		@param pPlane:[in] Pointer to an address of a four-element array of values that represent the clipping plane coefficients to be set, in the form of the general plane equation. 
		@param bClipSpace: true if the clip plane is in clip space, otherwise it is in world space. the effect will thus keep two copies of the clip planes for each coordinate system for later use.
		*/
		void SetClipPlane(DWORD Index,	const float * pPlane, bool bClipSpace);

		/**
		* both Fixed function (FF) pipeline and the programmable pipeline can call this function. 
		* in most cases, this function is called by FF. The PF contains a same function in the effect file class.
		* @param bUseGlobalAmbient: if true and that the ambient in pSurfaceMaterial is 0, the ambient in the material is ignored. and the scene's ambient color will be used.
		*/
		void applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient = true);

		/**
		* both Fixed function (FF) pipeline and the programmable pipeline can call this function. 
		*/
		void applyLocalLightingData(const LightList* lights, int nLightNum);
		/**
		* both Fixed function (FF) pipeline and the programmable pipeline can call this function. 
		* using the last GetBestLights() results from the light manager.
		*/
		void applyLocalLightingData();
		
		/** automatically apply fog. this function only works on programmable pipeline. */
		void applyFogParameters();

		/** automatically apply local lighting to a scene object according to the bounding volume of the object. */
		void applyObjectLocalLighting(CBaseObject* pObj);

		/** enable global sun lighting.*/
		void EnableGlobalLighting(bool bEnable);
		/** whether global lighting is enabled.*/
		bool IsGlobalLightingEnabled();

		/** enable lighting.*/
		void EnableLocalLighting(bool bEnable);
		/** whether lighting is enabled.*/
		bool IsLocalLightingEnabled();

		/** enable shadow map */
		void  EnableUsingShadowMap(bool bEnable);
		/** whether shadow map is enabled*/
		bool IsUsingShadowMap();
		/** get the shadow map's tex view projection matrix */
		const Matrix4* GetTexViewProjMatrix();

		/** one occasion to disable textures is during the shadow pass. When textures are disabled all setTexture call will take no effect. */
		void EnableTextures(bool bEnable);
		bool AreTextureEnabled();

		/** 
		* @param bDisable: if true, setting the alpha testing parameter of the effect files will not affect the d3d render state, only shader parameters are set.
		*/
		void DisableD3DAlphaTesting(bool bDisable);

		/** return true, if the alpha testing have been disabled.*/
		bool IsD3DAlphaTestingDisabled();

		/** 
		* @param bDisable: if true, setting the culling mode of the effect files will not affect the d3d render state, only shader parameters are set.
		*/
		void DisableD3DCulling(bool bDisable);
		/** return true, if the alpha testing have been disabled.*/
		bool IsD3DCullingDisabled();

		/** disable z write. Any subsequent calls will be disabled. */
		void DisableZWrite(bool bDisable);
		/** return true, if the alpha testing have been disabled.*/
		bool IsZWriteDisabled();

		/** this function only takes effects when IsZWriteDisabled() is false. */
		void EnableZWrite(bool bZWriteEnabled);

		/** enable the D3D device's blending state*/
		void EnableD3DAlphaBlending(bool bEnable);

		/** set the culling mode of the current effect file or the fixed function pipeline. 
		*@param dwCullMode: D3DCULL_NONE, D3DCULL_CW, D3DCULL_CCW
		*/
		void SetCullingMode(DWORD dwCullMode);
		/** 
		@param bEnable: true to enable the scene's current culling mode, false to disable it.*/
		void SetCullingMode(bool bEnable);

		/** set d3d fog for the fixed function pipeline. */
		bool SetD3DFogState();

		/** whether full screen glow effect is used. */
		bool IsUsingFullScreenGlow();

		/** whether full screen glow effect is used. */
		void EnableFullScreenGlow( bool bEnable);

		/** disable reflection texture, this is used when rendering in to reflection texture */
		inline bool IsReflectionRenderingEnabled(){return m_bEnableReflectionRendering;};
		/** disable reflection texture, this is used when rendering in to reflection texture*/
		inline void EnableReflectionRendering(bool bEnabled){m_bEnableReflectionRendering = bEnabled;};

		/**
		* set the glow technique
		* @param nTech 0 for 5*5 filter, 1 for 9*9 filter
		*/
		void SetGlowTechnique(int nTech);

		/**
		* Get the glow technique
		* @return 0 for 5*5 filter, 1 for 9*9 filter
		*/
		int GetGlowTechnique();
		
		/**
		* glowness controls the intensity of the glow in the full screen glow effect. 
		* 0 means no glow, 1 is normal glow, 3 is three times glow. default value is 1.
		* @param Glowness
		*/
		void SetGlowness(const Vector4& Glowness);
		/**
		* glowness controls the intensity of the glow in the full screen glow effect. 
		* 0 means no glow, 1 is normal glow, 3 is three times glow. default value is 1.
		*/
		//change glowness type to vector4  --clayman 2011.7.19
		const Vector4& GetGlowness();

		WaveEffect* GetScreenWaveEffect();
		bool IsUsingScreenWaveEffect();
		void EnableScreenWaveEffect(bool enable);
		void SetScreenWaveTechnique(int nTech);
		int GetScreenWaveTechnique();
		
		/**
		* set maximum number of local lights per object. 
		* @param nNum must be smaller than some internal value, currently its upper limit is 8
		*/
		void SetMaxLocalLightsNum(int nNum);

		/**
		* get maximum number of local lights per object. 
		* @return 
		*/
		int GetMaxLocalLightsNum();

		/** which scene object the effect manager is current serving */
		IScene * GetScene(){return m_pScene;}

		/** which scene object the effect manager is current serving */
		void SetScene(IScene * pScene){m_pScene = pScene;}

		/** if the last call to BeginEffect() returns false, this function will be set to false. */
		bool IsCurrentEffectValid() { return m_bEffectValid; }

	private:
		// which scene object the effect manager is current serving
		IScene * m_pScene;

		/** return true if a clip plane is currently set for the device */
		ClipPlaneState GetClipPlaneState();
		/** begin effect for fixed function pipeline. Used Internally*/
		bool BeginEffectFF(int nHandle);
		bool BeginEffectShader(int nHandle, CEffectFile** pOutEffect = NULL);

		/** a mapping from handle to effect file object*/
		map<int, CEffectFile*> m_HandleMap;
		/** current effect object, this may be 0, which denotes fixed programming pipeline.*/
		int m_nCurrentEffect;
		/** current effect object, this may be NULL, which denotes fixed programming pipeline.*/
		CEffectFile* m_pCurrentEffect;

		/** if the last call to BeginEffect() returns false, this value will be set to false. */
		bool m_bEffectValid;
		/** the current transformation matrix which is used by the effect.*/
		/*Matrix4 m_mWorld;
		Matrix4 m_mView;
		Matrix4 m_mProj;*/

		/** whether use lighting: global sun light and local point lights */
		bool m_bEnableLocalLighting;
		/** whether use fog*/
		bool m_bUseFog;
		/** if true, setting the alpha testing parameter of the effect files will have no effect. */
		bool m_bDisableD3DAlphaTesting;
		/** whether zwrite should be disabled. so that any EnableZWrite call will not take effect. */
		bool m_bDisableZWrite;
		/** whether using shadow map */
		bool m_bUsingShadowMap;
		/** whether full screen glow effect is used. */
		bool m_bIsUsingFullScreenGlow;
		//-clayman 2011.7.18
		Vector4 m_colorGlowness;
		/** the glow technique to use. */
		int m_nGlowTechnique;

		int m_screenWaveTechnique;

		/** maximum number of local lights per object. Default is 4 */
		int m_nMaxLocalLightsNum;
		
		/** if true, setting the culling mode of the effect files will have no effect. */
		bool m_bDisableD3DCulling;

		/** disable reflection texture, this is used when rendering in to reflection texture*/
		bool m_bEnableReflectionRendering;

		ClipPlaneState	m_ClipPlaneState;
		bool			m_bClipPlaneEnabled;
		static const int MaxClipPlanesNum = 3;
		Plane m_ClipPlaneWorldSpace[MaxClipPlanesNum];
		Plane m_ClipPlaneClipSpace[MaxClipPlanesNum];

		WaveEffect* m_pWaveEffect;
		bool m_enableScreenWaveEffect;

		int m_nEffectLevel;

		DWORD m_lastSamplerStates[8][16];
		DWORD m_defaultSamplerStates[8][16];
		
		/** all vertex declarations*/
		VertexDeclarationPtr m_pVertexDeclarations[MAX_DECLARATIONS_NUM];
		int m_pVertexDeclarations_status[MAX_DECLARATIONS_NUM];

#ifdef USE_DIRECTX_RENDERER
		CShadowMap* m_pShadowMap;
		CGlowEffect* m_pGlowEffect;
#endif
	};
}

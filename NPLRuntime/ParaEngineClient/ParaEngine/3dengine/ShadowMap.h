#pragma once

namespace ParaEngine
{
	class CEffectFile;

	/** some functions about shadow map*/
	class CShadowMap
	{
	public:
		CShadowMap(void);
		~CShadowMap(void);

		enum SHADOW_TECHNIQUE
		{
			SHADOWTYPE_PSM=0,
			SHADOWTYPE_LSPSM=1,
			SHADOWTYPE_TSM=2,
			SHADOWTYPE_ORTHO=3,
		};
	public:
		HRESULT InvalidateDeviceObjects(); // called just before device is Reset
		HRESULT RestoreDeviceObjects();    // called when device is restored
		/** The function is responsible for building the perspective shadow map projection matrix and set the render 
		and effect states for rendering the actual shadow map 
		@return: return  S_OK if succeeded.*/
		HRESULT BeginShadowPass();
		/** end the shadow map */
		HRESULT EndShadowPass();

		/** save shadow map to file*/
		bool SaveShadowMapToFile(string filePath);

		const Matrix4* GetTexViewProjMatrix();
		const Matrix4* GetViewProjMatrix();
		
		/**
		* apply the current shadow map to the selected effect file at the specified texture index
		* @param pEffect 
		* @param nTextureIndex 
		* @param nUseBlur if 0, the depth shadow map is set, if 1, the blurred image texture is set. 
		*	blurred image is used for terrain ground or other larger shadow receivers.
		* @return 
		*/
		HRESULT SetShadowTexture(CEffectFile& pEffect, int nTextureIndex, int nUseBlur=0);
		/** unset projective texture */
		void UnsetShadowTexture(int nTextureIndex);
		/** return true if device support hardware shadow map.*/
		bool SupportsHWShadowMaps(){return m_bSupportsHWShadowMaps;}

		/** get the size of the shadow map. assuming width == height.*/
		int GetShadowMapTexelSize();


		/**
		* I will support two shadow mapping method. LiPSM and Orthogonal SM, basically: 
		* 	- Use LiPSM (with multiple bounding boxes for the terrain receiver): I think it is the best One pass general Shadow Mapping technique.
		* 	- Use Orthogonal SM (with only one bounding box for the terrain receiver): 
		* 	It is fast and looks almost the same as TSM. But only a small region of the terrain(shadow receiver) can has shadow. 
		* 	- TODO: Cascaded Shadow Maps with Orthogonal SM is the final solution for more general cases, which I have not implemented yet.
		* @return 
		*/
		SHADOW_TECHNIQUE GetShadowTechnique(){return (SHADOW_TECHNIQUE)m_iShadowType;};
		/**
		* I will support two shadow mapping method. LiPSM and Orthogonal SM, basically: 
		* 	- Use LiPSM (with multiple bounding boxes for the terrain receiver): I think it is the best One pass general Shadow Mapping technique.
		* 	- Use Orthogonal SM (with only one bounding box for the terrain receiver): 
		* 	It is fast and looks almost the same as TSM. But only a small region of the terrain(shadow receiver) can has shadow. 
		* 	- TODO: Cascaded Shadow Maps with Orthogonal SM is the final solution for more general cases, which I have not implemented yet.
		* @param technique 
			enum SHADOW_TECHNIQUE
			{
				SHADOWTYPE_PSM=0,
				SHADOWTYPE_LSPSM=1,
				SHADOWTYPE_TSM=2,
				SHADOWTYPE_ORTHO=3,
			};
		*/
		void SetShadowTechnique(SHADOW_TECHNIQUE technique){m_iShadowType = technique;};

		/** whether to use blurred shadow map for the global terrain. */
		bool UsingBlurredShadowMap(){return m_bBlurSMColorTexture;}

		/** change the size level of the shadow map.
		@param nLevel: 0 means 1024 texels; 1 means 1536 texels. default is level 0.*/
		void SetShadowMapTexelSizeLevel(int nLevel);
		/** Set the size of the shadow map.*/
		void SetShadowMapTexelSize(int nWidth, int nHeight);

		/** set the size level of the shadow map when it is created. This function is usually called before a shadow map is constructed.
		@param nLevel: 0 means 1024 texels; 1 means 1536 texels. default is level 0.*/
		static void SetDefaultShadowMapTexelSizeLevel(int nLevel){g_nShadowMapTexelSizeLevel = nLevel;};

		/** prepare all device objects. */
		bool PrepareAllSurfaces();

		/** add a shadow caster point. */
		void AddShadowCasterPoint(const CShapeAABB& aabb);
	private:
		static int g_nShadowMapTexelSizeLevel;
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DSURFACE9 m_pBackBuffer, m_pZBuffer; 
		LPDIRECT3DSURFACE9 m_pSMColorSurface, m_pSMColorSurfaceBlurredHorizontal, m_pSMColorSurfaceBlurredVertical, m_pSMZSurface;
#else
		GLuint mOldFrameBufferObject;
		GLuint mSMFrameBufferObject,mSMDepthStencilBufferObject;
#endif
		asset_ptr<TextureEntity> m_pSMColorTexture, m_pSMColorTextureBlurredHorizontal, m_pSMColorTextureBlurredVertical, m_pSMZTexture;
		int m_shadowTexWidth, m_shadowTexHeight;
		// light direction
		Vector3 m_lightDir;

		/** caster AABB in camera space */
		std::vector<CShapeAABB> m_ShadowCasterPoints;
		/** receiver AABB in camera space */
		std::vector<CShapeAABB> m_ShadowReceiverPoints;

		//bit depth of shadow map
		int m_bitDepth;

		// Transforms
		Matrix4 m_LightViewProj;

		// this value can only be set when this instance is being created
		bool m_bSupportsHWShadowMaps;
		// this value can only be set when this instance is being created
		bool m_bBlurSMColorTexture;

		int  m_iShadowType;
		bool m_bUnitCubeClip;
		bool m_bSlideBack;
		bool m_bDisplayShadowMap;
		bool m_bShadowTestInverted;
		float m_fAspect, m_zNear, m_zFar, m_ppNear, m_ppFar, m_fSlideBack, m_fLSPSM_Nopt, m_fCosGamma;

		//various bias values
		float m_fBiasSlope;
		int m_iDepthBias;
		float m_fMinInfinityZ;
		float m_fLSPSM_NoptWeight;
		float m_fTSM_Delta;

		Matrix4 m_World;
		Matrix4 m_View;
		Matrix4 m_Projection;
		Matrix4 m_textureMatrix;

		bool    ComputeVirtualCameraParameters();
		bool    BuildTSMProjectionMatrix();
		bool    BuildPSMProjectionMatrix();
		bool    BuildLSPSMProjectionMatrix();
		bool    BuildOrthoShadowProjectionMatrix();
#ifdef USE_DIRECTX_RENDERER
		HRESULT CheckResourceFormatSupport(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage);
#endif
	};

}

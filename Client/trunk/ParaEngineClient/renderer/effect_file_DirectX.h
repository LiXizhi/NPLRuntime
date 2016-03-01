#pragma once
#include "effect_file.h"

namespace ParaEngine
{
	class CEffectFileDirectX : public CEffectFileBase
	{
	public:
		CEffectFileDirectX(const char* filename);
		CEffectFileDirectX(const AssetKey& key);
		virtual  ~CEffectFileDirectX();

		// Operators...
		// Mutators...
		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		bool loadResource(); // load the resource from a file (or NULL to use the resource name)
		bool saveResource(const char* filename=0);	// save the resource to a file (or NULL to use the resource name)

		/** set inner file name, usually at the load time. */
		void SetFileName(const std::string& filename);

		/**
		@param bApplyParam: whether to apply current camera and lighting parameters to shader
		@param flag: default is not saving any state
		D3DXFX_DONOTSAVESTATE:  No state is saved when calling Begin or restored when calling End.  (1 << 0) 
		D3DXFX_DONOTSAVESAMPLERSTATE:  A stateblock saves state when calling Begin and restores state when calling End. (1 << 2) 
		D3DXFX_DONOTSAVESHADERSTATE: A stateblock saves state (except shaders and shader constants) when calling Begin and restores state when calling End. 
		@param bShare: if this false, it will only begin the effect if the effect file is not in shared mode.
		*/
		bool begin(bool bApplyParam = true, DWORD flag=D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESAMPLERSTATE|D3DXFX_DONOTSAVESHADERSTATE, bool bForceBegin = false);
		bool BeginPass(int pass,bool bForceBegin = false);
		void CommitChanges();
		void EndPass(bool bForceEnd = false);
		void end(bool bForceEnd = false);
		void EnableShareMode(bool bEnable);
		bool IsInShareMode();

		/** This is a handy function that calls EnableShareMode(true), begin() and then BeginPass(). 
		*/
		bool BeginSharePassMode(bool bApplyParam = true, DWORD flag=D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESAMPLERSTATE|D3DXFX_DONOTSAVESHADERSTATE, bool bForceBegin = true);
	
		/** This is a handy function that calls EnableShareMode(false), EndPass(true), end(true). 
		*/
		void EndSharePassMode();

		/** this is called when the shader is selected by the pipeline. There may be many objects drawn using the same shader between switch in and out. */
		void OnSwitchInShader();

		/** this is called when the shader is deselected by the pipeline. There may be many objects drawn using the same shader between switch in and out. */
		void OnSwitchOutShader();

		/** a technique description */
		struct TechniqueDescDX : public TechniqueDesc
		{
			D3DXHANDLE hTechnique;
			D3DXTECHNIQUE_DESC techniqueDesc;
		public:
			TechniqueDescDX(){
				memset(this, 0, sizeof(TechniqueDescDX));
			}
		};

		/** 
		* Set technique if it is different from the current one.
		*@param nCat: @see TechniqueCategory
		*@return: return true if the technique is found and successfully set.
		*/
		bool SetFirstValidTechniqueByCategory(TechniqueCategory nCat);
		/**
		* Set technique if it is different from the current one.
		* multiple calls to this function with the same index takes no effect.
		* @param nIndex 0 based index.
		* @return: return true if the technique is found and successfully set.
		*/
		bool SetTechniqueByIndex(int nIndex);
		/** get the current technique description. This function may return NULL*/
		const TechniqueDesc* GetCurrentTechniqueDesc();
		/**
		* current technique index. 
		* @return 
		*/
		int GetCurrentTechniqueIndex();
	
		// Accessors...
		LPD3DXEFFECT effect()const;
		int totalPasses()const;

		bool isParameterUsed(eParameterHandles index)const;
		bool isMatrixUsed(eParameterHandles index)const;
		bool isTextureUsed(int index)const;
		bool isTextureMatrixUsed(int index)const;
		bool isShadowTextureUsed()const;

		bool setParameter(eParameterHandles index, const void* data, INT32 size=D3DX_DEFAULT)const;
		bool setBool(eParameterHandles index, BOOL bBoolean) const;
		bool setInt(eParameterHandles index, int nValue) const;
		bool setFloat(eParameterHandles index, float fValue) const;
		bool setMatrix(eParameterHandles index, const Matrix4* data)const;
		bool setMatrixArray(eParameterHandles index, const Matrix4* data, UINT32 count)const;
		bool setMatrixInArray(eParameterHandles index, UINT32 element, const Matrix4* data)const;
		bool setFloatArray(eParameterHandles index, const float* data, UINT32 count)const;
		bool setVectorArray(eParameterHandles index,const Vector4* pVector,UINT count)const;
		bool setFloatInArray(eParameterHandles index, UINT32 element, float data)const;
		bool setTexture(int index, TextureEntity* data);
		bool setTexture(int index, LPDIRECT3DTEXTURE9 pTex);
		bool setTextureInternal(int index, LPDIRECT3DTEXTURE9 pTex);
		bool setTextureMatrix(int index, const Matrix4* data)const;
		//bool setShadowTexture(const TextureEntity* data)const;
		void SetShadowMapSize(int nsize);
		/** set the shadow radius */
		void SetShadowRadius(float fRadius);

		/**
		* @param bUseGlobalAmbient: if true and that the ambient in pSurfaceMaterial is 0, the ambient in the material is ignored. and the scene's ambient color will be used.
		*/
		void applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient = true);
		void applyCameraMatrices();
		void applyWorldMatrices();
		void applyGlobalLightingData(CSunLight& sunlight);
		void applyLocalLightingData(const LightList* lights, int nLightNum);
		void applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor);
		void applyLayersNum(int nLayers);
		void applyTexWorldViewProj(const Matrix4* mat);

		// texture alpha operation attribute
		/** whether to enable alpha blending in shader. */
		void EnableAlphaBlending(bool bAlphaBlending);
		/** whether to enable alpha testing */
		void EnableAlphaTesting(bool bAlphaTesting);
		/** whether to enable sun light calculation */
		void EnableSunLight(bool bEnableSunLight);
		/** one occasion to disable textures is during the shadow pass. When textures are disabled all setTexture call will take no effect. */
		static void EnableTextures(bool bEnable);
		static bool AreTextureEnabled();
		/** whether to enable rendering with shadow map
		*@param nShadowMethod: 0 disable shadow map; 1 hardware shadow map; 2 F32 shadow map
		*/
		void EnableShadowmap(int nShadowMethod);

		/** set a boolean parameter. 
		@param nIndex: range 0-15, value 0-7 is reserved for lights.
		@return true if successful
		*/
		bool SetBoolean(int nIndex, bool value);

		/**
		* enable environment mapping 
		* @param bEnable 
		* @return true if successfully set. 
		*/
		bool EnableEnvironmentMapping(bool bEnable);
		/**
		* enable reflection mapping 
		* @param bEnable 
		* @param fSurfaceHeight: surface height in current model space. only used when bEnable is true. 
		* @return true if successfully set. 
		*/
		bool EnableReflectionMapping(bool bEnable, float fSurfaceHeight=0.f);
	
		/**
		* set shader parameter "reflectFactor"
		* @param fFactor (0,1). 1 is fully reflective. 0 is not reflective.
		*/
		void SetReflectFactor(float fFactor);
		/**
		* whether to enable the use of normal map 
		* @param bEnable 
		*/
		void EnableNormalMap(bool bEnable);

		/**
		* whether to enable the use of light map 
		* @param bEnable 
		*/
		void EnableLightMap(bool bEnable);

		/** get directX effect object associated with this object. */
		LPD3DXEFFECT GetDXEffect() {return m_pEffect;};

		/** get effect parameter block with this object. 
		* @param bCreateIfNotExist: 
		*/
		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);

		/** Lock a texture at a given index. A locked texture will make all subsequent calls to setTexture takes no effect, thus preserving old value in video card.
		* @param nIndex: 0 based index of the texture registry. If -1 it will lock all. 
		*/ 
		void LockTexture(int nIndex);
		/** UnLock a texture at a given index. A locked texture will make all subsequent calls to setTexture takes no effect, thus preserving old value in video card.
		* @param nIndex: 0 based index of the texture registry. If -1 it will unlock all. 
		*/ 
		void UnLockTexture(int nIndex);

		/** check whether a given texture is locked at the given index. 
		* @param nIndex: 0 based index of the texture registry. 
		*/
		bool IsTextureLocked(int nIndex) const;

		/** get texture handle at the given index. */
		D3DXHANDLE& GetTextureHandle(int nIndex);

		HRESULT SetRawValue(D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes);
		bool SetBool(D3DXHANDLE hParameter, BOOL bBoolean);
		bool SetInt(D3DXHANDLE hParameter, int nValue);
		bool SetFloat(D3DXHANDLE hParameter, float fValue);
		bool SetVector2(D3DXHANDLE hParameter, const Vector2& vValue);
		bool SetVector3(D3DXHANDLE hParameter, const Vector3& vValue);
		bool SetVector4(D3DXHANDLE hParameter, const Vector4& vValue);
		bool SetMatrix(D3DXHANDLE hParameter, const Matrix4& data);
	private:
		// effect file name
		string m_filename;
		// Data Types & Constants...
		static bool s_bUseHalfPrecision; // set to TRUE to use half-precision floats in all shaders

		// Private Data...
		LPD3DXEFFECT m_pEffect;
		D3DXEFFECT_DESC m_EffectDesc;
		vector<TechniqueDescDX> m_techniques;
		/** indicate whether a texture at a given index is locked. A locked texture will make all subsequent calls to setTexture takes no effect, thus preserving old value in video card.*/ 
		vector<bool> m_LockedTextures;
		/** it stores the last texture pointer passed to setTexture(), thus it will remove duplicated setTexture calls before passing to dx pipeline. 
		When a shader is switched in, all m_LastTextures will be cleared. */
		vector<void*> m_LastTextures;

		// the current technique index
		int m_nTechniqueIndex;
		bool m_bIsBegin;
		/** Whether effect file is in shared mode. if this is true, the begin() BeginPass() will takes no effects on shader, but only setting parameters.	*/
		bool m_bSharedMode;
		static bool g_bTextureEnabled;

		/** parameters that are shared by all objects using this effect.  When you call begin(), these parameters are set. 
		* It will overridden predefined parameter names, however most user will set custom shader parameters and textures that are shared by all objects here. 
		*/
		CParameterBlock m_SharedParamBlock;
	
		D3DXHANDLE m_paramHandle[k_max_param_handles];

		// Private Functions...
		void parseParameters();
		void releaseEffect();
		/** whether str begins with searchStr, the index of the next character following searchStr is returned. if not found -1 is returned. */
		int BeginWith(LPCSTR str, LPCSTR searchStr);
		/** get the integer from string str+nBeginIndex, pOut is the integer. return true if succeeded. */
		bool GetNumber(LPCSTR str, int nBeginIndex, int* pOut);
	};

	// this is the actually class used externally. it does following:
	// typedef CEffectFileDirectX CEffectFile;
	class CEffectFile : public CEffectFileDirectX
	{
	public:
		CEffectFile(const char* filename) :CEffectFileDirectX(filename){};
		CEffectFile(const AssetKey& key) :CEffectFileDirectX(key){};
		virtual  ~CEffectFile(){};
	};
} 

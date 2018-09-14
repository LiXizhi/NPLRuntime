#pragma once
#include "effect_file.h"
#include "ParameterBlock.h"
#include <unordered_map>
#include <functional>

namespace ParaEngine
{


	class CEffectFileImpl : public CEffectFileBase
	{


	public:
		CEffectFileImpl(const char* filename);
		CEffectFileImpl(const AssetKey& key);
	public:
		virtual  ~CEffectFileImpl() override;
		virtual HRESULT InitDeviceObjects() override;
		virtual HRESULT RestoreDeviceObjects() override;
		virtual HRESULT InvalidateDeviceObjects() override;
		virtual HRESULT DeleteDeviceObjects() override;
		virtual void SetFileName(const std::string& filename) override;

		virtual bool begin(bool bApplyParam = true, bool forceBegin = false) override;
		virtual bool BeginPass(int pass, bool bForceBegin = false) override;
		virtual void CommitChanges() override;
		virtual void EndPass(bool bForceEnd = false) override;
		virtual void end(bool bForceEnd = false) override;

		virtual bool SetFirstValidTechniqueByCategory(TechniqueCategory nCat) override;
		virtual bool SetTechniqueByIndex(int nIndex) override;
		virtual const TechniqueDesc* GetCurrentTechniqueDesc() override;


		virtual bool setParameter(eParameterHandles index, const void* data, uint32_t size) override;
		virtual bool setBool(eParameterHandles index, bool bBoolean)  override;
		virtual bool setInt(eParameterHandles index, int nValue)  override;
		virtual bool setFloat(eParameterHandles index, float fValue)  override;
		virtual bool setMatrix(eParameterHandles index, const Matrix4* data) override;
		virtual bool setMatrixArray(eParameterHandles index, const Matrix4* data, uint32_t count);
		virtual bool setFloatArray(eParameterHandles index, const float* data, uint32_t count);
		virtual bool setVectorArray(eParameterHandles index, const Vector4* pVector, uint32_t count);
		virtual bool setTexture(int index, TextureEntity* data) override;
		virtual bool setTexture(int index, IParaEngine::ITexture* pTex) override;
		virtual bool setTexture(eParameterHandles index, IParaEngine::ITexture* texture) override;


		virtual bool SetRawValue(const char* name, const void* pData, uint32_t ByteOffset, uint32_t Bytes) override;
		virtual bool SetBool(const char* name, bool bBoolean) override;
		virtual bool SetInt(const char* name, int nValue) override;
		virtual bool SetFloat(const char* name, float fValue) override;
		virtual bool SetVector2(const char* name, const Vector2& vValue) override;
		virtual bool SetVector3(const char* name, const Vector3& vValue) override;
		virtual bool SetVector4(const char* name, const Vector4& vValue) override;
		virtual bool SetMatrix(const char* name, const Matrix4& data) override;
		virtual bool SetBoolean(int nIndex, bool value) override;

		void applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient = true) override;
		void applyCameraMatrices() override;
		void applyWorldMatrices() override;
		void applyGlobalLightingData(CSunLight& sunlight) override;

		void EnableAlphaBlending(bool bAlphaBlending) override;
		virtual void EnableAlphaTesting(bool bAlphaTesting) override;
		virtual void EnableSunLight(bool bEnableSunLight) override;
		virtual bool EnableEnvironmentMapping(bool bEnable) override;
		virtual bool EnableReflectionMapping(bool bEnable, float fSurfaceHeight = 0.f) override;
		virtual void SetReflectFactor(float fFactor) override;
		virtual void EnableNormalMap(bool bEnable) override;
		virtual void EnableLightMap(bool bEnable) override;

	public:

		// save the resource to a file (or NULL to use the resource name)
		bool saveResource(const char* filename = 0);	
	 	
		void EnableShareMode(bool bEnable);
		bool IsInShareMode();


		/** This is a handy function that calls EnableShareMode(true), begin() and then BeginPass().
		*/
		bool BeginSharePassMode(bool bApplyParam = true);

		/** This is a handy function that calls EnableShareMode(false), EndPass(true), end(true).
		*/
		void EndSharePassMode();

		/** this is called when the shader is selected by the pipeline. There may be many objects drawn using the same shader between switch in and out. */
		void OnSwitchInShader();

		/** this is called when the shader is deselected by the pipeline. There may be many objects drawn using the same shader between switch in and out. */
		void OnSwitchOutShader();

		/**
		* current technique index.
		* @return
		*/
		int GetCurrentTechniqueIndex();

		int totalPasses() const;

		bool isParameterUsed(eParameterHandles index)const;
		bool isMatrixUsed(eParameterHandles index)const;
		bool isTextureUsed(int index)const;
		bool isTextureMatrixUsed(int index)const;
		bool setTextureInternal(int index, IParaEngine::ITexture* pTex);
		virtual void SetShadowMapSize(int nsize);
		/** set the shadow radius */
		void SetShadowRadius(float fRadius);

		void applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor);
		void applyLayersNum(int nLayers);
		void applyTexWorldViewProj(const Matrix4* mat);
		void applyLocalLightingData(const LightList* lights, int nLightNum);
		void applyBuiltinVariables();
		static void EnableTextures(bool bEnable);
		static bool AreTextureEnabled();
		void EnableShadowmap(int nShadowMethod);
		/** get directX effect object associated with this object. */
		std::shared_ptr<IParaEngine::IEffect> GetDeviceEffect();
		/** get effect parameter block with this object.
		* @param bCreateIfNotExist:
		*/
		CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);

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
		IParaEngine::ParameterHandle& GetTextureHandle(int nIndex);


	private:
		// effect file name
		string m_filename;
		// Data Types & Constants...
		static bool s_bUseHalfPrecision; // set to TRUE to use half-precision floats in all shaders

										 // Private Data...
		IParaEngine::EffectDesc m_EffectDesc;

		std::shared_ptr<IParaEngine::IEffect> m_pEffect;


		std::vector<TechniqueDesc> m_techniques;
		/** indicate whether a texture at a given index is locked. A locked texture will make all subsequent calls to setTexture takes no effect, thus preserving old value in video card.*/
		std::vector<bool> m_LockedTextures;
		/** it stores the last texture pointer passed to setTexture(), thus it will remove duplicated setTexture calls before passing to dx pipeline.
		When a shader is switched in, all m_LastTextures will be cleared. */
		std::vector<void*> m_LastTextures;

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

		IParaEngine::ParameterHandle m_paramHandle[k_max_param_handles];

		// Private Functions...
		void parseParameters();
		void releaseEffect();
		/** whether str begins with searchStr, the index of the next character following searchStr is returned. if not found -1 is returned. */
		int BeginWith(LPCSTR str, LPCSTR searchStr);
		/** get the integer from string str+nBeginIndex, pOut is the integer. return true if succeeded. */
		bool GetNumber(LPCSTR str, int nBeginIndex, int* pOut);

	};

	
	// this is the actually class used externally. it does following:

	class CEffectFile : public CEffectFileImpl
	{
	public:
		CEffectFile(const char* filename) :CEffectFileImpl(filename) {};
		CEffectFile(const AssetKey& key) :CEffectFileImpl(key) {};
		virtual  ~CEffectFile() {};
	};
} 
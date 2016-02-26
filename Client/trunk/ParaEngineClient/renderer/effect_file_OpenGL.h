#pragma once
#include "effect_file.h"
#include "ParameterBlock.h"

#include <unordered_map>

namespace cocos2d
{
	class GLProgram;
	struct Uniform;
}

namespace ParaEngine
{
	class CEffectFileOpenGL : public CEffectFileBase
	{
	public:
		CEffectFileOpenGL(const char* filename);
		CEffectFileOpenGL(const AssetKey& key);

		void Init();

		virtual  ~CEffectFileOpenGL();

		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		virtual HRESULT RendererRecreated();
		
	public:
		/** one occasion to disable texture s is during the shadow pass. When textures are disabled all setTexture call will take no effect. */
		static void EnableTextures(bool bEnable);
		static bool AreTextureEnabled();

		int totalPasses()const;

		/** whether to enable sun light calculation */
		void EnableSunLight(bool bEnableSunLight);
		/**
		* @param bUseGlobalAmbient: if true and that the ambient in pSurfaceMaterial is 0, the ambient in the material is ignored. and the scene's ambient color will be used.
		*/
		void applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient = true);
		void applyCameraMatrices();
		void applyWorldMatrices();
		void applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor);
		void applyGlobalLightingData(CSunLight& sunlight);

		bool setParameter(eParameterHandles index, const void* data, int32 size = D3DX_DEFAULT);
		bool setParameter(eParameterHandles index, const Vector2* data);
		bool setParameter(eParameterHandles index, const Vector3* data);
		bool setParameter(eParameterHandles index, const Vector4* data);

		bool setBool(eParameterHandles index, BOOL bBoolean);
		bool setInt(eParameterHandles index, int nValue);
		bool setFloat(eParameterHandles index, float fValue);

		HRESULT SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 Bytes);
		bool SetBool(const char* hParameter, BOOL bBoolean);
		bool SetInt(const char* hParameter, int nValue);
		bool SetFloat(const char* hParameter, float fValue);
		bool SetVector2(const char* hParameter, const Vector2& vValue);
		bool SetVector3(const char* hParameter, const Vector3& vValue);
		bool SetVector4(const char* hParameter, const Vector4& vValue);
		bool SetMatrix(const char* hParameter, const Matrix4& data);

		bool isParameterUsed(eParameterHandles index);
		bool setMatrix(eParameterHandles index, const Matrix4* data);
		bool isMatrixUsed(eParameterHandles index);

		bool setTexture(int index, TextureEntity* data);
		bool setTexture(int index, DeviceTexturePtr_type pTex);

		bool SetBoolean(int nIndex, bool value);

		bool begin(bool bApplyParam = true, DWORD flag = 0);
		bool BeginPass(int pass, bool bForceBegin = false);
		void CommitChanges();
		void EndPass(bool bForceEnd = false);
		void end(bool bForceEnd = false);

		virtual HRESULT InitDeviceObjects();

		bool LoadBuildinShader();

		virtual HRESULT DeleteDeviceObjects();
		/** 
		* @param nPass: -1 to release all 
		*/
		void releaseEffect(int nPass = -1);

		/** set inner file name, usually at the load time. */
		void SetFileName(const std::string& filename);
		const std::string& GetFileName();

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
		bool EnableReflectionMapping(bool bEnable, float fSurfaceHeight = 0.f);

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

		/** whether to enable alpha blending in shader. */
		void EnableAlphaBlending(bool bAlphaBlending);
		/** whether to enable alpha testing */
		void EnableAlphaTesting(bool bAlphaTesting);
	

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

	public:
		bool setParameter(cocos2d::Uniform* uniform, const void* data, int32 size = D3DX_DEFAULT);

		cocos2d::GLProgram* GetGLProgram(int nPass = 0, bool bCreateIfNotExist = false);
		
		/** Initializes the GLProgram with a vertex and fragment with bytes array
		*/
		bool initWithByteArrays(const char* vShaderByteArray, const char* fShaderByteArray, int nPass = 0);

		/** Initializes the GLProgram with a vertex and fragment with contents of filenames
		*/
		bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, int nPass = 0);

		/** links the glProgram */
		bool link(int nPass = 0);
		/** it will call glUseProgram() */
		bool use(int nPass = 0);
		/** It will create 4 uniforms:
		- kUniformPMatrix
		- kUniformMVMatrix
		- kUniformMVPMatrix
		- GLProgram::UNIFORM_SAMPLER

		And it will bind "GLProgram::UNIFORM_SAMPLER" to 0
		@param nPass: if -1, it will update active pass, otherwise it will be current active pass. 
		*/
		void updateUniforms(int nPass = -1);
		
		cocos2d::Uniform* GetUniformByID(eParameterHandles id);
		cocos2d::Uniform* GetUniform(const std::string& sName);
		
		/** add changes to shader parameters, those changes are commited to device when CommitChange() is called. */
		template <typename ValueType>
		void AddParamChange(const std::string& sName, const ValueType& value)
		{
			for (uint32 i = 0; i < m_pendingChangesCount; ++i)
			{
				if (m_pendingChanges[i].GetName() == sName)
				{
					m_pendingChanges[i] = value;
					return;
				}
			}
			++ m_pendingChangesCount;
			if (m_pendingChanges.size() < m_pendingChangesCount)
				m_pendingChanges.resize(m_pendingChangesCount + 4);
			m_pendingChanges[m_pendingChangesCount - 1].SetName(sName);
			m_pendingChanges[m_pendingChangesCount - 1] = value;
		}

	protected:
		std::vector<cocos2d::GLProgram*> m_programs;
		static std::unordered_map<uint32, std::string> s_id_to_names;
		static std::unordered_map<uint32, std::string> LoadStaticIdNameMap();
		/** current active pass */
		int m_nActivePassIndex;
		bool m_bIsBegin;
		std::string m_filename;
		
		std::vector<CParameter> m_pendingChanges;
		uint32 m_pendingChangesCount;
	};

	
	// this is the actually class used externally. it does following:
	// typedef CEffectFileOpenGL CEffectFile;
	class CEffectFile : public CEffectFileOpenGL
	{
	public:
		CEffectFile(const char* filename) :CEffectFileOpenGL(filename){};
		CEffectFile(const AssetKey& key) :CEffectFileOpenGL(key){};
		virtual  ~CEffectFile(){};
	};
} 
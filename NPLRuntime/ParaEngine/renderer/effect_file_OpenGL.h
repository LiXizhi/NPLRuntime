#pragma once
#include "effect_file.h"
#include "ParameterBlock.h"
#include "dxeffects.h"
#include <unordered_map>
#include <functional>

struct UniformInfo
{
	std::string name;
	std::string semantic;
	std::string type;
	bool operator == (const UniformInfo& r)
	{
		return r.name == this->name;
	}
	bool operator != (const UniformInfo&r)
	{
		return r.name != this->name;
	}
};

namespace ParaEngine
{
	class GLProgram;
	struct Uniform;


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

		bool begin(bool bApplyParam = true, DWORD flag = 0, bool bForceBegin = false);
		bool BeginPass(int pass, bool bForceBegin = false);
		void CommitChanges();
		void EndPass(bool bForceEnd = false);
		void end(bool bForceEnd = false);

		virtual HRESULT InitDeviceObjects() override;
		virtual HRESULT DeleteDeviceObjects() override;

		/** get effect parameter block with this object.
		* @param bCreateIfNotExist:
		*/
		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);

		/** 
		* @param nPass: -1 to release all 
		*/
		void releaseEffect(int nTech = -1, int nPass = -1);

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
		/** Initializes the GLProgram with a vertex and fragment with bytes array
		*/
		bool initWithByteArrays(const char* vShaderByteArray, const char* fShaderByteArray, int nTech = 0, int nPass = 0);

		/** Initializes the GLProgram with a vertex and fragment with contents of filenames
		*/
		bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, int nTech = 0, int nPass = 0);

		/** links the glProgram */
		bool link(int nTech = 0, int nPass = 0);
		
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

		void SetShadowMapSize(int nsize);

	protected:
		GLProgram* GetGLProgram(int nTech, int nPass, bool bCreateIfNotExist = false);
		typedef std::function<int (GLProgram* pProgram)> ProgramCallbackFunction_t;
		bool SetProgramParams(ProgramCallbackFunction_t func);
		/** it will call glUseProgram() */
		bool use(int nTech = -1, int nPass = -1);

		bool setParameter(Uniform* uniform, const void* data, int32 size = D3DX_DEFAULT);
		bool MappingEffectUniforms(const std::vector<UniformInfo>& uniforms);
		bool GeneratePasses();

		/** It will create 4 uniforms:
		- kUniformPMatrix
		- kUniformMVMatrix
		- kUniformMVPMatrix
		- GLProgram::UNIFORM_SAMPLER

		And it will bind "GLProgram::UNIFORM_SAMPLER" to 0
		@param nPass: if -1, it will update active pass, otherwise it will be current active pass.
		*/
		void updateUniforms(int nTech = -1, int nPass = -1);

		Uniform* GetUniformByID(eParameterHandles id);
		Uniform* GetUniform(const std::string& sName);
	protected:
		std::unordered_map<uint32, std::string> m_ID2Names;
		DxEffectsTree* m_Effect;

		struct TechniqueDescGL : public TechniqueDesc
		{
			std::vector<GLProgram*> m_passes;
		};
		std::vector<TechniqueDescGL> m_techniques;
		int m_nTechniqueIndex;
		/** current active pass */
		int m_nActivePassIndex;
		bool m_bIsBegin;
		bool m_bIsBeginPass;
		std::string m_filename;
		
		std::vector<CParameter> m_pendingChanges;
		uint32 m_pendingChangesCount;

		/** parameters that are shared by all objects using this effect.  When you call begin(), these parameters are set.
		* It will overridden predefined parameter names, however most user will set custom shader parameters and textures that are shared by all objects here.
		*/
		CParameterBlock m_SharedParamBlock;
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
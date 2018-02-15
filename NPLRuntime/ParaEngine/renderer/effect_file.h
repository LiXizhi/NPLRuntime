#pragma once
#include <string>
#include <vector>
#include <list>
#include "AssetEntity.h"
#include "LightManager.h"
#include "ParameterBlock.h"
#include "EffectFileHandles.h"

namespace ParaEngine
{
	using namespace std;
	class CSunLight;

	/**
	* asset entity: CEffectFile
	*/
	class CEffectFileBase : public AssetEntity
	{
	public:
		const static int MAX_EFFECT_LIGHT_NUM = 4;

		/** The game engine will identified some predefined technique name from the the effect file, and given them a category number
		* e.g. a technique called, GenShadowMap, will be assigned to the shadow map category. if the technique name is not identified
		it will be assigned to the default technique. */
		enum TechniqueCategory
		{
			/// technique name is not identified
			TechCategory_default = 0,
			/// technique name is "GenShadowMap"
			TechCategory_GenShadowMap,
			/// technique name is "Character"
			TechCategory_Character,
			/// technique name is "DetailCharacter"
			TechCategory_DetailCharacter,
			/// technique name is "Vegetation"
			TechCategory_Vegetation,
		};

		enum eParameterHandles
		{
			// matrix
			k_worldMatrix = 0,
			k_worldInverseMatrix,
			k_worldViewMatrix,
			k_worldViewProjMatrix,
			k_worldMatrixArray,
			k_viewMatrix,
			k_projMatrix,
			k_viewProjMatrix,
			k_TexWorldViewProjMatrix,
			k_skyBoxMatrix,


			// parameters
			k_first_parameter,
			k_boneInfluenceCount,
			k_ambientMaterialColor,
			k_diffuseMaterialColor,
			k_emissiveMaterialColor,
			k_specularMaterialColor,
			k_specularMaterialPower,
			k_reflectFactor,
			k_LayersNum, // how many alpha texture layers
			k_LocalLightNum,	// how many local lights are enabled
			k_LightPositions,	// vector array of float4(x,y,z,range)
			k_LightColors,		// vector array of float4(r,g,b,intensity)
			k_LightParams,		// vector array of float4(reserved, reserved,reserved,reserved)
			k_patchCorners,
			k_posScaleOffset,
			k_uvScaleOffset,
			k_lensFlareColor,
			k_sunVector,
			k_sunColor,
			k_fresnelR0,
			k_ambientLight,
			k_atmosphericLighting,
			k_cameraPos,
			k_cameraDistances,
			k_cameraFacing,
			k_sunlightInscatter,
			k_sunlightExtinction,
			k_LightStrength,
			k_bSunlightEnable,
			k_nShadowmapSize,
			k_fShadowRadius,
			k_fogEnable,
			k_fogParameters,/// float4(fog_start, fog_range, fogDensity, reserved)
			k_fogColor,
			k_worldPos,
			k_ConstVector0,
			k_ConstVector1,
			k_ConstVector2,
			k_ConstVector3,
			k_time, // scaler
			k_specularPower, // specular scaler
			k_texCoordOffset, // vector
			k_opacity, // scaler
			k_shadowFactor, //shadow factor, float2(x,1-x)
			k_transitionFactor,
			/**
			boolean constants are for static branching,
			 - Boolean0-3 is reserved for lights.
			 - Boolean4 is reserved for environment mapping.
			 - Boolean5 is reserved for reflection mapping
			 - Boolean6 is reserved for normal mapping
			 - Boolean 8,9 is reserved for shadow mapping.
			 - Other booleans can be used where necessary
			*/
			k_bAlphaTesting,
			k_bAlphaBlending,
			k_bBoolean0,
			k_bBoolean1,
			k_bBoolean2,
			k_bBoolean3,
			k_bBoolean4,
			k_bBoolean5,
			k_bBoolean6,
			k_bBoolean7,
			k_bBoolean8,
			k_bBoolean9,
			k_bBoolean10,
			k_bBoolean11,
			k_bBoolean12,
			k_bBoolean13,
			k_bBoolean14,
			k_bBoolean15,
			k_bBooleanMAX, // this is not used.

			// textures
			k_tex0,
			k_tex1,
			k_tex2,
			k_tex3,
			k_tex4,
			k_tex5,
			k_tex6,
			k_tex7,
			k_tex8,
			k_tex9,
			k_tex10,
			k_tex11,
			k_tex12,
			k_tex13,
			k_tex14,
			k_tex15,
			k_tex_max,

			// textures matrix
			k_tex_mat0,
			k_tex_mat1,
			k_tex_mat2,
			k_tex_mat3,
			k_tex_mat4,
			k_tex_mat5,
			k_tex_mat6,
			k_tex_mat7,
			k_tex_mat8,
			k_tex_mat9,
			k_tex_mat10,
			k_tex_mat11,
			k_tex_mat12,
			k_tex_mat13,
			k_tex_mat14,
			k_tex_mat15,
			k_tex_mat_max,

			// last one
			k_max_param_handles
		};

		/** a technique description */
		struct TechniqueDesc
		{
			TechniqueCategory nCategory;
		public:
			TechniqueDesc() :nCategory(TechCategory_default){
			}
		};

	public:
		CEffectFileBase(){};
		CEffectFileBase(const AssetKey& key) :AssetEntity(key){};
		virtual  ~CEffectFileBase(){};

		virtual AssetType GetType(){return effectfile;};

		// Mutators...
		virtual HRESULT InitDeviceObjects(){ return S_OK; };
		virtual HRESULT RestoreDeviceObjects(){ return S_OK; };
		virtual HRESULT InvalidateDeviceObjects(){ return S_OK; };
		virtual HRESULT DeleteDeviceObjects(){ return S_OK; };

		/** set inner file name, usually at the load time. */
		void SetFileName(const std::string& filename){};

		static void EnableTextures(bool bEnable){};
		static bool AreTextureEnabled(){ return true; };

		int totalPasses()const { return 0; };

		/** whether to enable sun light calculation */
		void EnableSunLight(bool bEnableSunLight) {};
		/**
		* @param bUseGlobalAmbient: if true and that the ambient in pSurfaceMaterial is 0, the ambient in the material is ignored. and the scene's ambient color will be used.
		*/
		void applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient = true) {};
		void applyCameraMatrices() {};
		void applyWorldMatrices() {};
		void applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor) {};
		void applyGlobalLightingData(CSunLight& sunlight) {};

		bool setParameter(eParameterHandles index, const void* data, int32 size = D3DX_DEFAULT) { return false; };
		bool setParameter(eParameterHandles index, const Vector2* data)  { return false; };
		bool setParameter(eParameterHandles index, const Vector3* data)  { return false; };
		bool setParameter(eParameterHandles index, const Vector4* data)  { return false; };

		bool setBool(eParameterHandles index, BOOL bBoolean)  { return false; };
		bool setInt(eParameterHandles index, int nValue)  { return false; };
		bool setFloat(eParameterHandles index, float fValue)  { return false; };

		HRESULT SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 Bytes)  { return S_OK; };
		bool SetBool(const char* hParameter, BOOL bBoolean)  { return false; };
		bool SetInt(const char* hParameter, int nValue)  { return false; };
		bool SetFloat(const char* hParameter, float fValue)  { return false; };
		bool SetVector2(const char* hParameter, const Vector2& vValue)  { return false; };
		bool SetVector3(const char* hParameter, const Vector3& vValue)  { return false; };
		bool SetVector4(const char* hParameter, const Vector4& vValue)  { return false; };
		bool SetMatrix(const char* hParameter, const Matrix4& data)  { return false; };

		bool isParameterUsed(eParameterHandles index)  { return false; };
		bool setMatrix(eParameterHandles index, const Matrix4* data) { return false; };
		bool isMatrixUsed(eParameterHandles index) { return false; };

		bool setTexture(int index, TextureEntity* data) { return false; };
		bool setTexture(int index, DeviceTexturePtr_type pTex) { return false; };

		bool SetBoolean(int nIndex, bool value) { return false; };

		bool begin(bool bApplyParam = true, DWORD flag = 0) { return false; };
		bool BeginPass(int pass, bool bForceBegin = false) { return false; };
		void CommitChanges() {};
		void EndPass(bool bForceEnd = false) {};
		void end(bool bForceEnd = false) {};

		bool LoadBuildinShader() { return true; };

		/**
		* @param nPass: -1 to release all
		*/
		void releaseEffect(int nPass = -1) {};

		/**
		* enable environment mapping
		* @param bEnable
		* @return true if successfully set.
		*/
		bool EnableEnvironmentMapping(bool bEnable) { return false; };
		/**
		* enable reflection mapping
		* @param bEnable
		* @param fSurfaceHeight: surface height in current model space. only used when bEnable is true.
		* @return true if successfully set.
		*/
		bool EnableReflectionMapping(bool bEnable, float fSurfaceHeight = 0.f) { return false; };

		/**
		* set shader parameter "reflectFactor"
		* @param fFactor (0,1). 1 is fully reflective. 0 is not reflective.
		*/
		void SetReflectFactor(float fFactor) {};
		/**
		* whether to enable the use of normal map
		* @param bEnable
		*/
		void EnableNormalMap(bool bEnable) {};

		/**
		* whether to enable the use of light map
		* @param bEnable
		*/
		void EnableLightMap(bool bEnable) {};

		/** whether to enable alpha blending in shader. */
		void EnableAlphaBlending(bool bAlphaBlending) {};

		/** whether to enable alpha testing */
		void EnableAlphaTesting(bool bAlphaTesting) {};


		/**
		* Set technique if it is different from the current one.
		*@param nCat: @see TechniqueCategory
		*@return: return true if the technique is found and successfully set.
		*/
		bool SetFirstValidTechniqueByCategory(TechniqueCategory nCat) { return false; };
		/**
		* Set technique if it is different from the current one.
		* multiple calls to this function with the same index takes no effect.
		* @param nIndex 0 based index.
		* @return: return true if the technique is found and successfully set.
		*/
		bool SetTechniqueByIndex(int nIndex) { return false; };
		/** get the current technique description. This function may return NULL*/
		const TechniqueDesc* GetCurrentTechniqueDesc() { return NULL; };
	};
}

#ifdef USE_DIRECTX_RENDERER
#include "effect_file_DirectX.h"
#elif defined(USE_OPENGL_RENDERER)
#include "effect_file_OpenGL.h"
#else
namespace ParaEngine{
	class CEffectFile : public CEffectFileBase {
	public:
		CEffectFile(const AssetKey& key){};
	};
}
#endif

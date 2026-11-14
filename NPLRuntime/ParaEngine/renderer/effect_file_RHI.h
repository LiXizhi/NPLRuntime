#pragma once

#include "effect_file.h"
#include "ParameterBlock.h"
#include <string>
#include <unordered_map>
#include <vector>

#ifndef D3DXFX_DONOTSAVESTATE
#define D3DXFX_DONOTSAVESTATE        (1 << 0)
#endif
#ifndef D3DXFX_DONOTSAVESHADERSTATE
#define D3DXFX_DONOTSAVESHADERSTATE  (1 << 1)
#endif
#ifndef D3DXFX_DONOTSAVESAMPLERSTATE
#define D3DXFX_DONOTSAVESAMPLERSTATE (1 << 2)
#endif

namespace ParaEngine
{
	class GLProgram;
	class IRHIShaderProgram;
	struct FxSamplerDesc;
	struct FxRenderStateAssignment;

	/**
	 * Unified cross-platform effect file implementation (ParaRHI).
	 * Replaces compile-time CEffectFileDirectX / CEffectFileOpenGL split and
	 * supports runtime backend selection across D3D11, D3D12, Vulkan, Metal,
	 * OpenGL/GLES/WebGL, WebGPU, and headless Null.
	 */
	class CEffectFileRHI : public CEffectFileBase
	{
	public:
		CEffectFileRHI(const char* filename);
		CEffectFileRHI(const AssetKey& key);
		virtual ~CEffectFileRHI() override;

		virtual HRESULT InitDeviceObjects() override;
		virtual HRESULT RestoreDeviceObjects() override;
		virtual HRESULT InvalidateDeviceObjects() override;
		virtual HRESULT DeleteDeviceObjects() override;
		virtual HRESULT RendererRecreated() override;

		void SetFileName(const std::string& filename);
		const std::string& GetFileName() const;

		static void EnableTextures(bool bEnable);
		static bool AreTextureEnabled();

		int totalPasses() const;

		bool begin(bool bApplyParam = true, DWORD flag = D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE, bool bForceBegin = false);
		bool BeginPass(int pass, bool bForceBegin = false);
		void CommitChanges();
		void EndPass(bool bForceEnd = false);
		void end(bool bForceEnd = false);

		void EnableShareMode(bool bEnable);
		bool IsInShareMode() const;
		bool BeginSharePassMode(bool bApplyParam = true, DWORD flag = D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE, bool bForceBegin = true);
		void EndSharePassMode();

		void OnSwitchInShader();
		void OnSwitchOutShader();

		bool SetFirstValidTechniqueByCategory(TechniqueCategory nCat);
		bool SetTechniqueByIndex(int nIndex);
		const TechniqueDesc* GetCurrentTechniqueDesc();
		int GetCurrentTechniqueIndex() const;

		bool isParameterUsed(eParameterHandles index) const;
		bool isMatrixUsed(eParameterHandles index) const;
		bool isTextureUsed(int index) const;
		bool isTextureMatrixUsed(int index) const;

		bool setParameter(eParameterHandles index, const void* data, int32 size = D3DX_DEFAULT);
		bool setParameter(eParameterHandles index, const Vector2* data);
		bool setParameter(eParameterHandles index, const Vector3* data);
		bool setParameter(eParameterHandles index, const Vector4* data);

		bool setBool(eParameterHandles index, BOOL bBoolean);
		bool setInt(eParameterHandles index, int nValue);
		bool setFloat(eParameterHandles index, float fValue);
		bool setMatrix(eParameterHandles index, const Matrix4* data);
		bool setMatrixArray(eParameterHandles index, const Matrix4* data, uint32 count);
		bool setMatrixInArray(eParameterHandles index, uint32 element, const Matrix4* data);
		bool setFloatArray(eParameterHandles index, const float* data, uint32 count);
		bool setVectorArray(eParameterHandles index, const Vector4* pVector, uint32 count);
		bool setFloatInArray(eParameterHandles index, uint32 element, float data);
		bool setTextureMatrix(int index, const Matrix4* data);

		bool setTexture(int index, TextureEntity* data);
		bool setTexture(int index, DeviceTexturePtr_type pTex);
		bool SetTextureByName(const char* name, TextureEntity* data);

		void LockTexture(int nIndex);
		void UnLockTexture(int nIndex);
		bool IsTextureLocked(int nIndex) const;

		HRESULT SetRawValue(const char* hParameter, const void* pData, uint32 ByteOffset, uint32 Bytes);
		bool SetBool(const char* hParameter, BOOL bBoolean);
		bool SetInt(const char* hParameter, int nValue);
		bool SetFloat(const char* hParameter, float fValue);
		bool SetVector2(const char* hParameter, const Vector2& vValue);
		bool SetVector3(const char* hParameter, const Vector3& vValue);
		bool SetVector4(const char* hParameter, const Vector4& vValue);
		bool SetMatrix(const char* hParameter, const Matrix4& data);

		void SetShadowMapSize(int nsize);
		void SetShadowRadius(float fRadius);
		void EnableShadowmap(int nShadowMethod);
		bool SetBoolean(int nIndex, bool value);

		void EnableSunLight(bool bEnableSunLight);
		void EnableAlphaBlending(bool bAlphaBlending);
		void EnableAlphaTesting(bool bAlphaTesting);
		bool EnableEnvironmentMapping(bool bEnable);
		bool EnableReflectionMapping(bool bEnable, float fSurfaceHeight = 0.f);
		void SetReflectFactor(float fFactor);
		void EnableNormalMap(bool bEnable);
		void EnableLightMap(bool bEnable);

		void applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient = true);
		void applyCameraMatrices();
		void applyWorldMatrices();
		void applyGlobalLightingData(CSunLight& sunlight);
		void applyLocalLightingData(const LightList* lights, int nLightNum);
		void applyFogParameters(bool bEnableFog, const Vector4* fogParam, const LinearColor* fogColor);
		void applyLayersNum(int nLayers);
		void applyTexWorldViewProj(const Matrix4* mat);

		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);
		void releaseEffect(int nTech = -1, int nPass = -1);

	private:
		enum class EParamKind : uint8_t
		{
			Float = 0,
			Bool,
			Int,
			Matrix,
			RawStruct
		};

		struct ParamSlot
		{
			std::string name;
			std::string semantic;
			EParamKind kind = EParamKind::Float;
			uint8_t scalarElemBytes = 16;
			uint32_t elementCount = 1;
			std::vector<uint8_t> data;
			uint32_t revision = 0;
		};

		struct PassParamBinding
		{
			int32_t slotIndex = -1;
			int32_t vsOffset = -1;
			uint32_t vsSize = 0;
			int32_t psOffset = -1;
			uint32_t psSize = 0;
			uint32_t syncedRevision = 0;
		};

		struct PassStateEntry
		{
			ERenderState state = ERenderState::Unknown;
			uint32_t value = 0;
		};

		struct SamplerDescEntry
		{
			std::string samplerName;
			std::string textureName;
			int registerIndex = 0;
			bool hasMinFilter = false;
			bool hasMagFilter = false;
			bool hasMipFilter = false;
			bool hasAddressU = false;
			bool hasAddressV = false;
			bool hasBorderColor = false;
			uint32_t minFilter = 2;
			uint32_t magFilter = 2;
			uint32_t mipFilter = 2;
			uint32_t addressU = 1;
			uint32_t addressV = 1;
			uint32_t borderColor = 0;
		};

		struct PassRuntime
		{
			std::string name;
			GLProgram* glProgram = nullptr;
			IRHIShaderProgram* rhiProgram = nullptr;
			std::vector<uint8_t> vsCBuffer;
			std::vector<uint8_t> psCBuffer;
			bool cbufferDirty = true;
			std::vector<PassParamBinding> bindings;
			std::vector<PassStateEntry> renderStates;
		};

		struct TechniqueRuntime : public TechniqueDesc
		{
			std::string name;
			std::vector<PassRuntime> passes;
		};

		struct SavedRenderState
		{
			ERenderState state;
			uint32_t oldValue;
		};

		struct SavedSamplerState
		{
			uint32_t stage;
			ESamplerStateType type;
			uint32_t oldValue;
		};

		bool WriteSlotBytes(int32_t slotIndex, const void* pData, uint32_t byteOffset, uint32_t byteCount, EParamKind writeKind);
		void SyncPassUniforms(PassRuntime& pass, bool isOpenGL);
		void ApplyPassStates(const PassRuntime& pass);
		void RestoreSavedStates();
		int32_t FindOrCreateSlotByName(const std::string& name);

	private:
		std::string m_filename;
		std::vector<TechniqueRuntime> m_techniques;
		std::vector<ParamSlot> m_paramSlots;
		std::unordered_map<std::string, int32_t> m_nameToSlot;
		int32_t m_handleToSlot[k_max_param_handles];
		bool m_textureUsed[16];
		std::vector<SamplerDescEntry> m_samplers;
		std::unordered_map<std::string, int> m_textureNameToStage;

		std::vector<bool> m_LockedTextures;
		std::vector<DeviceTexturePtr_type> m_LastTextures;
		std::vector<TextureEntity*> m_NamedStageTextures;

		std::vector<SavedRenderState> m_savedRenderStates;
		std::vector<SavedSamplerState> m_savedSamplerStates;

		int m_nTechniqueIndex = 0;
		int m_nActivePassIndex = 0;
		DWORD m_dwBeginFlags = 0;
		bool m_bIsBegin = false;
		bool m_bIsBeginPass = false;
		bool m_bSharedMode = false;
		static bool g_bTextureEnabled;

		CParameterBlock m_SharedParamBlock;
	};

	using CEffectFileOpenGL = CEffectFileRHI;

	class CEffectFile : public CEffectFileRHI
	{
	public:
		CEffectFile(const char* filename) : CEffectFileRHI(filename) {}
		CEffectFile(const AssetKey& key) : CEffectFileRHI(key) {}
		virtual ~CEffectFile() override = default;
	};
}

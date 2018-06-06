#pragma once
#include <vector>
#include <cstdint>
#include "Core/PEtypes.h"
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Interface/Common.h"

namespace ParaEngine
{
	class Matrix4;
	class Vector4;
}

namespace IParaEngine
{

	PARA_HANDLE(TechniqueHandle)

	PARA_HANDLE(ParameterHandle)

	struct TechniqueDesc
	{
		std::string Name;
		uint8_t Passes;
	};

	struct EffectDesc
	{
		uint16_t Parameters; // Number of parameters
		uint16_t Techniques; // Number of techniques
	};

	enum EParameterType
	{
		PT_VOID,
		PT_BOOL,
		PT_INT,
		PT_FLOAT,
		PT_FLOAT2,
		PT_FLOAT3,
		PT_FLOAT4,
		PT_FLOAT2x2,
		PT_FLOAT2x3,
		PT_FLOAT2x4,
		PT_FLOAT3x3,
		PT_FLOAT3x4,
		PT_FLOAT4x2,
		PT_FLOAT4x3,
		PT_FLOAT4x4,
		PT_TEXTURE,
		PT_TEXTURE1D,
		PT_TEXTURE2D,
		PT_TEXTURE3D,
		PT_TEXTURECUBE,
		PT_SAMPLER,
		PT_SAMPLER1D,
		PT_SAMPLER2D,
		PT_SAMPLER3D,
		PT_SAMPLERCUBE,
		PT_STRUCT,
		PT_UNSUPPORTED
	};

	struct ParameterDesc
	{
		std::string Name;		// Parameter name
		std::string Semantic;	// Parameter semantic
		EParameterType Type;	// Component type
		uint32_t Elements;		// Number of array elements
	};


	class IEffectInclude
	{
	public:
		virtual ~IEffectInclude() = default;
		virtual bool Open(const std::string& filename,void** ppData,uint32_t* pBytes) = 0;
		virtual void Close(void* pData) = 0;
	};

	class IEffect
	{
	public:
		IEffect() = default;
		virtual ~IEffect() = default;
		virtual bool GetDesc(EffectDesc* pOutDesc) = 0;

		virtual TechniqueHandle GetTechnique(uint32_t index) = 0;
		virtual bool GetTechniqueDesc(const TechniqueHandle& handle, TechniqueDesc* pOutDesc) = 0;

		virtual ParameterHandle GetParameter(uint32_t index) = 0;
		virtual ParameterHandle GetParameterByName(const char* name) = 0;
		virtual bool GetParameterDesc(const ParameterHandle& handle, ParameterDesc* pOutDesc) = 0;
		virtual bool SetTechnique(const TechniqueHandle& handle) = 0;

		virtual bool SetMatrixArray(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data, uint32_t count) = 0;
		virtual bool SetMatrix(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data) = 0;
		virtual bool SetVectorArray(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data, uint32_t count) = 0;
		virtual bool SetVector(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data) = 0;
		virtual bool SetFloatArray(const ParameterHandle& handle, const float* data, uint32_t count) = 0;
		virtual bool SetValue(const ParameterHandle& handle, const void* data, uint32_t size) = 0;
		virtual bool SetBool(const ParameterHandle& handle, bool value) = 0;
		virtual bool SetInt(const ParameterHandle& handle, int value) = 0;
		virtual bool SetFloat(const ParameterHandle& handle, float value) = 0;
		virtual bool SetTexture(const ParameterHandle& handle, ParaEngine::DeviceTexturePtr_type texture) = 0;
		virtual bool SetTexture(const char* name, ParaEngine::DeviceTexturePtr_type texture) = 0;
		virtual bool SetRawValue(const ParameterHandle& handle, const void* data, uint32_t offset, uint32_t size) = 0;
		virtual bool SetRawValue(const char* name, const void* data, uint32_t offset, uint32_t size) = 0;

		virtual void OnLostDevice() = 0;
		virtual void OnResetDevice() = 0;
		virtual bool Begin() = 0;
		virtual bool BeginPass(const uint8_t pass) = 0;
		virtual bool EndPass() = 0;
		virtual bool End() = 0;
		virtual TechniqueHandle GetCurrentTechnique() = 0;
		virtual bool CommitChanges() = 0;

	};
}
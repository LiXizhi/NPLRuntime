#pragma once
#include <vector>
#include <cstdint>
#include "Framework/Interface/Common.h"
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
		FLOAT2,
		FLOAT3,
		FLOAT4
	};

	struct ParameterDesc
	{
		std::string Name;
		std::string Semantic;
		EParameterType Type;
	};


	class IEffectInclude
	{
	public:
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
		virtual bool GetParameterDesc(const ParameterHandle& handle, ParameterHandle* pOutDesc) = 0;
		virtual bool SetTechnique(const TechniqueHandle& handle) = 0;
	};
}
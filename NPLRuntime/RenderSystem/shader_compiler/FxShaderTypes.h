#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "RenderTypes.h"

namespace ParaEngine
{
	enum class EShaderTarget : uint8_t
	{
		Unknown = 0,
		GLSL_120 = 1,
		GLSL_330 = 2,
		ESSL_100 = 3,
		ESSL_300 = 4,
		DXBC_SM40_93 = 5,
		DXBC_SM40 = 6,
		DXBC_SM50 = 7,
		DXIL_SM60 = 8,
		SPIRV_1_0 = 9,
		MSL_2_0 = 10,
		WGSL_1_0 = 11
	};

	struct FxRenderStateAssignment
	{
		ERenderState state = ERenderState::Unknown;
		uint32_t value = 0;
	};

	struct FxSamplerDesc
	{
		std::string samplerName;
		std::string textureName;
		std::string samplerType = "sampler2D"; // sampler2D, sampler3D, samplerCUBE
		int registerIndex = -1;

		bool hasMinFilter = false;
		bool hasMagFilter = false;
		bool hasMipFilter = false;
		bool hasAddressU = false;
		bool hasAddressV = false;
		bool hasAddressW = false;
		bool hasBorderColor = false;

		uint32_t minFilter = 2;   // D3DTEXF_LINEAR
		uint32_t magFilter = 2;   // D3DTEXF_LINEAR
		uint32_t mipFilter = 2;   // D3DTEXF_LINEAR
		uint32_t addressU = 1;    // D3DTADDRESS_WRAP
		uint32_t addressV = 1;    // D3DTADDRESS_WRAP
		uint32_t addressW = 1;    // D3DTADDRESS_WRAP
		uint32_t borderColor = 0; // ARGB
	};

	struct FxUniformParam
	{
		std::string name;
		std::string semantic;
		std::string typeName;
		int registerIndex = -1;
		int elementCount = 1; // > 1 for arrays
		bool isBool = false;
		bool isInt = false;
		bool hasDefault = false;
		std::vector<float> defaultFloats;
	};

	struct FxConstantMember
	{
		std::string name;
		uint32_t byteOffset = 0;
		uint32_t byteSize = 0;
		uint16_t rows = 1;
		uint16_t cols = 1;
		uint16_t elements = 1;
		uint8_t baseType = 0; // 0 = float, 1 = int, 2 = bool, 3 = uint
	};

	struct FxResourceBinding
	{
		std::string name;
		uint32_t bindPoint = 0;
		uint32_t bindCount = 1;
	};

	struct FxCompiledStage
	{
		bool valid = false;
		EShaderTarget target = EShaderTarget::Unknown;
		std::string entryPoint;
		std::string profile;
		std::vector<uint8_t> bytecode; // Binary DXBC/SPIRV/DXIL or UTF-8 source text for GLSL/ESSL/MSL/WGSL
		uint32_t cbufferSize = 0;
		std::vector<FxConstantMember> cbufferMembers;
		std::vector<FxResourceBinding> textures;
		std::vector<FxResourceBinding> samplers;
	};

	struct FxCompiledPass
	{
		std::string name;
		bool stateOnly = false;
		FxCompiledStage vs;
		FxCompiledStage ps;
		std::vector<FxRenderStateAssignment> renderStates;
	};

	struct FxCompiledTechnique
	{
		std::string name;
		std::vector<FxCompiledPass> passes;
	};

	struct FxCompiledEffect
	{
		std::string sourceName;
		EShaderTarget target = EShaderTarget::Unknown;
		std::vector<FxUniformParam> params;
		std::vector<FxSamplerDesc> samplers;
		std::vector<FxCompiledTechnique> techniques;

		bool SerializeToPFX(std::vector<uint8_t>& outBytes) const;
		bool DeserializeFromPFX(const void* data, size_t size);
	};

	// Canonical ParaRHI Vertex Attribute Location mapping (0..15) shared by SPIR-V, Metal, and WebGPU
	inline int32_t MapSemanticToVSLocation(const char* semanticName, uint32_t semanticIndex = 0)
	{
		if (!semanticName || !semanticName[0])
			return -1;
		std::string s;
		for (const char* p = semanticName; *p; ++p)
		{
			char c = *p;
			if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
			s.push_back(c);
		}
		// Strip trailing digits if embedded in string (e.g. "TEXCOORD2")
		size_t digitPos = s.size();
		while (digitPos > 0 && s[digitPos - 1] >= '0' && s[digitPos - 1] <= '9')
			--digitPos;
		if (digitPos < s.size())
		{
			semanticIndex = static_cast<uint32_t>(std::stoul(s.substr(digitPos)));
			s.resize(digitPos);
		}

		if (s == "POSITION" || s == "POSITIONT" || s == "SV_POSITION") return 0;
		if (s == "BLENDWEIGHT") return 1;
		if (s == "BLENDINDICES") return 2;
		if (s == "NORMAL") return 3;
		if (s == "COLOR") return (semanticIndex == 0) ? 4 : 5;
		if (s == "TEXCOORD") return (semanticIndex < 8) ? static_cast<int32_t>(6 + semanticIndex) : 13;
		if (s == "TANGENT") return 14;
		if (s == "BINORMAL" || s == "PSIZE") return 15;
		return -1;
	}
}

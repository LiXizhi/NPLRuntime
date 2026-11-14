#include "WgslCodeGen.h"
#include "AstTextCodeGenCommon.h"

namespace ParaEngine
{
	namespace
	{
		std::mutex g_wgslAstMutex;
	}

	bool CWgslCodeGen::CompileStageWGSL(
		const std::string& hlslSource,
		const std::string& entryPoint,
		bool isVertex,
		const std::vector<FxSamplerDesc>& declaredSamplers,
		const std::vector<FxUniformParam>& declaredParams,
		FxCompiledStage& outStage,
		std::vector<FxUniformParam>& ioDiscoveredParams,
		std::string& outError)
	{
		std::lock_guard<std::mutex> lock(g_wgslAstMutex);

		Hlsl2Glsl_Initialize();
		ShHandle parser = Hlsl2Glsl_ConstructCompiler(isVertex ? EShLangVertex : EShLangFragment);
		if (!parser)
		{
			Hlsl2Glsl_Shutdown();
			outError = "Hlsl2Glsl_ConstructCompiler failed for WGSL_1_0";
			return false;
		}

		AstTextGenDetail::StructuredTextModuleBuilder builder(
			AstTextGenDetail::ETextLang::WGSL_1_0,
			isVertex,
			entryPoint,
			declaredSamplers,
			declaredParams);
		AstTextGenDetail::TextAstVisitorPayload payload{ &builder, &outStage, &ioDiscoveredParams };

		// opt = 0: WebGPU WGPUVertexFormat_Unorm8x4BGRA swizzles D3DCOLOR vertex attributes in hardware
		int ok = Hlsl2Glsl_ParseAST(parser, hlslSource.c_str(), ETargetGLSL_140, nullptr, 0, &AstTextGenDetail::TextAstVisitorCallback, &payload);
		if (!ok)
		{
			if (!builder.errorMsg.empty())
			{
				outError = builder.errorMsg;
			}
			else
			{
				const char* log = Hlsl2Glsl_GetInfoLog(parser);
				outError = log ? log : "Hlsl2Glsl_ParseAST failed for WGSL_1_0";
			}
			Hlsl2Glsl_DestructCompiler(parser);
			Hlsl2Glsl_Shutdown();
			return false;
		}

		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return true;
	}

	bool CWgslCodeGen::ValidateWgslSource(
		const std::vector<uint8_t>& wgslBytes,
		bool isVertex,
		std::string& outError)
	{
		if (wgslBytes.empty())
		{
			outError = "WGSL_1_0 source is empty";
			return false;
		}

		size_t len = wgslBytes.size();
		while (len > 0 && wgslBytes[len - 1] == 0)
			--len;
		for (size_t i = 0; i < len; ++i)
		{
			if (wgslBytes[i] == 0)
			{
				outError = "WGSL_1_0 source contains embedded NUL byte";
				return false;
			}
		}

		std::string src(reinterpret_cast<const char*>(wgslBytes.data()), len);
		if (!AstTextGenDetail::ValidateBalancedDelimiters(src, outError))
			return false;

		if (isVertex)
		{
			if (src.find("@vertex") == std::string::npos || src.find("fn vs_main(") == std::string::npos)
			{
				outError = "WGSL_1_0 vertex shader missing `@vertex fn vs_main(...)`";
				return false;
			}
			if (src.find("@builtin(position)") == std::string::npos)
			{
				outError = "WGSL_1_0 vertex shader missing `@builtin(position)` output";
				return false;
			}
			const char* illegalInVs[] = { "textureSample(", "textureSampleBias(", "dpdx(", "dpdy(", "fwidth(", "discard;" };
			for (const char* pat : illegalInVs)
			{
				if (src.find(pat) != std::string::npos)
				{
					outError = std::string("WGSL_1_0 vertex shader uses fragment-only construct: ") + pat;
					return false;
				}
			}
		}
		else
		{
			if (src.find("@fragment") == std::string::npos || src.find("fn fs_main(") == std::string::npos)
			{
				outError = "WGSL_1_0 fragment shader missing `@fragment fn fs_main(...)`";
				return false;
			}
		}

		// WGSL 1.0 forbids any identifier starting with double underscore `__`
		for (size_t i = 0; i < src.size();)
		{
			unsigned char ch = static_cast<unsigned char>(src[i]);
			if (std::isalpha(ch) || ch == '_')
			{
				size_t start = i;
				while (i < src.size() && (std::isalnum(static_cast<unsigned char>(src[i])) || src[i] == '_'))
					++i;
				if (i - start >= 2 && src[start] == '_' && src[start + 1] == '_')
				{
					outError = "WGSL_1_0 source contains reserved leading-double-underscore identifier: " + src.substr(start, i - start);
					return false;
				}
			}
			else
			{
				++i;
			}
		}

		const char* forbiddenHlsl[] = { "tex2D(", "texCUBE(", "tex3D(", "float4(", "float3(", "float2(" };
		for (const char* pat : forbiddenHlsl)
		{
			if (src.find(pat) != std::string::npos)
			{
				outError = std::string("WGSL_1_0 source contains untranslated HLSL syntax: ") + pat;
				return false;
			}
		}

		return true;
	}
} // namespace ParaEngine

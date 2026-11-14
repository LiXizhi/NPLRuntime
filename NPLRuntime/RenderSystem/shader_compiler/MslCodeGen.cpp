#include "MslCodeGen.h"
#include "AstTextCodeGenCommon.h"

namespace ParaEngine
{
	namespace
	{
		std::mutex g_mslAstMutex;
	}

	bool CMslCodeGen::CompileStageMSL(
		const std::string& hlslSource,
		const std::string& entryPoint,
		bool isVertex,
		const std::vector<FxSamplerDesc>& declaredSamplers,
		const std::vector<FxUniformParam>& declaredParams,
		FxCompiledStage& outStage,
		std::vector<FxUniformParam>& ioDiscoveredParams,
		std::string& outError)
	{
		std::lock_guard<std::mutex> lock(g_mslAstMutex);

		Hlsl2Glsl_Initialize();
		ShHandle parser = Hlsl2Glsl_ConstructCompiler(isVertex ? EShLangVertex : EShLangFragment);
		if (!parser)
		{
			Hlsl2Glsl_Shutdown();
			outError = "Hlsl2Glsl_ConstructCompiler failed for MSL_2_0";
			return false;
		}

		AstTextGenDetail::StructuredTextModuleBuilder builder(
			AstTextGenDetail::ETextLang::MSL_2_0,
			isVertex,
			entryPoint,
			declaredSamplers,
			declaredParams);
		AstTextGenDetail::TextAstVisitorPayload payload{ &builder, &outStage, &ioDiscoveredParams };

		// opt = 0: Metal MTLVertexFormatUChar4Normalized_BGRA swizzles D3DCOLOR vertex attributes in hardware
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
				outError = log ? log : "Hlsl2Glsl_ParseAST failed for MSL_2_0";
			}
			Hlsl2Glsl_DestructCompiler(parser);
			Hlsl2Glsl_Shutdown();
			return false;
		}

		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return true;
	}

	bool CMslCodeGen::ValidateMslSource(
		const std::vector<uint8_t>& mslBytes,
		bool isVertex,
		std::string& outError)
	{
		if (mslBytes.empty())
		{
			outError = "MSL_2_0 source is empty";
			return false;
		}

		size_t len = mslBytes.size();
		while (len > 0 && mslBytes[len - 1] == 0)
			--len;
		for (size_t i = 0; i < len; ++i)
		{
			if (mslBytes[i] == 0)
			{
				outError = "MSL_2_0 source contains embedded NUL byte";
				return false;
			}
		}

		std::string src(reinterpret_cast<const char*>(mslBytes.data()), len);
		if (src.find("#include <metal_stdlib>") == std::string::npos ||
			src.find("using namespace metal;") == std::string::npos)
		{
			outError = "MSL_2_0 source missing metal_stdlib header";
			return false;
		}

		if (!AstTextGenDetail::ValidateBalancedDelimiters(src, outError))
			return false;

		if (isVertex)
		{
			if (src.find("vertex ") == std::string::npos || src.find("vs_main(") == std::string::npos)
			{
				outError = "MSL_2_0 vertex shader missing `vertex ... vs_main(...)` entry point";
				return false;
			}
			if (src.find("[[position]]") == std::string::npos)
			{
				outError = "MSL_2_0 vertex shader missing `[[position]]` output";
				return false;
			}
			if (src.find("discard_fragment()") != std::string::npos)
			{
				outError = "MSL_2_0 vertex shader illegally calls discard_fragment()";
				return false;
			}
		}
		else
		{
			if (src.find("fragment ") == std::string::npos || src.find("fs_main(") == std::string::npos)
			{
				outError = "MSL_2_0 fragment shader missing `fragment ... fs_main(...)` entry point";
				return false;
			}
		}

		const char* forbiddenHlsl[] = { "tex2D(", "texCUBE(", "tex3D(", "tex2Dproj(", "tex2Dlod(" };
		for (const char* pat : forbiddenHlsl)
		{
			if (src.find(pat) != std::string::npos)
			{
				outError = std::string("MSL_2_0 source contains untranslated HLSL intrinsic: ") + pat;
				return false;
			}
		}

		return true;
	}
} // namespace ParaEngine

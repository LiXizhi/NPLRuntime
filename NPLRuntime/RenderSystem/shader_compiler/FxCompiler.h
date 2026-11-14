#pragma once

#include "FxParserV2.h"
#include "FxShaderTypes.h"
#include <string>
#include <vector>

namespace ParaEngine
{
	class CFxCompiler
	{
	public:
		static bool Compile(
			const std::string& rawSource,
			const std::string& sourceName,
			EShaderTarget target,
			FxCompiledEffect& outEffect,
			std::string& outError,
			FxIncludeCallback includeResolver = nullptr);

		static bool CompileParsed(
			const FxParsedDocument& doc,
			EShaderTarget target,
			FxCompiledEffect& outEffect,
			std::string& outError);

		static bool CompileMultiTargetPFX(
			const std::string& rawSource,
			const std::string& sourceName,
			const std::vector<EShaderTarget>& targets,
			std::vector<uint8_t>& outPfxBytes,
			std::string& outError,
			FxIncludeCallback includeResolver = nullptr);

		static bool ExtractTargetFromPFX(
			const void* pfxData,
			size_t pfxSize,
			EShaderTarget preferredTarget,
			FxCompiledEffect& outEffect);
	};
}

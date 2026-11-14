#pragma once

#include "FxShaderTypes.h"
#include <functional>
#include <string>
#include <vector>

namespace ParaEngine
{
	using FxIncludeCallback = std::function<bool(const std::string& includePath, const std::string& parentSourceName, std::string& outContent)>;

	struct FxParsedPass
	{
		std::string name;
		std::string vsProfile;
		std::string vsEntry;
		std::string psProfile;
		std::string psEntry;
		bool vsExplicitNull = false;
		bool psExplicitNull = false;
		std::vector<FxRenderStateAssignment> renderStates;
	};

	struct FxParsedTechnique
	{
		std::string name;
		std::vector<FxParsedPass> passes;
	};

	struct FxParsedDocument
	{
		std::string sourceName;
		std::string hlslCodeForD3D;
		std::string hlslCodeForDXC;
		std::string hlslCodeForGLSL;
		std::vector<FxUniformParam> params;
		std::vector<FxSamplerDesc> samplers;
		std::vector<FxParsedTechnique> techniques;
	};

	class CFxParserV2
	{
	public:
		static bool Parse(
			const std::string& rawSource,
			const std::string& sourceName,
			FxParsedDocument& outDoc,
			std::string& outError,
			FxIncludeCallback includeResolver = nullptr);

		static std::string ResolveIncludes(
			const std::string& rawSource,
			const std::string& sourceName,
			FxIncludeCallback includeResolver,
			int depth = 0);
	};
}

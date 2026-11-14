#pragma once

#include "FxShaderTypes.h"
#include <string>
#include <vector>

namespace ParaEngine
{
	class CMslCodeGen
	{
	public:
		static bool CompileStageMSL(
			const std::string& hlslSource,
			const std::string& entryPoint,
			bool isVertex,
			const std::vector<FxSamplerDesc>& declaredSamplers,
			const std::vector<FxUniformParam>& declaredParams,
			FxCompiledStage& outStage,
			std::vector<FxUniformParam>& ioDiscoveredParams,
			std::string& outError);

		// Structural validator verifying MSL 2.0 header, balanced delimiters,
		// vertex/fragment entry point signature, attribute/buffer/texture/sampler bindings,
		// and absence of untranslated HLSL constructs.
		static bool ValidateMslSource(
			const std::vector<uint8_t>& mslBytes,
			bool isVertex,
			std::string& outError);
	};
}

#pragma once

#include "FxShaderTypes.h"
#include <string>
#include <vector>

namespace ParaEngine
{
	class CWgslCodeGen
	{
	public:
		static bool CompileStageWGSL(
			const std::string& hlslSource,
			const std::string& entryPoint,
			bool isVertex,
			const std::vector<FxSamplerDesc>& declaredSamplers,
			const std::vector<FxUniformParam>& declaredParams,
			FxCompiledStage& outStage,
			std::vector<FxUniformParam>& ioDiscoveredParams,
			std::string& outError);

		// Structural validator verifying WGSL 1.0 syntax invariants:
		// balanced braces/parens/brackets, @vertex/@fragment entry attribute,
		// @group/@binding layout declarations, absence of reserved '__' identifiers,
		// and absence of untranslated HLSL constructs.
		static bool ValidateWgslSource(
			const std::vector<uint8_t>& wgslBytes,
			bool isVertex,
			std::string& outError);
	};
}

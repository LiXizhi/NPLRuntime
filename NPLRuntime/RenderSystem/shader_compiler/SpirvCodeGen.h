#pragma once

#include "FxShaderTypes.h"
#include <string>
#include <vector>

namespace ParaEngine
{
	class CSpirvCodeGen
	{
	public:
		static bool CompileStageSPIRV(
			const std::string& hlslSource,
			const std::string& entryPoint,
			bool isVertex,
			const std::vector<FxSamplerDesc>& declaredSamplers,
			const std::vector<FxUniformParam>& declaredParams,
			FxCompiledStage& outStage,
			std::vector<FxUniformParam>& ioDiscoveredParams,
			std::string& outError);

		// Lightweight structural validator checking SPIR-V 1.0 header, word bounds, ID bounds,
		// section ordering, OpEntryPoint, and basic-block terminators.
		static bool ValidateSpirvBinary(
			const std::vector<uint8_t>& spirvBytes,
			std::string& outError);
	};
}

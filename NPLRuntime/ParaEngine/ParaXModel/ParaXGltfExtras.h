#pragma once

#include "ParaXModel.h"
#include "json/json.h"
#include <string>

namespace ParaEngine
{
	class ParaXGltfExtras
	{
	public:
		/** Extract raw JSON root from a .gltf text buffer or a .glb binary buffer (chunk 0 JSON). */
		static bool ExtractGltfJson(const char* buffer, int nSize, Json::Value& outRoot);

		/** Apply extras.parax and/or node.extras particle & ribbon emitters onto an already-loaded CParaXModel. */
		static bool ApplyExtrasToModel(const Json::Value& gltfRoot, CParaXModel* pMesh);
		static bool ApplyExtrasFromBuffer(const char* buffer, int nSize, CParaXModel* pMesh);

		/** Parse a standalone pure-emitter .glb/.gltf (even when mesh primitives are empty) into a CParaXModel. */
		static CParaXModel* ParseStandaloneModelFromBuffer(const char* buffer, int nSize);

		/** Populate extras.parax JSON from a CParaXModel's textures, animations, bones, particleSystems, and ribbons. */
		static bool ExportExtrasFromModel(CParaXModel* pMesh, Json::Value& outParaxExtras);
	};
}

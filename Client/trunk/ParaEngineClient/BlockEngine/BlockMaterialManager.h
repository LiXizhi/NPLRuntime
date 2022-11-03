#pragma once

#include "AssetManager.h"
#include "BlockMaterial.h"

namespace ParaEngine
{
	/** manage all block materials. */
	class CBlockMaterialManager : public AssetManager<CBlockMaterial>
	{
	public:
		CBlockMaterialManager();
		~CBlockMaterialManager();
	};
}
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

		int32_t GetNextMaterialID() { return ++m_nextMaterialId; }

		CBlockMaterial* CreateBlockMaterial();
		CBlockMaterial* GetBlockMaterialByID(int32_t id);

		
		AssetItemsSet_t::iterator begin() { return m_items.begin(); }
		AssetItemsSet_t::iterator end() { return m_items.end(); }


		virtual void Cleanup()
		{
			AssetManager::Cleanup();
			m_nextMaterialId = 0;
		}
		
	private:
		int32_t m_nextMaterialId;
	};
}
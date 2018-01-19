#pragma once

#include "AssetManager.h"
#include "SequenceEntity.h"

namespace ParaEngine
{
	/** a manger of sequences */
	class CSequenceManager : public AssetManager<SequenceEntity>
	{
	public:
		CSequenceManager();
		~CSequenceManager();
	};
}

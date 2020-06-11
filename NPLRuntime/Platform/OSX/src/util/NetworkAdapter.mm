#include "ParaEngine.h"
#include "util/NetworkAdapter.h"

namespace ParaEngine {
	void NetworkAdapter::Init()
	{
		if (m_bInitialized)
			return;
		m_bInitialized = true;
	}
} // namespace ParaEngine


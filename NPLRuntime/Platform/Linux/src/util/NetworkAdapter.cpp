#include "ParaEngine.h"
#include "util/NetworkAdapter.h"

namespace ParaEngine {
	void NetworkAdapter::Init()
	{
		if (m_bInitialized)
			return;
		m_bInitialized = true;
	}
	const std::string& NetworkAdapter::GetMaxMacAddress()
	{
		static std::string s_str = "";
		return s_str;
	}

	const std::string& NetworkAdapter::GetMaxIPAddress()
	{
		static std::string s_str = "";
		return s_str;
	}
} // namespace ParaEngine


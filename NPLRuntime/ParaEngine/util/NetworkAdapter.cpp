#include "ParaEngine.h"
#include "NetworkAdapter.h"

namespace ParaEngine
{

	NetworkAdapter::NetworkAdapter() :m_pNextAdapter(NULL), m_bInitialized(false)
	{
		Init();
	}
	NetworkAdapter::~NetworkAdapter()
	{

	}

	NetworkAdapter* NetworkAdapter::GetInstance()
	{
		static NetworkAdapter s_singleton;
		return &s_singleton;
	}

//	const std::string& NetworkAdapter::GetMaxMacAddress()
//	{
//		return m_sMacAddress;
//	}

//	const std::string& NetworkAdapter::GetMaxIPAddress()
//	{
//		return m_sIpAddress;
//	}
}

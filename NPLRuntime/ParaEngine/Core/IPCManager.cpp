//-----------------------------------------------------------------------------
// Class:	IPC manager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2010.4.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef PARAENGINE_CLIENT
#include "IPCManager.h"

using namespace ParaEngine;

ParaEngine::CIPCManager::CIPCManager()
{
	
}

ParaEngine::CIPCManager::~CIPCManager()
{
}

CIPCManager* ParaEngine::CIPCManager::GetInstance()
{
	static CIPCManager g_sington;
	return &g_sington;
}

CInterprocessQueuePtr ParaEngine::CIPCManager::CreateGetQueue( const std::string& name, IPQueueUsageEnum nUsage )
{
	ParaEngine::Lock lock_(m_mutex);

	ipc_queue_map_t::iterator itCur =  m_queues.find(name);
	if(itCur != m_queues.end())
	{
		return itCur->second;
	}
	else
	{
		CInterprocessQueuePtr pWatcher(new CInterprocessQueue(name.c_str(), nUsage));
		m_queues[name] = pWatcher;
		return pWatcher;
	}
}


void ParaEngine::CIPCManager::RemoveQueue( const std::string& name )
{
	ParaEngine::Lock lock_(m_mutex);
	ipc_queue_map_t::iterator itCur =  m_queues.find(name);
	if(itCur != m_queues.end())
	{
		if(itCur->second.use_count() == 1)
		{
			m_queues.erase(itCur);
		}
		else
		{
			OUTPUT_LOG("warning: CIPCManager::DeleteQueue can not delete %s because there is some external references\n", name.c_str());
		}
	}
}

void ParaEngine::CIPCManager::Clear()
{
	ParaEngine::Lock lock_(m_mutex);
	m_queues.clear();
}
#endif
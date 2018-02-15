#pragma once
#ifdef PARAENGINE_CLIENT
#include "InterprocessQueue.hpp"
#include "util/mutex.h"
#include <queue>

namespace ParaEngine
{
	typedef boost::shared_ptr<CInterprocessQueue> CInterprocessQueuePtr;

	/** file system watcher service. this is a singleton. */
	class CIPCManager
	{
	public:
		typedef std::map<std::string, CInterprocessQueuePtr > ipc_queue_map_t;
		
		CIPCManager();
		~CIPCManager();

		static CIPCManager* GetInstance();

		/** clear all system watcher references that is created by GetQueue() */
		void Clear();

		/** create get a watcher by its name. 
		* it is good practice to use the directory name as watcher name, since it will reuse it as much as possible. 
		*/
		CInterprocessQueuePtr CreateGetQueue(const std::string& name, IPQueueUsageEnum nUsage = IPQU_open_or_create);

		/** delete a watcher, it will no longer receive callbacks. 
		* @please note that if someone else still keeps a pointer to the directory watcher, it will not be deleted. 
		*/
		void RemoveQueue(const std::string& name);

	private:
		ipc_queue_map_t m_queues;
		ParaEngine::mutex m_mutex;
	};

}

#endif
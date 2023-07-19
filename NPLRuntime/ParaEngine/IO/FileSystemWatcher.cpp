//-----------------------------------------------------------------------------
// Class:	File System Watcher
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2010.3.29
// Desc: modified on https://github.com/berkus/dir_monitor
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef EMSCRIPTEN

#if !defined(PARAENGINE_MOBILE)


#include "CSingleton.h"
#include "FileSystemWatcher.h"
// define this to enable recursive directory watch by default, default to false under windows.
#define RECUSRIVE_DIRECTORY_MONITOR TRUE
#include "dirmonitor/dir_monitor.hpp"
#include <boost/bind.hpp>

using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
//
// CFileSystemWatcherService
//
//////////////////////////////////////////////////////////////////////////

ParaEngine::CFileSystemWatcherService::CFileSystemWatcherService()
 : m_io_service(new boost::asio::io_service()), m_io_service_work(new boost::asio::io_service::work(*m_io_service)), m_bIsStarted(false)
{

}

ParaEngine::CFileSystemWatcherService::~CFileSystemWatcherService()
{
	Clear();
}

CFileSystemWatcherService* ParaEngine::CFileSystemWatcherService::GetInstance()
{
	return CAppSingleton<CFileSystemWatcherService>::GetInstance();
}

CFileSystemWatcherPtr ParaEngine::CFileSystemWatcherService::GetDirWatcher( const std::string& name )
{
	file_watcher_map_t::iterator itCur =  m_file_watchers.find(name);
	if(itCur != m_file_watchers.end())
	{
		return itCur->second;
	}
	else
	{
		CFileSystemWatcherPtr pWatcher(new CFileSystemWatcher(name));
		m_file_watchers[name] = pWatcher;
		return pWatcher;
	}
}

int ParaEngine::CFileSystemWatcherService::DispatchEvents()
{
	int nCount = 0;
	if(IsStarted())
	{
		file_watcher_map_t::iterator itCur, itEnd = m_file_watchers.end();
		for (itCur = m_file_watchers.begin(); itCur != itEnd; ++itCur)
		{
			nCount += itCur->second->DispatchEvents();
		}
	}
	return nCount;
}

void ParaEngine::CFileSystemWatcherService::DeleteDirWatcher( const std::string& name )
{
	file_watcher_map_t::iterator itCur =  m_file_watchers.find(name);
	if(itCur != m_file_watchers.end())
	{
		if(itCur->second.use_count() == 1)
		{
			itCur->second->Destroy();
			// sleep some time for watcher to exit.
			// TODO: poll for a signal, rather than this
			SLEEP(100);
			m_file_watchers.erase(itCur);
		}
		else
		{
			OUTPUT_LOG("warning: CFileSystemWatcherService::DeleteDirWatcher can not delete %s because there is some external references\n", name.c_str());
		}
	}
}

void ParaEngine::CFileSystemWatcherService::Clear()
{
	try
	{
		if(m_io_service_work)
		{
			for (auto& watcher : m_file_watchers)
			{
				if (watcher.second.use_count() != 1)
				{
					OUTPUT_LOG("error: CFileSystemWatcherService has external references\n", watcher.first.c_str());
				}
				watcher.second->Destroy();
			}
			m_io_service_work.reset();

			if(m_work_thread)
			{
				m_work_thread->join();
			}
			m_io_service.reset();
			m_file_watchers.clear();
		}
	}
	catch (...)
	{
		OUTPUT_LOG("warning: CFileSystemWatcherService service exit\n");
	}
}


int ParaEngine::CFileSystemWatcherService::fileWatcherThreadMain()
{
	try
	{
		if (m_io_service)
			return (int)(m_io_service->run());
	}
	catch (...)
	{
		OUTPUT_LOG("error: directory monitor throwed an exception. May be it is not supported on your platform.\n");
	}
	return 0;
}


bool ParaEngine::CFileSystemWatcherService::Start()
{
	if(!IsStarted())
	{
		m_bIsStarted = true;
		if(!m_work_thread)
		{
			// m_work_thread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, m_io_service.get())));
			m_work_thread.reset(new boost::thread(boost::bind(&CFileSystemWatcherService::fileWatcherThreadMain, this)));
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// CFileSystemWatcher
//
//////////////////////////////////////////////////////////////////////////

ParaEngine::CFileSystemWatcher::CFileSystemWatcher(const std::string& filename)
	: m_bDispatchInMainThread(true), m_monitor_imp(NULL)
{
	m_monitor_imp = new boost::asio::dir_monitor(CFileSystemWatcherService::GetInstance()->GetIOService());
	((boost::asio::dir_monitor*)m_monitor_imp)->async_monitor(boost::bind(&ParaEngine::CFileSystemWatcher::FileHandler, this, _1, _2));
	CFileSystemWatcherService::GetInstance()->Start();
	SetName(filename);
	OUTPUT_LOG("FileSystemWatcher: %s created\n", filename.c_str());
}

ParaEngine::CFileSystemWatcher::CFileSystemWatcher()
	:CFileSystemWatcher("default")
{

}

ParaEngine::CFileSystemWatcher::~CFileSystemWatcher()
{
	OUTPUT_LOG("FileSystemWatcher removed\n");
}

void ParaEngine::CFileSystemWatcher::FileHandler( const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev )
{
	if(!ec)
	{
		if(IsDispatchInMainThread())
		{
			m_file_event(ev);
            {
                ParaEngine::Lock lock_(m_mutex);
                if (m_monitor_imp)
                    // continuously polling
                    ((boost::asio::dir_monitor*)m_monitor_imp)->async_monitor(boost::bind(&ParaEngine::CFileSystemWatcher::FileHandler, this, _1, _2));
            }
		}
		else
		{
			ParaEngine::Lock lock_(m_mutex);
			m_msg_queue.push(ev);
            
            if (m_monitor_imp)
                // continuously polling
                ((boost::asio::dir_monitor*)m_monitor_imp)->async_monitor(boost::bind(&ParaEngine::CFileSystemWatcher::FileHandler, this, _1, _2));
		}
		
	}
	else
	{
		std::string sError = ec.message();
		OUTPUT_LOG("warning: in ParaEngine::CFileSystemWatcher::FileHandler. msg is %s\n", sError.c_str());
	}
}

void ParaEngine::CFileSystemWatcher::SetDispatchInMainThread( bool bMainThread )
{
	m_bDispatchInMainThread = bMainThread;
}

bool ParaEngine::CFileSystemWatcher::IsDispatchInMainThread()
{
	return m_bDispatchInMainThread;
}

int ParaEngine::CFileSystemWatcher::DispatchEvents()
{
	int nCount = 0;

	if(IsDispatchInMainThread())
	{
		ParaEngine::Lock lock_(m_mutex);
		while (!m_msg_queue.empty())
		{
			m_file_event(m_msg_queue.front());
			m_msg_queue.pop();
			nCount ++;
		}
	}
	else
	{
		while (!m_msg_queue.empty()){
			m_msg_queue.pop();
		}
	}
	return nCount;
}

CFileSystemWatcher::FileSystemEvent_Connection_t ParaEngine::CFileSystemWatcher::AddEventCallback( FileSystemEvent_t::slot_type callback )
{
	return m_file_event.connect(callback);
}

bool ParaEngine::CFileSystemWatcher::add_directory( const std::string &dirname )
{
	bool bRes = true;
	try
	{
		((boost::asio::dir_monitor*)m_monitor_imp)->add_directory(dirname);
		OUTPUT_LOG("FileSystemWatcher %s begins monitoring all files in %s\n", m_name.c_str(), dirname.c_str());
	}
	catch (...)
	{
		OUTPUT_LOG("warning: failed adding dir %s to monitor \n", dirname.c_str());
		bRes = false;
	}
	return bRes;
}

bool ParaEngine::CFileSystemWatcher::remove_directory( const std::string &dirname )
{
	bool bRes = true;
	try
	{
		((boost::asio::dir_monitor*)m_monitor_imp)->remove_directory(dirname);
		OUTPUT_LOG("FileSystemWatcher %s stops monitoring in %s\n", m_name.c_str(), dirname.c_str());
	}
	catch (...)
	{
		OUTPUT_LOG("warning: failed removing dir %s to monitor \n", dirname.c_str());
		bRes = false;
	}
	return bRes;
}

const std::string& ParaEngine::CFileSystemWatcher::GetName() const
{
	return m_name;
}

void ParaEngine::CFileSystemWatcher::SetName(const std::string& val)
{
	m_name = val;
}

void ParaEngine::CFileSystemWatcher::Destroy()
{
    ParaEngine::Lock lock_(m_mutex);
	boost::asio::dir_monitor* pObj = (boost::asio::dir_monitor*)m_monitor_imp;
	SAFE_DELETE(pObj);
	m_monitor_imp = NULL;
}

#endif

#endif
//-----------------------------------------------------------------------------
// Class:	Auto Updater Client
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.3.8
// Desc: Emulation of the NPL runtime so that it can communicate with simple C++ based NPL activation dll, such as AutoUpdater.dll
//-----------------------------------------------------------------------------
#include "config.h"
#include "AutoUpdaterClient.h"

using namespace ParaEngine;
using namespace NPL;

CAutoUpdaterClient::CAutoUpdaterClient()
: m_current_msg_length(0), m_current_msg(NULL), m_progress_status(AUP_NOT_STARTED), m_pUpdaterCallback(NULL)
{
	m_name = "main";
	// TODO: use a different one per web page. 
	m_session_dir = "web";
}

CAutoUpdaterClient::~CAutoUpdaterClient()
{
	ExitUpdater();
}

const std::string& CAutoUpdaterClient::GetName() const
{
	return m_name;
}

int CAutoUpdaterClient::activate( const char * sNPLFilename, const char* sCode, int nCodeLength/*=0*/, int priority/*=2*/, int reliability/*=4*/ )
{
	NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sCode, nCodeLength);
	OnMessageCallback(tabMsg);

	return NPL::NPL_OK;
}

NPL::NPLReturnCode CAutoUpdaterClient::Activate_async( const std::string & filepath, const char * code /*= NULL*/,int nLength/*=0*/, int priority/*=0*/ )
{
	return NPL::NPL_OK;
}

NPL::NPLReturnCode CAutoUpdaterClient::Activate_async( NPLMessage_ptr& msg, int priority/*=0*/ )
{
	return NPL::NPL_OK;
}

NPL::NPLReturnCode CAutoUpdaterClient::ActivateLocal( const char* filepath, const char * code /*= NULL*/,int nLength/*=0*/, int priority/*=0*/ )
{
	return NPL::NPL_OK;
}

NPL::NPLReturnCode CAutoUpdaterClient::SendMessage( NPLMessage_ptr& msg, int priority/*=0*/ )
{
	return NPL::NPL_OK;
}

const char* CAutoUpdaterClient::GetCurrentMsg()
{
	return m_current_msg;
}

int CAutoUpdaterClient::GetCurrentMsgLength()
{
	return m_current_msg_length;
}

INPLRuntime* CAutoUpdaterClient::GetNPLRuntime()
{
	return NULL;
}

void CAutoUpdaterClient::WriteLog( const char* text, int nTextLen/*=0*/, int nLogType /*= 0*/ )
{
	OutputDebugStringA(text);
}

//////////////////////////////////////////////////////////////////////////
//
// Auto Updater Client APIs
//
//////////////////////////////////////////////////////////////////////////

void CAutoUpdaterClient::ActivateAutoUpdater( const std::string& sMsg )
{
	if(m_auto_updater_plugin.IsValid())
	{
		m_current_msg = sMsg.c_str();
		m_current_msg_length = (int)(sMsg.size());

		m_auto_updater_plugin.Activate(ParaEngine::PluginActType_STATE, this);
	}
}

int CAutoUpdaterClient::BeginUpdate( const std::string& curVersion, const char* sUpdaterName )
{
	if(sUpdaterName){
		m_session_dir = sUpdaterName;
	}

	if(m_updater_plugin_path.empty())
	{
#ifdef _DEBUG
		// autoupdater_filename = "AutoUpdater_d.dll";
		m_updater_plugin_path = "D:\\lxzsrc\\ParaEngine\\ParaWorld\\AutoUpdater_d.dll";
#else
		m_updater_plugin_path = "AutoUpdater.dll";
#endif
	}

	std::string autoupdater_filename_mem = "autoupdater.mem.dll";

	if(!CopyFileA(m_updater_plugin_path.c_str(), autoupdater_filename_mem.c_str(), FALSE))
	{
		WriteLog("Error: failed copy file to autoupdater.mem.dll\n");
	}

	m_auto_updater_plugin.Init(autoupdater_filename_mem.c_str());

	if(!m_auto_updater_plugin.IsValid())
	{
		return E_FAIL;
	}

	NPLInterface::NPLObjectProxy msg;
	msg["cmd"] = "update";
	msg["curver"] = curVersion;
	msg["lastver"] = "";
	msg["callback"] = "(gl)";
	msg["updatedir"] = m_session_dir;
	if(!m_updateurl.empty())
	{
		msg["updateurl"] = m_updateurl;
	}

	std::string sMsg;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, sMsg);

	ActivateAutoUpdater(sMsg);
	OnProgress(AUP_STARTED, "AutoUpdater: start");
	return S_OK;
}


void CAutoUpdaterClient::OnProgress( AutoUpdaterProgress eventType, const char* msg/*=NULL*/, int finishcount/*=0*/, int allcount/*=0*/ )
{
	{
		ParaEngine::Mutex::ScopedLock lock(m_mutex);

		m_progress_status = eventType;

		if(msg)
		{
			OutputDebugStringA(msg);
		}
		if(eventType  == AUP_PROGRESS)
		{
			char tmp[256];
			_snprintf(tmp, 256, "%d/%d\n", finishcount, allcount);
			OutputDebugStringA(tmp);
		}

		m_msgs.push(ProgressMessage(eventType, msg, finishcount, allcount));
	}
	m_semaphore.post();
}

void CAutoUpdaterClient::OnMessageCallback( NPLInterface::NPLObjectProxy& msg )
{
	std::string ischanged = msg["ischanged"];
	std::string isfinished = msg["isfinished"];
	double finishcount = msg["finishcount"];
	double allcount = msg["allcount"];
	if(ischanged == "no")
	{
		// nothing changed, go on loading the game. 
		OnProgress(AUP_NOCHANGE);
		OutputDebugString(_T("AutoUpdater: No change is detected\n"));
	}
	else if(ischanged == "unknown")
	{
		OnProgress(AUP_UNKNOWN, "AutoUpdater: We are not sure if you have the lastest version");
		// MessageBox(NULL, _T("无法确认您是否在使用最新的版本， 请到官方网站重新下载客户端"), _T("更新消息"), MB_OK);
	}
	else if(isfinished == "yes")
	{
		if(allcount == finishcount)
		{
			OnProgress(AUP_PROGRESS, "AutoUpdater: download completed, now applying patch...\n", (int)finishcount, (int)allcount);
			// log("Core ParaEngine Client update completed!\n");
			NPLInterface::NPLObjectProxy msgRestart;
			msgRestart["cmd"] = "applypatch";
			msgRestart["updatedir"] = m_session_dir;
			msgRestart["version"] = msg["version"];
			msgRestart["filelist"] = msg["filelist"];

			std::string sMsg;
			NPLInterface::NPLHelper::NPLTableToString("msg", msgRestart, sMsg);
			// Fixed a bug: instead of apply patch in the calling thread, we will send AUP_DO_APPLY_PATCH message and let the host to apply the patch. 
			m_sApplyPatchMsg = sMsg;
			
			OnProgress(AUP_DO_APPLY_PATCH, "please apply patch.\n");
		}
		else
		{
			OnProgress(AUP_BROKENFILE, "AutoUpdater: file is broken\n");
		}
	}
	else if(isfinished == "error")
	{
		OnProgress(AUP_ERROR, "AutoUpdater: unknown error\n");
	}
	else if(finishcount>=0 && allcount>0)
	{
		OnProgress(AUP_PROGRESS, "AutoUpdater: progress", (int)finishcount, (int)allcount);

		float fPercentage = (float)finishcount/(float)allcount;
		int percentage = int(100*fPercentage);
	}
}

NPL::CAutoUpdaterClient::AutoUpdaterProgress NPL::CAutoUpdaterClient::WaitForCompletion(IAutoUpdaterCallback* pCallback)
{
	m_pUpdaterCallback = pCallback;

	ParaEngine::Mutex::ScopedLock lock(m_mutex);
	while(m_progress_status != AUP_COMPLETED && 
		m_progress_status != AUP_NOCHANGE && 
		m_progress_status != AUP_UNKNOWN && 
		m_progress_status != AUP_ERROR)
	{
		lock.unlock();
		m_semaphore.wait();
		lock.lock();

		while (!m_msgs.empty())
		{
			if(m_pUpdaterCallback)
			{
				m_pUpdaterCallback->OnProgress(m_msgs.front());
			}
			m_msgs.pop();
		}
	}
	return m_progress_status;
}

bool NPL::CAutoUpdaterClient::GetUpdaterMessage( ProgressMessage& msg )
{
	ParaEngine::Mutex::ScopedLock lock(m_mutex);
	while (m_msgs.empty())
	{
		lock.unlock();
		m_semaphore.wait();
		lock.lock();
		if ( !m_msgs.empty())
		{
			msg = m_msgs.front();
			m_msgs.pop();
			return true;
		}
	}
	msg = m_msgs.front();
	m_msgs.pop();
	return true;
}

bool NPL::CAutoUpdaterClient::PeekUpdaterMessage( ProgressMessage& msg )
{
	ParaEngine::Mutex::ScopedLock lock(m_mutex);
	if (!m_msgs.empty())
	{
		msg = m_msgs.front();
		m_msgs.pop();
		return true;
	}
	return false;
}

void NPL::CAutoUpdaterClient::ExitUpdater()
{
	if(m_auto_updater_plugin.IsValid())
	{
		NPLInterface::NPLObjectProxy msg;
		msg["cmd"] = "exit";
		std::string sMsg;
		NPLInterface::NPLHelper::NPLTableToString("msg", msg, sMsg);
		ActivateAutoUpdater(sMsg);

		m_auto_updater_plugin.UnLoad();
	}
}

void NPL::CAutoUpdaterClient::SetUpdaterDllPath( const char* sDllPath /*= NULL*/ )
{
	if(sDllPath)
	{
		m_updater_plugin_path = sDllPath;
	}
}

void NPL::CAutoUpdaterClient::SetUpdateUrl( const std::string& updateurl )
{
	m_updateurl = updateurl;
}

void NPL::CAutoUpdaterClient::ApplyPatch()
{
	if(!m_sApplyPatchMsg.empty())
	{
		ActivateAutoUpdater(m_sApplyPatchMsg);
		OnProgress(AUP_COMPLETED, "AutoUpdater: Updating finished\n");
	}
}

void NPL::CAutoUpdaterClient::RegisterFile(const char* sFilename, INPLActivationFile* pFileHandler /*= NULL*/)
{

}

void NPL::CAutoUpdaterClient::call(const char * sNPLFilename, const char* sCode, int nCodeLength /*= 0*/)
{

}

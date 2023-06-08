//-----------------------------------------------------------------------------
// Class:	NPL Net client 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.11.28
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef HAS_JABBER_CLIENT
#include "NPLJabberClient.h"
#endif
#include "AssetEntity.h"
#include "NPLHelper.h"
#include "AISimulator.h"
#include "util/HttpUtility.h"
#include "NPLNetClient.h"

#ifdef PARAENGINE_CLIENT
#include "ParaWorldAsset.h"
#include "memdebug.h"
#endif

/**@def default download request pool size */
#define DEFAULT_DOWNLOAD_POOL_SIZE		2
/**@def default REST request pool size */
#define DEFAULT_REST_POOL_SIZE		5
/**@def default web request pool size */
#define DEFAULT_WEB_POOL_SIZE		5

using namespace ParaEngine;

#ifndef EMSCRIPTEN_SINGLE_THREAD

static CNPLNetClient* g_pNPLNetClient;

ParaEngine::CNPLNetClient::CNPLNetClient()
: m_dispatcher_io_service()
{
	g_pNPLNetClient = this;
	m_work_lifetime.reset(new boost::asio::io_service::work(m_dispatcher_io_service));
	m_dispatcherThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &m_dispatcher_io_service)));
}

ParaEngine::CNPLNetClient::~CNPLNetClient()
{
	g_pNPLNetClient = NULL;
	Cleanup();
}

void ParaEngine::CNPLNetClient::DeleteThis()
{
	delete this;
}


void ParaEngine::CNPLNetClient::Cleanup()
{
	m_work_lifetime.reset();
#ifdef HAS_JABBER_CLIENT
	// disconnect all jabber clients
	{
		// try closing everything gracefully by sending the close message to server. 
		std::map <std::string, CNPLJabberClient*>::iterator itCur, itEnd = m_jabberClients.end();
		for(itCur = m_jabberClients.begin();itCur!=itEnd; ++itCur)
		{
			itCur->second->Close();
		}
	}
#endif
	if(m_dispatcherThread.get() != 0)
	{
		m_dispatcherThread->timed_join(boost::posix_time::millisec(10000));
		//m_dispatcherThread->join();
	}
#ifdef HAS_JABBER_CLIENT
	{
		std::map <std::string, CNPLJabberClient*>::iterator itCur, itEnd = m_jabberClients.end();
		for(itCur = m_jabberClients.begin();itCur!=itEnd; ++itCur)
		{
			delete (itCur->second);
		}
		m_jabberClients.clear();
	}
#endif
	// delete all pools
	{
		std::map <std::string, CRequestTaskPool*>::iterator itCur, itEnd = m_request_pools.end();
		for(itCur = m_request_pools.begin();itCur!=itEnd; ++itCur)
		{
			delete (itCur->second);
		}
		m_request_pools.clear();
	}
}

INPLWebService* ParaEngine::CNPLNetClient::GetWebService( const char* sURL )
{
	return NULL;
}

INPLWebService* ParaEngine::CNPLNetClient::OpenWebService( const char* sURL, const char* sUserName, const char* sPassword, const char* sDomain )
{
	return NULL;
}

bool ParaEngine::CNPLNetClient::CloseWebService( const char* sURL )
{
	return false;
}

INPLJabberClient* ParaEngine::CNPLNetClient::GetJabberClient( const char* sJID )
{
#ifdef HAS_JABBER_CLIENT
	if(sJID == NULL)
		return NULL;
	std::map <std::string, CNPLJabberClient*>::iterator iter = m_jabberClients.find(sJID);
	if(iter!=m_jabberClients.end())
	{
		return (INPLJabberClient*)(iter->second);
	}
#endif
	return NULL;
}

INPLJabberClient* ParaEngine::CNPLNetClient::CreateJabberClient( const char* sJID )
{
#ifdef HAS_JABBER_CLIENT
	if(sJID == NULL)
		return NULL;

	INPLJabberClient* jc = NULL;
	if((jc=GetJabberClient(sJID))!=NULL)
		return jc;
	try
	{
		// create the client
		CNPLJabberClient* jc_imp = new CNPLJabberClient(m_dispatcher_io_service, sJID);
		m_jabberClients[sJID] = jc_imp;

		jc = (INPLJabberClient*)jc_imp;
	}
	catch (...)
	{
		OUTPUT_LOG("NPL_JC: error occurred when Creating Jabber Client: jid: %s\n", sJID);
	}
	return jc;
#else
	return NULL;
#endif
}

bool ParaEngine::CNPLNetClient::CloseJabberClient( const char* sJID )
{
#ifdef HAS_JABBER_CLIENT
	if(sJID)
	{
		if(sJID[0] != '\0')
		{
			// close the given client
			std::map <std::string, CNPLJabberClient*>::iterator iter = m_jabberClients.find(sJID);
			if(iter!=m_jabberClients.end())
			{
				delete iter->second;
				m_jabberClients.erase(iter);
			}
		}
		else
		{
			// close all client
			std::map <std::string, CNPLJabberClient*>::iterator itCur, itEnd = m_jabberClients.end();
			for(itCur = m_jabberClients.begin();itCur!=itEnd; ++itCur)
			{
				itCur->second->ResetAllEventListeners();
				delete (itCur->second);
			}
			m_jabberClients.clear();
		}
	}
#endif
	return true;
}

void ParaEngine::CNPLNetClient::AsyncDownload( const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName )
{

}

void ParaEngine::CNPLNetClient::CancelDownload( const char* DownloaderName )
{
	OUTPUT_LOG("warning: NPL.CancelDownload() is not implemented yet\n");
}

int ParaEngine::CNPLNetClient::Download( const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName )
{
	OUTPUT_LOG("warning: NPL.Download() is not implemented yet. Use NPL.AsyncDownload() instead\n");
	return 0;
}

int ParaEngine::CNPLNetClient::ProcessResults()
{
#ifdef HAS_JABBER_CLIENT
	{
		std::map <std::string, CNPLJabberClient*>::iterator itCur, itEnd = m_jabberClients.end();
		for(itCur = m_jabberClients.begin();itCur!=itEnd; ++itCur)
		{
			(itCur->second)->ProcessMessage();
		}
	}
#endif
	return 0;
}

int ParaEngine::CNPLNetClient::ProcessDownloaderResults()
{
	return 0;
}

CRequestTaskPool* ParaEngine::CNPLNetClient::CreateGetRequestTaskPool( const char* sPoolName /*= NULL*/ )
{
	string strPoolName;
	if(sPoolName)
	{
		strPoolName = sPoolName;
	}
	std::map <std::string, CRequestTaskPool*>::iterator iter = m_request_pools.find(strPoolName);
	CRequestTaskPool* pTaskPool = NULL;
	if(iter == m_request_pools.end())
	{
		if(m_request_pools.size() > 20)
		{
			OUTPUT_LOG("warning: max number of pool numbers reached. %s is not created. AppendURLRequest ignored. \n", strPoolName.c_str());
			return NULL;
		}
		pTaskPool = new CRequestTaskPool();
		if(strPoolName == "d")
		{
			pTaskPool->SetMaxTaskSlotsCount(DEFAULT_DOWNLOAD_POOL_SIZE);
		}
		else if(strPoolName == "r")
		{
			pTaskPool->SetMaxTaskSlotsCount(DEFAULT_REST_POOL_SIZE);
		}
		else if(strPoolName == "w")
		{
			pTaskPool->SetMaxTaskSlotsCount(DEFAULT_WEB_POOL_SIZE);
		}
		m_request_pools[strPoolName] = pTaskPool;
	}
	else
	{
		pTaskPool = iter->second;
	}
	return pTaskPool;
}

bool ParaEngine::CNPLNetClient::AppendURLRequest( CURLRequestTask* pUrlTask, const char* sPoolName )
{
	CRequestTaskPool* pTaskPool =CreateGetRequestTaskPool(sPoolName);
	if(pTaskPool)
	{
		return pTaskPool->AppendURLRequest(pUrlTask);
	}
	return false;
}

bool ParaEngine::CNPLNetClient::ChangeRequestPoolSize( const char* sPoolName, int nCount )
{
	CRequestTaskPool* pTaskPool =CreateGetRequestTaskPool(sPoolName);
	if(pTaskPool)
	{
		pTaskPool->SetMaxTaskSlotsCount(nCount);
		return true;
	}
	return false;
}
int ParaEngine::CNPLNetClient::ProcessUrlRequests()
{
	int nCount = 0;
	std::map <std::string, CRequestTaskPool*>::iterator itCur, itEnd = m_request_pools.end();
	for(itCur = m_request_pools.begin();itCur!=itEnd; ++itCur)
	{
		if(itCur->second)
		{
			nCount += itCur->second->DoProcess();
		}
	}
	return nCount;
}

void ParaEngine::CNPLNetClient::AddPendingRequest( const char* sURL )
{
	if(sURL)
	{
		m_pending_requests.insert(sURL);
		if((int)(m_pending_requests.size())>500)
		{
			OUTPUT_LOG("warning: too many (>500) pending URL request found \n");
		}
	}
}

void ParaEngine::CNPLNetClient::RemovePendingRequest( const char* sURL )
{
	if(sURL)
		m_pending_requests.erase(sURL);
}

void ParaEngine::CNPLNetClient::ClearAllPendingRequests()
{
	m_pending_requests.clear();
}

bool ParaEngine::CNPLNetClient::HasPendingRequest( const char* sURL )
{
	if(sURL)
		return m_pending_requests.find(sURL) != m_pending_requests.end();
	else
		return false;
}

const string g_cache_root="temp/cache/";

string ParaEngine::CNPLNetClient::GetCachePath( const char* sFileUrl )
{
	if(sFileUrl)
		return g_cache_root+CHttpUtility::HashStringMD5(sFileUrl);
	else
		return "";
}

CNPLNetClient* ParaEngine::CNPLNetClient::GetInstance()
{
	if(g_pNPLNetClient != 0)
	{
		return g_pNPLNetClient;
	}
	else
	{
		g_pNPLNetClient = new CNPLNetClient();
	}
	return g_pNPLNetClient;
}

void ParaEngine::CNPLNetClient::ReleaseInstance()
{
	SAFE_DELETE(g_pNPLNetClient);
}

bool ParaEngine::CRequestTaskPool::AppendURLRequest( CURLRequestTask* pUrlTask )
{
	if(pUrlTask==NULL)
		return false;
	if((int)(m_task_pool.size()) >= m_nMaxQueuedTask)
	{
		OUTPUT_LOG("warning: max number of queued tasks reached.URLrequest ignored. \n");
		return false;
	}
	// TODO: add task accoding to the priority 
	m_task_pool.push_back(pUrlTask);
	CNPLNetClient::GetInstance()->AddPendingRequest(pUrlTask->m_url.c_str());
	return true;
}

ParaEngine::CRequestTaskPool::~CRequestTaskPool()
{
	if( ! m_task_pool.empty() )
	{
		std::list <CURLRequestTask*>::iterator itCur, itEnd = m_task_pool.end();
		for(itCur = m_task_pool.begin();itCur!=itEnd; ++itCur)
		{
			delete (*itCur);
		}
		m_task_pool.clear();
	}
	// free multi
	if(m_multi_handle)
		curl_multi_cleanup(m_multi_handle);

	// free the CURL handles
	{
		std::list <CUrlWorkerState>::iterator itCur, itEnd = m_easy_handles.end();
		for(itCur = m_easy_handles.begin();itCur!=itEnd; ++itCur)
		{
			if(itCur->m_easy_handle != NULL)
				curl_easy_cleanup(itCur->m_easy_handle);
		}
		m_easy_handles.clear();
	}
}

int ParaEngine::CRequestTaskPool::DoProcess()
{
	if(m_task_pool.size() == 0)
		return 0;
	int nCount = 0;
	bool bStillNeedPerform = false;
	// all running tasks are performed using the curl multi interface. The result of each finished request is saved to CURLRequestTask struct. 
	int nFinishedCount = CURL_MultiPerform();
	do 
	{
		bStillNeedPerform = false;

		// the pool of CURLRequestTask is traversed  during which finished CURLRequestTask is removed from the the queue and the callback is called. 
		if(nFinishedCount>0)
		{
			nFinishedCount = 0;
			// process finished
			std::list <CURLRequestTask*>::iterator itCur, itEnd = m_task_pool.end();
			for(itCur = m_task_pool.begin();itCur!=itEnd; )
			{
				CURLRequestTask* pTask = (*itCur);
				if(pTask && pTask->m_nStatus == CURLRequestTask::URL_REQUEST_COMPLETED)
				{
					//ParaEngine::Lock lock_(pTask->GetMutex());
					if(pTask->m_nStatus == CURLRequestTask::URL_REQUEST_COMPLETED)
					{
						//
						// remove from worker slot
						//
						m_nRunningTaskCount --;
						nCount ++;
						// complete the task. 
						pTask->CompleteTask();
						// delete the task
						SAFE_DELETE(pTask);
						// remove it from the queue
						itCur = m_task_pool.erase(itCur);
						continue;
					}
				}
				++itCur;
			}
		}
		// new tasks are added to the available task slots for further processing. 
		if(m_nRunningTaskCount < m_nMaxWorkerThreads)
		{
			std::list <CURLRequestTask*>::iterator itCur, itEnd = m_task_pool.end();
			for(itCur = m_task_pool.begin();itCur!=itEnd; itCur++)
			{
				CURLRequestTask* pTask = (*itCur);
				if(pTask && pTask->m_nStatus == CURLRequestTask::URL_REQUEST_UNSTARTED)
				{
					CUrlWorkerState* pWorker = GetFreeWorkerSlot();
					if(pWorker && pWorker->m_easy_handle)
					{
						//
						// Add to worker slot: assign task to worker slot and make the handle busy. 
						//
						m_nRunningTaskCount ++;
						pWorker->m_bIsCompleted = false;
						pWorker->m_pCurrentTask = pTask;
						pTask->UpdateTime();
						pTask->SetCurlEasyOpt(pWorker->m_easy_handle);
						pTask->m_nStatus = CURLRequestTask::URL_REQUEST_INCOMPLETE;
						curl_multi_add_handle(m_multi_handle, pWorker->m_easy_handle);
						bStillNeedPerform = true;
					}
					else
					{
						break;
					}
				}
			}
		}
		if(bStillNeedPerform)
		{
			// immediately do some processing if there are new tasks added. 
			nFinishedCount = CURL_MultiPerform();
			bStillNeedPerform = (nFinishedCount>0);
		}
	} while (bStillNeedPerform); // the above three steps are repeated until there is no queued task to be added to any available slots. 
	return nCount;
}

ParaEngine::CRequestTaskPool::CUrlWorkerState* ParaEngine::CRequestTaskPool::GetFreeWorkerSlot()
{
	if(m_nRunningTaskCount >= m_nMaxWorkerThreads)
		return NULL;
	if(m_multi_handle == NULL)
		m_multi_handle = curl_multi_init();
	if(m_multi_handle == NULL)
		return NULL;
	int nCount = 0;
	std::list <CUrlWorkerState>::iterator itCur, itEnd = m_easy_handles.end();
	for(itCur = m_easy_handles.begin();itCur!=itEnd; ++itCur, ++nCount)
	{
		if(itCur->m_bIsCompleted)
		{
			return &(*itCur);
		}
	}
	if(nCount < m_nMaxWorkerThreads)
	{
		// create a new one
		m_easy_handles.push_back(CUrlWorkerState());

		CUrlWorkerState* pWorker = &(m_easy_handles.back());
		if(pWorker->m_easy_handle == NULL)
		{
			pWorker->m_easy_handle = curl_easy_init();
			// The official doc says if multi-threaded use, this one should be set to 1. 
			curl_easy_setopt(pWorker->m_easy_handle, CURLOPT_NOSIGNAL , 1);
			/**
			Pass a long. It should contain the maximum time in seconds that you allow the connection to the server to take. 
			This only limits the connection phase, once it has connected, this option is of no more use. Set to zero to disable 
			connection timeout (it will then only timeout on the system's internal timeouts). See also the CURLOPT_TIMEOUT option
			*/
			// curl_easy_setopt(pWorker->m_easy_handle, CURLOPT_CONNECTTIMEOUT, 10);
			// curl_easy_setopt(pWorker->m_easy_handle, CURLOPT_TIMEOUT, 10);
			// None of these timeout works in Windows Vista, I have to manually implement time out using the multi interface. 
		}
		else
		{
			OUTPUT_LOG("warning: failed creating curl_easy_init interface\n");
		}
		return pWorker;
	}
	return NULL;
}

int ParaEngine::CRequestTaskPool::CURL_MultiPerform()
{
	if(m_multi_handle == NULL)
		return 0;

	int still_running = 0; /* keep number of running handles */

	/* we start some action by calling perform right away */
	while(CURLM_CALL_MULTI_PERFORM ==
		curl_multi_perform(m_multi_handle, &still_running));

	int nCount = 0;

	if(m_nRunningTaskCount > still_running)
	{
		int nProcessed = m_nRunningTaskCount - still_running;

		// for picking up messages with the transfer status
		CURLMsg *msg; 
		// how many messages are left
		int msgs_left; 
		/* See how the transfers went */
		while ((msg = curl_multi_info_read(m_multi_handle, &msgs_left))) 
		{
			if (msg->msg == CURLMSG_DONE) 
			{
				/* Find out which handle this message is about */
				std::list <CUrlWorkerState>::iterator itCur, itEnd = m_easy_handles.end();
				for(itCur = m_easy_handles.begin();itCur!=itEnd; ++itCur)
				{
					if(itCur->m_easy_handle == msg->easy_handle)
					{
						// get return code. 
						itCur->m_returnCode = msg->data.result;
						itCur->m_bIsCompleted = true;
						// remove the easy handle to be reused later. 
						if(m_multi_handle)
							curl_multi_remove_handle(m_multi_handle, itCur->m_easy_handle);

						if(itCur->m_pCurrentTask)
						{
							curl_easy_getinfo (itCur->m_easy_handle, CURLINFO_RESPONSE_CODE, &(itCur->m_pCurrentTask->m_responseCode));
							itCur->m_pCurrentTask->m_nStatus = CURLRequestTask::URL_REQUEST_COMPLETED;
							itCur->m_pCurrentTask->m_returnCode = itCur->m_returnCode;
						}
						nCount++;
					}
				}
			}
			else
			{
				OUTPUT_LOG("warning: unknown message type for curl_multi_info_read \n");
			}
		}
	}
	{
		/*
		** GetTickCount() is available on _all_ Windows versions from W95 up
		** to nowadays. Returns milliseconds elapsed since last system boot,
		** increases monotonically and wraps once 49.7 days have elapsed.
		*/
		DWORD timeNow = GetTickCount();

		// let us check the possible time out. 
		// I believe it is a bug in windows vista, where DNS look up timeout is never reported using the multi interface. 
		// so, let us do a manually time out check here. 
		std::list <CUrlWorkerState>::iterator itCur, itEnd = m_easy_handles.end();
		for(itCur = m_easy_handles.begin();itCur!=itEnd; ++itCur)
		{
			if( ! itCur->m_bIsCompleted && itCur->m_pCurrentTask && itCur->m_pCurrentTask->IsTimedOut(timeNow))
			{
				// get return code. 
				itCur->m_returnCode = CURLE_OPERATION_TIMEDOUT;
				itCur->m_bIsCompleted = true;
				// remove the easy handle to be reused later. 
				if(m_multi_handle)
					curl_multi_remove_handle(m_multi_handle, itCur->m_easy_handle);

				if(itCur->m_pCurrentTask)  
				{
					curl_easy_reset(itCur->m_easy_handle);

					// reset data for timed out requests. 
					itCur->m_pCurrentTask->m_data.clear();
					itCur->m_pCurrentTask->m_header.clear();
					itCur->m_pCurrentTask->m_nStatus = CURLRequestTask::URL_REQUEST_COMPLETED;
					itCur->m_pCurrentTask->m_returnCode = itCur->m_returnCode;
				}
				nCount++;
			}
		}
	}

	return nCount;
}

void ParaEngine::CRequestTaskPool::SetMaxTaskSlotsCount(int nCount)
{
	m_nMaxWorkerThreads = nCount;
}
void ParaEngine::CURLRequestTask::SetCurlEasyOpt( CURL* handle )
{
	// reset data 
	m_data.clear();
	m_header.clear();
	curl_easy_setopt(handle, CURLOPT_URL, m_url.c_str());

	/* Define our callback to get called when there's data to be written */
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CUrl_write_data_callback);
	/* Set a pointer to our struct to pass to the callback */
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, CUrl_write_header_callback);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);

	// form if any. 
	if(m_pFormPost)
	{
		curl_easy_setopt(handle, CURLOPT_HTTPPOST, m_pFormPost);
	}
	else
	{
		curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
	}
}

size_t ParaEngine::CURLRequestTask::CUrl_write_data_callback( void *buffer, size_t size, size_t nmemb, void *stream )
{
	CURLRequestTask * pTask=(CURLRequestTask *) stream;
	if(pTask) 
	{
		pTask->UpdateTime();
		int nByteCount = (int)size*(int)nmemb;
		if(nByteCount>0)
		{
			int nOldSize = (int)pTask->m_data.size();
			pTask->m_data.resize(nOldSize+nByteCount);
			memcpy(&(pTask->m_data[nOldSize]), buffer, nByteCount);

			// just for testing: remove this, dump to debug. 
			// ParaEngine::CLogger::GetSingleton().Write((const char*)buffer, (int)nByteCount);
		}
		return nByteCount;
	}
	return 0;
}

size_t ParaEngine::CURLRequestTask::CUrl_write_header_callback( void *buffer, size_t size, size_t nmemb, void *stream )
{
	CURLRequestTask * pTask=(CURLRequestTask *) stream;
	if(pTask) 
	{
		pTask->UpdateTime();
		int nByteCount = (int)size*(int)nmemb;
		if(nByteCount>0)
		{
			int nOldSize = (int)pTask->m_header.size();
			pTask->m_header.resize(nOldSize+nByteCount);
			memcpy(&(pTask->m_header[nOldSize]), buffer, nByteCount);

		}
		return nByteCount;
	}
	return 0;
}
void ParaEngine::CURLRequestTask::CompleteTask()
{
	// complete the request, call the callback and delete the task and free the handle
#ifdef _DEBUG
	OUTPUT_LOG("-->: URL request task %s completed \n", m_url.c_str());
#endif
	CNPLNetClient::GetInstance()->RemovePendingRequest(m_url.c_str());

	if(!m_sNPLCallback.empty())
	{
		NPL::CNPLWriter writer;
		writer.WriteName("msg");
		writer.BeginTable();

		if(!m_header.empty())
		{
			writer.WriteName("header");
			writer.WriteValue((const char*)(&(m_header[0])), (int)m_header.size());
		}

		if(!m_data.empty())
		{
			writer.WriteName("data");
			writer.WriteValue((const char*)(&(m_data[0])), (int)m_data.size());
		}

		writer.WriteName("code");
		writer.WriteValue(m_returnCode);
		writer.WriteName("rcode");
		writer.WriteValue(m_responseCode);
		writer.EndTable();
		writer.WriteParamDelimiter();
		writer.Append(m_sNPLCallback.c_str());

		CGlobals::GetAISim()->NPLDoString(writer.ToString().c_str(), (int)(writer.ToString().size()));

	}
	if(m_pfuncCallBack)
	{
		m_pfuncCallBack(m_returnCode, this, this->m_pUserData);
	}
}

ParaEngine::CURLRequestTask::~CURLRequestTask()
{
	if(m_pFormPost)
	{
		/* then cleanup the form post chain */
		curl_formfree(m_pFormPost);
	}
	SafeDeleteUserData();
}

CURLFORMcode ParaEngine::CURLRequestTask::AppendFormParam( const char* name, const char* value )
{
	return curl_formadd(&m_pFormPost, &m_pFormLast, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, value, CURLFORM_END);
}

CURLFORMcode ParaEngine::CURLRequestTask::AppendFormParam( const char* name, const char* type, const char* file, const char* data, int datalen )
{
	CURLFORMcode rc = CURL_FORMADD_OK;
	/* file upload */
	if ((file != NULL) && (data == NULL)) 
	{
		rc = (type == NULL)?
			curl_formadd(&m_pFormPost, &m_pFormLast, CURLFORM_COPYNAME, name, 
			CURLFORM_FILE, file, CURLFORM_END): 
		curl_formadd(&m_pFormPost, &m_pFormLast, CURLFORM_COPYNAME, name, 
			CURLFORM_FILE, file, 
			CURLFORM_CONTENTTYPE, type, CURLFORM_END); 
	}
	/* data field */
	else if ((file != NULL) && (data != NULL)) 
	{
		/* Add a buffer to upload */
		rc = (type != NULL)? 
			curl_formadd(&m_pFormPost, &m_pFormLast,
			CURLFORM_COPYNAME, name,
			CURLFORM_BUFFER, file, CURLFORM_BUFFERPTR, data, CURLFORM_BUFFERLENGTH, datalen,
			CURLFORM_CONTENTTYPE, type, 
			CURLFORM_END):
		curl_formadd(&m_pFormPost, &m_pFormLast,
			CURLFORM_COPYNAME, name,
			CURLFORM_BUFFER, file, CURLFORM_BUFFERPTR, data, CURLFORM_BUFFERLENGTH, datalen,
			CURLFORM_END);
	}
	else 
	{
		OUTPUT_LOG("warning: Mandatory: \"file\" \n");
	}
	if (rc != CURL_FORMADD_OK) 
	{
		OUTPUT_LOG("warning:  cannot add form: %d \n", rc);
	}
	return rc;
}

void ParaEngine::CURLRequestTask::SafeDeleteUserData()
{
	if(m_nUserDataType>0)
	{
		if(m_nUserDataType==1)
		{
			SAFE_DELETE(m_pAssetData);
		}
	}
}

void ParaEngine::CURLRequestTask::SetAssetRequestOpt( CAssetRequestData* pRequestData, URL_REQUEST_TASK_CALLBACK pFuncCallback/*=NULL*/ )
{
	SafeDeleteUserData();
	m_nUserDataType=1;
	m_pAssetData = pRequestData;
	if(pFuncCallback == 0)
	{
		// if none is specified, it will pick a default one to use according to pRequestData->m_nAssetType;
		if(m_pAssetData)
		{
			m_pfuncCallBack = Asset_HTTP_request_callback;
		}
	}
	else
	{
		m_pfuncCallBack = pFuncCallback;
	}
}

DWORD ParaEngine::CURLRequestTask::Asset_HTTP_request_callback( int nResult, CURLRequestTask* pRequest, LPVOID lpUserData )
{
#ifdef PARAENGINE_CLIENT
	CAssetRequestData* pRequestData = (CAssetRequestData*) lpUserData;
	if(pRequestData == 0)
		return E_FAIL;
	AssetEntity* pAssetEntity = NULL;
	if(pRequestData->m_nAssetType == AssetEntity::texture)
		pAssetEntity = (CGlobals::GetAssetManager()->GetTextureManager().get(pRequestData->m_sAssetKey.c_str()));
	else if(pRequestData->m_nAssetType == AssetEntity::mesh)
		pAssetEntity = (CGlobals::GetAssetManager()->GetMeshManager().get(pRequestData->m_sAssetKey.c_str()));
	else if(pRequestData->m_nAssetType == AssetEntity::parax)
		pAssetEntity = (CGlobals::GetAssetManager()->GetParaXManager().get(pRequestData->m_sAssetKey.c_str()));
	if(!pAssetEntity)
		return E_FAIL;

	// the HTTP status code must be 200 in order to proceed. 
	if(nResult == CURLE_OK && (pRequest->m_responseCode==200) && (int)(pRequest->m_data.size())>0)
	{
		if( !(pRequestData->m_sAssetKey.empty()) )
		{
			// encode the m_sAssetKey
			string sCachedFileName = CNPLNetClient::GetInstance()->GetCachePath(pRequestData->m_sAssetKey.c_str());
			{
				CParaFile file;

				if(file.CreateNewFile(sCachedFileName.c_str()))
				{
					file.write(&(pRequest->m_data[0]), (int)(pRequest->m_data.size()));
					file.close();

					pAssetEntity->SetState(AssetEntity::ASSET_STATE_SYNC_SUCCEED);
					pAssetEntity->Refresh(sCachedFileName.c_str(), pRequestData->m_bLazyLoading);
					return S_OK;
				}
			}
		}
	}
	else
	{
		OUTPUT_LOG("warning: sync asset file %s. status code %d, return code %d\n", pRequest->m_url.c_str(), pRequest->m_responseCode, pRequest->m_returnCode);
	}
	pAssetEntity->SetState(AssetEntity::ASSET_STATE_SYNC_FAIL);
	return E_FAIL;
#else
	return S_OK;
#endif
}

ParaEngine::CAssetRequestData::CAssetRequestData( AssetEntity& asset )
{
	m_nAssetType = asset.GetType();
	m_sAssetKey = asset.GetKey();
}

bool ParaEngine::CURLRequestTask::IsTimedOut( DWORD nCurrentTime )
{
	return ( ( m_nStartTime+m_nTimeOutTime) < nCurrentTime);
}

DWORD ParaEngine::CURLRequestTask::UpdateTime()
{
	m_nStartTime = ::GetTickCount();
	return m_nStartTime;
}

void ParaEngine::CURLRequestTask::SetTimeOut( int nMilliSeconds )
{
	m_nTimeOutTime = nMilliSeconds;
}

int ParaEngine::CURLRequestTask::GetTimeOut()
{
	return m_nTimeOutTime;
}

#endif
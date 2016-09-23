//-----------------------------------------------------------------------------
// Class:	Async Loader
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: for async asset streaming. It uses architecture proposed by the content streaming sample in DirectX 9&10
//-----------------------------------------------------------------------------
/** Design 

---++ File Format

---+++ download info log
output service_log to ".downloadinfo.txt"

format is [threadid]:[Replaced|Created] [message]

1544:Starting 8 downloader threads
17b8:Replaced config/GameClient.Config.xml
13b4:Created  Resources/ABC.xml
0e0c:Downloader thread done
1544:Finished in 117 seconds

---+++ assets log
output all asynchronous assets log to service log file "assets_[date].txt"

Examples:

2009-08-17 21:33:20|0|AssetRequested Texture/tooltip2_32bits.PNG
2009-08-17 21:33:44|0|AssetLoaded Texture/tooltip2_32bits.PNG

---+++ Asset manifest
output assets to "Assets_manifest.txt"

format is [relative path],md5,fileSize 
if the name ends with .z, it is zipped. This could be 4MB uncompressed in size
md5 is checksum code of the file. fileSize is the compressed file size. 

character/v3/elf/female/elffemale.x.z,67829385df2e30076044b59aff13c257,398976
character/v3/elf/hair01_01.dds.z,c16e596c3cdac944d1eaa2a26124448a,22328
character/v3/elf/female/cartoonface_r7.dds.z,30d258b7fd5b0d59426610cd23cc17f4,15542
model/skybox/skybox3/skybox3.x.z,a9486905adf2931e57b6bdfc6efd0b12,679
games/tutorial.swf,1157008036,171105

---+++ Client config
the following related settings are in GameClient.config.xml

World=test_world
PreDownloadAssets=1

[AssetDelivery]
IndirectServerAddress=http://patch.paraengine.com/assets/

*/
#include "ParaEngine.h"
#include "util/CSingleton.h"
#ifdef PARAENGINE_CLIENT
#include "DirectXEngine.h"
#include "GDIEngine.h"
#include "ParaWorldAsset.h"
#endif

#include "AssetManifest.h"
#include "UrlLoaders.h"

#include "AsyncLoader.h"

using namespace ParaEngine;

/** default resource queue size */
#define DEFAULT_RESOURCE_QUEUE_SIZE		500

#ifdef PARAENGINE_CLIENT
#define DEFAULT_LOCAL_THREAD_COUNT		1
#define DEFAULT_ASSETS_THREAD_COUNT		2
#define DEFAULT_WEB_THREAD_COUNT		1
#define DEFAULT_BIGFILE_THREAD_COUNT	1
#define DEFAULT_AUDIOFILE_THREAD_COUNT	1
#else
#define DEFAULT_LOCAL_THREAD_COUNT		1
#define DEFAULT_ASSETS_THREAD_COUNT		1
#define DEFAULT_WEB_THREAD_COUNT		1
#define DEFAULT_BIGFILE_THREAD_COUNT	1
#define DEFAULT_AUDIOFILE_THREAD_COUNT	1
#endif

/** easy define :-) */
#define ASSETS_LOG(...) SERVICE_LOG1(g_asset_logger, ## __VA_ARGS__)

///////////////////////////////////////////////////////
//
// ProcessorWorkerThread
// 
///////////////////////////////////////////////////////

CAsyncLoader::ProcessorWorkerThread::ProcessorWorkerThread()
:m_nQueueID(0), m_curl(0),m_nBytesProcessed(0)
{
}

CAsyncLoader::ProcessorWorkerThread::ProcessorWorkerThread(int nQueueID)
:m_nQueueID(nQueueID), m_curl(0),m_nBytesProcessed(0)
{
}

CAsyncLoader::ProcessorWorkerThread::~ProcessorWorkerThread()
{
	if(m_curl)
	{
		curl_easy_cleanup(m_curl);
		m_curl = NULL;
	}
}

void ParaEngine::CAsyncLoader::ProcessorWorkerThread::AddBytesProcessed( int nBytesProcessed )
{
	m_nBytesProcessed += nBytesProcessed;
}

int ParaEngine::CAsyncLoader::ProcessorWorkerThread::GetBytesProcessed()
{
	return m_nBytesProcessed;
}


void* CAsyncLoader::ProcessorWorkerThread::GetCurlInterface(int nID)
{
	if(m_curl)
	{
		return m_curl;
	}
	else
	{
		m_curl = curl_easy_init();

		return (void*)m_curl;
	}
}

void* CAsyncLoader::DefaultWorkerThreadData::GetCurlInterface(int nID)
{
	if(m_curl)
	{
		return m_curl;
	}
	else
	{
		m_curl = curl_easy_init();

		// The official doc says if multi-threaded use, this one should be set to 1. 
		curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
		return (void*)m_curl;
	}
}

bool CAsyncLoader::ProcessorWorkerThread::timed_join(int nSeconds)
{
	if(m_thread.get()) 
	{
		return m_thread->timed_join(boost::posix_time::millisec(nSeconds*1000));
	}
	return true;
}

///////////////////////////////////////////////////////
//
// ResourceRequest: head file in IDataLoader.h
// 
///////////////////////////////////////////////////////

ResourceRequest::~ResourceRequest()
{
	SAFE_DELETE(m_pDataLoader);
	SAFE_DELETE(m_pDataProcessor);
}

CResourceRequestQueue::CResourceRequestQueue()
	: NPL::concurrent_ptr_queue<ResourceRequest_ptr>(DEFAULT_RESOURCE_QUEUE_SIZE)
{
}

CResourceRequestQueue::CResourceRequestQueue(int capacity)
	: NPL::concurrent_ptr_queue<ResourceRequest_ptr>(capacity)
{
}

CResourceRequestQueue::~CResourceRequestQueue()
{
}

///////////////////////////////////////////////////////
//
// CAsyncLoader
// 
///////////////////////////////////////////////////////

CAsyncLoader::CAsyncLoader()
:m_NumOutstandingResources(0), m_nRemainingBytes(0),
#ifdef PARAENGINE_CLIENT
m_pXFileParser(NULL), m_pEngine(NULL), m_pGDIEngine(NULL),
#endif
m_bDone(false), m_bProcessThreadDone(false), m_bIOThreadDone(false), m_bInterruptSignal(false), m_default_processor_worker_data(NULL)
{
	g_asset_logger.reset(new CServiceLogger("assets.log", false));
	g_asset_logger->SetForceFlush(false);
	m_RenderThreadQueue.SetUseEvent(false);

	CAssetManifest::GetSingleton().PrintStat();

	m_default_processor_worker_data = new DefaultWorkerThreadData();
}

CAsyncLoader::~CAsyncLoader()
{
	CleanUp();
}

void ParaEngine::CAsyncLoader::CleanUp()
{
	if(m_default_processor_worker_data == 0)
		return;

	Stop();

	for(int i=0; i<(int)(m_workers.size()); ++i)
	{
		SAFE_DELETE(m_workers[i]);
	}
	m_workers.clear();

	SAFE_DELETE(m_default_processor_worker_data);
}

CAsyncLoader& ParaEngine::CAsyncLoader::GetSingleton()
{
	return *(CAppSingleton<CAsyncLoader>::GetInstance());
}

void ParaEngine::CAsyncLoader::Interrupt() 
{
	m_bInterruptSignal = true;
};

void ParaEngine::CAsyncLoader::log(const string& message)
{
	g_asset_logger->WriteServiceFormated("%s", message.c_str());
}

bool ParaEngine::CAsyncLoader::CreateWorkerThreads(int nProcessorQueueID, int nMaxCount)
{
	int nCount = 0;
	int nWorkerCount = (int)(m_workers.size());
	int i;
	for(i=0; i<nWorkerCount; ++i)
	{
		if(m_workers[i]->GetProcessorQueueID() == nProcessorQueueID)
		{
			nCount++;
		}
	}

	for(i=nCount; i<nMaxCount; ++i)
	{
		ProcessorWorkerThread* worker_thread= new ProcessorWorkerThread(nProcessorQueueID);
		worker_thread->reset(new boost::thread(boost::bind(&CAsyncLoader::ProcessingThreadProc, this, worker_thread)));
		m_workers.push_back(worker_thread);
	}
	return true;
}

int ParaEngine::CAsyncLoader::GetEstimatedSizeInBytes()
{
	return m_nRemainingBytes;
}

int ParaEngine::CAsyncLoader::GetItemsLeft(int nItemType)
{
	if(nItemType == -1)
	{
		// total requests in the queue
		return m_NumOutstandingResources;
	}
	else if (nItemType>=ResourceRequestID_Local && nItemType<=ResourceRequestID_Asset_BigFile)
	{
		// only remote requests in the queue. 
		return (int)(m_ProcessQueues[nItemType].size());
	}
	else if(nItemType == -2)
	{
		return GetItemsLeft(ResourceRequestID_Asset_BigFile) + GetItemsLeft(ResourceRequestID_Asset);
	}
	return 0;
}


int ParaEngine::CAsyncLoader::GetBytesProcessed( int nItemType /*= -1*/ )
{
	int nBytesProcessed = 0;
	if(nItemType == -1)
	{
		// total requests in the queue
		int nWorkerCount = (int)(m_workers.size());
		for(int i=0; i<nWorkerCount; ++i)
		{
			nBytesProcessed += m_workers[i]->GetBytesProcessed();
		}
		return nBytesProcessed;
	}
	else if (nItemType>=ResourceRequestID_Local && nItemType<=ResourceRequestID_Asset_BigFile)
	{
		// only remote requests in the queue. 
		int nWorkerCount = (int)(m_workers.size());
		for(int i=0; i<nWorkerCount; ++i)
		{
			if(m_workers[i]->GetProcessorQueueID() == nItemType)
			{
				nBytesProcessed += m_workers[i]->GetBytesProcessed();
			}
		}
		return nBytesProcessed;
	}
	else if(nItemType == -2)
	{
		return GetBytesProcessed(ResourceRequestID_Asset_BigFile) + GetBytesProcessed(ResourceRequestID_Asset);
	}
	return nBytesProcessed;
}


void ParaEngine::CAsyncLoader::WaitForAllItems()
{
	if(m_NumOutstandingResources > 0)
	{
		OUTPUT_LOG("CAsyncLoader waiting for %d pending resources. \n", m_NumOutstandingResources);
		ProcessDeviceWorkItems( 100000, false );

		// let us wait only limited time. 30*100 = 3000ms 
		for(int i=0; i<30; ++i)
		{
			// Only exit when all resources are loaded
			if( 0 == m_NumOutstandingResources )
				return;

			// Service Queues
			ProcessDeviceWorkItems( 100000, false );
			SLEEP( 100);
		}
		if(m_NumOutstandingResources != 0)
		{
			OUTPUT_LOG("warning: CAsyncLoader still have %d pending resources, but we will not wait for their completion. \n", m_NumOutstandingResources);
		}
	}
}

int CAsyncLoader::Stop()
{
	// Do not call WaitForAllItems, because Device is already lost. or there will be a crash
	// WaitForAllItems();

	int nWorkerCount = (int)(m_workers.size());

	int i;
	for(i=0; i<nWorkerCount; ++i)
	{
		ResourceRequest_ptr msg(new ResourceRequest(ResourceRequestType_Quit));
		m_ProcessQueues[m_workers[i]->GetProcessorQueueID()].push(msg);
	}

	// set the interrupt signal, so some downloading process will exit gracefully.
	Interrupt();

	int nSecondLeft = 1;
	for(i=0; i<nWorkerCount; ++i)
	{
		if(!(m_workers[i]->timed_join(nSecondLeft)))
		{
			OUTPUT_LOG("warning: CAsyncLoader worker thread %d of %d does not exit after %d seconds. we shall terminate the hard way. \n", i, nWorkerCount, nSecondLeft);
			// TODO: find a way to terminate thread the hard way. 
			m_workers[i]->join();
		}
		m_workers[i]->reset();
		OUTPUT_LOG("CAsyncLoader worker thread %d of %d is exited\n", i, nWorkerCount);
		SAFE_DELETE(m_workers[i]);
	}
	m_workers.clear();

	if(m_io_thread.get()!=0)
	{
		ResourceRequest_ptr msg(new ResourceRequest(ResourceRequestType_Quit));
		m_IOQueue.push(msg);

		m_io_thread->join();
		m_io_thread.reset();
		OUTPUT_LOG("CAsyncLoader IO thread is exited\n");
	}

	// clear all queued messages,  since we already called WaitForAllItems(). the following should never be needed. 
	// however, for safety we just empty all queued items. 
	ResourceRequest_ptr req;
	for(i=0; i<MAX_PROCESS_QUEUE; ++i)
	{
		while(m_ProcessQueues[i].try_pop(req))
		{
			OUTPUT_LOG("warning: process queue %d still has pending item %s \n", i, req->m_pDataLoader->GetFileName());
		}
	}

	while(m_IOQueue.try_pop(req))
	{
		OUTPUT_LOG("warning: IO queue still has pending item %s \n", req->m_pDataLoader->GetFileName());
	}
	while(m_RenderThreadQueue.try_pop(req))
	{
		OUTPUT_LOG("warning: render queue still has pending item %s \n", req->m_pDataLoader->GetFileName());
	}

#ifdef PARAENGINE_CLIENT
	SAFE_RELEASE(m_pXFileParser);

	if(m_pEngine)
	{
		m_pEngine->Destroy();
		SAFE_DELETE(m_pEngine);
	}

	if(m_pGDIEngine)
	{
		m_pGDIEngine->Destroy();
		SAFE_DELETE(m_pGDIEngine);
	}
#endif
	return 0;
}

int CAsyncLoader::Start(int nWorkerCount)
{
	if((int)(m_workers.size()) != 0 || nWorkerCount <= 0)
	{
		return 0;
	}
#ifdef PARAENGINE_CLIENT
	CGlobals::GetAssetManager()->CreateXFileParser(&m_pXFileParser);
#endif
	m_bInterruptSignal = false;

	// start the io thread
	m_io_thread.reset(new boost::thread(boost::bind(&CAsyncLoader::FileIOThreadProc, this)));

	// queue[0] is for local CPU intensive tasks like unzip. (only one thread process it)
	CreateWorkerThreads(ResourceRequestID_Local, DEFAULT_LOCAL_THREAD_COUNT); 
	// queue[1] is for remote background asset loading. (2 threads process it)
	CreateWorkerThreads(ResourceRequestType_Asset, DEFAULT_ASSETS_THREAD_COUNT);
	// queue[2] is for remote REST URL request. (1 threads process it)
	CreateWorkerThreads(ResourceRequestType_Web, DEFAULT_WEB_THREAD_COUNT);
	// queue[3] is for remote background asset loading (Big file only). (1 threads process it)
	CreateWorkerThreads(ResourceRequestID_Asset_BigFile, DEFAULT_BIGFILE_THREAD_COUNT);
	// queue[4] is for remote background asset loading (Big file only). (1 threads process it)
	CreateWorkerThreads(ResourceRequestID_AudioFile, DEFAULT_AUDIOFILE_THREAD_COUNT);
	

	OUTPUT_LOG("CAsyncLoader is started with 1 IO thread and %d worker thread\n", nWorkerCount);

	return 0;
}
#ifdef PARAENGINE_CLIENT
CDirectXEngine* CAsyncLoader::GetEngine() 
{
	if(m_pEngine == 0)
	{
		m_pEngine = new CDirectXEngine();
		m_pEngine->Create();
	}
	return m_pEngine;
}

CGDIEngine* CAsyncLoader::GetGDIEngine()
{
	if(m_pGDIEngine == 0)
	{
		m_pGDIEngine = new CGDIEngine();
		m_pGDIEngine->Create();
	}
	return m_pGDIEngine;
}
#endif
int CAsyncLoader::FileIOThreadProc()
{
	ResourceRequest_ptr ResourceRequest;
	HRESULT hr = S_OK;

	ASSETS_LOG("CAsyncLoader IO Thread started");

	int nRes = 0;
	while(nRes != -1)
	{
		m_IOQueue.wait_and_pop(ResourceRequest);
		if(ResourceRequest->m_nType == ResourceRequestType_Quit)
		{
			break;
		}
		// Handle a read request
		if( !ResourceRequest->m_bCopy )
		{
			// ASSETS_LOG("DEBUG: IO msg(to proc) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

			if( !ResourceRequest->m_bError )
			{
				// Load the data
				hr = ResourceRequest->m_pDataLoader->Load();

				if( FAILED( hr ) )
				{
					const char * keyname = ResourceRequest->m_pDataLoader->GetKeyName();
					if(keyname == 0)
						keyname = "key not set";
					const char * filename = ResourceRequest->m_pDataLoader->GetFileName();
					if(filename == 0)
						filename = "unknown file";
					if(hr != E_PENDING)
					{
						OUTPUT_LOG( "FileIO Error: hr = %x; key=%s; filename=%s\n", hr, keyname, filename);
					}
					
					ResourceRequest->m_bError = true;
					ResourceRequest->m_last_error_code = hr;
					if( ResourceRequest->m_pHR )
						*ResourceRequest->m_pHR = hr;
					ResourceRequest->m_pDataProcessor->SetResourceError();
				}
			}

			// Add it to the ProcessQueue
			if(m_ProcessQueues[ResourceRequest->m_nProcessorQueueID].try_push( ResourceRequest ) == CResourceRequestQueue::BufferOverFlow)
			{
				ASSETS_LOG("ERROR: IO msg(to proc) failed push to queue for %s\n", ResourceRequest->m_pDataLoader->GetFileName());
			}
		}

		// Handle a copy request
		else
		{
			// ASSETS_LOG("DEBUG: IO msg(COPY) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

			if( !ResourceRequest->m_bError )
			{
				// Create the data
				hr = ResourceRequest->m_pDataProcessor->CopyToResource();

				if( FAILED( hr ) )
				{
					if(hr != E_PENDING)
					{
						OUTPUT_LOG( "Failed to Copy Data to Device Object:hr = %x; %s\n", hr, ResourceRequest->m_pDataLoader->GetFileName());
					}

					ResourceRequest->m_bError = true;
					ResourceRequest->m_last_error_code = hr;
					if( ResourceRequest->m_pHR )
						*ResourceRequest->m_pHR = hr;
				}
			}
			else
			{
				ResourceRequest->m_pDataProcessor->SetResourceError();
			}

			// send an unlock request
			ResourceRequest->m_bLock = false;
			if(m_RenderThreadQueue.try_push( ResourceRequest ) == m_RenderThreadQueue.BufferOverFlow)
			{
				ASSETS_LOG("ERROR: IO msg(COPY) failed push to queue for %s\n", ResourceRequest->m_pDataLoader->GetFileName());
			}
		}
	}
	return 0;
}

int CAsyncLoader::ProcessingThreadProc(ProcessorWorkerThread* pThreadData)
{
	ResourceRequest_ptr ResourceRequest;
	HRESULT hr = S_OK;

	if(pThreadData == 0)
		return E_FAIL;

	int nQueueID = pThreadData->GetProcessorQueueID();

	{
		// print thread info to log
		const char* ThreadType = "";
		switch(nQueueID)
		{
		case ResourceRequestType_Web:
			ThreadType = "Web";
			break;
		case ResourceRequestType_Asset:
			ThreadType = "Asset";
			break;
		case ResourceRequestID_Local:
			ThreadType = "Local";
			break;
		case ResourceRequestID_Asset_BigFile:
			ThreadType = "BigFile";
			break;
		case ResourceRequestID_AudioFile:
			ThreadType = "AudioFile";
			break;
		}
		ASSETS_LOG("Async Processing Thread %s(%d) Started\n", ThreadType, nQueueID);
	}

	int nRes = 0;
	while(nRes != -1)
	{
		m_ProcessQueues[nQueueID].wait_and_pop(ResourceRequest);
		if(ResourceRequest->m_nType == ResourceRequestType_Quit)
		{
			break;
		}
		// let us sleep some time to emulate slow connection for debugging purposes.
		// Sleep(300);

		// ASSETS_LOG("DEBUG: process msg %s\n", ResourceRequest->m_pDataLoader->GetFileName());
		
		// Decompress the data
		if( !ResourceRequest->m_bError )
		{
			void* pData = NULL;
			int cDataSize = 0;
			hr = ResourceRequest->m_pDataLoader->Decompress( &pData, &cDataSize );
			if( SUCCEEDED( hr ) )
			{
				// Process the data
				ResourceRequest->m_pDataProcessor->SetProcessorWorkerData(pThreadData);
				hr = ResourceRequest->m_pDataProcessor->Process( pData, cDataSize );
			}
		}

		if( FAILED( hr ) )
		{
			OUTPUT_LOG( "Processing Thread Error: hr = %x\n", hr );
			
			ResourceRequest->m_bError = true;
			ResourceRequest->m_last_error_code = hr;
			if( ResourceRequest->m_pHR )
				*ResourceRequest->m_pHR = hr;
		}

		// Add it to the RenderThreadQueue
		ResourceRequest->m_bLock = true;
		if(m_RenderThreadQueue.try_push( ResourceRequest ) == m_RenderThreadQueue.BufferOverFlow)
		{
			ASSETS_LOG("ERROR: process msg failed push to queue for %s\n", ResourceRequest->m_pDataLoader->GetFileName());
		}
	}
	return 0;
}

HRESULT ParaEngine::CAsyncLoader::RunWorkItem( IDataLoader* pLoader, IDataProcessor* pProcessor, HRESULT* pHResult, void** ppDeviceObject)
{
	if( !pLoader || !pProcessor )
		return E_INVALIDARG;

	HRESULT res = S_OK;
	void* pLocalData;
	int Bytes;

	ParaEngine::Lock lock_(m_default_processor_mutex);
	pProcessor->SetProcessorWorkerData(m_default_processor_worker_data);

	if( SUCCEEDED(res=pLoader->Load()) && 
		SUCCEEDED(res=pLoader->Decompress( &pLocalData, &Bytes )) && 
		SUCCEEDED(res=pProcessor->Process( pLocalData, Bytes )) && 
		SUCCEEDED(res=pProcessor->LockDeviceObject()) && 
		SUCCEEDED(res=pProcessor->CopyToResource()) && 
		SUCCEEDED(res=pProcessor->UnLockDeviceObject()) )
	{
	}
	
	if(FAILED(res))
	{
		pProcessor->SetResourceError();
	}
	pProcessor->Destroy();
	pLoader->Destroy();

	return res;
}

HRESULT ParaEngine::CAsyncLoader::RunWorkItem( ResourceRequest_ptr& request )
{
	OUTPUT_LOG("warning: TODO: RunWorkItem(ResourceRequest_ptr) is not supported yet\n");
	return S_OK;
}

int ParaEngine::CAsyncLoader::AddWorkItem( IDataLoader* pDataLoader, IDataProcessor* pDataProcessor, HRESULT* pHResult, void** ppDeviceObject , int nProcessorThreadID)
{
	if( !pDataLoader || !pDataProcessor )
		return E_INVALIDARG;

	ResourceRequest_ptr msg(new ResourceRequest(ResourceRequestType_Local));
	msg->m_nProcessorQueueID = nProcessorThreadID;
	msg->m_pDataLoader = pDataLoader;
	msg->m_pDataProcessor = pDataProcessor;
	msg->m_pHR = pHResult;
	msg->m_ppDeviceObject = ppDeviceObject;
	msg->m_bCopy = false;
	msg->m_bLock = false;
	msg->m_bError = false;
	
	if( ppDeviceObject)
		*ppDeviceObject = NULL;

	AddWorkItem(msg);
	return S_OK;
}

int ParaEngine::CAsyncLoader::AddWorkItem( ResourceRequest_ptr& msg)
{
	const char* name = msg->m_pDataLoader->GetFileName();
	if(name == 0)
		name = "unknown";

	if(msg->m_nProcessorQueueID == ResourceRequestID_Local)
	{
		ASSETS_LOG("AssetRequested %s\n", name);
	}
	else
	{
		ASSETS_LOG("AddWorkItem(%d) %s\n", (int)(msg->m_nProcessorQueueID), name);
	}
	int nSizeBytes = msg->m_pDataLoader->GetEstimatedSizeInBytes();

	if(m_IOQueue.try_push(msg) != m_IOQueue.BufferOverFlow)
	{
		m_nRemainingBytes += nSizeBytes;
		m_NumOutstandingResources ++;
	}
	else
	{
		OUTPUT_LOG("warning: there is no more room in the CAsyncLoader worker queue for asset file %s\n", name);
	}
	return S_OK;
}

void ParaEngine::CAsyncLoader::ProcessDeviceWorkItems( int CurrentNumResourcesToService, bool bRetryLoads )
{
	HRESULT hr = S_OK;
	ResourceRequest_ptr ResourceRequest;

	for( int i = 0; (i<CurrentNumResourcesToService) && m_RenderThreadQueue.try_pop(ResourceRequest); i++ )
	{
		if( ResourceRequest->m_bLock )
		{
			// ASSETS_LOG("DEBUG: render msg(lock) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

			if( !ResourceRequest->m_bError )
			{
				hr = ResourceRequest->m_pDataProcessor->LockDeviceObject();
				if( E_TRYAGAIN == hr && bRetryLoads )
				{
					// add it back to the list
					m_RenderThreadQueue.push( ResourceRequest );
					
					// move on to the next guy
					continue;
				}
				else if( FAILED( hr ) )
				{
					ResourceRequest->m_bError = true;
					ResourceRequest->m_last_error_code = hr;
					if( ResourceRequest->m_pHR )
						*ResourceRequest->m_pHR = hr;
				}
			}

			ResourceRequest->m_bCopy = true;
			if(m_IOQueue.try_push( ResourceRequest ) == m_IOQueue.BufferOverFlow)
			{
				ASSETS_LOG("ERROR: render msg(lock) failed push to queue for %s\n", ResourceRequest->m_pDataLoader->GetFileName());
			}
		}
		else
		{
			// ASSETS_LOG("DEBUG: render msg(unlock) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

			if( !ResourceRequest->m_bError )
			{
				HRESULT hr = ResourceRequest->m_pDataProcessor->UnLockDeviceObject();
				ResourceRequest->m_last_error_code = hr;
				if( ResourceRequest->m_pHR )
					*ResourceRequest->m_pHR = hr;

				const char* name = ResourceRequest->m_pDataLoader->GetFileName();
				if(name == 0)
					name = "unknown";
				ASSETS_LOG("AssetLoaded %s\n", name);
			}
			else
			{
				if(ResourceRequest->m_last_error_code != E_PENDING)
				{
					const char* sFileName = ResourceRequest->m_pDataLoader->GetFileName();
					if(sFileName && sFileName[0] != '\0')
					{
						// bug fix: we only set resource error if file name is not ""
						// for some reason, if we set resource error for empty file, some models will not show up.  Empty file is considered valid. 
						ASSETS_LOG("ERROR: AssetFailed (%d)%s\n", ResourceRequest->m_last_error_code, sFileName);
						ResourceRequest->m_pDataProcessor->SetResourceError();
					}
				}
			}

			m_nRemainingBytes -= ResourceRequest->m_pDataLoader->GetEstimatedSizeInBytes();

			ResourceRequest->m_pDataLoader->Destroy();
			ResourceRequest->m_pDataProcessor->Destroy();

			SAFE_DELETE( ResourceRequest->m_pDataLoader );
			SAFE_DELETE( ResourceRequest->m_pDataProcessor );

			// Decrement num outstanding resources
			m_NumOutstandingResources --;
		}
	}
}

void ParaEngine::CAsyncLoader::AddPendingRequest( const char* sURL )
{
	if(sURL)
	{
		ParaEngine::Lock lock_(m_pending_request_mutex);

		m_pending_requests.insert(sURL);
		if((int)(m_pending_requests.size())>500)
		{
			OUTPUT_LOG("warning: too many (>500) pending URL request found \n");
		}
	}
}

void ParaEngine::CAsyncLoader::RemovePendingRequest( const char* sURL )
{
	if(sURL)
	{
		ParaEngine::Lock lock_(m_pending_request_mutex);
		m_pending_requests.erase(sURL);
	}
}

void ParaEngine::CAsyncLoader::ClearAllPendingRequests()
{
	ParaEngine::Lock lock_(m_pending_request_mutex);
	m_pending_requests.clear();
}

bool ParaEngine::CAsyncLoader::HasPendingRequest( const char* sURL )
{
	if(sURL)
	{
		ParaEngine::Lock lock_(m_pending_request_mutex);
		return m_pending_requests.find(sURL) != m_pending_requests.end();
	}
	else
		return false;
}


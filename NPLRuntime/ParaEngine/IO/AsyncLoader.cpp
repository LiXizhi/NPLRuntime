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
#define DEFAULT_RESOURCE_QUEUE_SIZE		1000
#define MAX_RESOURCE_QUEUE_SIZE		100000

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

const int DEFAULT_IO_THREADS = 1;

/** easy define :-) */
#define ASSETS_LOG(nLevel, ...) if(m_nLogLevel<=nLevel){ SERVICE_LOG1(g_asset_logger, ## __VA_ARGS__) }

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
#ifndef EMSCRIPTEN
		curl_easy_cleanup(m_curl);
#endif
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
#ifndef EMSCRIPTEN
		m_curl = curl_easy_init();
#endif

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
#ifndef EMSCRIPTEN
		m_curl = curl_easy_init();

		// The official doc says if multi-threaded use, this one should be set to 1. 
		curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
#endif
		return (void*)m_curl;
	}
}

bool CAsyncLoader::ProcessorWorkerThread::timed_join(int nSeconds)
{
	if(m_thread.get()) 
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		return m_thread->timed_join(boost::posix_time::millisec(nSeconds*1000));
#endif
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

CResourceRequestQueue::BufferStatus CResourceRequestQueue::try_push(ResourceRequest_ptr& item)
{
	if (full())
	{
		if (capacity() < MAX_RESOURCE_QUEUE_SIZE)
			set_capacity((std::min)((int)capacity() + DEFAULT_RESOURCE_QUEUE_SIZE, MAX_RESOURCE_QUEUE_SIZE));
	}
	return concurrent_ptr_queue<ResourceRequest_ptr>::try_push(item);
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
m_bDone(false), m_bProcessThreadDone(false), m_bIOThreadDone(false), m_bInterruptSignal(false), m_default_processor_worker_data(NULL), m_nLogLevel(CAsyncLoader::Log_Warn)
, m_UsedIOThread(0)
{
#ifdef _DEBUG
	// SetLogLevel(Log_All);
#endif
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
	log(0, message);
}

void ParaEngine::CAsyncLoader::log(int nLogLevel, const string& message)
{
	if (GetLogLevel() <= nLogLevel)
	{
		g_asset_logger->WriteServiceFormated("%s", message.c_str());
	}
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
	int nNewlyCreated = 0;
	for(i=nCount; i<nMaxCount; ++i, ++nNewlyCreated)
	{
		ProcessorWorkerThread* worker_thread= new ProcessorWorkerThread(nProcessorQueueID);
#ifndef EMSCRIPTEN_SINGLE_THREAD
		worker_thread->reset(new boost::thread(boost::bind(&CAsyncLoader::ProcessingThreadProc, this, worker_thread)));
#else
		worker_thread->reset(CoroutineThread::StartCoroutineThread([this](CoroutineThread* t) -> CO_ASYNC {
			CO_AWAIT(this->ProcessingCorountineThreadProc(t));
		}, worker_thread));
#endif 
		m_workers.push_back(worker_thread);
	}
	if (nNewlyCreated > 0) {
		std::string sName;
		if (nProcessorQueueID == ResourceRequestID_Local)
			sName = "Local";
		else if (nProcessorQueueID == ResourceRequestID_Asset)
			sName = "Asset";
		else if (nProcessorQueueID == ResourceRequestID_Web)
			sName = "Web";
		else if (nProcessorQueueID == ResourceRequestID_Asset_BigFile)
			sName = "Asset_BigFile";
		else if (nProcessorQueueID == ResourceRequestID_AudioFile)
			sName = "AudioFile";
		OUTPUT_LOG("CAsyncLoader QueueID:%d(%s) has %d worker threads\n", nProcessorQueueID, sName.c_str(), nMaxCount);
	}
	return true;
}

int ParaEngine::CAsyncLoader::GetWorkerThreadsCount(int nProcessorQueueID)
{
	int nCount = 0;
	int nWorkerCount = (int)(m_workers.size());
	for (int i = 0; i < nWorkerCount; ++i)
	{
		if (m_workers[i]->GetProcessorQueueID() == nProcessorQueueID)
		{
			nCount++;
		}
	}
	return nCount;
}

void ParaEngine::CAsyncLoader::SetProcessorQueueSize(int nProcessorQueueID, int nSize)
{
	if (nProcessorQueueID >= 0 && nProcessorQueueID < MAX_PROCESS_QUEUE && nSize>0)
		m_ProcessQueues[nProcessorQueueID].set_capacity(nSize);
}

int ParaEngine::CAsyncLoader::GetProcessorQueueSize(int nProcessorQueueID)
{
	if (nProcessorQueueID >= 0 && nProcessorQueueID < MAX_PROCESS_QUEUE)
		return (int)m_ProcessQueues[nProcessorQueueID].capacity();
	else
		return 0;
}

int ParaEngine::CAsyncLoader::GetEstimatedSizeInBytes()
{
	ParaEngine::Lock lock_(m_request_stats);
	return m_nRemainingBytes;
}

int ParaEngine::CAsyncLoader::GetItemsLeft(int nItemType)
{
	if(nItemType == -1)
	{
		ParaEngine::Lock lock_(m_request_stats);
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
	if(GetItemsLeft()> 0)
	{
		OUTPUT_LOG("CAsyncLoader waiting for %d pending resources. \n", GetItemsLeft());
		ProcessDeviceWorkItems( 100000, false );

		// let us wait only limited time. 30*100 = 3000ms 
		for(int i=0; i<30; ++i)
		{
			// Only exit when all resources are loaded
			if( 0 == GetItemsLeft())
				return;

			// Service Queues
			ProcessDeviceWorkItems( 100000, false );
			SLEEP( 100);
		}
		if(GetItemsLeft() != 0)
		{
			OUTPUT_LOG("warning: CAsyncLoader still have %d pending resources, but we will not wait for their completion. \n", GetItemsLeft());
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


	auto count = m_io_threads.size();
	if (count > 0)
	{
		SetIOThreadCount(count);
		for (size_t i = 0; i < count; i++)
		{
            ResourceRequest_ptr msg(new ResourceRequest(ResourceRequestType_Quit));
            m_IOQueue.push(msg);
		}
        for (size_t i = 0; i < count; i++)
        {
            m_io_threads[i]->join();
        }
		m_io_threads.clear();
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

void CAsyncLoader::SetIOThreadCount(int count)
{
	if (m_io_threads.size() < 1)
		return;

	if (count < 1)
		count = 1;
	if (count > m_io_threads.size())
		count = m_io_threads.size();

	if (count != m_UsedIOThread)
	{
		//m_io_semaphore.reset(count);
		m_UsedIOThread = count;
	}
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
	OUTPUT_LOG("CAsyncLoader::Start, %d\n", std::thread::hardware_concurrency());

	{
#ifdef EMSCRIPTEN
		auto count = 1;
#else
		// auto count = (std::max)(std::thread::hardware_concurrency() -1, (unsigned int)1);
		auto count = DEFAULT_IO_THREADS;
#endif
		m_UsedIOThread = count;
		m_io_threads.resize(m_UsedIOThread);

		//m_io_semaphore.reset(1);

		for (unsigned int i = 0; i < count; i++)
		{
			auto& t = m_io_threads[i];
#ifndef EMSCRIPTEN_SINGLE_THREAD
			t.reset(new std::thread(&CAsyncLoader::FileIOThreadProc, this, i + 1));
#else
			t.reset(CoroutineThread::StartCoroutineThread([this](CoroutineThread* co_thread) -> CO_ASYNC {
				unsigned int id = (unsigned int)(co_thread->GetThreadData());

				ResourceRequest_ptr ResourceRequest;
				HRESULT hr = S_OK;

				ASSETS_LOG(Log_All, "CAsyncLoader IO Thread started");

				OUTPUT_LOG("CAsyncLoader IO Thread started"); // deleted

				int nRes = 0;
				while(nRes != -1)
				{
					while (id > m_UsedIOThread) 
					{
						CO_AWAIT(co_thread->Sleep(10));
					}

					while (!m_IOQueue.try_pop(ResourceRequest))
					{
						CO_AWAIT(co_thread->Sleep(100));
					}
					if(ResourceRequest->m_nType == ResourceRequestType_Quit)
					{
						break;
					}
					hr = FileIOThreadProc_HandleRequest(ResourceRequest);
				}
			}, (void*)(i+1)));
#endif
		}
	}

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
	

	OUTPUT_LOG("CAsyncLoader is started with 1 IO thread and %d worker threads\n", (int)m_workers.size());

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

int CAsyncLoader::FileIOThreadProc_HandleRequest(ResourceRequest_ptr& ResourceRequest)
{
	HRESULT hr = S_OK;

	// Handle a read request
	if (!ResourceRequest->m_bCopy)
	{
		// ASSETS_LOG(Log_Debug, "DEBUG: IO msg(to proc) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

		if (!ResourceRequest->m_bError)
		{
			// Load the data
			hr = ResourceRequest->m_pDataLoader->Load();

			if (FAILED(hr))
			{
				hr = ResourceRequest->m_pDataLoader->Load();
				const char * keyname = ResourceRequest->m_pDataLoader->GetKeyName();
				if (keyname == 0)
					keyname = "key not set";
				const char * filename = ResourceRequest->m_pDataLoader->GetFileName();
				if (filename == 0)
					filename = "unknown file";
				if (hr != E_PENDING)
				{
					OUTPUT_LOG("FileIO Error: hr = %x; key=%s; filename=%s\n", hr, keyname, filename);
				}

				ResourceRequest->m_bError = true;
				ResourceRequest->m_last_error_code = hr;
				if (ResourceRequest->m_pHR)
					*ResourceRequest->m_pHR = hr;
				ResourceRequest->m_pDataProcessor->SetResourceError();
			}
		}

		// Add it to the ProcessQueue
		if (m_ProcessQueues[ResourceRequest->m_nProcessorQueueID].try_push(ResourceRequest) == CResourceRequestQueue::BufferOverFlow)
		{
			OUTPUT_LOG("ERROR: AsyncLoader IO msg(to proc) failed push to m_ProcessQueues because queue is full\n");
		}
	}

	// Handle a copy request
	else
	{
		// ASSETS_LOG(Log_Debug, "DEBUG: IO msg(COPY) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

		if (!ResourceRequest->m_bError)
		{
			// Create the data
			hr = ResourceRequest->m_pDataProcessor->CopyToResource();

			if (FAILED(hr))
			{
				if (hr != E_PENDING)
				{
					OUTPUT_LOG("Failed to Copy Data to Device Object:hr = %x; %s\n", hr, ResourceRequest->m_pDataLoader->GetFileName());
				}

				ResourceRequest->m_bError = true;
				ResourceRequest->m_last_error_code = hr;
				if (ResourceRequest->m_pHR)
					*ResourceRequest->m_pHR = hr;
			}
		}
		else
		{
			ResourceRequest->m_pDataProcessor->SetResourceError();
		}

		// send an unlock request
		ResourceRequest->m_bLock = false;
		if (ResourceRequest->m_pDataProcessor->IsDeviceObject())
		{
			if (m_RenderThreadQueue.try_push(ResourceRequest) == m_RenderThreadQueue.BufferOverFlow)
			{
				OUTPUT_LOG("ERROR: AsyncLoader IO msg(COPY) failed push to m_RenderThreadQueue because queue is full\n");
			}
		}
		else
		{
			ProcessDeviceWorkItemImp(ResourceRequest);
		}
	}
	return hr;
}

int CAsyncLoader::FileIOThreadProc(unsigned int id)
{
	ResourceRequest_ptr ResourceRequest;
	HRESULT hr = S_OK;

	ASSETS_LOG(Log_All, "CAsyncLoader IO Thread started");

	OUTPUT_LOG("CAsyncLoader IO Thread started"); // deleted

	int nRes = 0;
	while(nRes != -1)
	{
		//m_io_semaphore.wait();
		while (id > m_UsedIOThread)
			std::this_thread::sleep_for(std::chrono::milliseconds(3));

		m_IOQueue.wait_and_pop(ResourceRequest);
		if(ResourceRequest->m_nType == ResourceRequestType_Quit)
		{
			break;
		}
		hr = FileIOThreadProc_HandleRequest(ResourceRequest);
		//m_io_semaphore.post();
	}
	return 0;
}

#ifdef EMSCRIPTEN_SINGLE_THREAD
CO_ASYNC CAsyncLoader::ProcessingCorountineThreadProc(CoroutineThread* co_thread)
{
	ResourceRequest_ptr ResourceRequest;
	HRESULT hr = S_OK;
	ProcessorWorkerThread* pThreadData = (ProcessorWorkerThread*)(co_thread->GetThreadData());
	if(pThreadData == 0) CO_RETURN;

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
		ASSETS_LOG(Log_All, "Async Processing Thread %s(%d) Started\n", ThreadType, nQueueID);
	}

	int nRes = 0;

	while(nRes != -1)
	{
		while (!m_ProcessQueues[nQueueID].try_pop(ResourceRequest)) 
		{
			CO_AWAIT(co_thread->Sleep(300));
		}

		if(ResourceRequest->m_nType == ResourceRequestType_Quit)
		{
			break;
		}
		// let us sleep some time to emulate slow connection for debugging purposes.
		// Sleep(300);

		// ASSETS_LOG(Log_All, "DEBUG: process msg %s\n", ResourceRequest->m_pDataLoader->GetFileName());
		
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

		
		ResourceRequest->m_bLock = true;
		if (ResourceRequest->m_pDataProcessor->IsDeviceObject())
		{
			// Add it to the RenderThreadQueue
			if (m_RenderThreadQueue.try_push(ResourceRequest) == m_RenderThreadQueue.BufferOverFlow)
			{
				OUTPUT_LOG("ERROR: AsyncLoader process msg failed push to m_RenderThreadQueue because queue is full \n");
			}
		}
		else
		{
			ProcessDeviceWorkItemImp(ResourceRequest);
		}
	}
}
#endif

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
		ASSETS_LOG(Log_All, "Async Processing Thread %s(%d) Started\n", ThreadType, nQueueID);
	}

	int nRes = 0;
	while(nRes != -1)
	{
		// auto queue = m_ProcessQueues[nQueueID];
		m_ProcessQueues[nQueueID].wait_and_pop(ResourceRequest);
		if(ResourceRequest->m_nType == ResourceRequestType_Quit)
		{
			break;
		}
		// let us sleep some time to emulate slow connection for debugging purposes.
		// Sleep(300);

		// ASSETS_LOG(Log_All, "DEBUG: process msg %s\n", ResourceRequest->m_pDataLoader->GetFileName());
		
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

		
		ResourceRequest->m_bLock = true;
		if (ResourceRequest->m_pDataProcessor->IsDeviceObject())
		{
			// Add it to the RenderThreadQueue
			if (m_RenderThreadQueue.try_push(ResourceRequest) == m_RenderThreadQueue.BufferOverFlow)
			{
				OUTPUT_LOG("ERROR: AsyncLoader process msg failed push to m_RenderThreadQueue because queue is full \n");
			}
		}
		else
		{
			ProcessDeviceWorkItemImp(ResourceRequest);
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

#ifdef EMSCRIPTEN
	auto async_callback = [this, pDataLoader, pDataProcessor, pHResult, ppDeviceObject, nProcessorThreadID] () {
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
	};

	if (!pDataProcessor->AsyncProcess(async_callback)) async_callback();

	return S_OK;
#else
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
#endif
}

int ParaEngine::CAsyncLoader::AddWorkItem( ResourceRequest_ptr& msg)
{
	const char* name = msg->m_pDataLoader->GetFileName();
	if(name == 0)
		name = "unknown";

	if(msg->m_nProcessorQueueID == ResourceRequestID_Local)
	{
		ASSETS_LOG(Log_All, "AssetRequested %s\n", name);
	}
	else
	{
		ASSETS_LOG(Log_Remote, "AddWorkItem(%d) %s\n", (int)(msg->m_nProcessorQueueID), name);
	}
	int nSizeBytes = msg->m_pDataLoader->GetEstimatedSizeInBytes();

	if(this->m_IOQueue.try_push(msg) != this->m_IOQueue.BufferOverFlow)
	{
		ParaEngine::Lock lock_(this->m_request_stats);
		this->m_nRemainingBytes += nSizeBytes;
		this->m_NumOutstandingResources ++;
	}
	else
	{
		OUTPUT_LOG("warning: there is no more room in the CAsyncLoader worker queue for asset file %s\n", name);
	}
	return S_OK;
}

void ParaEngine::CAsyncLoader::ProcessDeviceWorkItemImp(ResourceRequest_ptr& ResourceRequest, bool bRetryLoads)
{
	HRESULT hr = S_OK;
	if (ResourceRequest->m_bLock)
	{
		 //ASSETS_LOG(Log_Debug, "DEBUG: render msg(lock) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

		if (!ResourceRequest->m_bError)
		{
			hr = ResourceRequest->m_pDataProcessor->LockDeviceObject();
			if (E_TRYAGAIN == hr && bRetryLoads && ResourceRequest->m_pDataProcessor->IsDeviceObject())
			{
				// add it back to the list
				m_RenderThreadQueue.push(ResourceRequest);

				// move on to the next guy
				return;
			}
			else if (FAILED(hr))
			{
				ResourceRequest->m_bError = true;
				ResourceRequest->m_last_error_code = hr;
				if (ResourceRequest->m_pHR)
					*ResourceRequest->m_pHR = hr;
			}
		}

		ResourceRequest->m_bCopy = true;
		if (ResourceRequest->m_pDataProcessor->IsDeviceObject())
		{
			if (m_IOQueue.try_push(ResourceRequest) == m_IOQueue.BufferOverFlow)
			{
				OUTPUT_LOG("ERROR: AsyncLoader render msg(lock) failed push to m_IOQueue because queue is full\n");
			}
		}
		else
		{
			FileIOThreadProc_HandleRequest(ResourceRequest);
		}
	}
	else
	{
		// ASSETS_LOG(Log_Debug, "DEBUG: render msg(unlock) %s\n", ResourceRequest->m_pDataLoader->GetFileName());

		if (!ResourceRequest->m_bError)
		{
			HRESULT hr = ResourceRequest->m_pDataProcessor->UnLockDeviceObject();
			ResourceRequest->m_last_error_code = hr;
			if (ResourceRequest->m_pHR)
				*ResourceRequest->m_pHR = hr;

			const char* name = ResourceRequest->m_pDataLoader->GetFileName();
			if (name == 0)
				name = "unknown";
			ASSETS_LOG(Log_All, "AssetLoaded %s\n", name);
		}
		else
		{
			if (ResourceRequest->m_last_error_code != E_PENDING)
			{
				const char* sFileName = ResourceRequest->m_pDataLoader->GetFileName();
				if (sFileName && sFileName[0] != '\0')
				{
					// bug fix: we only set resource error if file name is not ""
					// for some reason, if we set resource error for empty file, some models will not show up.  Empty file is considered valid. 
					ASSETS_LOG(Log_Error, "ERROR: AssetFailed (%d)%s\n", ResourceRequest->m_last_error_code, sFileName);
					ResourceRequest->m_pDataProcessor->SetResourceError();
				}
			}
		}

		{
			ParaEngine::Lock lock_(m_request_stats);
			m_nRemainingBytes -= ResourceRequest->m_pDataLoader->GetEstimatedSizeInBytes();
		}

		ResourceRequest->m_pDataLoader->Destroy();
		ResourceRequest->m_pDataProcessor->Destroy();

		SAFE_DELETE(ResourceRequest->m_pDataLoader);
		SAFE_DELETE(ResourceRequest->m_pDataProcessor);

		{
			ParaEngine::Lock lock_(m_request_stats);
			// Decrement num outstanding resources
			m_NumOutstandingResources--;
		}
	}
}

void ParaEngine::CAsyncLoader::ProcessDeviceWorkItems( int CurrentNumResourcesToService, bool bRetryLoads )
{
	HRESULT hr = S_OK;
	ResourceRequest_ptr ResourceRequest;

	for( int i = 0; (i<CurrentNumResourcesToService) && m_RenderThreadQueue.try_pop(ResourceRequest); i++ )
	{
		ProcessDeviceWorkItemImp(ResourceRequest, bRetryLoads);
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

int ParaEngine::CAsyncLoader::GetLogLevel() const
{
	return m_nLogLevel;
}

void ParaEngine::CAsyncLoader::SetLogLevel(int val)
{
	m_nLogLevel = (ParaEngine::CAsyncLoader::AssetLogLevelEnum)val;
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

int ParaEngine::CAsyncLoader::InstallFields(CAttributeClass * pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	pClass->AddField("EstimatedSizeInBytes", FieldType_Int, (void*)0, (void*)GetEstimatedSizeInBytes_s, NULL, "", bOverride);
	pClass->AddField("ItemsLeft", FieldType_Int, (void*)0, (void*)GetItemsLeft_s, NULL, "", bOverride);
	pClass->AddField("BytesProcessed", FieldType_Int, (void*)0, (void*)GetBytesProcessed_s, NULL, "", bOverride);

	pClass->AddField("WorkerThreadsCount", FieldType_Vector2, (void*)SetWorkerThreads_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("ProcessorQueueSize", FieldType_Vector2, (void*)SetProcessorQueueSize_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("LogLevel", FieldType_Int, (void*)SetLogLevel_s, (void*)GetLogLevel_s, NULL, "", bOverride);
	pClass->AddField("log", FieldType_String, (void*)log_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("WaitForAllItems", FieldType_void, (void*)WaitForAllItems_s, (void*)0, NULL, "", bOverride);

	pClass->AddField("IOThreadsCount", FieldType_Int, (void*)SetIOThreadCount_s, (void*)0, NULL, "", bOverride);
	
	return S_OK;
	return 0;
}


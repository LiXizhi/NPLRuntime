#pragma once
#include "IAttributeFields.h"
#include "IDataLoader.h"
#include "util/LogService.h"
#include <vector>
#include <set>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#else
#include "util/CoroutineThread.h"
#endif
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <thread>
#include <atomic>


/** try again */
#define E_TRYAGAIN  -123456

/** the max number of async process queues. Please note that each process queue matches to one or more processor thread. 
And each processor thread can be associated with just one processor queue. Please note that, following are internal queues: 
- queue[0] is for local CPU intensive tasks like unzip. (only one thread process it)
- queue[1] is for remote background asset loading. (4 threads process it)
- queue[2] is for remote REST URL request. (2 threads process it)
@note: Threads are only created when the first message is inserted to the queue. 
*/
#define MAX_PROCESS_QUEUE 16

#ifdef EMSCRIPTEN_SINGLE_THREAD
typedef CoroutineThread npl_thread;
#else
typedef boost::thread npl_thread;
#endif

namespace ParaEngine
{
	class CDirectXEngine;
	class CGDIEngine;
	//--------------------------------------------------------------------------------------
	// Structures
	//--------------------------------------------------------------------------------------
	
	/**
	It uses architecture proposed by the content streaming sample in DirectX 9&10

	Three types of threads come into play. 
	- The first is the render thread. The render thread's main job is to draw the scene without stalling. 
		Lesser jobs of the rendering thread include sending I/O requests to the I/O thread by way of the request queue, 
		and handling lock and unlock requests from the I/O thread. To keep the render thread from stalling, both of these lesser jobs are handled when it is convenient for the render thread. 
	- The second type of thread is the I/O thread. The I/O thread handles requests from the render thread. 
		It also either loads files from the packed file or gets memory-mapped pointers to them. 
		The I/O thread handles communications with the various process threads and also places lock/unlock requests into the lock/unlock queue for consumption by the render thread. 
		In addition, on Direct3D 9, the I/O thread does the actual copying of bits into the device object. 
		There is generally one I/O thread for each storage device that will be used on the system. 

	- The last type of thread is the process thread. Process threads handle the dirty work of processing the data after it has been loaded from disk by the I/O thread. 
		Process threads can decompress data, prefetch data when using memory-mapped I/O, or handle other operations, such as swizzling, 
		that may need to be performed on the data before it is copied into the device object. 
		There should generally be a single process thread for each available hardware thread. 

	The typical chain of events is as follows: 
		1.The I/O thread and the process threads are sleeping, waiting for something to do. The graphics thread is rendering away.
		2.The graphics thread determines that a resource is going to be visible sometime in the near future. It places a resource request on the request queue.
		3.The I/O thread picks this request up and loads the data from the packed file or download from network. 
		4.The I/O thread then sends a process request to one of the waiting process threads. 
			The processing thread performs some computation on the data. Such as checking for integrity and decompression. 
		5.When the process thread is done processing, it puts a lock request into the lock/unlock queue of the render thread.
		6.When the render thread feels like it, it picks up the lock request, and locks the resource.
		7.The render thread then sends the request back to the I/O thread. On Direct3D 9, the I/O thread copies the data into the device object.
		8.Then the I/O thread puts an unlock request in the lock/unlock queue.
		9.When the render thread feels like it, it picks up the unlock request and unlocks the resource. The resource is now ready to use.
	*/
	class CAsyncLoader : public IAttributeFields
	{
	public:
		enum AssetLogLevelEnum
		{
			Log_All = 0,
			Log_Debug,
			Log_Remote,
			Log_Warn,
			Log_Error,
		};

		typedef boost::shared_ptr<npl_thread> Boost_Thread_ptr_type;

		CAsyncLoader();
		virtual ~CAsyncLoader();

		ATTRIBUTE_DEFINE_CLASS(CAsyncLoader);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CAsyncLoader, GetEstimatedSizeInBytes_s, int*) { *p1 = cls->GetEstimatedSizeInBytes(); return S_OK; }
		ATTRIBUTE_METHOD1(CAsyncLoader, GetItemsLeft_s, int*) { *p1 = cls->GetItemsLeft(); return S_OK; }
		ATTRIBUTE_METHOD1(CAsyncLoader, GetBytesProcessed_s, int*) { *p1 = cls->GetBytesProcessed(); return S_OK; }
		
		ATTRIBUTE_METHOD1(CAsyncLoader, SetWorkerThreads_s, Vector2) { cls->CreateWorkerThreads((int)p1.x, (int)p1.y); return S_OK; }
		
		ATTRIBUTE_METHOD1(CAsyncLoader, SetProcessorQueueSize_s, Vector2) { cls->SetProcessorQueueSize((int)p1.x, (int)p1.y); return S_OK; }

		ATTRIBUTE_METHOD1(CAsyncLoader, GetLogLevel_s, int*) { *p1 = cls->GetLogLevel(); return S_OK; }
		ATTRIBUTE_METHOD1(CAsyncLoader, SetLogLevel_s, int) { cls->SetLogLevel(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAsyncLoader, log_s, const char*) { cls->log(p1); return S_OK; }
		ATTRIBUTE_METHOD(CAsyncLoader, WaitForAllItems_s) { cls->WaitForAllItems(); return S_OK; }

		ATTRIBUTE_METHOD1(CAsyncLoader, SetIOThreadCount_s, int) { cls->SetIOThreadCount(p1); return S_OK; }
		
	private:
		struct ProcessorWorkerThread;
	public:
		/** get singleton instance. one needs to call start() before adding work items. */
		static CAsyncLoader& GetSingleton();

		/** clean up everything, exit all threads created. */
		void CleanUp();

		/** call this only once to start async loader 
		* @param nWorkerCount: how many worker threads to spawn.
		*/
		int Start(int nWorkerCount = 5);

		/** stop everything. this is like cleaning up. */
		int Stop();

		/** Add a work item to the queue of work items
		* Only call this from graphics thread
		@ param nProcessorThreadID: default to 0. it can also be ResourceRequestID
		*/
		int AddWorkItem( IDataLoader* pDataLoader, IDataProcessor* pDataProcessor, HRESULT* pHResult, void** ppDeviceObject, int nProcessorThreadID=0);

		/** Add a work item to the queue of work items 
		* Only call this from graphics thread
		*/
		int AddWorkItem( ResourceRequest_ptr& request );

		/** this is same as AddWorkItem, except that it is a synchronous function. and will only return after everything is processed. 
		* @note: use AddWorkItem instead if possible.
		*/
		HRESULT RunWorkItem( IDataLoader* pDataLoader, IDataProcessor* pDataProcessor, HRESULT* pHResult, void** ppDeviceObject);

		/** this is same as AddWorkItem, except that it is a synchronous function. and will only return after everything is processed. 
		* @note: use AddWorkItem instead if possible.
		*/
		HRESULT RunWorkItem( ResourceRequest_ptr& request );
		
		/** 
		* ProcessDeviceWorkItems is called by the graphics thread.  Depending on the request
		* it either Locks or Unlocks a resource (or calls UpdateSubresource for D3D10).  
		* Only call this from graphics thread at regular interval such as in the render frame move. 
		* @param CurrentNumResourcesToService: the number of resources to service.  This ensure that no matter
		* how many items are in the queue, the graphics thread doesn't stall trying to process all of them.
		* @param bRetryLoads: if true, we will retry if failed and retry is requested by the loader. 
		*/
		void ProcessDeviceWorkItems( int CurrentNumResourcesToService=100, bool bRetryLoads = false );
		/* this function is usually called by the render thread, but it may also be called in IO or worker thread is IsDeviceObject() is false. */
		void ProcessDeviceWorkItemImp(ResourceRequest_ptr& pResourceRequest, bool bRetryLoads = false);

		/** make sure that there are nMaxCount workers threads processing the queue at nProcessorQueueID. 
		* @note: worker threads of internal queues are created internally. 
		* [NOTE thread safe] function must be called by the main thread. 
		* @param nProcessorQueueID: [0, 16). And each processor thread can be associated with just one processor queue. Please note that, following are internal queues: 
		- queue[0] is for local CPU intensive tasks like unzip. (only one thread process it)
		- queue[1] is for remote background asset loading. (4 threads process it)
		- queue[2] is for remote REST URL request. (2 threads process it)
		* @param nMaxCount: the max number of threads that can coexist for the nProcessorQueueID queue. 
		* @return true if success. 
		*/
		bool CreateWorkerThreads(int nProcessorQueueID, int nMaxCount);
		int GetWorkerThreadsCount(int nProcessorQueueID);
		/** message queue size of a given processor id*/
		void SetProcessorQueueSize(int nProcessorQueueID, int nSize);
		int GetProcessorQueueSize(int nProcessorQueueID);

		/** Wait for all work in the queues to finish. 
		* Only call this from graphics thread
		*/
		void WaitForAllItems();

#ifdef PARAENGINE_CLIENT
		/** get the file parser used by the IO thread. */
		inline LPD3DXFILE GetFileParser() { return m_pXFileParser; };

		/** get directX device used in the IO thread for background texture composition.*/
		CDirectXEngine* GetEngine();

		/** get GDI device used in the IO thread for background texture composition.*/
		CGDIEngine* GetGDIEngine();
#endif

		/** add a string url to a set. this is used for tracking in processing requests. 
		* this function is called automatically when AppendURLRequest is called. 
		* [thread safe]
		*/
		void AddPendingRequest(const char* sURL);

		/** whether the given request is already in the pending set. 
		* one can use this function to check whether there is a pending request due to previous call of AppendURLRequest
		* [thread safe]
		*/
		bool HasPendingRequest(const char* sURL);

		/** remove a string url from a set. This function is automatically called when a url request is completed. 
		* [thread safe]
		*/
		void RemovePendingRequest(const char* sURL);

		/** clear all pending request. 
		* [thread safe]
		*/
		void ClearAllPendingRequests();

		int GetLogLevel() const;
		void SetLogLevel(int val);

		/** write formated text to "asset.log". the input is same to printf */
		void log(const string& msg);
		void log(int nLogLevel, const string& msg);

		/** this is a global interrupt signal. Once set, all thread should try to exit ASAP*/
		inline bool interruption_requested(){return m_bInterruptSignal;}

		/** call this function to interrupt all threads. This function returns immediately. Usually we call this function first, 
		and then call thread.timed_join() or join(). */
		void Interrupt();

		/**
		* Get the number of items left in the asynchronous content loader queue. 
		* @param nItemType: 
		*  -2 : all remote assets(excluding HTTP web requests) in the queue. 
		*  -1 : all outstanding resources in the queue. 
		*	0 : all local requests in the queue. 
		*	1 : Smaller remote requests in the queue.
		*	2 : HTTP web requests in the queue.
		*	3 : big remote requests in the queue.
		*/
		int GetItemsLeft(int nItemType = -1);

		/** get total estimated size in bytes. 
		*/
		int GetEstimatedSizeInBytes();

		/** get the total number of bytes processed. 
		* @param nItemType: 
		*  -2 : all remote assets(excluding HTTP web requests) in the queue. 
		*  -1 : all url requests(include web request) in the queue. 
		*	0 : all local requests in the queue. 
		*	1 : Smaller remote requests in the queue.
		*	2 : HTTP web requests in the queue.
		*	3 : big remote requests in the queue.
		*/
		int GetBytesProcessed(int nItemType = -1);

		/* count >= 1 and count <= cpu number - 1  */
		void SetIOThreadCount(int count);

	protected:

		/** 
		// This is the one IO threadproc.  This function is responsible for processing read
		// requests made by the application.  There should only be one IO thread per device.  
		// This ensures that the disk is only trying to read one part of the disk at a time.
		//
		// This thread performs double-duty as the copy thread as well.  It manages the copying
		// of resource data from temporary system memory buffer (or memory mapped pointer) into
		// the locked data of the resource.
		*/
		int FileIOThreadProc(unsigned int id);
		/** this is usually called by FileIOThreadProc(), but may be called by other thread as well if IsDeviceObject() is false.*/
		int FileIOThreadProc_HandleRequest(ResourceRequest_ptr& ResourceRequest);

		/**
		This is the threadproc for the processing thread.  There are multiple processing
		threads.  The job of the processing thread is to uncompress, unpack, or otherwise
		manipulate the data loaded by the loading thread in order to get it ready for the
		ProcessDeviceWorkItems function in the graphics thread to lock or unlock the resource.
		* @param pThreadData: some thread local data, such as which process message queue this thread should read from. 
		*/
		int ProcessingThreadProc(ProcessorWorkerThread* pThreadData);
		
	private:
		
		bool m_bDone;
		bool m_bProcessThreadDone;
		bool m_bIOThreadDone;

		/** this is a global interrupt signal. Once set, all thread should try to exit ASAP*/
		volatile bool m_bInterruptSignal;

		int m_NumResourcesToService;
		/** total number unfinished items. */
		int m_NumOutstandingResources;
		/** total number of bytes unfinished. */
		int m_nRemainingBytes;

		CResourceRequestQueue m_IOQueue;
		CResourceRequestQueue m_RenderThreadQueue;
		/** multiple processor queues. we can think of this like parallel message channels. 
		And each processor thread can be associated with just one processor queue. Please note that, following are internal queues: 
		- queue[0] is for local CPU intensive tasks like unzip. (only one thread process it)
		- queue[1] is for remote background asset loading (small and medium sized file only, such as smaller than 1MB). (2 threads process it)
		- queue[2] is for remote REST URL request. (1 threads process it)
		- queue[3] is for remote background asset loading (very big file only, such as larger than 1MB). (1 threads process it)
		enum ResourceRequestID can be used for index. 
		*/
		CResourceRequestQueue m_ProcessQueues[MAX_PROCESS_QUEUE];


		/** Thread used for running the m_io_service_dispatcher 's run loop for dispatching messages for all NPL Jabber Clients */
		Boost_Thread_ptr_type m_io_thread;
		std::vector<boost::shared_ptr<npl_thread>> m_io_threads;
		//int m_UsedIOThread;
		//Semaphore m_io_semaphore;
		std::atomic_int m_UsedIOThread;

		/** the worker thread. */
		struct ProcessorWorkerThread : public IProcessorWorkerData
		{
		public:
			ProcessorWorkerThread();
			ProcessorWorkerThread(int nQueueID);

			~ProcessorWorkerThread();
		public:
			void reset(npl_thread * pThread) {m_thread.reset(pThread);}
			void reset() {m_thread.reset();}
			void join() { if(m_thread.get()) m_thread->join();}

			/** If *this refers to a thread of execution, waits for that thread of execution to complete, 
			the time wait_until has been reach or the specified duration rel_time has elapsed. If *this doesn't refer to a thread of execution, returns immediately.
			true if *this refers to a thread of execution on entry, and that thread of execution has completed before the call times out, false otherwise.
			*/
			bool timed_join(int nSeconds);
			
			/** get processor queue id */
			virtual int GetProcessorQueueID() {return m_nQueueID;} 

			/** get the libcurl easy interface that this thread is related to. Normally there is only one curl interface per thread. 
			* curl interface is created on demand. 
			* @param nID: reserved for future use in case there are multiple curl interface per thread.
			*/
			virtual void* GetCurlInterface(int nID = 0);

			/** add number of bytes processed. this function can be called by the IDataProcessor for statistics */
			virtual void AddBytesProcessed(int nBytesProcessed);

			/** get total number of bytes processed. */
			virtual int GetBytesProcessed();
		public:
			//  thread ptr. 
			Boost_Thread_ptr_type m_thread;

			//  which queue to process. 
			int m_nQueueID;

			/** the curl interface. */
			void* m_curl;

			// bytes processed
			volatile int m_nBytesProcessed;
		};

		/** for the RunWorkItem() function */
		struct DefaultWorkerThreadData	: public ProcessorWorkerThread
		{
		public:
			/** the only difference with ProcessorWorkerThread is that it disables signal set CURLOPT_NOSIGNAL to 1.  */
			virtual void* GetCurlInterface(int nID = 0);
		};

		/** processor thread: workers */
		std::vector< ProcessorWorkerThread* > m_workers;

		/** the default processor worker data */
		DefaultWorkerThreadData* m_default_processor_worker_data;

		/** exclusive protection for m_default_processor_worker_data */
		ParaEngine::mutex m_default_processor_mutex;

		/** for logging */
		CServiceLogger_ptr g_asset_logger;
		
		/** default to Log_Remote */
		AssetLogLevelEnum m_nLogLevel;

#ifdef PARAENGINE_CLIENT
		/** another parax file parser used in the IO thread */
		LPD3DXFILE m_pXFileParser;

		/** another directX device used in the IO thread for background texture composition.*/
		CDirectXEngine* m_pEngine;

		/** another GDI+ device used in the IO thread for background texture composition.*/
		CGDIEngine* m_pGDIEngine;
#endif
		// only for pending request. 
		ParaEngine::mutex m_pending_request_mutex;
		// for statistics of request
		ParaEngine::mutex m_request_stats;

		// all pending url, this could prevent the same url to be request multiple times. 
		std::set <std::string> m_pending_requests;
	};
}

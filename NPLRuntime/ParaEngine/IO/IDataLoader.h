#pragma once
#include "NPLMessageQueue.h"
#include <boost/noncopyable.hpp>

namespace ParaEngine
{
	/**
	* IDataLoader is an interface that the AsyncLoader class uses to load data from disk.
	* 
	* Load is called from the IO thread to load data.
	* Decompress is called by one of the processing threads to decompress the data.
	* Destroy is called by the graphics thread when it has consumed the data.
	*/
	class IDataLoader
	{
	public:
		virtual ~IDataLoader(){};

		/** get the file name */
		virtual const char* GetFileName() {return 0;};

		/** get the key name */
		virtual const char* GetKeyName() {return 0;};
		
		/** Decompress is called by one of the processing threads to decompress the data.*/
		virtual HRESULT Decompress( void** ppData, int* pcBytes ) = 0;
		/** Destroy is called by the graphics thread when it has consumed the data, unless IsDeviceObject() is false. */
		virtual HRESULT Destroy() = 0;
		/** Load is called from the IO thread to load data. */
		virtual HRESULT Load() = 0;

		/** this function is solely used in statistics reporting. can only be called once immediately after a data loader is constructed. */
		virtual void SetEstimatedSizeInBytes(int nSize) {};

		/** this function is solely used in statistics reporting. */
		virtual int GetEstimatedSizeInBytes() {return 0;};

		/** default to true. If not true, Destroy() will be called in the worker thread instead of render thread. */
		virtual bool IsDeviceObject() { return true; };
	};


	/** interface of processor worker data */
	class IProcessorWorkerData
	{
	public:
		virtual int GetProcessorQueueID(){return 0;};

		/** get the libcurl easy interface that this thread is related to. Normally there is only one curl interface per thread. 
		* curl interface is created on demand. 
		* @param nID: reserved for future use in case there are multiple curl interface per thread.
		*/
		virtual void* GetCurlInterface(int nID = 0) {return NULL;};

		/** add number of bytes processed. this function can be called by the IDataProcessor for statistics */
		virtual void AddBytesProcessed(int nBytesProcessed) {};

		/** get total number of bytes processed. */
		virtual int GetBytesProcessed() {return 0;};
	};

	/**
	* IDataProcessor is an interface that the AsyncLoader class uses to process and copy
	* data into locked resource pointers.
	* 
	* Process is called by one of the processing threads to process the data before it is
	*   consumed.
	* LockDeviceObject is called from the Graphics thread to lock the device object (D3D9).
	* UnLockDeviceObject is called from the Graphics thread to unlock the device object, or
	*   to call updatesubresource for D3D10.
	* CopyToResource copies the data from memory to the locked device object (D3D9).
	* SetResourceError is called to set the resource pointer to an error code in the event
	*   that something went wrong.
	* Destroy is called by the graphics thread when it has consumed the data.
	*/
	class IDataProcessor
	{
	public:
		virtual ~IDataProcessor(){};

		virtual bool AsyncProcess(std::function<void()>) { return false; }

		/** LockDeviceObject is called from the Graphics thread to lock the device object (D3D9), unless IsDeviceObject() is false. */
		virtual HRESULT LockDeviceObject() = 0;
		/** UnLockDeviceObject is called from the Graphics thread to unlock the device object, or call updatesubresource for D3D10, unless IsDeviceObject() is false. */
		virtual HRESULT UnLockDeviceObject() = 0;
		/** Destroy is called by the graphics thread when it has consumed the data, unless IsDeviceObject() is false.  */
		virtual HRESULT	Destroy() = 0;
		/** Process is called by one of the processing threads to process the data before it is consumed.*/
		virtual HRESULT Process( void* pData, int cBytes ) = 0;
		/** CopyToResource copies the data from memory to the locked device object (D3D9). Also by the IO thread. */
		virtual HRESULT CopyToResource() = 0;
		/** SetResourceError is called to set the resource pointer to an error code in the event that something went wrong.*/
		virtual void SetResourceError() = 0;

		/** set thread local data. */
		virtual void SetProcessorWorkerData(IProcessorWorkerData * pThreadLocalData ) {};
		/** get thread local data. It may return NULL if the processor does not support thread local data. */
		virtual IProcessorWorkerData * GetProcessorWorkerData() {return NULL;};

		/** default to true. If not true, LockDeviceObject, UnLockDeviceObject are not called and Destroy will be called in the worker thread instead of render thread. */
		virtual bool IsDeviceObject() { return true; };
	};


	/** resource request type. */
	enum ResourceRequestType
	{
		ResourceRequestType_Local,
		ResourceRequestType_Asset,
		ResourceRequestType_Web,
		ResourceRequestType_Quit,
	};

	/** predefined resource request id. */
	enum ResourceRequestID
	{
		ResourceRequestID_Local = 0,
		ResourceRequestID_Asset,
		ResourceRequestID_Web,
		ResourceRequestID_Asset_BigFile,
		ResourceRequestID_AudioFile,
	};

	/** ParaEngine resource request. */
	struct ResourceRequest : 
		public ParaEngine::PoolBase<ResourceRequest>,
		public ParaEngine::intrusive_ptr_thread_safe_base,
		private boost::noncopyable
	{
		ResourceRequest(ResourceRequestType nType = ResourceRequestType_Local):m_nType(nType),m_pDataLoader(NULL), m_pDataProcessor(NULL), m_ppDeviceObject(NULL), m_pHR(NULL), m_nProcessorQueueID(0), m_last_error_code(S_OK) {}
		virtual ~ResourceRequest();

		/** request type */
		ResourceRequestType m_nType;

		IDataLoader* m_pDataLoader;
		IDataProcessor* m_pDataProcessor;
		HRESULT* m_pHR;
		HRESULT m_last_error_code;
		void** m_ppDeviceObject;
		/** process queue id, default to 0. Each process queue matches to one or more processor threads. 
		And each processor thread can be associated with just one processor queue. Please note that, following are internal queues: 
		- queue[0] is for local CPU intensive tasks like unzip. (only one thread process it)
		- queue[1] is for remote background asset loading. (4 threads process it)
		- queue[2] is for remote REST URL request. (2 threads process it)
		@note: internal queues are created automatically. if one wants to use other queues, one needs to create the processor worker first. 
		*/
		int m_nProcessorQueueID;

		bool m_bLock;
		bool m_bCopy;
		bool m_bError;
	};
	typedef ParaIntrusivePtr<ResourceRequest> ResourceRequest_ptr;

	/** the message queue */
	class CResourceRequestQueue : public NPL::concurrent_ptr_queue<ResourceRequest_ptr>
	{
	public:
		CResourceRequestQueue();
		CResourceRequestQueue(int capacity);
		~CResourceRequestQueue();

		BufferStatus try_push(ResourceRequest_ptr& item);
	};
}

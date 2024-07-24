#pragma once
#include "IDataLoader.h"
#include <vector>
/* curl specific */
#include <curl/curl.h>
#include "util/mutex.h"

namespace NPL
{
	class NPLObjectProxy;
}

namespace ParaEngine
{
	class CUrlProcessor;
	class IProcessorWorkerData;

	/**
	* CTextureLoader implementation of IDataLoader
	* it will first search locally. If not found or version expired (as indicated in the assets_manifest file), 
	* we will download from the asset update server. 
	*/
	class CUrlLoader : public IDataLoader
	{
	public:
		/**
		* @param sFileName: if this is "", m_asset->GetLocalFileName() is used. 
		*/
		CUrlLoader();
		CUrlLoader(const string& url);
		~CUrlLoader();

		void CleanUp();

		/** this function is solely used in statistics reporting. can only be called once immediately after a data loader is constructed. */
		virtual void SetEstimatedSizeInBytes(int nSize);

		/** this function is solely used in statistics reporting. */
		virtual int GetEstimatedSizeInBytes();
	public:
		/** get file name */
		const char* GetFileName();

		/** set url to process. */
		void SetUrl(const char* url);

		/** Decompress is called by one of the processing threads to decompress the data.*/
		virtual HRESULT Decompress( void** ppData, int* pcBytes );
		/** Destroy is called by the graphics thread when it has consumed the data. */
		virtual HRESULT Destroy();
		/** Load is called from the IO thread to load data. Load the texture from the packed file.  
		*/
		virtual HRESULT Load();

		virtual bool IsDeviceObject() { return false; };
	private:
		std::string m_url;
		int m_nEstimatedSizeInBytes;
	};

	/** one should derive from this class with virtual destructor. */
	class CUrlProcessorUserData
	{
	public:
		CUrlProcessorUserData(){};
		virtual ~CUrlProcessorUserData(){};
	};

	typedef DWORD (*URL_LOADER_CALLBACK)(int nResult, CUrlProcessor* pRequest, CUrlProcessorUserData* pUserData);

	/** for read function */
	struct upload_context
	{
		upload_context(const char* data, int nDataSize = 0);
		const char* m_pData;
		int m_nDataSize;
		int m_nBytesSent;
	};

	/**
	* CUrlProcessor implementation of IDataProcessor
	*/
	class CUrlProcessor : public IDataProcessor
	{
	public:
		// default time out in milliseconds
		static const DWORD DEFAULT_TIME_OUT = 15000;
		CUrlProcessor();
		CUrlProcessor(const string& url, const string& npl_callback);
		~CUrlProcessor();

		void CleanUp();
	// overrides
	public:
		virtual HRESULT LockDeviceObject();
		virtual HRESULT UnLockDeviceObject();
		virtual HRESULT Destroy();
		virtual HRESULT Process( void* pData, int cBytes );
		virtual HRESULT CopyToResource();
		virtual void    SetResourceError();

		/** set thread local data. */
		virtual void SetProcessorWorkerData(IProcessorWorkerData * pThreadLocalData );
		/** get thread local data. It may return NULL if the processor does not support thread local data. */
		virtual IProcessorWorkerData * GetProcessorWorkerData();

		virtual bool IsDeviceObject() { return false; };
	public:
		/** type of the url request task */
		enum URLREQUEST_TYPE
		{
			URL_REQUEST_HTTP_AUTO,
			URL_REQUEST_HTTP_GET,
			URL_REQUEST_HTTP_POST,
			URL_REQUEST_HTTP_GETFILE,
			URL_REQUEST_HTTP_HEADERS_ONLY,
		};

		/** url request status */
		enum URLREQUEST_STATUS
		{
			// not yet started
			URL_REQUEST_UNSTARTED,
			URL_REQUEST_INCOMPLETE,
			URL_REQUEST_COMPLETED,
		};

	public:
		/** set the url to download */
		void SetUrl(const char* url);

		/** get only headers, without body*/
		void SetHeadersOnly();

		/** set the npl callback script. 
		* @param sCallback: a string callback function. it may begin with (runtime_state_name) such as "(main)my_function()", 
		* if no runtime state is provided, it is the main state(Not the calling thread). This prevents the user to use multiple threads to download to the same file location by mistake. 
		*/
		void SetScriptCallback(const char* sCallback);

		/** to which file we shall save the url request to. This is usually for downloading large web file. 
		* if we specify a file to save to. the callback script will no longer contain the response body. 
		*/
		void SetSaveToFile(const char* filename);
		
		/** append http headers*/
		void AppendHTTPHeader(const char* text);

		/** append form parameters */
		CURLFORMcode AppendFormParam(const char* name, const char* value);

		/** append a file, such as 
		* {name = {file="/tmp/test.txt",	type="text/plain"}}
		* {name = {file="dummy.html",	data="<html><bold>bold</bold></html>, type="text/html"}}
		* input can be NULL if not available. 
		* @param bCacheData: if false, data must be valid until the request is complete (in other thread), otherwise it may be rubbish.
		* if true, we will copy the data and the caller can release the data buffer as soon as the function returns
		*/
		CURLFORMcode AppendFormParam(const char* name, const char* type, const char* file, const char* data, int datalen, bool bCacheData = false);

		/** init the easy handle according to the settings. It will also prepare the task for the curl interface by clearing data, etc. */
		void SetCurlEasyOpt(CURL* handle);

		/** call the call back if any, this function must be called in the main game thread. */
		void CompleteTask();

		/** curl call back. */
		static size_t CUrl_write_data_callback(void *buffer, size_t size, size_t nmemb, void *stream);
		static size_t CUrl_write_header_callback(void *buffer, size_t size, size_t nmemb, void *stream);
		static int CUrl_progress_callback(void *clientp,double dltotal,double dlnow,double ultotal, double ulnow);
		static size_t CUrl_read_email_payload(void *ptr, size_t size, size_t nmemb, void *userp);

		size_t write_data_callback(void *buffer, size_t size, size_t nmemb);
		size_t write_header_callback(void *buffer, size_t size, size_t nmemb);
		int progress_callback(double dltotal, double dlnow, double ultotal, double ulnow);

		/** set the time out of the request. default is 15000 milliseconds. */
		void SetTimeOut(int nMilliSeconds);
		/** Get the time out of the request. default is 15000 milliseconds. */
		int GetTimeOut();

		/** whether to close the connection when the request is completed. */
		void SetForbidReuse(bool bForbidReuse);

		/** set the options for asset request. 
		* @param pFuncCallback: the call back function to use. if none is specified, it will pick a default one to use according to pRequestData->m_nAssetType;
		* this function is always called by the main thread. 
		* @param lpUserData is expected to be classes derived from CUrlProcessorUserData, such as CAssetRequestData
		* @param bDeleteUserData: if true, we will delete lpUserData. 
		*/
		void SetCallBack(URL_LOADER_CALLBACK pFuncCallback=NULL, CUrlProcessorUserData* pUserData=NULL, bool bDeleteUserData=false);

		/** whether this request is timed out relative to a given time
		* @param nCurrentTime: it must be a value returned from GetTickCount();
		*/
		bool IsTimedOut(DWORD nCurrentTime);

		/** this is to set the timer to current time. it is used to check for TimedOut() 
		* @return : return the current time. 
		*/
		DWORD UpdateTime();

		/** safe delete user data */
		void SafeDeleteUserData();

		/** get user data. */
		CUrlProcessorUserData* GetUserData() {return m_pUserData;}

		/** get request data. */
		inline vector<char>& GetData() {return m_data;}

		/** get request header. */
		inline vector<char>& GetHeader() {return m_header;}

		/** get the total number of bytes. only for statistics. */
		int GetTotalBytes();

		/** get bytes received. only for statistics */
		int GetBytesReceived();

		/** add bytes received. only for statistics */
		void AddBytesReceived(int nBytesReceived);
			
		bool IsEnableProgressUpdate() const;

		void SetEnableProgressUpdate(bool val);

		/* whether to enable data streaming, once enabled, we will also include actual data received in progress update callback */
		bool IsEnableDataStreaming() const;
		void SetEnableDataStreaming(bool val);

		const char* CopyRequestData(const char* pData, int nLength);

		/** get options as NPL table object. */
		NPL::NPLObjectProxy& GetOptions();

		/** whether we will invoke callback immediately using NPL.call instead of NPL.activate. This is only enabled in sync-mode api. */
		bool IsSyncCallbackMode() const;
		void SetSyncCallbackMode(bool val);

		/*是否需要断点续传*/
		void SetNeedResumeDownload(bool needResume);
	private:
		int InvokeCallbackScript(const char* sCode, int nLength);
	public:
		/** CURLOPT_URL*/
		string m_url;
		/** http headers to send*/
		struct curl_slist * m_pHttpHeaders;
		/** CURLOPT_HTTPPOST */
		struct curl_httppost* m_pFormPost;
		struct curl_httppost* m_pFormLast;

		IProcessorWorkerData * m_pThreadLocalData;

		/** task priority */
		int m_nPriority;
		/** the NPL function to call when task is finished. */
		std::string m_sNPLCallback;
		/** the NPL runtime state name in which the NPL call back will be invoked. if empty, it will be the default main state.*/
		std::string m_sNPLStateName;

		/** the C++ function to call when task is finished. */
		URL_LOADER_CALLBACK m_pfuncCallBack;
		/** the file name to save the response data to */
		string m_sSaveToFileName;
		/** the file to which to save the content to. usually we only save to memory. */
		CParaFile* m_pFile;

		/** default to 0. if 0, the request will not SAFE_DELETE the user data. the caller is responsible for the task.
		* if it is 1, the destructor will try to SAFE_DELETE(m_pAssetData)
		*/
		int m_nUserDataType;
		/** optional user data to be send along with the m_pfuncCallBack */
		CUrlProcessorUserData * m_pUserData;
		
		/** The type of the url request task */
		URLREQUEST_TYPE m_type;
		/** the current status of the request. */
		URLREQUEST_STATUS m_nStatus;

		/** The time that this request is started or received any response. in milliseconds */
		DWORD m_nStartTime;
		/** we send progress data at fixed 1 second interval, even when there are many or zero data. */
		DWORD m_nLastProgressTime;

		/** for how long the request is considered timed out. If a request was not responding, either because of DNS resolving timeout or connection timed out. 
		* we will manually remove the request and free the slot for next request. This is usually not needed. However, in windows vista, DNS time out is not working as expected, in windows XP
		* the libcurl can detect time out by itself. */
		DWORD m_nTimeOutTime;
		/** whether to reuse connection. CURLOPT_FORBID_REUSE in libcurl */
		bool m_bForbidReuse;
		/** whether to send progress update via callback */
		bool m_bEnableProgressUpdate;
		/** whether to enable data streaming, once enabled, we will also include actual data received in progress update callback */
		bool m_bEnableDataStreaming;
		/** we will invoke callback immediately using NPL.call instead of NPL.activate. This is only enabled in sync-mode api. */
		bool m_bIsSyncCallbackMode;
		
		int m_nBytesReceived;
		int m_nTotalBytes;
		
		vector<char> m_data;
		vector<char> m_header;
		std::string m_sResponseData;
		std::string m_sResponseHeader;
		// this is cache of the request data if any
		std::string m_sRequestData;
		// curl return code
		CURLcode m_returnCode;   
		// the last received HTTP or FTP code. We will expect 200 for successful HTTP response
		long m_responseCode;  

		upload_context* m_pUploadContext;

		/** all lib curl options */
		std::unique_ptr<NPL::NPLObjectProxy> m_options;

		/** this mutex is only used for determine the state. */
		//ParaEngine::mutex m_mutex;

		bool m_needResumeDownload;//是否断点续传
		long m_lastDownloadSize;//上次下载了多少
	};
}
#pragma once
#include "IParaWebService.h"
#include <string>
#include <map>
#include <set>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/asio.hpp>
#include <boost/thread.hpp>
/* curl specific */
#include <curl/curl.h>
#else 
#include "UrlLoaders.h"
#endif
#include <boost/scoped_ptr.hpp>




namespace ParaEngine
{
#ifdef HAS_JABBER_CLIENT
	class CNPLJabberClient;
#endif
	struct AssetEntity;

	using namespace std;

	/** an asset request data to be sent via the CURLRequestTask */
	class CAssetRequestData
	{
	public:
		CAssetRequestData():m_nAssetType(-1), m_bLazyLoading(false){};
		CAssetRequestData(AssetEntity& asset);
	public:
		/** it is of AssetEntity::AssetType */
		int m_nAssetType;
		/** this is usually used to locate the asset entity once request is finished. */
		string m_sAssetKey;
		/** default to true. if true, it will immediately refresh texture when data is available. otherwise, we will wait until the next round. */
		bool m_bLazyLoading;
	};

	class CURLRequestTask;
	typedef DWORD (*URL_REQUEST_TASK_CALLBACK)(int nResult, CURLRequestTask* pRequest, LPVOID lpUserData);

	/** URL request can be a HTTP get/post request etc. 
	* note: Only create an instance of this class using the new operator. 
	* CURLRequest can be inserted to a task pool for processing. 
	*/
	class CURLRequestTask
	{
	public:
		/** type of the url request task */
		enum URLREQUEST_TYPE
		{
			URL_REQUEST_HTTP_AUTO,
			URL_REQUEST_HTTP_GET,
			URL_REQUEST_HTTP_POST,
			URL_REQUEST_HTTP_GETFILE,
		};

		/** url request status */
		enum URLREQUEST_STATUS
		{
			// not yet started
			URL_REQUEST_UNSTARTED,
			URL_REQUEST_INCOMPLETE,
			URL_REQUEST_COMPLETED,
		};
		// default time out in milliseconds
		static const DWORD DEFAULT_TIME_OUT = 15000;

#ifndef EMSCRIPTEN_SINGLE_THREAD
		CURLRequestTask():m_pFormPost(0), m_nTimeOutTime(DEFAULT_TIME_OUT), m_nStartTime(0),m_responseCode(0), m_pFormLast(0), m_pUserData(0), m_returnCode(CURLE_OK), m_type(URL_REQUEST_HTTP_AUTO), m_nPriority(0), m_nStatus(URL_REQUEST_UNSTARTED), m_pfuncCallBack(0), m_nBytesReceived(0), m_nTotalBytes(0), m_nUserDataType(0) {};
#else
		CURLRequestTask():m_nTimeOutTime(DEFAULT_TIME_OUT), m_nStartTime(0),m_responseCode(0), m_pUserData(0), m_returnCode(CURLE_OK), m_type(URL_REQUEST_HTTP_AUTO), m_nPriority(0), m_nStatus(URL_REQUEST_UNSTARTED), m_pfuncCallBack(0), m_nBytesReceived(0), m_nTotalBytes(0), m_nUserDataType(0) {};
#endif
		~CURLRequestTask();

	public:
		/** append form parameters */
		CURLFORMcode AppendFormParam(const char* name, const char* value);

		/** append a file, such as 
		{name = {file="/tmp/test.txt",	type="text/plain"}}
		{name = {file="dummy.html",	data="<html><bold>bold</bold></html>, type="text/html"}}
		input can be NULL if not available. 
		*/
		CURLFORMcode AppendFormParam(const char* name, const char* type, const char* file, const char* data, int datalen);

#ifndef EMSCRIPTEN_SINGLE_THREAD
		/** init the easy handle according to the settings. It will also prepare the task for the curl interface by clearing data, etc. */
		void SetCurlEasyOpt(CURL* handle);
#endif
		/** call the call back if any, this function must be called in the main game thread. */
		void CompleteTask();

		/** curl call back. */
		static size_t CUrl_write_data_callback(void *buffer, size_t size, size_t nmemb, void *stream);
		static size_t CUrl_write_header_callback(void *buffer, size_t size, size_t nmemb, void *stream);

		/** set the time out of the request. default is 15000 milliseconds. */
		void SetTimeOut(int nMilliSeconds);
		/** Get the time out of the request. default is 15000 milliseconds. */
		int GetTimeOut();

		/** set the options for asset request. 
		* @param pRequestData: it must be created using new operator. The ownership of this object is transfered to the request. and the caller should never call the delete. 
		* @param pFuncCallback: the call back function to use. if none is specified, it will pick a default one to use according to pRequestData->m_nAssetType;
		*/
		void SetAssetRequestOpt(CAssetRequestData* pRequestData, URL_REQUEST_TASK_CALLBACK pFuncCallback=NULL);

		/** this function can be used for HTTP texture callback. see SetAssetRequestOpt.
		* @param lpUserData is expected to be CAssetRequestData
		*/
		static DWORD Asset_HTTP_request_callback(int nResult, CURLRequestTask* pRequest, LPVOID lpUserData);

		/** safe delete user data */
		void SafeDeleteUserData();

		/** whether this request is timed out relative to a given time
		* @param nCurrentTime: it must be a value returned from GetTickCount();
		*/
		bool IsTimedOut(DWORD nCurrentTime);

		/** this is to set the timer to current time. it is used to check for TimedOut() 
		* @return : return the current time. 
		*/
		DWORD UpdateTime();

	public:
		/** CURLOPT_URL*/
		string m_url;
		/** CURLOPT_HTTPPOST */
#ifndef EMSCRIPTEN_SINGLE_THREAD
		struct curl_httppost* m_pFormPost;
		struct curl_httppost* m_pFormLast;
#endif
		/** task priority */
		int m_nPriority;
		/** the NPL function to call when task is finished. */
		string m_sNPLCallback;

		/** the C++ function to call when task is finished. */
		URL_REQUEST_TASK_CALLBACK m_pfuncCallBack;
		/** the file name to save the response data to */
		string m_sSaveToFileName;

		/** default to 0. if 0, the request will not SAFE_DELETE the user data. the caller is responsible for the task.
		* if it is 1, the destructor will try to SAFE_DELETE(m_pAssetData)
		*/
		int m_nUserDataType;
		/** optional user data to be send along with the m_pfuncCallBack */
		union {
			LPVOID m_pUserData;
			CAssetRequestData* m_pAssetData;
		};


		/** The type of the url request task */
		URLREQUEST_TYPE m_type;
		/** the current status of the request. */
		URLREQUEST_STATUS m_nStatus;

		/** The time that this request is started or received any response. in milliseconds */
		DWORD m_nStartTime;
		/** for how long the request is considered timed out. If a request was not responding, either because of DNS resolving timeout or connection timed out. 
		* we will manually remove the request and free the slot for next request. This is usually not needed. However, in windows vista, DNS time out is not working as expected, in windows XP
		* the libcurl can detect time out by itself. */
		DWORD m_nTimeOutTime;

		int m_nBytesReceived;
		int m_nTotalBytes;
		vector<char> m_data;
		vector<char> m_header;
		string m_sResponseData;
		string m_sResponseHeader;
		// curl return code
		CURLcode m_returnCode;   
		// the last received HTTP or FTP code. We will expect 200 for successful HTTP response
		long m_responseCode;   

	};

	/** a task pool is a collection of requests of which only a specified limited number of worker threads are allowed to process. 
	* for example, we can create a task pool for each type of URL requests, and assign the max number of processors allowed. 

	Implementation Details:

	The main game (scripting) thread pushes any number of URL request to a pool of CURLRequestTask and returns immediately. 

	On each frame move
	- all running tasks are performed using the curl multi interface. The result of each finished request is saved to CURLRequestTask struct. 
	- the pool of CURLRequestTask is traversed  during which finished CURLRequestTask is removed from the the queue and the callback is called. 
	- new tasks are added to the available task slots for further processing. 
	- the above three steps are repeated until there is no queued task to be added to any available slots. 

	*/
	class CRequestTaskPool 
	{
	public:
		/** url worker state. */
		class CUrlWorkerState
		{
		public:
#ifndef EMSCRIPTEN_SINGLE_THREAD
			CUrlWorkerState(): m_easy_handle(NULL), m_pCurrentTask(NULL), m_returnCode(CURLE_OK), m_bIsCompleted(true) {};
			CURL* m_easy_handle;
			/* return code for last transfer */
			CURLcode m_returnCode;   
#endif
			/** what this message means */
			bool m_bIsCompleted;   
			CURLRequestTask* m_pCurrentTask;
		};

#ifndef EMSCRIPTEN_SINGLE_THREAD
		CRequestTaskPool():m_nRunningTaskCount(0), m_nMaxWorkerThreads(1), m_nMaxQueuedTask(65535), m_multi_handle(NULL){}
#else
		CRequestTaskPool():m_nRunningTaskCount(0), m_nMaxWorkerThreads(1), m_nMaxQueuedTask(65535) {}
#endif
		~CRequestTaskPool();
	public:
		/** Append URL request to a pool. 
		* @param pUrlTask: must be new CURLRequestTask(), the ownership of the task is transfered to the manager. so the caller should never delete the pointer. 
		*/
		bool AppendURLRequest(CURLRequestTask* pUrlTask);

		/** process queued tasks and if any has result, call the callback procedure. 
		* return the number of processed result. 
		*/
		int DoProcess();

		/** set maximum number of concurrent task being processed. default is 1.*/
		void SetMaxTaskSlotsCount(int nCount);
	protected:
		/** get the next free worker thread.
		* @param: return NULL, if no worker slot is available. */
		CUrlWorkerState* GetFreeWorkerSlot();

		/** do the multi interface. 
		* This function should be called regularly to process the result using the multi interface. 
		* @return :the number of slots finished during this perform. 
		*/
		int CURL_MultiPerform();

	private:

		/** this list is usually sorted at inserting time according to task priority. */
		std::list <CURLRequestTask*> m_task_pool;

		/** the max number of worker threads. Default to 5 */
		int m_nMaxWorkerThreads;
		/** all active thread */
		std::list <CUrlWorkerState> m_easy_handles;

		/** the max number of queued tasks. Default to 65535 */
		int m_nMaxQueuedTask;
		/** the multi handle. */
#ifndef EMSCRIPTEN_SINGLE_THREAD
		CURLM * m_multi_handle;
#endif
		/** the number of still running tasks. */
		int m_nRunningTaskCount;
	};

#ifndef EMSCRIPTEN_SINGLE_THREAD

	/// <summary>
	/// Option names.  These must be well-formed XML element names.
	/// </summary>
	class CNPLNetClient : public INPLWebServiceClient
	{
	public:
		CNPLNetClient();
		virtual ~CNPLNetClient();

		/** get a singleton instance of this class. */
		static CNPLNetClient* GetInstance();
		/** release the instance if it has not been released yet */
		static void ReleaseInstance();
	public:

		/**
		* get the web service interface by its URL, if the web service does not exists it will return NULL, in which case 
		* one needs to call OpenWebService() 
		*/
		virtual INPLWebService* GetWebService(const char* sURL);
		/**
		* open a new web service with optional credentials
		* @param sURL: url of the web service, such as http://paraengine.com/test.asmx
		* @param sUserName the user name associated with the credentials
		* @param sPassword the password for the user name associated with the credentials.
		* @param sDomain the domain or computer name that verifies the credentials
		*/
		virtual INPLWebService* OpenWebService(const char* sURL, const char* sUserName, const char* sPassword, const char* sDomain);

		/**
		* close a given web service. Basically, there is no need to close a web service, 
		* unless one wants to reopen it with different credentials
		*/
		virtual bool CloseWebService(const char* sURL);

		/**
		* get an existing jabber client instance interface by its JID.
		* If the client is not created using CreateJabberClient() before, function may return NULL.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual INPLJabberClient* GetJabberClient(const char* sJID);
		/**
		* Create a new jabber client instance with the given jabber client ID. It does not open a connection immediately.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual INPLJabberClient* CreateJabberClient(const char* sJID);

		/**
		* close a given jabber client instance. Basically, there is no need to close a web service, 
		* unless one wants to reopen it with different credentials
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual bool CloseJabberClient(const char* sJID);

		/**
		* Asynchronously download a file from the url.
		* @param callbackScript: script code to be called, a global variable called msg is assigned, as below
		*  msg = {DownloadState=""|"complete"|"terminated", totalFileSize=number, currentFileSize=number, PercentDone=number}
		*/
		virtual void AsyncDownload(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName);


		/**
		* cancel all asynchronous downloads that matches a certain downloader name pattern
		* @param DownloaderName:regular expression. such as "proc1", "proc1.*", ".*"
		*/
		virtual void CancelDownload(const char* DownloaderName);

		/**
		* Synchronous call of the function AsyncDownload(). This function will not return until download is complete or an error occurs. 
		* this function is rarely used. AsyncDownload() is used. 
		* @return:1 if succeed, 0 if fail
		*/
		virtual int Download(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName);

		/** this function is called once on each frame to polling all results and calling the callbacks in scripting interface 
		* @note: this function must be called from the main NPL runtime thread,since it will execute NPL code internally.
		* @return the number of asynchronous messages processed are returned.
		*/
		virtual int ProcessResults();

		/** this function is called once on each frame to polling all results and calling the callbacks in scripting interface 
		* @note: this function must be called from the main NPL runtime thread,since it will execute NPL code internally.
		* @return the number of asynchronous download processed are returned.
		*/
		virtual int ProcessDownloaderResults();

		/** delete this object when it is no longer needed.*/
		virtual void DeleteThis();

		///////////////////////////////////////////////////////////////////////////
		//
		// Do url request 
		//
		///////////////////////////////////////////////////////////////////////////

		/** Append URL request to a pool. connections in the same pool are reused as much as possible. 
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots. 
		*  the default number is 1. One can change this number with ChangeRequestPoolSize. 
		* @param pUrlTask: must be new CURLRequestTask(), the ownership of the task is transfered to the manager. so the caller should never delete the pointer. 
		* @param sPoolName: the request pool name. If the pool does not exist, it will be created. If null, the default pool is used. 
		*  there are some reserved pool names used by ParaEngine. They are: 
		*    - "d": download pool. default size is 2, for downloading files. 
		*    - "r": rest pool. default size is 5, for REST like HTTP get/post calls.
		*    - "w": web pool. default size is 5, for web based requests.
		*/
		virtual bool AppendURLRequest(CURLRequestTask* pUrlTask, const char* sPoolName = NULL);

		/**
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots. 
		*  the default number is 1. One can change this number with this function. 
		*/
		virtual bool ChangeRequestPoolSize(const char* sPoolName, int nCount);

		/** this function is called once on each frame to polling all results and calling the callbacks in scripting interface 
		* @note: this function must be called from the main NPL runtime thread,since it will execute NPL code internally.
		* @return the number of asynchronous messages processed are returned.
		*/
		virtual int ProcessUrlRequests();

		/** add a string url to a set. this is used for tracking in processing requests. 
		* this function is called automatically when AppendURLRequest is called. 
		*/
		void AddPendingRequest(const char* sURL);

		/** whether the given request is already in the pending set. 
		* one can use this function to check whether there is a pending request due to previous call of AppendURLRequest
		*/
		bool HasPendingRequest(const char* sURL);

		/** remove a string url from a set. This function is automatically called when a url request is completed. */
		void RemovePendingRequest(const char* sURL);

		/** clear all pending request. */
		void ClearAllPendingRequests();

		/** get the md5 of sFileUrl. the returned value is URLEncoded.*/
		string GetCachePath(const char* sFileUrl);

		/** just clean up everything. */
		void Cleanup();
	protected:

		/** get a task pool by name. and create it if it does not exist. */
		CRequestTaskPool* CreateGetRequestTaskPool(const char* sPoolName = NULL);

	private:
#ifdef HAS_JABBER_CLIENT
		std::map <std::string, CNPLJabberClient*> m_jabberClients;
#endif
		std::map <std::string, CRequestTaskPool*> m_request_pools;

		std::set <std::string> m_pending_requests;

		/** for accepting and dispatching messages for all NPL Jabber Clients. Only one m_DispatcherThread is used to run it. */
		boost::asio::io_service m_dispatcher_io_service;

		/** Thread used for running the m_dispatcher_io_service 's run loop for accepting and dispatching messages for all NPL Jabber Clients */
		boost::scoped_ptr<boost::thread> m_dispatcherThread;

		/** Work for the private m_dispatcher_io_service to perform. If we do not give the
		io_service some work to do then the io_service::run() function will exit immediately.*/
		boost::scoped_ptr<boost::asio::io_service::work> m_work_lifetime;
	};
#endif
}

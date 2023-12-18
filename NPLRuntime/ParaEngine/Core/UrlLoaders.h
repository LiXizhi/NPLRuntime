#pragma once
#include "IDataLoader.h"
#include <vector>
/* curl specific */
#include "util/mutex.h"
// #ifndef EMSCRIPTEN_SINGLE_THREAD
#ifndef EMSCRIPTEN
#include <curl/curl.h>
#else
typedef enum {
  CURL_FORMADD_OK, /* first, no error */

  CURL_FORMADD_MEMORY,
  CURL_FORMADD_OPTION_TWICE,
  CURL_FORMADD_NULL,
  CURL_FORMADD_UNKNOWN_OPTION,
  CURL_FORMADD_INCOMPLETE,
  CURL_FORMADD_ILLEGAL_ARRAY,
  CURL_FORMADD_DISABLED, /* libcurl was built with this disabled */

  CURL_FORMADD_LAST /* last */
} CURLFORMcode;

typedef enum {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                    7.17.0, reused in April 2011 for 7.21.5] */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                    due to lack of access - when login fails
                                    this is not returned. */
  CURLE_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
                                    7.15.4, reused in Dec 2011 for 7.24.0]*/
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
                                    [was obsoleted in August 2007 for 7.17.0,
                                    reused in Dec 2011 for 7.24.0]*/
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_HTTP2,                   /* 16 - A problem in the http2 framing layer.
                                    [was obsoleted in August 2007 for 7.17.0,
                                    reused in July 2014 for 7.38.0] */
  CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_OBSOLETE20,              /* 20 - NOT USED */
  CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_OBSOLETE24,              /* 24 - NOT USED */
  CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
  CURLE_READ_ERROR,              /* 26 - couldn't open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  /* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
           instead of a memory allocation error if CURL_DOES_CONVERSIONS
           is defined
  */
  CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
  CURLE_OBSOLETE29,              /* 29 - NOT USED */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_OBSOLETE32,              /* 32 - NOT USED */
  CURLE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_OBSOLETE40,              /* 40 - NOT USED */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_OBSOLETE44,              /* 44 - NOT USED */
  CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
  CURLE_OBSOLETE46,              /* 46 - NOT USED */
  CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
  CURLE_OBSOLETE50,              /* 50 - NOT USED */
  CURLE_PEER_FAILED_VERIFICATION, /* 51 - peer's certificate or fingerprint
                                     wasn't verified fine */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_OBSOLETE57,              /* 57 - NOT IN USE */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
  CURLE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
  CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
  CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
  CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                    that failed */
  CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
  CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                    accepted and we failed to login */
  CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
  CURLE_TFTP_PERM,               /* 69 - permission problem on server */
  CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
  CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
  CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
  CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
  CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
  CURLE_CONV_FAILED,             /* 75 - conversion failed */
  CURLE_CONV_REQD,               /* 76 - caller must register conversion
                                    callbacks using curl_easy_setopt options
                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION,
                                    CURLOPT_CONV_TO_NETWORK_FUNCTION, and
                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
  CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                    or wrong format */
  CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
  CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                    generic so the error message will be of
                                    interest when this has happened */

  CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                    connection */
  CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                    wait till it's ready and try again (Added
                                    in 7.18.2) */
  CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                    wrong format (Added in 7.19.0) */
  CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                    7.19.0) */
  CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
  CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
  CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
  CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
  CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */
  CURLE_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
                                    session will be queued */
  CURLE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
                                     match */
  CURLE_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
  CURL_LAST /* never use! */
} CURLcode;

#endif
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

#ifdef EMSCRIPTEN
		virtual bool AsyncProcess(std::function<void()> callback);
#endif

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
		void EmscriptenFetch();
		void EmscriptenFetch2();
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
#ifndef EMSCRIPTEN
		void SetCurlEasyOpt(CURL* handle);
#endif
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

		const char* CopyRequestData(const char* pData, int nLength);

		/** get options as NPL table object. */
		NPL::NPLObjectProxy& GetOptions();

		/** whether we will invoke callback immediately using NPL.call instead of NPL.activate. This is only enabled in sync-mode api. */
		bool IsSyncCallbackMode() const;
		void SetSyncCallbackMode(bool val);
	private:
		int InvokeCallbackScript(const char* sCode, int nLength);
	public:
		/** CURLOPT_URL*/
		string m_url;
		/** http headers to send*/
#ifndef EMSCRIPTEN
		struct curl_slist * m_pHttpHeaders;
		/** CURLOPT_HTTPPOST */
		struct curl_httppost* m_pFormPost;
		struct curl_httppost* m_pFormLast;
#endif
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

#ifdef EMSCRIPTEN
		std::vector<std::string> m_request_headers;
		std::function<void()> m_async_callback;
		std::string m_fetch_response_data;
		std::string m_fetch_response_header;
#endif 
	};
}
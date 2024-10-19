//-----------------------------------------------------------------------------
// Class:	Url loaders
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.9.1
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLHelper.h"
#include "AISimulator.h"
#include "NPLRuntime.h"
#include "UrlLoaders.h"
#include "AsyncLoader.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/val.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#endif
#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

using namespace ParaEngine;


ParaEngine::upload_context::upload_context(const char* data, int nDataSize /*= 0*/)
	:m_pData(data), m_nBytesSent(0), m_nDataSize(nDataSize)
{
	if (m_nDataSize == 0 && m_pData)
	{
		m_nDataSize = strlen(m_pData);
	}
}

AbortController::AbortController(const char* name) : m_aborted(false), m_sRequestName(name)
{
	if (!m_sRequestName.empty()) {
		CAsyncLoader::GetSingleton().AddAbortableRequest(m_sRequestName.c_str());
	}

}

AbortController::~AbortController()
{
	if (!m_sRequestName.empty()) {
		CAsyncLoader::GetSingleton().RemoveAbortableRequest(m_sRequestName.c_str());
	}
}

bool AbortController::isAborted() const {
	return m_aborted.load() || (!m_sRequestName.empty() && CAsyncLoader::GetSingleton().IsRequestAborted(m_sRequestName.c_str()));
}

//////////////////////////////////////////////////////////////////////////
//
// CUrlLoader
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CUrlLoader::CUrlLoader(const string& url)
	: m_url(url), m_nEstimatedSizeInBytes(0)
{
}
ParaEngine::CUrlLoader::CUrlLoader()
	: m_nEstimatedSizeInBytes(0)
{
}

ParaEngine::CUrlLoader::~CUrlLoader()
{

}

const char* ParaEngine::CUrlLoader::GetFileName()
{
	return m_url.c_str();
}

void ParaEngine::CUrlLoader::SetUrl(const char* url)
{
	if (url)
		m_url = url;
}

HRESULT ParaEngine::CUrlLoader::Decompress(void** ppData, int* pcBytes)
{
	return S_OK;
}

void ParaEngine::CUrlLoader::CleanUp()
{
}

HRESULT ParaEngine::CUrlLoader::Destroy()
{
	return S_OK;
}

HRESULT ParaEngine::CUrlLoader::Load()
{
	return S_OK;
}

void ParaEngine::CUrlLoader::SetEstimatedSizeInBytes(int nSize)
{
	m_nEstimatedSizeInBytes = nSize;
}

int ParaEngine::CUrlLoader::GetEstimatedSizeInBytes()
{
	return m_nEstimatedSizeInBytes;
}

//////////////////////////////////////////////////////////////////////////
//
// CUrlProcessor
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CUrlProcessor::CUrlProcessor()
	:m_nTimeOutTime(DEFAULT_TIME_OUT), m_nStartTime(0), m_responseCode(0), m_nLastProgressTime(0),
	m_pUserData(0), m_returnCode(CURLE_OK), m_type(URL_REQUEST_HTTP_AUTO),
	m_nPriority(0), m_nStatus(URL_REQUEST_UNSTARTED), m_pfuncCallBack(0), m_nBytesReceived(0), m_pUploadContext(NULL),
	m_nTotalBytes(0), m_nUserDataType(0), m_pFile(NULL), m_pThreadLocalData(NULL), m_bForbidReuse(false), m_bEnableProgressUpdate(true), m_bIsSyncCallbackMode(false), m_bEnableDataStreaming(false)
{
#ifndef EMSCRIPTEN
	m_pHttpHeaders = 0;
	m_pFormPost = 0;
	m_pFormLast = 0;
#endif
}

ParaEngine::CUrlProcessor::CUrlProcessor(const string& url, const string& npl_callback)
	:m_nTimeOutTime(DEFAULT_TIME_OUT), m_nStartTime(0), m_responseCode(0), m_nLastProgressTime(0),
	m_pUserData(0), m_returnCode(CURLE_OK), m_type(URL_REQUEST_HTTP_AUTO),
	m_nPriority(0), m_nStatus(URL_REQUEST_UNSTARTED), m_pfuncCallBack(0), m_nBytesReceived(0), m_pUploadContext(NULL),
	m_nTotalBytes(0), m_nUserDataType(0), m_pFile(NULL), m_pThreadLocalData(NULL), m_bEnableProgressUpdate(true), m_bIsSyncCallbackMode(false), m_bEnableDataStreaming(false)
{
#ifndef EMSCRIPTEN
	m_pHttpHeaders = 0;
	m_pFormPost = 0;
	m_pFormLast = 0;
#endif
	m_url = url;
	SetScriptCallback(npl_callback.c_str());
}


ParaEngine::CUrlProcessor::~CUrlProcessor()
{
	CleanUp();
	if (m_abortController)
		m_abortController->abort();
}

void ParaEngine::CUrlProcessor::CleanUp()
{
	SAFE_DELETE(m_pFile);
#ifndef EMSCRIPTEN
	if (m_pFormPost)
	{
		/* then cleanup the form post chain */
		curl_formfree(m_pFormPost);
		m_pFormPost = NULL;
	}
	if (m_pHttpHeaders)
	{
		curl_slist_free_all(m_pHttpHeaders);
		m_pHttpHeaders = NULL;
	}
#endif
	SafeDeleteUserData();
	SAFE_DELETE(m_pUploadContext);
}

void ParaEngine::CUrlProcessor::SafeDeleteUserData()
{
	if (m_nUserDataType > 0)
	{
		if (m_nUserDataType == 1)
		{
			SAFE_DELETE(m_pUserData);
		}
	}
}

void CUrlProcessor::SetProcessorWorkerData(IProcessorWorkerData * pThreadLocalData)
{
	m_pThreadLocalData = pThreadLocalData;
}

ParaEngine::IProcessorWorkerData * CUrlProcessor::GetProcessorWorkerData()
{
	return m_pThreadLocalData;
}

HRESULT ParaEngine::CUrlProcessor::LockDeviceObject()
{
	return S_OK;
}

HRESULT ParaEngine::CUrlProcessor::UnLockDeviceObject()
{
	// complete the task
	CompleteTask();
	return S_OK;
}

HRESULT ParaEngine::CUrlProcessor::Destroy()
{
	return S_OK;
}

#ifdef EMSCRIPTEN
static emscripten::val JSFetch(ParaEngine::CUrlProcessor* self, std::function<void()> callback) {
	emscripten::val fetch = emscripten::val::global("jsfetch");
	// 创建请求配置对象
    emscripten::val options = emscripten::val::object();
	// 设置请求头部信息
    emscripten::val headers = emscripten::val::object();
	for (int i = 0; i < self->m_request_headers.size(); i+=2) 
	{
		auto& key = self->m_request_headers[i];
		auto& value = self->m_request_headers[i+1];
		headers.set(emscripten::val(key), emscripten::val(value));
	}	
	options.set(emscripten::val("headers"), headers);
	// 设置请求方法
	std::string method = "GET";
	if (self->m_options && self->m_options->GetField("CURLOPT_CUSTOMREQUEST")->isString()) method = self->m_options->GetField("CURLOPT_CUSTOMREQUEST").c_str();
	else if (!self->m_sRequestData.empty()) method = "POST";
	else method = "GET";
	options.set(emscripten::val("method"), emscripten::val(method));
	// 设置请求数据
	options.set(emscripten::val("body"), emscripten::val(self->m_sRequestData));

	// 发送请求
	emscripten::val response = co_await fetch(emscripten::val(self->m_url), options);
	self->m_responseCode = response["status"].as<int>();

	// 响应头
    self->m_fetch_response_header = response["headers"].as<std::string>();
	self->write_header_callback(self->m_fetch_response_header.data(), self->m_fetch_response_header.size(), 1);

	// 响应体
    bool is_stream = response["stream"].as<bool>();
    emscripten::val response_body = response["body"];
    while (is_stream)
    {
        emscripten::val response_body_value = co_await response_body();
        if (response_body_value["done"].as<bool>()) break;
        self->m_fetch_response_data = response_body_value["value"].as<std::string>();
		int nSize = self->m_fetch_response_data.size();
		auto nDataRead = self->write_data_callback(self->m_fetch_response_data.data(), nSize, 1);
		if (nDataRead < nSize) {
			// stop streaming,abort connection
			// TODO: one needs to options.set(emscripten::val("signal"), ... ) and abort the fetch here
			break;
		}
        // std::cout << "body: " << self->m_fetch_response_data << std::endl;
    }
	
	self->m_fetch_response_data = "";
	callback();
}

bool ParaEngine::CUrlProcessor::AsyncProcess(std::function<void()> callback)
{
	if (IsEnableDataStreaming()) {
		JSFetch(this, callback);
		return true;
	}
	std::vector<const char*> request_headers;
	int request_headers_size = m_request_headers.size();
	for (int i = 0; i < request_headers_size; i++) request_headers.push_back(m_request_headers[i].c_str());
	request_headers.push_back(0);

	m_async_callback = callback;
	auto fetch_finished_callback = [](emscripten_fetch_t *fetch) {
		auto self = (ParaEngine::CUrlProcessor*)(fetch->userData);
		if (self->m_async_callback == nullptr) return;
		self->m_responseCode = fetch->status;
		size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch);
		self->m_fetch_response_header.resize(headersLengthBytes);
		self->m_fetch_response_data.resize(fetch->totalBytes);
		emscripten_fetch_get_response_headers(fetch, (char*)(self->m_fetch_response_header.data()), headersLengthBytes);
		memcpy((void*)(self->m_fetch_response_data.data()), fetch->data, fetch->totalBytes);

		if (self->IsEnableDataStreaming() && self->m_fetch_response_data.size() > 0) {
			self->write_data_callback((void*)self->m_fetch_response_data.data(), self->m_fetch_response_data.size(), 1);
		}

		self->m_nStatus = CUrlProcessor::URL_REQUEST_COMPLETED;		
		self->m_async_callback();
		self->m_async_callback = nullptr;
		// emscripten_fetch_close(fetch); // Also free data on failure.
	};
	
	auto fetch_onprogress_callback = [](emscripten_fetch_t *fetch) {
		auto self = (ParaEngine::CUrlProcessor*)(fetch->userData);
		if (self->m_fetch_response_header.empty() && fetch->readyState >= 2) {
			size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch);
			self->m_fetch_response_header.resize(headersLengthBytes);
			emscripten_fetch_get_response_headers(fetch, (char*)self->m_fetch_response_header.data(), headersLengthBytes);
			self->write_header_callback((void*)self->m_fetch_response_header.data(), self->m_fetch_response_header.size(), 1);
		}
		std::cout << "data offset = " << fetch->dataOffset << std::endl;
		std::cout << "data size = " << fetch->numBytes << std::endl;
		std::cout << "data total = " << fetch->totalBytes << std::endl;
		if (fetch->numBytes > 0) {
			// self->m_fetch_response_data.resize(fetch->numBytes);
			// memcpy(self->m_fetch_response_data.data(), fetch->data, fetch->numBytes);
			// self->write_data_callback(self->m_fetch_response_data.data(), fetch->numBytes, 1);
			self->m_fetch_response_data.insert(self->m_fetch_response_data.end(), fetch->data + fetch->dataOffset, fetch->data + fetch->dataOffset + fetch->numBytes);
			std:cout << "part => " << (char*)(fetch->data + fetch->dataOffset) << std::endl;
		} else {
			std::cout << "all => " << (char*)(fetch->data) << std::endl;
		}
	};

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
	std::string method = "GET";
	if (m_options && m_options->GetField("CURLOPT_CUSTOMREQUEST")->isString()) method = m_options->GetField("CURLOPT_CUSTOMREQUEST").c_str();
	else if (!m_sRequestData.empty()) method = "POST";
	else method = "GET";
    strcpy(attr.requestMethod, method.c_str());
    attr.userData = this;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.requestData = m_sRequestData.c_str();
	attr.requestDataSize = m_sRequestData.size();
	attr.requestHeaders = request_headers.data();
	attr.onsuccess = fetch_finished_callback;
	attr.onerror = fetch_finished_callback;
	if (IsEnableDataStreaming()) {
		attr.attributes = attr.attributes | EMSCRIPTEN_FETCH_STREAM_DATA;
		attr.onprogress = fetch_onprogress_callback;
	} 

	emscripten_fetch(&attr, m_url.c_str()); // Blocks here until the operation is complete.
	return true;
}
#endif

void ParaEngine::CUrlProcessor::EmscriptenFetch()
{
#ifdef EMSCRIPTEN
	std::vector<const char*> request_headers;
	int request_headers_size = m_request_headers.size();
	for (int i = 0; i < request_headers_size; i++) request_headers.push_back(m_request_headers[i].c_str());
	request_headers.push_back(0);

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
	std::string method = "GET";
	if (m_options && m_options->GetField("CURLOPT_CUSTOMREQUEST")->isString()) method = m_options->GetField("CURLOPT_CUSTOMREQUEST").c_str();
	else if (!m_sRequestData.empty()) method = "POST";
	else method = "GET";
    strcpy(attr.requestMethod, method.c_str());
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
	attr.requestData = m_sRequestData.c_str();
	attr.requestDataSize = m_sRequestData.size();
	attr.requestHeaders = request_headers.data();
	emscripten_fetch_t *fetch = emscripten_fetch(&attr, m_url.c_str()); // Blocks here until the operation is complete.
	m_responseCode = fetch->status;
	std::vector<char> response_header;
	size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch) + 1;
	response_header.resize(headersLengthBytes);
  	emscripten_fetch_get_response_headers(fetch, response_header.data(), headersLengthBytes);
	write_header_callback(response_header.data(), response_header.size(), 1);
	write_data_callback((void*)(fetch->data), fetch->totalBytes, 1);
  	emscripten_fetch_close(fetch); // Also free data on failure.
	m_nStatus = CUrlProcessor::URL_REQUEST_COMPLETED;
	// if (m_responseCode != 200)
	// {
	// 	std::cout << "=================request error=================" << std::endl;
	// 	std::cout << "method: " << method << std::endl;
	// 	std::cout << "url: " << m_url << std::endl;
	// 	std::cout << "request data:" << m_sRequestData << std::endl;
	// 	std::cout << "status code: " << m_responseCode << std::endl;
	// 	std::cout << "response header size: " << headersLengthBytes << std::endl;
	// 	std::cout << "response data size: " << fetch->totalBytes << std::endl;
	// 	std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
	// 	// std::cout << "response header:" << response_header << std::endl;
	// }
#endif
	// return S_OK;
}

void ParaEngine::CUrlProcessor::EmscriptenFetch2()
{
#ifdef EMSCRIPTEN
    m_fetch_finish = false;
	if (IsEnableDataStreaming()) {
		JSFetch(this, [this](){
			this->m_fetch_finish = true;
		});
		while (!m_fetch_finish) emscripten_sleep(100);
		return;
	}
	std::vector<const char*> request_headers;
	int request_headers_size = m_request_headers.size();
	for (int i = 0; i < request_headers_size; i++) request_headers.push_back(m_request_headers[i].c_str());
	request_headers.push_back(0);

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
	std::string method = "GET";
	if (m_options && m_options->GetField("CURLOPT_CUSTOMREQUEST")->isString()) method = m_options->GetField("CURLOPT_CUSTOMREQUEST").c_str();
	else if (!m_sRequestData.empty()) method = "POST";
	else method = "GET";
    strcpy(attr.requestMethod, method.c_str());
    attr.userData = this;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.requestData = m_sRequestData.c_str();
	attr.requestDataSize = m_sRequestData.size();
	attr.requestHeaders = request_headers.data();
	attr.onsuccess = [](emscripten_fetch_t *fetch) { 
		void* userdata = fetch->userData; 
		auto self = (ParaEngine::CUrlProcessor*)userdata;
		self->m_fetch_finish = true; 
	};
	attr.onerror = [](emscripten_fetch_t *fetch) { 
		void* userdata = fetch->userData; 
		auto self = (ParaEngine::CUrlProcessor*)userdata;
		self->m_fetch_finish = true; 
	};
	emscripten_fetch_t *fetch = emscripten_fetch(&attr, m_url.c_str()); // Blocks here until the operation is complete.
	while (!m_fetch_finish) emscripten_sleep(100);
	m_responseCode = fetch->status;

  	emscripten_fetch_close(fetch); // Also free data on failure.
	// m_nStatus = CUrlProcessor::URL_REQUEST_COMPLETED;
	// {
	// 	std::cout << "=================request error=================" << std::endl;
	// 	std::cout << "method: " << method << std::endl;
	// 	std::cout << "url: " << m_url << std::endl;
	// 	std::cout << "request data:" << m_sRequestData << std::endl;
	// 	std::cout << "status code: " << m_responseCode << std::endl;
	// 	std::cout << "response header size: " << headersLengthBytes << std::endl;
	// 	std::cout << "response data size: " << fetch->totalBytes << std::endl;
	// } 
#endif
	// return S_OK;
}


HRESULT ParaEngine::CUrlProcessor::Process(void* pData, int cBytes)
{
#ifdef EMSCRIPTEN
	if (m_responseCode == 0) 
	{
#ifdef EMSCRIPTEN_SINGLE_THREAD
		EmscriptenFetch2();
#else
		EmscriptenFetch();
	    // std::thread(&ParaEngine::CUrlProcessor::EmscriptenFetch, this).join();
#endif 
	}
	else
	{
		if (!IsEnableDataStreaming()) {
			write_header_callback((void*)m_fetch_response_header.data(), m_fetch_response_header.size(), 1);
			write_data_callback((void*)m_fetch_response_data.data(), m_fetch_response_data.size(), 1);
		}
	}
	m_returnCode = CURLE_OK;
	return S_OK;

#else
	// Let us do the easy way. 
	CURL *curl = NULL;

	bool bShareCurlHandle = false;
	// we will reuse the same easy handle per thread
	if (GetProcessorWorkerData() && GetProcessorWorkerData()->GetCurlInterface())
	{
		curl = (CURL*)(GetProcessorWorkerData()->GetCurlInterface());
		bShareCurlHandle = true;
	}
	else
	{
		curl = curl_easy_init();
	}
	if (curl)
	{
		UpdateTime();

		SetCurlEasyOpt(curl);

		m_returnCode = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &m_responseCode);
		m_nStatus = CUrlProcessor::URL_REQUEST_COMPLETED;

		// SLEEP(2000);

		if (!bShareCurlHandle)
		{
			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		else
		{
			curl_easy_reset(curl);
		}
		return S_OK;
	}
	return E_FAIL;
#endif
}

HRESULT ParaEngine::CUrlProcessor::CopyToResource()
{
	return S_OK;
}

void ParaEngine::CUrlProcessor::SetResourceError()
{
}


void ParaEngine::CUrlProcessor::SetScriptCallback(const char* sCallback)
{
	if (sCallback)
	{

		m_sNPLStateName.clear();
		if (sCallback[0] == '(')
		{
			int i = 1;
			while ((sCallback[i] != ')') && (sCallback[i] != '\0'))
			{
				i++;
			}
			i++;
			if (sCallback[i - 1] != '\0')
			{
				m_sNPLCallback = sCallback + i;
			}
			if (i > 2 && !(i == 4 && (sCallback[1] == 'g') && (sCallback[2] == 'l')))
				m_sNPLStateName.assign(sCallback + 1, i - 2);
		}
		else
		{
			m_sNPLCallback = sCallback;
		}
	}
}

void ParaEngine::CUrlProcessor::SetSaveToFile(const char* filename)
{
	if (filename)
		m_sSaveToFileName = filename;
}

void ParaEngine::CUrlProcessor::SetUrl(const char* url)
{
	if (url)
		m_url = url;
}

void ParaEngine::CUrlProcessor::SetHeadersOnly()
{
	m_type = URL_REQUEST_HTTP_HEADERS_ONLY;
}

void ParaEngine::CUrlProcessor::SetForbidReuse(bool bForbidReuse)
{
	m_bForbidReuse = bForbidReuse;
}

int ParaEngine::CUrlProcessor::GetTotalBytes()
{
	//ParaEngine::Lock lock_(m_mutex);
	int nTotalBytes = m_nTotalBytes;
	return nTotalBytes;
}

int ParaEngine::CUrlProcessor::GetBytesReceived()
{
	//ParaEngine::Lock lock_(m_mutex);
	int nBytesReceived = m_nBytesReceived;
	return nBytesReceived;
}

void ParaEngine::CUrlProcessor::AddBytesReceived(int nByteCount)
{
	//ParaEngine::Lock lock_(m_mutex);
	m_nBytesReceived += nByteCount;

	if (m_pThreadLocalData)
	{
		m_pThreadLocalData->AddBytesProcessed(nByteCount);
	}
}

#ifndef EMSCRIPTEN
void ParaEngine::CUrlProcessor::SetCurlEasyOpt(CURL* handle)
{
	// reset data 
	m_data.clear();
	m_header.clear();
	m_nLastProgressTime = 0;
	curl_easy_setopt(handle, CURLOPT_URL, m_url.c_str());

	bool bIsSMTP = false;
	bool bIsCustomRequest = false;
	if (m_url.size() > 5 && m_url[0] == 's' && m_url[1] == 'm' && m_url[2] == 't' && m_url[3] == 'p')
	{
		// smtp protocol for sending email
		bIsSMTP = true;
	}

	bool bAllowSlowRequest = false;

	if (m_options)
	{
		for (auto iter = m_options->begin(); iter != m_options->end(); iter++)
		{
			const std::string& sKey = iter->first;
			if (sKey == "CURLOPT_USERNAME") {
				curl_easy_setopt(handle, CURLOPT_USERNAME, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_PASSWORD") {
				curl_easy_setopt(handle, CURLOPT_PASSWORD, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_PROXY") {
				curl_easy_setopt(handle, CURLOPT_PROXY, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_PROXYUSERPWD") {
				curl_easy_setopt(handle, CURLOPT_PROXYUSERPWD, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_CUSTOMREQUEST") {
				bIsCustomRequest = true;
				curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_SSLCERT") {
				curl_easy_setopt(handle, CURLOPT_SSLCERT, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_SSLKEY") {
				curl_easy_setopt(handle, CURLOPT_SSLKEY, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_SSLCERTPASSWD") {
				curl_easy_setopt(handle, CURLOPT_SSLCERTPASSWD, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_CAINFO") {
				curl_easy_setopt(handle, CURLOPT_CAINFO, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_CAPATH") {
				curl_easy_setopt(handle, CURLOPT_CAPATH, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_MAIL_FROM") {
				curl_easy_setopt(handle, CURLOPT_MAIL_FROM, iter->second.c_str());
			}
			else if (sKey == "CURLOPT_VERBOSE") {
				curl_easy_setopt(handle, CURLOPT_VERBOSE, iter->second.toInt());
			}
			else if (sKey == "CURLOPT_READDATA")
			{
				m_pUploadContext = new upload_context(iter->second.c_str(), ((const std::string&)(iter->second)).size());
				curl_easy_setopt(handle, CURLOPT_READFUNCTION, &CUrl_read_email_payload);
				curl_easy_setopt(handle, CURLOPT_READDATA, m_pUploadContext);
			}
			else if (sKey == "CURLOPT_UPLOAD") {
				curl_easy_setopt(handle, CURLOPT_UPLOAD, iter->second.toInt());
			}
			else if (sKey == "CURLOPT_MAIL_RCPT") {
				if (iter->second->isString())
					AppendHTTPHeader(iter->second.c_str());
				else if (iter->second->isTable())
				{
					for (auto iter2 = iter->second.index_begin(); iter2 != iter->second.index_end(); iter2++)
					{
						AppendHTTPHeader(iter2->second.c_str());
					}
				}
				curl_easy_setopt(handle, CURLOPT_MAIL_RCPT, m_pHttpHeaders);
			}
			else if (sKey == "CURLOPT_CONNECTTIMEOUT")
			{
				curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, iter->second.toInt());
				bAllowSlowRequest = true;
			}
			else if (sKey == "CURLOPT_LOW_SPEED_TIME")
			{
				curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, iter->second.toInt());
				bAllowSlowRequest = true;
			}
			else if (sKey == "CURLOPT_LOW_SPEED_LIMIT")
			{
				curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, iter->second.toInt());
				bAllowSlowRequest = true;
			}
			else if (sKey == "CURLOPT_ALLOW_SLOW_REQUEST")
			{
				bAllowSlowRequest = true;
			}
		}
	}

	/* Define our callback to get called when there's data to be written */
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CUrl_write_data_callback);
	/* Set a pointer to our struct to pass to the callback */
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, CUrl_write_header_callback);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);

	/* Pass a long. Set to 1 to make the next transfer explicitly close the connection when done. Normally, libcurl keeps all connections alive when done with one transfer in case a succeeding one follows that can re-use them. */
	curl_easy_setopt(handle, CURLOPT_FORBID_REUSE, m_bForbidReuse ? 1 : 0);

	/* do not verify host */
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);

	//curl_easy_setopt(handle,CURLOPT_CAINFO, NULL);
	//curl_easy_setopt(handle,CURLOPT_CAPATH, NULL); 

	/**
	Pass a long. It should contain the maximum time in seconds that you allow the connection to the server to take.
	This only limits the connection phase, once it has connected, this option is of no more use. Set to zero to disable
	connection timeout (it will then only timeout on the system's internal timeouts). See also the CURLOPT_TIMEOUT option
	*/

	if (!bAllowSlowRequest)
	{
		/* abort if slower than 30 bytes/sec during 10 seconds */
		curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, 10L);
		curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 30L);
		// time allowed to connect to server, usually to resolve DNS and connect via IP. This can sometimes be very long on mobile devices on first use. 
		curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 20);
	}

	// progress callback is only used to terminate the url progress
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, CUrl_progress_callback);
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, this);
	/* enable all supported built-in compressions */
	curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "");

	// The official doc says if multi-threaded use, this one should be set to 1. otherwise it may crash on certain conditions. 
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);

	if (bIsSMTP)
	{

	}
	else
	{
		// any http headers
		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, m_pHttpHeaders);
		// form if any. 
		curl_easy_setopt(handle, CURLOPT_HTTPPOST, m_pFormPost);
		if (!bIsCustomRequest) {
			curl_easy_setopt(handle, CURLOPT_HTTPGET, m_pFormPost ? 0 : 1);
		}

		if (m_pFormPost == 0 && !m_sRequestData.empty())
		{
			curl_easy_setopt(handle, CURLOPT_POSTFIELDS, m_sRequestData.c_str());
			curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, m_sRequestData.size());
		}
		curl_easy_setopt(handle, CURLOPT_NOBODY, (m_type == URL_REQUEST_HTTP_HEADERS_ONLY) ? 1 : 0);
	}
}
#endif

size_t ParaEngine::CUrlProcessor::CUrl_read_email_payload(void *ptr, size_t size, size_t nmemb, void *userp)
{
	upload_context *upload_ctx = (upload_context*)userp;
	const char *data;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	if (upload_ctx->m_pData && upload_ctx->m_nBytesSent < upload_ctx->m_nDataSize)
	{
		data = upload_ctx->m_pData + upload_ctx->m_nBytesSent;
		int len = size * nmemb;
		if (len > (upload_ctx->m_nDataSize - upload_ctx->m_nBytesSent))
		{
			len = upload_ctx->m_nDataSize - upload_ctx->m_nBytesSent;
		}
		memcpy(ptr, data, len);
		upload_ctx->m_nBytesSent += len;
		return len;
	}
	return 0;
}

size_t ParaEngine::CUrlProcessor::write_data_callback(void *buffer, size_t size, size_t nmemb)
{
	UpdateTime();
	int nByteCount = (int)size*(int)nmemb;
	if (nByteCount > 0)
	{
		AddBytesReceived(nByteCount);

		if (m_sSaveToFileName.empty())
		{
			// save to memory if no disk file name is provided
			int nOldSize = (int)m_data.size();
			m_data.resize(nOldSize + nByteCount);
			memcpy(&(m_data[nOldSize]), buffer, nByteCount);
		}
		else
		{
			if (m_pFile == 0)
			{
				m_pFile = new CParaFile();
				if (!m_pFile->CreateNewFile(m_sSaveToFileName.c_str(), true))
				{
					OUTPUT_LOG("warning: Failed create new file %s\n", m_sSaveToFileName.c_str());
				}
			}
			m_pFile->write((const char*)buffer, (int)nByteCount);
		}
		if (IsEnableDataStreaming())
		{
			if (!m_sNPLCallback.empty())
			{
				NPL::CNPLWriter writer;
				writer.WriteName("msg");
				writer.BeginTable();

				if (!m_header.empty() && m_data.size() == nByteCount)
				{
					// include http response headers ONLY for the first callback.
					writer.WriteName("header");
					writer.WriteValue((const char*)(&(m_header[0])), (int)m_header.size());
				}

				writer.WriteName("data");
				writer.WriteValue((const char*)(buffer), (int)nByteCount, true);

				writer.WriteName("type");
				writer.WriteValue("stream");
				writer.EndTable();
				writer.WriteParamDelimiter();
				writer.Append(m_sNPLCallback.c_str());
				InvokeCallbackScript(writer.ToString().c_str(), (int)(writer.ToString().size()));
			}
		}
		// just for testing: remove this, dump to debug. 
		// ParaEngine::CLogger::GetSingleton().Write((const char*)buffer, (int)nByteCount);
	}
	// SLEEP(200);
	if (IsAborted())
		return 0;
	return nByteCount;
}

size_t ParaEngine::CUrlProcessor::CUrl_write_data_callback(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CUrlProcessor * pTask = (CUrlProcessor *)stream;
	if (pTask)
	{
		return pTask->write_data_callback(buffer, size, nmemb);
	}
	// If that amount differs from the amount passed to your function, it'll signal an error to the library and 
	// it will abort the transfer and return CURLE_WRITE_ERROR. 
	return 0;
}


int ParaEngine::CUrlProcessor::CUrl_progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (CAsyncLoader::GetSingleton().interruption_requested())
		return -1;
	if (clientp)
	{
		CUrlProcessor* pUrlProcessor = (CUrlProcessor*)clientp;
		if (pUrlProcessor->IsEnableProgressUpdate())
			return pUrlProcessor->progress_callback(dltotal, dlnow, ultotal, ulnow);
	}
	return 0;
}

int ParaEngine::CUrlProcessor::progress_callback(double dltotal, double dlnow, double ultotal, double ulnow)
{
	DWORD nCurTime = ::GetTickCount();
	const int nMinProgressCallbackInterval = 500;
	if ((nCurTime - m_nLastProgressTime) > nMinProgressCallbackInterval)
	{
		m_nLastProgressTime = nCurTime;
		// OUTPUT_LOG("file %s: %d/%d \n", m_url.c_str(), (int)dlnow, (int)dltotal);
		if (!m_sNPLCallback.empty())
		{
			NPL::CNPLWriter writer;
			writer.WriteName("msg");
			writer.BeginTable();
			if (!m_sSaveToFileName.empty())
			{
				// if a disk file name is specified, we shall also insert following field info to the message struct. 
				// {DownloadState=""|"complete"|"terminated", totalFileSize=number, currentFileSize=number, PercentDone=number}
				writer.WriteName("DownloadState");
				writer.WriteValue("");
				writer.WriteName("totalFileSize");
				writer.WriteValue((int)dltotal);
				writer.WriteName("currentFileSize");
				writer.WriteValue((int)dlnow);
				writer.WriteName("PercentDone");
				writer.WriteValue((float)(dlnow / dltotal));
			}
			writer.EndTable();
			writer.WriteParamDelimiter();
			writer.Append(m_sNPLCallback.c_str());

			InvokeCallbackScript(writer.ToString().c_str(), (int)(writer.ToString().size()));
		}
	}
	return 0;
}

size_t ParaEngine::CUrlProcessor::write_header_callback(void *buffer, size_t size, size_t nmemb)
{
	UpdateTime();
	int nByteCount = (int)size*(int)nmemb;
	if (nByteCount > 0)
	{
		int nOldSize = (int)m_header.size();
		m_header.resize(nOldSize + nByteCount);
		memcpy(&(m_header[nOldSize]), buffer, nByteCount);
	}
	if (IsAborted())
		return -1;
	return nByteCount;
}

size_t ParaEngine::CUrlProcessor::CUrl_write_header_callback(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CUrlProcessor * pTask = (CUrlProcessor *)stream;
	if (pTask)
	{
		return pTask->write_header_callback(buffer, size, nmemb);
	}
	// The callback function must return the number of bytes actually taken care of, or 
	// return -1 to signal error to the library (it will cause it to abort the transfer with 
	// a CURLE_WRITE_ERROR return code
	return -1;
}

void ParaEngine::CUrlProcessor::CompleteTask()
{
	// complete the request, call the callback and delete the task and free the handle
#ifdef _DEBUG
	OUTPUT_LOG("-->: URL request task %s completed \n", m_url.c_str());
#endif
	CAsyncLoader::GetSingleton().RemovePendingRequest(m_url.c_str());
	if (m_pFile)
	{
		m_pFile->close();
		SAFE_DELETE(m_pFile);
	}

	if (!m_sNPLCallback.empty())
	{
		NPL::CNPLWriter writer;
		writer.WriteName("msg");
		writer.BeginTable();

		if (!m_header.empty())
		{
			writer.WriteName("header");
			writer.WriteValue((const char*)(&(m_header[0])), (int)m_header.size());
		}

		if (IsEnableDataStreaming())
		{
			// indicate end of stream
			writer.WriteName("data");
			writer.WriteValue("");

			writer.WriteName("type");
			writer.WriteValue("stream");
		}
		else if (!m_data.empty())
		{
			writer.WriteName("data");
			writer.WriteValue((const char*)(&(m_data[0])), (int)m_data.size());
		}

		if (!m_sSaveToFileName.empty())
		{
			// if a disk file name is specified, we shall also insert following field info to the message struct. 
			// {DownloadState=""|"complete"|"terminated", totalFileSize=number, currentFileSize=number, PercentDone=number}
			writer.WriteName("DownloadState");
			if (m_responseCode == 200 && m_returnCode == CURLE_OK)
			{
				writer.WriteValue("complete");
				string sTmp = string("NPL.AsyncDownload Completed:") + m_url + "\n";
				CAsyncLoader::GetSingleton().log(sTmp);
			}
			else
			{
				writer.WriteValue("terminated");
				string sTmp = string("NPL.AsyncDownload Failed:") + m_url + "\n";
				CAsyncLoader::GetSingleton().log(sTmp);
			}
			writer.WriteName("totalFileSize");
			writer.WriteValue(m_nBytesReceived);
			writer.WriteName("currentFileSize");
			writer.WriteValue(m_nBytesReceived);
			writer.WriteName("PercentDone");
			writer.WriteValue(100);
		}


		writer.WriteName("code");
		writer.WriteValue(m_returnCode);
		writer.WriteName("rcode");
		writer.WriteValue(m_responseCode);
		writer.EndTable();
		writer.WriteParamDelimiter();
		writer.Append(m_sNPLCallback.c_str());

		InvokeCallbackScript(writer.ToString().c_str(), (int)(writer.ToString().size()));
	}
	if (m_pfuncCallBack)
	{
		m_pfuncCallBack(m_returnCode, this, this->m_pUserData);
	}
}

void ParaEngine::CUrlProcessor::AppendHTTPHeader(const char* text)
{
#ifdef EMSCRIPTEN
	std::string header = text;
	auto pos = header.find(':');
	if (pos != string::npos)
	{
		m_request_headers.push_back(header.substr(0, pos));
		m_request_headers.push_back(header.substr(pos + 1));
	}
#else
	if (text != 0) m_pHttpHeaders = curl_slist_append(m_pHttpHeaders, text);
#endif
}

CURLFORMcode ParaEngine::CUrlProcessor::AppendFormParam(const char* name, const char* value)
{
#ifndef EMSCRIPTEN
	return curl_formadd(&m_pFormPost, &m_pFormLast, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, value, CURLFORM_END);
#else
	return CURL_FORMADD_OK;
#endif
}

CURLFORMcode ParaEngine::CUrlProcessor::AppendFormParam(const char* name, const char* type, const char* file, const char* data, int datalen, bool bCacheData)
{
#ifndef EMSCRIPTEN
	CURLFORMcode rc = CURL_FORMADD_OK;
	if (bCacheData)
	{
		data = CopyRequestData(data, datalen);
	}

	/* file upload */
	if ((file != NULL) && (data == NULL))
	{
		rc = (type == NULL) ?
			curl_formadd(&m_pFormPost, &m_pFormLast, CURLFORM_COPYNAME, name,
				CURLFORM_FILE, file, CURLFORM_END) :
			curl_formadd(&m_pFormPost, &m_pFormLast, CURLFORM_COPYNAME, name,
				CURLFORM_FILE, file,
				CURLFORM_CONTENTTYPE, type, CURLFORM_END);
	}
	/* data field */
	else if ((file != NULL) && (data != NULL))
	{
		/* Add a buffer to upload */
		rc = (type != NULL) ?
			curl_formadd(&m_pFormPost, &m_pFormLast,
				CURLFORM_COPYNAME, name,
				CURLFORM_BUFFER, file, CURLFORM_BUFFERPTR, data, CURLFORM_BUFFERLENGTH, datalen,
				CURLFORM_CONTENTTYPE, type,
				CURLFORM_END) :
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
#else
	return CURL_FORMADD_OK;
#endif
}

void ParaEngine::CUrlProcessor::SetCallBack(URL_LOADER_CALLBACK pFuncCallback, CUrlProcessorUserData* pUserData, bool bDeleteUserData)
{
	m_pfuncCallBack = pFuncCallback;
	m_pUserData = pUserData;
	m_nUserDataType = bDeleteUserData ? 1 : 0;
}

bool ParaEngine::CUrlProcessor::IsTimedOut(DWORD nCurrentTime)
{
	return ((m_nStartTime + m_nTimeOutTime) < nCurrentTime);
}

DWORD ParaEngine::CUrlProcessor::UpdateTime()
{
	m_nStartTime = ::GetTickCount();
	return m_nStartTime;
}

void ParaEngine::CUrlProcessor::SetTimeOut(int nMilliSeconds)
{
	m_nTimeOutTime = nMilliSeconds;
}

int ParaEngine::CUrlProcessor::GetTimeOut()
{
	return m_nTimeOutTime;
}

bool ParaEngine::CUrlProcessor::IsEnableProgressUpdate() const
{
	return m_bEnableProgressUpdate;
}

void ParaEngine::CUrlProcessor::SetEnableProgressUpdate(bool val)
{
	m_bEnableProgressUpdate = val;
}

bool ParaEngine::CUrlProcessor::IsEnableDataStreaming() const
{
	return m_bEnableDataStreaming;
}

void ParaEngine::CUrlProcessor::SetEnableDataStreaming(bool val)
{
	m_bEnableDataStreaming = val;
}

const char* ParaEngine::CUrlProcessor::CopyRequestData(const char* pData, int nLength)
{
	if (!m_sRequestData.empty())
	{
		OUTPUT_LOG("error: only one copied data can be used in url request\n");
		// TODO: i may need an array of m_sRequestData to store multiple copied request data. 
	}
	m_sRequestData = std::string(pData, nLength);
	return m_sRequestData.c_str();
}

NPL::NPLObjectProxy& ParaEngine::CUrlProcessor::GetOptions()
{
	if (!m_options)
	{
		m_options.reset(new NPL::NPLObjectProxy());
	}
	return *(m_options.get());
}

bool ParaEngine::CUrlProcessor::IsSyncCallbackMode() const
{
	return m_bIsSyncCallbackMode;
}

void ParaEngine::CUrlProcessor::SetSyncCallbackMode(bool val)
{
	m_bIsSyncCallbackMode = val;
}

int ParaEngine::CUrlProcessor::InvokeCallbackScript(const char* sCode, int nLength)
{
	if (!IsSyncCallbackMode())
	{
		NPL::NPLRuntimeState_ptr pState = CGlobals::GetNPLRuntime()->GetRuntimeState(m_sNPLStateName);
		if (pState)
		{
			// thread safe
			return pState->activate(NULL, sCode, nLength);
		}
	}
	else
	{
		NPL::NPLRuntimeState_ptr pState = CGlobals::GetNPLRuntime()->GetRuntimeState(m_sNPLStateName);
		if (pState)
		{
			pState->DoString(sCode, nLength);
		}
	}
	return S_OK;
}

void ParaEngine::CUrlProcessor::SetAbortController(std::shared_ptr<AbortController> controller)
{
	m_abortController = controller;
}

bool ParaEngine::CUrlProcessor::IsAborted() const
{
	if ((m_abortController && m_abortController->isAborted()) || CAsyncLoader::GetSingleton().interruption_requested())
		return true;
	return false;
}

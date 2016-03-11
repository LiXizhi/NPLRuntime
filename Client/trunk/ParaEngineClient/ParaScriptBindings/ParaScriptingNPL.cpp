//-----------------------------------------------------------------------------
// Class:	NPL in the scripting interface
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.4
// Revised: 2005.4, 2007.2
// Desc: partially cross platformed. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace luabind;

#include "NPLRuntime.h"
#include "NPLNetServer.h"
#include "NPLHelper.h"
#include "NPLCompiler.h"
#include "ParaScriptingNPL.h"
#ifdef PARAENGINE_CLIENT
#include "EditorHelper.h"
#endif
#include "NPLNetClient.h"
#include "util/HttpUtility.h"
#include "util/StringHelper.h"
// #define USE_TINY_JSON
#ifdef USE_TINY_JSON
#include "util/tinyjson.hpp"
#elif defined USE_RAPID_JSON
#include "external/json/rapidjson.h"
#include "external/json/document.h"

#else
#include "json/json.h"
#endif
#include "UrlLoaders.h"
#include "AsyncLoader.h"
#include "EventsCenter.h"
#include "NPLHelper.h"
#include "ParaScriptingIO.h"

#include "memdebug.h"

namespace ParaScripting
{
	//////////////////////////////////////////////////////////////////////////
	//
	// NPL 
	//
	//////////////////////////////////////////////////////////////////////////
	int CNPL::activate_(const char * strNPLFileName, const char* sCode, int channel, int priority, int reliability)
	{
		return NPL::CNPLRuntime::GetInstance()->NPL_Activate(NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState(), strNPLFileName, sCode, 0, channel, priority, reliability);
	}

	int CNPL::activate2_(const char * strNPLFileName, const char* sCode)
	{
		int channel = NPL::CNPLRuntime::GetInstance()->NPL_GetDefaultChannel();
		int priority = NPL::MEDIUM_PRIORITY;
		int reliability = NPL::RELIABLE_ORDERED;
		return NPL::CNPLRuntime::GetInstance()->NPL_Activate(NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState(), strNPLFileName, sCode, 0, channel, priority, reliability);
	}

	int CNPL::activate(const object& strNPLFileName, const object& input)
	{
		int channel = NPL::CNPLRuntime::GetInstance()->NPL_GetDefaultChannel();
		int priority = NPL::MEDIUM_PRIORITY;
		int reliability = NPL::RELIABLE_ORDERED;
		NPL::CNPLRuntime::GetInstance()->NPL_GetChannelProperty(channel, &priority, &reliability);
		return activate5(strNPLFileName,input,channel,priority, reliability);
	}

	int CNPL::activate1(const object& strNPLFileName)
	{
		const char* sNPLFileName=NULL;

		int nType = type(strNPLFileName);
		if(nType == LUA_TSTRING)
		{
			// arbitrary code is allowed in sCode
			sNPLFileName = object_cast<const char*>(strNPLFileName);
		}
		return NPL::CNPLRuntime::GetInstance()->NPL_Activate(NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(strNPLFileName), 
			sNPLFileName);
	}

	int CNPL::activate3(const object& strNPLFileName, const object& input, int channel)
	{
		int priority = NPL::MEDIUM_PRIORITY;
		int reliability = NPL::RELIABLE_ORDERED;
		NPL::CNPLRuntime::GetInstance()->NPL_GetChannelProperty(channel, &priority, &reliability);
		return activate5(strNPLFileName,input,channel,priority, reliability);
	}
	int CNPL::activate5(const object& strNPLFileName, const object& input, int channel, int priority, int reliability)
	{
		// when sending a message, no serious validation(security) is performed. It is generally considered the receiver's job.
		StringBuilder sCode;
		const char* sNPLFileName=NULL;

		int nType = type(strNPLFileName);
		if(nType == LUA_TSTRING)
		{
			// arbitrary code is allowed in sCode
			sNPLFileName = object_cast<const char*>(strNPLFileName);
		}

		nType = type(input);
		if(nType == LUA_TSTRING)
		{
			// arbitrary code is allowed in sCode
			int nSize = 0;
			const char* pStr = NPL::NPLHelper::LuaObjectToString(input, &nSize);
			sCode.append(pStr, nSize);
		}
		else
		{
			// reserve some space for average code size.
			sCode.reserve(100);

			// encode the table or variables in to the "msg" variable and serialize in to sCode string.
			// it is the receiver's responsibility to validate the scode according to its source.
			NPL::NPLHelper::SerializeToSCode("msg", input, sCode);
		}

		return NPL::CNPLRuntime::GetInstance()->NPL_Activate(NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(strNPLFileName), 
			sNPLFileName, sCode.c_str(), (int)sCode.size(), channel, priority, reliability);
	}

	void CNPL::call(const object& strNPLFileName, const object& input )
	{
		string sCode;
		const char* sNPLFileName=NULL;

		int nType = type(strNPLFileName);
		if(nType == LUA_TSTRING)
		{
			// arbitrary code is allowed in sCode
			sNPLFileName = object_cast<const char*>(strNPLFileName);
		}

		nType = type(input);
		if(nType == LUA_TSTRING)
		{
			// arbitrary code is allowed in sCode
			sCode = object_cast<const char*>(input);
		}
		else
		{
			// reserve some space for average code size.
			sCode.reserve(100);
			// encode the table or variables in to the "msg" variable and serialize in to sCode string.
			// it is the receiver's responsibility to validate the scode according to its source.
			NPL::NPLHelper::SerializeToSCode("msg", input, sCode);
		}

		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(strNPLFileName);
		if(runtime_state.get() != 0)
		{
			NPL::NPLFileName filename(sNPLFileName);
			
			runtime_state->LoadFile_any(filename.sRelativePath, false);
			runtime_state->ActivateFile_any(filename.sRelativePath, sCode.c_str(), (int)sCode.size());
		}
	}

	void CNPL::call_(const char * strNPLFileName, const char* sCode)
	{
		NPL::NPLFileName filename(strNPLFileName);
		NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState()->ActivateFile_any(filename.sRelativePath, sCode);
	}

	void CNPL::this_(const object& funcActivate)
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(funcActivate);
		if(runtime_state.get() != 0)
		{
			runtime_state->BindFileActivateFunc(funcActivate);
		}
	}

	ParaAttributeObject CNPL::GetAttributeObject()
	{
		return ParaAttributeObject((IAttributeFields*)(NPL::CNPLRuntime::GetInstance()));
	}

	void CNPL::load(const object& filePath, bool bReload)
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(filePath);
		if(runtime_state.get() != 0)
		{
			int nType = type(filePath);
			if(nType == LUA_TSTRING)
			{
				NPL::NPLFileName filename(object_cast<const char*>(filePath));
				runtime_state->LoadFile_any(filename.sRelativePath, bReload);
			}
		}
	}

	void CNPL::load_(const char* filePath, bool bReload)
	{
		NPL::CNPLRuntime::GetInstance()->NPL_LoadFile(NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState(), filePath, bReload);
	}

	void CNPL::load1(const object& filePath)
	{
		CNPL::load(filePath, false);
	}

	void CNPL::EnableNetwork( bool bEnable, const char* CenterName, const char* password )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_EnableNetwork(bEnable, CenterName, password);
	}

	void CNPL::AddDNSRecord(const char * sDNSName, const char* sAddress)
	{
		NPL::CNPLRuntime::GetInstance()->NPL_AddDNSRecord(sDNSName, sAddress);
	}

	std::string CNPL::GetExternalIP()
	{
		return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetExternalIP();
	}

	bool CNPL::SetTimer(const object& nIDEvent, float fElapse, const object& strNPLFileName)
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(nIDEvent);
		if(runtime_state.get() != 0)
		{
			int nID = 0;
			if(type(nIDEvent) == LUA_TNUMBER)
				nID = object_cast<int>(nIDEvent);

			if(type(strNPLFileName) == LUA_TSTRING)
			{
				return NPL::CNPLRuntime::GetInstance()->NPL_SetTimer(runtime_state, nID, fElapse, object_cast<const char*>(strNPLFileName));
			}
		}
		return false;
	}

	bool CNPL::ChangeTimer( const object& nIDEvent, int dueTime, int period )
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(nIDEvent);
		if(runtime_state.get() != 0)
		{
			int nID = 0;
			if(type(nIDEvent) == LUA_TNUMBER)
				nID = object_cast<int>(nIDEvent);
			return NPL::CNPLRuntime::GetInstance()->NPL_ChangeTimer(runtime_state,nID, dueTime, period);
		}
		return false;
	}

	bool CNPL::KillTimer(const object& nIDEvent)
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(nIDEvent);
		if(runtime_state.get() != 0)
		{
			int nID = 0;
			if(type(nIDEvent) == LUA_TNUMBER)
				nID = object_cast<int>(nIDEvent);
			return NPL::CNPLRuntime::GetInstance()->NPL_KillTimer(runtime_state, nID);
		}
		return false;
	}

	void CNPL::DoString(const object&  sCode)
	{
		DoString2(sCode, NULL);
	}

	void CNPL::DoString2( const object& sCode, const char* sFilename )
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(sCode);
		if(runtime_state.get() != 0)
		{
			int nType = type(sCode);
			if(nType == LUA_TSTRING)
			{
				// arbitrary code is allowed in sCode, make sure DoString is called from authenticated code.
				int nSize = 0;
				const char* pStr = NPL::NPLHelper::LuaObjectToString(sCode, &nSize);
				runtime_state->DoString(pStr, nSize, sFilename, false);
			}
		}
	}

	void CNPL::DoString_(const char* sCode)
	{
		// check for security?
		NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState()->DoString(sCode, (int)strlen(sCode));
	}

	const string& CNPL::SerializeToSCode(const char* sStorageVar, const object& input)
	{
		NPL::NPLRuntimeState_ptr runtime_state = NPL::CNPLRuntimeState::GetRuntimeStateFromLuaObject(input);
		if(runtime_state.get() != 0)
		{
			std::string& sCode = runtime_state->GetStringBuffer(0);
			sCode.clear();
			NPL::NPLHelper::SerializeToSCode(sStorageVar, input, sCode);
			return sCode;
		}
		else
		{
			return CGlobals::GetString();
		}
	}

	bool CNPL::IsSCodePureData( const char* sCode )
	{
		return NPL::NPLHelper::IsSCodePureData(sCode);
	}

	bool CNPL::IsPureData( const char* sCode )
	{
		return NPL::NPLHelper::IsPureData(sCode);
	}

	bool CNPL::IsPureTable( const char* sCode )
	{
		return NPL::NPLHelper::IsPureTable(sCode);
	}

	const char* CNPL::GetSourceName()
	{
		return NPL::CNPLRuntime::GetInstance()->NPL_GetSourceName().c_str();
	}

	void CNPL::SetSourceName( const char* sName )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_SetSourceName(sName);
	}

	void CNPL::SetDefaultChannel( int channel_ID )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_SetDefaultChannel(channel_ID);
	}

	int CNPL::GetDefaultChannel()
	{
		return NPL::CNPLRuntime::GetInstance()->NPL_GetDefaultChannel();
	}

	void CNPL::SetChannelProperty( int channel_ID, int priority, int reliability )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_SetChannelProperty(channel_ID, priority, reliability);
	}

	void CNPL::ResetChannelProperties()
	{
		NPL::CNPLRuntime::GetInstance()->NPL_ResetChannelProperties();

	}

	void CNPL::GetChannelProperty( int channel_ID, int* priority, int* reliability )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_GetChannelProperty(channel_ID, priority, reliability);
	}

	void CNPL::RegisterWSCallBack( const char * sWebServiceFile, const char * sCode )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_RegisterWSCallBack(sWebServiceFile, sCode);
	}

	void CNPL::UnregisterWSCallBack( const char * sWebServiceFile )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_UnregisterWSCallBack(sWebServiceFile);
	}

	const char* CNPL::GetFileName()
	{
		// TODO:: 
		return NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState()->GetFileName().c_str();
	}

	const char* CNPL::GetNPLID()
	{
		//"Seer_Dev" for developer internal use
		//"Seer" for release
           
		return "Seer";
	}

	void CNPL::AsyncDownload( const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName )
	{
		NPL::CNPLRuntime::GetInstance()->AsyncDownload(url, destFolder, callbackScript, DownloaderName);
	}

	void CNPL::CancelDownload( const char* DownloaderName )
	{
		NPL::CNPLRuntime::GetInstance()->CancelDownload(DownloaderName);
	}

	int CNPL::Download( const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName )
	{
		return NPL::CNPLRuntime::GetInstance()->Download(url, destFolder, callbackScript, DownloaderName);
	}

	void CNPL::Compile( const char* arguments )
	{
#ifdef USE_NPLCompiler
		string cmdLine = "\"script/bin/luac5.1.exe\" ";
		cmdLine += arguments;
		if(NPL::CNPLCompiler::Compile(cmdLine.c_str()) !=S_OK)
		{
			OUTPUT_LOG("\nerror: Failed to compile with %s \n\n", arguments);
		}
#else
		OUTPUT_LOG("warning: NPL.Compile is obsoleted. Use the NPL script version instead \n");
#endif
		//ParaEngine::CEditorHelper::CreateProcess("script/bin/luac5.1.exe", arguments, true);
		//ParaEngine::CEditorHelper::ShellExecute("open", "script/bin/luac5.1.exe", arguments, NULL, 1);
	}

	bool CNPL::ChangeRequestPoolSize( const char* sPoolName, int nCount )
	{
		return NPL::CNPLRuntime::GetInstance()->ChangeRequestPoolSize(sPoolName, nCount);
	}

	bool CNPL::AppendURLRequest1( const char * url, const char* sCallback, const object& sForm, const char* sPoolName )
	{
		if(url == NULL || sCallback == NULL)
			return false;

		CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());
		CUrlLoader* pLoader = new CUrlLoader();
		CUrlProcessor* pProcessor = new CUrlProcessor();

		// command line parameters
		while (url[0] == '-')
		{
			if (url[1] == 'I' && url[2] == ' ')
			{
				// "-I" to get headers only
				pProcessor->SetHeadersOnly();
				url += 3;
			}
		}

		// do not enable progress, if we are just downloading a simple url in plain text, use DownloadFile for large file based request. 
		pProcessor->SetEnableProgressUpdate(false);
		
		ParaEngine::CUrlBuilder urlBuilder;
		urlBuilder.SetBaseURL(url);

		

		if(type(sForm) == LUA_TTABLE)
		{
			for (luabind::iterator itCur(sForm), itEnd;itCur!=itEnd;++itCur)
			{
				// we only serialize item with a string key
				const object& key = itCur.key();
				if(type(key) == LUA_TSTRING)
				{
					const char* sKey = object_cast<const char*>(key);
					const object& input = *itCur;
					switch(type(input))
					{
					case LUA_TNUMBER:
						{
							double value = object_cast<double>(input);

							if(strcmp(sKey, "request_timeout") ==0)
							{
								pProcessor->SetTimeOut((int)value);
							}
							else
							{
								char buff[40];
								// similar to "%.5f" but without trailing zeros. 
								ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5);
								pProcessor->AppendFormParam(sKey, buff);
							}
							break;
						}
					case LUA_TBOOLEAN:
						{
							bool bValue = object_cast<bool>(input);
							if(strcmp(sKey, "forbid_reuse") ==0)
							{
								pProcessor->SetForbidReuse(bValue);
							}
							else
							{
								pProcessor->AppendFormParam(sKey, bValue ? "true" : "false");
							}
							break;
						}
					case LUA_TSTRING:
						{
							pProcessor->AppendFormParam(sKey, object_cast<const char*>(input));
							break;
						}
					case LUA_TUSERDATA:
						{
							// this must be a CParaFile object. this is currently the only object supported. 
							try
							{
								ParaScripting::ParaFileObject file = object_cast<ParaScripting::ParaFileObject>(input);
								const char* pStr = file.GetBase64String();
								if(pStr)
								{
									pProcessor->AppendFormParam(sKey, pStr);
								}
								// TODO: LXZ: figure out a way to check user data type, instead of throwing exceptions
								// the following two may be possible ways, but I have not figured out how.

								//ParaScripting::ParaFileObject* pFile = (ParaScripting::ParaFileObject*)luaL_checkudata(input.interpreter(), 0, "ParaIO.ParaFileObject");

								//ParaScripting::ParaFileObject * pFile = object_cast<ParaScripting::ParaFileObject*>(input);
								//if(pFile)
								//{
								//	//OUTPUT_LOG("file name:%s\n", pFile->readline());
								//}
							}
							catch (...)
							{
								OUTPUT_LOG("warning: only ParaFileObject user type is supported in msg input. \n");
							}
							break;
						}
					case LUA_TTABLE:
						{
							/* for file. 
							{name = {file="/tmp/test.txt",	type="text/plain"}}
							{name = {file="dummy.html",	data="<html><bold>bold</bold></html>", type="text/html"}}
							*/
							const char* file = NULL;
							const char* type_ = NULL;
							std::string data;
							for (luabind::iterator itCur1(input), itEnd1;itCur1!=itEnd1;++itCur1)
							{
								const object& key = itCur1.key();
								if(type(key) == LUA_TSTRING)
								{
									const char* sKeyName = object_cast<const char*>(key);
									if(strcmp(sKeyName, "file") ==0)
									{
										file = object_cast<const char*>(*itCur1);
									}
									else if(strcmp(sKeyName,"data") ==0)
									{
										data = object_cast<std::string>(*itCur1);
									}
									else if(strcmp(sKeyName,"type") ==0)
									{
										type_ = object_cast<const char*>(*itCur1);
									}
								}
							}
							pProcessor->AppendFormParam(sKey, type_, file, data.c_str(), (int)data.size(), true);
							break;
						}
					}
				}
				else if(type(key) == LUA_TNUMBER)
				{
					int nKey = object_cast<int>(key);

					const object& input = *itCur;
					const char* sCode = NULL;
					switch(type(input))
					{
					case LUA_TNUMBER:
						{
							double value = object_cast<double>(input);
							char buff[40];
							ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
							sCode = buff;
							if((nKey%2) == 1)
								urlBuilder.InsertParam(nKey/2, sCode, NULL);
							else
								urlBuilder.InsertParam(nKey/2-1, NULL, sCode);
							break;
						}
					case LUA_TBOOLEAN:
						{
							bool bValue = object_cast<bool>(input);
							sCode = bValue ? "true" : "false";
							if((nKey%2) == 1)
								urlBuilder.InsertParam(nKey/2, sCode, NULL);
							else
								urlBuilder.InsertParam(nKey/2-1, NULL, sCode);
							break;
						}
					case LUA_TSTRING:
						{
							sCode = object_cast<const char*>(input);
							if((nKey%2) == 1)
								urlBuilder.InsertParam(nKey/2, sCode, NULL);
							else
								urlBuilder.InsertParam(nKey/2-1, NULL, sCode);
							break;
						}
					}
				}
			}
		}
		
		pLoader->SetUrl(urlBuilder.ToString().c_str());
		pProcessor->SetUrl(urlBuilder.ToString().c_str());
		pProcessor->SetScriptCallback(sCallback);

		return (pAsyncLoader->AddWorkItem( pLoader, pProcessor, NULL, NULL,ResourceRequestID_Web) == S_OK);
	}

	string CNPL::EncodeURLQuery( const char * baseUrl, const object& sParams )
	{
		ParaEngine::CUrlBuilder urlBuilder;

		if(baseUrl)
			urlBuilder.SetBaseURL(baseUrl);

		if(type(sParams) == LUA_TTABLE)
		{
			for (luabind::iterator itCur(sParams), itEnd;itCur!=itEnd;++itCur)
			{
				// we only serialize item with a string key
				const object& key = itCur.key();
				if(type(key) == LUA_TNUMBER)
				{
					int nKey = object_cast<int>(key);
					const object& input = *itCur;
					const char* sCode = NULL;
					switch(type(input))
					{
					case LUA_TNUMBER:
						{
							double value = object_cast<double>(input);
							char buff[40];
							ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
							sCode = buff;
							if((nKey%2) == 1)
								urlBuilder.InsertParam(nKey/2, sCode, NULL);
							else
								urlBuilder.InsertParam(nKey/2-1, NULL, sCode);
							break;
						}
					case LUA_TBOOLEAN:
						{
							bool bValue = object_cast<bool>(input);
							sCode = bValue ? "true" : "false";
							if((nKey%2) == 1)
								urlBuilder.InsertParam(nKey/2, sCode, NULL);
							else
								urlBuilder.InsertParam(nKey/2-1, NULL, sCode);
							break;
						}
					case LUA_TSTRING:
						{
							sCode = object_cast<const char*>(input);
							if((nKey%2) == 1)
								urlBuilder.InsertParam(nKey/2, sCode, NULL);
							else
								urlBuilder.InsertParam(nKey/2-1, NULL, sCode);
							break;
						}
					}
				}
			}
		}
		return urlBuilder.ToString();
	}
#ifdef USE_TINY_JSON
	template <class T>
	void traverse(json::grammar<char>::variant const & var, const object& outTable, const T& sKey, bool bFirstTable=false)
	{
		if(var->empty())
		{
			// variant is empty => it's a "null" value
		}
		else if(var->type() == typeid(bool))
		{
			// variant is of type "bool"...
			bool bValue = boost::any_cast< bool >(*var);
			outTable[sKey] = bValue;
		}
		else if(var->type() == typeid(int))
		{
			// variant is of type "int"...
			int iValue = boost::any_cast< int >(*var);
			outTable[sKey] = iValue;
		}
		else if(var->type() == typeid(double))
		{
			// variant is of type "double"...
			double dValue = boost::any_cast< double >(*var);
			outTable[sKey] = dValue;

		}
		else if(var->type() == typeid(std::string))
		{
			// variant is a string...
			std::string strValue = boost::any_cast< std::string >(*var);
			outTable[sKey] = strValue;
		}
		else if(var->type() == typeid(json::grammar<char>::array))
		{
			if(!bFirstTable)
			{
				object table = newtable(outTable.interpreter());
				outTable[sKey] = table;

				// variant is an array => use recursion
				json::grammar<char>::array const & a = boost::any_cast< json::grammar<char>::array >(*var);

				int i = 0;
				for(json::grammar<char>::array::const_iterator it = a.begin(); it != a.end(); ++it)
				{
					traverse(*it, table, ++i);
				}
			}
			else
			{
				// variant is an array => use recursion
				json::grammar<char>::array const & a = boost::any_cast< json::grammar<char>::array >(*var);

				int i = 0;
				for(json::grammar<char>::array::const_iterator it = a.begin(); it != a.end(); ++it)
				{
					traverse(*it, outTable, ++i);
				}
			}

		}
		else if(var->type() == typeid(json::grammar<char>::object))
		{
			if(!bFirstTable)
			{
				object table = newtable(outTable.interpreter());
				outTable[sKey] = table;

				// variant is an object => use recursion
				json::grammar<char>::object const & o = boost::any_cast< json::grammar<char>::object >(*var);

				for(json::grammar<char>::object::const_iterator it = o.begin(); it != o.end(); ++it)
				{
					std::string strName = (*it).first;
					traverse((*it).second, table, strName);
				}
			}
			else
			{
				// variant is an object => use recursion
				json::grammar<char>::object const & o = boost::any_cast< json::grammar<char>::object >(*var);

				for(json::grammar<char>::object::const_iterator it = o.begin(); it != o.end(); ++it)
				{
					std::string strName = (*it).first;
					traverse((*it).second, outTable, strName);
				}
			}
		}
		else
		{
			// ERROR: unknown type...
		}
	}
#elif defined USE_RAPID_JSON
    template <class T>
    void traverse(const rapidjson::Value & var, const object& outTable, const T& sKey, bool bFirstTable=false)
    {
        if(var.IsNull())
        {
            // variant is empty => it's a "null" value
        }
        else if(var.IsBool())
        {
            // variant is of type "bool"...
            outTable[sKey] = var.GetBool();
        }
        //else if(var.isIntegral())
        //{
        //	// variant is of type "int"...
        //	outTable[sKey] = var.asInt();
        //}
        //else if(var.isDouble())
        else if(var.IsNumber())
        {
            // variant is of type "double"...
            outTable[sKey] = var.GetDouble();
            
        }
        else if(var.IsString())
        {
            // variant is a string...
            outTable[sKey] = var.GetString();
        }
        else if(var.IsArray())
        {
            if(!bFirstTable)
            {
                object table = newtable(outTable.interpreter());
                outTable[sKey] = table;
                
                // variant is an array => use recursion
                int nSize = var.Size();
                for(int i=0;i<nSize; ++i)
                {
                    traverse(var[i], table, i+1); // since NPL use 1 based index
                }
            }
            else
            {
                // variant is an array => use recursion
                int nSize = var.Size();
                for(int i=0;i<nSize; ++i)
                {
                    traverse(var[i], outTable, i+1); // since NPL use 1 based index
                }
            }
            
        }
        else if(var.IsObject())
        {
            if(!bFirstTable)
            {
                object table = newtable(outTable.interpreter());
                outTable[sKey] = table;
                
                // variant is an object => use recursion
                rapidjson::Value::ConstMemberIterator itEnd = var.MemberonEnd();
                for(rapidjson::Value::ConstMemberIterator it = var.MemberonBegin(); it != itEnd; ++it)
                {
                    std::string strName = it->name.GetString();
                    traverse(it->value, table, strName);
                }
            }
            else
            {
                // variant is an object => use recursion
                rapidjson::Value::ConstMemberIterator itEnd = var.MemberonEnd();
                for(rapidjson::Value::ConstMemberIterator it = var.MemberonBegin(); it != itEnd; ++it)
                {
                    std::string strName = it->name.GetString();
                    traverse(it->value, outTable, strName);
                }
            }
        }
        else
        {
            // ERROR: unknown type...
        }
    }
#else
	template <class T>
	void traverse(const Json::Value & var, const object& outTable, const T& sKey, bool bFirstTable=false)
	{
		if(!var)
		{
			// variant is empty => it's a "null" value
		}
		else if(var.isBool())
		{
			// variant is of type "bool"...
			outTable[sKey] = var.asBool();
		}
		//else if(var.isIntegral())
		//{
		//	// variant is of type "int"...
		//	outTable[sKey] = var.asInt();
		//}
		//else if(var.isDouble())
		else if(var.isNumeric())
		{
			// variant is of type "double"...
			outTable[sKey] = var.asDouble();

		}
		else if(var.isString())
		{
			// variant is a string...
			outTable[sKey] = var.asString();
		}
		else if(var.isArray())
		{
			if(!bFirstTable)
			{
				object table = newtable(outTable.interpreter());
				outTable[sKey] = table;

				// variant is an array => use recursion
				int nSize = var.size();
				for(int i=0;i<nSize; ++i)
				{
					traverse(var[i], table, i+1); // since NPL use 1 based index
				}
			}
			else
			{
				// variant is an array => use recursion
				int nSize = var.size();
				for(int i=0;i<nSize; ++i)
				{
					traverse(var[i], outTable, i+1); // since NPL use 1 based index
				}
			}

		}
		else if(var.isObject())
		{
			if(!bFirstTable)
			{
				object table = newtable(outTable.interpreter());
				outTable[sKey] = table;

				// variant is an object => use recursion
				Json::Value::const_iterator itEnd = var.end();
				for(Json::Value::const_iterator it = var.begin(); it != itEnd; ++it)
				{
					std::string strName = it.memberName();
					traverse(*it, table, strName);
				}
			}
			else
			{
				// variant is an object => use recursion
				Json::Value::const_iterator itEnd = var.end();
				for(Json::Value::const_iterator it = var.begin(); it != itEnd; ++it)
				{
					std::string strName = it.memberName();
					traverse(*it, outTable, strName);
				}
			}
		}
		else
		{
			// ERROR: unknown type...
		}
	}
#endif
	bool CNPL::FromJson( const char* sJson, const object& output )
	{
		if(sJson == NULL || type(output) != LUA_TTABLE )
			return false;
		try
		{
#ifdef USE_TINY_JSON
			std::string json = sJson; 
			json::grammar<char>::variant var = json::parse(json.begin(), json.end()); 

			if(var->type() != typeid(json::grammar<char>::object))
			{
				// ERROR: parsing failed (bad format)
				return false;
			}

			// further processing of the JSON object model
			traverse(var, output, 0, true);
#elif defined USE_RAPID_JSON
            rapidjson::Document doc;
            doc.Parse<0>(sJson);
            if(doc.HasParseError())
            {
                // report to the user the failure and their locations in the document.
                OUTPUT_LOG("warning: NPL.FromJson cannot parse input string. error message is %s\n", doc.GetParseError());
                return false;
            }
            traverse(doc, output, 0, true);
#else
			Json::Value var;   // will contains the root value after parsing.
			// strict mode: no comments are allowed, root must be array or object, and string must be in utf8
			Json::Reader reader(Json::Features().strictMode());

			bool parsingSuccessful = reader.parse( sJson, sJson + strlen(sJson), var, false);
			if ( !parsingSuccessful )
			{
				// report to the user the failure and their locations in the document.
				OUTPUT_LOG("warning: NPL.FromJson cannot parse input string. error message is %s\n", reader.getFormatedErrorMessages().c_str());
				return false;
			}
			traverse(var, output, 0, true);
#endif
		}
		catch (...)
		{
			OUTPUT_LOG("error parsing json string. NPL::FromJson throws an exception. \n");
			return false;
		}
		return true;
	}

	luabind::object CNPL::LoadTableFromString(const object& input)
	{
		if(type(input) == LUA_TSTRING) 
		{
			luabind::object out;
			int nSize = 0;
			const char* pStr = NPL::NPLHelper::LuaObjectToString(input, &nSize);
			NPL::NPLHelper::StringToLuaObject(pStr, nSize, out, input.interpreter());
			return out;
		}
		return luabind::object();
	}

	luabind::object CNPL::LoadObjectFromString(const object& input)
	{
		if (type(input) == LUA_TSTRING)
		{
			luabind::object out;
			int nSize = 0;
			const char* pStr = NPL::NPLHelper::LuaObjectToString(input, &nSize);
			NPL::NPLHelper::StringToAnyObject(pStr, nSize, out, input.interpreter());
			return out;
		}
		return luabind::object();
	}

	ParaScripting::ParaNPLRuntimeState CNPL::CreateRuntimeState( const string& name, int type_ )
	{
		return ParaScripting::ParaNPLRuntimeState(NPL::CNPLRuntime::GetInstance()->CreateRuntimeState(name, (NPL::NPLRuntimeStateType)type_));
	}

	ParaScripting::ParaNPLRuntimeState CNPL::GetRuntimeState( const string& name )
	{
		return ParaScripting::ParaNPLRuntimeState(NPL::CNPLRuntime::GetInstance()->GetRuntimeState(name));
	}

	ParaScripting::ParaNPLRuntimeState CNPL::CreateGetRuntimeState( const string& name, int type_ )
	{
		return ParaScripting::ParaNPLRuntimeState(NPL::CNPLRuntime::GetInstance()->CreateGetRuntimeState(name, (NPL::NPLRuntimeStateType)type_));
	}

	bool CNPL::DeleteRuntimeState( ParaNPLRuntimeState runtime_state )
	{
		if(runtime_state.m_rts!=0)
			return NPL::CNPLRuntime::GetInstance()->DeleteRuntimeState(runtime_state.m_rts->shared_from_this());
		return true;
	}

	void CNPL::StartNetServer( const object& server, const object& port)
	{
		NPL::CNPLRuntime::GetInstance()->NPL_StartNetServer(NPL::NPLHelper::LuaObjectToString(server), NPL::NPLHelper::LuaObjectToString(port));
	}

	void CNPL::StopNetServer()
	{
		NPL::CNPLRuntime::GetInstance()->NPL_StopNetServer();
	}

	void CNPL::AddPublicFile( const string& filename, int nID )
	{
		NPL::CNPLRuntime::GetInstance()->NPL_AddPublicFile(filename, nID);
	}

	void CNPL::ClearPublicFiles()
	{
		NPL::CNPLRuntime::GetInstance()->NPL_ClearPublicFiles();
	}

	bool CNPL::AddNPLRuntimeAddress( const object& npl_address )
	{
		int nType = type(npl_address);
		if(nType == LUA_TTABLE)
		{
			const char* host = 0;
			if(type(npl_address["host"]) == LUA_TSTRING){
				host = object_cast<const char*>(npl_address["host"]);
			}
			const char* port = 0;
			if(type(npl_address["port"]) == LUA_TSTRING){
				port = object_cast<const char*>(npl_address["port"]);
			}
			const char* nid = 0;
			if(type(npl_address["nid"]) == LUA_TSTRING){
				nid = object_cast<const char*>(npl_address["nid"]);
			}

			if(host == 0)
				host = "127.0.0.1";
			if(port == 0)
				port = "60001";
			if(nid == 0)
				nid = "localhost";
			NPL::NPLRuntimeAddress_ptr address(new NPL::NPLRuntimeAddress(host, port, nid));
			return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().AddNPLRuntimeAddress(address);
		}
		return false;
	}

	void CNPL::SetUseCompression(bool bCompressIncoming, bool bCompressOutgoing)
	{
		NPL::CNPLRuntime::GetInstance()->SetUseCompression(bCompressIncoming, bCompressOutgoing);
	}

	void CNPL::SetCompressionKey(const object& input)
	{
		// input: a table, such as { key = "", size = 100, UsePlainTextEncoding = 1}
		int nType = type(input);
		if(nType == LUA_TTABLE)
		{
			const char* sKey = object_cast<const char*>(input["key"]);
			int nSize = 0;
			if(type(input["size"]) == LUA_TNUMBER)
			{
				nSize = object_cast<int>(input["size"]);
			}
			int nUsePlainTextEncoding = 0;
			if(type(input["UsePlainTextEncoding"]) == LUA_TNUMBER)
			{
				nUsePlainTextEncoding = object_cast<int>(input["UsePlainTextEncoding"]);
			}
			NPL::CNPLRuntime::GetInstance()->SetCompressionKey((const byte*)sKey, nSize, nUsePlainTextEncoding);
		}
	}

	string CNPL::GetIP(const char* nid)
	{
		return NPL::CNPLRuntime::GetInstance()->NPL_GetIP(nid);
	}

	void CNPL::accept(const object& tid, const object& nid)
	{
		const char * sTID = NPL::NPLHelper::LuaObjectToString(tid);
		const char * sNID = NPL::NPLHelper::LuaObjectToString(nid);
		NPL::CNPLRuntime::GetInstance()->NPL_accept(sTID, sNID);
	}
	void CNPL::accept_(const char* tid, const char* nid)
	{
		NPL::CNPLRuntime::GetInstance()->NPL_accept(tid, nid);
	}

	void CNPL::reject(const object& nid)
	{
		const char * sNID = NULL;
		int nReason = 0;
		if(type(nid) == LUA_TTABLE)
		{
			sNID = NPL::NPLHelper::LuaObjectToString(nid["nid"]);
			if(type(nid["reason"]) == LUA_TNUMBER)
			{
				nReason = object_cast<int>(nid["reason"]);
			}
		}
		else
		{
			sNID = NPL::NPLHelper::LuaObjectToString(nid);
		}
		NPL::CNPLRuntime::GetInstance()->NPL_reject(sNID, nReason);
	}

	void CNPL::reject_(const char* sNID, int nReason)
	{
		NPL::CNPLRuntime::GetInstance()->NPL_reject(sNID, nReason);
	}

	void CNPL::RegisterEvent(int nType, const char* sID, const char* sScript)
	{
		std::string strID = "_n";
		if(nType == 0)
		{
			strID = "_n";
			strID += sID;
			CGlobals::GetEventsCenter()->RegisterEvent(strID, sScript);	
		}
	}

	void CNPL::UnregisterEvent(int nType, const char* sID)
	{
		std::string strID = "_n";
		if(nType == 0)
		{
			strID = "_n";
			strID += sID;
			CGlobals::GetEventsCenter()->UnregisterEvent(strID);
		}
	}

	// only used in NPL::GetStats
	struct NPL_GetNidsStr_Iterator : public NPL::CNPLConnectionManager::NPLConnectionCallBack
	{
	public:
		NPL_GetNidsStr_Iterator(){m_nids_str.reserve(500);}

		virtual int DoCallBack(const NPL::NPLConnection_ptr& c) 
		{
			if(c->IsAuthenticated())
			{
				const std::string& sNid = c->GetNID();
				if(!sNid.empty())
				{
					m_nids_str += sNid;
					m_nids_str += ",";
				}
			}
			return 0;
		};
		const std::string& ToString() {return m_nids_str;}

	private:
		std::string m_nids_str;
	};

	// only used in NPL::GetStats
	struct NPL_GetNidsArray_Iterator : public NPL::CNPLConnectionManager::NPLConnectionCallBack
	{
	public:
		NPL_GetNidsArray_Iterator(luabind::object& nids_array):m_nids_array(nids_array), m_nCount(0){}

		virtual int DoCallBack(const NPL::NPLConnection_ptr& c) 
		{
			if(c->IsAuthenticated())
			{
				const std::string& sNid = c->GetNID();
				if(!sNid.empty())
				{
					++ m_nCount;
					m_nids_array[m_nCount] = sNid;
				}
			}
			return 0;
		};

		luabind::object& GetNidArray() {return m_nids_array;}
	private:
		luabind::object& m_nids_array;
		int m_nCount;
	};

	luabind::object CNPL::GetStats(const object& input)
	{
		int nType = type(input);
		luabind::object output = luabind::newtable(input.interpreter());

		if(nType == LUA_TTABLE)
		{
			for (luabind::iterator itCur(input), itEnd; itCur!=itEnd; ++itCur)
			{
				// we only serialize item with a string key
				const object& key = itCur.key();
				if(type(key) == LUA_TSTRING)
				{
					std::string sFieldName = object_cast<const char*>(key);
					if(sFieldName == "connection_count")
					{
						int nConnCount = CGlobals::GetNPLRuntime()->GetNetServer()->GetConnectionManager().get_connection_count();
						output[sFieldName] = nConnCount;
					}
					else if(sFieldName == "nids_str")
					{
						NPL_GetNidsStr_Iterator iter;
						int nConnCount = CGlobals::GetNPLRuntime()->GetNetServer()->GetConnectionManager().ForEachConnection(&iter);
						output[sFieldName] = iter.ToString();
					}
					else if(sFieldName == "nids")
					{
						luabind::object nids_array = luabind::newtable(input.interpreter());
						NPL_GetNidsArray_Iterator iter(nids_array);
						int nConnCount = CGlobals::GetNPLRuntime()->GetNetServer()->GetConnectionManager().ForEachConnection(&iter);
						output[sFieldName] = iter.GetNidArray();
					}
				}
			}
			
		}
		return output;
	}

#pragma region NPL Runtime State
	//////////////////////////////////////////////////////////////////////////
	//
	// ParaNPLRuntimeState
	//
	//////////////////////////////////////////////////////////////////////////
	ParaNPLRuntimeState::~ParaNPLRuntimeState()
	{
		//m_rts.reset();
	}

	int ParaNPLRuntimeState::Start()
	{
		if(m_rts!=0)
		{
			m_rts->Run_Async();
		}
		return 0;
	}

	bool ParaNPLRuntimeState::Stop()
	{
		return true;
	}

	luabind::object ParaNPLRuntimeState::GetStats( const object& inout )
	{
		return object(inout);
	}

	ParaNPLRuntimeState::ParaNPLRuntimeState()
	{

	}

	ParaNPLRuntimeState::ParaNPLRuntimeState( NPL::CNPLRuntimeState* rts_ )
		: m_rts(rts_)
	{

	}

	ParaNPLRuntimeState::ParaNPLRuntimeState(NPL::NPLRuntimeState_ptr rts_)
		: m_rts(rts_.get())
	{
	}

	bool ParaNPLRuntimeState::IsValid()
	{
		return (m_rts!=0);
	}

	const char* ParaNPLRuntimeState::GetName() const
	{
		if(m_rts!=0)
		{
			return m_rts->GetName().c_str();
		}
		return NULL;
	}

	void ParaNPLRuntimeState::Reset()
	{
		if(m_rts!=0)
		{
			return m_rts->Reset();
		}
	}
	void ParaNPLRuntimeState::Reset1(const char* onResetScode)
	{
		if(m_rts!=0)
		{
			return m_rts->Reset(onResetScode);
		}
	}

	int ParaNPLRuntimeState::GetCurrentQueueSize()
	{
		if(m_rts!=0)
		{
			return m_rts->GetCurrentQueueSize();
		}
		return 0;
	}

	int ParaNPLRuntimeState::GetProcessedMsgCount()
	{
		if(m_rts!=0)
		{
			return m_rts->GetProcessedMsgCount();
		}
		return 0;
	}

	int ParaNPLRuntimeState::GetMsgQueueSize()
	{
		if(m_rts!=0)
		{
			return m_rts->GetMsgQueueSize();
		}
		return 0;
	}

	void ParaNPLRuntimeState::SetMsgQueueSize( int nSize /*= 500*/ )
	{
		if(m_rts!=0)
		{
			m_rts->SetMsgQueueSize(nSize);
		}
	}

#pragma endregion NPL Runtime State
}// namespace ParaScripting

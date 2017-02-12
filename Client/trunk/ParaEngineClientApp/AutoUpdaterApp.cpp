//-----------------------------------------------------------------------------
// Class:	Interprocess Application client. 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.4.22
// Desc:
//-----------------------------------------------------------------------------
#include "config.h"
#include "AutoUpdaterClient.h"
#include "ParaEngineClientApp.h"

#include "AutoUpdaterApp.h"
#include "CommandLineParser.hpp"
#include "InterprocessMsg.h"

#include "shlobj.h"

using namespace ParaEngine;

#ifndef OUTPUT_LOG
#define OUTPUT_LOG OutputDebugString
#endif

// uncomment following lines to debug 
//#undef OUTPUT_LOG
//#define OUTPUT_LOG(x) ::MessageBoxA(NULL, x, "LOG", MB_OK)

ParaEngine::CAutoUpdaterApp::CAutoUpdaterApp()
	:m_pAutoUpdater(NULL), m_pCmdLineParams(NULL),m_bAutoUpdateComplete(true)
{
	// NOTE: Add any trusted domains here, right now it is hard coded in disk. 
	m_trusted_domains.push_back("http://*.paraengine.com/webplayer/");
	m_trusted_domains.push_back("http://update.paraengine.com/");
	m_trusted_domains.push_back("http://*.61.com/webplayer/");
	m_trusted_domains.push_back("http://*.*.61.com/webplayer/");
	m_trusted_domains.push_back("http://update.61.com/");
	m_trusted_domains.push_back("http://*.magic.primo168.tw/");
	m_trusted_domains.push_back("http://*.*.primo168.tw/webplayer/");
}

ParaEngine::CAutoUpdaterApp::~CAutoUpdaterApp()
{
	SAFE_DELETE(m_pCmdLineParams);
	ReleaseAutoUpdater();
}

int ParaEngine::CAutoUpdaterApp::SetCommandLine(const char* sCmdLine)
{
	if(sCmdLine == NULL)
	{
		m_sCmdLine.clear();
		SAFE_DELETE(m_pCmdLineParams);
		m_pCmdLineParams = new CCommandLineParser();
	}
	else if(m_sCmdLine != sCmdLine)
	{
		m_sCmdLine = sCmdLine;
		SAFE_DELETE(m_pCmdLineParams);
		m_pCmdLineParams = new CCommandLineParser(sCmdLine);
		return 1;
	}
	return 0;
}

int ParaEngine::CAutoUpdaterApp::SetAppDir()
{
	// set the current working directory 
	char buffer[MAX_PATH]; 
	BOOL res; 
	memset(buffer, 0, sizeof(buffer)); 
	res = SHGetSpecialFolderPathA(NULL, buffer, CSIDL_LOCAL_APPDATA, 1); 
	if (res) 
	{ 
		std::string rootDir = (const char*)buffer;

		// set the application directory relative to local app data folder, it must be in the format "company_name/product_name", 
		// such as "ParaEngine/Demo" or slash can be in other form like "\\TaoMee/Haqi/". 
		// For security reasons, company name can only be "ParaEngine" or "TaoMee"
		std::string app_dir = m_pCmdLineParams->GetValue("app_dir", "");
		if(app_dir.empty())
			app_dir = "TaoMee/Haqi";
		std::string company_name, app_name, app_name_lower;
		int nState = 0;
		for(int i=0; i<(int)(app_dir.size()); ++i)
		{
			char c = app_dir[i];
			if(IsAlphaNumeric(c))
			{
				if(nState == 0)
					company_name += c;
				else if(nState == 1)
				{
					app_name += c;
					app_name_lower += tolower(c);
				}
			}
			else if(c=='\\' || c =='/')
			{
				if(company_name.empty())
					nState = 0;
				else if(app_name.empty())
					nState = 1;
			}
		}

		// check for security
		if((company_name == "ParaEngine" || company_name=="TaoMee") && 
			!app_name.empty() && app_name_lower != "redist")
		{
			app_dir = "\\" + company_name + "\\" + app_name;
		}
		else
		{
			MessageBoxA(NULL, "app_dir is not allowed", "", MB_OK);
			OUTPUT_LOG("warning: app_dir is not allowed. valid form is \\ParaEngine\\Taurus");
			return -1;
		}
		rootDir.append(app_dir);

		if(::SetCurrentDirectoryA(rootDir.c_str()))
		{
			OUTPUT_LOG(("ParaEngine working dir set to"+rootDir).c_str());
		}
		else
		{
			bool bFailed=false;
			// create directory if not exist.
			if(::CreateDirectoryA(rootDir.c_str(), NULL))
			{
				if(::SetCurrentDirectoryA(rootDir.c_str()))
				{
					OUTPUT_LOG(("ParaEngine working dir set to"+rootDir).c_str());
				}
			}
			if(bFailed)
			{
				MessageBoxA(NULL, "Failed setting working directory", "", MB_OK);
				OUTPUT_LOG("Failed setting working directory. Make sure you have CSIDL_LOCAL_APPDATA/[app_dir]");
			}
		}
	}
	else{
		OUTPUT_LOG("Failed getting CSIDL_LOCAL_APPDATA");
	}
#ifdef _DEBUG
	// for debugging purposes, we will set working dir to this folder. 
	::SetCurrentDirectoryA("D:\\lxzsrc\\ParaEngine\\ParaWorld\\");
#endif
	return 0;
}

bool ParaEngine::CAutoUpdaterApp::SendHostMsg( int nMsg, DWORD param1 /*= 0*/, DWORD param2 /*= 0*/, const char* filename /*= NULL*/, const char* sCode /*= NULL*/, int nPriority /*= 0*/ )
{
	return false;
}

void ParaEngine::CAutoUpdaterApp::SendError( int nErrorCode, const char* errorMsg /*= NULL*/ )
{
	SendHostMsg(PEAPP_Error, nErrorCode,0, NULL, errorMsg);
}

void ParaEngine::CAutoUpdaterApp::SendLog( const char* logMsg )
{
	SendHostMsg(PEAPP_Log, 0,0, NULL, logMsg);
}

bool ParaEngine::CAutoUpdaterApp::BeginCoreUpdate( const char* sCurVersion, const char* sSessionDir )
{
	NPL::CAutoUpdaterClient* pUpdater = CreateGetAutoUpdater();
	if(pUpdater)
	{
		m_bAutoUpdateComplete = false;
		// we will use the "AutoUpdater.dll" under the current working directory if it exists, 
		WIN32_FIND_DATAA FindFileData;
		HANDLE hFind;
		std::string sFileName = "AutoUpdater.dll";
		hFind = FindFirstFileA(sFileName.c_str(), &FindFileData);

		if(hFind == INVALID_HANDLE_VALUE)
		{
			// if auto updater file does not exist, copy it from the redist directory to the current app directory.
			char sWorkingDir[MAX_PATH];
			if(GetModuleFileName(NULL, sWorkingDir, MAX_PATH)>0)
			{
				std::string sModuleDir = sWorkingDir;
				size_t nPos = sModuleDir.find_last_of("\\/");
				if(nPos != std::string::npos)
				{
					sModuleDir = sModuleDir.substr(0, nPos+1);
					sModuleDir += sFileName;
					if(CopyFileA(sModuleDir.c_str(), sFileName.c_str(), FALSE))
					{
						hFind = FindFirstFileA(sFileName.c_str(), &FindFileData);
					}
				}
			}
		}

		if(hFind != INVALID_HANDLE_VALUE) 
		{
			FindClose(hFind);
			pUpdater->SetUpdaterDllPath("AutoUpdater.dll");

			std::string updateurl;
			if(m_pCmdLineParams){
				updateurl = m_pCmdLineParams->GetValue("updateurl", "");
			}
			
			if(! updateurl.empty())
			{
				if(IsUrlTrusted(updateurl))
				{
					pUpdater->SetUpdateUrl(updateurl);
				}
				else
				{
					MessageBoxA(NULL, "updateurl is not trusted", "", MB_OK);
					SendError(0, "updateurl is not trusted");
					return false;
				}
			}

			if(pUpdater->BeginUpdate(sCurVersion, sSessionDir) == S_OK)
			{
				SendLog("ParaEngine BeginCoreUpdate using ./AutoUpdater.dll");
				return true;
			}
			else
			{
				SendError(0, "Failed to BeginCoreUpdate using ./AutoUpdater.dll");
				return false;
			}
		}
		else
		{
			SendLog("can not find ./AutoUpdater.dll in current working directory. ");
		}
		return false;
	}
	else
	{
		SendLog("can not create AutoUpdater");
	}
	return false;
}

bool ParaEngine::CAutoUpdaterApp::TryGetNextCoreUpdateMessage()
{
	NPL::CAutoUpdaterClient* pUpdater = m_pAutoUpdater;
	if(pUpdater)
	{
		NPL::IAutoUpdaterClient::ProgressMessage msg;
		if(pUpdater->PeekUpdaterMessage(msg))
		{
			// we shall skip any duplicated AUP_PROGRESS message
			if(msg.m_eventType == NPL::CAutoUpdaterClient::AUP_PROGRESS)
			{
				while(pUpdater->PeekUpdaterMessage(msg))
				{
					if(msg.m_eventType != NPL::CAutoUpdaterClient::AUP_PROGRESS)
					{
						break;
					}
				}
			}
			OnProgress(msg);
			int nStatus = msg.m_eventType;
			return  (nStatus == NPL::CAutoUpdaterClient::AUP_COMPLETED) || 
				(nStatus == NPL::CAutoUpdaterClient::AUP_NOCHANGE);
		}
	}
	return false;
}

void ParaEngine::CAutoUpdaterApp::OnProgress( const NPL::IAutoUpdaterClient::ProgressMessage& msg )
{
	switch(msg.m_eventType)
	{
	case NPL::IAutoUpdaterClient::AUP_STARTED:
		SendHostMsg(PEAPP_UpdateProgress_Started, 0,0, NULL, msg.m_msg.c_str());
		("updt.s", msg.m_msg, 0);
		break;
	case NPL::IAutoUpdaterClient::AUP_NOCHANGE:
		SendHostMsg(PEAPP_UpdateProgress_NoChangeDetected, 0,0, NULL, msg.m_msg.c_str());
		m_bAutoUpdateComplete = true;
		break;
	case NPL::IAutoUpdaterClient::AUP_UNKNOWN:
		SendHostMsg(PEAPP_UpdateProgress_UnknownVersion, 0,0, NULL, msg.m_msg.c_str());
		break;
	case NPL::IAutoUpdaterClient::AUP_BROKENFILE:
		SendHostMsg(PEAPP_UpdateProgress_BrokenFile, msg.m_finishcount, msg.m_allcount, NULL, msg.m_msg.c_str());
		break;
	case NPL::IAutoUpdaterClient::AUP_ERROR:
		SendHostMsg(PEAPP_UpdateProgress_Error, 0,0, NULL, msg.m_msg.c_str());
		break;
	case NPL::IAutoUpdaterClient::AUP_DO_APPLY_PATCH:
		// download complete, now apply the patch. 
		if(m_pAutoUpdater!=0)
		{
			m_pAutoUpdater->ApplyPatch();
		}
		break;
	case NPL::IAutoUpdaterClient::AUP_COMPLETED:
		SendHostMsg(PEAPP_UpdateProgress_Completed, 0,0, NULL, msg.m_msg.c_str());
		m_bAutoUpdateComplete = true;
		break;
	case NPL::IAutoUpdaterClient::AUP_PROGRESS:
		SendHostMsg(PEAPP_UpdateProgress_Progress, msg.m_finishcount, msg.m_allcount);
		break;
	default:
		break;
	}
}

NPL::CAutoUpdaterClient* ParaEngine::CAutoUpdaterApp::CreateGetAutoUpdater()
{
	if(!m_pAutoUpdater)
	{
		m_pAutoUpdater = new NPL::CAutoUpdaterClient();
		m_bAutoUpdateComplete = false;
	}
	return m_pAutoUpdater;
}

void ParaEngine::CAutoUpdaterApp::ReleaseAutoUpdater()
{
	SAFE_DELETE(m_pAutoUpdater);
}

bool ParaEngine::CAutoUpdaterApp::IsUrlTrusted( const std::string& sUrl )
{
	Domain_Map_t::iterator itCur, itEnd = m_trusted_domains.end();

	for (itCur = m_trusted_domains.begin(); itCur!=itEnd; ++itCur)
	{
		if(MatchFilePattern(sUrl, *itCur) >= 0)
		{
			return true;
		}
	}
	return false;
}

int ParaEngine::CAutoUpdaterApp::MatchFilePattern( const std::string& input, const std::string& sPattern )
{
	int nInputSize = (int)(input.size());
	int nPatternSize = (int)(sPattern.size());
	int nPatternIndex = 0;
	bool bTokenMatched = false;
	for (int i=0; i<nInputSize;)
	{
		char c = input[i];
		if(nPatternIndex<nPatternSize)
		{
			char current = sPattern[nPatternIndex];

			if(current == '*')
			{
				// char look_ahead = sPattern[nPatternIndex];
				if(!IsAlphaNumeric(c))
				{
					++ nPatternIndex;
				}
				else
				{
					++ i;
					bTokenMatched = true;
				}
			}
			else
			{
				if(current != c)
				{
					return (bTokenMatched && nPatternIndex == (nPatternSize-1)) ? 1 : -1;
				}
				else
				{
					++ nPatternIndex;
					++ i;
					bTokenMatched = false;
				}
			}
		}
		else
		{
			// pattern is matched, but there are still unread input. 
			return 1;
		}
	}
	return 0;
}

bool ParaEngine::CAutoUpdaterApp::IsAlphaNumeric( char c )
{
	return (c >= '0' && c <= '9') ||  (c >= 'a' && c <= 'z') ||  (c >= 'A' && c <= 'Z');
}

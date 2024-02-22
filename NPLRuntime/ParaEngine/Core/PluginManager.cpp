//-----------------------------------------------------------------------------
// Class:	CPluginManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.8.4, revised 2009.7.2 (linux port)
// Desc: Windows and linux uses different dll loading mode. 
// This is a must read paper for shared libraries under linux: http://people.redhat.com/drepper/dsohowto.pdf
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PluginAPI.h"
#include "ParaEngineCore.h"
#include "FileManager.h"
#include "util/os_calls.h"
#include "util/regularexpression.h"
#include "IParaEngineApp.h"
#include "PluginManager.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

using namespace ParaEngine;
/*@def undefine this if all dll interface function must be defined.  */
#define NONE_STRICT_NPL_DLL_INTERFACE

//////////////////////////////////////////////////////////////////////////
//
// CPluginManager
//
//////////////////////////////////////////////////////////////////////////

CPluginManager::CPluginManager(void)
{
}

CPluginManager::~CPluginManager(void)
{
}

CPluginManager* CPluginManager::GetInstance()
{
	static CPluginManager g_instance;
	return &g_instance;
}


DLLPlugInEntity* CPluginManager::LoadDLL(const string&  sIdentifier, const string&  fileName)
{
	ParaEngine::Lock lock_(m_mutex);
	string sFileName;
	CParaFile::ToCanonicalFilePath(sFileName, fileName,false);
	pair<DLLPlugInEntity*, bool> res = CreateEntity(sIdentifier, sFileName);
	if(res.second == true)
	{
		DLLPlugInEntity* pNewEntity = res.first;
		pNewEntity->Init(sFileName.c_str()); 
	}
	return res.first;
}

DLLPlugInEntity* CPluginManager::GetPluginEntity(const string& fileName)
{
	ParaEngine::Lock lock_(m_mutex);
	string sFileName;
	CParaFile::ToCanonicalFilePath(sFileName, fileName,false);
	return (DLLPlugInEntity*)get(sFileName);
}

int CPluginManager::ActivateDLL(const string& sDllPath,int nType, void* pVoid)
{
	ParaEngine::Lock lock_(m_mutex);
	DLLPlugInEntity* pEntity =(DLLPlugInEntity*) get(sDllPath);
	if(pEntity!=NULL)
	{
		lock_.unlock();
		return pEntity->Activate(nType, pVoid);
	}
	return E_FAIL;
}

int CPluginManager::LoadAllDLLinDirectory(const string& sDirectory)
{
	CSearchResult* result = CGlobals::GetFileManager()->SearchFiles(CParaFile::GetParentDirectoryFromPath(sDirectory), "*.dll", "");
	int nNum = 0;

	if(result!=0)
	{
		nNum = result->GetNumOfResult();
		for (int i=0; i<nNum; ++i)
		{
			LoadDLL("", result->GetItem(i));
		}
	}

	return nNum;
}


//////////////////////////////////////////////////////////////////////////
//
// DLLPlugInEntity
//
//////////////////////////////////////////////////////////////////////////


DLLPlugInEntity::DLLPlugInEntity(const AssetKey& key) :AssetEntity(key), m_nLibVersion(0)
{
	m_pFuncActivate = NULL;
	m_hDLL = NULL;
}

DLLPlugInEntity::~DLLPlugInEntity()
{
	// free the library.
	if(IsValid())
	{
		FreeLibrary();
	}
}

int DLLPlugInEntity::Activate(int nType, void* pVoid)
{
	if(IsValid() && m_pFuncActivate!=0)
	{
		return m_pFuncActivate(nType, pVoid);
	}
	return E_FAIL;
}

const char* DLLPlugInEntity::GetDLLFilePath()
{
	return m_sDllFilePath.c_str();
}

ClassDescriptor* DLLPlugInEntity::GetClassDescriptor(int i)
{
	if(IsValid() && i>=0 && i<GetNumberOfClasses())
		return m_listClassDesc[i];
	else
		return NULL;
}

const char* DLLPlugInEntity::GetLibDescription()
{
	return m_sLibDescription.c_str();
}

int DLLPlugInEntity::GetNumberOfClasses()
{
	return (int)m_listClassDesc.size();
}

void DLLPlugInEntity::Init(const char* sFilename)
{
	OUTPUT_LOG("DLLPlugInEntity::Init %s \n", sFilename);

	m_bIsInitialized = true;
	m_bIsValid = true;

	m_sDllFilePath = sFilename;
	string sDLLPath = m_sDllFilePath; // CParaFile::ToCanonicalFilePath(m_sDllFilePath);
#if defined(WIN32) ||  PARA_TARGET_PLATFORM == PARA_PLATFORM_LINUX || PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC
	// replace sDLLPath's file extension with 'dll', it is 'so'. remove the heading 'lib' if there is one
	if(sDLLPath.size()>5)
	{
#if defined(WIN32)
		// remove the heading 'lib' if there is one
		sDLLPath = regex_replace(sDLLPath, regex("lib([\\w\\.]*)$"), "$1");
		// replace sDLLPath's file extension with 'dll', it is 'so'
		sDLLPath = regex_replace(sDLLPath, regex("so$"), "dll");
#elif PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC
        /*
        sDLLPath = regex_replace(sDLLPath, regex("lib([\\w\\.]*)$"), "$1");
        sDLLPath = regex_replace(sDLLPath, regex("([\\w\\.]+)$"), "lib$1");
        sDLLPath = regex_replace(sDLLPath, regex("dll$"), "dylib");
        */
        sDLLPath = "lib" + sDLLPath;
#else
		sDLLPath = regex_replace(sDLLPath, regex("lib([\\w\\.]*)$"), "$1");
		sDLLPath = regex_replace(sDLLPath, regex("([\\w\\.]+)$"), "lib$1");
		sDLLPath = regex_replace(sDLLPath, regex("dll$"), "so");
#endif
	}

#ifdef WIN32
	// check for core dll files
	if (GetKey().find("cAudioEngine") != std::string::npos || GetKey().find("PhysicsBT") != std::string::npos || GetKey().find("sqlite") != std::string::npos || GetKey().find("ParaWebView") != std::string::npos)
	{
		// we will try to find core dll files only in executable directory first, such as in "bin64/..."
		std::string sFullPath = CGlobals::GetApp()->GetModuleDir();
		sFullPath += sDLLPath;
		if (CParaFile::DoesFileExist2(sFullPath.c_str(), FILE_ON_DISK))
			sDLLPath = sFullPath;
	}
#endif

	// load the library.
	if ( CParaFile::DoesFileExist2(sDLLPath.c_str(), FILE_ON_DISK | FILE_ON_SEARCH_PATH, &sDLLPath) == 0)
	{
		// If dll file is Not found, search in zip archive, if there it is, unpack to a temp position.  
		if(CParaFile::DoesFileExist2(sDLLPath.c_str(), FILE_ON_ZIP_ARCHIVE))
		{
			CParaFile file;
			if (file.OpenFile(sDLLPath.c_str(), true, 0, false, FILE_ON_ZIP_ARCHIVE))
			{
				std::string sTargetFile = CParaFile::GetWritablePath() + "temp/plugins/" + CParaFile::GetFileName(sDLLPath);
				CParaFile::CreateDirectory(sTargetFile.c_str());
				CParaFile fileTarget;
				if (fileTarget.OpenFile(sTargetFile.c_str(), false))
				{
					fileTarget.SetEndOfFile();
					fileTarget.write(file.getBuffer(), file.getSize());
					
					OUTPUT_LOG("Security warning: dll file %s is deployed from zip archive to %s \n", sDLLPath.c_str(), sTargetFile.c_str());
					sDLLPath = sTargetFile;
				}else if (CParaFile::DoesAssetFileExist2(sTargetFile.c_str(), FILE_ON_DISK)) {
					sDLLPath = sTargetFile;
				}
			}
		}
		else
		{
			// also find dll in module directory. This could be a security problem.
			std::string sFullPath = CGlobals::GetApp()->GetModuleDir();
			sFullPath += sDLLPath;
			if (CParaFile::DoesFileExist2(sFullPath.c_str(), FILE_ON_DISK))
				sDLLPath = sFullPath;
		}
	}
	
#ifdef WIN32
	m_hDLL = (HINSTANCE)ParaEngine::LoadLibrary(sDLLPath.c_str());
#else
	//// if there is no slash in the file name, add './' to load only from the current directory. 
	//if (!CParaFile::IsAbsolutePath(sDLLPath))
	//{
	//	sDLLPath = std::string("./") + sDLLPath;
	//}
	fs::path filenamePath(sDLLPath);
	sDLLPath = filenamePath.string();
	m_hDLL = ParaEngine::LoadLibrary(sDLLPath.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif

	if (m_hDLL != NULL)
	{
#ifdef WIN32
		{
			// see http://support.microsoft.com/default.aspx?scid=kb;en-us;814472

			// ... initialization code
			pfnEnsureInit pfnDll= (pfnEnsureInit) ParaEngine::GetProcAddress(m_hDLL, "DllEnsureInit");
			if(pfnDll!=0)
			{
				// Exit, return; there is nothing else to do.
				pfnDll();
			}
		}
#endif

		lpFnLibDescription pLibDescription = (lpFnLibDescription)ParaEngine::GetProcAddress(m_hDLL, "LibDescription");

		if (pLibDescription != 0)
		{
			// call the function
			m_sLibDescription = pLibDescription();
			//OUTPUT_LOG("lib desc %s \r\n", m_sLibDescription.c_str());
		}
		else
		{
#if !defined(NONE_STRICT_NPL_DLL_INTERFACE)
#ifdef	WIN32
			OUTPUT_LOG("failed loading %s : because it does not expose the LibDescription method\r\n", sDLLPath.c_str());
#else
			const char* sErrorMsg = dlerror();
			if(sErrorMsg == 0)
				sErrorMsg = "unknown error";
			OUTPUT_LOG("warning: ParaEngine::GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
			// handle the error
			FreeLibrary();
#endif
		}

		lpFnLibVersion pLibVersion = (lpFnLibVersion)ParaEngine::GetProcAddress(m_hDLL, "LibVersion");
		if (pLibVersion  != 0)
		{
			// call the function
			m_nLibVersion = pLibVersion();
			unsigned long nParaEngineVersion = GetParaEngineVersion();
			//OUTPUT_LOG("lib version %d \r\n", (int)nParaEngineVersion);
			if(nParaEngineVersion >m_nLibVersion)
			{
				OUTPUT_LOG("warning: %s has a lower version number than the current instance of ParaEngine.\r\n", sDLLPath.c_str());
				// TODO: if the version is not compatible with the current ParaEngine version.
			}
		}
		else
		{
#if !defined(NONE_STRICT_NPL_DLL_INTERFACE)
#ifdef	WIN32
			OUTPUT_LOG("failed loading %s : because it does not expose the LibVersion method\r\n", sDLLPath.c_str());
#else
			const char* sErrorMsg = dlerror();
			if(sErrorMsg == 0)
				sErrorMsg = "unknown error";
			OUTPUT_LOG("warning: ParaEngine::GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
			// handle the error
			FreeLibrary();
#endif
		}
		int nClassNum=0;

		lpFnLibNumberClasses pLibNumberClasses = (lpFnLibNumberClasses)ParaEngine::GetProcAddress(m_hDLL, "LibNumberClasses");
		if (pLibNumberClasses != 0)
		{
			// call the function
			nClassNum= pLibNumberClasses();
			//OUTPUT_LOG("lib classes count %d \r\n", nClassNum);
			m_listClassDesc.reserve(nClassNum);
		}
		else
		{
#if !defined(NONE_STRICT_NPL_DLL_INTERFACE)
#ifdef	WIN32
			OUTPUT_LOG("failed loading %s : because it does not expose the LibNumberClasses method\r\n", sDLLPath.c_str());
#else
			const char* sErrorMsg = dlerror();
			if(sErrorMsg == 0)
				sErrorMsg = "unknown error";
			OUTPUT_LOG("warning: ParaEngine::GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
			// handle the error
			FreeLibrary();
#endif
		}
		lpFnLibClassDesc pLibClassDesc = (lpFnLibClassDesc)ParaEngine::GetProcAddress(m_hDLL, "LibClassDesc");
		if (pLibDescription != 0)
		{
			// call the function
			for (int i=0; i<nClassNum; ++i)
			{
				ClassDescriptor* pClassDesc = pLibClassDesc(i);
				if(pClassDesc!=0)
				{
					m_listClassDesc.push_back(pClassDesc);
				}
				else
				{
					OUTPUT_LOG("the %d th class in %s is not loaded \r\n", i, sDLLPath.c_str());
				}
			}
		}
		else
		{
#if !defined(NONE_STRICT_NPL_DLL_INTERFACE)
#ifdef	WIN32
			OUTPUT_LOG("failed loading %s : because it does not expose the LibClassDesc method\r\n", sDLLPath.c_str());
#else
			const char* sErrorMsg = dlerror();
			if(sErrorMsg == 0)
				sErrorMsg = "unknown error";
			OUTPUT_LOG("warning: ParaEngine::GetProcAddress( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
			// handle the error
			FreeLibrary();
#endif
		}

#ifdef WIN32
		lpFnLibInit pLibInit = (lpFnLibInit)ParaEngine::GetProcAddress(m_hDLL, "LibInit");
		if (pLibDescription != 0)
		{
			// call the function
			pLibInit();
		}
#endif
		lpFnLibInitParaEngine pLibInitParaEngine = (lpFnLibInitParaEngine)ParaEngine::GetProcAddress(m_hDLL, "LibInitParaEngine");
		if (pLibInitParaEngine != 0)
		{
			// call the function
			pLibInitParaEngine(CParaEngineCore::GetStaticInterface());
		}

		m_pFuncActivate = (lpFnLibActivate)ParaEngine::GetProcAddress(m_hDLL, "LibActivate");
	}
	else
	{
#ifdef WIN32
		char szBuf[256]; 
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError(); 

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		sprintf(szBuf, 
			"%s (%s) failed with error %d: %s\n", 
			"LoadLibrary", sDLLPath.c_str(), dw, (const char*)lpMsgBuf); 

		OUTPUT_LOG("%s", szBuf);
#else
		const char* sErrorMsg = dlerror();
		if(sErrorMsg == 0)
			sErrorMsg = "unknown error";
		OUTPUT_LOG("warning: ParaEngine::LoadLibrary( %s ) failed because %s\n", sDLLPath.c_str(), sErrorMsg);
#endif
		m_bIsValid = false;
	}
#else
	m_bIsValid = false;
#endif
	if(IsValid())
	{
		OUTPUT_LOG("Plug-in loaded: %s version: %d \r\n", sDLLPath.c_str(), GetLibVersion());
	}
	else
	{
		OUTPUT_LOG("Failed loading plug-in: %s\r\n", sDLLPath.c_str());
	}
}

void  DLLPlugInEntity::FreeLibrary()
{
	if(m_hDLL!=0)
	{
#ifdef WIN32
		{
			// see http://support.microsoft.com/default.aspx?scid=kb;en-us;814472
			pfnForceTerm pfnDll=( pfnForceTerm) ParaEngine::GetProcAddress(m_hDLL, "DllForceTerm");
			if(pfnDll!=0)
			{
				pfnDll();	
			}
		}
#endif
		ParaEngine::FreeLibrary(m_hDLL);
		m_hDLL = 0;
	}
	m_bIsValid = false;
}

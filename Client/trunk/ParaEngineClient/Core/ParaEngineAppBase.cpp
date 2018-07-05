//-----------------------------------------------------------------------------
// Class:	ParaEngineApp base
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.1.1
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "util/os_calls.h"
#include "util/StringHelper.h"
#include "ObjectAutoReleasePool.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIDirectInput.h"
#include "AttributesManager.h"
#include "PluginManager.h"
#include "MeshPhysicsObject.h"
#include "MeshObject.h"
#include "MissileObject.h"
#include "BipedObject.h"
#include "BMaxModel/BMaxObject.h"
#include "SkyMesh.h"
#include "BlockPieceParticle.h"
#include "ContainerObject.h"
#include "RenderTarget.h"
#include "WeatherEffect.h"
#include "OverlayObject.h"
#include "LightObject.h"
#include "NPLRuntime.h"
#include "EventsCenter.h"
#include "BootStrapper.h"
#include "NPL/NPLHelper.h"
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "FileManager.h"
#include "Archive.h"
#include "ParaEngineAppBase.h"
#include "NPLPackageConfig.h"
#include "IO/ResourceEmbedded.h"
#include "GeosetObject.h"
#include "AutoRigger.h"

using namespace ParaEngine;

CParaEngineApp* CParaEngineAppBase::g_pCurrentApp = NULL;

/** default bootstrapper file */
#define NPL_CODE_WIKI_BOOTFILE "script/apps/WebServer/WebServer.lua"

// temp TEST code here
void ParaEngine::CParaEngineAppBase::DoTestCode()
{
}

ParaEngine::CParaEngineAppBase::CParaEngineAppBase()
	: m_bEnable3DRendering(true), m_isTouching(false), m_nReturnCode(0), m_pSingletonReleasePool(NULL), m_nAppState(PEAppState_None), m_hasClosingRequest(false)
{
	InitCommon();
}

ParaEngine::CParaEngineAppBase::CParaEngineAppBase(const char* sCmd)
	: CCommandLineParams(sCmd), m_bEnable3DRendering(true), m_isTouching(false), m_nReturnCode(0), m_pSingletonReleasePool(NULL), m_nAppState(PEAppState_None), m_hasClosingRequest(false)
{
	InitCommon();
}

ParaEngine::CParaEngineAppBase::~CParaEngineAppBase()
{
	DestroySingletons();
	SetCurrentInstance(NULL);
}

void ParaEngine::CParaEngineAppBase::DeleteInterface()
{
	delete this;
}

BaseInterface* ParaEngine::CParaEngineAppBase::AcquireInterface()
{
	addref();
	return(BaseInterface*)this;
}

void ParaEngine::CParaEngineAppBase::ReleaseInterface()
{
	if (delref()){
		DeleteInterface();
	}
}

CParaEngineApp* ParaEngine::CParaEngineAppBase::GetInstance()
{
	return g_pCurrentApp;
}

CParaEngineAppBase::LifetimeType ParaEngine::CParaEngineAppBase::LifetimeControl()
{
	return wantsRelease;
}

void CParaEngineAppBase::SetCurrentInstance(CParaEngineApp* pInstance)
{
	g_pCurrentApp = pInstance;
}


void ParaEngine::CParaEngineAppBase::DestroySingletons()
{
	CObjectAutoReleasePool::DestoryInstance();
	SAFE_DELETE(m_pSingletonReleasePool);
}

void ParaEngine::CParaEngineAppBase::OnFrameEnded()
{
	CObjectAutoReleasePool::GetInstance()->clear();
}

bool ParaEngine::CParaEngineAppBase::InitCommandLineParams()
{
	const char* sWritablePath = GetAppCommandLineByParam("writablepath", NULL);
	if (sWritablePath && sWritablePath[0] != 0) {
		CParaFile::SetWritablePath(sWritablePath);
	}

	const char* nAssetLogLevel = GetAppCommandLineByParam("assetlog_level", NULL);
	if (nAssetLogLevel && nAssetLogLevel[0] != 0) 
	{
		int nLevel = 0;
		if (strcmp(nAssetLogLevel, "all") == 0)
			nLevel = ParaEngine::CAsyncLoader::Log_All;
		else if (strcmp(nAssetLogLevel, "remote") == 0)
			nLevel = ParaEngine::CAsyncLoader::Log_Remote;
		else if (strcmp(nAssetLogLevel, "error") == 0)
			nLevel = ParaEngine::CAsyncLoader::Log_Error;
		ParaEngine::CAsyncLoader::GetSingleton().SetLogLevel(0);
	}

	const char* sLogFile = GetAppCommandLineByParam("logfile", NULL);
	if (sLogFile && sLogFile[0] != 0){
		CLogger::GetSingleton().SetLogFile(sLogFile);
	}

	const char* sServerMode = GetAppCommandLineByParam("servermode", NULL);
	const char* sInteractiveMode = GetAppCommandLineByParam("i", NULL);
	bool bIsServerMode = (sServerMode && strcmp(sServerMode, "true") == 0);
	bool bIsInterpreterMode = (sInteractiveMode && strcmp(sInteractiveMode, "true") == 0);
	Enable3DRendering(!bIsServerMode && !bIsInterpreterMode);

	const char* sDevFolder = GetAppCommandLineByParam("dev", NULL);
	if (sDevFolder)
	{
		if (sDevFolder[0] == '\0' || (sDevFolder[1] == '\0' && (sDevFolder[0] == '/' || sDevFolder[0] == '.')))
		{
			sDevFolder = CParaFile::GetCurDirectory(0).c_str();
		}
		CParaFile::SetDevDirectory(sDevFolder);
	}
	return true;
}

bool ParaEngine::CParaEngineAppBase::ForceRender()
{
	return false;
}

const char * ParaEngine::CParaEngineAppBase::GetModuleDir() 
{ 
	return m_sModuleDir.c_str(); 
}

void ParaEngine::CParaEngineAppBase::InitCommon()
{
	CStaticInitRes::StaticInit();

	SetCurrentInstance((CParaEngineApp*)this);

	srand((unsigned long)time(NULL));

	FindParaEngineDirectory();
	RegisterObjectClasses();

	DoTestCode();
}

// use RegisterObjectFactory instead
void ParaEngine::CParaEngineAppBase::RegisterObjectClass(IAttributeFields* pObject)
{
	if (pObject)
	{
		ref_ptr<IAttributeFields> obj(pObject);
		pObject->GetAttributeClass();
	}
}

void ParaEngine::CParaEngineAppBase::RegisterObjectClasses()
{
	CAttributesManager* pAttManager = CGlobals::GetAttributesManager();
	pAttManager->RegisterObjectFactory("CWeatherEffect", new CDefaultObjectFactory<CWeatherEffect>());
	pAttManager->RegisterObjectFactory("CMeshPhysicsObject", new CDefaultObjectFactory<CMeshPhysicsObject>());
	pAttManager->RegisterObjectFactory("CMeshObject", new CDefaultObjectFactory<CMeshObject>());
	pAttManager->RegisterObjectFactory("BlockPieceParticle", new CDefaultObjectFactory<CBlockPieceParticle>());
	pAttManager->RegisterObjectFactory("CContainerObject", new CDefaultObjectFactory<CContainerObject>());
	pAttManager->RegisterObjectFactory("CRenderTarget", new CDefaultObjectFactory<CRenderTarget>());
	pAttManager->RegisterObjectFactory("CMissileObject", new CDefaultObjectFactory<CMissileObject>());
	pAttManager->RegisterObjectFactory("BMaxObject", new CDefaultObjectFactory<BMaxObject>());
	pAttManager->RegisterObjectFactory("CSkyMesh", new CDefaultObjectFactory<CSkyMesh>());
	pAttManager->RegisterObjectFactory("COverlayObject", new CDefaultObjectFactory<COverlayObject>());
	pAttManager->RegisterObjectFactory("CLightObject", new CDefaultObjectFactory<CLightObject>());
	pAttManager->RegisterObjectFactory("CGeosetObject",new CDefaultObjectFactory<CGeosetObject>());
	pAttManager->RegisterObjectFactory("CAutoRigger", new CDefaultObjectFactory<CAutoRigger>());
	// TODO add more here: 
}

bool ParaEngine::CParaEngineAppBase::Is3DRenderingEnabled()
{
	return m_bEnable3DRendering;
}

ParaEngine::IAttributeFields* ParaEngine::CParaEngineAppBase::GetAttributeObject()
{
	return &ParaEngineSettings::GetSingleton();
}

void ParaEngine::CParaEngineAppBase::Enable3DRendering(bool val)
{
	m_bEnable3DRendering = val;
}

bool ParaEngine::CParaEngineAppBase::IsTouchInputting()
{
	return m_isTouching;
}

bool ParaEngine::CParaEngineAppBase::IsSlateMode()
{
	return false;
}

void ParaEngine::CParaEngineAppBase::SetTouchInputting(bool bTouchInputting)
{
	m_isTouching = bTouchInputting;
}

void ParaEngine::CParaEngineAppBase::GetCursorPosition(int* pX, int * pY, bool bInBackbuffer /*= true*/)
{
	CGUIRoot::GetInstance()->GetMouse()->GetDeviceCursorPos(*pX, *pY);
}

bool ParaEngine::CParaEngineAppBase::IsAppActive()
{
	return true;
}

DWORD ParaEngine::CParaEngineAppBase::GetCoreUsage()
{
	return PE_USAGE_STANDALONE | PE_USAGE_OWN_D3D | PE_USAGE_OWN_WINDOW;
}

void ParaEngine::CParaEngineAppBase::SystemMessageBox(const std::string& msg)
{
	OUTPUT_LOG("System Message: %s \n", msg.c_str());
#ifdef WIN32
	//::MessageBox(CGlobals::GetAppHWND(), msg.c_str(), "System Message", MB_OK);
#endif
}

void ParaEngine::CParaEngineAppBase::SetAppCommandLine(const char* pCommandLine)
{
	CCommandLineParams::SetAppCommandLine(pCommandLine);
	InitCommandLineParams();

	static bool bFirstCall = true;
	if (bFirstCall)
	{
		bFirstCall = false;
		OUTPUT_LOG1("NPL Runtime started\n");
		OUTPUT_LOG("NPL bin dir: %s\n", m_sModuleDir.c_str());
		if (m_sPackagesDir.empty()) {
			OUTPUT_LOG("no packages at: %s\n", m_sPackagesDir.c_str());
		}
		OUTPUT_LOG("WorkingDir: %s\n", m_sInitialWorkingDir.c_str());
		OUTPUT_LOG("WritablePath: %s\n", CParaFile::GetWritablePath().c_str());
	}
}

const char* ParaEngine::CParaEngineAppBase::GetAppCommandLine()
{
	return CCommandLineParams::GetAppCommandLine();
}

const char* ParaEngine::CParaEngineAppBase::GetAppCommandLineByParam(const char* pParam, const char* defaultValue)
{
	return CCommandLineParams::GetAppCommandLineByParam(pParam, defaultValue);
}

void ParaEngine::CParaEngineAppBase::Exit(int nReturnCode /*= 0*/)
{
	OUTPUT_LOG("program exited with code %d\n", nReturnCode);
	SetReturnCode(nReturnCode);
	SetAppState(PEAppState_Exiting);
}

ParaEngine::PEAppState ParaEngine::CParaEngineAppBase::GetAppState()
{
	return m_nAppState;
}

void ParaEngine::CParaEngineAppBase::SetAppState(ParaEngine::PEAppState state)
{
	m_nAppState = state;
}

HRESULT ParaEngine::CParaEngineAppBase::FinalCleanup()
{
	if (!CGlobals::GetAISim()->IsCleanedUp())
	{
		if (CGlobals::GetEventsCenter())
		{
			SystemEvent event(SystemEvent::SYS_WM_DESTROY, "");
			// set sync mode, so that event is processed immediately, since there is no next frame move. 
			event.SetAsyncMode(false);
			CGlobals::GetEventsCenter()->FireEvent(event);
		}
	}
	// AI simulator should be cleaned up prior to NPL runtime. 
	CGlobals::GetAISim()->CleanUp();
	CGlobals::GetNPLRuntime()->Cleanup();
	
	return S_OK;
}

NPL::INPLRuntime* ParaEngine::CParaEngineAppBase::GetNPLRuntime()
{
	return CGlobals::GetNPLRuntime();
}

CRefCounted* ParaEngine::CParaEngineAppBase::AddToSingletonReleasePool(CRefCounted* pObject)
{
	if (!m_pSingletonReleasePool)
		m_pSingletonReleasePool = new CObjectAutoReleasePool();
	m_pSingletonReleasePool->AddObject(pObject);
	return pObject;
}

void ParaEngine::CParaEngineAppBase::VerifyCommandLine(const char* sCommandLine, std::string &strCmd)
{
	if (sCommandLine)
		strCmd = sCommandLine;
	if (strCmd.find("bootstrapper") == string::npos)
	{
		auto nPos = string::npos;
		if ( (((nPos=strCmd.rfind(".npl")) != string::npos) || ((nPos = strCmd.rfind(".lua")) != string::npos)) )
		{
			// just in case, user has specified XXX.lua instead of bootstrapper=XXX.lua || .npl in the command line. 
			// following format are valid for specifying bootstrapper file. 
			// - [parameters] bootstrapper.lua
			// - bootstrapper.lua [parameters]
			auto nFilenameFromPos = strCmd.rfind(" ", nPos);
			if (nFilenameFromPos != string::npos)
			{
				strCmd = strCmd.substr(0, nFilenameFromPos + 1) + std::string("bootstrapper=") +
					strCmd.substr(nFilenameFromPos + 1, nPos - nFilenameFromPos + 3) + strCmd.substr(nPos + 4);
			}
			else
			{
				strCmd = std::string("bootstrapper=") + strCmd.substr(0, nPos + 4) + strCmd.substr(nPos + 4);
			}
		}
		else
		{
			// if no bootstrapper is specified, try to find the config.txt in current directory. 
			CParaFile configFile;
			if (configFile.OpenFile("config.txt"))
			{
				std::string sCmdLine;
				configFile.GetNextAttribute("cmdline", sCmdLine);
				configFile.close();
				if (sCmdLine.find("bootstrapper") != string::npos)
				{
					strCmd = strCmd + " " + sCmdLine;
				}
			}
		}
	}
}

bool ParaEngine::CParaEngineAppBase::HasClosingRequest()
{
	return m_hasClosingRequest;
}

void ParaEngine::CParaEngineAppBase::SetHasClosingRequest(bool val)
{
	m_hasClosingRequest = val;
}


bool ParaEngine::CParaEngineAppBase::LoadNPLPackage(const char* sFilePath_, std::string * pOutMainFile)
{
	std::string sFilePath = sFilePath_;
	std::string sPKGDir;
	bool bHasOutputMainFile = false;
	CNPLPackageConfig config;
	if (sFilePath[sFilePath.size() - 1] == '/')
	{
		std::string sDirName = sFilePath.substr(0, sFilePath.size() - 1);
		
		if (!CParaFile::GetDevDirectory().empty())
		{
			std::string sFullDir = CParaFile::GetAbsolutePath(sDirName, CParaFile::GetDevDirectory());
			if (CParaFile::DoesFileExist2(sFullDir.c_str(), FILE_ON_DISK))
			{
				sPKGDir = sFullDir;
			}
		}
		
		if (!sPKGDir.empty())
		{
			// found packages under dev folder
		}
		else 
		{
			std::string sFullDir;

			if ( CParaFile::IsAbsolutePath(sDirName) && CParaFile::DoesFileExist2(sDirName.c_str(), FILE_ON_DISK, &sFullDir))
			{
				sPKGDir = sFullDir;
			}
			else
			{
				sFullDir = CParaFile::GetAbsolutePath(sDirName, CParaFile::GetCurDirectory(0));
				if (CParaFile::DoesFileExist2(sFullDir.c_str(), FILE_ON_DISK))
				{
					sPKGDir = sFullDir;
				}

				if (sPKGDir.empty() && !m_sModuleDir.empty())
				{
					std::string workingDir = m_sModuleDir;
					// search for all parent directory for at most 5 levels
					for (int i = 0; i < 5 && !workingDir.empty(); ++i)
					{
						std::string sFullDir = CParaFile::GetAbsolutePath(sDirName, workingDir);
						if (CParaFile::DoesFileExist2(sFullDir.c_str(), FILE_ON_DISK))
						{
							sPKGDir = sFullDir;
							break;
						}
						else
						{
							workingDir = CParaFile::GetParentDirectoryFromPath(workingDir, 1);
						}
					}
				}
			}
		}
		if (sPKGDir.empty())
		{
			// if package folder is not found, we will search for zip and pkg file with the same name as the folder name.
			std::string pkgFile = sDirName + ".zip";
			CArchive* pArchive = CFileManager::GetInstance()->GetArchive(pkgFile);
			if (pArchive == 0)
			{
				if (CFileManager::GetInstance()->OpenArchive(pkgFile, false))
				{
					pArchive = CFileManager::GetInstance()->GetArchive(pkgFile);
				}
			}
			if (pArchive!=0)
			{
				// locate "package.npl" in root folder of zip file
				CParaFile file;
				if (file.OpenFile(pArchive, "package.npl"))
				{
					config.open(file.getBuffer(), file.getSize());
				}
				else
				{
					static CSearchResult result;
					result.InitSearch(sDirName + "/", 0, 1, 0);
					pArchive->FindFiles(result, "", "*/package.npl", 0);
					if (result.GetNumOfResult() > 0)
					{
						// "*/package" just in case we zipped data in a subfolder.
						CParaFile file;
						if (file.OpenFile(pArchive, result.GetItem(0).c_str()))
						{
							std::string sBaseDir = result.GetItem(0).substr(0, result.GetItem(0).size() - 12);
							pArchive->SetBaseDirectory(sBaseDir.c_str());
							config.open(file.getBuffer(), file.getSize());
						}
					}
				}
			}
			if (config.IsOpened())
			{
				sPKGDir = pkgFile;

				if (!config.IsSearchPath())
				{
					pArchive->SetRootDirectory(sFilePath);
					config.SetMainFile(CParaFile::GetAbsolutePath(config.GetMainFile(), sFilePath));
				}
				if (pOutMainFile)
					*pOutMainFile = config.GetMainFile();
			}
		}
	}
	if (!sPKGDir.empty())
	{
		if (!config.IsOpened())
		{
			// disk file based package
			std::string packageFile = sPKGDir + "/package.npl";
			CParaFile file;
			if (file.OpenFile(packageFile.c_str(), true, 0, false, FILE_ON_DISK))
			{
				config.open(file.getBuffer(), file.getSize());
				// output main file
				if (!config.IsSearchPath())
				{
					if (sFilePath.size() > 3 && sFilePath[0] == '.' && ((sFilePath[1] == '.' && sFilePath[2] == '/') || (sFilePath[1] == '/')))
					{
						// use absolute path if folder begins with ../ or ./
						config.SetMainFile(CParaFile::GetAbsolutePath(config.GetMainFile(), sPKGDir));
					}
					else
					{
						// relative to root path
						config.SetMainFile(CParaFile::GetAbsolutePath(config.GetMainFile(), sFilePath));
					}
				}
			}
			if (pOutMainFile)
				*pOutMainFile = config.GetMainFile();
			if (config.IsSearchPath())
				return CFileManager::GetInstance()->AddSearchPath(sPKGDir.c_str());
		}
		return true;
	}
	return false;
}

void CParaEngineAppBase::AutoSetLocale()
{
	// set locale according to current system language. 
	const char* lang = "enUS";
	if (ParaEngineSettings::GetSingleton().GetCurrentLanguage() == LanguageType::CHINESE)
		lang = "zhCN";
	ParaEngineSettings::GetSingleton().SetLocale(lang);
}


void CParaEngineAppBase::WriteToLog(const char* zFormat, ...)
{
	va_list args;
	va_start(args, zFormat);
	ParaEngine::CLogger::GetSingleton().WriteFormatedVarList(zFormat, args);
	va_end(args);
}

void CParaEngineAppBase::AppLog(const char* sFormat)
{
	OUTPUT_LOG1("%s\n", sFormat);
}

void CParaEngineAppBase::SetReturnCode(int nReturnCode)
{
	m_nReturnCode = nReturnCode;
}

int CParaEngineAppBase::GetReturnCode()
{
	return m_nReturnCode;
}


void ParaEngine::CParaEngineAppBase::LoadPackagesInFolder(const std::string& sPkgFolder)
{
	/** we will load all packages that matches the following pattern in the order given by their name,
	* such that "main_001.pkg" is always loaded before "main_002.pkg" */
#define MAIN_PACKAGE_FILE_PATTERN	"main*.pkg"

	CSearchResult* result = CFileManager::GetInstance()->SearchFiles(
		sPkgFolder,
		MAIN_PACKAGE_FILE_PATTERN, "", 0, 1000, 0);

	bool bIs64Bits = sizeof(void*) > 4;
	int nNum = 0;
	if (result != 0)
	{
		// we will sort by file name
		std::vector<std::string> fileList;
		nNum = result->GetNumOfResult();
		for (int i = 0; i < nNum; ++i)
		{
			fileList.push_back(result->GetItem(i));
		}
		// we will enqueue in reverse order, so that main_002 is pushed first, and then main_001
		std::sort(fileList.begin(), fileList.end(), std::greater<std::string>());

		if (fileList.size() == 0)
		{
			if (CParaFile::DoesFileExist("main.pkg") || CParaFile::DoesFileExist("main.zip"))
				fileList.push_back("main.zip");
			if (bIs64Bits && CParaFile::DoesFileExist("main_64bits.pkg"))
				fileList.push_back("main_64bits.pkg");
			else if (!bIs64Bits && CParaFile::DoesFileExist("main_32bits.pkg"))
				fileList.push_back("main_32bits.pkg");

			if (CParaFile::DoesFileExist("main_full.pkg"))
				fileList.push_back("main_full.pkg");
			if (bIs64Bits && CParaFile::DoesFileExist("main_full_64bits.pkg"))
				fileList.push_back("main_full_64bits.pkg");
			else if (!bIs64Bits && CParaFile::DoesFileExist("main_full_32bits.pkg"))
				fileList.push_back("main_full_32bits.pkg");
		}


		for (auto& filename : fileList)
		{
			int nSize = (int)filename.size();

			if (nSize > 11 && filename[nSize - 11] == '_' && filename[nSize - 8] == 'b')
			{
				if (filename[nSize - 10] == '3' && filename[nSize - 9] == '2' && bIs64Bits)
				{
					continue;
				}
				else if (filename[nSize - 10] == '6' && filename[nSize - 9] == '4' && !bIs64Bits)
				{
					continue;
				}
			}
			// always load by relative path first, and then by absolute path. 
			// For example, when there is a package in current working directory, it will be used instead the one in packages/ folder.
			if (!CFileManager::GetInstance()->OpenArchive(filename))
			{
				if (!CFileManager::GetInstance()->OpenArchive(sPkgFolder + filename))
				{
					OUTPUT_LOG("failed loading package: %s%s\n", sPkgFolder.c_str(), filename.c_str());
				}
			}
		}
	}
}

bool ParaEngine::CParaEngineAppBase::FindBootStrapper()
{
	const char* pBootFileName = GetAppCommandLineByParam("bootstrapper", "");
	if (pBootFileName[0] == '\0' && ! CBootStrapper::GetSingleton()->IsEmpty())
	{
		// may be the `loadpackage` params have set the bootstrapper if not explicitly specified by the application. 
		pBootFileName = CBootStrapper::GetSingleton()->GetMainLoopFile().c_str();
		OUTPUT_LOG("try bootstrapper in loadpackage command: %s\n", pBootFileName);
	}

	bool bHasBootstrapper = CBootStrapper::GetSingleton()->LoadFromFile(pBootFileName);
	if (!bHasBootstrapper)
	{
		if (pBootFileName && pBootFileName[0] != '\0'){
			OUTPUT_LOG("error: can not find bootstrapper file at %s\n", pBootFileName);
		}
		pBootFileName = NPL_CODE_WIKI_BOOTFILE;
		bHasBootstrapper = CBootStrapper::GetSingleton()->LoadFromFile(pBootFileName);
		OUTPUT_LOG("We are using default bootstrapper at %s\n", pBootFileName);
		if (!bHasBootstrapper)
		{
			OUTPUT_LOG("However, we can not locate that file either. Have you installed npl_package/main? \n\n\nPlease install it at https://github.com/NPLPackages/main\n\n\n");
		}
	}
	// OUTPUT_LOG("cmd line: %s \n", GetAppCommandLine());
	OUTPUT_LOG("main loop: %s \n", CBootStrapper::GetSingleton()->GetMainLoopFile().c_str());
	return true;
}

void CParaEngineAppBase::LoadPackages()
{
	std::string sRootDir = CParaFile::GetCurDirectory(0);
	OUTPUT_LOG("ParaEngine Root Dir is %s\n", sRootDir.c_str());
	// always load main package folder if exist
	std::string sOutputFile;
	if (LoadNPLPackage("npl_packages/main/", &sOutputFile))
	{
		if (!sOutputFile.empty()) {
			CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Loadfile_async(sOutputFile.c_str());
		}
	}
	
	// load packages via command line
	const char* sPackages = GetAppCommandLineByParam("loadpackage", NULL);
	if (sPackages)
	{
		std::vector<std::string> listPackages;
		StringHelper::split(sPackages, ",;", listPackages);
		for (const std::string& package : listPackages)
		{
			std::string sOutputFile;
			if (LoadNPLPackage(package.c_str(), &sOutputFile))
			{
				if (!sOutputFile.empty()) {
					CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Loadfile_async(sOutputFile.c_str());
				}
			}
		}
	}

	LoadPackagesInFolder(sRootDir);
	if (m_sPackagesDir.empty())
		m_sPackagesDir = sRootDir;
	OUTPUT_LOG("./packages dir: %s\n", m_sPackagesDir.c_str());
	if (m_sPackagesDir != sRootDir)
		LoadPackagesInFolder(m_sPackagesDir);
}


bool CParaEngineAppBase::FindParaEngineDirectory(const char* sHint)
{
	std::string sModuleDir = ParaEngine::GetExecutablePath();
	if (!sModuleDir.empty())
	{
		m_sModuleDir = CParaFile::GetParentDirectoryFromPath(sModuleDir);
		std::string packagesDir = m_sModuleDir + "packages";
		if (!CParaFile::DoesFileExist(packagesDir.c_str(), false))
		{
			packagesDir = CParaFile::GetParentDirectoryFromPath(m_sModuleDir, 1) + "packages";
			if (!CParaFile::DoesFileExist(packagesDir.c_str(), false))
			{
				packagesDir = "";
			}
		}
		if (!packagesDir.empty())
		{
			m_sPackagesDir = packagesDir + "\\";
		}
	}
#ifdef WIN32
	// ParaEngine.sig must be called first, to locate the root dir. 
	if (!CParaFile::DoesFileExist("ParaEngine.sig", false))
	{
		if (!sModuleDir.empty())
		{
			std::string workingDir = m_sModuleDir;
			std::string sigPath = workingDir + "ParaEngine.sig";
			bool bFoundSigFile = false;
			if (!CParaFile::DoesFileExist(sigPath.c_str(), false))
			{
				// search the parent directory of the module for signature file, if file exist, use it as current working directory. 
				sigPath = CParaFile::GetParentDirectoryFromPath(m_sModuleDir, 1) + "ParaEngine.sig";
				if (CParaFile::DoesFileExist(sigPath.c_str(), false))
				{
					workingDir = CParaFile::GetParentDirectoryFromPath(m_sModuleDir, 1);
					bFoundSigFile = true;
				}
			}
			else
				bFoundSigFile = true;
			if (bFoundSigFile)
			{
				::SetCurrentDirectory(workingDir.c_str());
			}
			else
			{
				// OUTPUT_LOG("ParaEngine.sig file not found\n");
			}
		}
		// set the current directory by reading from the registry.
		/*const char* sInstallDir = ReadRegStr("HKEY_CURRENT_USER", "SOFTWARE\\ParaEngine\\ParaWorld", "");
		if(sInstallDir)
		{
			::SetCurrentDirectory(sInstallDir);
		}*/
	}
	{
		char sWorkingDir[512 + 1] = { 0 };
		memset(sWorkingDir, 0, sizeof(sWorkingDir));
		::GetCurrentDirectory(MAX_PATH, sWorkingDir);
		m_sInitialWorkingDir = sWorkingDir;
#ifdef PARAENGINE_MOBILE
		CGlobals::GetFileManager()->AddDiskSearchPath(sWorkingDir);
#endif
	}
#endif
	return true;
}

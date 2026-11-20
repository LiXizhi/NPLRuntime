//-----------------------------------------------------------------------------
// Class:	ParaEngineSettings
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.2.22
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "SceneObject.h"
#include "EffectManager.h"
#include "ParaWorldAsset.h"
#include "ParaVertexBufferPool.h"
#include "2dengine/GUIRoot.h"
#include "PaintEngine/Painter.h"
#include "SceneObject.h"
#include "ViewportManager.h"
#include "ParaScriptingScene.h"
#include "AutoCamera.h"
#include "ParaEngineInfo.h"
#include "NPLHelper.h"
#include "AsyncLoader.h"
#include "TextureEntity.h"
#include "ParaEngineSettings.h"
#include "NPLRuntime.h"
#include "util/os_calls.h"
#include "SelectionManager.h"
#include "BufferPicking.h"
#include "FrameRateController.h"
#include "AudioEngine2.h"
#include "StringHelper.h"
#include "WorldInfo.h"

#include "util/CommonFileDialog.h"
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
#include "util/EnumProcess.hpp"
#endif

#if !defined(WIN32) && !defined(__EMSCRIPTEN__)
#include <sys/utsname.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <dirent.h>
#endif
#endif

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <thread>
#include <boost/thread/tss.hpp>
#endif
#include <boost/log/attributes/current_process_name.hpp>

#include <time.h>
#include "Globals.h"
#include "IParaEngineApp.h"
//#include "OSWindows.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "ParaAntlr4.h"

using namespace ParaEngine;
using namespace luabind;

namespace ParaEngine {
	extern CFrameRateController g_gameTime;
}

/** the default locale in this compilation. */
#define DEFAULT_LOCALE	"zhCN"
//#define DEFAULT_LOCALE	"enUS"


#ifdef WIN32
bool ParaEngine::ParaEngineSettings::m_bSandboxMode = true;
#else
bool ParaEngine::ParaEngineSettings::m_bSandboxMode = false;
#endif

ParaEngineSettings::ParaEngineSettings(void)
	:m_currentLanguage(-1)
{
	LoadNameIndex();
	// default value for a variety of settings
	m_ctorHeight = 0.2f; // in meters
	m_ctorColor = LinearColor(1.0f,1.0f,1.0f,1.f);
	m_ctorSpeed = 1.0f;// 0.1f; // unit per second.
	m_sScriptEditorPath = "editor\\notepad.exe";
	m_selectionColor = LinearColor(1.0f,1.0f,1.0f,0.5f);
	m_bEditingMode = false;
	m_sLocale = DEFAULT_LOCALE;
	m_bInverseMouse = false;
}

ParaEngineSettings::~ParaEngineSettings(void)
{
}

void ParaEngineSettings::SetOceanTechnique(DWORD dwTechnique)
{
	if (CGlobals::GetOceanManager())
		CGlobals::GetOceanManager()->SetRenderTechnique(dwTechnique);
}

ParaEngineSettings& ParaEngineSettings::GetSingleton()
{
	static ParaEngineSettings g_singleton;
	return g_singleton;
}

LinearColor ParaEngineSettings::GetSelectionColor(int nGroupID)
{
	return m_selectionColor;
}
void ParaEngineSettings::SetSelectionColor(const LinearColor& color,int nGroupID)
{
	m_selectionColor = color;
}

LinearColor ParaEngineSettings::GetCtorColor()
{
	return m_ctorColor;
}

void ParaEngineSettings::SetCtorColor(const LinearColor& color)
{
	m_ctorColor = color;
}

float  ParaEngineSettings::GetCtorHeight()
{
	return m_ctorHeight;
}

void ParaEngineSettings::SetCtorHeight(float fHeight)
{
	m_ctorHeight = fHeight;
}


float  ParaEngineSettings::GetCtorSpeed()
{
	return m_ctorSpeed;
}

string ParaEngineSettings::GetStats(DWORD dwFields)
{
	string output;
	CGlobals::GetApp()->GetStats(output, dwFields);
	return output;
}

void ParaEngineSettings::SetCtorSpeed(float fSpeed)
{
	m_ctorSpeed  = fSpeed;
}

int ParaEngineSettings::GetVertexShaderVersion()
{
	auto* pDevice = CGlobals::GetRenderDevice();
	return pDevice ? pDevice->GetCaps().GetShaderModelMajor() : 0;
}

int ParaEngineSettings::GetPixelShaderVersion()
{
	auto* pDevice = CGlobals::GetRenderDevice();
	return pDevice ? pDevice->GetCaps().GetShaderModelMajor() : 0;
}

const std::string& ParaEngineSettings::GetDispalyMode()
{
#ifdef WIN32
	DEVMODEW dm = {};
	dm.dmSize = sizeof(dm);
	std::stringstream ss;
	std::string lastEntry;
	int modeCount = 0;
	for (DWORD iModeNum = 0; EnumDisplaySettingsW(NULL, iModeNum, &dm) != 0; ++iModeNum)
	{
		if (dm.dmBitsPerPel < 32 || dm.dmPelsWidth == 0 || dm.dmPelsHeight == 0)
			continue;
		std::ostringstream item;
		item << dm.dmPelsWidth << " " << dm.dmPelsHeight << " " << dm.dmDisplayFrequency;
		std::string cur = item.str();
		if (cur == lastEntry)
			continue;
		if (modeCount > 0)
			ss << ",";
		ss << cur;
		lastEntry = cur;
		++modeCount;
	}
	if (modeCount < 1)
		return CGlobals::GetString(0);
	m_displayModes = ss.str();
	return m_displayModes;
#else
	Vector2 res(0, 0);
	if (CGlobals::GetApp())
		CGlobals::GetApp()->GetScreenResolution(&res);
	if (res.x <= 0 || res.y <= 0)
		return CGlobals::GetString(0);
	std::ostringstream ss;
	ss << (int)res.x << " " << (int)res.y << " 60";
	m_displayModes = ss.str();
	return m_displayModes;
#endif
}

Vector2 ParaEngineSettings::GetMonitorResolution()
{
#ifdef WIN32
	return Vector2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
#else
	Vector2 res(0, 0);
	if (CGlobals::GetApp())
		CGlobals::GetApp()->GetScreenResolution(&res);
	return res;
#endif
}

int g_nSetID = 0;
int ParaEngineSettings::GetGameEffectSet()
{
	return g_nSetID;
}

bool ParaEngineSettings::IsLowEndRenderDevice()
{
	if (g_nSetID == 1024 || g_nSetID < 0)
		return true;
	auto* pDevice = CGlobals::GetRenderDevice();
	if (!pDevice)
		return false;
	return pDevice->GetCaps().IsLowEndDevice();
}

void ParaEngineSettings::LoadGameEffectSet(int nSetID)
{
	auto* pDevice = CGlobals::GetRenderDevice();
	if (pDevice && pDevice->GetCaps().backend == ERHIBackend::Null)
	{
		g_nSetID = nSetID;
		return;
	}

	int ShaderVersion = pDevice ? pDevice->GetCaps().GetShaderModelMajor() : 0;
	const bool bLowEndHw = pDevice ? pDevice->GetCaps().IsLowEndDevice() : false;
	if (ShaderVersion != 0 && nSetID != 1024)
	{
		if (ShaderVersion < 2)
		{
			OUTPUT_LOG("warning: you can not set effect level to %d, because your shader version is too low. Fixed function is used instead. \n", nSetID);
			nSetID = 1024;
		}
		else if (ShaderVersion < 3 || bLowEndHw)
		{
			if (nSetID > 0)
			{
				OUTPUT_LOG("warning: effect level %d clamped to 0 for low-end / DirectX 9-class device.\n", nSetID);
				nSetID = 0;
			}
		}
	}

	g_nSetID = nSetID;
	switch (nSetID)
	{
	case 1024:
		CGlobals::GetEffectManager()->SetDefaultEffectMapping(0);
		CGlobals::GetScene()->EnableFullScreenGlow(false);
		ParaScripting::ParaScene::EnableLighting(false);
		SetTextureLOD(1);
		break;
	case -1:
		{
			CGlobals::GetEffectManager()->SetDefaultEffectMapping(20);
			CGlobals::GetScene()->EnableFullScreenGlow(false);
			ParaScripting::ParaScene::EnableLighting(false);
			ParaScripting::ParaScene::SetShadowMethod(0);

			COceanManager* pOcean = CGlobals::GetOceanManager();
			pOcean->EnableReflectionItem(COceanManager::R_SKY_BOX, true);
			pOcean->EnableReflectionItem(COceanManager::R_GLOBAL_TERRAIN, false);
			pOcean->EnableReflectionItem(COceanManager::R_CURRENT_PLAYER, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_FRONT_TO_BACK, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_TRANSPARENT, false);
			pOcean->EnableReflectionItem(COceanManager::R_CHARACTERS, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_BACK_TO_FRONT, false);
			SetTextureLOD(1);
			break;
		}
	case 0:
		{
			CGlobals::GetEffectManager()->SetDefaultEffectMapping(20);
			ParaScripting::ParaScene::EnableLighting(true);
			ParaScripting::ParaScene::SetShadowMethod(0);
			CGlobals::GetScene()->EnableFullScreenGlow(false);
			COceanManager* pOcean = CGlobals::GetOceanManager();
			pOcean->EnableReflectionItem(COceanManager::R_SKY_BOX, true);
			pOcean->EnableReflectionItem(COceanManager::R_GLOBAL_TERRAIN, false);
			pOcean->EnableReflectionItem(COceanManager::R_CURRENT_PLAYER, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_FRONT_TO_BACK, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_TRANSPARENT, false);
			pOcean->EnableReflectionItem(COceanManager::R_CHARACTERS, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_BACK_TO_FRONT, false);
			break;
		}
	case 1:
		{
			CGlobals::GetEffectManager()->SetDefaultEffectMapping(30);
			ParaScripting::ParaScene::EnableLighting(true);
			ParaScripting::ParaScene::SetShadowMethod(1);
			CGlobals::GetScene()->EnableFullScreenGlow(false);
			COceanManager* pOcean = CGlobals::GetOceanManager();
			pOcean->EnableReflectionItem(COceanManager::R_SKY_BOX, true);
			pOcean->EnableReflectionItem(COceanManager::R_GLOBAL_TERRAIN, true);
			pOcean->EnableReflectionItem(COceanManager::R_CURRENT_PLAYER, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_FRONT_TO_BACK, true);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_TRANSPARENT, true);
			pOcean->EnableReflectionItem(COceanManager::R_CHARACTERS, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_BACK_TO_FRONT, false);
			break;
		}
	case 2:
		{
			CGlobals::GetEffectManager()->SetDefaultEffectMapping(30);
			ParaScripting::ParaScene::EnableLighting(true);
			ParaScripting::ParaScene::SetShadowMethod(1);
			COceanManager* pOcean = CGlobals::GetOceanManager();
			pOcean->EnableReflectionItem(COceanManager::R_SKY_BOX, true);
			pOcean->EnableReflectionItem(COceanManager::R_GLOBAL_TERRAIN, true);
			pOcean->EnableReflectionItem(COceanManager::R_CURRENT_PLAYER, true);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_FRONT_TO_BACK, true);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_TRANSPARENT, true);
			pOcean->EnableReflectionItem(COceanManager::R_CHARACTERS, false);
			pOcean->EnableReflectionItem(COceanManager::R_MESH_BACK_TO_FRONT, false);
			break;
		}
	default:
		CGlobals::GetEffectManager()->SetDefaultEffectMapping(30);
		break;
	}
}

void ParaEngineSettings::SetScriptEditor(const string& sEditorFilePath)
{
	m_sScriptEditorPath = sEditorFilePath;
}

const string& ParaEngineSettings::GetScriptEditor()
{
	return m_sScriptEditorPath;
}

void ParaEngineSettings::GetClientSize(int* width, int* height)
{
	auto* pDevice = CGlobals::GetRenderDevice();
	if (width)
		*width = pDevice ? static_cast<int>(pDevice->GetBackBufferWidth()) : 0;
	if (height)
		*height = pDevice ? static_cast<int>(pDevice->GetBackBufferHeight()) : 0;
}
bool ParaEngineSettings::IsDebugging()
{
#if defined(_DEBUG) && !defined(PARAENGINE_MOBILE)
	return true;
#else
	return false;
#endif
}

bool ParaEngineSettings::IsTouchInputting()
{
	return CGlobals::GetApp()->IsTouchInputting();
}

bool ParaEngineSettings::IsSlateMode()
{
	return CGlobals::GetApp()->IsSlateMode();
}

bool ParaEngineSettings::IsEditing()
{
	return GetSingleton().m_bEditingMode;
}

void ParaEngineSettings::SetEditingMode(bool bEnable)
{
	GetSingleton().m_bEditingMode = bEnable;
}

const char* ParaEngineSettings::GetLocale()
{
	return m_sLocale.c_str();
}

void ParaEngineSettings::SetLocale( const char* sLocale )
{
	if(sLocale==NULL)
		return;
	else
	{
		// TODO: I may need to verify it. 
		/*
		"frFR": French
		"deDE": German
		"enUS": American English
		"enGB": British English
		"koKR": Korean
		"zhCN": Chinese (simplified)
		"zhTW": Chinese (traditional)
		"ruRU": Russian (UI AddOn)
		"esES": Spanish
		*/
		m_sLocale = sLocale;
	}
}

/* maximum length of the product activation key*/
#define MAX_ACTIVATION_KEY_LENGTH	16
DWORD GetCurrentSearchKey();
int CharToInt(char s);
char IntToChar(int n);

/** a is 32 bits, k is 8 bits. 
* by symetric, I mean SYMETRIC_ENCODE_32_BY_8(SYMETRIC_ENCODE_32_BY_8(a,k),k) = a */
#define SYMETRIC_ENCODE_32_BY_8(a,k)		(((a^(k<<24))&0xff000000) + ((a^(k<<16))&0x00ff0000) + ((a^(k<<8))&0x0000ff00)+ ((a^(k))&0x000000ff))
/**
* simple encode a key 
* @param nKey1	32 bits are used
* @param nKey2  24 bits are used
* @param nKey3  8 bits are used
* @param Format: if this is 0, the output will be "%x-%x-%x-%x", where %x is 4 letter hex number, if this is 1, it is "%x%x%x%x"
* @return encode key is returned, which is an array of MAX_ACTIVATION_KEY_LENGTH characters. 
*/
const char* EncodeKeys(DWORD nKey1, int nKey2, int nKey3,int Format=0)
{
	nKey2 = 0xffffff & nKey2; // only used 24 bit
	nKey3 = 0xff & nKey3; // only use 8 bits
	static char g_key[256];
	
	// TODO: 
	DWORD part1 = SYMETRIC_ENCODE_32_BY_8(nKey1, nKey3);
	DWORD part2 = SYMETRIC_ENCODE_32_BY_8(nKey2, nKey3)<<8;
	part2 += nKey3;
	
	if(Format == 0)
	{
		snprintf(g_key, 256, "%04x-%04x-%04x-%04x", (unsigned short)(part1>>16), (unsigned short)(part1&0xffff), (unsigned short)(part2>>16), (unsigned short)(part2&0xffff));
	}
	else
		snprintf(g_key, 256, "%04x%04x%04x%04x", (unsigned short)(part1>>16), (unsigned short)(part1&0xffff), (unsigned short)(part2>>16), (unsigned short)(part2&0xffff));
	g_key[255] = '\0';
	return g_key;
}

/**
* decode key into three parts
* @param key the key must be in the format "%x-%x-%x-%x"where %x is 4 letter hex number
* @param nKey1 
* @param nKey2 
* @param nKey3 
* @return 
*/
bool DecodeKeys(const char* key, DWORD& nKey1, int& nKey2, int& nKey3)
{
	DWORD parts[4];
	sscanf(key, "%x-%x-%x-%x", &(parts[0]), &(parts[1]), &(parts[2]), &(parts[3]));
	
	nKey3 = 0xff & parts[3];
	nKey1 = (parts[0]<<16)+parts[1];
	nKey1 = SYMETRIC_ENCODE_32_BY_8(nKey1, nKey3);
	nKey2 = (parts[2]<<8)+(parts[3]>>8);
	nKey2 = (SYMETRIC_ENCODE_32_BY_8(nKey2, nKey3) & 0x00ffffff);
	return true;
}

/* int is 0-63 */
int CharToBase64(char s)
{
	int n=0;
	if(s>='0' && s<='9')
		n=s-'0';
	else if(s>='a' && s<='z')
		n=10+s-'a';
	else if(s>='A' && s<='Z')
		n=36+s-'A';
	else if(s=='.')
		n=63;
	return n;
}

char Base64ToChar(int n)
{
	char c;
	if(n<10)
		c = '0'+n;
	else if(n<36)	
		c = 'a'+n;
	else if(n<62)	
		c = 'A'+n;
	else
		c= '.';
	return c;
}


DWORD GetCurrentSearchKey()
{
	DWORD key = 0;
	// encode string
	const char* sSearchKey ="LXZ";
	for (int i=0;i<3 && sSearchKey[0]!='\0';++i)
	{
		key = (key<<6)+CharToBase64(sSearchKey[i]);
	}
	// encode ParaEngine and Product version 
	key = (key<<3)+ParaEngineInfo::CParaEngineInfo::GetParaEngineMajorVersion();
	key = (key<<4)+ParaEngineInfo::CParaEngineInfo::GetParaEngineMinorVersion();
	key = (key<<3)+ParaEngineInfo::CParaEngineInfo::GetProductMajorVersion();
	key = (key<<4)+ParaEngineInfo::CParaEngineInfo::GetProductMinorVersion();
	return key;
}

bool g_IsProductActivated = false;
bool ParaEngineSettings::IsProductActivated()
{
	return g_IsProductActivated;
}

bool ParaEngineSettings::ActivateProduct( const char* sActivationCode )
{
	DWORD nSearchKey=0;
	int nKey2 = 0, nKey3 = 0;
	if(DecodeKeys(sActivationCode, nSearchKey, nKey2, nKey3))
	{
		g_IsProductActivated = (GetCurrentSearchKey() == nSearchKey);
	}
	if(g_IsProductActivated)
	{
		// write registration information to file or registry.
		if(!CParaFile::DoesFileExist(((CParaFile::GetCurDirectory(CParaFile::APP_CONFIG_DIR)+"license.txt").c_str())))
		{
			CParaFile file;
			if(file.CreateNewFile(((CParaFile::GetCurDirectory(CParaFile::APP_CONFIG_DIR)+"license.txt").c_str())))
			{
				file.WriteString(sActivationCode);
			}
		}
	}
	return g_IsProductActivated;
}

const char* ParaEngineSettings::GetProductKey(const char* sProductName)
{
	static string g_key;
	CParaFile file((CParaFile::GetCurDirectory(CParaFile::APP_CONFIG_DIR)+"license.txt").c_str());
	if(!file.isEof())
	{
		char buf[255+1];
		memset(buf,0, sizeof(buf));
		// register the fist line of file
		if(file.GetNextLine(buf, 255)>0)
		{
			g_key = buf;
		}
	}
	else
		g_key = "";

	return g_key.c_str();
}

bool ParaEngineSettings::SetWindowedMode(bool bWindowed)
{
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->SetWindowedMode(bWindowed);
	else
		return false;
}

bool ParaEngineSettings::IsWindowedMode()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->IsWindowedMode();
	else
		return false;
}

void ParaEngineSettings::SetWindowMaximized(bool isMaximized)
{
	if (CGlobals::GetApp())
		CGlobals::GetApp()->SetWindowMaximized(isMaximized);
}

bool ParaEngineSettings::IsWindowMaximized()
{
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->IsWindowMaximized();
	else
		return false;
}

Vector2 ParaEngineSettings::GetVisibleSize()
{
	Vector2 res(1024, 768);
	if (CGlobals::GetApp()) {
		CGlobals::GetApp()->GetVisibleSize(&res);
	}
	return res;
}

void ParaEngineSettings::SetAllowWindowClosing(bool bWindowed)
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetAllowWindowClosing(bWindowed);
}

bool ParaEngineSettings::IsWindowClosingAllowed()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->IsWindowClosingAllowed();
	else
		return false;
}

void ParaEngineSettings::SetWindowText( const char* pChar )
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->SetWindowText(pChar);
}

const char* ParaEngineSettings::GetWindowText()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->GetWindowText();
	else
		return CGlobals::GetString(0).c_str();
}

void ParaEngineSettings::WriteConfigFile(const char* sFileName)
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->WriteConfigFile(sFileName);
}

bool ParaEngineSettings::GenerateActivationCodes( const char* sOutputFileName, int nFrom, int nTo )
{
#ifdef _DEBUG
	if(nFrom<0 || nFrom>nTo)
	{
		OUTPUT_LOG("invalid range\r\n");
		return false;
	}
	/*
	* generate product activation codes in large numbers in to a given file. 
	* this function is only available in debug build. 
	* @param sOutputFileName the text file that store the generated keys: each key is on a separate line
	* @param nFrom copy number
	* @param nTo copy number
	* @return 
	*/
	string sFileName;
	if (sOutputFileName!=NULL && sOutputFileName[0] !='\0')
	{
		sFileName = sOutputFileName;
	}
	else
	{
		/*  if this is NULL, a automatically generated file name will be used, which is of the following format.
		*   temp/ParaEngine Licenses 1.0.1.0_enUS_1-2000.txt, where v1.0.0 is the software version number, enUS is the language, 0-2000 is nFrom-nTo
		*	This file is usually submitted to shareware website so that they can sell your software. 
		*/
		char buf[256];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, 256,"temp//ParaEngine Licenses %s_%s_%d-%d.txt", ParaEngineInfo::CParaEngineInfo::GetVersion().c_str(), GetLocaleS(), nFrom, nTo);
		sFileName = buf;
	}

	CParaFile file;
	if( !file.CreateNewFile(sFileName.c_str()))
	{
		OUTPUT_LOG("unable to create file %s\r\n", sFileName.c_str());
		return false;
	}

	DWORD nSearchKey = GetCurrentSearchKey();

	srand((unsigned long)time(NULL));
	for (int i=nFrom; i<=nTo;++i)
	{
		const char* sKey = EncodeKeys(nSearchKey, i, rand()&0xff);
		
		// test each generated key
		DWORD nTestKey=0;
		int nKey2 = 0, nKey3 = 0;
		if(DecodeKeys(sKey, nTestKey, nKey2, nKey3))
		{
			if(nTestKey!= nSearchKey)
			{
				file.WriteFormated(" invalid key: %s", sKey);
			}
		}
		file.WriteFormated("%s\r\n", sKey);
	}
	OUTPUT_LOG("license keys file generated at %s\r\n", sFileName.c_str());
	return true;
#else
	OUTPUT_LOG("GenerateActivationCodes() can not be called in release mode. \r\n");
	return false;
#endif
}
void ParaEngineSettings::SetMouseInverse( bool bInverse )
{
	m_bInverseMouse = bInverse;
	if(CGlobals::GetScene() && CGlobals::GetScene()->GetCurrentCamera())
	{
		CGlobals::GetScene()->GetCurrentCamera()->SetInvertPitch(bInverse);
	}
}

bool ParaEngineSettings::GetMouseInverse()
{
	return m_bInverseMouse;
}

void ParaEngine::ParaEngineSettings::EnablePassiveRendering( bool bEnable )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->EnablePassiveRendering(bEnable);
}

bool ParaEngine::ParaEngineSettings::IsPassiveRenderingEnabled()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->IsPassiveRenderingEnabled();
	else
		return false;
}

void ParaEngineSettings::Enable3DRendering(bool bEnable)
{
	// OUTPUT_LOG("Enable3DRendering: %s\n", bEnable ? "true": "false");
	CGlobals::GetApp()->Enable3DRendering(bEnable);
}

bool ParaEngineSettings::Is3DRenderingEnabled()
{
	return CGlobals::GetApp()->Is3DRenderingEnabled();
}

const char* ParaEngine::ParaEngineSettings::GetAppCommandLine()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->GetAppCommandLine();
	else
		return NULL;
}

void ParaEngine::ParaEngineSettings::SetAppCommandLine( const char* pCommandLine )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetAppCommandLine(pCommandLine);
}

const char* ParaEngine::ParaEngineSettings::GetAppCommandLineByParam( const char* pParam , const object& defaultValue)
{
	if(CGlobals::GetApp())
	{
		return CGlobals::GetApp()->GetAppCommandLineByParam(pParam, NPL::NPLHelper::LuaObjectToString(defaultValue));
	}
	return NULL;
}

Vector2 ParaEngine::ParaEngineSettings::GetScreenResolution()
{
	Vector2 res(1024, 768);
	if (CGlobals::GetApp()) {
		CGlobals::GetApp()->GetScreenResolution(&res);
	}
	return res;
}

void ParaEngine::ParaEngineSettings::SetScreenResolution( const Vector2& vSize )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetScreenResolution(vSize);
}

void ParaEngine::ParaEngineSettings::SetLockWindowSize(bool bEnabled)
{
	auto app = CGlobals::GetApp();
	if (app)
	{
		app->FixWindowSize(bEnabled);
	}
}

const char* ParaEngine::ParaEngineSettings::GetWritablePath()
{
	return CParaFile::GetWritablePath().c_str();
}

void ParaEngine::ParaEngineSettings::SetWritablePath(const char* sPath)
{
	if(sPath!=NULL)
		CParaFile::SetWritablePath(sPath);
}

void ParaEngine::ParaEngineSettings::SetFullScreenMode( bool bFullscreen )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetFullScreenMode(bFullscreen);
}

bool ParaEngine::ParaEngineSettings::IsFullScreenMode()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->IsFullScreenMode();
	else
		return false;
}

void ParaEngine::ParaEngineSettings::BringWindowToTop()
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->BringWindowToTop();
}

void ParaEngine::ParaEngineSettings::UpdateScreenMode()
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->UpdateScreenMode();
}

void ParaEngine::ParaEngineSettings::ShowMenu( bool bShow )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->ShowMenu(bShow);
}

int ParaEngine::ParaEngineSettings::GetMultiSampleType()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->GetMultiSampleType();
	else
		return 0;
}

void ParaEngine::ParaEngineSettings::SetMultiSampleType( int nType )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetMultiSampleType(nType);
}

int ParaEngine::ParaEngineSettings::GetMultiSampleQuality()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->GetMultiSampleQuality();
	else
		return 0;
}

void ParaEngine::ParaEngineSettings::SetMultiSampleQuality( int nType )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetMultiSampleQuality(nType);
}



void ParaEngine::ParaEngineSettings::SetIgnoreWindowSizeChange( bool bIgnoreSizeChange )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetIgnoreWindowSizeChange(bIgnoreSizeChange);
}

bool ParaEngine::ParaEngineSettings::GetIgnoreWindowSizeChange()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->GetIgnoreWindowSizeChange();
	else
		return false;
}

void ParaEngine::ParaEngineSettings::SetTextureLOD( int nLOD )
{
	TextureEntity::g_nTextureLOD = nLOD;
}

int ParaEngine::ParaEngineSettings::GetTextureLOD()
{
	return TextureEntity::g_nTextureLOD;
}


bool ParaEngine::ParaEngineSettings::HasNewConfig()
{
	if(CGlobals::GetApp())
		return CGlobals::GetApp()->HasNewConfig();
	return false;
}

void ParaEngine::ParaEngineSettings::SetHasNewConfig( bool bHasNewConfig )
{
	if(CGlobals::GetApp())
		CGlobals::GetApp()->SetHasNewConfig(bHasNewConfig);
}

bool ParaEngine::ParaEngineSettings::IsServerMode()
{
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->IsServerMode();
	else
		return true;
}

void ParaEngine::ParaEngineSettings::EnableProfiling(bool bEnable)
{
	CProfiler::EnableProfiling_S(bEnable);
}

bool ParaEngine::ParaEngineSettings::IsProfilingEnabled()
{
	return CProfiler::IsProfilingEnabled_S();
}

int ParaEngine::ParaEngineSettings::GetAsyncLoaderItemsLeft(int nItemType)
{
	return CAsyncLoader::GetSingleton().GetItemsLeft(nItemType);
}


int ParaEngine::ParaEngineSettings::GetAsyncLoaderBytesReceived( int nItemType )
{
	return CAsyncLoader::GetSingleton().GetBytesProcessed(nItemType);
}


int ParaEngine::ParaEngineSettings::GetAsyncLoaderRemainingBytes()
{
	return CAsyncLoader::GetSingleton().GetEstimatedSizeInBytes();
}

#ifdef WIN32
#include "GuiConsole.h"
#endif
void ParaEngine::ParaEngineSettings::AllocConsole()
{
#ifdef WIN32
	ParaEngine::RedirectIOToConsole();
#endif
}
void ParaEngine::ParaEngineSettings::FlushDiskIO()
{
#ifdef __EMSCRIPTEN__
	EM_ASM(FS.syncfs(false, function(err) { if (err) { console.log("FS.syncfs", err); } }););
#endif
}

#ifdef __EMSCRIPTEN__
EM_JS(void, SendMsgToJS_JS_, (const char* c_msg_data_json), {
    var msg_data_json = UTF8ToString(c_msg_data_json); 
    if (typeof RecvMsgFromEmscripten == "function")
    {
        RecvMsgFromEmscripten(msg_data_json);
    }
})
#endif

void ParaEngine::ParaEngineSettings::SendMsgToJS(const char* msg_json_data)
{
#ifdef __EMSCRIPTEN__
	SendMsgToJS_JS_(msg_json_data);
#endif
}

// std::string ParaEngine::ParaEngineSettings::PyToLua(std::string pycode)
// {
// #ifdef __EMSCRIPTEN__
//     emscripten::val js_global_LuaCallPy2Lua = emscripten::val::global("LuaCallPy2Lua");
// 	return js_global_LuaCallPy2Lua(pycode);
// #endif
// }

void ParaEngine::ParaEngineSettings::SetConsoleTextAttribute( int wAttributes )
{
#ifdef WIN32
	ParaEngine::SetOutputConsoleTextAttribute((WORD)wAttributes);
#endif
}

DWORD ParaEngine::ParaEngineSettings::GetCoreUsage()
{
	return CGlobals::GetApp()->GetCoreUsage();
}

void ParaEngine::ParaEngineSettings::SetCoreUsage( DWORD dwUsage )
{
	CGlobals::GetApp()->SetCoreUsage(dwUsage);
}

const std::string& ParaEngine::ParaEngineSettings::GetSystemInfoString( bool bRefresh /*= false*/ )
{
	static std::string s_sysInfo;
	if (!bRefresh && !s_sysInfo.empty())
		return s_sysInfo;

#ifdef WIN32
	std::ostringstream ss;
	DWORD dwMajor = 0, dwMinor = 0, dwBuild = 0;
	HMODULE hNtdll = ::GetModuleHandleW(L"ntdll.dll");
	if (hNtdll)
	{
		typedef LONG(WINAPI* PRtlGetVersion)(PRTL_OSVERSIONINFOW);
		auto pRtlGetVersion = reinterpret_cast<PRtlGetVersion>(::GetProcAddress(hNtdll, "RtlGetVersion"));
		if (pRtlGetVersion)
		{
			RTL_OSVERSIONINFOW osvi = {};
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			if (pRtlGetVersion(&osvi) == 0)
			{
				dwMajor = osvi.dwMajorVersion;
				dwMinor = osvi.dwMinorVersion;
				dwBuild = osvi.dwBuildNumber;
			}
		}
	}
	ss << "OS version:" << dwMajor << "." << dwMinor << " (Build " << dwBuild << ")\n";

	HKEY hCpuKey = nullptr;
	if (::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hCpuKey) == ERROR_SUCCESS)
	{
		WCHAR wCpuName[256] = { 0 };
		DWORD cbData = sizeof(wCpuName) - sizeof(WCHAR);
		DWORD dwType = 0;
		if (::RegQueryValueExW(hCpuKey, L"ProcessorNameString", nullptr, &dwType, reinterpret_cast<LPBYTE>(wCpuName), &cbData) == ERROR_SUCCESS && dwType == REG_SZ)
		{
			const char* pUtf8Cpu = StringHelper::WideCharToMultiByte(wCpuName, CP_UTF8);
			if (pUtf8Cpu && pUtf8Cpu[0] != '\0')
				ss << "Processor:" << pUtf8Cpu << "\n";
		}
		DWORD dwMHz = 0;
		cbData = sizeof(dwMHz);
		if (::RegQueryValueExW(hCpuKey, L"~MHz", nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwMHz), &cbData) == ERROR_SUCCESS && dwType == REG_DWORD)
		{
			ss << "Processor speed:" << dwMHz << " MHz\n";
		}
		::RegCloseKey(hCpuKey);
	}

	SYSTEM_INFO sysInfo = {};
	::GetNativeSystemInfo(&sysInfo);
	ss << "Number of processors:" << sysInfo.dwNumberOfProcessors << "\n";
	ss << "Processor type:" << sysInfo.dwProcessorType << "\n";

	MEMORYSTATUSEX memStatus = {};
	memStatus.dwLength = sizeof(memStatus);
	if (::GlobalMemoryStatusEx(&memStatus))
	{
		// Keep legacy COSInfo::GetSystemInfoString format (bare integer KB) expected by os.lua / SystemSettingsPage.lua
		ss << "TotalPhysicalMemory:" << (memStatus.ullTotalPhys / 1024ULL) << "\n";
		ss << "FreePhysicalMemory:" << (memStatus.ullAvailPhys / 1024ULL) << "\n";
	}

	s_sysInfo = ss.str();
	return s_sysInfo;
#elif defined(__EMSCRIPTEN__)
	std::ostringstream ss;
	ss << "OS version:WebAssembly (Emscripten)\n";
#ifndef EMSCRIPTEN_SINGLE_THREAD
	unsigned int nCores = std::thread::hardware_concurrency();
	if (nCores > 0)
		ss << "Number of processors:" << nCores << "\n";
#endif
	s_sysInfo = ss.str();
	return s_sysInfo;
#else
	std::ostringstream ss;
	struct utsname uts = {};
	if (uname(&uts) == 0)
	{
		ss << "OS version:" << uts.sysname << " " << uts.release << " (" << uts.machine << ")\n";
	}
#if defined(__APPLE__)
	char cpuBrand[256] = { 0 };
	size_t cpuBrandLen = sizeof(cpuBrand) - 1;
	if (sysctlbyname("machdep.cpu.brand_string", cpuBrand, &cpuBrandLen, nullptr, 0) == 0 && cpuBrand[0] != '\0')
	{
		ss << "Processor:" << cpuBrand << "\n";
	}
	uint64_t memBytes = 0;
	size_t memLen = sizeof(memBytes);
	if (sysctlbyname("hw.memsize", &memBytes, &memLen, nullptr, 0) == 0 && memBytes > 0)
	{
		ss << "TotalPhysicalMemory:" << (memBytes / 1024ULL) << "\n";
	}
#elif defined(__linux__)
	{
		std::ifstream cpuFile("/proc/cpuinfo");
		std::string line;
		while (std::getline(cpuFile, line))
		{
			if (line.find("model name") == 0)
			{
				auto colonPos = line.find(':');
				if (colonPos != std::string::npos)
				{
					std::string model = line.substr(colonPos + 1);
					auto firstChar = model.find_first_not_of(" \t");
					if (firstChar != std::string::npos)
						ss << "Processor:" << model.substr(firstChar) << "\n";
				}
				break;
			}
		}
		struct sysinfo si = {};
		if (sysinfo(&si) == 0 && si.mem_unit > 0)
		{
			uint64_t totalKB = ((uint64_t)si.totalram * (uint64_t)si.mem_unit) / 1024ULL;
			uint64_t availKB = ((uint64_t)si.freeram * (uint64_t)si.mem_unit) / 1024ULL;
			ss << "TotalPhysicalMemory:" << totalKB << "\n";
			ss << "FreePhysicalMemory:" << availKB << "\n";
		}
	}
#endif
#ifndef EMSCRIPTEN_SINGLE_THREAD
	unsigned int nCores = std::thread::hardware_concurrency();
	if (nCores > 0)
		ss << "Number of processors:" << nCores << "\n";
#endif
	s_sysInfo = ss.str();
	return s_sysInfo;
#endif
}

void ParaEngine::ParaEngineSettings::SetAutoLowerFrameRateWhenNotFocused( bool bEnabled )
{
	if (CGlobals::GetApp())
		CGlobals::GetApp()->SetAutoLowerFrameRateWhenNotFocused(bEnabled);
}

bool ParaEngine::ParaEngineSettings::GetAutoLowerFrameRateWhenNotFocused()
{
	return CGlobals::GetApp() ? CGlobals::GetApp()->GetAutoLowerFrameRateWhenNotFocused() : false;
}

void ParaEngine::ParaEngineSettings::SetToggleSoundWhenNotFocused( bool bEnabled )
{
	if (CGlobals::GetApp())
		CGlobals::GetApp()->SetToggleSoundWhenNotFocused(bEnabled);
}

bool ParaEngine::ParaEngineSettings::GetToggleSoundWhenNotFocused()
{
	return CGlobals::GetApp() ? CGlobals::GetApp()->GetToggleSoundWhenNotFocused() : false;
}

void ParaEngine::ParaEngineSettings::Enable32bitsTexture( bool bEnable )
{
	TextureEntity::g_bEnable32bitsTexture = bEnable;
}

bool ParaEngine::ParaEngineSettings::Is32bitsTextureEnabled()
{
	return TextureEntity::g_bEnable32bitsTexture;
}

void ParaEngine::ParaEngineSettings::SetAppHasFocus( bool bEnabled )
{
	OUTPUT_LOG("warning: SetAppHasFocus is not implemented \n");
}

bool ParaEngine::ParaEngineSettings::GetAppHasFocus()
{
	return CGlobals::GetApp()->AppHasFocus();
}

void ParaEngine::ParaEngineSettings::SetCaptureMouse(bool bCapture)
{
	CGlobals::GetGUI()->SetCaptureMouse(bCapture);
}

bool ParaEngine::ParaEngineSettings::IsMouseCaptured()
{
	return CGlobals::GetGUI()->IsMouseCaptured();
}

Vector2 ParaEngine::ParaEngineSettings::GetWindowResolution()
{
#ifdef WIN32
	HWND hWnd = CGlobals::GetAppHWND();
	if (hWnd)
	{
		RECT rect = {};
		if (GetWindowRect(hWnd, &rect))
			return Vector2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
	}
#elif defined(PLATFORM_MAC)
	if (CGlobals::GetApp() && CGlobals::GetApp()->GetRenderWindow())
	{
		auto* pWin = CGlobals::GetApp()->GetRenderWindow();
		return Vector2((float)pWin->GetWidth(), (float)pWin->GetHeight());
	}
#endif
	Vector2 res(0, 0);
	if (CGlobals::GetApp())
		CGlobals::GetApp()->GetScreenResolution(&res);
	return res;
}

bool ParaEngine::ParaEngineSettings::IsSandboxMode()
{
	return m_bSandboxMode;
}

void ParaEngine::ParaEngineSettings::SetSandboxMode(bool val)
{
	m_bSandboxMode = val;
}

void ParaEngine::ParaEngineSettings::SetDefaultOpenFileFolder(const char* sDefaultOpenFileFolder)
{
#if !defined(__EMSCRIPTEN__) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	CCommonFileDialog::GetSingleton()->SetDefaultOpenFileFolder(sDefaultOpenFileFolder);
#endif
}

const char* ParaEngine::ParaEngineSettings::GetOpenFolder()
{
#if !defined(__EMSCRIPTEN__) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	return CCommonFileDialog::GetSingleton()->OpenFolder();
#else
	return NULL;
#endif
}

int ParaEngine::ParaEngineSettings::GetPlatform()
{
	return PARA_TARGET_PLATFORM;
}

bool ParaEngine::ParaEngineSettings::IsMobilePlatform()
{
#ifdef PARAENGINE_MOBILE
	return true;
#else
	return false;
#endif
}

void ParaEngine::ParaEngineSettings::RecreateRenderer()
{
#ifdef PARAENGINE_MOBILE
	//TODO:RecreateRenderer
	//CParaEngineApp::GetInstance()->listenRendererRecreated(NULL);
#endif
}

ParaEngine::LanguageType ParaEngine::ParaEngineSettings::GetCurrentLanguage()
{
	if (m_currentLanguage >= 0)
		return (ParaEngine::LanguageType)m_currentLanguage;
	LanguageType ret = LanguageType::ENGLISH;
#ifdef WIN32
	LCID localeID = GetUserDefaultLCID();
	unsigned short primaryLanguageID = localeID & 0xFF;

	switch (primaryLanguageID)
	{
	case LANG_CHINESE:
		ret = LanguageType::CHINESE;
		break;
	case LANG_ENGLISH:
		ret = LanguageType::ENGLISH;
		break;
	case LANG_FRENCH:
		ret = LanguageType::FRENCH;
		break;
	case LANG_ITALIAN:
		ret = LanguageType::ITALIAN;
		break;
	case LANG_GERMAN:
		ret = LanguageType::GERMAN;
		break;
	case LANG_SPANISH:
		ret = LanguageType::SPANISH;
		break;
	case LANG_DUTCH:
		ret = LanguageType::DUTCH;
		break;
	case LANG_RUSSIAN:
		ret = LanguageType::RUSSIAN;
		break;
	case LANG_KOREAN:
		ret = LanguageType::KOREAN;
		break;
	case LANG_JAPANESE:
		ret = LanguageType::JAPANESE;
		break;
	case LANG_HUNGARIAN:
		ret = LanguageType::HUNGARIAN;
		break;
	case LANG_PORTUGUESE:
		ret = LanguageType::PORTUGUESE;
		break;
	case LANG_ARABIC:
		ret = LanguageType::ARABIC;
		break;
	case LANG_NORWEGIAN:
		ret = LanguageType::NORWEGIAN;
		break;
	case LANG_POLISH:
		ret = LanguageType::POLISH;
		break;
	}
#else
	// we only provide win32 implementation, for other implementations, one can use SetCurrentLanguage. 
#endif
	m_currentLanguage = (int)ret;
	return ret;
}

void ParaEngine::ParaEngineSettings::SetCurrentLanguage(LanguageType lang)
{
	m_currentLanguage = (int)lang;
}

int ParaEngine::ParaEngineSettings::GetAppCount()
{
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	std::string sProcessName = GetModuleFileName();
	if (sProcessName.empty())
		sProcessName = "ParaEngineClient.exe";
	CProcessIterator   itp;
	int nCount = 0;
	for (DWORD pid = itp.First(); pid; pid = itp.Next())   {
		char   name[_MAX_PATH] = { 0 };
		CProcessModuleIterator   itm(pid);
		HMODULE   hModule = itm.First();   //   .EXE   
		if (hModule)
		{
			GetModuleBaseNameA(itm.GetProcessHandle(), hModule, name, _MAX_PATH);

			if (_stricmp(sProcessName.c_str(), name) == 0 || _stricmp("ParaEngineClient.exe", name) == 0)
				nCount ++;
		}
	}
	return nCount > 0 ? nCount : 1;
#elif defined(__linux__) && !defined(ANDROID) && !defined(NPLRUNTIME_SERVER)
	std::string selfComm;
	{
		std::ifstream selfFile("/proc/self/comm");
		std::getline(selfFile, selfComm);
	}
	if (selfComm.empty())
		return 1;
	int nCount = 0;
	DIR* procDir = opendir("/proc");
	if (procDir)
	{
		struct dirent* entry = nullptr;
		while ((entry = readdir(procDir)) != nullptr)
		{
			if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
				continue;
			std::string commPath = std::string("/proc/") + entry->d_name + "/comm";
			std::ifstream commFile(commPath);
			std::string comm;
			if (std::getline(commFile, comm) && comm == selfComm)
				++nCount;
		}
		closedir(procDir);
	}
	return nCount > 0 ? nCount : 1;
#else
	return 1;
#endif
}

const char* ParaEngine::ParaEngineSettings::GetProcessName()
{
	static std::string g_processName;
	if (g_processName.empty())
	{
#ifdef WIN32
	#if BOOST_VERSION >= 108600
		g_processName = GetModuleFileName();
	#else
		g_processName = boost::log::aux::get_process_name();
	#endif
#else
		g_processName = GetModuleFileName();
#endif
	}
	return g_processName.c_str();
}

int ParaEngine::ParaEngineSettings::GetFPS()
{
	return (int)CGlobals::GetApp()->GetFPS();
}

int ParaEngine::ParaEngineSettings::GetTriangleCount()
{
	return 0;
}

int ParaEngine::ParaEngineSettings::GetDrawCallCount()
{
	return 0;
}

bool ParaEngine::ParaEngineSettings::Is64BitsSystem()
{
	return sizeof(void*) > 4;
}

void ParaEngine::ParaEngineSettings::LoadNameIndex()
{
	m_name_to_index.clear();
	m_name_to_index["AssetManager"] = 0;
	m_name_to_index["GUI"] = 1;
	m_name_to_index["Scene"] = 2;
	m_name_to_index["ViewportManager"] = 3;
	m_name_to_index["NPL"] = 4;
	m_name_to_index["SelectionManager"] = 5;
	m_name_to_index["Painter"] = 6;
	m_name_to_index["BufferPicking"] = 7;
	m_name_to_index["OverlayPicking"] = 8;
	m_name_to_index["AsyncLoader"] = 9;
	m_name_to_index["gameFRC"] = 10;
	m_name_to_index["AudioEngine"] = 11;
}

IAttributeFields* ParaEngine::ParaEngineSettings::GetChildAttributeObject(const char * sName)
{
	auto it = m_name_to_index.find(sName);
	return (it != m_name_to_index.end()) ? GetChildAttributeObject(it->second, 0) : NULL;
}

IAttributeFields* ParaEngine::ParaEngineSettings::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nRowIndex == 0)
		return CGlobals::GetAssetManager();
	else if (nRowIndex == 1)
		return CGlobals::GetGUI();
	else if (nRowIndex == 2)
		return CGlobals::GetScene();
	else if (nRowIndex == 3)
		return CGlobals::GetViewportManager();
	else if (nRowIndex == 4)
		return NPL::CNPLRuntime::GetInstance();
	else if (nRowIndex == 5)
		return CGlobals::GetSelectionManager();
	else if (nRowIndex == 6)
		return CGlobals::GetGUI()->GetPainter();
	else if (nRowIndex == 7)
		return CGlobals::GetAssetManager()->LoadBufferPick("backbuffer");
	else if (nRowIndex == 8)
		return CGlobals::GetAssetManager()->LoadBufferPick("overlay");
	else if (nRowIndex == 9)
		return &(CAsyncLoader::GetSingleton());
	else if (nRowIndex == 10)
		return &g_gameTime;
	else if (nRowIndex == 11)
		return CAudioEngine2::GetInstance();
	else
		return NULL;
}

int ParaEngine::ParaEngineSettings::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return (int)m_name_to_index.size();
}

int ParaEngine::ParaEngineSettings::GetChildAttributeColumnCount()
{
	return 1;
}

void ParaEngine::ParaEngineSettings::SetIcon(const char* sIconFile)
{
	if (!sIconFile)
		return;
#ifdef WIN32
	// TODO: perhaps CreateIcon(), though it is not recommended by Microsoft. 
	std::string sIcon = sIconFile;
	HICON hIcon = 0;
	if (sIcon == "IDI_APPLICATION")
		hIcon = LoadIcon(NULL, IDI_APPLICATION);
	else if (sIcon == "IDI_HAND")
		hIcon = LoadIcon(NULL, IDI_HAND);
	else if (sIcon == "IDI_ERROR")
		hIcon = LoadIcon(NULL, IDI_ERROR);
	else if (sIcon == "IDI_WARNING")
		hIcon = LoadIcon(NULL, IDI_WARNING);
	else
	{
		CParaFile file(sIconFile);
		if (!file.isEof())
		{

			PBYTE iconData = (PBYTE)file.getBuffer();

			if (file.getSize() > (6 + 16))
			{
				BYTE w = *(iconData + 6);
				BYTE h = *(iconData + 7);

				int offset = LookupIconIdFromDirectoryEx(iconData, TRUE, w, h, LR_DEFAULTCOLOR);
				if (offset != 0)
				{
					hIcon = CreateIconFromResourceEx(iconData + offset, file.getSize() - offset, TRUE, 0x30000, w, h, LR_DEFAULTCOLOR);
				}
			}


		}
	}


	if (hIcon != 0)
		SendMessage(CGlobals::GetAppHWND(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	else
	{
		OUTPUT_LOG("warning: icon file:%s not supported: use IDI_APPLICATION, IDI_ERROR, etc \n", sIconFile);
	}
#endif
}

void ParaEngine::ParaEngineSettings::SetShowWindowTitleBar(bool bEnabled)
{
#ifdef WIN32
	if (CGlobals::GetApp() && CGlobals::GetApp()->IsWindowedMode())
	{
		HWND hWnd = CGlobals::GetAppHWND();
		if (!hWnd || !::IsWindow(hWnd))
			return;
		LONG dwAttr = GetWindowLong(hWnd, GWL_STYLE);
		if (bEnabled)
		{
			dwAttr |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		}
		else
		{
			dwAttr &= (~(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
		}
		SetWindowLong(hWnd, GWL_STYLE, dwAttr);
		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
#endif
}

bool ParaEngine::ParaEngineSettings::IsShowWindowTitleBar()
{
#ifdef WIN32
	if (CGlobals::GetApp()->IsWindowedMode())
	{
		LONG dwAttr = GetWindowLong(CGlobals::GetAppHWND(), GWL_STYLE);
		return (dwAttr & WS_CAPTION) != 0;
	}
#endif
	return true;
}

const std::string& ParaEngine::ParaEngineSettings::GetMaxMacAddress()
{
	return NetworkAdapter::GetInstance()->GetMaxMacAddress();
}

const std::string& ParaEngine::ParaEngineSettings::GetMaxIPAddress()
{
	return NetworkAdapter::GetInstance()->GetMaxIPAddress();
}

const std::string& ParaEngine::ParaEngineSettings::GetMachineID_old()
{
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
	static std::string str = "";
	if (str.empty())
	{
		struct dmi_header_local
		{
			BYTE type;
			BYTE length;
			WORD handle;
		};

		struct RawSMBIOSDataLocal
		{
			BYTE Used20CallingMethod;
			BYTE SMBIOSMajorVersion;
			BYTE SMBIOSMinorVersion;
			BYTE DmiRevision;
			DWORD Length;
			BYTE SMBIOSTableData[1];
		};

		DWORD bufSize = 0;
		std::vector<BYTE> buf(65535, 0);
		int flag = 1;

		do
		{
			auto ret = GetSystemFirmwareTable('RSMB', 0, 0, 0);
			if (!ret || ret > buf.size())
			{
				OUTPUT_LOG("GetSystemFirmwareTable function init failed\n");
				break;
			}

			bufSize = ret;
			ret = GetSystemFirmwareTable('RSMB', 0, buf.data(), bufSize);
			if (!ret)
			{
				OUTPUT_LOG("GetSystemFirmwareTable function failed\n");
				break;
			}

			RawSMBIOSDataLocal* Smbios = reinterpret_cast<RawSMBIOSDataLocal*>(buf.data());
			BYTE* p = Smbios->SMBIOSTableData;
			BYTE* pEnd = Smbios->SMBIOSTableData + Smbios->Length;

			if (Smbios->Length != bufSize - 8)
			{
				OUTPUT_LOG("Smbios length error\n");
				break;
			}

			for (DWORD i = 0; i < Smbios->Length && p + sizeof(dmi_header_local) <= pEnd; i++)
			{
				dmi_header_local* h = reinterpret_cast<dmi_header_local*>(p);
				if (h->type == 0 && flag)
				{
					flag = 0;
				}
				else if (h->type == 1 && p + 0x18 <= pEnd)
				{
					const BYTE* uuidPtr = p + 0x8;
					short ver = static_cast<short>(Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion);
					int only0xFF = 1, only0x00 = 1;
					for (int k = 0; k < 16 && (only0x00 || only0xFF); k++)
					{
						if (uuidPtr[k] != 0x00) only0x00 = 0;
						if (uuidPtr[k] != 0xFF) only0xFF = 0;
					}
					if (!only0xFF && !only0x00)
					{
						char uuidStr[64] = { 0 };
						if (ver >= 0x0206)
						{
							snprintf(uuidStr, 64, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
								uuidPtr[3], uuidPtr[2], uuidPtr[1], uuidPtr[0], uuidPtr[5], uuidPtr[4], uuidPtr[7], uuidPtr[6],
								uuidPtr[8], uuidPtr[9], uuidPtr[10], uuidPtr[11], uuidPtr[12], uuidPtr[13], uuidPtr[14], uuidPtr[15]);
						}
						else
						{
							snprintf(uuidStr, 64, "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
								uuidPtr[0], uuidPtr[1], uuidPtr[2], uuidPtr[3], uuidPtr[4], uuidPtr[5], uuidPtr[6], uuidPtr[7],
								uuidPtr[8], uuidPtr[9], uuidPtr[10], uuidPtr[11], uuidPtr[12], uuidPtr[13], uuidPtr[14], uuidPtr[15]);
						}
						str = uuidStr;
					}
					break;
				}

				p += h->length;
				while (p + 1 < pEnd && (*(reinterpret_cast<WORD*>(p))) != 0)
				{
					p++;
				}
				p += 2;
			}
		} while (false);
	}
	return str;
#else
	return GetMachineID();
#endif
}

int ParaEngine::ParaEngineSettings::GetPeakMemoryUse()
{
	return (int)ParaEngine::GetPeakMemoryUse();
}

int ParaEngine::ParaEngineSettings::GetCurrentMemoryUse()
{
	return (int)ParaEngine::GetCurrentMemoryUse();
}

int ParaEngine::ParaEngineSettings::GetProcessId()
{
	return ParaEngine::GetProcessID();
}

size_t ParaEngine::ParaEngineSettings::GetVertexBufferPoolTotalBytes()
{
	return CGlobals::GetAssetManager()->GetVertexBufferPoolManager().GetVertexBufferPoolTotalBytes();
}

bool ParaEngine::ParaEngineSettings::HasClosingRequest()
{
	return CGlobals::GetApp()->HasClosingRequest();
}

void ParaEngine::ParaEngineSettings::SetHasClosingRequest(bool val)
{
	CGlobals::GetApp()->SetHasClosingRequest(val);
}

intptr_t ParaEngine::ParaEngineSettings::GetAppHWND()
{
#if defined (PLATFORM_WINDOWS)
	return (intptr_t)CGlobals::GetAppHWND();
#else
	return 0;
#endif
}

int ParaEngine::ParaEngineSettings::GetWebVersion()
{
	return 1;
}

void ParaEngine::ParaEngineSettings::ResetAudioDevice(const char* deviceName)
{
	CAudioEngine2::GetInstance()->ResetAudioDevice(deviceName);
}

const char* ParaEngine::ParaEngineSettings::GetAudioDeviceName()
{
	static std::string g_audioDeviceName;
	g_audioDeviceName.clear();
	unsigned int count = CAudioEngine2::GetInstance()->GetDeviceCount();
	for (unsigned int i = 0; i < count; i++)
	{
		std::string name = CAudioEngine2::GetInstance()->GetDeviceName(i);
		if (!name.empty())
		{
			g_audioDeviceName.append(name);
			g_audioDeviceName.append(";");
		}
	}
	return g_audioDeviceName.c_str();
}

void ParaEngineSettings::SetRefreshTimer(float fTimerInterval)
{
	CGlobals::GetApp()->SetRefreshTimer(fTimerInterval);
}

float ParaEngineSettings::GetRefreshTimer()
{
	return CGlobals::GetApp()->GetRefreshTimer();
}

const char* ParaEngineSettings::GetModuleFileName()
{
	static std::string sModuleDir;
	sModuleDir = ParaEngine::GetExecutablePath();
	auto pos = sModuleDir.find_last_of('/');

	if (pos == std::string::npos)
	{
		pos = sModuleDir.find_last_of('\\');
	}

	if (pos == std::string::npos)
	{
		return sModuleDir.c_str();
	}

	return sModuleDir.c_str() + pos + 1;
}

const std::string& ParaEngineSettings::GetDefaultFileAPIEncoding()
{
	static std::string ret = "";
	ret = StringHelper::GetDefaultCPName();
	return ret;
}

const char* ParaEngineSettings::GetWorldDirectory()
{
	static std::string g_strWorldDir;
	if (CGlobals::GetWorldInfo())
	{
		g_strWorldDir = CGlobals::GetWorldInfo()->GetWorldDirectory();
		return g_strWorldDir.c_str();
	}
	return CGlobals::GetString(0).c_str();
}

void ParaEngineSettings::SetWorldDirectory(const char* sWorldDirectory)
{
	if (sWorldDirectory && CGlobals::GetWorldInfo())
	{
		CGlobals::GetWorldInfo()->ResetWorldName(sWorldDirectory);
	}
}

void ParaEngine::ParaEngineSettings::SetPythonToLua(const char* python_code)
{
	*GetPythonToLua() = ParaPythonToLua(python_code);
}

void ParaEngine::ParaEngineSettings::SetCppToLua(const char* cpp_code)
{
	*GetCppToLua() = ParaCppToLua(cpp_code);
}

const std::string& ParaEngine::ParaEngineSettings::GetLandscapeMode()
{
	static std::string s_landscape = "";
	s_landscape = CGlobals::GetApp()->GetLandscapeMode();
	return s_landscape;
}

void ParaEngine::ParaEngineSettings::SetLandscapeMode(const char* mode)
{
	CGlobals::GetApp()->SetLandscapeMode(mode);
}

bool ParaEngine::ParaEngineSettings::IsScreenRotated()
{
	return CGlobals::GetApp()->IsRotateScreen();
}

const char* ParaEngine::ParaEngineSettings::GetRendererName()
{
	auto* pDevice = CGlobals::GetRenderDevice();
	return pDevice ? pDevice->GetCaps().backendName : "Null";
}

int ParaEngineSettings::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("script editor", FieldType_String, (void*)SetScriptEditor_s, (void*)GetScriptEditor_s, NULL, NULL, bOverride);

	pClass->AddField("Ctor Color", FieldType_Vector3, (void*)SetCtorColor_s, (void*)GetCtorColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);
	pClass->AddField("Ctor Height", FieldType_Float, (void*)SetCtorHeight_s, (void*)GetCtorHeight_s, NULL, NULL, bOverride);
	pClass->AddField("Ctor Speed", FieldType_Float, (void*)SetCtorSpeed_s, (void*)GetCtorSpeed_s, NULL, NULL, bOverride);

	pClass->AddField("Selection Color", FieldType_Vector3, (void*)SetSelectionColor_s, (void*)GetSelectionColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);

	pClass->AddField("Is Debugging", FieldType_Bool, NULL, (void*)IsDebugging_s, NULL, NULL, bOverride);

	pClass->AddField("IsTouchInputting", FieldType_Bool, NULL, (void*)IsTouchInputting_s, NULL, NULL, bOverride);

	pClass->AddField("IsSlateMode", FieldType_Bool, NULL, (void*)IsSlateMode_s, NULL, NULL, bOverride);
	
	pClass->AddField("Is Editing", FieldType_Bool, (void*)SetEditingMode_s, (void*)IsEditing_s, NULL, NULL, bOverride);
	pClass->AddField("Effect Level", FieldType_Int, (void*)LoadGameEffectSet_s, (void*)GetGameEffectSet_s, NULL, NULL, bOverride);
	pClass->AddField("TextureLOD", FieldType_Int, (void*)SetTextureLOD_s, (void*)GetTextureLOD_s, NULL, NULL, bOverride);

	pClass->AddField("Locale", FieldType_String, (void*)SetLocale_s, (void*)GetLocale_s, NULL, NULL, bOverride);
	pClass->AddField("CurrentLanguage", FieldType_Int, (void*)NULL, (void*)GetCurrentLanguage_s, NULL, NULL, bOverride);

	pClass->AddField("IsMouseInverse", FieldType_Bool, (void*)SetMouseInverse_s1, (void*)GetMouseInverse_s1, NULL, NULL, bOverride);
	pClass->AddField("WindowText", FieldType_String, (void*)SetWindowText1_s, (void*)GetWindowText1_s, NULL, NULL, bOverride);
	pClass->AddField("IgnoreWindowSizeChange", FieldType_Bool, (void*)SetIgnoreWindowSizeChange_s, (void*)GetIgnoreWindowSizeChange_s, NULL, NULL, bOverride);
	pClass->AddField("CaptureMouse", FieldType_Bool, (void*)SetCaptureMouse_s, (void*)IsMouseCaptured_s, NULL, NULL, bOverride);
		
	pClass->AddField("HasNewConfig", FieldType_Bool, (void*)SetHasNewConfig_s, (void*)HasNewConfig_s, NULL, NULL, bOverride);

	pClass->AddField("IsWindowClosingAllowed", FieldType_Bool, (void*)SetAllowWindowClosing_s, (void*)IsWindowClosingAllowed_s, NULL, NULL, bOverride);
	pClass->AddField("HasClosingRequest", FieldType_Bool, (void*)SetHasClosingRequest_s, (void*)HasClosingRequest_s, NULL, NULL, bOverride);
	pClass->AddField("IsFullScreenMode", FieldType_Bool, (void*)SetFullScreenMode_s, (void*)IsFullScreenMode_s, NULL, NULL, bOverride);
	pClass->AddField("BringWindowToTop", FieldType_void, (void*)BringWindowToTop_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("UpdateScreenMode", FieldType_void, (void*)UpdateScreenMode_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ShowMenu", FieldType_Bool, (void*)ShowMenu_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ScreenResolution", FieldType_Vector2, (void*)SetScreenResolution_s, (void*)GetScreenResolution_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);
	pClass->AddField("MultiSampleType", FieldType_Int, (void*)SetMultiSampleType_s, (void*)GetMultiSampleType_s, NULL, NULL, bOverride);
	pClass->AddField("MultiSampleQuality", FieldType_Int, (void*)SetMultiSampleQuality_s, (void*)GetMultiSampleQuality_s, NULL, NULL, bOverride);

	pClass->AddField("EnableProfiling", FieldType_Bool, (void*)EnableProfiling_s, (void*)IsProfilingEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("Enable3DRendering", FieldType_Bool, (void*)Enable3DRendering_s, (void*)Is3DRenderingEnabled_s, NULL, NULL, bOverride);

	pClass->AddField("PixelShaderVersion", FieldType_Int, NULL, (void*)GetPixelShaderVersion_s, NULL, NULL, bOverride);
	pClass->AddField("VertexShaderVersion", FieldType_Int, NULL, (void*)GetVertexShaderVersion_s, NULL, NULL, bOverride);
	pClass->AddField("SystemInfoString", FieldType_String, NULL, (void*)GetSystemInfoString_s, NULL, NULL, bOverride);
	pClass->AddField("DisplayMode", FieldType_String, NULL, (void*)GetDisplayMode_s, NULL, NULL, bOverride);
	pClass->AddField("MonitorResolution", FieldType_Vector2, NULL, (void*)GetMonitorResolution_s, NULL, NULL, bOverride);
	pClass->AddField("WindowResolution", FieldType_Vector2, NULL, (void*)GetWindowResolution_s, NULL, NULL, bOverride);
	pClass->AddField("VisibleSize", FieldType_Vector2, NULL, (void*)GetVisibleSize_s, NULL, NULL, bOverride);
	pClass->AddField("IsWindowMaximized", FieldType_Bool, (void*)SetWindowMaximized_s, (void*)IsWindowMaximized_s, NULL, NULL, bOverride);

	pClass->AddField("AsyncLoaderItemsLeft", FieldType_Int, NULL, (void*)GetAsyncLoaderItemsLeft_s, NULL, NULL, bOverride);
	pClass->AddField("AsyncLoaderBytesReceived", FieldType_Int, NULL, (void*)GetAsyncLoaderBytesReceived_s, NULL, NULL, bOverride);
	pClass->AddField("AsyncLoaderRemainingBytes", FieldType_Int, NULL, (void*)GetAsyncLoaderRemainingBytes_s, NULL, NULL, bOverride);

#if !defined(NPLRUNTIME_SERVER)
	pClass->AddField("ProfileUnlimitedFPS", FieldType_Bool, (void*)SetProfileUnlimitedFPS_s, (void*)GetProfileUnlimitedFPS_s,
		NULL, "Request the fastest supported rendering for this profiling session", bOverride);
#endif
	pClass->AddField("RefreshTimer", FieldType_Float, (void*)SetRefreshTimer_s, (void*)GetRefreshTimer_s, NULL, NULL, bOverride);
	
	pClass->AddField("AllocConsole", FieldType_void, (void*)AllocConsole_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ConsoleTextAttribute", FieldType_Int, (void*)SetConsoleTextAttribute_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("CoreUsage", FieldType_Int, (void*)SetCoreUsage_s, (void*)GetCoreUsage_s, NULL, NULL, bOverride);
	pClass->AddField("AppCount", FieldType_Int, NULL, (void*)GetAppCount_s, NULL, NULL, bOverride);
	pClass->AddField("ProcessName", FieldType_String, NULL, (void*)GetProcessName_s, NULL, NULL, bOverride);

	pClass->AddField("ToggleSoundWhenNotFocused", FieldType_Bool, (void*)SetToggleSoundWhenNotFocused_s, (void*)GetToggleSoundWhenNotFocused_s, NULL, NULL, bOverride);
	pClass->AddField("AutoLowerFrameRateWhenNotFocused", FieldType_Bool, (void*)SetAutoLowerFrameRateWhenNotFocused_s, (void*)GetAutoLowerFrameRateWhenNotFocused_s, NULL, NULL, bOverride);
	pClass->AddField("AppHasFocus", FieldType_Bool, (void*)SetAppHasFocus_s, (void*)GetAppHasFocus_s, NULL, NULL, bOverride);

	pClass->AddField("Is32bitsTextureEnabled", FieldType_Bool, (void*)Enable32bitsTexture_s, (void*)Is32bitsTextureEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("IsServerMode", FieldType_Bool, (void*)0, (void*)IsServerMode_s, NULL, NULL, bOverride);

	pClass->AddField("MaxMacAddress", FieldType_String, NULL, (void*)GetMaxMacAddress_s, NULL, NULL, bOverride);
	pClass->AddField("MaxIPAddress", FieldType_String, NULL, (void*)GetMaxIPAddress_s, NULL, NULL, bOverride);
	pClass->AddField("MachineID", FieldType_String, NULL, (void*)GetMachineID_s, NULL, NULL, bOverride);
	pClass->AddField("MachineID_old", FieldType_String, NULL, (void*)GetMachineID_old_s, NULL, NULL, bOverride);

	pClass->AddField("OpenFileFolder", FieldType_String, (void*)SetDefaultOpenFileFolder_s, (void*)GetOpenFolder_s, CAttributeField::GetSimpleSchema(SCHEMA_DIALOG), NULL, bOverride);
	pClass->AddField("Platform", FieldType_Int, NULL, (void*)GetPlatform_s, NULL, NULL, bOverride);
	pClass->AddField("ProcessId", FieldType_Int, NULL, (void*)GetProcessId_s, NULL, NULL, bOverride);
	pClass->AddField("IsMobilePlatform", FieldType_Bool, NULL, (void*)IsMobilePlatform_s, NULL, NULL, bOverride);
	pClass->AddField("Is64BitsSystem", FieldType_Bool, NULL, (void*)Is64BitsSystem_s, NULL, NULL, bOverride);
	pClass->AddField("RecreateRenderer", FieldType_void, (void*)RecreateRenderer_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("Icon", FieldType_String, (void*)SetIcon_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("LockWindowSize", FieldType_Bool, (void*)SetLockWindowSize_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ShowWindowTitleBar", FieldType_Bool, (void*)SetShowWindowTitleBar_s, (void*)IsShowWindowTitleBar_s, NULL, NULL, bOverride);
	pClass->AddField("WritablePath", FieldType_String, (void*)SetWritablePath_s, (void*)GetWritablePath_s, NULL, NULL, bOverride);
	pClass->AddField("SandboxMode", FieldType_Bool, (void*)SetSandboxMode_s, (void*)IsSandboxMode_s, NULL, NULL, bOverride);

	pClass->AddField("FPS", FieldType_Float, NULL, (void*)GetFPS_s, NULL, NULL, bOverride);
	pClass->AddField("TriangleCount", FieldType_Int, NULL, (void*)GetTriangleCount_s, NULL, NULL, bOverride);
	pClass->AddField("DrawCallCount", FieldType_Int, NULL, (void*)GetDrawCallCount_s, NULL, NULL, bOverride);

	pClass->AddField("CurrentMemoryUse", FieldType_Int, NULL, (void*)GetCurrentMemoryUse_s, NULL, NULL, bOverride);
	pClass->AddField("PeakMemoryUse", FieldType_Int, NULL, (void*)GetPeakMemoryUse_s, NULL, NULL, bOverride);
	pClass->AddField("VertexBufferPoolTotalBytes", FieldType_Int, NULL, (void*)GetVertexBufferPoolTotalBytes_s, NULL, NULL, bOverride);

	pClass->AddField("AppHWND", FieldType_Double, NULL, (void*)GetAppHWND_s, NULL, NULL, bOverride);

	pClass->AddField("GetModuleFileName", FieldType_String, nullptr, (void*)GetModuleFileName_s, NULL, NULL, bOverride);
	
	pClass->AddField("ResetAudioDevice", FieldType_String, (void*)ResetAudioDevice_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("AudioDeviceName", FieldType_String, NULL, (void*)GetAudioDeviceName_s, NULL, NULL, bOverride);

	pClass->AddField("FlushDiskIO", FieldType_void, (void*)FlushDiskIO_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("SendMsgToJS", FieldType_String, (void*)SendMsgToJS_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("DefaultFileAPIEncoding", FieldType_String, NULL, (void*)GetDefaultFileAPIEncoding_s, NULL, NULL, bOverride);
	pClass->AddField("WorldDirectory", FieldType_String, (void*)SetWorldDirectory_s, (void*)GetWorldDirectory_s, NULL, NULL, bOverride);

	pClass->AddField("WebViewVersion", FieldType_Int, NULL, (void*)GetWebViewVersion_s, NULL, NULL, bOverride);
#ifdef ANDROID
	pClass->AddField("GetUsbMode", FieldType_Bool, NULL, (void*)GetUsbMode_s, NULL, NULL, bOverride);
	pClass->AddField("SendMsgToJava", FieldType_String, (void*)SendMsgToJava_s, NULL, NULL, NULL, bOverride);
#endif

#ifdef IOS
	pClass->AddField("SendMsgToObjectiveC", FieldType_String, (void*)SendMsgToObjectiveC_s, NULL, NULL, NULL, bOverride);
#endif

#if defined(ANDROID) || defined(IOS)
    pClass->AddField("SaveImageToGallery", FieldType_String, (void*)SaveImageToGallery_s, NULL, NULL, NULL, bOverride);
#endif

	pClass->AddField("PythonToLua", FieldType_String, (void*)SetPythonToLua_s, (void*)GetPythonToLua_s, NULL, NULL, bOverride);
	pClass->AddField("CppToLua", FieldType_String, (void*)SetCppToLua_s, (void*)GetCppToLua_s, NULL, NULL, bOverride);
	pClass->AddField("LandscapeMode", FieldType_String, (void*)SetLandscapeMode_s, (void*)GetLandscapeMode_s, NULL, NULL, bOverride);
	pClass->AddField("IsScreenRotated", FieldType_Bool, NULL, (void*)IsScreenRotated_s, NULL, NULL, bOverride);

	pClass->AddField("RendererName", FieldType_String, NULL, (void*)GetRendererName_s, NULL, NULL, bOverride);
	pClass->AddField("IsLowEndDevice", FieldType_Bool, NULL, (void*)IsLowEndRenderDevice_s, NULL, NULL, bOverride);
	
	return S_OK;
}

//-----------------------------------------------------------------------------
// Class:	CHTMLBrowserManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.10.28
// Note: I used a ParaEngine.plugin called MozHTMLRenderer, which in turn is a wrapper of uBrowser(see uBrowser.com), 
// which in turn is a wrapper of Mozilla (same core used in firefox 2.0)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FrameRateController.h"
#include "ParaWorldAsset.h"
#include "2dengine/GUIRoot.h"
#include "NPLHelper.h"
#include "IHTMLRenderer.h"
#include "PluginManager.h"
#include "PluginAPI.h"
#include "HtmlBrowserManager.h"
#include "Framework/Interface/Render/ITexture.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif
#include "memdebug.h"

using namespace ParaEngine;

// 6 browsers can be opened at the same time by default
#define DEFAULT_MAX_BROWSER_WINDOW_COUNT	6

/** @def if a window is not active for this number of seconds, it will be closed.*/
#define DEFAULT_WINDOW_TIMEOUT	10.f

/** @def default window width */
#define 	DEFAULT_WINDOW_WIDTH	512
/** @def default window height */
#define 	DEFAULT_WINDOW_HEIGHT	512


const char FILE_HTML_RENDERER_MANAGER[] = "MozHTMLRenderer.dll";

//////////////////////////////////////////////////////////////////////////
//
// CHTMLBrowserManager
//
//////////////////////////////////////////////////////////////////////////

CHTMLBrowserManager::CHTMLBrowserManager()
:m_pInterface(NULL), m_pEventbinding()
{
	m_browsers.clear();
	CEventBinding* pEventBinding =  new CEventBinding();
	pEventBinding->InitEventMappingTable();
	m_pEventbinding = EventBinding_cow_type(pEventBinding);

	SetMaxWindowNum(DEFAULT_MAX_BROWSER_WINDOW_COUNT);	
}

CHTMLBrowserManager::~CHTMLBrowserManager(void)
{
	if(m_pInterface)
	{
		Cleanup();
		reset();

		m_pInterface->DeleteThis();
		m_pInterface = NULL;
	}
}

void ParaEngine::CHTMLBrowserManager::Cleanup()
{
	UnloadAsset();
}

void ParaEngine::CHTMLBrowserManager::LoadAsset()
{

}

void ParaEngine::CHTMLBrowserManager::UnloadAsset()
{
	// unload flash controls
	int nCount = GetMaxWindowNum();
	for (int i=0;i<nCount;++i)
	{
		SAFE_DELETE(m_browsers[i]);
	}
}

void ParaEngine::CHTMLBrowserManager::GarbageCollectAll()
{
	UnloadAsset();
}

HRESULT ParaEngine::CHTMLBrowserManager::InitDeviceObjects()
{
	return S_OK;
}

HRESULT ParaEngine::CHTMLBrowserManager::RestoreDeviceObjects()
{
	return S_OK;
}

HRESULT ParaEngine::CHTMLBrowserManager::InvalidateDeviceObjects()
{
	int nCount = GetMaxWindowNum();
	for (int i=0;i<nCount;++i)
	{
		CHTMLBrowser* pBrowser = m_browsers[i];
		if(pBrowser)
		{
			pBrowser->InvalidateDeviceObjects();
		}
	}
	return S_OK;
}

HRESULT ParaEngine::CHTMLBrowserManager::DeleteDeviceObjects()
{
	return S_OK;
}

void CHTMLBrowserManager::SetMaxWindowNum(int nNum)
{
	if(nNum>=GetMaxWindowNum())
	{
		int nCount = GetMaxWindowNum();
		m_browsers.resize(nNum, NULL);
		for (int i=nCount;i<nNum;++i)
		{
			m_browsers[i] = 0;
		}
	}
	else
	{
		int nCount = GetMaxWindowNum();
		for (int i=nNum;i<nCount;++i)
		{
			SAFE_DELETE(m_browsers[i]);
		}
		m_browsers.resize(nNum, NULL);
	}
}

int CHTMLBrowserManager::GetMaxWindowNum()
{
	return (int)m_browsers.size();
}

bool ParaEngine::CHTMLBrowserManager::init( std::string appBaseDirIn, std::string profileDirNameIn )
{
	if(LoadHTMLManagerPlugin())
	{
		if(m_pInterface->init(appBaseDirIn, profileDirNameIn))
		{
			OUTPUT_LOG("HTML browser is successfully initialized with profile %s \n ", profileDirNameIn.c_str());
			return true;
		}
		else
		{
			OUTPUT_LOG("warning: HTML browser failed to initialize. error code is %d\n", getLastError());
		}
	}
	return false;
}

bool ParaEngine::CHTMLBrowserManager::LoadHTMLManagerPlugin()
{
	if(m_pInterface==0)
	{
		//////////////////////////////////////////////////////////////////////////
		// load from plug-in
		//////////////////////////////////////////////////////////////////////////
		const char * sFileName = FILE_HTML_RENDERER_MANAGER;
		DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(sFileName);
		if(pPluginEntity==0)
		{
			// load the plug-in if it has never been loaded before. 
			pPluginEntity = CGlobals::GetPluginManager()->LoadDLL("",sFileName);
		}
		if(pPluginEntity!=0)
		{
			// the plug-in must contain at least one implementation
			if(pPluginEntity->GetNumberOfClasses()>0)
			{
				// assume it is the first class
				ClassDescriptor* pDesc = pPluginEntity->GetClassDescriptor(0);

				// the super class must be of type HTMLRENDERER_CLASS_ID, so that we can safely cast.
				if(pDesc && pDesc->SuperClassID() == HTMLRENDERER_CLASS_ID)
				{
					m_pInterface = (IHTMLBrowserManager*)pDesc->Create();
					if(m_pInterface)
					{
						/* init upon first load */
						OUTPUT_LOG("HTML browser version is %s\n", getVersion().c_str());
						
						string sBaseDir;
						CParaFile::ToCanonicalFilePath(sBaseDir, CParaFile::GetCurDirectory(0), true);
						string profileBaseDir = sBaseDir.substr( 0, sBaseDir.find_last_of("\\/") );

						if(init(profileBaseDir, "PEBrowser"))
						{
							setBrowserAgentId( "PEBrowser.agent");
						}
					}
				}
			}
		}
		if(m_pInterface==0)
		{
			OUTPUT_LOG("error: unable to load IHTMLBrowserManager plug-in. \r\n");
		}
	}
	return m_pInterface!=0;
}

bool ParaEngine::CHTMLBrowserManager::reset()
{
	if(!LoadHTMLManagerPlugin())
		return false;
	return m_pInterface->reset();
}

bool ParaEngine::CHTMLBrowserManager::clearCache()
{
	if(!LoadHTMLManagerPlugin())
		return false;
	return m_pInterface->clearCache();
}

int ParaEngine::CHTMLBrowserManager::getLastError()
{
	if(!LoadHTMLManagerPlugin())
		return 0;
	return m_pInterface->getLastError();
}

const std::string ParaEngine::CHTMLBrowserManager::getVersion()
{
	if(!LoadHTMLManagerPlugin())
		return "";
	return m_pInterface->getVersion();
}

void ParaEngine::CHTMLBrowserManager::setBrowserAgentId( std::string idIn )
{
	if(!LoadHTMLManagerPlugin())
		return;
	m_pInterface->setBrowserAgentId(idIn);
}

CHTMLBrowser* ParaEngine::CHTMLBrowserManager::createBrowserWindow( const char* sFileName , int browserWindowWidthIn, int browserWindowHeightIn )
{
	if(!LoadHTMLManagerPlugin())
		return NULL;

	CHTMLBrowser* pBrowser = GetBrowserWindow(sFileName);
	if(pBrowser == 0)
	{
		int nCount = GetMaxWindowNum();
		// try creating or updating the a player if there is no flash player matching the file name.
		for (int i=0;i<nCount;++i)
		{
			pBrowser = m_browsers[i];
			if(pBrowser == 0)
			{
				int nBrowserWindowId = m_pInterface->createBrowserWindow(CGlobals::GetAppHWND(), browserWindowWidthIn, browserWindowHeightIn);
				if(nBrowserWindowId>=0)
				{
					OUTPUT_LOG("web browser window who ID is %d is created\n ", nBrowserWindowId);
					
					m_pInterface->setBackgroundColor(nBrowserWindowId, 255,255,255);
					pBrowser = new CHTMLBrowser(this, nBrowserWindowId);
					pBrowser->m_filename = sFileName;
					m_browsers[i] = pBrowser;
					break;
				}
			}
		}
	}
	return pBrowser;
}

bool ParaEngine::CHTMLBrowserManager::destroyBrowserWindow( int browserWindowIdIn )
{
	if(!LoadHTMLManagerPlugin())
		return false;
	int nCount = GetMaxWindowNum();
	for (int i=0;i<nCount;++i)
	{
		CHTMLBrowser* pBrowser = m_browsers[i];
		if(pBrowser && pBrowser->GetBrowserWindowID() == browserWindowIdIn)
		{
			m_pInterface->destroyBrowserWindow(browserWindowIdIn);
			SAFE_DELETE(m_browsers[i]);
		}
	}
	return true;
}

CHTMLBrowser* ParaEngine::CHTMLBrowserManager::GetBrowserWindow( const char* sFileName )
{
	// return the texture if it already exists.
	int nCount = GetMaxWindowNum();
	for (int i=0;i<nCount;++i)
	{
		CHTMLBrowser* pBrowser = m_browsers[i];
		if(pBrowser && pBrowser->m_filename == sFileName)
		{
			return pBrowser;
		}
	}
	return NULL;
}

CHTMLBrowser* ParaEngine::CHTMLBrowserManager::GetBrowserWindow( int nWindowID )
{
	// return the texture if it already exists.
	int nCount = GetMaxWindowNum();
	for (int i=0;i<nCount;++i)
	{
		CHTMLBrowser* pBrowser = m_browsers[i];
		if(pBrowser && pBrowser->GetBrowserWindowID() == nWindowID)
		{
			return pBrowser;
		}
	}
	return NULL;
}

void ParaEngine::CHTMLBrowserManager::SetEventScript( int nEvent,const SimpleScript *script )
{
	if(!!m_pEventbinding)
		m_pEventbinding->MapEventToScript(nEvent,script);
}

const SimpleScript* ParaEngine::CHTMLBrowserManager::GetEventScript( int nEvent ) const
{
	if(!!m_pEventbinding)
		return m_pEventbinding.c_ptr()->GetEventScript(nEvent);
	else
		return NULL;
}

bool ParaEngine::CHTMLBrowserManager::HasEvent( int etype )
{
	if(!!m_pEventbinding)
		return m_pEventbinding.c_ptr()->HasEventScript(etype);
	else
		return false;
}

bool ParaEngine::CHTMLBrowserManager::ActivateScript( int etype, const string &code )
{
	const SimpleScript *tempScript=GetEventScript(etype);
	if (tempScript) {
		CGUIRoot::GetInstance()->m_scripts.AddScript(tempScript->szFile,etype,code+tempScript->szCode);
		return true;
	}
	return false;
}

CHTMLBrowser* CHTMLBrowserManager::CreateGetBrowserWindow(const char* sFileName)
{
	CHTMLBrowser* pBrowser = GetBrowserWindow(sFileName);
	if(pBrowser == 0)
		pBrowser = createBrowserWindow(sFileName, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	return pBrowser;
}

//////////////////////////////////////////////////////////////////////////
//
// CHTMLBrowser
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CHTMLBrowser::CHTMLBrowser( CHTMLBrowserManager* manager, int nBrowserWindowId )
:m_manager(manager), m_nBrowserWindowId(nBrowserWindowId)
{
	m_manager->GetInterface()->addObserver( m_nBrowserWindowId, this );

	m_fTimeOut = DEFAULT_WINDOW_TIMEOUT;
	m_fLastUsedTime = 0.f;
	m_bTextureUpdated = false;
	m_bNeedUpdate = false;
	m_bUseDynamicTextures = false;
	m_pTexture = NULL;
}
CHTMLBrowser::~CHTMLBrowser()
{
	m_manager->GetInterface()->addObserver( m_nBrowserWindowId, this );
	InvalidateDeviceObjects();
}

void ParaEngine::CHTMLBrowser::Release()
{
	m_manager->GetInterface()->destroyBrowserWindow(m_nBrowserWindowId);
}

bool ParaEngine::CHTMLBrowser::setSize( int widthIn, int heightIn )
{
	// this causes the d3d texture to be recreated with the new size
	if(m_manager->GetInterface()->setSize(m_nBrowserWindowId, widthIn, heightIn))
	{
		InvalidateDeviceObjects();
		return true;
	}
	else
	{
		return false;
	}
}

bool ParaEngine::CHTMLBrowser::scrollByLines( int linesIn )
{
	return m_manager->GetInterface()->scrollByLines(m_nBrowserWindowId,linesIn);
}

bool ParaEngine::CHTMLBrowser::setBackgroundColor( const int redIn, const int greenIn, const int blueIn )
{
	return m_manager->GetInterface()->setBackgroundColor(m_nBrowserWindowId, redIn, greenIn, blueIn);
}

bool ParaEngine::CHTMLBrowser::setEnabled( bool enabledIn )
{
	return m_manager->GetInterface()->setEnabled(m_nBrowserWindowId,enabledIn);
}

bool ParaEngine::CHTMLBrowser::addObserver( IEmbeddedBrowserWindowObserver* subjectIn )
{
	return m_manager->GetInterface()->addObserver(m_nBrowserWindowId,subjectIn);
}

bool ParaEngine::CHTMLBrowser::remObserver( IEmbeddedBrowserWindowObserver* subjectIn )
{
	return m_manager->GetInterface()->remObserver(m_nBrowserWindowId,subjectIn);
}

bool ParaEngine::CHTMLBrowser::navigateTo( const std::string uriIn )
{
	m_LastNavURL = uriIn;
	return m_manager->GetInterface()->navigateTo(m_nBrowserWindowId,uriIn);
}

bool ParaEngine::CHTMLBrowser::navigateStop()
{
	return m_manager->GetInterface()->navigateStop(m_nBrowserWindowId);
}

bool ParaEngine::CHTMLBrowser::canNavigateBack()
{
	return m_manager->GetInterface()->canNavigateBack(m_nBrowserWindowId);
}

bool ParaEngine::CHTMLBrowser::navigateBack()
{
	return m_manager->GetInterface()->navigateBack(m_nBrowserWindowId);
}

bool ParaEngine::CHTMLBrowser::canNavigateForward()
{
	return m_manager->GetInterface()->canNavigateForward(m_nBrowserWindowId);
}

bool ParaEngine::CHTMLBrowser::navigateForward()
{
	return m_manager->GetInterface()->navigateForward(m_nBrowserWindowId);
}

const unsigned char* ParaEngine::CHTMLBrowser::grabBrowserWindow()
{
	return m_manager->GetInterface()->grabBrowserWindow(m_nBrowserWindowId);
}

const unsigned char* ParaEngine::CHTMLBrowser::getBrowserWindowPixels()
{
	return m_manager->GetInterface()->getBrowserWindowPixels(m_nBrowserWindowId);
}

const int ParaEngine::CHTMLBrowser::getBrowserWidth()
{
	return m_manager->GetInterface()->getBrowserWidth(m_nBrowserWindowId);
}

const int ParaEngine::CHTMLBrowser::getBrowserHeight()
{
	return m_manager->GetInterface()->getBrowserHeight(m_nBrowserWindowId);
}

const int ParaEngine::CHTMLBrowser::getBrowserDepth()
{
	return m_manager->GetInterface()->getBrowserDepth(m_nBrowserWindowId);
}

const int ParaEngine::CHTMLBrowser::getBrowserRowSpan()
{
	return m_manager->GetInterface()->getBrowserRowSpan(m_nBrowserWindowId);
}

bool ParaEngine::CHTMLBrowser::mouseDown( int xPosIn, int yPosIn )
{
	return m_manager->GetInterface()->mouseDown(m_nBrowserWindowId,xPosIn, yPosIn);
}

bool ParaEngine::CHTMLBrowser::mouseUp( int xPosIn, int yPosIn )
{
	return m_manager->GetInterface()->mouseUp(m_nBrowserWindowId,xPosIn, yPosIn);
}

bool ParaEngine::CHTMLBrowser::mouseMove( int xPosIn, int yPosIn )
{
	return m_manager->GetInterface()->mouseMove(m_nBrowserWindowId,xPosIn, yPosIn);
}

bool ParaEngine::CHTMLBrowser::keyPress( int keyCodeIn )
{
	return m_manager->GetInterface()->keyPress(m_nBrowserWindowId,keyCodeIn);
}

bool ParaEngine::CHTMLBrowser::focusBrowser( bool focusBrowserIn )
{
	if(focusBrowserIn)
	{
		CGlobals::GetGUI()->UseDefaultMouseCursor(true);
	}
	bool bRes = m_manager->GetInterface()->focusBrowser(m_nBrowserWindowId,focusBrowserIn);

	if(!focusBrowserIn)
	{
		CGlobals::GetGUI()->UseDefaultMouseCursor(false);

		// LXZ 2008.4.21: this is fixed, we shall never call SetCapture or focus as below, instead the scripting interface needs to call ParaUI.SetUseSystemCursor() when mouse is insidea browser window. 
		// when a browser loses focus, we will just let the game main window have the key and mouse focus. 
		//::SetCapture(CGlobals::GetAppHWND());
		//::SetFocus(CGlobals::GetAppHWND());
	}
	return bRes;
}

void ParaEngine::CHTMLBrowser::setNoFollowScheme( std::string schemeIn )
{
	return m_manager->GetInterface()->setNoFollowScheme(m_nBrowserWindowId,schemeIn);
}

std::string ParaEngine::CHTMLBrowser::getNoFollowScheme()
{
	return m_manager->GetInterface()->getNoFollowScheme(m_nBrowserWindowId);
}

LRESULT ParaEngine::CHTMLBrowser::SendMessage( UINT Msg,WPARAM wParam,LPARAM lParam )
{
	return S_OK;
}

LRESULT ParaEngine::CHTMLBrowser::PostMessage( UINT Msg,WPARAM wParam,LPARAM lParam )
{
	return S_OK;
}

const char* ParaEngine::CHTMLBrowser::GetName()
{
	return m_filename.c_str();
}

void ParaEngine::CHTMLBrowser::InvalidateDeviceObjects()
{
	if (m_pTexture)
	{
		m_pTexture->DefRef();

	}
	m_bTextureUpdated = false;
}

IParaEngine::ITexture* ParaEngine::CHTMLBrowser::GetTexture()
{
	m_fLastUsedTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->GetTime());
	if(m_bNeedUpdate || !m_bTextureUpdated)
	{
		//////////////////////////////////////////////////////////////////////////
		//
		// copy from lpPixels to d3d texture
		//
		//////////////////////////////////////////////////////////////////////////

		if(m_pTexture == 0)
		{
			// TODO: create a new texture if the resolution of flash changes.
			CreateTexture(&m_pTexture);
		}
		if(m_pTexture)
		{
			UpdateTexture(m_pTexture);
		}
		else
		{
			// failed creating or updating HTML texture surface, should never get here. 
			OUTPUT_LOG("warning: failed creating or updating HTML texture surface. \n");
			m_bNeedUpdate = false;
			m_bTextureUpdated = true;
			setEnabled(false);
		}
		//OUTPUT_LOG("surface is updated \n\n");
	}
	return m_pTexture;
}

HRESULT ParaEngine::CHTMLBrowser::CreateTexture( IParaEngine::ITexture **ppTexture )
{
	// Create the texture that maps to this media type
	HRESULT hr = E_UNEXPECTED;
	uint32_t uintWidth = 2;
	uint32_t uintHeight = 2;

	// here let's check if we can use dynamic textures
	auto pRenderDevice = CGlobals::GetRenderDevice();
	if(pRenderDevice->GetCaps().DynamicTextures)
	{
		m_bUseDynamicTextures = true;
	}

	int nBufWidth = getBrowserWidth();
	int nBufHeight = getBrowserHeight();

	if(!pRenderDevice->GetCaps().NPOT)
	{
		while( (LONG)uintWidth < nBufWidth )
		{
			uintWidth = uintWidth << 1;
		}
		while( (LONG)uintHeight < nBufHeight )
		{
			uintHeight = uintHeight << 1;
		}
	}
	else
	{
		uintWidth = nBufWidth;
		uintHeight = nBufHeight;
	}

	// use the size in buffer
	if( m_bUseDynamicTextures )
	{

		*ppTexture = pRenderDevice->CreateTexture(uintWidth, uintHeight, EPixelFormat::X8R8G8B8,ETextureUsage::Dynamic);

		if(*ppTexture == nullptr)
		{
			m_bUseDynamicTextures = FALSE;
		}
	}
	else
	{
		*ppTexture = pRenderDevice->CreateTexture(uintWidth, uintHeight, EPixelFormat::X8R8G8B8, ETextureUsage::Default);
	}

	if( FAILED(hr))
	{
		OUTPUT_LOG(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
		return hr;
	}

	// CreateTexture can silently change the parameters on us
	{
		// fill texture with white color
		IParaEngine::ITexture* pTexture = (*ppTexture);
		unsigned char* tempBuffer = new unsigned char[nBufWidth*nBufHeight * 4];
		memset(tempBuffer, 0xff, nBufWidth*nBufHeight * 4);
		pTexture->UpdateImage(0, 0, 0, nBufWidth, nBufHeight, tempBuffer);
		delete[] tempBuffer;

	}

	return S_OK;
}

HRESULT ParaEngine::CHTMLBrowser::UpdateTexture(IParaEngine::ITexture* pTexture )
{
	if(!m_bNeedUpdate && m_bTextureUpdated)
		return S_OK;
	const byte* lpPixels = m_bNeedUpdate ? (const byte*)grabBrowserWindow() : (const byte*)getBrowserWindowPixels(); 
	m_bTextureUpdated = true;
	m_bNeedUpdate = false;
	
	if(pTexture == 0 ||  lpPixels == 0 )
		return E_FAIL;


	int nBufWidth = getBrowserWidth();
	int nBufHeight = getBrowserHeight();

	
	{
		// sometimes the rowspan != width * bytes per pixel (mBrowserWindowWidth)
		int nBrowserRowSpan = getBrowserRowSpan(); // number of bytes per line.
		int nBrowserDepth = getBrowserDepth(); // number of bytes per pixel.

		BYTE* pTextureBits = new BYTE[nBufWidth*nBufHeight * 4];
		uint32_t pitch = nBufWidth * 4;

		if(nBrowserDepth == 3)
		{
			const byte* pBitmapBits = lpPixels;
			BYTE* pLineTextureBits = pTextureBits;
			for (int j = 0; j < nBufHeight; j++)
			{
				DWORD* pPixels = (DWORD*)pLineTextureBits;
				pBitmapBits = lpPixels + nBrowserRowSpan*j;

				for (int i = 0; i < nBufWidth; i++)
				{
					*pPixels = (pBitmapBits[0]&(pBitmapBits[1]<<8)&(pBitmapBits[2]<<16)&0xff000000);
					pPixels++;
					pBitmapBits += 3;
				}
				pLineTextureBits += pitch;
			}
		}
		else
		{
			const DWORD* pBitmapBits = (const DWORD*)lpPixels;
			BYTE* pLineTextureBits = pTextureBits;
			for (int j = 0; j < nBufHeight; j++)
			{
				DWORD* pPixels = (DWORD*)pLineTextureBits;
				pBitmapBits = (const DWORD*)(lpPixels + nBrowserRowSpan*j);

				for (int i = 0; i < nBufWidth; i++)
				{
					*pPixels = *pBitmapBits;
					pPixels++;
					pBitmapBits ++;
				}
				pLineTextureBits += pitch;
			}
		}
		pTexture->UpdateImage(0, 0, 0, nBufWidth, nBufHeight, pTextureBits);
		delete[] pTextureBits;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// implementation of IEmbeddedBrowserWindowObserver interface
// 
//////////////////////////////////////////////////////////////////////////

/**
* generate sCode string: msg={windowid=number, value=[ValueInt|ValueString|nil]}
*/
string GenerateOnBrowserScript(int windowid, int* ValueInt, const char* ValueString)
{
	char ctemp[30];
	string code="msg={windowid=";
	::itoa(windowid,ctemp,10);
	code+=ctemp;code+=",";
	if(ValueInt)
	{
		code+="value=";
		::itoa(windowid,ctemp,10);
		code+=ctemp;code+=",";
	}
	else if(ValueString)
	{
		code+="value=";
		NPL::NPLHelper::EncodeStringInQuotation(code, (int)code.size(), ValueString);
		code+=",";
	}
	code+="};";
	return code;
}

void ParaEngine::CHTMLBrowser::onPageChanged( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// flag that an update is required - page grab happens in idle() so we don't stall
	if(eventIn.getEventWindowId() == m_nBrowserWindowId)
	{
		m_bNeedUpdate = true;
		/*
		// example of how to grab the location of the dirty rectangle
		int x, y, width, height;
		eventIn.getRectValue( x, y, width, height );
		OUTPUT_LOG("HTML onPageChanged %d %d %d %d\n", x, y, width, height);
		*/
		
		if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onPageChanged))
		{
			m_manager->ActivateScript(CHTMLBrowserManager::EM_onPageChanged, GenerateOnBrowserScript(GetBrowserWindowID(), NULL, NULL));
		}
	}
	else
	{
		OUTPUT_LOG("warning: ParaEngine::CHTMLBrowser::onPageChanged 's window id mismatched\n");
	}
}

void ParaEngine::CHTMLBrowser::onNavigateBegin( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// could do something here like start a throbber :)
	//OUTPUT_LOG("HTML onNavigateBegin %s\n", eventIn.getStringValue().c_str());

	if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onNavigateBegin))
		m_manager->ActivateScript(CHTMLBrowserManager::EM_onNavigateBegin, GenerateOnBrowserScript(GetBrowserWindowID(), NULL, eventIn.getStringValue().c_str()));
}

void ParaEngine::CHTMLBrowser::onNavigateComplete( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// could do something here like stop a throbber :)
	if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onNavigateComplete))
		m_manager->ActivateScript(CHTMLBrowserManager::EM_onNavigateComplete, GenerateOnBrowserScript(GetBrowserWindowID(), NULL, eventIn.getStringValue().c_str()));
}

void ParaEngine::CHTMLBrowser::onUpdateProgress( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// observed event - page progress changes
	//OUTPUT_LOG("HTML onUpdateProgress %d\n", eventIn.getIntValue());
	if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onUpdateProgress))
	{
		int nValue = eventIn.getIntValue();
		m_manager->ActivateScript(CHTMLBrowserManager::EM_onUpdateProgress, GenerateOnBrowserScript(GetBrowserWindowID(), &nValue, NULL));
	}
}

void ParaEngine::CHTMLBrowser::onStatusTextChange( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// NOTE: only display the first 100 chars since anything longer breaks the display of percent loaded
	// on the right hand side - normally, you'd want to use the whole thing.
	//OUTPUT_LOG("HTML onStatusTextChange %s\n", eventIn.getStringValue().c_str());
	if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onStatusTextChange))
		m_manager->ActivateScript(CHTMLBrowserManager::EM_onStatusTextChange, GenerateOnBrowserScript(GetBrowserWindowID(), NULL, eventIn.getStringValue().c_str()));
}

void ParaEngine::CHTMLBrowser::onLocationChange( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// observed event - URL location changes - e.g. when a site redirects somewhere else
	// (ought to check that this is the top frame or this will be wrong)
	//OUTPUT_LOG("HTML onLocationChange %s\n", eventIn.getStringValue().c_str());
	if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onLocationChange))
		m_manager->ActivateScript(CHTMLBrowserManager::EM_onLocationChange, GenerateOnBrowserScript(GetBrowserWindowID(), NULL, eventIn.getStringValue().c_str()));
}

void ParaEngine::CHTMLBrowser::onClickLinkHref( const IEmbeddedBrowserWindowObserver::EventType& eventIn )
{
	// this is fired when you click on a link in the browser	
	//OUTPUT_LOG("HTML onClickLinkHref %s\n", eventIn.getStringValue().c_str());
	if(m_manager && m_manager->HasEvent(CHTMLBrowserManager::EM_onClickLinkHref))
		m_manager->ActivateScript(CHTMLBrowserManager::EM_onClickLinkHref, GenerateOnBrowserScript(GetBrowserWindowID(), NULL, eventIn.getStringValue().c_str()));
}

//-----------------------------------------------------------------------------
// Class:	CGUIWebBrowser
// Authors: LiXizhi
// Date:	2012.10.24
// desc: IE activeX control based web browser. Only this sparingly. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#if defined(USE_DIRECTX_RENDERER) && !defined(NPLRUNTIME)
#include "ParaWorldAsset.h"
#include "IParaEngineApp.h"
#include "GUIResource.h"
#include "GUIWebBrowser.h"
#include "GUIRoot.h"
#include "ObjectManager.h"
#include "WebBrowser/WebBrowser.h"
#include "EventBinding.h"
#include "ic/ICConfigManager.h"
#include "util/StringHelper.h"
#include "memdebug.h"

using namespace ParaEngine;
using namespace std;
const IType* CGUIWebBrowser::m_type=NULL;
std::list<CGUIWebBrowser*> ParaEngine::CGUIWebBrowser::g_active_browsers;




/** @def custom user messages that are posted from the main thread to the window thread. */
#define PE_WM_WEBBROWSER_FIRST					WM_USER+2350
#define PE_WM_WEBBROWSER_LOADURL				WM_USER+2351
#define PE_WM_WEBBROWSER_ONSIZE_CHANGED			WM_USER+2352
#define PE_WM_WEBBROWSER_SETCAPTURE				WM_USER+2354
#define PE_WM_WEBBROWSER_RELEASECAPTURE			WM_USER+2355
#define PE_WM_WEBBROWSER_SETFOCUS				WM_USER+2356
#define PE_WM_WEBBROWSER_QUIT					WM_USER+2357
#define PE_WM_WEBBROWSER_CREATE_WINDOW			WM_USER+2359
#define PE_WM_WEBBROWSER_REFRESH				WM_USER+2360
#define PE_WM_WEBBROWSER_STOP					WM_USER+2361
#define PE_WM_WEBBROWSER_GOBACK					WM_USER+2362
#define PE_WM_WEBBROWSER_GOFORWARD				WM_USER+2363
#define PE_WM_WEBBROWSER_SETCONTENT				WM_USER+2364
#define PE_WM_WEBBROWSER_GETCONTENT				WM_USER+2365
#define PE_WM_WEBBROWSER_FINDTEXT				WM_USER+2366
#define PE_WM_WEBBROWSER_LAST					WM_USER+2367


CGUIWebBrowser::CGUIWebBrowser(void):CGUIBase(), m_hBrowserHostWnd(NULL), m_nWindowID(0), m_bAutoSize(TRUE), m_pWebBrowser(NULL), m_hBrowserWnd(NULL),
	m_nWndX(0), m_nWndY(0), m_nWndWidth(100), m_nWndHeight(100), m_bIsBrowserVisible(false), m_bHasContent(false), m_bWindowCreated(false), m_bHasOnChangeEvent(false), m_nLastFindTextResult(0)
{
	if (!m_type){
		m_type=IType::GetType("guiwebbrowser");
	}
}

CGUIWebBrowser::~CGUIWebBrowser(void)
{
	g_active_browsers.remove(this);
	
	if(m_hBrowserHostWnd != NULL)
	{
		if(m_win_thread && m_dwWinThreadID!=0)
		{
			PostWinThreadMessage(PE_WM_WEBBROWSER_QUIT, 0,0);
			m_win_thread->join();
		}
		OUTPUT_LOG("Web browser window %d is destroyed", m_hBrowserHostWnd);
		m_hBrowserHostWnd = NULL;
	}
}

void CGUIWebBrowser::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CGUIWebBrowser* pText=(CGUIWebBrowser*)pobj;
	CGUIBase::Clone(pText);
}

IObject *CGUIWebBrowser::Clone()const 
{
	CGUIWebBrowser *pText=new CGUIWebBrowser();
	Clone(pText);
	return pText;
}

void CGUIWebBrowser::StaticInit()
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUIWebBrowser *pText=new CGUIWebBrowser();
	pOm->CloneObject("default_CGUIBase",pText);

	pText->m_szText.clear();
	CEventBinding* pBinding=pText->m_event->GetEventBindingObj();
	pBinding->DisableKeyboard();
	pBinding->DisableMouse();

	pOm->SetObject("default_CGUIWebBrowser",pText);
	SAFE_RELEASE(pText);
}

void CGUIWebBrowser::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIWebBrowser")) {
		CGUIWebBrowser::StaticInit();
	}
	pOm->CloneObject("default_CGUIWebBrowser",this);
	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);
}

void CGUIWebBrowser::SetColor(DWORD nColor)
{
	m_objResource->SetFontColor((Color)(nColor));
}

void CGUIWebBrowser::SetTransparency(DWORD transparency)
{
	m_objResource->SetFontTransparency(transparency);
}

void CGUIWebBrowser::SetTextA(const char* szText)
{
	PE_ASSERT( szText != NULL );
	u16string out;
	StringHelper::UTF8ToUTF16_Safe(szText, out);
	SetText(out.c_str());
}

void CGUIWebBrowser::SetText(const char16_t* wszText)
{
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);
		PE_ASSERT( wszText != NULL );
		m_szText=wszText;
		StringHelper::UTF16ToUTF8(m_szText, m_szUTF8_Text);
	}

	if(!m_szText.empty())
	{
		if(m_bWindowCreated && m_hBrowserHostWnd!=0)
		{
			if(PostWinThreadMessage(PE_WM_WEBBROWSER_LOADURL, 1, 0))
			{
			}
		}
		else
		{
			CreateGetBrowser();
		}
	}
}

int CGUIWebBrowser::GetTextA(std::string& out) 
{
	return StringHelper::UTF16ToUTF8(m_szText, out) ? (int) out.size() : 0;
}

const char16_t* ParaEngine::CGUIWebBrowser::GetText()
{
	if(m_pWebBrowser)
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);
		if(!m_szCurrentURL.empty() && m_szCurrentURL!=m_szUTF8_Text)
		{
			m_szUTF8_Text = m_szCurrentURL;
			StringHelper::UTF8ToUTF16_Safe(m_szUTF8_Text, m_szText);
		}
	}
	return m_szText.c_str();
}

//-----------------------------------------------------------------------------------
// name: Render
/// desc: 
/// if pGUIState->bIsBatchRender is false, then Add to batch rendering list
/// otherwise, render the text with sprite. It is up to the caller to ensure that
/// both the font and the sprite object used to render the text are valid.
//-----------------------------------------------------------------------------------
HRESULT CGUIWebBrowser::Render(GUIState* pGUIState,float fElapsedTime )
{
	HRESULT hr = S_OK;
	SetDirty(false);
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	
	RECT rcWindow = m_objResource->GetDrawingRects(0);
	RECT tempRect = rcWindow;

	if(m_hBrowserHostWnd!=0 && m_pWebBrowser)
	{
		int x = rcWindow.left;
		int y = rcWindow.top;
		int width = RectWidth(rcWindow); 
		int height = RectHeight(rcWindow);

		CGlobals::GetApp()->GameToClient(x, y);
		CGlobals::GetApp()->GameToClient(width, height);

		float fScaleX = 1.f;
		float fScaleY = 1.f;
		CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);

		if(fScaleX!=1.f)
		{
			x  = (int)((float)x * fScaleX);
			width = (int)((float)width * fScaleX);
		}

		if(fScaleY!=1.f)
		{
			y  = (int)((float)y * fScaleY);
			height = (int)((float)height * fScaleY);
		}

		if( !(x == m_nWndX && y == m_nWndY && 
			width == m_nWndWidth && width == m_nWndHeight) ) 
		{
			MoveWindow(x, y, width, height);
		}
	}

	ShowBrowser(true);
	
	OnFrameMove(fElapsedTime);
	return hr;
}

void ParaEngine::CGUIWebBrowser::MoveWindow( int x, int y, int width, int height, bool bRepaint/*=true*/ )
{
	if(m_hBrowserHostWnd!=0 && m_pWebBrowser)
	{
		m_nWndX = x;
		m_nWndY = y;
		m_nWndWidth = width;
		m_nWndHeight = height;

		::MoveWindow(m_hBrowserHostWnd, x, y, width, height, TRUE);
		PostWinThreadMessage(PE_WM_WEBBROWSER_ONSIZE_CHANGED, width,height);
	}
}

HRESULT CGUIWebBrowser::RenderInBatch(GUIState* pGUIState)
{
	HRESULT hr = S_OK;
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	RECT rcWindow = m_objResource->GetDrawingRects(0);
	RECT tempRect = rcWindow;

	// DrawElement(m_objResource->GetTextureElement(0),&tempRect,&rcWindow);
	return hr;
}

void CGUIWebBrowser::UpdateRects()
{
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos,&m_position);
	if(m_position.IsRelativeTo3DObject())
	{
		SetDepth(tempPos.GetDepth());
	}

	RECT rcInner = tempPos.rect;
	m_objResource->SetDrawingRects(&tempPos.rect,0);

	m_bNeedUpdate=false;
}

WebBrowser* ParaEngine::CGUIWebBrowser::CreateGetBrowser()
{
	if(m_hBrowserHostWnd == NULL)
	{
		StartWindowThread();
	}
	return m_pWebBrowser;
}

// this function is called in win thread
void ParaEngine::CGUIWebBrowser::OnWebPageLoaded()
{
	m_bHasContent = true;
	if(m_pWebBrowser)
	{
		// update the url. 
		ParaEngine::Lock lock_(m_win_thread_mutex);
		m_szCurrentURL = m_pWebBrowser->get_LocationURL();

		// signal the onchange event
		m_bHasOnChangeEvent = true;
	}
}

HRESULT ParaEngine::CGUIWebBrowser::StartWindowThread()
{
	if(m_win_thread.get()==0)
	{
		// start the io thread
		m_win_thread.reset(new boost::thread(boost::bind(&CGUIWebBrowser::DefaultWinThreadProc, this)));
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

void ParaEngine::CGUIWebBrowser::DefaultWinThreadProc()
{
	CoInitialize(NULL);
	//
	// create a window to render into. Register the windows class
	// 
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);

		m_hBrowserHostWnd  = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"static", NULL, WS_CHILD,
			m_nWndX, m_nWndY, m_nWndWidth, m_nWndHeight, 
			CGlobals::GetAppHWND(), (HMENU)GetID(), CGlobals::GetApp()->GetModuleHandle(), NULL);

		if(m_hBrowserHostWnd != 0)
		{
			m_dwWinThreadID = ::GetWindowThreadProcessId(m_hBrowserHostWnd, NULL);

			// inform window creation
			m_bWindowCreated = true;
			PostWinThreadMessage(PE_WM_WEBBROWSER_CREATE_WINDOW, 1, 0);
		}
		else
		{
			OUTPUT_LOG("error: CreateWindow web browser failed\n");
			m_bWindowCreated = true;
			return;
		}
	}
	
	//
	// Dispatching window messages in this window thread. 
	//

	// Now we're ready to receive and process Windows messages.
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessageW( &msg, NULL, 0U, 0U, PM_NOREMOVE );

	while( WM_QUIT != msg.message  )
	{
		if( GetMessageW( &msg, NULL, 0U, 0U ) != 0 )
		{
			if(MsgProcCustom(msg.message, msg.wParam, msg.lParam) == 0)
			{
				// Translate and dispatch the message
				TranslateMessage( &msg );
				DispatchMessageW( &msg );
			}
		}
	}
	OUTPUT_LOG("web browser window thread exit\n");
	m_dwWinThreadID = 0;
}

bool ParaEngine::CGUIWebBrowser::MsgProcCustom( UINT message,WPARAM wParam,LPARAM lParam )
{
	if (message>=PE_WM_WEBBROWSER_FIRST && message<=PE_WM_WEBBROWSER_LAST)
	{
		switch (message)
		{
		case PE_WM_WEBBROWSER_CREATE_WINDOW:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				
				if(m_hBrowserHostWnd != 0 && m_pWebBrowser == 0)
				{
					OUTPUT_LOG("web browser window thread started. and a window is created in it\n");

					m_pWebBrowser = new WebBrowser(m_hBrowserHostWnd,this);
					if(m_pWebBrowser)
					{
						if(!m_szUTF8_Text.empty())
						{
							m_pWebBrowser->ShowHTMLPage(m_szUTF8_Text.c_str());
						}
					}
				}
				break;
			}
		case PE_WM_WEBBROWSER_LOADURL:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->ShowHTMLPage(m_szUTF8_Text.c_str());
				}
				break;
			}
		case PE_WM_WEBBROWSER_REFRESH:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->Refresh();
				}
				break;
			}
		case PE_WM_WEBBROWSER_STOP:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->Stop();
				}
				break;
			}
		case PE_WM_WEBBROWSER_GOBACK:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->GoBack();
				}
				break;
			}
		case PE_WM_WEBBROWSER_GOFORWARD:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->GoForward();
				}
				break;
			}
		case PE_WM_WEBBROWSER_ONSIZE_CHANGED:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					/*RECT rect;
					GetClientRect(m_hBrowserHostWnd, &rect);*/
					HWND hWnd = ::GetWindow(m_hBrowserHostWnd, GW_CHILD);
					if(hWnd!=0)
					{
						::MoveWindow(hWnd,  0,0, (int)wParam, (int)lParam, TRUE);
					}
					// m_pWebBrowser->ChangeWindowSize((int)wParam, (int)lParam);
				}
				break;
			}
		case PE_WM_WEBBROWSER_FINDTEXT:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					if(m_pWebBrowser->FindText(m_sLastFindText)){
						m_nLastFindTextResult = 1;
					}
					else{
						m_nLastFindTextResult = 0;
					}
				}
				break;
			}
		case PE_WM_WEBBROWSER_SETCONTENT:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->WriteContent(m_sLastWriteContent);
				}
				break;
			}
		case PE_WM_WEBBROWSER_GETCONTENT:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(m_pWebBrowser)
				{
					m_pWebBrowser->GetContent(m_sLastGetContent);
					if(m_sLastGetContent.empty()){
						m_sLastGetContent = "empty";
					}
				}
				break;
			}
		case PE_WM_WEBBROWSER_QUIT:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				OUTPUT_LOG("PE_WM_WEBBROWSER_QUIT begins \n ");
				SAFE_DELETE(m_pWebBrowser);
				OUTPUT_LOG("browser destroyed \n ");

				if(! DestroyWindow( m_hBrowserHostWnd ))
				{
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

					OUTPUT_LOG("failed to destroy browser window because %d, %s\n", dw, lpMsgBuf);
				}

				OUTPUT_LOG("PE_WM_WEBBROWSER_QUIT ends \n ");
				PostQuitMessage(wParam);
				break;
			}
		default:
			break;
		}
		return 1;
	}
	return 0;
}

bool ParaEngine::CGUIWebBrowser::PostWinThreadMessage( UINT message,WPARAM wParam,LPARAM lParam )
{
	if(m_dwWinThreadID!=0)
	{
		return !!::PostThreadMessage(m_dwWinThreadID, message, wParam, lParam);
	}
	return false;
}


bool ParaEngine::CGUIWebBrowser::OnChange( const char* code/*=NULL*/ )
{
	if(m_bHasOnChangeEvent)
	{
		{
			ParaEngine::Lock lock_(m_win_thread_mutex);
			m_bHasOnChangeEvent = false;
		}
		return CGUIBase::OnChange();
	}
	return true;
}

void ParaEngine::CGUIWebBrowser::GlobalFrameMove()
{
	for (std::list<CGUIWebBrowser*>::iterator it=g_active_browsers.begin(); it!=g_active_browsers.end(); it++)
	{
		CGUIWebBrowser* pBrowser = (*it);
		if( pBrowser->m_bIsBrowserVisible && ! pBrowser->GetVisibleRecursive() )
		{
			pBrowser->ShowBrowser(false);
		}
		// broadcast the on change event
		if(pBrowser->m_bHasOnChangeEvent)
		{
			pBrowser->OnChange();
		}
	}
}

void ParaEngine::CGUIWebBrowser::ShowBrowser( bool bShow /*= true*/ )
{
	if(bShow)
	{
		if(m_bHasContent && !m_bIsBrowserVisible)
		{
			if(m_hBrowserHostWnd!=0)
			{
				m_bIsBrowserVisible = true;
				// add WS_CLIPCHILDREN to the parent window. 
				SetWindowLong(CGlobals::GetAppHWND(), GWL_STYLE, GetWindowLong(CGlobals::GetAppHWND(), GWL_STYLE) | WS_CLIPCHILDREN);
				::ShowWindow(m_hBrowserHostWnd,SW_SHOW);
				g_active_browsers.remove(this);
				g_active_browsers.push_back(this);
			}
		}
	}
	else
	{
		if(m_hBrowserHostWnd!=0)
		{
			if(m_bIsBrowserVisible)
			{
				m_bIsBrowserVisible = false;
				::ShowWindow(m_hBrowserHostWnd,SW_HIDE);
			}
		}
	}
}

void ParaEngine::CGUIWebBrowser::Refresh()
{
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_REFRESH, 0,0);
	}
}

void ParaEngine::CGUIWebBrowser::GoBack()
{
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_GOBACK, 0,0);
	}
}

void ParaEngine::CGUIWebBrowser::GoForward()
{
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_GOFORWARD, 0,0);
	}
}

void ParaEngine::CGUIWebBrowser::Stop()
{
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_STOP, 0,0);
	}
}


bool ParaEngine::CGUIWebBrowser::FindText( const std::string& sFindText )
{
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);
		m_sLastFindText = sFindText;
		m_nLastFindTextResult = -1;
	}
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_FINDTEXT, 0,0);
	}
	return false;
}

void ParaEngine::CGUIWebBrowser::SetContent( const std::string& sContent )
{
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);
		m_sLastWriteContent = sContent;
	}
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_SETCONTENT, 0,0);
	}
}


const std::string& ParaEngine::CGUIWebBrowser::GetContent()
{
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);
		m_sLastGetContent.clear();
	}
	if(m_hBrowserHostWnd != NULL && m_win_thread)
	{
		PostWinThreadMessage(PE_WM_WEBBROWSER_GETCONTENT, 0,0);
		// fetch for 1 seconds
		for (int i=0;i<10;i++)
		{
			::Sleep(100);
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				if(!m_sLastGetContent.empty())
				{
					m_sContent = m_sLastGetContent;
					break;
				}
			}
		}
	}
	return m_sContent;
}



int  ParaEngine::CGUIWebBrowser::GetLastFindTextResult()
{
	ParaEngine::Lock lock_(m_win_thread_mutex);
	int nResult = m_nLastFindTextResult;
	return nResult;
}

int ParaEngine::CGUIWebBrowser::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CGUIBase::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass!=NULL);
	pClass->AddField("Refresh",FieldType_void, Refresh_s, NULL, NULL, "", bOverride);
	pClass->AddField("Stop",FieldType_void, Stop_s, NULL, NULL, "", bOverride);
	pClass->AddField("GoBack",FieldType_void, GoBack_s, NULL, NULL, "", bOverride);
	pClass->AddField("GoForward",FieldType_void, GoForward_s, NULL, NULL, "", bOverride);

	pClass->AddField("Content",FieldType_String, SetContent_s, GetContent_s, NULL, "", bOverride);
	pClass->AddField("FindText",FieldType_String, SetFindText_s, NULL, NULL, "", bOverride);
	pClass->AddField("FindTextResult",FieldType_Int, GetFindTextResult_s, NULL, NULL, "", bOverride);
	return S_OK;
}

#endif
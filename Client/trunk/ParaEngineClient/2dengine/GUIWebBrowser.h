#pragma once
#include "GUIBase.h"
#include <string>
#include <list>
#include "WebBrowser/IBrowserMsgListener.h"
#include "util/mutex.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace ParaEngine
{
class WebBrowser;
struct SpriteFontEntity;
struct AssetEntity;
/**
* This class represents a string inside a rectangular box in the screen coordinates
* Multiple Type_GUIWebBrowser object with the same font are batched together at rendering time
* So it is important that you are not rendering them in the z-ordering.
* This class is suitable of rendering large blocks of text, or multiple short text
* with the same font. It's good practice to keep the text instance as few as possible,
* however, the Type_GUIWebBrowser has been extremely optimized so that the only overhead of having
* many instances of this class is in the data structure keeping. There will be NO
* or very few texture swapping during batched rendering.
* 
* You can dynamically create and discard many instances of this object during frame
* rendering. Only simple data structure is recreated, not any D3D device objects.  
*/
class CGUIWebBrowser :public CGUIBase, IBrowserMsgListener
{
public:
	typedef boost::shared_ptr<boost::thread> Boost_Thread_ptr_type;

	//////////////////////////////////////////////////////////////////////////
	// implementation of IAttributeFields

	/** attribute class ID should be identical, unless one knows how overriding rules work.*/
	virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CGUIWebBrowser;}
	/** a static string, describing the attribute class object's name */
	virtual const char* GetAttributeClassName(){static const char name[] = "CGUIWebBrowser"; return name;}
	/** a static string, describing the attribute class object */
	virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
	/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
	virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

	ATTRIBUTE_METHOD(CGUIWebBrowser, Refresh_s)	{cls->Refresh();return S_OK;}
	ATTRIBUTE_METHOD(CGUIWebBrowser, GoBack_s)	{cls->GoBack();return S_OK;}
	ATTRIBUTE_METHOD(CGUIWebBrowser, GoForward_s)	{cls->GoForward();return S_OK;}
	ATTRIBUTE_METHOD(CGUIWebBrowser, Stop_s)	{cls->Stop();return S_OK;}
	
	ATTRIBUTE_METHOD1(CGUIWebBrowser, GetContent_s, const char**)	{*p1 = cls->GetContent().c_str(); return S_OK;}
	ATTRIBUTE_METHOD1(CGUIWebBrowser, SetContent_s, const char*)	{cls->SetContent(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CGUIWebBrowser, SetFindText_s, const char*)	{cls->FindText(p1); return S_OK;}

	ATTRIBUTE_METHOD1(CGUIWebBrowser, GetFindTextResult_s, int*)	{*p1 = cls->GetLastFindTextResult(); return S_OK;}


	/** a static frame move. it just hide the browser window when it is not rendered. */
	static void GlobalFrameMove();

	/**
	* Set the text of this object
	* @param szText 
	*/
	virtual void SetTextA(const char* szText);
	/**
	* Set the text of this object
	* The control internally store everything in Unicode.
	* @param wszText 
	*/
	virtual void SetText(const char16_t* wszText);
	
	/**
	* get the text of the control
	* @param szText [out] buffer to receive the text
	* @param nLength size of the input buffer
	* @return return the number of bytes written to the buffer. If nLength is 0, this function returns the total number of characters of the text
	*/
	virtual int GetTextA(std::string& out);
	/**
	* get the text of the control 
	* The control internally store everything in Unicode. So it is faster this version than the ascii version.
	* @return 
	*/
	virtual const char16_t* GetText();

	/** find a given text*/
	bool FindText(const std::string& sFindText);
	/** get the last find text result. 1 is found, 0 is not found, -1 is finding.  */
	int GetLastFindTextResult();

	/** Set user specified content to web browser */
	void SetContent(const std::string& sContent);
	/** get the current html content */
	const std::string& GetContent();

	void SetColor(DWORD color);
	void SetTransparency(DWORD transparency);

	/** refresh the web page*/
	void Refresh();
	void GoBack();
	void GoForward();
	void Stop();

	virtual void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
	virtual void UpdateRects();
	virtual HRESULT Render(GUIState* pGUIState ,float fElapsedTime);	
	/** this function is similar to render(), except that it assume that all states have been set properly*/
	virtual HRESULT RenderInBatch(GUIState* pGUIState);
	virtual bool OnChange(const char* code=NULL);

	virtual void Clone(IObject* pobj)const;
	virtual IObject* Clone()const;
	virtual const IType* GetType()const{return m_type;}

	static void		StaticInit();
	CGUIWebBrowser(void);
	virtual ~CGUIWebBrowser(void);

	virtual void OnWebPageLoaded();

	/** create get the browser hwnd */
	WebBrowser* CreateGetBrowser();

public:
	/** create the default win32 windows for rendering. and assigning to m_hWnd. 
	* window is created in another thread. Since windows uses single threaded apartment, the window procedure should also be in that thread. 
	*/
	HRESULT StartWindowThread();
	void DefaultWinThreadProc();

	/** process WM_USER messages*/
	bool MsgProcCustom(UINT message,WPARAM wParam,LPARAM lParam);

	/** post a WM_USER message to the window thread. */
	bool PostWinThreadMessage(UINT message,WPARAM wParam,LPARAM lParam);

	/** move or resize the window. */
	void MoveWindow(int x, int y, int width, int height, bool bRepaint=true);

	/** show browser */
	void ShowBrowser(bool bShow = true);
protected:
	static const IType* m_type;
	/// all active browsers that has already created window
	static std::list<CGUIWebBrowser*> g_active_browsers;
	/// Text to output to screen
	std::u16string		m_szText;
	std::string		m_szUTF8_Text;
	/// current url updated automatically between page load.
	std::string		m_szCurrentURL;	
	bool	m_bAutoSize;
	HWND	m_hBrowserHostWnd;
	HWND	m_hBrowserWnd;
	int		m_nWindowID;
	WebBrowser* m_pWebBrowser;
	int m_nWndX, m_nWndY, m_nWndWidth, m_nWndHeight;
	/// whether the browser has content to show now
	bool	m_bHasContent;
	bool	m_bIsBrowserVisible;
	bool	m_bHasOnChangeEvent;

	//  the default window thread ptr. 
	Boost_Thread_ptr_type m_win_thread;
	ParaEngine::mutex	  m_win_thread_mutex;
	bool m_bWindowCreated;
	int m_nLastFindTextResult;
	std::string m_sLastFindText;
	std::string m_sLastWriteContent;

	std::string m_sLastGetContent;
	std::string m_sContent;

	DWORD m_dwWinThreadID;
};
}
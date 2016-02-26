//----------------------------------------------------------------------
// ParaScriptingGUI
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2005.9.8
// Desc: 
// Containing the interface between script and the engine. 
//----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "HTMLBrowserManager.h"
#include "ParaWorldAsset.h"
#include "ParaScriptingHTMLBrowser.h"

using namespace ParaScripting;

//////////////////////////////////////////////////////////////////////////
//
// ParaHTMLBrowser
//
//////////////////////////////////////////////////////////////////////////

bool ParaHTMLBrowser::IsValid()
{
	return (m_nID > 0) && (CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID) != 0);
}

void ParaHTMLBrowser::Release()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		pBrowser->Release();
	}
}

bool ParaHTMLBrowser::setSize(int widthIn, int heightIn)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->setSize(widthIn, heightIn);
	}
	return false;
}

int ParaHTMLBrowser::getBrowserWidth()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->getBrowserWidth();
	}
	return 0;
}

int ParaHTMLBrowser::getBrowserHeight()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->getBrowserHeight();
	}
	return false;
}

bool ParaHTMLBrowser::scrollByLines(int linesIn)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->scrollByLines(linesIn);
	}
	return false;
}

bool ParaHTMLBrowser::setBackgroundColor(int red, int green, int blue)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->setBackgroundColor(red, green, blue);
	}
	return false;
}

bool ParaHTMLBrowser::setEnabled(bool enabledIn)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->setEnabled(enabledIn);
	}
	return false;
}

bool ParaHTMLBrowser::navigateTo(const char* url)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->navigateTo(url);
	}
	return false;
}

bool ParaHTMLBrowser::navigateStop()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->navigateStop();
	}
	return false;
}

bool ParaHTMLBrowser::canNavigateBack()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->canNavigateBack();
	}
	return false;
}

bool ParaHTMLBrowser::navigateBack()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->navigateBack();
	}
	return false;
}

bool ParaHTMLBrowser::canNavigateForward()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->canNavigateForward();
	}
	return false;
}

bool ParaHTMLBrowser::navigateForward()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->navigateForward();
	}
	return false;
}

bool ParaHTMLBrowser::mouseDown(int xPos, int yPos)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->mouseDown(xPos, yPos);
	}
	return false;
}

bool ParaHTMLBrowser::mouseUp(int xPos, int yPos)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->mouseUp(xPos, yPos);
	}
	return false;
}

bool ParaHTMLBrowser::mouseMove(int xPos, int yPos)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->mouseMove(xPos, yPos);
	}
	return false;
}

bool ParaHTMLBrowser::keyPress(int keyCode)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->keyPress(keyCode);
	}
	return false;
}

bool ParaHTMLBrowser::focusBrowser(bool focusBrowser)
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->focusBrowser(focusBrowser);
	}
	return false;
}

const char* ParaHTMLBrowser::GetName()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->GetName();
	}
	return NULL;
}

const char* ParaHTMLBrowser::GetLastNavURL()
{
	CHTMLBrowser* pBrowser = CGlobals::GetAssetManager()->GetHTMLBrowserManager().GetBrowserWindow(m_nID);
	if (pBrowser)
	{
		return pBrowser->GetLastNavURL().c_str();
	}
	return NULL;
}
#endif
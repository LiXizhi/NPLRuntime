//----------------------------------------------------------------------
// Class: IME delegate
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2014.9.24
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIIMEDelegate.h"

#ifdef PARAENGINE_MOBILE
#include "platform/OpenGLWrapper.h"
USING_NS_CC;

using namespace ParaEngine;

namespace ParaEngine
{
	/* this is just a proxy class to wrapper around the cocos' IMEDelegate, 
	without needing the main header file to include or depends on the cocos' IME related header files. */
	class CIMEDelegateProxy : public IMEDelegate
	{
	public:
		friend class GUIIMEDelegate;
		CIMEDelegateProxy(GUIIMEDelegate* pProxyTarget) :m_pProxyTarget(pProxyTarget){};

		virtual void insertText(const char * text, size_t len)
		{
			m_pProxyTarget->insertText(text, len);
		}

		virtual void deleteBackward()
		{
			m_pProxyTarget->deleteBackward();
		}

		virtual const std::string& getContentText()
		{
			return m_pProxyTarget->getContentText();
		}

		virtual bool canAttachWithIME()
		{
			return m_pProxyTarget->canAttachWithIME();
		}

		virtual void didAttachWithIME()
		{
			return m_pProxyTarget->didAttachWithIME();
		}

		virtual bool canDetachWithIME()
		{
			return m_pProxyTarget->canDetachWithIME();
		}

		virtual void didDetachWithIME()
		{
			return m_pProxyTarget->didDetachWithIME();
		}

	protected:
		GUIIMEDelegate* m_pProxyTarget;
	};
}

ParaEngine::GUIIMEDelegate::GUIIMEDelegate()
	:m_delegate(new CIMEDelegateProxy(this))
{
	
}

ParaEngine::GUIIMEDelegate::~GUIIMEDelegate()
{
	
}
CIMEDelegateProxy* ParaEngine::GUIIMEDelegate::GetIMEDelegateProxy()
{
	return m_delegate.get();
}

bool ParaEngine::GUIIMEDelegate::attachWithIME()
{
	bool ret = m_delegate->attachWithIME();
	if (ret)
	{
		// open keyboard
		GLView * pGlView = Director::getInstance()->getOpenGLView();
		if (pGlView)
		{
			pGlView->setIMEKeyboardState(true);
		}
	}
	return ret;
}

bool ParaEngine::GUIIMEDelegate::detachWithIME()
{
	bool ret = m_delegate->detachWithIME();
	if (ret)
	{
		// close keyboard
		GLView * glView = Director::getInstance()->getOpenGLView();
		if (glView)
		{
			glView->setIMEKeyboardState(false);
		}
	}
	return ret;
}

#else
ParaEngine::GUIIMEDelegate::GUIIMEDelegate()
{
}

bool ParaEngine::GUIIMEDelegate::attachWithIME()
{
	return false;
}

bool ParaEngine::GUIIMEDelegate::detachWithIME()
{
	return false;
}

ParaEngine::GUIIMEDelegate::~GUIIMEDelegate()
{

}

ParaEngine::CIMEDelegateProxy* ParaEngine::GUIIMEDelegate::GetIMEDelegateProxy()
{
	return NULL;
}

#endif
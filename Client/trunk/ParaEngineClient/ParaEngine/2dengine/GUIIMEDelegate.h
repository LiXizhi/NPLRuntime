#pragma once

namespace ParaEngine
{
	class CIMEDelegateProxy;

	/** base interface for cross-platform IME and also null implementation under win32 client. 
	* this interface is similar to IMEDelegate.h in cocos2dx. 
	*/
	class GUIIMEDelegate
	{
	public:
		virtual ~GUIIMEDelegate();
		virtual bool attachWithIME();
		virtual bool detachWithIME();
		
	protected:
		/**
		@brief    Decide if the delegate instance is ready to receive an IME message.
		Called by IMEDispatcher.
		*/
		virtual bool canAttachWithIME() { return false; }

		/**
		@brief    When the delegate detaches from the IME, this method is called by IMEDispatcher.
		*/
		virtual void didAttachWithIME() {}

		/**
		@brief    Decide if the delegate instance can stop receiving IME messages.
		*/
		virtual bool canDetachWithIME() { return false; }

		/**
		@brief    When the delegate detaches from the IME, this method is called by IMEDispatcher.
		*/
		virtual void didDetachWithIME() {}

		/**
		@brief    Called by IMEDispatcher when text input received from the IME.
		*/
		virtual void insertText(const char * text, size_t len) { }

		/**
		@brief    Called by IMEDispatcher after the user clicks the backward key.
		*/
		virtual void deleteBackward() {}

		/**
		@brief    Called by IMEDispatcher for text stored in delegate.
		*/
		virtual const std::string& getContentText() { return CGlobals::GetString(); }

	protected:
		GUIIMEDelegate();

		friend class CIMEDelegateProxy;
		CIMEDelegateProxy* GetIMEDelegateProxy();
#ifdef PARAENGINE_MOBILE
		std::unique_ptr<CIMEDelegateProxy> m_delegate;
#endif
	};

}
	



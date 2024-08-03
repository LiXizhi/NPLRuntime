#pragma once

// #ifdef PARAENGINE_MOBILE
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)

#include "Globals.h"
#include <boost/core/noncopyable.hpp>

namespace ParaEngine {

	class GUIIMEDelegate;

	class GUIIMEDispatcher : 
		private boost::noncopyable
	{
	protected:
		GUIIMEDispatcher();
	public:
		~GUIIMEDispatcher();

		static GUIIMEDispatcher& GetInstance();

		/**
		 * @brief Dispatches the input text from IME.
		 * @lua NA
		 */
		void dispatchInsertText(const std::u16string& text);
		void dispatchInsertText(const std::string& text);

		/**
		 * @brief Dispatches the delete-backward operation.
		 * @lua NA
		 */
		void dispatchDeleteBackward();

		/**
		 * @brief Get the content text from IMEDelegate, retrieved previously from IME.
		 * @lua NA
		 */
		const std::string& getContentText();
		const std::u16string& getContentUTF16Text();

		/**
		*@brief Returns if any delegate is attached with IME.
		*/
		bool isAnyDelegateAttachedWithIME() const;

	protected:
		friend class GUIIMEDelegate;

		/**
		 *@brief Add delegate to receive IME messages.
		 *@param delegate A instance implements IMEDelegate delegate.
		 */
		void addDelegate(GUIIMEDelegate * delegate);

		/**
		 *@brief Attach the Delegate to the IME.
		 *@param delegate A instance implements IMEDelegate delegate.
		 *@return If the old delegate can detach from the IME, and the new delegate
		 *       can attach to the IME, return true, otherwise false.
		 */
		bool attachDelegateWithIME(GUIIMEDelegate * delegate);

		/**
		* Detach the delegate to the IME
		*@see `attachDelegateWithIME(IMEDelegate*)`
		*@param delegate  A instance implements IMEDelegate delegate.
		*@return Whether the IME is detached or not.
		*/
		bool detachDelegateWithIME(GUIIMEDelegate * delegate);

		/**
		 *@brief Remove the delegate from the delegates which receive IME messages.
		 *@param delegate A instance implements the IMEDelegate delegate.
		 */
		void removeDelegate(GUIIMEDelegate * delegate);

	private:
		class Impl;
		Impl * _impl;


	}; //class GUIIMEDispatcher


	class GUIIMEDelegate
	{
	protected:
		GUIIMEDelegate();

	public:
		virtual ~GUIIMEDelegate();

		virtual bool attachWithIME();

		/**
		 * Determine whether the IME is detached or not.
		 * @js NA
		 * @lua NA
		 */
		virtual bool detachWithIME();

	protected:
		friend class GUIIMEDispatcher;

		/**
		@brief    Decide if the delegate instance is ready to receive an IME message.

		Called by IMEDispatcher.
		* @js NA
		* @lua NA
		*/
		virtual bool canAttachWithIME() { return false; }

		/**
		   @brief    When the delegate detaches from the IME, this method is called by IMEDispatcher.
		   * @js NA
		   * @lua NA
		   */
		virtual void didAttachWithIME() {}

		/**
		@brief    Decide if the delegate instance can stop receiving IME messages.
		* @js NA
		* @lua NA
		*/
		virtual bool canDetachWithIME() { return false; }

		/**
		@brief    When the delegate detaches from the IME, this method is called by IMEDispatcher.
		* @js NA
		* @lua NA
		*/
		virtual void didDetachWithIME() {}

		/**
		@brief    Called by IMEDispatcher when text input received from the IME.
		* @js NA
		* @lua NA
		*/
		virtual void insertText(const std::string& text) {}
		virtual void insertText(const std::u16string& text) {}

		/**
		@brief    Called by IMEDispatcher after the user clicks the backward key.
		* @js NA
		* @lua NA
		*/
		virtual void deleteBackward() {}

		/**
		@brief    Called by IMEDispatcher for text stored in delegate.
		* @js NA
		* @lua NA
		*/
		virtual const std::string& getContentText() { return CGlobals::GetString(0); }
		virtual const std::u16string& getContentUTF16Text() { return CGlobals::GetUTF16String(0); }
	};

} // namespace ParaEngine

#endif // PARAENGINE_MOBILE
// #ifdef PARAENGINE_MOBILE
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)

#include "ParaEngine.h"
#include "GUIIMEDelegate.h"

#include <list>

namespace ParaEngine {

	typedef std::list< GUIIMEDelegate * > DelegateList;
	typedef std::list< GUIIMEDelegate * >::iterator  DelegateIter;

	class GUIIMEDispatcher::Impl
	{
	public:
		Impl()
			: _delegateWithIme(nullptr)
		{
		}

		~Impl()
		{

		}

		DelegateIter findDelegate(GUIIMEDelegate* delegate)
		{
			auto end = _delegateList.end();
			for (auto iter = _delegateList.begin(); iter != end; ++iter)
			{
				if (delegate == *iter)
				{
					return iter;
				}
			}
			return end;
		}

		DelegateList    _delegateList;
		GUIIMEDelegate*  _delegateWithIme;
	};

	GUIIMEDispatcher& GUIIMEDispatcher::GetInstance()
	{
		static GUIIMEDispatcher s_instance;
		return s_instance;
	}

	GUIIMEDispatcher::GUIIMEDispatcher()
		: _impl(new Impl())
	{

	}

	GUIIMEDispatcher::~GUIIMEDispatcher()
	{
		SAFE_DELETE(_impl);
	}

	void GUIIMEDispatcher::dispatchInsertText(const std::u16string& text)
	{
		do
		{
			if (text.empty())
				break;

			// there is no delegate attached to IME
			if (!_impl->_delegateWithIme)
				break;

			_impl->_delegateWithIme->insertText(text);
		} while (0);
	}

	void GUIIMEDispatcher::dispatchInsertText(const std::string& text)
	{
		do
		{
			if (text.empty())
				break;

			// there is no delegate attached to IME
			if (!_impl->_delegateWithIme)
				break;

			_impl->_delegateWithIme->insertText(text);
		} while (0);
	}

	void GUIIMEDispatcher::dispatchDeleteBackward()
	{
		do
		{
			// there is no delegate attached to IME
			if (!_impl->_delegateWithIme)
				break;

			_impl->_delegateWithIme->deleteBackward();
		} while (0);
	}

	const std::string& GUIIMEDispatcher::getContentText()
	{
		if (_impl->_delegateWithIme)
			return _impl->_delegateWithIme->getContentText();

		return CGlobals::GetString(0);
	}

	const std::u16string& GUIIMEDispatcher::getContentUTF16Text()
	{
		if (_impl->_delegateWithIme)
			return _impl->_delegateWithIme->getContentUTF16Text();

		return CGlobals::GetUTF16String(0);
	}

	bool GUIIMEDispatcher::isAnyDelegateAttachedWithIME() const
	{
		return _impl->_delegateWithIme != nullptr;
	}

	void GUIIMEDispatcher::addDelegate(GUIIMEDelegate * delegate)
	{
		if (!delegate || !_impl)
		{
			return;
		}
		if (_impl->_delegateList.end() != _impl->findDelegate(delegate))
		{
			// pDelegate already in list
			return;
		}
		_impl->_delegateList.push_front(delegate);
	}

	bool GUIIMEDispatcher::attachDelegateWithIME(GUIIMEDelegate * delegate)
	{
		bool ret = false;
		do
		{
			if (!delegate)
				break;

			auto end = _impl->_delegateList.end();
			auto iter = _impl->findDelegate(delegate);

			// if pDelegate is not in delegate list, return
			if (iter == end)
				break;

			if (_impl->_delegateWithIme)
			{
				if (_impl->_delegateWithIme != delegate)
				{
					// if old delegate canDetachWithIME return false 
					// or pDelegate canAttachWithIME return false,
					// do nothing.
					if (!_impl->_delegateWithIme->canDetachWithIME() || !delegate->canDetachWithIME())
						break;
					auto oldDelegate = _impl->_delegateWithIme;
					_impl->_delegateWithIme = nullptr;
					oldDelegate->didDetachWithIME();

					_impl->_delegateWithIme = *iter;
					delegate->didAttachWithIME();
				}

				ret = true;
				break;
			}

			// delegate hasn't attached to IME yet
			if (!delegate->canAttachWithIME())
				break;

			_impl->_delegateWithIme = *iter;
			delegate->didAttachWithIME();
			ret = true;

		} while (false);

		return ret;
	}

	bool GUIIMEDispatcher::detachDelegateWithIME(GUIIMEDelegate * delegate)
	{
		bool ret = false;

		do
		{
			if (!delegate)
				break;

			if (_impl->_delegateWithIme != delegate)
				break;

			if (!delegate->canDetachWithIME())
				break;

			_impl->_delegateWithIme = nullptr;
			delegate->didDetachWithIME();
			ret = true;

		} while (false);

		return ret;
	}

	void GUIIMEDispatcher::removeDelegate(GUIIMEDelegate * delegate)
	{
		do
		{
			if (!delegate || !_impl)
				break;

			auto iter = _impl->findDelegate(delegate);
			auto end = _impl->_delegateList.end();

			if (iter == end)
				break;

			if (_impl->_delegateWithIme)
			{
				if (delegate == _impl->_delegateWithIme)
					_impl->_delegateWithIme = nullptr;
			}

			_impl->_delegateList.erase(iter);

		} while (false);
	}

	GUIIMEDelegate::GUIIMEDelegate()
	{
		GUIIMEDispatcher::GetInstance().addDelegate(this);
	}

	GUIIMEDelegate::~GUIIMEDelegate()
	{
		GUIIMEDispatcher::GetInstance().removeDelegate(this);
	}

	bool GUIIMEDelegate::attachWithIME()
	{
		return GUIIMEDispatcher::GetInstance().attachDelegateWithIME(this);
	}

	bool GUIIMEDelegate::detachWithIME()
	{
		return GUIIMEDispatcher::GetInstance().detachDelegateWithIME(this);
	}

}// namespace ParaEngine

#endif // PARAENGINE_MOBILE
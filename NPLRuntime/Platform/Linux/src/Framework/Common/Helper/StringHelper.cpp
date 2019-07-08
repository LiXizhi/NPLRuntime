#include "ParaEngine.h"
#include <boost/thread/tss.hpp>
#include <boost/locale/encoding_utf.hpp>
#include "util/StringHelper.h"
#include "2dengine/GUIEdit.h"

namespace ParaEngine {
	void CGUIEditBox::CopyToClipboard()
	{
#ifdef WIN32
		// Copy the selection text to the clipboard
		if (m_nCaret != m_nSelStart && OpenClipboard(NULL))
		{
			EmptyClipboard();

			HGLOBAL hBlock = GlobalAlloc(GMEM_MOVEABLE, sizeof(WCHAR) * (m_Buffer.GetTextSize() + 1));
			if (hBlock)
			{
				WCHAR *pwszText = (WCHAR*)GlobalLock(hBlock);
				if (pwszText)
				{
					int nFirst = Math::Min(m_nCaret, m_nSelStart);
					int nLast = Math::Max(m_nCaret, m_nSelStart);
					if (nLast - nFirst > 0)
					{
						if (m_PasswordChar == '\0')
						{
							CopyMemory(pwszText, m_Buffer.GetBuffer() + nFirst, (nLast - nFirst) * sizeof(WCHAR));
						}
						else
						{
							for (int i = 0; i < (nLast - nFirst); ++i)
							{
								pwszText[i] = m_PasswordChar;
							}
						}
					}
					pwszText[nLast - nFirst] = '\0';  // Terminate it
					GlobalUnlock(hBlock);
				}
				SetClipboardData(CF_UNICODETEXT, hBlock);
			}
			CloseClipboard();
			// We must not free the object until CloseClipboard is called.
			if (hBlock)
				GlobalFree(hBlock);
		}
#endif
	}

	void CGUIEditBox::PasteFromClipboard()
	{
		DeleteSelectionText();
#ifdef WIN32
		if (OpenClipboard(NULL))
		{
			HANDLE handle = GetClipboardData(CF_UNICODETEXT);
			if (handle)
			{
				// Convert the ANSI string to Unicode, then
				// insert to our buffer.
				char16_t *pwszText = (char16_t*)GlobalLock(handle);
				if (pwszText)
				{
					// Copy all characters up to null.
					if (m_Buffer.InsertString(m_nCaret, pwszText))
						PlaceCaret(m_nCaret + lstrlenW((WCHAR*)pwszText));
					m_nSelStart = m_nCaret;
					m_bIsModified = true;
					GlobalUnlock(handle);
				}
			}
			CloseClipboard();
		}
#endif
	}

	bool StringHelper::CopyTextToClipboard(const string& text_)
	{
#ifdef WIN32
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();

			std::u16string text;
			UTF8ToUTF16(text_, text);

			HGLOBAL hBlock = GlobalAlloc(GMEM_MOVEABLE, sizeof(char16_t) * (text.size() + 1));
			if (hBlock)
			{
				char16_t *szText = (char16_t*)GlobalLock(hBlock);
				if (szText)
				{
					CopyMemory(szText, &(text[0]), text.size() * sizeof(char16_t));
					szText[(int)text.size()] = '\0';  // Terminate it
					GlobalUnlock(hBlock);
				}
				SetClipboardData(CF_UNICODETEXT, hBlock);
			}
			CloseClipboard();
			// We must not free the object until CloseClipboard is called.
			if (hBlock)
				GlobalFree(hBlock);
			return true;
		}
#endif
		return false;
	}

	const char* StringHelper::GetTextFromClipboard()
	{
		static boost::thread_specific_ptr< std::string > g_str_;
		if (!g_str_.get()) {
			// first time called by this thread
			// construct test element to be used in all subsequent calls from this thread
			g_str_.reset(new std::string());
		}
		std::string& g_str = *g_str_;

		bool bSucceed = false;
#ifdef WIN32
		if (OpenClipboard(NULL))
		{
			HANDLE handle = GetClipboardData(CF_UNICODETEXT);
			if (handle)
			{
				const char16_t *szText = (const char16_t*)GlobalLock(handle);
				if (szText)
				{
					std::u16string szText_(szText);
					ParaEngine::StringHelper::UTF16ToUTF8(szText_, g_str);
					bSucceed = true;
					GlobalUnlock(handle);
				}
			}
			CloseClipboard();
		}
#endif
		if (!bSucceed)
			g_str.clear();

		return g_str.c_str();
	}
}

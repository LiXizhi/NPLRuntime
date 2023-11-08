#include "ParaEngine.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif
#include <boost/locale/encoding_utf.hpp>
#include "util/StringHelper.h"
#include "2dengine/GUIEdit.h"
#include "SDL2/SDL.h"

#ifdef EMSCRIPTEN
#include "Js.h"
#endif

namespace ParaEngine {
	void CGUIEditBox::CopyToClipboard()
	{
		// Copy the selection text to the clipboard
		if (m_nCaret != m_nSelStart)
		{
			int nFirst = Math::Min(m_nCaret, m_nSelStart);
			int nLast = Math::Max(m_nCaret, m_nSelStart);
			std::u16string text;
			if (nLast - nFirst > 0)
			{
				text.resize(m_Buffer.GetTextSize() + 1);
				if (m_PasswordChar == '\0')
				{
					text = std::u16string(m_Buffer.GetBuffer() + nFirst, m_Buffer.GetBuffer() + nLast);
				}
				else
				{
					for (int i = 0; i < (nLast - nFirst); ++i)
					{
						text[i] = m_PasswordChar;
					}
				}
			}
			text[nLast - nFirst] = '\0';  // Terminate it
			std::string texta = "";
			if (StringHelper::UTF16ToUTF8(text, texta)) {
				StringHelper::CopyTextToClipboard(texta);
			}
		}
	}

	void CGUIEditBox::PasteFromClipboard()
	{
		DeleteSelectionText();
		std::string text = StringHelper::GetTextFromClipboard();
		if (m_Buffer.InsertStringA(m_nCaret, text.c_str())) {
			PlaceCaret(m_nCaret + text.length());
		}
		m_nSelStart = m_nCaret;
		m_bIsModified = true;
	}

	bool StringHelper::CopyTextToClipboard(const string& text_)
	{
#ifdef EMSCRIPTEN
		JS::SetClipboardText(text_);
		return true;
#endif
		return 0 == SDL_SetClipboardText(text_.c_str());
	}

	const char* StringHelper::GetTextFromClipboard()
	{
#ifdef EMSCRIPTEN
		static std::string s_clip_text;
		s_clip_text =  JS::GetClipboardText();
		return s_clip_text.c_str();
#endif
		static std::string text;
		if (SDL_HasClipboardText() == SDL_FALSE) return "";
		char* str = SDL_GetClipboardText();
		text = str;
		SDL_free(str);
		return text.c_str();
	}
}

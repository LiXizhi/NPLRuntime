#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "2dengine/GUIEdit.h"
#include "jni/JniHelper.h"

#include <boost/thread/tss.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace ParaEngine {
	void CGUIEditBox::CopyToClipboard()
	{
		if (m_nCaret != m_nSelStart)
		{
			int nFirst = Math::Min(m_nCaret, m_nSelStart);
			int nLast = Math::Max(m_nCaret, m_nSelStart);

			if (nLast - nFirst > 0)
			{
				if (m_PasswordChar == '\0')
				{
					std::u16string text(m_Buffer.GetBuffer() + nFirst, nLast - nFirst);
					std::string utf8Text;
					StringHelper::UTF16ToUTF8(text, utf8Text);
					StringHelper::CopyTextToClipboard(utf8Text);
				}
				else
				{
					std::string utf8Text;
					for (int i = 0; i < (nLast - nFirst); ++i)
					{
						utf8Text += m_PasswordChar;
					}
					StringHelper::CopyTextToClipboard(utf8Text);
				}
			}
		}
	}

	void CGUIEditBox::PasteFromClipboard()
	{
		DeleteSelectionText();

		auto utf8Text = StringHelper::GetTextFromClipboard();
		std::u16string text;
		StringHelper::UTF8ToUTF16(utf8Text, text);

		if (m_Buffer.InsertString(m_nCaret, text.c_str()))
			PlaceCaret(m_nCaret + text.length());

		m_nSelStart = m_nCaret;
		m_bIsModified = true;
	}

	bool StringHelper::CopyTextToClipboard(const string& text_)
	{
		return JniHelper::callStaticBooleanMethod("com/tatfook/paracraft/ParaEngineHelper", "CopyTextToClipboard", text_);
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

		g_str = JniHelper::callStaticStringMethod("com/tatfook/paracraft/ParaEngineHelper", "GetTextFromClipboard");

		return g_str.c_str();
	}
}

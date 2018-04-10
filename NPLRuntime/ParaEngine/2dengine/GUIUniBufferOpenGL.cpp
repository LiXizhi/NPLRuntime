#ifdef USE_OPENGL_RENDERER

#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "GUIUniBufferOpenGL.h"
#include "GUIResource.h"



namespace ParaEngine {

	const size_t DXUT_MAX_EDITBOXLENGTH = 0xFFFF;

	CUniLine::CUniLine(CUniBuffer* pParent, int nInitialSize)
		: m_pParent(pParent), m_isDirty(true), m_height(-1)
	{
	}

	//--------------------------------------------------------------------------------------
	CUniLine::~CUniLine()
	{
	}

	//--------------------------------------------------------------------------------------
	void CUniLine::Clear()
	{
		m_utf16Text.clear();
		m_utf8Text.clear();
		m_isDirty = true;
	}

	//--------------------------------------------------------------------------------------
	// Inserts the char at specified index.
	//--------------------------------------------------------------------------------------
	bool CUniLine::InsertChar(int nIndex, char16_t wchar)
	{
		PE_ASSERT(nIndex >= 0);
		if (nIndex < 0)
			return false; // invalid index

		auto nTextSize = m_utf16Text.size();

		if (nIndex > nTextSize)
			return false;

		if (nIndex == 0)
		{
			m_utf16Text = wchar + m_utf16Text;
		}
		else if (nIndex == nTextSize)
		{
			m_utf16Text += wchar;
		}
		else
		{
			std::u16string start(m_utf16Text, nIndex);
			std::u16string end(&m_utf16Text[nIndex]);

			m_utf16Text = start + wchar + end;
		}

		StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);

		m_isDirty = true;
		return true;
	}

	//--------------------------------------------------------------------------------------
	// Removes the char at specified index.
	// If nIndex == -1, remove the last char.
	//--------------------------------------------------------------------------------------
	bool CUniLine::RemoveChar(int nIndex)
	{
		if (m_utf16Text.size() > 0)
		{
			if (nIndex >= 0 && nIndex < m_utf16Text.size())
				m_utf16Text = m_utf16Text.substr(0, nIndex) + m_utf16Text.substr(nIndex + 1);
			else
				m_utf16Text = m_utf16Text.substr(0, m_utf16Text.size() - 1);
			StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);

			m_isDirty = true;
		}
		return true;
	}


	bool CUniLine::ReplaceChar(int nIndex, char16_t wchar)
	{
		if (nIndex < m_utf16Text.size())
		{
			m_utf16Text[nIndex] = wchar;
			StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);

			m_isDirty = true;
		}
		return true;
	}

	//--------------------------------------------------------------------------------------
	// Inserts the first nCount characters of the string pStr at specified index.
	// If nCount == -1, the entire string is inserted.
	//--------------------------------------------------------------------------------------
	bool CUniLine::InsertString(int nIndex, const char16_t *pStr, int nCount)
	{
		m_utf16Text = m_utf16Text.substr(0, nIndex) + pStr + m_utf16Text.substr(nIndex);
		StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);

		m_isDirty = true;
		return true;
	}

	bool CUniLine::InsertStringA(int nIndex, const char *pStr, int nCount)
	{
		if (nIndex >= 0)
			m_utf8Text = m_utf8Text.substr(0, nIndex) + pStr + m_utf8Text.substr(nIndex);
		else
			m_utf8Text += pStr;
		StringHelper::UTF8ToUTF16_Safe(m_utf8Text, m_utf16Text);

		m_isDirty = true;
		return true;
	}

	//--------------------------------------------------------------------------------------
	bool CUniLine::SetTextA(LPCSTR szText)
	{
		m_utf8Text = szText;
		StringHelper::UTF8ToUTF16_Safe(m_utf8Text, m_utf16Text);

		m_isDirty = true;
		return true;
	}

	bool CUniLine::SetText(const char16_t* wszText)
	{
		m_utf16Text = wszText;
		StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);
		m_isDirty = true;
		return true;
	}



	//--------------------------------------------------------------------------------------
	HRESULT CUniLine::CPtoX(int nCP, BOOL bTrail, int *pX)
	{
		std::vector<GLLabel::LetterInfo>* lettersInfo;
		int* horizontalKernings;
		float labelHeight;

		updateLettersInfo(lettersInfo, horizontalKernings, labelHeight);

		if (!lettersInfo)
		{
			if (pX)
			{
					*pX = 0;
			}
			return S_OK;
		}

		if (nCP < 0)
			nCP = 0;

		auto textSize = GetTextSize();
		size_t i;
		size_t j = 0;

		for (i = 0;
			(i < lettersInfo->size()) && ((i - j) < textSize);
			i++)
		{
			auto& curLetter = (*lettersInfo)[i];
			if (!curLetter.def.validDefinition)
			{
				j++;
				continue;
			}

			if (nCP == i - j) break;
		}

		auto hIndex = i - j;
		if (hIndex >= textSize)
			hIndex = textSize - 1;

		auto index = i;
		if (index < lettersInfo->size())
		{
			auto& info = (*lettersInfo)[index];

			auto horizontalKerning = horizontalKernings[hIndex];

			if (pX)
			{
				int result = (int)(info.position.x - info.def.offsetX - horizontalKerning / 2);
				*pX = result;
			}
		}
		else
		{
			for (i = lettersInfo->size() - 1; i >= 0; i--)
			{
				auto& info = (*lettersInfo)[i];

				if (info.def.validDefinition)
				{
					if (pX)
					{
						int result = (int)(info.position.x - info.def.offsetX + info.def.xAdvance);
						*pX = result;
					}
					break;
				}
					
			}
		}

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	HRESULT CUniLine::XtoCP(int nX, int *pCP, int *pnTrail)
	{
		std::vector<GLLabel::LetterInfo>* lettersInfo;
		int* horizontalKernings;
		float labelHeight;


		updateLettersInfo(lettersInfo, horizontalKernings, labelHeight);

		if (!lettersInfo || !horizontalKernings)
		{
			if (pCP)
				*pCP = GetTextSize(); 
			if (pnTrail)
				*pnTrail = 0;

			return S_OK;
		}

		auto textSize = GetTextSize();
		bool bFound = false;
		float fX = (float)nX;
		size_t i;
		size_t j = 0;
		int ret = 0;

		for (i = 0;
			(i < lettersInfo->size()) && ((i - j) < textSize);
			i++)
		{
			auto& curLetter = (*lettersInfo)[i];
			if (!curLetter.def.validDefinition)
			{
				j++;
				continue;
			}

			ret = (int)(i - j);

			auto& def = curLetter.def;
			auto& curPos = curLetter.position;
			auto horizontalKerning = horizontalKernings[ret];
			auto nextHorizontalKerning = 0;

			if (ret + 1 < textSize)
			{
				nextHorizontalKerning = horizontalKernings[ret + 1];
			}

			auto x = curPos.x - def.offsetX - horizontalKerning / 2;
			auto w = horizontalKerning / 2 + def.xAdvance + nextHorizontalKerning / 2;

			

			if (ret == 0 && fX < x)	break;
			if (ret == textSize - 1 && fX >= x + w) break;

			if (fX >= x && fX < x + w)
			{
				bFound = true;
				break;
			}
		}

		if (bFound)
		{
			if (pnTrail)
				*pnTrail = FALSE;
		}
		else
		{
			if (pnTrail)
				*pnTrail = TRUE;
		}
		
		if (pCP)
			*pCP = ret;

		return S_OK;
	}

	int CUniLine::GetHeight()
	{
		if (m_height == -1)
		{
			auto pFontNode = m_pParent->GetFontNode();
			if (!pFontNode)
			{
				auto pFont = (SpriteFontEntityOpenGL*)pFontNode->GetFont();
				if (pFont)
				{
					auto dwTextFormat = pFontNode->dwTextFormat;
					auto& rect = m_pParent->GetRect();

					RECT textRect = { 0, 0, rect.right - rect.left, rect.bottom - rect.top };


					m_height = pFont->GetLineHeight(GetBuffer(), -1, textRect, dwTextFormat);
				}
			}
		}

		return m_height;
	}

	bool CUniLine::IsDirty() const
	{
		return m_isDirty;
	}

	void CUniLine::updateLettersInfo(std::vector<GLLabel::LetterInfo>*& lettersInfo, int*& horizontalKernings, float& labelHeight) const
	{
		lettersInfo = nullptr;
		horizontalKernings = nullptr;
		labelHeight = 0;

		auto pFontNode = m_pParent->GetFontNode();
		if (pFontNode)
		{
			auto pFont = (SpriteFontEntityOpenGL*)pFontNode->GetFont();
			if (pFont)
			{
				auto dwTextFormat = pFontNode->dwTextFormat;
				auto& rect = m_pParent->GetRect();
				
				RECT textRect = { 0, 0, rect.right - rect.left, rect.bottom - rect.top };

				if (!pFont->GetLettersInfo(lettersInfo, horizontalKernings, labelHeight, GetBuffer(), -1, textRect, dwTextFormat))
				{
					lettersInfo = nullptr;
					horizontalKernings = nullptr;
					labelHeight = 0;
				}
				
			}
		}
	}

	//--------------------------------------------------------------------------------------
	void CUniLine::GetPriorItemPos(int nCP, int *pPrior)
	{
		*pPrior = nCP;  // Default is the char itself
	}


	//--------------------------------------------------------------------------------------
	void CUniLine::GetNextItemPos(int nCP, int *pPrior)
	{
		*pPrior = nCP;  // Default is the char itself
	}

	int CUniLine::GetBufferA(std::string& out) const
	{
		out = m_utf8Text;
		return m_utf8Text.size();
	}
	
	int CUniLine::GetTextSize() const
	{
		return m_utf16Text.size();
	}

	bool CUniLine::IsEmpty()
	{
		return m_utf16Text.empty();
	}
	
	const char16_t* CUniLine::GetBuffer() const
	{
		return m_utf16Text.c_str();
	}

	const std::string& CUniLine::GetUtf8Text() const
	{
		return m_utf8Text;
	}

	//--------------------------------------------------------------------------------------
	CUniBuffer::CUniBuffer(int nInitialSize)
		: m_bMultiline(false), m_curLine(NULL),m_pFontNode(nullptr)
	{
		CUniLine* line = new CUniLine(this, nInitialSize);
		m_lines.push_back(line);
	}

	//--------------------------------------------------------------------------------------
	CUniBuffer::~CUniBuffer()
	{
		for (auto iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			delete (*iter);
		}
		m_lines.clear();
	}

	//--------------------------------------------------------------------------------------
	const char16_t& CUniBuffer::operator[](size_t n) // No param checking
	{
		// This version of operator[] is called only
		// if we are asking for write access, so
		// re-analysis is required.
		int charpos;
		if ((charpos = GetLineAt(n)) == -1) {
			CUniLine *temp = m_lines.back();
			return (*temp)[temp->GetTextSize() - 1];
		}
		else
			return (*GetCurLine())[charpos];

	}
	
	//--------------------------------------------------------------------------------------
	void CUniBuffer::Clear()
	{
		for (auto iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			(*iter)->Clear();
		}
		m_curLine = NULL;
	}

	int CUniBuffer::GetLineAt(int nIndex)
	{
		int charcount = 0;
		list<CUniLine*>::iterator iter;
		for (iter = m_lines.begin(); iter != m_lines.end() && charcount <= nIndex; iter++) {
			charcount += (*iter)->GetTextSize();
		};
		if (charcount >= nIndex) {
			iter--;
			charcount -= (*iter)->GetTextSize();
			m_curLine = (*iter);
			return nIndex - charcount;
		}
		return -1;
	}

	ParaEngine::CUniLine * CUniBuffer::GetCurLine() const
	{
		return m_curLine;
	}

	int CUniBuffer::GetTextSize()const
	{
		int charcount = 0;
		list<CUniLine*>::const_iterator iter;
		for (iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			charcount += (*iter)->GetTextSize();
		};
		return charcount;
	}

	const char16_t* CUniBuffer::GetBuffer() const
	{
		if (!m_bMultiline) {
			return m_lines.front()->GetBuffer();
		}
		return nullptr;
	}

	//--------------------------------------------------------------------------------------
	// Inserts the char at specified index.
	//--------------------------------------------------------------------------------------
	bool CUniBuffer::InsertChar(int nIndex, char16_t wChar)
	{
		PE_ASSERT(nIndex >= 0);

		//if( nIndex < 0 || nIndex > lstrlenW( m_pwszBuffer ) )
		//	return false;  // invalid index

		//// Check for maximum length allowed
		//if( GetTextSize() + 1 >= DXUT_MAX_EDITBOXLENGTH )
		//	return false;
		int charpos;
		if ((charpos = GetLineAt(nIndex)) == -1) {
			return false;
		}
		else
			return GetCurLine()->InsertChar(charpos, wChar);
	}

	bool CUniBuffer::ReplaceChar(int nIndex, char16_t wchar)
	{
		int charpos;
		if ((charpos = GetLineAt(nIndex)) == -1) {
			return false;
		}
		else
			return GetCurLine()->ReplaceChar(charpos, wchar);
	}

	//--------------------------------------------------------------------------------------
	// Removes the char at specified index.
	//--------------------------------------------------------------------------------------
	bool CUniBuffer::RemoveChar(int nIndex)
	{
		int charpos;
		if ((charpos = GetLineAt(nIndex)) == -1) {
			return false;
		}
		else
			return GetCurLine()->RemoveChar(charpos);
	}

	//--------------------------------------------------------------------------------------
	// Inserts the first nCount characters of the string pStr at specified index.
	// If nCount == -1, the entire string is inserted.
	//--------------------------------------------------------------------------------------
	bool CUniBuffer::InsertString(int nIndex, const char16_t *pStr, int nCount)
	{
		PE_ASSERT(nIndex >= 0);

		int charpos;
		if ((charpos = GetLineAt(nIndex)) == -1) {
			return false;
		}
		else
			return GetCurLine()->InsertString(charpos, pStr, nCount);

	}

	bool CUniBuffer::InsertStringA(int nIndex, const char *pStr, int nCount)
	{
		PE_ASSERT(nIndex >= 0);

		int charpos;
		if ((charpos = GetLineAt(nIndex)) == -1) {
			return false;
		}
		else
			return GetCurLine()->InsertStringA(charpos, pStr, nCount);
	}

	//--------------------------------------------------------------------------------------
	bool CUniBuffer::SetTextA(const char* szText)
	{
		PE_ASSERT(szText != nullptr);

		return m_lines.front()->SetTextA(szText);
		//TODO: multiline support
	}

	bool CUniBuffer::SetText(const char16_t* wszText)
	{
		PE_ASSERT(wszText != nullptr);
		return m_lines.front()->SetText(wszText);
		//TODO: multiline support
	}

	//--------------------------------------------------------------------------------------
	HRESULT CUniBuffer::CPtoXY(int nCP, BOOL bTrail, int *pX, int *pY)
	{
		PE_ASSERT(pX && pY);
		if (nCP < 0 || !m_bMultiline) 
		{
			*pY = 0;
			auto ret = m_lines.front()->CPtoX(nCP, bTrail, pX);
			return ret;
		}
		int charpos = GetLineAt(nCP);
		list<CUniLine*>::iterator iter;
		CUniLine *line;
		int y = 0;

		if (charpos == -1)
		{
			for (iter = m_lines.begin(); iter != m_lines.end(); iter++)
			{
				line = (*iter);
				auto height = line->GetHeight();
				y += height == -1 ? 0 : height;
			}

			*pY = y;
			return m_lines.back()->CPtoX(charpos, bTrail, pX);
		}
		else
		{
			for (iter = m_lines.begin(); (*iter) != GetCurLine(); iter++)
			{
				line = (*iter);
				auto height = line->GetHeight();
				y += height == -1 ? 0 : height;
			}
			*pY = y;
			return GetCurLine()->CPtoX(charpos, bTrail, pX);
		}

		//OUTPUT_LOG("CUniBuffer::CPtoXY : nCP = %d pX = %d pY = %d", nCP, *pX, *pY);
		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	HRESULT CUniBuffer::XYtoCP(int nX, int nY, int *pCP, int *pnTrail)
	{
		PE_ASSERT(pCP && pnTrail);
		auto ret = m_lines.front()->XtoCP(nX, pCP, pnTrail);
		return ret;
		//TODO: multiline support
	}

	//--------------------------------------------------------------------------------------
	void CUniBuffer::GetPriorItemPos(int nCP, int *pPrior)
	{
		m_lines.front()->GetPriorItemPos(nCP, pPrior);
		//TODO: multiline support

	}

	//--------------------------------------------------------------------------------------
	void CUniBuffer::GetNextItemPos(int nCP, int *pPrior)
	{
		m_lines.front()->GetNextItemPos(nCP, pPrior);
		//TODO: multiline support
	}

	bool CUniBuffer::GetMultiline() const
	{
		return m_bMultiline;
	}

	void CUniBuffer::SetMultiline(bool multiline)
	{
		m_bMultiline = multiline;
	}

	int CUniBuffer::GetBufferA(std::string& out) const
	{
		//TODO: multiline support
		return m_lines.front()->GetBufferA(out);
	}

	bool CUniBuffer::IsEmpty()
	{
		for (auto iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			if (!((*iter)->IsEmpty()))
				return false;
		};
		return true;
	}

	const std::string& CUniBuffer::GetUtf8Text() const
	{
		if(!m_lines.empty())
			return m_lines.front()->GetUtf8Text();
		else
			return CGlobals::GetString(0);
	}

} // end namespace

#endif
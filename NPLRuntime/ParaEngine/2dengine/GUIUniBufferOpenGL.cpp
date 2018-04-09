#ifdef USE_OPENGL_RENDERER

#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "GUIUniBufferOpenGL.h"
#include "GUIResource.h"



namespace ParaEngine {

	const size_t DXUT_MAX_EDITBOXLENGTH = 0xFFFF;

	CUniLine::CUniLine(CUniBuffer* pParent, int nInitialSize)
		: m_nBufferSize(0)
		, m_pwszBuffer(nullptr)
		, m_pParent(pParent)
		, m_height(-1)
	{
		if (nInitialSize > 0)
			SetBufferSize(nInitialSize);
	}

	bool CUniLine::SetBufferSize(int nNewSize)
	{
		// If the current size is already the maximum allowed,
		// we can't possibly allocate more.
		if (m_nBufferSize == DXUT_MAX_EDITBOXLENGTH)
			return false;

		if (m_nBufferSize > nNewSize && nNewSize >= 0)
		{
			return true;
		}

		int nAllocateSize = (nNewSize == -1 || nNewSize < m_nBufferSize * 2) ? (m_nBufferSize ? m_nBufferSize * 2 : 16) : nNewSize * 2;

		// Cap the buffer size at the maximum allowed.
		if (nAllocateSize > DXUT_MAX_EDITBOXLENGTH)
			nAllocateSize = DXUT_MAX_EDITBOXLENGTH;

		char16_t *pTempBuffer = new char16_t[nAllocateSize];

		if (!pTempBuffer)
			return false;

		if (m_pwszBuffer)
		{
			if (m_nBufferSize > 0)
			{
				memcpy(pTempBuffer, m_pwszBuffer, m_nBufferSize * sizeof(char16_t));
			}
			delete[] m_pwszBuffer;
		}
		else
		{
			memset(pTempBuffer, 0, sizeof(char) * nAllocateSize);
		}

		m_pwszBuffer = pTempBuffer;
		m_nBufferSize = nAllocateSize;

		return true;
	}

	//--------------------------------------------------------------------------------------
	CUniLine::~CUniLine()
	{
		SAFE_DELETE(m_pwszBuffer);
	}

	//--------------------------------------------------------------------------------------
	void CUniLine::Clear()
	{
		*m_pwszBuffer = (char16_t)(L'\0');
	}

	static size_t widecslen(const char16_t* pStr)
	{
		auto pCur = pStr;
		while (*pCur)
			++pCur;
		return pCur - pStr;
	}

	static HRESULT safeWidecscpy(char16_t* pDest, size_t cbDest, const char16_t* pSrc)
	{
		if (cbDest == 0)
			return S_FALSE;

		while (*pSrc)
		{
			if (cbDest == 0)
				return S_FALSE;

			*pDest = *pSrc;
			++pDest;
			++pSrc;
			--cbDest;
		}
		
		if (cbDest == 0)
			return S_FALSE;

		*pDest = 0;

		return S_OK;

	}

	//--------------------------------------------------------------------------------------
	// Inserts the char at specified index.
	//--------------------------------------------------------------------------------------
	bool CUniLine::InsertChar(int nIndex, char16_t wChar)
	{
		PE_ASSERT(nIndex >= 0);


		if (nIndex < 0)
			return false;  // invalid index
		int nTextSize = widecslen(m_pwszBuffer); //lstrlenW((WCHAR*)m_pwszBuffer);
		if (nIndex > nTextSize)
			return false;

		// Check for maximum length allowed
		if ((nTextSize + 1) >= DXUT_MAX_EDITBOXLENGTH)
			return false;


		if ((nTextSize + 1) >= m_nBufferSize)
		{
			if (!SetBufferSize(-1))
				return false;  // out of memory
		}

		PE_ASSERT(m_nBufferSize >= 2);

		// Shift the characters after the index, start by copying the null terminator
		char16_t* dest = m_pwszBuffer + nTextSize + 1;
		char16_t* stop = m_pwszBuffer + nIndex;
		char16_t* src = dest - 1;

		while (dest > stop)
		{
			*dest-- = *src--;
		}

		// Set new character
		m_pwszBuffer[nIndex] = wChar;

		return true;
	}

	//--------------------------------------------------------------------------------------
	// Removes the char at specified index.
	// If nIndex == -1, remove the last char.
	//--------------------------------------------------------------------------------------
	bool CUniLine::RemoveChar(int nIndex)
	{
		auto len = widecslen(m_pwszBuffer);
		if (!len || nIndex < 0 || nIndex >= len)
			return false;  // Invalid index

		memmove(m_pwszBuffer + nIndex, m_pwszBuffer + nIndex + 1, sizeof(char16_t)* (len - nIndex));

		return true;
	}


	bool CUniLine::ReplaceChar(int nIndex, char16_t wchar)
	{
		m_pwszBuffer[nIndex] = wchar;
		return true;
	}

	//--------------------------------------------------------------------------------------
	// Inserts the first nCount characters of the string pStr at specified index.
	// If nCount == -1, the entire string is inserted.
	//--------------------------------------------------------------------------------------
	bool CUniLine::InsertString(int nIndex, const char16_t *pStr, int nCount)
	{
		PE_ASSERT(nIndex >= 0);

		auto curLen = widecslen(m_pwszBuffer);

		if (nIndex > curLen)
			return false;  // invalid index

	
		if (-1 == nCount)
			nCount = widecslen(pStr);

		// Check for maximum length allowed
		if (GetTextSize() + nCount >= DXUT_MAX_EDITBOXLENGTH)
			return false;

		if (curLen + nCount >= m_nBufferSize)
		{
			if (!SetBufferSize(curLen + nCount + 1))
				return false;  // out of memory
		}

		memmove(m_pwszBuffer + nIndex + nCount, m_pwszBuffer + nIndex, sizeof(char16_t)* (curLen - nIndex + 1));
		memcpy(m_pwszBuffer + nIndex, pStr, nCount * sizeof(char16_t));

		return true;
	}

	bool CUniLine::InsertStringA(int nIndex, const char *pStr, int nCount)
	{
		bool rvalue = InsertString(nIndex, (const char16_t*)StringHelper::MultiByteToWideChar(pStr, DEFAULT_GUI_ENCODING), nCount);
		return rvalue;
	}

	//--------------------------------------------------------------------------------------
	bool CUniLine::SetTextA(LPCSTR szText)
	{
		PE_ASSERT(szText != nullptr);
		bool rvalue = SetText((const char16_t*)StringHelper::MultiByteToWideChar(szText, DEFAULT_GUI_ENCODING));
		return rvalue;
	}

	bool CUniLine::SetText(const char16_t* wszText)
	{
		PE_ASSERT(wszText != nullptr);

		int nRequired = int(widecslen(wszText) + 1);

		// Check for maximum length allowed
		if (nRequired >= DXUT_MAX_EDITBOXLENGTH)
			return false;

		SetBufferSize(nRequired);

		// Check again in case out of memory occurred inside while loop.
		if (GetBufferSize() >= nRequired)
		{
			//StringCchCopyW((WCHAR*)m_pwszBuffer, GetBufferSize(), (WCHAR*)wszText);

			safeWidecscpy(m_pwszBuffer, GetBufferSize(), wszText);

			return true;
		}
		else
			return false;
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
		return StringHelper::UTF16ToUTF8(GetBuffer(), out) ? (int)out.size() : 0;
	}
	
	int CUniLine::GetTextSize() const
	{
		return widecslen(m_pwszBuffer);
	}

	bool CUniLine::IsEmpty()
	{
		return (m_nBufferSize == 0 || (m_pwszBuffer && m_pwszBuffer[0] == (L'\0')));
	}

	int CUniLine::GetBufferSize() const
	{
		return m_nBufferSize;
	}

	const char16_t* CUniLine::GetBuffer() const
	{
		return m_pwszBuffer;
	}


	//--------------------------------------------------------------------------------------
	CUniBuffer::CUniBuffer(int nInitialSize)
		: m_bMultiline(false)
		, m_pFontNode(nullptr)
	{
		CUniLine* line = new CUniLine(this, nInitialSize);
		m_lines.push_back(line);
	}

	//--------------------------------------------------------------------------------------
	CUniBuffer::~CUniBuffer()
	{
		Clear();
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

	bool CUniBuffer::SetBufferSize(int nNewSize)
	{
		if (!m_bMultiline) {
			return m_lines.front()->SetBufferSize(nNewSize);
		}
		return false;
	}

	//--------------------------------------------------------------------------------------
	void CUniBuffer::Clear()
	{
		list<CUniLine*>::iterator iter;
		for (iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			delete (*iter);
		}
		m_lines.clear();

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

	int CUniBuffer::GetTextSize()const
	{
		int charcount = 0;
		list<CUniLine*>::const_iterator iter;
		for (iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			charcount += (*iter)->GetTextSize();
		};
		return charcount;
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
		bool rvalue = InsertString(nIndex, (const char16_t*)StringHelper::MultiByteToWideChar(pStr, DEFAULT_GUI_ENCODING), nCount);
		return rvalue;
	}

	//--------------------------------------------------------------------------------------
	bool CUniBuffer::SetTextA(const char* szText)
	{
		PE_ASSERT(szText != nullptr);
		bool rvalue = SetText((const char16_t*)StringHelper::MultiByteToWideChar(szText, DEFAULT_GUI_ENCODING));
		return rvalue;
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

	int CUniBuffer::GetBufferA(std::string& out) const
	{
		//TODO: multiline support
		return m_lines.front()->GetBufferA(out);
	}

	bool CUniBuffer::IsEmpty()
	{
		list<CUniLine*>::const_iterator iter;
		for (iter = m_lines.begin(); iter != m_lines.end(); iter++) {
			if (!((*iter)->IsEmpty()))
				return false;
		};
		return true;
	}

	const std::string& CUniBuffer::GetUtf8Text() const
	{
		static std::string s;
		m_lines.front()->GetBufferA(s);
		return s;
	}

} // end namespace

#endif
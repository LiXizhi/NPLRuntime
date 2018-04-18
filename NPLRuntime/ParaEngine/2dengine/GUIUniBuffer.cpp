//----------------------------------------------------------------------
// Class:	CGUIUniBuffer
// Authors:	LiXizhi, Liu Weili
// Company: ParaEngine
// Date:	2005.8.3
// Revised: 2010.3.13
// Desc: used by GUIEditbox and GUIIMEEditbox
//-----------------------------------------------------------------------

#if !defined(USE_DIRECTX_RENDERER) && !defined(USE_OPENGL_RENDERER)
int ParaEngine::CUniBuffer::GetBufferA(std::string& out) const
{
	out = m_utf8Text;
	return 0;
}

bool ParaEngine::CUniBuffer::InsertChar(int nIndex, char16_t wchar)
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


	return true;
}

bool ParaEngine::CUniBuffer::RemoveChar(int nIndex)
{
	if (m_utf16Text.size() > 0)
	{
		if (nIndex >= 0)
			m_utf16Text = m_utf16Text.substr(0, nIndex) + m_utf16Text.substr(nIndex + 1);
		else
			m_utf16Text = m_utf16Text.substr(0, m_utf16Text.size() - 1);
		StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);
	}
	return true;
}

bool ParaEngine::CUniBuffer::InsertString(int nIndex, const char16_t *pStr, int nCount /*= -1*/)
{
	m_utf16Text = m_utf16Text.substr(0, nIndex) + pStr + m_utf16Text.substr(nIndex);
	StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);
	return true;
}

bool ParaEngine::CUniBuffer::InsertStringA(int nIndex, const char *pStr, int nCount /*= -1*/)
{
	if (nIndex >= 0)
		m_utf8Text = m_utf8Text.substr(0, nIndex) + pStr + m_utf8Text.substr(nIndex);
	else
		m_utf8Text += pStr;
	StringHelper::UTF8ToUTF16_Safe(m_utf8Text, m_utf16Text);
	return true;
}

bool ParaEngine::CUniBuffer::SetTextA(LPCSTR szText)
{
	m_utf8Text = szText;
	StringHelper::UTF8ToUTF16_Safe(m_utf8Text, m_utf16Text);
	return true;
}

bool ParaEngine::CUniBuffer::SetText(const char16_t* wszText)
{
	m_utf16Text = wszText;
	StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);
	return true;
}

void ParaEngine::CUniBuffer::Clear()
{
	m_utf8Text.clear();
	m_utf16Text.clear();
}

int ParaEngine::CUniBuffer::GetTextSize() const
{
	return m_utf16Text.size();
}

const char16_t* ParaEngine::CUniBuffer::GetBuffer() const
{
	return m_utf16Text.c_str();
}

char16_t& ParaEngine::CUniBuffer::operator[](int n)
{
	if ((int)m_utf16Text.size() > n)
		return m_utf16Text[n];
	else
	{
		static char16_t s_char = '\0';
		return s_char;
	}
}

HRESULT ParaEngine::CUniBuffer::CPtoXY(int nCP, BOOL bTrail, int *pX, int *pY)
{
	if (*pX)
		*pX = 0;
	if (*pY)
		*pY = 0;
	return S_OK;
}

HRESULT ParaEngine::CUniBuffer::XYtoCP(int nX, int nY, int *pCP, int *pnTrail)
{
	if (*pCP)
		*pCP = 0;
	if (*pnTrail)
		*pnTrail = 0;
	return S_OK;
}

void ParaEngine::CUniBuffer::GetPriorItemPos(int nCP, int *pPrior)
{
	if (*pPrior)
		*pPrior = 0;
}

void ParaEngine::CUniBuffer::GetNextItemPos(int nCP, int *pPrior)
{
	if (*pPrior)
		*pPrior = 0;
}

bool ParaEngine::CUniBuffer::IsEmpty()
{
	return m_utf16Text.empty();
}
#endif
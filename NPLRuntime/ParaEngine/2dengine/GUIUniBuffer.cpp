//----------------------------------------------------------------------
// Class:	CGUIUniBuffer
// Authors:	LiXizhi, Liu Weili
// Company: ParaEngine
// Date:	2005.8.3
// Revised: 2010.3.13
// Desc: used by GUIEditbox and GUIIMEEditbox
//-----------------------------------------------------------------------

#if !defined(USE_DIRECTX_RENDERER) && !defined(USE_OPENGL_RENDERER)
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "GUIUniBufferDirectX.h"
#include "GUIResource.h"
#include "MiscEntity.h"
#include "memdebug.h"

using namespace ParaEngine;
using namespace std;

ParaEngine::CUniBuffer::CUniBuffer()
	: m_pFontNode(nullptr)
	, m_pLettersInfo(nullptr)
	, m_isDirty(true)
	, m_lastTextFormat(0)
	, m_horizontalKernings(nullptr)
{

}

int ParaEngine::CUniBuffer::GetBufferA(std::string& out) const
{
	out = m_utf8Text;
	return 0;
}

bool ParaEngine::CUniBuffer::InsertChar(int nIndex, char16_t wchar)
{
	//PE_ASSERT(nIndex >= 0);


	//if (nIndex < 0)
	//	return false;  // invalid index
	//int nTextSize = lstrlenW((WCHAR*)m_pwszBuffer);
	//if (nIndex > nTextSize)
	//	return false;

	//// Check for maximum length allowed
	//if ((nTextSize + 1) >= DXUT_MAX_EDITBOXLENGTH)
	//	return false;


	//if ((nTextSize + 1) >= m_nBufferSize)
	//{
	//	if (!SetBufferSize(-1))
	//		return false;  // out of memory
	//}

	//PE_ASSERT(m_nBufferSize >= 2);

	//// Shift the characters after the index, start by copying the null terminator
	//char16_t* dest = m_pwszBuffer + nTextSize + 1;
	//char16_t* stop = m_pwszBuffer + nIndex;
	//char16_t* src = dest - 1;

	//while (dest > stop)
	//{
	//	*dest-- = *src--;
	//}

	//// Set new character
	//m_pwszBuffer[nIndex] = wChar;
	//m_bAnalyseRequired = true;

	//return true;

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

bool ParaEngine::CUniBuffer::RemoveChar(int nIndex)
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



bool ParaEngine::CUniBuffer::ReplaceChar(int nIndex, char16_t wchar)
{
	if (nIndex < m_utf16Text.size())
	{
		m_utf16Text[nIndex] = wchar;
		StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);

		m_isDirty = true;
	}
	return true;
}

bool ParaEngine::CUniBuffer::InsertString(int nIndex, const char16_t *pStr, int nCount /*= -1*/)
{
	m_utf16Text = m_utf16Text.substr(0, nIndex) + pStr + m_utf16Text.substr(nIndex);
	StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);

	m_isDirty = true;
	return true;
}

bool ParaEngine::CUniBuffer::InsertStringA(int nIndex, const char *pStr, int nCount /*= -1*/)
{
	if (nIndex >= 0)
		m_utf8Text = m_utf8Text.substr(0, nIndex) + pStr + m_utf8Text.substr(nIndex);
	else
		m_utf8Text += pStr;
	StringHelper::UTF8ToUTF16_Safe(m_utf8Text, m_utf16Text);

	m_isDirty = true;
	return true;
}

bool ParaEngine::CUniBuffer::SetTextA(LPCSTR szText)
{
	m_utf8Text = szText;
	StringHelper::UTF8ToUTF16_Safe(m_utf8Text, m_utf16Text);

	m_isDirty = true;
	return true;
}

bool ParaEngine::CUniBuffer::SetText(const char16_t* wszText)
{
	m_utf16Text = wszText;
	StringHelper::UTF16ToUTF8(m_utf16Text, m_utf8Text);
	m_isDirty = true;
	return true;
}

void ParaEngine::CUniBuffer::Clear()
{
	m_utf8Text.clear();
	m_utf16Text.clear();

	m_isDirty = true;
}

int ParaEngine::CUniBuffer::GetTextSize() const
{
	return m_utf16Text.size();
}

const char16_t* ParaEngine::CUniBuffer::GetBuffer() const
{
	return m_utf16Text.c_str();
}

const char16_t& ParaEngine::CUniBuffer::operator[](int n)
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
	updateLettersInfo();

	if (!m_pLettersInfo)
	{
		if (pX)
			*pX = 0;
		if (pY)
			*pY = 0;

		return S_OK;
	}

	auto& lettersInfo = *m_pLettersInfo;

	if (nCP < 0)
		nCP = 0;

	size_t i;
	size_t j = 0;
	for (i = 0;
		(i < lettersInfo.size() - 1) && ((i - j) < m_utf16Text.size() - 1);
		i++)
	{
		auto& curLetter = lettersInfo[i];
		if (!curLetter.def.validDefinition)
		{
			j++;
			continue;
		}

		if (nCP == i - j)
		{
			break;
		}
	}

	auto index = i;
	if (index < lettersInfo.size())
	{
		auto& info = lettersInfo[index];

		auto horizontalKerning = m_horizontalKernings[index];

		if (pX)
			*pX = (int)(info.position.x - info.def.offsetX - horizontalKerning / 2);
		if (pY)
		{
			auto y = info.position.y + info.def.offsetY;
			y = m_labelHeight - y;

			*pY = (int)y;
		}
	}
	else
	{
		for (i = lettersInfo.size() - 1; i >= 0; i--)
		{
			auto& info = lettersInfo[i];

			if (pX)
				*pX = (int)(info.position.x - info.def.offsetX + info.def.xAdvance);
			if (pY)
			{
				auto y = info.position.y + info.def.offsetY;
				y = m_labelHeight - y;

				*pY = (int)y;
			}
		}
	}

	return S_OK;
}

HRESULT ParaEngine::CUniBuffer::XYtoCP(int nX, int nY, int *pCP, int *pnTrail)
{
	// TODO: we will always position at the back of the line for the moment. 
	// this is consistent with the draw function of GUIEdit which always draw caret at tail
	updateLettersInfo();

	if (!m_pLettersInfo || !m_horizontalKernings)
	{
		if (pCP)
			*pCP = m_utf16Text.size();
		if (pnTrail)
			*pnTrail = 0;

		return S_OK;
	}

	auto& lettersInfo = *m_pLettersInfo;
	bool bFound = false;

	float fX = (float)nX;
	float fY = (float)nY;
	size_t i;
	size_t j = 0;
	int ret = 0;
	for (i = 0;
		(i < lettersInfo.size() - 1) && ((i - j) < m_utf16Text.size() - 1);
		i++)
	{
		auto& curLetter = lettersInfo[i];
		if (!curLetter.def.validDefinition)
		{
			j++;
			continue;
		}

		auto& def = lettersInfo[i].def;
		auto& curPos = lettersInfo[i].position;
		auto horizontalKerning = m_horizontalKernings[i];
		auto nextHorizontalKerning = 0;
		if (i + 1 < lettersInfo.size())
		{
			nextHorizontalKerning = m_horizontalKernings[i + 1];
		}


		auto x = curPos.x - def.offsetX - horizontalKerning / 2;
		auto w = horizontalKerning / 2 + def.xAdvance + nextHorizontalKerning / 2;

		//OUTPUT_LOG("######### XYtoCP : index = %d, x = %.0f, xAdvance = %d, fx = %.0f", i, x, def.xAdvance, fX);

		ret = (int)(i - j);

		if (ret == 0 && fX < x)
		{
			break;
		}
		if (ret == m_utf16Text.size() && fX >= x + w)
		{
			break;
		}


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

	OUTPUT_LOG("###### XYtoCP : in %d, %d out %d", nX, nY, *pCP);

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

void ParaEngine::CUniBuffer::SetFontNode(GUIFontElement *pFontNode)
{
	//OUTPUT_LOG("CUniBuffer::SetFontNode");

	m_pFontNode = pFontNode;

	m_isDirty = true;
}

void ParaEngine::CUniBuffer::SetRect(const RECT& r)
{
	//OUTPUT_LOG("CUniBuffer::SetRect");

	m_rect = r;

	m_isDirty = true;
}

bool ParaEngine::CUniBuffer::IsDirty() const
{
	return m_isDirty;
}

void ParaEngine::CUniBuffer::updateLettersInfo()
{
	if (m_pFontNode != nullptr)
	{
		auto pFont = (SpriteFontEntityOpenGL*)m_pFontNode->GetFont();
		if (pFont)
		{
			auto dwTextFormat = m_pFontNode->dwTextFormat;

			RECT rect = { 0, 0, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top };
			//m_pLettersInfo = pFont->GetLettersInfo(GetBuffer(), -1, rect, dwTextFormat, m_labelHeight);
			if (!pFont->GetLettersInfo(m_pLettersInfo, m_horizontalKernings, m_labelHeight, GetBuffer(), -1, rect, dwTextFormat))
			{
				m_pLettersInfo = nullptr;
				m_horizontalKernings = nullptr;
				m_labelHeight = 0;
			}

		}
	}

	/*
	if (!IsDirty() && m_pFontNode != nullptr)
	{
	auto pFont = (SpriteFontEntityOpenGL*)m_pFontNode->GetFont();
	if (pFont)
	{
	auto dwTextFormat = m_pFontNode->dwTextFormat;
	if (dwTextFormat != m_lastTextFormat)
	{
	m_isDirty = true;
	m_lastTextFormat = dwTextFormat;
	}
	}
	}

	if (IsDirty())
	{
	if (m_pFontNode != nullptr)
	{
	auto pFont = (SpriteFontEntityOpenGL*)m_pFontNode->GetFont();
	if (pFont)
	{
	auto dwTextFormat = m_pFontNode->dwTextFormat;
	m_pLettersInfo = pFont->GetLettersInfo(GetBuffer(), -1, m_rect, dwTextFormat);

	if (!m_pLettersInfo)
	{
	//OUTPUT_LOG("!!!!!!!!!! m_pLettersInfo is null");
	}
	else
	{
	m_isDirty = false;
	}
	return;
	}
	}

	m_pLettersInfo = nullptr;
	}
	*/
}
#endif
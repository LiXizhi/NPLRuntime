//----------------------------------------------------------------------
// Class:	CGUIUniBuffer
// Authors:	Liu Weili, LiXizhi
// Company: ParaEngine
// Date:	2005.8.3
// Revised: 2010.3.13
// Desc: used by GUIEditbox and GUIIMEEditbox
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "GUIUniBuffer.h"
#ifdef USE_DIRECTX_RENDERER
#include "MiscEntity.h"
#include <strsafe.h>
#include "GUIResource.h"
#include "memdebug.h"

using namespace ParaEngine;
using namespace std;
#define UNISCRIBE_DLLNAME "\\usp10.dll"

#define GETPROCADDRESS( Module, APIName, Temp ) \
	Temp = GetProcAddress( Module, #APIName ); \
	if( Temp ) \
	*(FARPROC*)&_##APIName = Temp

#define PLACEHOLDERPROC( APIName ) \
	_##APIName = Dummy_##APIName

#define DXUT_MAX_EDITBOXLENGTH 0xFFFF

// Static member initialization
HINSTANCE CUniLine::s_hDll = NULL;
HRESULT (WINAPI *CUniLine::_ScriptApplyDigitSubstitution)( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* ) = Dummy_ScriptApplyDigitSubstitution;
HRESULT (WINAPI *CUniLine::_ScriptStringAnalyse)( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* ) = Dummy_ScriptStringAnalyse;
HRESULT (WINAPI *CUniLine::_ScriptStringCPtoX)( SCRIPT_STRING_ANALYSIS, int, BOOL, int* ) = Dummy_ScriptStringCPtoX;
HRESULT (WINAPI *CUniLine::_ScriptStringXtoCP)( SCRIPT_STRING_ANALYSIS, int, int*, int* ) = Dummy_ScriptStringXtoCP;
HRESULT (WINAPI *CUniLine::_ScriptStringFree)( SCRIPT_STRING_ANALYSIS* ) = Dummy_ScriptStringFree;
const SCRIPT_LOGATTR* (WINAPI *CUniLine::_ScriptString_pLogAttr)( SCRIPT_STRING_ANALYSIS ) = Dummy_ScriptString_pLogAttr;
const int* (WINAPI *CUniLine::_ScriptString_pcOutChars)( SCRIPT_STRING_ANALYSIS ) = Dummy_ScriptString_pcOutChars;
bool (WINAPI *CUniLine::_GetTextMetrics)(HDC,	LPTEXTMETRIC)=Dummy_GetTextMetrics;


//--------------------------------------------------------------------------------------
// CUniLine class
//--------------------------------------------------------------------------------------
void CUniLine::Initialize()
{
	if( s_hDll ) // Only need to do once
		return;

	TCHAR wszPath[MAX_PATH+1];
	//DXUtil_ConvertAnsiStringToWideCch

	//	TCHAR temp1[MAX_PATH];

	if( !::GetSystemDirectory( wszPath, MAX_PATH+1 ) )
		return;

	// Verify whether it is safe to concatenate these strings
	int len1 = lstrlen(wszPath);
	//	DXUtil_ConvertWideStringToAnsiCch(temp1,UNISCRIBE_DLLNAME,MAX_PATH);

	int len2 = lstrlen(UNISCRIBE_DLLNAME);
	if (len1 + len2 > MAX_PATH)
	{
		return;
	}

	// We have verified that the concatenated string will fit into wszPath,
	// so it is safe to concatenate them.
	StringCchCat( wszPath, MAX_PATH, UNISCRIBE_DLLNAME );

	s_hDll = LoadLibrary( wszPath );
	if( s_hDll )
	{
		FARPROC Temp;
		GETPROCADDRESS( s_hDll, ScriptApplyDigitSubstitution, Temp );
		GETPROCADDRESS( s_hDll, ScriptStringAnalyse, Temp );
		GETPROCADDRESS( s_hDll, ScriptStringCPtoX, Temp );
		GETPROCADDRESS( s_hDll, ScriptStringXtoCP, Temp );
		GETPROCADDRESS( s_hDll, ScriptStringFree, Temp );
		GETPROCADDRESS( s_hDll, ScriptString_pLogAttr, Temp );
		GETPROCADDRESS( s_hDll, ScriptString_pcOutChars, Temp );
		GETPROCADDRESS( s_hDll, GetTextMetrics, Temp );
	}
}


//--------------------------------------------------------------------------------------
void CUniLine::Uninitialize()
{
	if( s_hDll )
	{
		PLACEHOLDERPROC( ScriptApplyDigitSubstitution );
		PLACEHOLDERPROC( ScriptStringAnalyse );
		PLACEHOLDERPROC( ScriptStringCPtoX );
		PLACEHOLDERPROC( ScriptStringXtoCP );
		PLACEHOLDERPROC( ScriptStringFree );
		PLACEHOLDERPROC( ScriptString_pLogAttr );
		PLACEHOLDERPROC( ScriptString_pcOutChars );
		PLACEHOLDERPROC( GetTextMetrics );

		FreeLibrary( s_hDll );
		s_hDll = NULL;
	}
}

//--------------------------------------------------------------------------------------
bool CUniLine::SetBufferSize( int nNewSize )
{
	// If the current size is already the maximum allowed,
	// we can't possibly allocate more.
	if( m_nBufferSize == DXUT_MAX_EDITBOXLENGTH )
		return false;
	if(m_nBufferSize > nNewSize && nNewSize >=0)
	{
		return true;
	}
	
	int nAllocateSize = ( nNewSize == -1 || nNewSize < m_nBufferSize * 2 ) ? ( m_nBufferSize ? m_nBufferSize * 2 : 16 ) : nNewSize * 2;

	// Cap the buffer size at the maximum allowed.
	if( nAllocateSize > DXUT_MAX_EDITBOXLENGTH )
		nAllocateSize = DXUT_MAX_EDITBOXLENGTH;

	char16_t *pTempBuffer = new char16_t[nAllocateSize];
	if( !pTempBuffer )
		return false;
	if( m_pwszBuffer )
	{
		if(m_nBufferSize > 0)
		{
			CopyMemory(pTempBuffer, m_pwszBuffer, m_nBufferSize * sizeof(char16_t));
		}
		delete[] m_pwszBuffer;
	}
	else
	{
		ZeroMemory( pTempBuffer, sizeof(char) * nAllocateSize );
	}

	m_pwszBuffer = pTempBuffer;
	m_nBufferSize = nAllocateSize;
	
	return true;
}

//--------------------------------------------------------------------------------------
// Uniscribe -- Analyse() analyses the string in the buffer
//--------------------------------------------------------------------------------------
HRESULT CUniLine::Analyse()
{
#ifdef USE_DIRECTX_RENDERER
	if( m_Analysis )
		_ScriptStringFree( &m_Analysis );

	SCRIPT_CONTROL ScriptControl; // For uniscribe
	SCRIPT_STATE   ScriptState;   // For uniscribe
	ZeroMemory( &ScriptControl, sizeof(ScriptControl) );
	ZeroMemory( &ScriptState, sizeof(ScriptState) );
	_ScriptApplyDigitSubstitution ( NULL, &ScriptControl, &ScriptState );

	if( m_pFontNode==0 || m_pFontNode->GetFont()==0)
	{
		OUTPUT_LOG("warning: m_pFontNode failed in CUniLine::Analyse\n ");
		return E_FAIL;
	}

	LPD3DXFONT pFontNode = ((SpriteFontEntityDirectX*)m_pFontNode->GetFont())->GetFont();
	HRESULT hr = _ScriptStringAnalyse(  pFontNode? pFontNode->GetDC() : NULL,
		m_pwszBuffer,
		lstrlenW( (WCHAR*)m_pwszBuffer ) + 1,  // NULL is also analyzed.
		lstrlenW((WCHAR*)m_pwszBuffer) * 3 / 2 + 16,
		-1,
		SSA_BREAK | SSA_GLYPHS | SSA_FALLBACK | SSA_LINK,
		0,
		&ScriptControl,
		&ScriptState,
		NULL,
		NULL,
		NULL,
		&m_Analysis );
	if( SUCCEEDED( hr )&&_GetTextMetrics((pFontNode ? pFontNode->GetDC() : NULL),&m_TextMetric) )
		m_bAnalyseRequired = false;  // Analysis is up-to-date

	return hr;
#else
	return E_FAIL;
#endif
	
}


//--------------------------------------------------------------------------------------
CUniLine::CUniLine( int nInitialSize )
{
	CUniLine::Initialize();  // ensure static vars are properly init'ed first

	m_nBufferSize = 0;
	m_pwszBuffer = NULL;
	m_bAnalyseRequired = true;
	m_Analysis = NULL;
	m_pFontNode = NULL;
	ZeroMemory(&m_TextMetric,sizeof(TEXTMETRIC));
	if( nInitialSize > 0 )
		SetBufferSize( nInitialSize );
}


//--------------------------------------------------------------------------------------
CUniLine::~CUniLine()
{
	SAFE_DELETE(m_pwszBuffer);
	if( m_Analysis )
		_ScriptStringFree( &m_Analysis );
}


//--------------------------------------------------------------------------------------
char16_t& CUniLine::operator[]( int n )  // No param checking
{
	// This version of operator[] is called only
	// if we are asking for write access, so
	// re-analysis is required.
	m_bAnalyseRequired = true;
	return m_pwszBuffer[n];
}


//--------------------------------------------------------------------------------------
void CUniLine::Clear()
{
	*m_pwszBuffer = (char16_t)(L'\0');
	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
// Inserts the char at specified index.
//--------------------------------------------------------------------------------------
bool CUniLine::InsertChar(int nIndex, char16_t wChar)
{
	PE_ASSERT( nIndex >= 0 );

	
	if( nIndex < 0 )
		return false;  // invalid index
	int nTextSize = lstrlenW((WCHAR*)m_pwszBuffer);
	if(nIndex > nTextSize)
		return false;

	// Check for maximum length allowed
	if( (nTextSize + 1) >= DXUT_MAX_EDITBOXLENGTH )
		return false;


	if( (nTextSize + 1) >= m_nBufferSize )
	{
		if( !SetBufferSize( -1 ) )
			return false;  // out of memory
	}

	PE_ASSERT( m_nBufferSize >= 2 );

	// Shift the characters after the index, start by copying the null terminator
	char16_t* dest = m_pwszBuffer + nTextSize +1;
	char16_t* stop = m_pwszBuffer + nIndex;
	char16_t* src = dest - 1;

	while( dest > stop )
	{
		*dest-- = *src--;
	}

	// Set new character
	m_pwszBuffer[ nIndex ] = wChar;
	m_bAnalyseRequired = true;

	return true;
}


//--------------------------------------------------------------------------------------
// Removes the char at specified index.
// If nIndex == -1, remove the last char.
//--------------------------------------------------------------------------------------
bool CUniLine::RemoveChar( int nIndex )
{
	if (!lstrlenW((WCHAR*)m_pwszBuffer) || nIndex < 0 || nIndex >= lstrlenW((WCHAR*)m_pwszBuffer))
		return false;  // Invalid index

	MoveMemory(m_pwszBuffer + nIndex, m_pwszBuffer + nIndex + 1, sizeof(char16_t)* (lstrlenW((WCHAR*)m_pwszBuffer) - nIndex));
	m_bAnalyseRequired = true;
	return true;
}


//--------------------------------------------------------------------------------------
// Inserts the first nCount characters of the string pStr at specified index.
// If nCount == -1, the entire string is inserted.
//--------------------------------------------------------------------------------------
bool CUniLine::InsertString(int nIndex, const char16_t *pStr, int nCount)
{
	PE_ASSERT( nIndex >= 0 );

	if (nIndex > lstrlenW((WCHAR*)m_pwszBuffer))
		return false;  // invalid index

	if( -1 == nCount )
		nCount = lstrlenW((WCHAR*)pStr);

	// Check for maximum length allowed
	if( GetTextSize() + nCount >= DXUT_MAX_EDITBOXLENGTH )
		return false;

	if (lstrlenW((WCHAR*)m_pwszBuffer) + nCount >= m_nBufferSize)
	{
		if (!SetBufferSize(lstrlenW((WCHAR*)m_pwszBuffer) + nCount + 1))
			return false;  // out of memory
	}

	MoveMemory(m_pwszBuffer + nIndex + nCount, m_pwszBuffer + nIndex, sizeof(char16_t)* (lstrlenW((WCHAR*)m_pwszBuffer) - nIndex + 1));
	CopyMemory(m_pwszBuffer + nIndex, pStr, nCount * sizeof(char16_t));
	m_bAnalyseRequired = true;

	return true;
}


bool CUniLine::InsertStringA( int nIndex, const char *pStr, int nCount )
{
	bool rvalue=InsertString(nIndex, (const char16_t*)StringHelper::MultiByteToWideChar(pStr, DEFAULT_GUI_ENCODING),nCount);
	return rvalue;
}


//--------------------------------------------------------------------------------------
bool CUniLine::SetTextA( LPCSTR szText )
{
	PE_ASSERT( szText != NULL );
	bool rvalue=SetText((const char16_t*)StringHelper::MultiByteToWideChar(szText, DEFAULT_GUI_ENCODING));
	return rvalue;
}


bool CUniLine::SetText( const char16_t* wszText )
{
	PE_ASSERT( wszText != NULL );

	int nRequired = int(wcslen((WCHAR*)wszText) + 1);

	// Check for maximum length allowed
	if( nRequired >= DXUT_MAX_EDITBOXLENGTH )
		return false;

	SetBufferSize(nRequired);

	// Check again in case out of memory occurred inside while loop.
	if( GetBufferSize() >= nRequired )
	{
		StringCchCopyW((WCHAR*)m_pwszBuffer, GetBufferSize(), (WCHAR*)wszText);
		m_bAnalyseRequired = true;
		return true;
	}
	else
		return false;
}


//--------------------------------------------------------------------------------------
HRESULT CUniLine::CPtoX( int nCP, BOOL bTrail, int *pX )
{
	PE_ASSERT( pX );
	*pX = 0;  // Default

	HRESULT hr = S_OK;
	if( m_bAnalyseRequired )
		hr = Analyse();

	if( SUCCEEDED( hr ) )
		hr = _ScriptStringCPtoX( m_Analysis, nCP, bTrail, pX );


	return hr;
}


//--------------------------------------------------------------------------------------
HRESULT CUniLine::XtoCP( int nX, int *pCP, int *pnTrail )
{
	PE_ASSERT( pCP && pnTrail );
	*pCP = 0; *pnTrail = FALSE;  // Default

	HRESULT hr = S_OK;
	if( m_bAnalyseRequired )
		hr = Analyse();

	if( SUCCEEDED( hr ) )
		hr = _ScriptStringXtoCP( m_Analysis, nX, pCP, pnTrail );

	// If the coordinate falls outside the text region, we
	// can get character positions that don't exist.  We must
	// filter them here and convert them to those that do exist.
	if( *pCP == -1 && *pnTrail == TRUE )
	{
		*pCP = 0; *pnTrail = FALSE;
	} else
		if( *pCP > lstrlenW( (WCHAR*)m_pwszBuffer ) && *pnTrail == FALSE )
		{
			*pCP = lstrlenW((WCHAR*)m_pwszBuffer); *pnTrail = TRUE;
		}

		return hr;
}


//--------------------------------------------------------------------------------------
void CUniLine::GetPriorItemPos( int nCP, int *pPrior )
{
	*pPrior = nCP;  // Default is the char itself

	if( m_bAnalyseRequired )
		if( FAILED( Analyse() ) )
			return;

	const SCRIPT_LOGATTR *pLogAttr = _ScriptString_pLogAttr( m_Analysis );
	if( !pLogAttr )
		return;

	if( !_ScriptString_pcOutChars( m_Analysis ) )
		return;
	int nInitial = *_ScriptString_pcOutChars( m_Analysis );
	if( nCP - 1 < nInitial )
		nInitial = nCP - 1;
	for( int i = nInitial; i > 0; --i )
		if( pLogAttr[i].fWordStop ||       // Either the fWordStop flag is set
			( !pLogAttr[i].fWhiteSpace &&  // Or the previous char is whitespace but this isn't.
			pLogAttr[i-1].fWhiteSpace ) )
		{
			*pPrior = i;
			return;
		}
		// We have reached index 0.  0 is always a break point, so simply return it.
		*pPrior = 0;
}


//--------------------------------------------------------------------------------------
void CUniLine::GetNextItemPos( int nCP, int *pPrior )
{
	*pPrior = nCP;  // Default is the char itself

	HRESULT hr = S_OK;
	if( m_bAnalyseRequired )
		hr = Analyse();
	if( FAILED( hr ) )
		return;

	const SCRIPT_LOGATTR *pLogAttr = _ScriptString_pLogAttr( m_Analysis );
	if( !pLogAttr )
		return;

	if( !_ScriptString_pcOutChars( m_Analysis ) )
		return;
	int nInitial = *_ScriptString_pcOutChars( m_Analysis );
	if( nCP + 1 < nInitial )
		nInitial = nCP + 1;
	for( int i = nInitial; i < *_ScriptString_pcOutChars( m_Analysis ) - 1; ++i )
	{
		if( pLogAttr[i].fWordStop )      // Either the fWordStop flag is set
		{
			*pPrior = i;
			return;
		}
		else
			if( pLogAttr[i].fWhiteSpace &&  // Or this whitespace but the next char isn't.
				!pLogAttr[i+1].fWhiteSpace )
			{
				*pPrior = i+1;  // The next char is a word stop
				return;
			}
	}
	// We have reached the end. It's always a word stop, so simply return it.
	*pPrior = *_ScriptString_pcOutChars( m_Analysis ) - 1;
}




int CUniLine::GetBufferA(std::string& out) const
{
	return StringHelper::UTF16ToUTF8(GetBuffer(), out) ? (int) out.size() : 0;
}

int ParaEngine::CUniLine::GetTextSize() const
{
	return lstrlenW((WCHAR*)m_pwszBuffer);
}

bool CUniLine::IsEmpty()
{
	return (m_nBufferSize == 0 || (m_pwszBuffer && m_pwszBuffer[0] == (L'\0')));
}

int ParaEngine::CUniLine::GetBufferSize() const
{
	return m_nBufferSize;
}

const char16_t* ParaEngine::CUniLine::GetBuffer() const
{
	return m_pwszBuffer;
}

//--------------------------------------------------------------------------------------
// CUniBuffer class
//--------------------------------------------------------------------------------------

void CUniBuffer::Initialize()
{
	CUniLine::Initialize();
}


//--------------------------------------------------------------------------------------
void CUniBuffer::Uninitialize()
{
	CUniLine::Uninitialize();
}

//--------------------------------------------------------------------------------------
CUniBuffer::CUniBuffer( int nInitialSize )
{
	CUniBuffer::Initialize();  // ensure static vars are properly init'ed first

	m_bMultiline=false;
	CUniLine* line=new CUniLine(nInitialSize);
	m_lines.push_back(line);
}


//--------------------------------------------------------------------------------------
CUniBuffer::~CUniBuffer()
{
	Clear();
}


//--------------------------------------------------------------------------------------
char16_t& CUniBuffer::operator[](int n)  // No param checking
{
	// This version of operator[] is called only
	// if we are asking for write access, so
	// re-analysis is required.
	int charpos;
	if ((charpos=GetLineAt(n))==-1) {
		CUniLine *temp=m_lines.back();
		return (*temp)[temp->GetTextSize()-1];
	}
	else
		return (*GetCurLine())[charpos];

}
  
bool CUniBuffer::SetBufferSize( int nNewSize )
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
	for (iter=m_lines.begin();iter!=m_lines.end();iter++) {
		delete (*iter);
	}
	m_lines.clear();

}

int CUniBuffer::GetLineAt(int nIndex)
{
	int charcount=0;
	list<CUniLine*>::iterator iter;
	for (iter=m_lines.begin();iter!=m_lines.end()&&charcount<=nIndex;iter++){
		charcount+=(*iter)->GetTextSize();
	};
	if (charcount>=nIndex) {
		iter--;
		charcount-=(*iter)->GetTextSize();
		m_curLine=(*iter);
		return nIndex-charcount;
	}
	return -1;
}

int CUniBuffer::GetTextSize()const
{
	int charcount=0;
	list<CUniLine*>::const_iterator iter;
	for (iter=m_lines.begin();iter!=m_lines.end();iter++){
		charcount+=(*iter)->GetTextSize();
	};
	return charcount;
}
//--------------------------------------------------------------------------------------
// Inserts the char at specified index.
//--------------------------------------------------------------------------------------
bool CUniBuffer::InsertChar(int nIndex, char16_t wChar)
{
	PE_ASSERT( nIndex >= 0 );

	//if( nIndex < 0 || nIndex > lstrlenW( m_pwszBuffer ) )
	//	return false;  // invalid index

	//// Check for maximum length allowed
	//if( GetTextSize() + 1 >= DXUT_MAX_EDITBOXLENGTH )
	//	return false;
	int charpos;
	if ((charpos=GetLineAt(nIndex))==-1) {
		return false;
	}
	else
		return GetCurLine()->InsertChar(charpos,wChar);
}


//--------------------------------------------------------------------------------------
// Removes the char at specified index.
//--------------------------------------------------------------------------------------
bool CUniBuffer::RemoveChar( int nIndex )
{
	int charpos;
	if ((charpos=GetLineAt(nIndex))==-1) {
		return false;
	}
	else
		return GetCurLine()->RemoveChar(charpos);
}


//--------------------------------------------------------------------------------------
// Inserts the first nCount characters of the string pStr at specified index.
// If nCount == -1, the entire string is inserted.
//--------------------------------------------------------------------------------------
bool CUniBuffer::InsertString( int nIndex, const char16_t *pStr, int nCount )
{
	PE_ASSERT( nIndex >= 0 );

	int charpos;
	if ((charpos=GetLineAt(nIndex))==-1) {
		return false;
	}
	else
		return GetCurLine()->InsertString(charpos,pStr,nCount);
	
}


bool CUniBuffer::InsertStringA( int nIndex, const char *pStr, int nCount )
{
	bool rvalue=InsertString(nIndex, (const char16_t*)StringHelper::MultiByteToWideChar(pStr, DEFAULT_GUI_ENCODING),nCount);
	return rvalue;
}


//--------------------------------------------------------------------------------------
bool CUniBuffer::SetTextA( const char* szText )
{
	PE_ASSERT( szText != NULL );
	bool rvalue=SetText((const char16_t*)StringHelper::MultiByteToWideChar(szText, DEFAULT_GUI_ENCODING));
	return rvalue;
}


bool CUniBuffer::SetText( const char16_t* wszText )
{
	PE_ASSERT( wszText != NULL );
	return m_lines.front()->SetText(wszText);
	//TODO: multiline support

}


//--------------------------------------------------------------------------------------
HRESULT CUniBuffer::CPtoXY( int nCP, BOOL bTrail, int *pX, int *pY )
{
	PE_ASSERT( pX&&pY );
	if(nCP<0||!m_bMultiline){
		*pY=0;
		return m_lines.front()->CPtoX(nCP,bTrail,pX);
	}
	int charpos;
	charpos=GetLineAt(nCP);
	list<CUniLine*>::iterator iter;
	CUniLine *line;
	int y=0;
	if ((charpos)==-1) {
		for (iter=m_lines.begin();iter!=m_lines.end();iter++) {
			line=(*iter);
			y+=line->GetTextMetric().tmInternalLeading+line->GetTextMetric().tmExternalLeading+line->GetTextMetric().tmHeight;
		}
		*pY=y;
		return m_lines.back()->CPtoX(charpos,bTrail,pX);
	}
	else{
		for (iter=m_lines.begin();(*iter)!=GetCurLine();iter++) {
			line=(*iter);
			y+=line->GetTextMetric().tmInternalLeading+line->GetTextMetric().tmExternalLeading+line->GetTextMetric().tmHeight;
		}
		*pY=y;
		return GetCurLine()->CPtoX(charpos,bTrail,pX);
	}
}

//--------------------------------------------------------------------------------------
HRESULT CUniBuffer::XYtoCP( int nX, int nY, int *pCP, int *pnTrail )
{
	PE_ASSERT( pCP && pnTrail );
	
	return m_lines.front()->XtoCP(nX,pCP,pnTrail);
	//TODO: multiline support
}


//--------------------------------------------------------------------------------------
void CUniBuffer::GetPriorItemPos( int nCP, int *pPrior )
{
	m_lines.front()->GetPriorItemPos(nCP,pPrior);
	//TODO: multiline support

}

//--------------------------------------------------------------------------------------
void CUniBuffer::GetNextItemPos( int nCP, int *pPrior )
{
	m_lines.front()->GetNextItemPos(nCP,pPrior);
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
	for (iter = m_lines.begin(); iter != m_lines.end(); iter++){
		if(!((*iter)->IsEmpty()))
			return false;
	};
	return true;
}

#else

int ParaEngine::CUniBuffer::GetBufferA(std::string& out) const
{
	out = m_utf8Text;
	return 0;
}

bool ParaEngine::CUniBuffer::InsertChar(int nIndex, char16_t wchar)
{
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
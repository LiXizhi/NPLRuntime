#pragma once
#ifdef USE_DIRECTX_RENDERER
#include <list>
#include <usp10.h>
#include <dimm.h>
namespace ParaEngine
{
	struct GUIFontElement;

	class CUniLine{
	public:
		CUniLine( int nInitialSize = 0 );
		~CUniLine();

		static void Initialize();
		static void Uninitialize();

		int  GetBufferSize() const;
		bool SetBufferSize( int nSize );
		int  GetTextSize()  const;
		const char16_t* GetBuffer() const;
		/**
		@param nLength:	[in] Specifies the size, in bytes, of the buffer pointed to by the lpMultiByteStr parameter. If this value is zero, the function returns the number of bytes required for the buffer. (In this case, the lpMultiByteStr buffer is not used.) 
		@return:
		If the function succeeds, and nLength is nonzero, the return value is the number of bytes written to the buffer pointed to by szText. The number includes the byte for the null terminator.
		If the function succeeds, and nLength is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string. 
		*/
		int GetBufferA(std::string& out) const;

		const char16_t& operator[]( int n ) const { return m_pwszBuffer[n]; }
		char16_t& operator[](int n);
		GUIFontElement* GetFontNode() const{ return m_pFontNode; }
		void SetFontNode( GUIFontElement *pFontNode ) { m_pFontNode = pFontNode; }
		void Clear();

		bool InsertChar(int nIndex, char16_t wchar); // Inserts the char at specified index. If nIndex == -1, insert to the end.
		bool RemoveChar( int nIndex );  // Removes the char at specified index. If nIndex == -1, remove the last char.
		bool InsertString(int nIndex, const char16_t *pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool InsertStringA(int nIndex, const char*pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool SetTextA( LPCSTR szText );
		bool SetText( const char16_t* wszText );
		bool IsEmpty();
		// Uniscribe
		HRESULT CPtoX( int nCP, BOOL bTrail, int *pX );
		HRESULT XtoCP( int nX, int *pCP, int *pnTrail );
		void GetPriorItemPos( int nCP, int *pPrior );
		void GetNextItemPos( int nCP, int *pPrior );
		const TEXTMETRIC& GetTextMetric(){
			if( m_bAnalyseRequired )
				Analyse();
			return m_TextMetric;
		}
		int GetHeight()const;
	private:
		HRESULT Analyse();      // Uniscribe -- Analyse() analyses the string in the buffer

		char16_t* m_pwszBuffer;    // Buffer to hold text
		int    m_nBufferSize;   // Size of the buffer allocated, in characters

		// Uniscribe-specific
		GUIFontElement* m_pFontNode;          // Font node for the font that this buffer uses
		bool m_bAnalyseRequired;            // True if the string has changed since last analysis.
		SCRIPT_STRING_ANALYSIS m_Analysis;  // Analysis for the current string
		TEXTMETRIC m_TextMetric;			// analysis for the current font
	private:
		// Empty implementation of the Uniscribe API
		static HRESULT WINAPI Dummy_ScriptApplyDigitSubstitution( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* ) { return E_NOTIMPL; }
		static HRESULT WINAPI Dummy_ScriptStringAnalyse( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* ) { return E_NOTIMPL; }
		static HRESULT WINAPI Dummy_ScriptStringCPtoX( SCRIPT_STRING_ANALYSIS, int, BOOL, int* ) { return E_NOTIMPL; }
		static HRESULT WINAPI Dummy_ScriptStringXtoCP( SCRIPT_STRING_ANALYSIS, int, int*, int* ) { return E_NOTIMPL; }
		static HRESULT WINAPI Dummy_ScriptStringFree( SCRIPT_STRING_ANALYSIS* ) { return E_NOTIMPL; }
		static const SCRIPT_LOGATTR* WINAPI Dummy_ScriptString_pLogAttr( SCRIPT_STRING_ANALYSIS ) { return NULL; }
		static const int* WINAPI Dummy_ScriptString_pcOutChars( SCRIPT_STRING_ANALYSIS ) { return NULL; }
		static bool WINAPI Dummy_GetTextMetrics(HDC, LPTEXTMETRIC){return true;}
		// Function pointers
		static HRESULT (WINAPI *_ScriptApplyDigitSubstitution)( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*, SCRIPT_STATE* );
		static HRESULT (WINAPI *_ScriptStringAnalyse)( HDC, const void *, int, int, int, DWORD, int, SCRIPT_CONTROL*, SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*, SCRIPT_STRING_ANALYSIS* );
		static HRESULT (WINAPI *_ScriptStringCPtoX)( SCRIPT_STRING_ANALYSIS, int, BOOL, int* );
		static HRESULT (WINAPI *_ScriptStringXtoCP)( SCRIPT_STRING_ANALYSIS, int, int*, int* );
		static HRESULT (WINAPI *_ScriptStringFree)( SCRIPT_STRING_ANALYSIS* );
		static const SCRIPT_LOGATTR* (WINAPI *_ScriptString_pLogAttr)( SCRIPT_STRING_ANALYSIS );
		static const int* (WINAPI *_ScriptString_pcOutChars)( SCRIPT_STRING_ANALYSIS );
		static bool (WINAPI *_GetTextMetrics)(HDC, LPTEXTMETRIC);
		static HINSTANCE s_hDll;  // Uniscribe DLL handle
	};

	class CUniBuffer
	{
	public:
		CUniBuffer( int nInitialSize = 1 );
		~CUniBuffer();

		static void Initialize();
		static void Uninitialize();

		int  GetBufferSize() const{ 
			if (!m_bMultiline) {
				return m_lines.front()->GetBufferSize();
            }
			return -1;
		}
		bool SetBufferSize( int nSize );
		int  GetTextSize()const;
		const char16_t* GetBuffer()const { 
			if (!m_bMultiline) {
				return m_lines.front()->GetBuffer();
			}
			return NULL;
		}
		/**
		@param nLength:	[in] Specifies the size, in bytes, of the buffer pointed to by the lpMultiByteStr parameter. If this value is zero, the function returns the number of bytes required for the buffer. (In this case, the lpMultiByteStr buffer is not used.) 
		@return:
		If the function succeeds, and nLength is nonzero, the return value is the number of bytes written to the buffer pointed to by szText. The number includes the byte for the null terminator.
		If the function succeeds, and nLength is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string. 
		*/
		int GetBufferA(std::string& out) const;
		char16_t& operator[]( int n );
		GUIFontElement* GetFontNode() const{
			if (!m_bMultiline) {
				return m_lines.front()->GetFontNode();
			}
			return NULL;
		}
		void SetFontNode( GUIFontElement *pFontNode ) { 
			if (!m_bMultiline) {
				m_lines.front()->SetFontNode(pFontNode);
			}
		}
		void Clear();

		bool InsertChar(int nIndex, char16_t wchar); // Inserts the char at specified index. If nIndex == -1, insert to the end.
		bool RemoveChar( int nIndex );  // Removes the char at specified index. If nIndex == -1, remove the last char.
		bool InsertString(int nIndex, const char16_t *pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool InsertStringA( int nIndex, const char *pStr, int nCount = -1 );  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool SetTextA( const char* szText );
		bool SetText( const char16_t* wszText );

		// Uniscribe
		HRESULT CPtoXY( int nCP, BOOL bTrail, int *pX, int *pY);
		HRESULT XYtoCP( int nX, int nY, int *pCP, int *pnTrail);
		void GetPriorItemPos( int nCP, int *pPrior );
		void GetNextItemPos( int nCP, int *pPrior );
		
		bool GetMultiline()const{return m_bMultiline;}
		void SetMultiline(bool multiline){m_bMultiline=multiline;};

		/**
		* Finds which line the nIndex character is in;
		* It will update the current line property to the line we found;
		* @param nIndex: the character index;
		* @return: return the character's index in the line we find. return -1 if the nIndex is out of range.
		*/
		int GetLineAt(int nIndex);

		CUniLine *GetCurLine(){return m_curLine;};

		bool IsEmpty();
	private:
		
		
		bool m_bMultiline;
		CUniLine* m_curLine; 
		// Uniscribe-specific
		//GUIFontElement* m_pFontNode;          // Font node for the font that this buffer uses
		//bool m_bAnalyseRequired;            // True if the string has changed since last analysis.
		//SCRIPT_STRING_ANALYSIS m_Analysis;  // Analysis for the current string
		//TEXTMETRICW m_TextMetric;			// analysis for the current font
		list<CUniLine*> m_lines;

	};
}
#else
namespace ParaEngine
{
	/** Null implementation. */
	class CUniBuffer
	{
	public:
		static void Initialize(){};
		static void Uninitialize(){};
		int  GetTextSize()const;
		const char16_t* GetBuffer()const;

		char16_t& operator[](int n);
		/**
		@param nLength:	[in] Specifies the size, in bytes, of the buffer pointed to by the lpMultiByteStr parameter. If this value is zero, the function returns the number of bytes required for the buffer. (In this case, the lpMultiByteStr buffer is not used.)
		@return:
		If the function succeeds, and nLength is nonzero, the return value is the number of bytes written to the buffer pointed to by szText. The number includes the byte for the null terminator.
		If the function succeeds, and nLength is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string.
		*/
		int GetBufferA(std::string& out) const;
		// Inserts the char at specified index. If nIndex == -1, insert to the end.
		bool InsertChar(int nIndex, char16_t wchar); 
		// Removes the char at specified index. If nIndex == -1, remove the last char.
		bool RemoveChar(int nIndex);  
		// Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool InsertString(int nIndex, const char16_t *pStr, int nCount = -1);  
		// Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool InsertStringA(int nIndex, const char *pStr, int nCount = -1);
		bool SetTextA(LPCSTR szText);
		bool SetText(const char16_t* wszText);
		void Clear();

		HRESULT CPtoXY(int nCP, BOOL bTrail, int *pX, int *pY);
		HRESULT XYtoCP(int nX, int nY, int *pCP, int *pnTrail);
		void GetPriorItemPos(int nCP, int *pPrior);;
		void GetNextItemPos(int nCP, int *pPrior);;

		bool GetMultiline()const{ return false; }
		void SetMultiline(bool multiline){};

		const std::string& GetUtf8Text() const { return m_utf8Text; }
		const std::u16string& GetUtf16Text() const { return m_utf16Text; }
		
		bool IsEmpty();
	protected:
		std::string m_utf8Text;
		std::u16string m_utf16Text;
	};
}
#endif
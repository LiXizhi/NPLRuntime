#pragma once
#ifdef USE_OPENGL_RENDERER

#include "SpriteFontEntity.h"
#include <vector>

namespace ParaEngine {

	struct GUIFontElement;
	class CUniBuffer;
	struct SpriteFontEntityOpenGL;

	class CUniLine
	{
	public:
		CUniLine(CUniBuffer* pParent, int nInitialSize = 0);
		~CUniLine();

		int  GetTextSize()  const;

		const char16_t* GetBuffer() const;

		const std::string& GetUtf8Text() const;
		const std::u16string& GetUtf16Text() const;

		/**
		@param nLength:	[in] Specifies the size, in bytes, of the buffer pointed to by the lpMultiByteStr parameter. If this value is zero, the function returns the number of bytes required for the buffer. (In this case, the lpMultiByteStr buffer is not used.)
		@return:
		If the function succeeds, and nLength is nonzero, the return value is the number of bytes written to the buffer pointed to by szText. The number includes the byte for the null terminator.
		If the function succeeds, and nLength is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string.
		*/
		int GetBufferA(std::string& out) const;

		const char16_t& operator[](size_t n) const { return m_utf16Text[n]; }
		void Clear();

		bool InsertChar(int nIndex, char16_t wchar); // Inserts the char at specified index. If nIndex == -1, insert to the end.
		bool RemoveChar(int nIndex);  // Removes the char at specified index. If nIndex == -1, remove the last char.
		bool ReplaceChar(int nIndex, char16_t wchar);
		bool InsertString(int nIndex, const char16_t *pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool InsertStringA(int nIndex, const char*pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool SetTextA(LPCSTR szText);
		bool SetText(const char16_t* wszText);
		bool IsEmpty();

		// Uniscribe
		HRESULT CPtoX(int nCP, BOOL bTrail, int *pX);
		HRESULT XtoCP(int nX, int *pCP, int *pnTrail);

		void GetPriorItemPos(int nCP, int *pPrior);
		void GetNextItemPos(int nCP, int *pPrior);

		int GetHeight();

		SpriteFontEntityOpenGL* GetFontEntity() const;

		bool IsDirty() const;
		void SetDirty();

	private:
		void updateLettersInfo();
	private:
		std::string m_utf8Text;
		std::u16string m_utf16Text;
		CUniBuffer* m_pParent;
		bool m_isDirty;
		std::vector<Vector2> m_lettersInfo;
	};



	class CUniBuffer
	{
	public:
		static void Initialize() {};
		static void Uninitialize() {};

	public:
		CUniBuffer(int nInitialSize = 1);
		~CUniBuffer();

		int  GetTextSize()const;
		const char16_t* GetBuffer()const;

		/**
		@param nLength:	[in] Specifies the size, in bytes, of the buffer pointed to by the lpMultiByteStr parameter. If this value is zero, the function returns the number of bytes required for the buffer. (In this case, the lpMultiByteStr buffer is not used.)
		@return:
		If the function succeeds, and nLength is nonzero, the return value is the number of bytes written to the buffer pointed to by szText. The number includes the byte for the null terminator.
		If the function succeeds, and nLength is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string.
		*/
		int GetBufferA(std::string& out) const;

		const char16_t& operator[](size_t n);

		GUIFontElement* GetFontNode() const {
			return m_pFontNode;
		}

		void SetFontNode(GUIFontElement *pFontNode) {
			m_pFontNode = pFontNode;
		}


		const std::string& GetUtf8Text() const;
		const std::u16string& GetUtf16Text() const;

		void Clear();

		bool InsertChar(int nIndex, char16_t wchar); // Inserts the char at specified index. If nIndex == -1, insert to the end.
		bool RemoveChar(int nIndex);  // Removes the char at specified index. If nIndex == -1, remove the last char.
		bool ReplaceChar(int nIndex, char16_t wchar);
		bool InsertString(int nIndex, const char16_t *pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool InsertStringA(int nIndex, const char *pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
		bool SetTextA(const char* szText);
		bool SetText(const char16_t* wszText);

		// Uniscribe
		HRESULT CPtoXY(int nCP, BOOL bTrail, int *pX, int *pY);
		HRESULT XYtoCP(int nX, int nY, int *pCP, int *pnTrail);

		void GetPriorItemPos(int nCP, int *pPrior);
		void GetNextItemPos(int nCP, int *pPrior);

		bool GetMultiline()const;
		void SetMultiline(bool multiline);;

		/**
		* Finds which line the nIndex character is in;
		* It will update the current line property to the line we found;
		* @param nIndex: the character index;
		* @return: return the character's index in the line we find. return -1 if the nIndex is out of range.
		*/
		int GetLineAt(int nIndex);

		CUniLine *GetCurLine() const;;

		bool IsEmpty();

		void SetRect(const RECT& r) { m_rect = r;  };
		const RECT& GetRect() const { return m_rect; };


	private:
		bool m_bMultiline;
		CUniLine* m_curLine;
		std::list<CUniLine*> m_lines;
		RECT m_rect;
		// Uniscribe-specific
		GUIFontElement* m_pFontNode;          // Font node for the font that this buffer uses
	};

} // end namespace


#endif
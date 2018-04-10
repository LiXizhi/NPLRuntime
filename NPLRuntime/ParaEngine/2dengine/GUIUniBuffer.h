#pragma once

#if defined(USE_DIRECTX_RENDERER)
#include "GUIUniBufferDirectX.h"
#elif defined(USE_OPENGL_RENDERER)
#include "GUIUniBufferOpenGL.h"
#else

#include "SpriteFontEntity.h"
#include <vector>

namespace ParaEngine
{
	struct GUIFontElement;


	/** Null implementation. */
	class CUniBuffer
	{
	public:
		CUniBuffer();
	public:
		static void Initialize() {};
		static void Uninitialize() {};
		int  GetTextSize()const;
		const char16_t* GetBuffer()const;

		const char16_t& operator[](int n);
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
		bool ReplaceChar(int nIndex, char16_t wchar);
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

		bool GetMultiline()const { return false; }
		void SetMultiline(bool multiline) {};

		const std::string& GetUtf8Text() const { return m_utf8Text; }
		const std::u16string& GetUtf16Text() const { return m_utf16Text; }

		bool IsEmpty();

		void SetFontNode(GUIFontElement *pFontNode);
		void SetRect(const RECT& r);

		bool IsDirty() const;

	private:
		void updateLettersInfo();
	protected:
		std::string m_utf8Text;
		std::u16string m_utf16Text;
		GUIFontElement* m_pFontNode;
		RECT m_rect;
		bool m_isDirty;
		std::vector<GLLabel::LetterInfo>* m_pLettersInfo;
		DWORD m_lastTextFormat;
		float m_labelHeight;
		int* m_horizontalKernings;
	};
}
#endif
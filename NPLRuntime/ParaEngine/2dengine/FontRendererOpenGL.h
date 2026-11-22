#pragma once
#include "OpenGLWrapper/GLLabel.h"
#include <list>
#include <unordered_map>
#include <utility>
namespace ParaEngine
{
	class CSpriteRenderer;

	/** used for rendering labels 
	* rendering a given font from TTF format. It emulates the d3dxfont class.
	*/
	class CFontRendererOpenGL : public GLLabel
	{
	public:
		static CFontRendererOpenGL* create(const std::string& sFontName, int nFontSize, bool bBold = false);
		CFontRendererOpenGL();
		virtual ~CFontRendererOpenGL();
	public:
		bool DrawTextA(CSpriteRenderer* pSprite, const char* strText, RECT& rect, DWORD dwTextFormat, DWORD textColor);
		bool DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, RECT& rect, DWORD dwTextFormat, DWORD textColor);

		void SetUTF16Text(const char16_t* strText);

		bool UpdateLetterSprites(DWORD dwTextFormat);

		void RenderLetterSprites(CSpriteRenderer* pSprite, DWORD color);

		void DoRender(CSpriteRenderer* pSprite, DWORD color);

		virtual float GetFontScaling() const;

		void RendererRecreated();

	protected:
		struct CachedTextLayout
		{
			uint64_t keyHash = 0;
			std::u16string rawText;
			std::u16string formattedText;
			std::vector<LetterInfo> lettersInfo;
			Size contentSize;
			int currNumLines = 0;
			int limitShowCount = 0;
			int scaledWidth = 0;
			int scaledHeight = 0;
			DWORD format = 0;
			uint64_t atlasRevision = 0;
			bool lineBreakWithoutSpaces = false;
			bool clipEnabled = true;
		};

		GLFontAtlas* GetScaledFontAtlas(int nRealFontSize, CSpriteRenderer* pSprite);
		void ReleaseScaledFontAtlases();
		static int SelectRenderFontSize(float screenFontSize);

	protected:
		int m_nFontSize;
		// to minimize the total number of font used, the real font-size may be smaller than the m_nFontSize, and a scale factor is used during rendering. 
		int m_nRealFontSize;
		float m_fFontScaling;
		bool m_bUseCustomShader;
		uint64_t m_layoutAtlasRevision = 0;
		DWORD m_layoutFormat = 0;
		// Most recently used first. Keep glyphs across alternating label scales,
		// but bound the number of retained sizes when the camera keeps zooming.
		std::list<std::pair<int, GLFontAtlas*>> m_scaledFontAtlases;
		std::list<CachedTextLayout> m_layoutCache;
		std::unordered_multimap<uint64_t, std::list<CachedTextLayout>::iterator> m_layoutCacheIndex;
	};
}


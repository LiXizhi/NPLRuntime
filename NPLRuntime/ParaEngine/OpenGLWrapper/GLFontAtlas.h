#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <iconv.h>

#include "GLType.h"

namespace ParaEngine
{

	class Font;
	class GLTexture2D;
	class FontFreeType;

	class GLFontAtlas : public CRefCountedOne
	{
	public:
		static const int CacheTextureWidth;
		static const int CacheTextureHeight;
		static const int ColorPageSlotBase = 10000;
		static const char* CMD_PURGE_FONTATLAS;
		static const char* CMD_RESET_FONTATLAS;
		
		GLFontAtlas(Font &theFont);
		virtual ~GLFontAtlas();

		void addLetterDefinition(char32_t utf32Char, const FontLetterDefinition &letterDefinition);
		bool getLetterDefinitionForChar(char32_t utf32Char, FontLetterDefinition &letterDefinition);

		uint64_t getLayoutRevision() const { return _layoutRevision; }
		bool prepareLetterDefinitions(const std::u16string& utf16String);

		inline const std::unordered_map<size_t, GLTexture2D*>& getTextures() const { return _atlasTextures; }
		void  addTexture(GLTexture2D *texture, int slot);
		float getLineHeight() const { return _lineHeight; }
		void  setLineHeight(float newHeight);



		GLTexture2D* getTexture(int slot);
		const Font* getFont() const { return _font; }


		float getCommonLineHeight() const
		{
			return _lineHeight;
		}

		/** Removes textures atlas.
		 It will purge the textures atlas and if multiple texture exist in the FontAtlas.
		 */
		void purgeTexturesAtlas();

		/** sets font texture parameters:
		 - GL_TEXTURE_MIN_FILTER = GL_LINEAR
		 - GL_TEXTURE_MAG_FILTER = GL_LINEAR
		 */
		void setAntiAliasTexParameters();

		/** sets font texture parameters:
		- GL_TEXTURE_MIN_FILTER = GL_NEAREST
		- GL_TEXTURE_MAG_FILTER = GL_NEAREST
		*/
		void setAliasTexParameters();

	protected:
		void init();
		void reset();

		void releaseTextures();
		void ensureColorPageInitialized();

		void findNewCharacters(const std::u16string& u16Text, std::unordered_map<char32_t, char32_t>& charCodeMap);

		void conversionU16TOGB2312(const std::u16string& u16Text, std::unordered_map<char32_t, char32_t>& charCodeMap);

		/**
		 * Scale each font letter by scaleFactor.
		 *
		 * @param scaleFactor A float scale factor for scaling font letter info.
		 */
		void scaleFontLetterDefinition(float scaleFactor);

		std::unordered_map<size_t, GLTexture2D*> _atlasTextures;
		std::unordered_map<char32_t, FontLetterDefinition> _letterDefinitions;
		uint64_t _layoutRevision = 0;
		float _lineHeight;
		Font* _font;
		FontFreeType* _fontFreeType;
        iconv_t _iconv;

		// Dynamic GlyphCollection related stuff
		int _currentPage;
		unsigned char *_currentPageData;
		int _currentPageDataSize;
		float _currentPageOrigX;
		float _currentPageOrigY;
		int _letterPadding;
		int _letterEdgeExtend;

		// Lazy RGBA8 color atlas pages for system color emoji glyphs
		bool _colorPageInited = false;
		int _currentColorPage = 0;
		unsigned char* _currentColorPageData = nullptr;
		int _currentColorPageDataSize = 0;
		float _currentColorOrigX = 0.0f;
		float _currentColorOrigY = 0.0f;
		int _currColorLineHeight = 0;

		int _fontAscender;

		bool _antialiasEnabled;
		int _currLineHeight;

		friend class Label;
		bool _inited;
	};
}

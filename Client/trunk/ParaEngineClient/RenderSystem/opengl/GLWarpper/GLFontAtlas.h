#pragma once
#include <string>
#include <memory>
#include <unordered_map>

#include "GLType.h"
#include "GLFont.h"
#include "GLTexture2D.h"

namespace ParaEngine
{
	class GLTexture2D;
	class FontFreeType;

	class FontAtlas
	{
	public:
		static const int CacheTextureWidth;
		static const int CacheTextureHeight;
		static const char* CMD_PURGE_FONTATLAS;
		static const char* CMD_RESET_FONTATLAS;
		
		FontAtlas(GLFontPtr &theFont);
		virtual ~FontAtlas();

		void addLetterDefinition(char16_t utf16Char, const FontLetterDefinition &letterDefinition);
		bool getLetterDefinitionForChar(char16_t utf16Char, FontLetterDefinition &letterDefinition);

		bool prepareLetterDefinitions(const std::u16string& utf16String);

		inline const std::unordered_map<size_t, GLTexture2DPtr>& getTextures() const { return _atlasTextures; }
		void  addTexture(GLTexture2DPtr texture, int slot);
		float getLineHeight() const { return _lineHeight; }
		void  setLineHeight(float newHeight);



		GLTexture2DPtr getTexture(int slot);
		const GLFontPtr getFont() const { return _font; }


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
		void reset();

		void releaseTextures();

		void findNewCharacters(const std::u16string& u16Text, std::unordered_map<unsigned short, unsigned short>& charCodeMap);

		void conversionU16TOGB2312(const std::u16string& u16Text, std::unordered_map<unsigned short, unsigned short>& charCodeMap);

		/**
		 * Scale each font letter by scaleFactor.
		 *
		 * @param scaleFactor A float scale factor for scaling font letter info.
		 */
		void scaleFontLetterDefinition(float scaleFactor);

		std::unordered_map<size_t, GLTexture2DPtr> _atlasTextures;
		std::unordered_map<char16_t, FontLetterDefinition> _letterDefinitions;
		float _lineHeight;
		GLFontPtr _font;
		FontFreeType* _fontFreeType;
		void* _iconv;

		// Dynamic GlyphCollection related stuff
		int _currentPage;
		unsigned char *_currentPageData;
		int _currentPageDataSize;
		float _currentPageOrigX;
		float _currentPageOrigY;
		int _letterPadding;
		int _letterEdgeExtend;

		int _fontAscender;

		bool _antialiasEnabled;
		int _currLineHeight;

		friend class Label;
	};

	using FontAtlasPtr = std::shared_ptr<FontAtlas>;
}
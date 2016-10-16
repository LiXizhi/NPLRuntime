
#ifndef _CCFontAtlas_h_
#define _CCFontAtlas_h_

/// @cond DO_NOT_SHOW

#include <string>
#include <unordered_map>

#include "CCType.h"

NS_CC_BEGIN

class Font;
class Texture2D;
class FontFreeType;



class FontAtlas : public Ref
{
public:
    static const int CacheTextureWidth;
    static const int CacheTextureHeight;
    static const char* CMD_PURGE_FONTATLAS;
    static const char* CMD_RESET_FONTATLAS;
    /**
     * @js ctor
     */
    FontAtlas(Font &theFont);
    /**
     * @js NA
     * @lua NA
     */
    virtual ~FontAtlas();

    void addLetterDefinition(char16_t utf16Char, const FontLetterDefinition &letterDefinition);
    bool getLetterDefinitionForChar(char16_t utf16Char, FontLetterDefinition &letterDefinition);

    bool prepareLetterDefinitions(const std::u16string& utf16String);

    inline const std::unordered_map<ssize_t, Texture2D*>& getTextures() const{ return _atlasTextures;}
    void  addTexture(Texture2D *texture, int slot);
    float getLineHeight() const { return _lineHeight; }
    void  setLineHeight(float newHeight);



    Texture2D* getTexture(int slot);
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

    std::unordered_map<ssize_t, Texture2D*> _atlasTextures;
    std::unordered_map<char16_t, FontLetterDefinition> _letterDefinitions;
    float _lineHeight;
    Font* _font;
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

NS_CC_END

/// @endcond
#endif /* defined(__cocos2d_libs__CCFontAtlas__) */

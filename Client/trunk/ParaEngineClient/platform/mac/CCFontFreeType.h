
#ifndef _FontFreetype_h_
#define _FontFreetype_h_

#include "CCType.h"
#include "CCFont.h"

#include <string>


#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_IMAGE_H




NS_CC_BEGIN

class FontFreeType : public Font
{
public:
    static const int DistanceMapSpread;

    static FontFreeType* create(const std::string &fontName, float fontSize, GlyphCollection glyphs,
        const char *customGlyphs,bool distanceFieldEnabled = false,int outline = 0);

    static void shutdownFreeType();
    static void shutdownFreeType_exit();

    bool isDistanceFieldEnabled() const { return _distanceFieldEnabled;}

    float getOutlineSize() const { return _outlineSize; }

    void renderCharAt(unsigned char *dest,int posX, int posY, unsigned char* bitmap,long bitmapWidth,long bitmapHeight);

    FT_Encoding getEncoding() const { return _encoding; }

    int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const override;

    unsigned char* getGlyphBitmap(unsigned short theChar, long &outWidth, long &outHeight, Rect &outRect,int &xAdvance);

    int getFontAscender() const;
    const char* getFontFamily() const;

    virtual FontAtlas* createFontAtlas() override;
    virtual int getFontMaxHeight() const override { return _lineHeight; }

    static void releaseFont(const std::string &fontName);

private:
    static const char* _glyphASCII;
    static const char* _glyphNEHE;
    static FT_Library _FTlibrary;
    static bool _FTInitialized;

    FontFreeType(bool distanceFieldEnabled = false, int outline = 0);
    virtual ~FontFreeType();

    bool createFontObject(const std::string &fontName, float fontSize);

    bool initFreeType();
    FT_Library getFTLibrary();

    int getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const;
    unsigned char* getGlyphBitmapWithOutline(unsigned short code, FT_BBox &bbox);

    void setGlyphCollection(GlyphCollection glyphs, const char* customGlyphs = nullptr);
    const char* getGlyphCollection() const;

    FT_Face _fontRef;
    FT_Stroker _stroker;
    FT_Encoding _encoding;

    std::string _fontName;
    bool _distanceFieldEnabled;
    float _outlineSize;
    int _lineHeight;
    FontAtlas* _fontAtlas;

    GlyphCollection _usedGlyphs;
    std::string _customGlyphs;
};

/// @endcond

NS_CC_END

#endif

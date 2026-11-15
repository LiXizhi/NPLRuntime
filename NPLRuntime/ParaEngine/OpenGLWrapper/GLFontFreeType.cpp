#include "ParaEngine.h"
#include "edtaa3func.h"

#include "IO/FileUtils.h"
#include "util/StringHelper.h"
#include "GLFontFreeType.h"
#include "GLFontAtlas.h"

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

using namespace ParaEngine;

namespace
{
	inline unsigned short readBE16U(const unsigned char* p)
	{
		return static_cast<unsigned short>((static_cast<unsigned short>(p[0]) << 8) | static_cast<unsigned short>(p[1]));
	}

	inline short readBE16S(const unsigned char* p)
	{
		return static_cast<short>(readBE16U(p));
	}

	// Match Windows GDI (CreateFont with lfHeight = -ppem, DEFAULT_CHARSET) vertical cell metrics:
	// 1) SFNT VDMX table when a full-charset (bCharSet == 1) 1:1 aspect group has an exact ppem record.
	// 2) SFNT OS/2 usWinAscent / usWinDescent rounded to pixels.
	// 3) Rounded FreeType size->metrics fallback for non-SFNT fonts.
	void computeGdiCompatibleVerticalMetrics(FT_Face face, int ppem, int& outAscender, int& outLineHeight)
	{
		if (face && ppem > 0)
		{
			FT_ULong vdmxLen = 0;
			if (FT_Load_Sfnt_Table(face, TTAG_VDMX, 0, nullptr, &vdmxLen) == 0 && vdmxLen >= 6)
			{
				std::vector<unsigned char> vdmx(vdmxLen);
				if (FT_Load_Sfnt_Table(face, TTAG_VDMX, 0, vdmx.data(), &vdmxLen) == 0)
				{
					const unsigned short numRatios = readBE16U(vdmx.data() + 4);
					const size_t offsetArrayPos = 6u + static_cast<size_t>(numRatios) * 4u;
					if (offsetArrayPos + static_cast<size_t>(numRatios) * 2u <= vdmxLen)
					{
						int matchedGroupOffset = -1;
						for (unsigned short r = 0; r < numRatios; ++r)
						{
							const unsigned char* rat = vdmx.data() + 6u + static_cast<size_t>(r) * 4u;
							const unsigned char bCharSet = rat[0];
							const unsigned char xRatio = rat[1];
							const unsigned char yStart = rat[2];
							const unsigned char yEnd = rat[3];
							if (bCharSet != 1)
								continue;

							const unsigned short groupOff = readBE16U(vdmx.data() + offsetArrayPos + static_cast<size_t>(r) * 2u);
							if (xRatio == 1 && yStart == 1 && yEnd == 1)
							{
								matchedGroupOffset = static_cast<int>(groupOff);
								break;
							}
							if (matchedGroupOffset < 0 && xRatio == 0 && yStart == 0 && yEnd == 0)
							{
								matchedGroupOffset = static_cast<int>(groupOff);
							}
						}

						if (matchedGroupOffset >= 0 && static_cast<size_t>(matchedGroupOffset) + 4u <= vdmxLen)
						{
							const unsigned char* grp = vdmx.data() + matchedGroupOffset;
							const unsigned short recs = readBE16U(grp);
							const unsigned char startsz = grp[2];
							const unsigned char endsz = grp[3];
							if (ppem >= static_cast<int>(startsz) && ppem <= static_cast<int>(endsz) &&
								static_cast<size_t>(matchedGroupOffset) + 4u + static_cast<size_t>(recs) * 6u <= vdmxLen)
							{
								for (unsigned short k = 0; k < recs; ++k)
								{
									const unsigned char* entry = grp + 4u + static_cast<size_t>(k) * 6u;
									const unsigned short yPelHeight = readBE16U(entry);
									if (static_cast<int>(yPelHeight) == ppem)
									{
										const short yMax = readBE16S(entry + 2);
										const short yMin = readBE16S(entry + 4);
										if (yMax > yMin)
										{
											outAscender = static_cast<int>(yMax);
											outLineHeight = static_cast<int>(yMax) - static_cast<int>(yMin);
											return;
										}
										break;
									}
								}
							}
						}
					}
				}
			}

			const TT_OS2* os2 = static_cast<const TT_OS2*>(FT_Get_Sfnt_Table(face, FT_SFNT_OS2));
			if (os2 && face->units_per_EM > 0 && (os2->usWinAscent > 0 || os2->usWinDescent > 0))
			{
				const double scale = static_cast<double>(ppem) / static_cast<double>(face->units_per_EM);
				const int winAsc = static_cast<int>(std::floor(static_cast<double>(os2->usWinAscent) * scale + 0.5));
				const int winDesc = static_cast<int>(std::floor(static_cast<double>(os2->usWinDescent) * scale + 0.5));
				if (winAsc + winDesc > 0)
				{
					outAscender = winAsc;
					outLineHeight = winAsc + winDesc;
					return;
				}
			}
		}

		if (face && face->size)
		{
			outAscender = static_cast<int>((face->size->metrics.ascender + 32) >> 6);
			outLineHeight = static_cast<int>((face->size->metrics.height + 32) >> 6);
		}
		else
		{
			outAscender = ppem;
			outLineHeight = ppem;
		}
	}
}

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

FT_Library FontFreeType::_FTlibrary;
bool       FontFreeType::_FTInitialized = false;
const int  FontFreeType::DistanceMapSpread = 3;

const char* FontFreeType::_glyphASCII = "\"!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ ";
const char* FontFreeType::_glyphNEHE = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";

typedef struct _DataRef
{
	FileData data;
    unsigned int referenceCount;
}DataRef;

static std::unordered_map<std::string, DataRef> s_cacheFontData;

FontFreeType * FontFreeType::create(const std::string &fontName, float fontSize, GlyphCollection glyphs, const char *customGlyphs,bool distanceFieldEnabled /* = false */,int outline /* = 0 */, const std::string &fallbackFontName /* = "" */, bool bold /* = false */)
{
    FontFreeType *tempFont =  new FontFreeType(distanceFieldEnabled,outline);

    if (!tempFont)
        return nullptr;

    tempFont->_bold = bold;
    tempFont->setGlyphCollection(glyphs, customGlyphs);

    if (!tempFont->createFontObject(fontName, fontSize))
    {
        delete tempFont;
        return nullptr;
    }

    if (!fallbackFontName.empty() && fallbackFontName != fontName)
    {
        tempFont->createFallbackFontObject(fallbackFontName, fontSize);
    }

    return tempFont;
}

bool FontFreeType::initFreeType()
{
    if (_FTInitialized == false)
    {
        // begin freetype
        if (FT_Init_FreeType( &_FTlibrary ))
            return false;

        _FTInitialized = true;
    }

    return  _FTInitialized;
}

void FontFreeType::shutdownFreeType()
{
    if (_FTInitialized == true)
    {
        FT_Done_FreeType(_FTlibrary);
        s_cacheFontData.clear();
        _FTInitialized = false;
    }
}

void FontFreeType::shutdownFreeType_exit()
{
    if (_FTInitialized == true)
    {
        FT_Done_FreeType(_FTlibrary);

        auto item = s_cacheFontData.begin();
        while (s_cacheFontData.end() != item)
        {
            item->second.data.SetOwnBuffer(0,0);
            item++;
        }

        s_cacheFontData.clear();
        _FTInitialized = false;
    }
}


FT_Library FontFreeType::getFTLibrary()
{
    initFreeType();
    return _FTlibrary;
}

FontFreeType::FontFreeType(bool distanceFieldEnabled /* = false */,int outline /* = 0 */)
: _fontRef(nullptr)
, _fallbackFontRef(nullptr)
, _stroker(nullptr)
, _distanceFieldEnabled(distanceFieldEnabled)
, _bold(false)
, _outlineSize(0.0f)
, _ascender(0)
, _lineHeight(0)
, _fontAtlas(nullptr)
, _encoding(FT_ENCODING_UNICODE)
, _usedGlyphs(GlyphCollection::ASCII)
{
    if (outline > 0)
    {
        _outlineSize = outline * GL_CONTENT_SCALE_FACTOR();
        FT_Stroker_New(FontFreeType::getFTLibrary(), &_stroker);
        FT_Stroker_Set(_stroker,
            (int)(_outlineSize * 64),
            FT_STROKER_LINECAP_ROUND,
            FT_STROKER_LINEJOIN_ROUND,
            0);
    }
}

bool FontFreeType::createFontObject(const std::string &fontName, float fontSize)
{
    FT_Face face;
    // save font name locally
    _fontName = fontName;

    auto it = s_cacheFontData.find(fontName);
    if (it != s_cacheFontData.end())
    {
        (*it).second.referenceCount += 1;
    }
    else
    {
        s_cacheFontData[fontName].referenceCount = 1;
        s_cacheFontData[fontName].data = CFileUtils::GetDataFromFile(fontName.c_str());

        if (s_cacheFontData[fontName].data.isNull())
        {
            return false;
        }
    }

    if (FT_New_Memory_Face(getFTLibrary(), (const FT_Byte*)(s_cacheFontData[fontName].data.GetBytes()), s_cacheFontData[fontName].data.GetSize(), 0, &face ))
        return false;

    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
    {
        int foundIndex = -1;
        for (int charmapIndex = 0; charmapIndex < face->num_charmaps; charmapIndex++)
        {
            if (face->charmaps[charmapIndex]->encoding != FT_ENCODING_NONE)
            {
                foundIndex = charmapIndex;
                break;
            }
        }

        if (foundIndex == -1)
        {
            return false;
        }

        _encoding = face->charmaps[foundIndex]->encoding;
        if (FT_Select_Charmap(face, _encoding))
        {
            return false;
        }
    }

    // set the requested font size
    int dpi = 72;
    const float scaledFontSize = fontSize * GL_CONTENT_SCALE_FACTOR();
    int fontSizePoints = static_cast<int>(std::floor(64.f * scaledFontSize + 0.5f));
    if (FT_Set_Char_Size(face, fontSizePoints, fontSizePoints, dpi, dpi))
        return false;

    // store the face globally
    _fontRef = face;
    const int ppem = static_cast<int>(std::floor(scaledFontSize + 0.5f));
    computeGdiCompatibleVerticalMetrics(_fontRef, ppem, _ascender, _lineHeight);

    // done and good
    return true;
}

bool FontFreeType::createFallbackFontObject(const std::string &fallbackFontName, float fontSize)
{
    if (fallbackFontName.empty())
        return false;

    auto it = s_cacheFontData.find(fallbackFontName);
    if (it != s_cacheFontData.end())
    {
        (*it).second.referenceCount += 1;
    }
    else
    {
        s_cacheFontData[fallbackFontName].referenceCount = 1;
        s_cacheFontData[fallbackFontName].data = CFileUtils::GetDataFromFile(fallbackFontName.c_str());

        if (s_cacheFontData[fallbackFontName].data.isNull())
        {
            s_cacheFontData.erase(fallbackFontName);
            return false;
        }
    }

    auto releaseFallbackCache = [&]() {
        auto cacheIt = s_cacheFontData.find(fallbackFontName);
        if (cacheIt != s_cacheFontData.end())
        {
            if (cacheIt->second.referenceCount <= 1)
                s_cacheFontData.erase(cacheIt);
            else
                cacheIt->second.referenceCount -= 1;
        }
    };

    FT_Face face = nullptr;
    if (FT_New_Memory_Face(getFTLibrary(), (const FT_Byte*)(s_cacheFontData[fallbackFontName].data.GetBytes()), s_cacheFontData[fallbackFontName].data.GetSize(), 0, &face))
    {
        releaseFallbackCache();
        return false;
    }

    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
    {
        int foundIndex = -1;
        for (int charmapIndex = 0; charmapIndex < face->num_charmaps; charmapIndex++)
        {
            if (face->charmaps[charmapIndex]->encoding != FT_ENCODING_NONE)
            {
                foundIndex = charmapIndex;
                break;
            }
        }

        if (foundIndex == -1 || FT_Select_Charmap(face, face->charmaps[foundIndex]->encoding))
        {
            FT_Done_Face(face);
            releaseFallbackCache();
            return false;
        }
    }

    int dpi = 72;
    int fontSizePoints = static_cast<int>(std::floor(64.f * fontSize * GL_CONTENT_SCALE_FACTOR() + 0.5f));
    if (FT_Set_Char_Size(face, fontSizePoints, fontSizePoints, dpi, dpi))
    {
        FT_Done_Face(face);
        releaseFallbackCache();
        return false;
    }

    _fallbackFontName = fallbackFontName;
    _fallbackFontRef = face;
    return true;
}

FontFreeType::~FontFreeType()
{
    if (_FTInitialized)
    {
        if (_stroker)
        {
            FT_Stroker_Done(_stroker);
        }
        if (_fontRef)
        {
            FT_Done_Face(_fontRef);
        }
        if (_fallbackFontRef)
        {
            FT_Done_Face(_fallbackFontRef);
        }
    }

    if (!_fontName.empty())
    {
        auto it = s_cacheFontData.find(_fontName);
        if (it != s_cacheFontData.end())
        {
            if (it->second.referenceCount <= 1)
                s_cacheFontData.erase(it);
            else
                it->second.referenceCount -= 1;
        }
    }

    if (!_fallbackFontName.empty())
    {
        auto it = s_cacheFontData.find(_fallbackFontName);
        if (it != s_cacheFontData.end())
        {
            if (it->second.referenceCount <= 1)
                s_cacheFontData.erase(it);
            else
                it->second.referenceCount -= 1;
        }
    }
}

GLFontAtlas * FontFreeType::createFontAtlas()
{
    if (_fontAtlas == nullptr)
    {
        _fontAtlas = new (std::nothrow) GLFontAtlas(*this);
        if (_fontAtlas && _usedGlyphs != GlyphCollection::DYNAMIC)
        {
            std::u16string utf16;
            if (StringHelper::UTF8ToUTF16(getGlyphCollection(), utf16))
            {
                _fontAtlas->prepareLetterDefinitions(utf16);
            }
        }
        this->AddToAutoReleasePool();
    }

    return _fontAtlas;
}

int * FontFreeType::getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const
{
    if (!_fontRef)
        return nullptr;

    outNumLetters = static_cast<int>(text.length());

    if (!outNumLetters)
        return nullptr;

    int *sizes = new (std::nothrow) int[outNumLetters];
    if (!sizes)
        return nullptr;
    memset(sizes,0,outNumLetters * sizeof(int));

    bool hasKerning = FT_HAS_KERNING( _fontRef ) != 0;
    if (hasKerning)
    {
        for (int c = 1; c < outNumLetters; ++c)
        {
            sizes[c] = getHorizontalKerningForChars(text[c-1], text[c]);
        }
    }

    return sizes;
}

int  FontFreeType::getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const
{
    // get the ID to the char we need
    int glyphIndex1 = FT_Get_Char_Index(_fontRef, firstChar);

    if (!glyphIndex1)
        return 0;

    // get the ID to the char we need
    int glyphIndex2 = FT_Get_Char_Index(_fontRef, secondChar);

    if (!glyphIndex2)
        return 0;

    FT_Vector kerning;

    if (FT_Get_Kerning( _fontRef, glyphIndex1, glyphIndex2,  FT_KERNING_DEFAULT,  &kerning))
        return 0;

    return static_cast<int>((kerning.x >= 0) ? ((kerning.x + 32) >> 6) : -((-kerning.x + 32) >> 6));
}

int FontFreeType::getFontAscender() const
{
    return _ascender;
}

const char* FontFreeType::getFontFamily() const
{
    if (!_fontRef)
        return nullptr;

    return _fontRef->family_name;
}

unsigned char* FontFreeType::getGlyphBitmap(unsigned short theChar, long &outWidth, long &outHeight, CCRect &outRect,int &xAdvance)
{
    bool invalidChar = true;
    unsigned char* ret = nullptr;
    do
    {
        if (_fontRef == nullptr)
            break;

        FT_Face face = _fontRef;
        if (_fallbackFontRef != nullptr && FT_Get_Char_Index(_fontRef, theChar) == 0)
        {
            if (FT_Get_Char_Index(_fallbackFontRef, theChar) != 0)
            {
                face = _fallbackFontRef;
            }
        }

        const bool synthesizeBold = _bold && ((face->style_flags & FT_STYLE_FLAG_BOLD) == 0);
        FT_Int32 loadFlags = _distanceFieldEnabled
            ? (FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT)
            : FT_LOAD_NO_AUTOHINT;
        if (!synthesizeBold)
        {
            loadFlags |= FT_LOAD_RENDER;
        }

        if (FT_Load_Char(face, theChar, loadFlags))
            break;

        if (synthesizeBold)
        {
            FT_GlyphSlot_Embolden(face->glyph);
            if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
            {
                if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
                    break;
            }
        }

        auto& metrics = face->glyph->metrics;
        outRect.origin.x = face->glyph->bitmap_left;
        outRect.origin.y = -face->glyph->bitmap_top;
        outRect.size.width = static_cast<float>(face->glyph->bitmap.width);
        outRect.size.height = static_cast<float>(face->glyph->bitmap.rows);

        xAdvance = static_cast<int>((metrics.horiAdvance + 32) >> 6);

        outWidth  = face->glyph->bitmap.width;
        outHeight = face->glyph->bitmap.rows;
        ret = face->glyph->bitmap.buffer;

        if (_outlineSize > 0 && outWidth > 0 && outHeight > 0 && ret != nullptr)
        {
            auto copyBitmap = new (std::nothrow) unsigned char[outWidth * outHeight];
            memcpy(copyBitmap,ret,outWidth * outHeight * sizeof(unsigned char));

            FT_BBox bbox;
            auto outlineBitmap = getGlyphBitmapWithOutline(face, theChar, bbox);
            if(outlineBitmap == nullptr)
            {
                ret = nullptr;
                delete [] copyBitmap;
                break;
            }

            long glyphMinX = outRect.origin.x;
            long glyphMaxX = outRect.origin.x + outWidth;
            long glyphMinY = -outHeight - outRect.origin.y;
            long glyphMaxY = -outRect.origin.y;

            auto outlineMinX = bbox.xMin >> 6;
            auto outlineMaxX = bbox.xMax >> 6;
            auto outlineMinY = bbox.yMin >> 6;
            auto outlineMaxY = bbox.yMax >> 6;
            auto outlineWidth = outlineMaxX - outlineMinX;
            auto outlineHeight = outlineMaxY - outlineMinY;

            auto blendImageMinX = MIN(outlineMinX, glyphMinX);
            auto blendImageMaxY = MAX(outlineMaxY, glyphMaxY);
            auto blendWidth = MAX(outlineMaxX, glyphMaxX) - blendImageMinX;
            auto blendHeight = blendImageMaxY - MIN(outlineMinY, glyphMinY);

            outRect.origin.x = blendImageMinX;
            outRect.origin.y = -blendImageMaxY + _outlineSize;

            long index, index2;
            auto blendImage = new (std::nothrow) unsigned char[blendWidth * blendHeight * 2];
            memset(blendImage, 0, blendWidth * blendHeight * 2);

            auto px = outlineMinX - blendImageMinX;
            auto py = blendImageMaxY - outlineMaxY;
            for (int x = 0; x < outlineWidth; ++x)
            {
                for (int y = 0; y < outlineHeight; ++y)
                {
                    index = px + x + ((py + y) * blendWidth);
                    index2 = x + (y * outlineWidth);
                    blendImage[2 * index] = outlineBitmap[index2];
                }
            }

            px = glyphMinX - blendImageMinX;
            py = blendImageMaxY - glyphMaxY;
            for (int x = 0; x < outWidth; ++x)
            {
                for (int y = 0; y < outHeight; ++y)
                {
                    index = px + x + ((y + py) * blendWidth);
                    index2 = x + (y * outWidth);
                    blendImage[2 * index + 1] = copyBitmap[index2];
                }
            }

            outRect.size.width  =  blendWidth;
            outRect.size.height =  blendHeight;
            outWidth  = blendWidth;
            outHeight = blendHeight;

            delete [] outlineBitmap;
            delete [] copyBitmap;
            ret = blendImage;
        }

        invalidChar = false;
    } while (0);

    if (invalidChar)
    {
        outRect.size.width  = 0;
        outRect.size.height = 0;
        xAdvance = 0;

        return nullptr;
    }
    else
    {
       return ret;
    }
}

unsigned char * FontFreeType::getGlyphBitmapWithOutline(FT_Face face, unsigned short theChar, FT_BBox &bbox)
{
    unsigned char* ret = nullptr;
    if (face && FT_Load_Char(face, theChar, FT_LOAD_NO_BITMAP) == 0)
    {
        if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            FT_Glyph glyph;
            if (FT_Get_Glyph(face->glyph, &glyph) == 0)
            {
                FT_Glyph_StrokeBorder(&glyph, _stroker, 0, 1);
                if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    FT_Outline *outline = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
                    FT_Glyph_Get_CBox(glyph,FT_GLYPH_BBOX_GRIDFIT,&bbox);
                    long width = (bbox.xMax - bbox.xMin)>>6;
                    long rows = (bbox.yMax - bbox.yMin)>>6;

                    FT_Bitmap bmp;
                    bmp.buffer = new (std::nothrow) unsigned char[width * rows];
                    memset(bmp.buffer, 0, width * rows);
                    bmp.width = (int)width;
                    bmp.rows = (int)rows;
                    bmp.pitch = (int)width;
                    bmp.pixel_mode = FT_PIXEL_MODE_GRAY;
                    bmp.num_grays = 256;

                    FT_Raster_Params params;
                    memset(&params, 0, sizeof (params));
                    params.source = outline;
                    params.target = &bmp;
                    params.flags = FT_RASTER_FLAG_AA;
                    FT_Outline_Translate(outline,-bbox.xMin,-bbox.yMin);
                    FT_Outline_Render(_FTlibrary, outline, &params);

                    ret = bmp.buffer;
                }
                FT_Done_Glyph(glyph);
            }
        }
    }

    return ret;
}

unsigned char * makeDistanceMap( unsigned char *img, long width, long height)
{
    long pixelAmount = (width + 2 * FontFreeType::DistanceMapSpread) * (height + 2 * FontFreeType::DistanceMapSpread);

    short * xdist = (short *)  malloc( pixelAmount * sizeof(short) );
    short * ydist = (short *)  malloc( pixelAmount * sizeof(short) );
    double * gx   = (double *) calloc( pixelAmount, sizeof(double) );
    double * gy      = (double *) calloc( pixelAmount, sizeof(double) );
    double * data    = (double *) calloc( pixelAmount, sizeof(double) );
    double * outside = (double *) calloc( pixelAmount, sizeof(double) );
    double * inside  = (double *) calloc( pixelAmount, sizeof(double) );
    long i,j;

    // Convert img into double (data) rescale image levels between 0 and 1
    long outWidth = width + 2 * FontFreeType::DistanceMapSpread;
    for (i = 0; i < width; ++i)
    {
        for (j = 0; j < height; ++j)
        {
            data[j * outWidth + FontFreeType::DistanceMapSpread + i] = img[j * width + i] / 255.0;
        }
    }

    width += 2 * FontFreeType::DistanceMapSpread;
    height += 2 * FontFreeType::DistanceMapSpread;

    // Transform background (outside contour, in areas of 0's)
    computegradient( data, (int)width, (int)height, gx, gy);
    edtaa3(data, gx, gy, (int)width, (int)height, xdist, ydist, outside);
    for( i=0; i< pixelAmount; i++)
        if( outside[i] < 0.0 )
            outside[i] = 0.0;

    // Transform foreground (inside contour, in areas of 1's)
    for( i=0; i< pixelAmount; i++)
        data[i] = 1 - data[i];
    computegradient( data, (int)width, (int)height, gx, gy);
    edtaa3(data, gx, gy, (int)width, (int)height, xdist, ydist, inside);
    for( i=0; i< pixelAmount; i++)
        if( inside[i] < 0.0 )
            inside[i] = 0.0;

    // The bipolar distance field is now outside-inside
    double dist;
    /* Single channel 8-bit output (bad precision and range, but simple) */
    unsigned char *out = (unsigned char *) malloc( pixelAmount * sizeof(unsigned char) );
    for( i=0; i < pixelAmount; i++)
    {
        dist = outside[i] - inside[i];
        dist = 128.0 - dist*16;
        if( dist < 0 ) dist = 0;
        if( dist > 255 ) dist = 255;
        out[i] = (unsigned char) dist;
    }
    /* Dual channel 16-bit output (more complicated, but good precision and range) */
    /*unsigned char *out = (unsigned char *) malloc( pixelAmount * 3 * sizeof(unsigned char) );
    for( i=0; i< pixelAmount; i++)
    {
        dist = outside[i] - inside[i];
        dist = 128.0 - dist*16;
        if( dist < 0.0 ) dist = 0.0;
        if( dist >= 256.0 ) dist = 255.999;
        // R channel is a copy of the original grayscale image
        out[3*i] = img[i];
        // G channel is fraction
        out[3*i + 1] = (unsigned char) ( 256 - (dist - floor(dist)* 256.0 ));
        // B channel is truncated integer part
        out[3*i + 2] = (unsigned char)dist;
    }*/

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( data );
    free( outside );
    free( inside );

    return out;
}

void FontFreeType::renderCharAt(unsigned char *dest,int posX, int posY, unsigned char* bitmap,long bitmapWidth,long bitmapHeight)
{
    int iX = posX;
    int iY = posY;

    if (_distanceFieldEnabled)
    {
        auto distanceMap = makeDistanceMap(bitmap,bitmapWidth,bitmapHeight);

        bitmapWidth += 2 * DistanceMapSpread;
        bitmapHeight += 2 * DistanceMapSpread;

        for (long y = 0; y < bitmapHeight; ++y)
        {
            long bitmap_y = y * bitmapWidth;

            for (long x = 0; x < bitmapWidth; ++x)
            {
                /* Dual channel 16-bit output (more complicated, but good precision and range) */
                /*int index = (iX + ( iY * destSize )) * 3;
                int index2 = (bitmap_y + x)*3;
                dest[index] = out[index2];
                dest[index + 1] = out[index2 + 1];
                dest[index + 2] = out[index2 + 2];*/

                //Single channel 8-bit output
                dest[iX + ( iY * GLFontAtlas::CacheTextureWidth )] = distanceMap[bitmap_y + x];

                iX += 1;
            }

            iX  = posX;
            iY += 1;
        }
        free(distanceMap);
    }
    else if(_outlineSize > 0)
    {
        unsigned char tempChar;
        for (long y = 0; y < bitmapHeight; ++y)
        {
            long bitmap_y = y * bitmapWidth;

            for (int x = 0; x < bitmapWidth; ++x)
            {
                tempChar = bitmap[(bitmap_y + x) * 2];
                dest[(iX + ( iY * GLFontAtlas::CacheTextureWidth ) ) * 2] = tempChar;
                tempChar = bitmap[(bitmap_y + x) * 2 + 1];
                dest[(iX + ( iY * GLFontAtlas::CacheTextureWidth ) ) * 2 + 1] = tempChar;

                iX += 1;
            }

            iX  = posX;
            iY += 1;
        }
        delete [] bitmap;
    }
    else
    {
        for (long y = 0; y < bitmapHeight; ++y)
        {
            long bitmap_y = y * bitmapWidth;

            for (int x = 0; x < bitmapWidth; ++x)
            {
                unsigned char cTemp = bitmap[bitmap_y + x];

                // the final pixel
                dest[(iX + ( iY * GLFontAtlas::CacheTextureWidth ) )] = cTemp;

                iX += 1;
            }

            iX  = posX;
            iY += 1;
        }
    }
}

void FontFreeType::setGlyphCollection(GlyphCollection glyphs, const char* customGlyphs /* = nullptr */)
{
    _usedGlyphs = glyphs;
    if (glyphs == GlyphCollection::CUSTOM)
    {
        _customGlyphs = customGlyphs;
    }
}

const char* FontFreeType::getGlyphCollection() const
{
    const char* glyphCollection = nullptr;
    switch (_usedGlyphs)
    {
    case GlyphCollection::DYNAMIC:
        break;
    case GlyphCollection::NEHE:
        glyphCollection = _glyphNEHE;
        break;
    case GlyphCollection::ASCII:
        glyphCollection = _glyphASCII;
        break;
    case GlyphCollection::CUSTOM:
        glyphCollection = _customGlyphs.c_str();
        break;
    default:
        break;
    }

    return glyphCollection;
}

void FontFreeType::releaseFont(const std::string &fontName)
{
    auto item = s_cacheFontData.begin();
    while (s_cacheFontData.end() != item)
    {
        if (item->first.find(fontName) != std::string::npos)
            item = s_cacheFontData.erase(item);
        else
            item++;
    }
}

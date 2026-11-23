#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif

#include "ParaEngine.h"
#include "edtaa3func.h"

#include "IO/FileUtils.h"
#include "util/StringHelper.h"
#include "GLFontFreeType.h"
#include "GLFontAtlas.h"

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <algorithm>
#include <cmath>
#include <cstring>
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
    _requestedFontSize = fontSize;
    int dpi = 72;
    const float scaledFontSize = fontSize * GL_CONTENT_SCALE_FACTOR();
    int fontSizePoints = static_cast<int>(std::floor(64.f * scaledFontSize + 0.5f));
    if (FT_Set_Char_Size(face, fontSizePoints, fontSizePoints, dpi, dpi))
    {
        // Fixed-size bitmap fonts (e.g. color emoji) may have fixed strikes instead of scalable outlines
        if (face->num_fixed_sizes > 0)
        {
            int bestIdx = 0;
            int bestDiff = 100000;
            for (int s = 0; s < face->num_fixed_sizes; ++s)
            {
                int diff = std::abs(face->available_sizes[s].height - static_cast<int>(scaledFontSize));
                if (diff < bestDiff) { bestDiff = diff; bestIdx = s; }
            }
            if (FT_Select_Size(face, bestIdx))
                return false;
        }
        else
        {
            return false;
        }
    }

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
        for (FT_Face extraFace : _extraFallbackFaces)
        {
            if (extraFace)
                FT_Done_Face(extraFace);
        }
    }
    _extraFallbackFaces.clear();

    auto decRefCache = [](const std::string& name) {
        if (name.empty())
            return;
        auto it = s_cacheFontData.find(name);
        if (it != s_cacheFontData.end())
        {
            if (it->second.referenceCount <= 1)
                s_cacheFontData.erase(it);
            else
                it->second.referenceCount -= 1;
        }
    };

    decRefCache(_fontName);
    decRefCache(_fallbackFontName);
    for (const auto& extraName : _extraFallbackNames)
    {
        decRefCache(extraName);
    }
    _extraFallbackNames.clear();
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

namespace
{
	bool isDefaultColorEmoji(char32_t cp, bool explicitEmojiVS)
	{
		if (explicitEmojiVS)
			return true;
		if (cp >= 0x1F000u && cp <= 0x1FAFFu)
			return true;
		if (cp >= 0x1F1E6u && cp <= 0x1F1FFu)
			return true;
		switch (cp)
		{
		case 0x231Au: case 0x231Bu:
		case 0x23E9u: case 0x23EAu: case 0x23EBu: case 0x23ECu: case 0x23F0u: case 0x23F3u:
		case 0x25FDu: case 0x25FEu:
		case 0x2614u: case 0x2615u:
		case 0x2648u: case 0x2649u: case 0x264Au: case 0x264Bu: case 0x264Cu: case 0x264Du:
		case 0x264Eu: case 0x264Fu: case 0x2650u: case 0x2651u: case 0x2652u: case 0x2653u:
		case 0x267Fu: case 0x2693u: case 0x26A1u: case 0x26AAu: case 0x26ABu:
		case 0x26BDu: case 0x26BEu: case 0x26C4u: case 0x26C5u: case 0x26CEu: case 0x26D4u:
		case 0x26EAu: case 0x26F2u: case 0x26F3u: case 0x26F5u: case 0x26FAu: case 0x26FDu:
		case 0x2705u: case 0x270Au: case 0x270Bu: case 0x2728u: case 0x274Cu: case 0x274Eu:
		case 0x2753u: case 0x2754u: case 0x2755u: case 0x2757u:
		case 0x2795u: case 0x2796u: case 0x2797u: case 0x27B0u: case 0x27BFu:
		case 0x2B1Bu: case 0x2B1Cu: case 0x2B50u: case 0x2B55u:
			return true;
		default:
			return false;
		}
	}

#if defined(__APPLE__)
	unsigned char* rasterizeCoreTextFallbackOrEmoji(
		char32_t cp, float scaledFontSize, bool bold, bool wantColorEmoji,
		long& outWidth, long& outHeight, CCRect& outRect, int& xAdvance, bool& outIsColor)
	{
		if (cp == 0 || scaledFontSize <= 1.0f)
			return nullptr;

		UniChar chars[2];
		CFIndex charLen = 0;
		if (cp <= 0xFFFFu)
		{
			chars[0] = static_cast<UniChar>(cp);
			charLen = 1;
		}
		else if (cp <= 0x10FFFFu)
		{
			const char32_t u = cp - 0x10000u;
			chars[0] = static_cast<UniChar>(0xD800u + (u >> 10));
			chars[1] = static_cast<UniChar>(0xDC00u + (u & 0x3FFu));
			charLen = 2;
		}
		else
		{
			return nullptr;
		}

		CTFontRef targetFont = nullptr;
		bool isEmojiFont = false;

		if (wantColorEmoji)
		{
			const CGFloat emojiPt = static_cast<CGFloat>(scaledFontSize * 0.90f);
			targetFont = CTFontCreateWithName(CFSTR("Apple Color Emoji"), emojiPt, nullptr);
			if (targetFont)
			{
				CGGlyph testGlyphs[2] = { 0, 0 };
				if (CTFontGetGlyphsForCharacters(targetFont, chars, testGlyphs, charLen) && testGlyphs[0] != 0)
				{
					isEmojiFont = true;
				}
				else
				{
					CFRelease(targetFont);
					targetFont = nullptr;
				}
			}
		}

		if (!targetFont)
		{
			CTFontRef baseSysFont = CTFontCreateUIFontForLanguage(
				bold ? kCTFontUIFontEmphasizedSystem : kCTFontUIFontSystem,
				static_cast<CGFloat>(scaledFontSize), nullptr);
			if (!baseSysFont)
				return nullptr;

			CFStringRef cfStr = CFStringCreateWithCharacters(kCFAllocatorDefault, chars, charLen);
			if (cfStr)
			{
				targetFont = CTFontCreateForString(baseSysFont, cfStr, CFRangeMake(0, charLen));
				CFRelease(cfStr);
			}
			CFRelease(baseSysFont);
			if (!targetFont)
				return nullptr;

			CFStringRef famName = CTFontCopyFamilyName(targetFont);
			if (famName)
			{
				if (CFStringFind(famName, CFSTR("Emoji"), kCFCompareCaseInsensitive).location != kCFNotFound)
					isEmojiFont = true;
				CFRelease(famName);
			}
		}

		CGGlyph glyphs[2] = { 0, 0 };
		if (!CTFontGetGlyphsForCharacters(targetFont, chars, glyphs, charLen) || glyphs[0] == 0)
		{
			CFRelease(targetFont);
			return nullptr;
		}

		CGSize adv = CGSizeZero;
		CTFontGetAdvancesForGlyphs(targetFont, kCTFontOrientationHorizontal, &glyphs[0], &adv, 1);
		CGRect bbox = CGRectZero;
		CTFontGetBoundingRectsForGlyphs(targetFont, kCTFontOrientationHorizontal, &glyphs[0], &bbox, 1);

		xAdvance = static_cast<int>(std::floor(adv.width + 0.5));
		if (xAdvance <= 0)
			xAdvance = static_cast<int>(std::floor(scaledFontSize + 0.5f));

		if (bbox.size.width <= 0.05 || bbox.size.height <= 0.05)
		{
			CFRelease(targetFont);
			outWidth = 0;
			outHeight = 0;
			outRect.origin.x = 0;
			outRect.origin.y = 0;
			outRect.size.width = 0;
			outRect.size.height = 0;
			outIsColor = false;
			return nullptr;
		}

		const int penX = static_cast<int>(std::ceil((std::max)(0.0, -bbox.origin.x))) + 3;
		const int penY = static_cast<int>(std::ceil((std::max)(0.0, -bbox.origin.y))) + 3;
		const int bufW = (std::min)(128, (std::max)(8, static_cast<int>(std::ceil(bbox.size.width + std::fabs(bbox.origin.x))) + 8));
		const int bufH = (std::min)(128, (std::max)(8, static_cast<int>(std::ceil(bbox.size.height + std::fabs(bbox.origin.y))) + 8));

		std::vector<unsigned char> rgba(static_cast<size_t>(bufW * bufH * 4), 0);
		CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
		CGContextRef ctx = CGBitmapContextCreate(
			rgba.data(), bufW, bufH, 8, bufW * 4, cs,
			static_cast<uint32_t>(kCGImageAlphaPremultipliedLast) | static_cast<uint32_t>(kCGBitmapByteOrder32Big));
		CGColorSpaceRelease(cs);

		if (!ctx)
		{
			CFRelease(targetFont);
			return nullptr;
		}

		CGContextSetAllowsAntialiasing(ctx, true);
		CGContextSetShouldAntialias(ctx, true);
		CGContextSetAllowsFontSmoothing(ctx, !isEmojiFont);
		CGContextSetShouldSmoothFonts(ctx, !isEmojiFont);
		CGContextSetRGBFillColor(ctx, 1.0, 1.0, 1.0, 1.0);

		CGPoint drawPt = CGPointMake(static_cast<CGFloat>(penX), static_cast<CGFloat>(penY));
		CTFontDrawGlyphs(targetFont, &glyphs[0], &drawPt, 1, ctx);
		CGContextRelease(ctx);
		CFRelease(targetFont);

		// Find tight non-transparent pixel bounds (row 0 is top of image)
		int minC = bufW, maxC = -1, minR = bufH, maxR = -1;
		for (int r = 0; r < bufH; ++r)
		{
			const unsigned char* rowPtr = rgba.data() + static_cast<size_t>(r * bufW * 4);
			for (int c = 0; c < bufW; ++c)
			{
				if (rowPtr[c * 4 + 3] > 0)
				{
					if (c < minC) minC = c;
					if (c > maxC) maxC = c;
					if (r < minR) minR = r;
					if (r > maxR) maxR = r;
				}
			}
		}

		if (maxC < minC || maxR < minR)
		{
			outWidth = 0;
			outHeight = 0;
			outIsColor = false;
			return nullptr;
		}

		const int cropW = maxC - minC + 1;
		const int cropH = maxR - minR + 1;
		const int baselineRowFromTop = bufH - penY;
		const int bitmapTop = baselineRowFromTop - minR;

		outRect.origin.x = static_cast<float>(minC - penX);
		outRect.origin.y = static_cast<float>(-bitmapTop);
		outRect.size.width = static_cast<float>(cropW);
		outRect.size.height = static_cast<float>(cropH);
		outWidth = cropW;
		outHeight = cropH;
		outIsColor = isEmojiFont;

		if (isEmojiFont)
		{
			unsigned char* outBuf = new (std::nothrow) unsigned char[static_cast<size_t>(cropW * cropH * 4)];
			if (!outBuf)
				return nullptr;
			for (int r = 0; r < cropH; ++r)
			{
				const unsigned char* srcRow = rgba.data() + static_cast<size_t>((minR + r) * bufW * 4);
				unsigned char* dstRow = outBuf + static_cast<size_t>(r * cropW * 4);
				for (int c = 0; c < cropW; ++c)
				{
					const unsigned char pr = srcRow[(minC + c) * 4 + 0];
					const unsigned char pg = srcRow[(minC + c) * 4 + 1];
					const unsigned char pb = srcRow[(minC + c) * 4 + 2];
					const unsigned char pa = srcRow[(minC + c) * 4 + 3];
					if (pa > 0 && pa < 255)
					{
						dstRow[c * 4 + 0] = static_cast<unsigned char>((std::min)(255, (static_cast<int>(pr) * 255 + pa / 2) / pa));
						dstRow[c * 4 + 1] = static_cast<unsigned char>((std::min)(255, (static_cast<int>(pg) * 255 + pa / 2) / pa));
						dstRow[c * 4 + 2] = static_cast<unsigned char>((std::min)(255, (static_cast<int>(pb) * 255 + pa / 2) / pa));
						dstRow[c * 4 + 3] = pa;
					}
					else
					{
						dstRow[c * 4 + 0] = pr;
						dstRow[c * 4 + 1] = pg;
						dstRow[c * 4 + 2] = pb;
						dstRow[c * 4 + 3] = pa;
					}
				}
			}
			return outBuf;
		}
		else
		{
			unsigned char* outGray = new (std::nothrow) unsigned char[static_cast<size_t>(cropW * cropH)];
			if (!outGray)
				return nullptr;
			for (int r = 0; r < cropH; ++r)
			{
				const unsigned char* srcRow = rgba.data() + static_cast<size_t>((minR + r) * bufW * 4);
				unsigned char* dstRow = outGray + static_cast<size_t>(r * cropW);
				for (int c = 0; c < cropW; ++c)
				{
					dstRow[c] = srcRow[(minC + c) * 4 + 3];
				}
			}
			return outGray;
		}
	}
#endif
}

bool FontFreeType::addExtraFallbackFont(const std::string &fontPath)
{
	if (fontPath.empty() || fontPath == _fontName || fontPath == _fallbackFontName)
		return false;
	for (const auto& existing : _extraFallbackNames)
	{
		if (existing == fontPath)
			return false;
	}
	if (!CFileUtils::FileExist(fontPath.c_str()))
		return false;

	auto it = s_cacheFontData.find(fontPath);
	if (it != s_cacheFontData.end())
	{
		it->second.referenceCount += 1;
	}
	else
	{
		s_cacheFontData[fontPath].referenceCount = 1;
		s_cacheFontData[fontPath].data = CFileUtils::GetDataFromFile(fontPath.c_str());
		if (s_cacheFontData[fontPath].data.isNull())
		{
			s_cacheFontData.erase(fontPath);
			return false;
		}
	}

	auto releaseCache = [&]() {
		auto cit = s_cacheFontData.find(fontPath);
		if (cit != s_cacheFontData.end())
		{
			if (cit->second.referenceCount <= 1)
				s_cacheFontData.erase(cit);
			else
				cit->second.referenceCount -= 1;
		}
	};

	FT_Face face = nullptr;
	if (FT_New_Memory_Face(getFTLibrary(), (const FT_Byte*)(s_cacheFontData[fontPath].data.GetBytes()), s_cacheFontData[fontPath].data.GetSize(), 0, &face))
	{
		releaseCache();
		return false;
	}
	if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
	{
		FT_Done_Face(face);
		releaseCache();
		return false;
	}

	const float scaledSize = _requestedFontSize * GL_CONTENT_SCALE_FACTOR();
	const int fontSizePoints = static_cast<int>(std::floor(64.f * scaledSize + 0.5f));
	if (FT_Set_Char_Size(face, fontSizePoints, fontSizePoints, 72, 72))
	{
		if (face->num_fixed_sizes > 0)
		{
			int bestIdx = 0, bestDiff = 100000;
			for (int s = 0; s < face->num_fixed_sizes; ++s)
			{
				int diff = std::abs(face->available_sizes[s].height - static_cast<int>(scaledSize));
				if (diff < bestDiff) { bestDiff = diff; bestIdx = s; }
			}
			if (FT_Select_Size(face, bestIdx))
			{
				FT_Done_Face(face);
				releaseCache();
				return false;
			}
		}
		else
		{
			FT_Done_Face(face);
			releaseCache();
			return false;
		}
	}

	_extraFallbackNames.push_back(fontPath);
	_extraFallbackFaces.push_back(face);
	return true;
}

void FontFreeType::ensureSystemFallbackChain()
{
	if (_systemFallbacksInited)
		return;
	_systemFallbacksInited = true;

#ifdef WIN32
	char szWinPath[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(szWinPath, MAX_PATH);
	std::string winDir = szWinPath;
	addExtraFallbackFont(winDir + "/Fonts/seguisym.ttf");
	addExtraFallbackFont(winDir + "/Fonts/seguiemj.ttf");
	addExtraFallbackFont(winDir + "/Fonts/arialuni.ttf");
#elif defined(__APPLE__)
	addExtraFallbackFont("/System/Library/Fonts/STHeiti Medium.ttc");
	addExtraFallbackFont("/System/Library/Fonts/Apple Symbols.ttf");
	addExtraFallbackFont("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
	addExtraFallbackFont("/System/Library/Fonts/Menlo.ttc");
	addExtraFallbackFont("/System/Library/Fonts/ZapfDingbats.ttf");
#else
	addExtraFallbackFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
	addExtraFallbackFont("/usr/share/fonts/truetype/noto/NotoSansSymbols-Regular.ttf");
	addExtraFallbackFont("/usr/share/fonts/truetype/noto/NotoSansSymbols2-Regular.ttf");
	addExtraFallbackFont("/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf");
#endif
}

unsigned char* FontFreeType::getGlyphBitmap(unsigned short theChar, long &outWidth, long &outHeight, CCRect &outRect, int &xAdvance)
{
	bool isColor = false;
	bool ownsBuffer = false;
	return getGlyphBitmap(static_cast<char32_t>(theChar), outWidth, outHeight, outRect, xAdvance, isColor, ownsBuffer);
}

unsigned char* FontFreeType::getGlyphBitmap(char32_t rawCharKey, long &outWidth, long &outHeight, CCRect &outRect, int &xAdvance, bool &outIsColor, bool &outOwnsBuffer)
{
    outIsColor = false;
    outOwnsBuffer = false;
    const bool explicitEmojiVS = (rawCharKey & kEmojiPresentationFlag) != 0;
    const char32_t theChar = rawCharKey & ~kEmojiPresentationFlag;

    // Zero-width variation selectors & joiners (`U+FE0F`, `U+FE0E`, `U+200B`, `U+200D`)
    if (theChar == 0xFE0Fu || theChar == 0xFE0Eu || theChar == 0x200Bu || theChar == 0x200Du)
    {
        outWidth = 0;
        outHeight = 0;
        outRect.origin.x = 0;
        outRect.origin.y = 0;
        outRect.size.width = 0;
        outRect.size.height = 0;
        xAdvance = 0;
        return nullptr;
    }

    const bool wantColorEmoji = isDefaultColorEmoji(theChar, explicitEmojiVS);

#if defined(__APPLE__)
    if (wantColorEmoji && !_distanceFieldEnabled && _outlineSize <= 0.0f)
    {
        const float scaledSize = _requestedFontSize * GL_CONTENT_SCALE_FACTOR();
        unsigned char* emojiBmp = rasterizeCoreTextFallbackOrEmoji(
            theChar, scaledSize, _bold, true, outWidth, outHeight, outRect, xAdvance, outIsColor);
        if (emojiBmp && outWidth > 0 && outHeight > 0)
        {
            outOwnsBuffer = true;
            return emojiBmp;
        }
        delete[] emojiBmp;
    }
#endif

    bool invalidChar = true;
    unsigned char* ret = nullptr;
    do
    {
        if (_fontRef == nullptr)
            break;

        FT_Face face = nullptr;
        if (!wantColorEmoji && FT_Get_Char_Index(_fontRef, static_cast<FT_ULong>(theChar)) != 0)
        {
            face = _fontRef;
        }
        else if (!wantColorEmoji && _fallbackFontRef != nullptr && FT_Get_Char_Index(_fallbackFontRef, static_cast<FT_ULong>(theChar)) != 0)
        {
            face = _fallbackFontRef;
        }
        else
        {
            ensureSystemFallbackChain();
            // If `wantColorEmoji`, prefer color-capable faces first (`FT_HAS_COLOR`)
            if (wantColorEmoji)
            {
                for (FT_Face extraFace : _extraFallbackFaces)
                {
                    if (extraFace && FT_HAS_COLOR(extraFace) && FT_Get_Char_Index(extraFace, static_cast<FT_ULong>(theChar)) != 0)
                    {
                        face = extraFace;
                        break;
                    }
                }
            }
            if (!face)
            {
                for (FT_Face extraFace : _extraFallbackFaces)
                {
                    if (extraFace && FT_Get_Char_Index(extraFace, static_cast<FT_ULong>(theChar)) != 0)
                    {
                        face = extraFace;
                        break;
                    }
                }
            }
        }

        if (face == nullptr)
        {
#if defined(__APPLE__)
            if (!_distanceFieldEnabled && _outlineSize <= 0.0f)
            {
                const float scaledSize = _requestedFontSize * GL_CONTENT_SCALE_FACTOR();
                unsigned char* ctBmp = rasterizeCoreTextFallbackOrEmoji(
                    theChar, scaledSize, _bold, wantColorEmoji, outWidth, outHeight, outRect, xAdvance, outIsColor);
                if (ctBmp || xAdvance > 0)
                {
                    outOwnsBuffer = (ctBmp != nullptr);
                    return ctBmp;
                }
            }
#endif
            break;
        }

        const bool faceHasColor = FT_HAS_COLOR(face) != 0;
        const bool synthesizeBold = _bold && !faceHasColor && ((face->style_flags & FT_STYLE_FLAG_BOLD) == 0);
        FT_Int32 loadFlags = _distanceFieldEnabled
            ? (FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT)
            : FT_LOAD_NO_AUTOHINT;
        if (faceHasColor && !_distanceFieldEnabled && _outlineSize <= 0.0f)
        {
#ifdef FT_LOAD_COLOR
            loadFlags |= FT_LOAD_COLOR;
#endif
        }
        if (!synthesizeBold)
        {
            loadFlags |= FT_LOAD_RENDER;
        }

        if (FT_Load_Char(face, static_cast<FT_ULong>(theChar), loadFlags))
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

        // Handle BGRA color emoji bitmaps from FreeType (`FT_PIXEL_MODE_BGRA`)
        if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && face->glyph->bitmap.buffer != nullptr
            && face->glyph->bitmap.width > 0 && face->glyph->bitmap.rows > 0)
        {
            const int srcW = static_cast<int>(face->glyph->bitmap.width);
            const int srcH = static_cast<int>(face->glyph->bitmap.rows);
            const int srcPitch = face->glyph->bitmap.pitch;
            const float targetMaxH = (std::max)(8.0f, _requestedFontSize * GL_CONTENT_SCALE_FACTOR() * 1.15f);
            const float downScale = (srcH > targetMaxH) ? (targetMaxH / static_cast<float>(srcH)) : 1.0f;
            const int dstW = (std::max)(1, static_cast<int>(std::floor(srcW * downScale + 0.5f)));
            const int dstH = (std::max)(1, static_cast<int>(std::floor(srcH * downScale + 0.5f)));

            unsigned char* rgbaOut = new (std::nothrow) unsigned char[static_cast<size_t>(dstW * dstH * 4)];
            if (!rgbaOut)
                break;

            for (int dy = 0; dy < dstH; ++dy)
            {
                const int sy = (std::min)(srcH - 1, static_cast<int>((dy + 0.5f) / downScale));
                const unsigned char* sRow = face->glyph->bitmap.buffer + sy * srcPitch;
                unsigned char* dRow = rgbaOut + static_cast<size_t>(dy * dstW * 4);
                for (int dx = 0; dx < dstW; ++dx)
                {
                    const int sx = (std::min)(srcW - 1, static_cast<int>((dx + 0.5f) / downScale));
                    const unsigned char b = sRow[sx * 4 + 0];
                    const unsigned char g = sRow[sx * 4 + 1];
                    const unsigned char r = sRow[sx * 4 + 2];
                    const unsigned char a = sRow[sx * 4 + 3];
                    if (a > 0 && a < 255)
                    {
                        dRow[dx * 4 + 0] = static_cast<unsigned char>((std::min)(255, (static_cast<int>(r) * 255 + a / 2) / a));
                        dRow[dx * 4 + 1] = static_cast<unsigned char>((std::min)(255, (static_cast<int>(g) * 255 + a / 2) / a));
                        dRow[dx * 4 + 2] = static_cast<unsigned char>((std::min)(255, (static_cast<int>(b) * 255 + a / 2) / a));
                        dRow[dx * 4 + 3] = a;
                    }
                    else
                    {
                        dRow[dx * 4 + 0] = r;
                        dRow[dx * 4 + 1] = g;
                        dRow[dx * 4 + 2] = b;
                        dRow[dx * 4 + 3] = a;
                    }
                }
            }

            outRect.origin.x = std::floor(face->glyph->bitmap_left * downScale + 0.5f);
            outRect.origin.y = -std::floor(face->glyph->bitmap_top * downScale + 0.5f);
            outRect.size.width = static_cast<float>(dstW);
            outRect.size.height = static_cast<float>(dstH);
            xAdvance = (std::max)(dstW, static_cast<int>(std::floor(((face->glyph->metrics.horiAdvance + 32) >> 6) * downScale + 0.5f)));
            outWidth = dstW;
            outHeight = dstH;
            outIsColor = true;
            outOwnsBuffer = true;
            return rgbaOut;
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
            outOwnsBuffer = true;
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

unsigned char * FontFreeType::getGlyphBitmapWithOutline(FT_Face face, char32_t theChar, FT_BBox &bbox)
{
    unsigned char* ret = nullptr;
    if (face && FT_Load_Char(face, static_cast<FT_ULong>(theChar), FT_LOAD_NO_BITMAP) == 0)
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

void FontFreeType::renderColorCharAt(unsigned char *dest, int posX, int posY, const unsigned char* rgbaBitmap, long bitmapWidth, long bitmapHeight)
{
    if (!dest || !rgbaBitmap || bitmapWidth <= 0 || bitmapHeight <= 0)
        return;
    for (long y = 0; y < bitmapHeight; ++y)
    {
        const int dstY = posY + static_cast<int>(y);
        if (dstY < 0 || dstY >= GLFontAtlas::CacheTextureHeight)
            continue;
        const unsigned char* srcRow = rgbaBitmap + static_cast<size_t>(y * bitmapWidth * 4);
        unsigned char* dstRow = dest + static_cast<size_t>((dstY * GLFontAtlas::CacheTextureWidth + posX) * 4);
        const long copyCols = (std::min)(bitmapWidth, static_cast<long>(GLFontAtlas::CacheTextureWidth - posX));
        if (copyCols > 0)
        {
            memcpy(dstRow, srcRow, static_cast<size_t>(copyCols * 4));
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

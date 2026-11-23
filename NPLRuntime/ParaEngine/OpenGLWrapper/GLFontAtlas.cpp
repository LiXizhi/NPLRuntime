#include "ParaEngine.h"
#include "iconv.h"
#include "GLFontAtlas.h"
#include "GLFont.h"
#include "GLTexture2D.h"
#include "GLFontFreeType.h"
#include "StringHelper.h"


using namespace ParaEngine;

const int GLFontAtlas::CacheTextureWidth = 512;
const int GLFontAtlas::CacheTextureHeight = 512;
const char* GLFontAtlas::CMD_PURGE_FONTATLAS = "__cc_PURGE_FONTATLAS";
const char* GLFontAtlas::CMD_RESET_FONTATLAS = "__cc_RESET_FONTATLAS";

GLFontAtlas::GLFontAtlas(Font &theFont)
: _font(&theFont)
, _fontFreeType(nullptr)
, _iconv(nullptr)
, _currentPageData(nullptr)
, _fontAscender(0)
, _antialiasEnabled(true)
, _currLineHeight(0)
, _inited(false)
{
    _font->addref();
    _fontFreeType = dynamic_cast<FontFreeType*>(_font);
	init();
}

GLFontAtlas::~GLFontAtlas()
{
    _font->Release();
    releaseTextures();

    delete []_currentPageData;
    delete []_currentColorPageData;
    _currentColorPageData = nullptr;


	if (_iconv)
	{
		iconv_close(_iconv);
		_iconv = nullptr;
	}
	_inited = false;
	_colorPageInited = false;
}

void GLFontAtlas::reset()
{
    ++_layoutRevision;
    releaseTextures();

    _currLineHeight = 0;
    _currentPage = 0;
    _currentPageOrigX = 0;
    _currentPageOrigY = 0;
    _colorPageInited = false;
    _currentColorPage = 0;
    _currentColorOrigX = 0;
    _currentColorOrigY = 0;
    _currColorLineHeight = 0;
    delete []_currentColorPageData;
    _currentColorPageData = nullptr;
    _letterDefinitions.clear();
	_inited = false;
}

void GLFontAtlas::releaseTextures()
{
    for( auto &item: _atlasTextures)
    {
        item.second->Release();
    }
    _atlasTextures.clear();
	_inited = false;
}

void GLFontAtlas::purgeTexturesAtlas()
{
	if (!_inited)return;
    if (_fontFreeType)
    {
        reset();
    }
	_inited = false;
}


void GLFontAtlas::addLetterDefinition(char32_t utf32Char, const FontLetterDefinition &letterDefinition)
{
    ++_layoutRevision;
    _letterDefinitions[utf32Char] = letterDefinition;
}

void GLFontAtlas::scaleFontLetterDefinition(float scaleFactor)
{
    ++_layoutRevision;
    for (auto&& fontDefinition : _letterDefinitions) {
        auto& letterDefinition = fontDefinition.second;
        letterDefinition.width *= scaleFactor;
        letterDefinition.height *= scaleFactor;
        letterDefinition.offsetX *= scaleFactor;
        letterDefinition.offsetY *= scaleFactor;
        letterDefinition.xAdvance *= scaleFactor;
    }
}

bool GLFontAtlas::getLetterDefinitionForChar(char32_t utf32Char, FontLetterDefinition &letterDefinition)
{
    auto outIterator = _letterDefinitions.find(utf32Char);

    if (outIterator != _letterDefinitions.end())
    {
        letterDefinition = (*outIterator).second;
        return letterDefinition.validDefinition;
    }
    else
    {
        return false;
    }
}

void GLFontAtlas::conversionU16TOGB2312(const std::u16string& u16Text, std::unordered_map<char32_t, char32_t>& charCodeMap)
{
	OUTPUT_LOG("conversionU16TOGB2312");
    size_t strLen = u16Text.length();
    auto gb2312StrSize = strLen * 2;
    auto gb2312Text = new (std::nothrow) char[gb2312StrSize];
    memset(gb2312Text, 0, gb2312StrSize);
	auto encoding = _fontFreeType->getEncoding();
    switch (encoding)
    {
	case FT_ENCODING_GB2312:
	{

		if (_iconv == nullptr)
		{
			_iconv = iconv_open("gb2312", "utf-16le");
		}

		if (_iconv == (iconv_t)-1)
		{
			OUTPUT_LOG("conversion from utf16 to gb2312 not available");
		}
		else
		{
			char* pout = gb2312Text;
			size_t inLen = strLen * 2;
			size_t outLen = gb2312StrSize;

#if _LIBICONV_VERSION == 0x109 || _LIBICONV_VERSION == 0x010F
			const char* pin = (char*)u16Text.c_str();
			iconv(_iconv, &pin, &inLen, &pout, &outLen);
#else
			char* pin = (char*)u16Text.c_str();
			iconv(_iconv, &pin, &inLen, &pout, &outLen);
#endif
		}

	}
	break;
    default:
        OUTPUT_LOG("Unsupported encoding:%d", _fontFreeType->getEncoding());
        break;
    }

    unsigned short gb2312Code = 0;
    unsigned char* dst = (unsigned char*)&gb2312Code;
    unsigned short u16Code;
    for (size_t index = 0, gbIndex = 0; index < strLen; ++index)
    {
        u16Code = u16Text[index];
        if (u16Code < 256)
        {
            charCodeMap[u16Code] = u16Code;
            gbIndex += 1;
        }
        else
        {
            dst[0] = gb2312Text[gbIndex + 1];
            dst[1] = gb2312Text[gbIndex];
            charCodeMap[u16Code] = gb2312Code;

            gbIndex += 2;
        }
    }

    delete[] gb2312Text;
}

void GLFontAtlas::findNewCharacters(const std::u16string& u16Text, std::unordered_map<char32_t, char32_t>& charCodeMap)
{
    FT_Encoding charEncoding = _fontFreeType->getEncoding();
    if (charEncoding == FT_ENCODING_UNICODE)
    {
        const size_t length = u16Text.length();
        for (size_t i = 0; i < length; )
        {
            size_t consumed = 1;
            const char32_t cpKey = decodeUTF16CodePointWithVS(u16Text, i, consumed);
            if (_letterDefinitions.find(cpKey) == _letterDefinitions.end())
            {
                charCodeMap[cpKey] = cpKey;
            }
            i += consumed;
        }
        return;
    }

    std::u16string newChars;
    const size_t length = u16Text.length();
    for (size_t i = 0; i < length; ++i)
    {
        if (_letterDefinitions.find(static_cast<char32_t>(u16Text[i])) == _letterDefinitions.end())
        {
            newChars.push_back(u16Text[i]);
        }
    }
    if (!newChars.empty())
    {
        if (charEncoding == FT_ENCODING_GB2312)
        {
            conversionU16TOGB2312(newChars, charCodeMap);
        }
        else
        {
            OUTPUT_LOG("FontAtlas::findNewCharacters: Unsupported encoding:%d", charEncoding);
        }
    }
}

void GLFontAtlas::ensureColorPageInitialized()
{
    if (_colorPageInited)
        return;
    _colorPageInited = true;
    _currentColorPage = 0;
    _currentColorOrigX = 0.0f;
    _currentColorOrigY = 0.0f;
    _currColorLineHeight = 0;
    _currentColorPageDataSize = CacheTextureWidth * CacheTextureHeight * 4;
    delete[] _currentColorPageData;
    _currentColorPageData = new (std::nothrow) unsigned char[_currentColorPageDataSize];
    if (_currentColorPageData)
    {
        memset(_currentColorPageData, 0, _currentColorPageDataSize);
        auto tex = new (std::nothrow) GLTexture2D;
        if (_antialiasEnabled)
            tex->setAntiAliasTexParameters();
        else
            tex->setAliasTexParameters();
        tex->initWithData(_currentColorPageData, _currentColorPageDataSize,
            PixelFormat::A8R8G8B8, CacheTextureWidth, CacheTextureHeight, Size(CacheTextureWidth, CacheTextureHeight));
        addTexture(tex, ColorPageSlotBase + _currentColorPage);
        tex->Release();
    }
}

bool GLFontAtlas::prepareLetterDefinitions(const std::u16string& utf16Text)
{

	init();

    if (_fontFreeType == nullptr)
    {
        return false;
    }
    std::unordered_map<char32_t, char32_t> codeMapOfNewChar;
    findNewCharacters(utf16Text, codeMapOfNewChar);
    if (codeMapOfNewChar.empty())
    {
        return false;
    }

    int adjustForDistanceMap = _letterPadding / 2;
    int adjustForExtend = _letterEdgeExtend / 2;
    long bitmapWidth;
    long bitmapHeight;
    int glyphHeight;
    CCRect tempRect;
    FontLetterDefinition tempDef;

    auto scaleFactor = GL_CONTENT_SCALE_FACTOR();
    auto  pixelFormat = _fontFreeType->getOutlineSize() > 0 ? PixelFormat::A8L8 : PixelFormat::A8;

    float startY = _currentPageOrigY;
    float colorStartY = _currentColorOrigY;
    bool touchedMonoPage = false;
    bool touchedColorPage = false;

    for (auto&& it : codeMapOfNewChar)
    {
        const char32_t rawCp = it.second & ~kEmojiPresentationFlag;
        // Invisible variation selectors / zero-width joiners (`U+FE0F`, `U+FE0E`, `U+200B`, `U+200D`)
        if (rawCp == 0xFE0Fu || rawCp == 0xFE0Eu || rawCp == 0x200Bu || rawCp == 0x200Du)
        {
            FontLetterDefinition zwDef{};
            zwDef.validDefinition = true;
            zwDef.isColor = false;
            zwDef.width = 0;
            zwDef.height = 0;
            zwDef.U = 0;
            zwDef.V = 0;
            zwDef.offsetX = 0;
            zwDef.offsetY = 0;
            zwDef.textureID = 0;
            zwDef.xAdvance = 0;
            _letterDefinitions[it.first] = zwDef;
            ++_layoutRevision;
            continue;
        }

        bool isColorGlyph = false;
        bool ownsBitmapBuf = false;
        tempDef = FontLetterDefinition{};
        auto bitmap = _fontFreeType->getGlyphBitmap(it.second, bitmapWidth, bitmapHeight, tempRect, tempDef.xAdvance, isColorGlyph, ownsBitmapBuf);
        if (bitmap && bitmapWidth > 0 && bitmapHeight > 0)
        {
            tempDef.validDefinition = true;
            tempDef.isColor = isColorGlyph;

            if (isColorGlyph)
            {
                ensureColorPageInitialized();
                colorStartY = touchedColorPage ? colorStartY : _currentColorOrigY;
                touchedColorPage = true;

                const int colorExtend = _letterEdgeExtend;
                const int colorHalfExt = colorExtend / 2;
                tempDef.width = tempRect.size.width + colorExtend;
                tempDef.height = tempRect.size.height + colorExtend;
                tempDef.offsetX = tempRect.origin.x - colorHalfExt;
                tempDef.offsetY = _fontAscender + tempRect.origin.y - colorHalfExt;

                glyphHeight = static_cast<int>(bitmapHeight) + colorExtend;
                if (_currentColorOrigX + tempDef.width > CacheTextureWidth)
                {
                    _currentColorOrigY += _currColorLineHeight;
                    _currColorLineHeight = 0;
                    _currentColorOrigX = 0;
                }
                const int minRowH = (std::max)(static_cast<int>(_lineHeight) + colorExtend, glyphHeight);
                if (_currentColorOrigY + minRowH >= CacheTextureHeight)
                {
                    const int slotId = ColorPageSlotBase + _currentColorPage;
                    if (_currentColorPageData && _atlasTextures.count(slotId))
                    {
                        unsigned char* cData = _currentColorPageData + CacheTextureWidth * static_cast<int>(colorStartY) * 4;
                        _atlasTextures[slotId]->updateWithData(cData, 0, colorStartY, CacheTextureWidth, CacheTextureHeight - colorStartY);
                    }
                    colorStartY = 0.0f;
                    _currColorLineHeight = 0;
                    _currentColorOrigX = 0;
                    _currentColorOrigY = 0;
                    if (_currentColorPageData)
                        memset(_currentColorPageData, 0, _currentColorPageDataSize);
                    _currentColorPage++;
                    auto tex = new (std::nothrow) GLTexture2D;
                    if (_antialiasEnabled)
                        tex->setAntiAliasTexParameters();
                    else
                        tex->setAliasTexParameters();
                    tex->initWithData(_currentColorPageData, _currentColorPageDataSize,
                        PixelFormat::A8R8G8B8, CacheTextureWidth, CacheTextureHeight, Size(CacheTextureWidth, CacheTextureHeight));
                    addTexture(tex, ColorPageSlotBase + _currentColorPage);
                    tex->Release();
                }
                if (glyphHeight > _currColorLineHeight)
                    _currColorLineHeight = glyphHeight;

                _fontFreeType->renderColorCharAt(_currentColorPageData,
                    static_cast<int>(_currentColorOrigX) + colorHalfExt,
                    static_cast<int>(_currentColorOrigY) + colorHalfExt,
                    bitmap, bitmapWidth, bitmapHeight);

                tempDef.U = _currentColorOrigX;
                tempDef.V = _currentColorOrigY;
                tempDef.textureID = ColorPageSlotBase + _currentColorPage;
                _currentColorOrigX += tempDef.width + 1;

                tempDef.width /= scaleFactor;
                tempDef.height /= scaleFactor;
                tempDef.U /= scaleFactor;
                tempDef.V /= scaleFactor;

                if (ownsBitmapBuf)
                    delete[] bitmap;
            }
            else
            {
                touchedMonoPage = true;
                tempDef.width = tempRect.size.width + _letterPadding + _letterEdgeExtend;
                tempDef.height = tempRect.size.height + _letterPadding + _letterEdgeExtend;
                tempDef.offsetX = tempRect.origin.x + adjustForDistanceMap - adjustForExtend;
                tempDef.offsetY = _fontAscender + tempRect.origin.y - adjustForDistanceMap - adjustForExtend;

                glyphHeight = static_cast<int>(bitmapHeight) + _letterPadding + _letterEdgeExtend;
                if (_currentPageOrigX + tempDef.width > CacheTextureWidth)
                {
                    _currentPageOrigY += _currLineHeight;
                    _currLineHeight = 0;
                    _currentPageOrigX = 0;
                }
                const int minRowHeight = (std::max)(static_cast<int>(_lineHeight) + _letterPadding + _letterEdgeExtend, glyphHeight);
                if (_currentPageOrigY + minRowHeight >= CacheTextureHeight)
                {
                    unsigned char *data = nullptr;
                    if (pixelFormat == PixelFormat::A8L8)
                    {
                        data = _currentPageData + CacheTextureWidth * (int)startY * 2;
                    }
                    else
                    {
                        data = _currentPageData + CacheTextureWidth * (int)startY;
                    }
                    _atlasTextures[_currentPage]->updateWithData(data, 0, startY,
                        CacheTextureWidth, CacheTextureHeight - startY);

                    startY = 0.0f;

                    _currLineHeight = 0;
                    _currentPageOrigX = 0;
                    _currentPageOrigY = 0;
                    memset(_currentPageData, 0, _currentPageDataSize);
                    _currentPage++;
                    auto tex = new (std::nothrow) GLTexture2D;
                    if (_antialiasEnabled)
                    {
                        tex->setAntiAliasTexParameters();
                    }
                    else
                    {
                        tex->setAliasTexParameters();
                    }
                    tex->initWithData(_currentPageData, _currentPageDataSize,
                        pixelFormat, CacheTextureWidth, CacheTextureHeight, Size(CacheTextureWidth, CacheTextureHeight));
                    addTexture(tex, _currentPage);
                    tex->Release();
                }
                if (glyphHeight > _currLineHeight)
                {
                    _currLineHeight = glyphHeight;
                }
                _fontFreeType->renderCharAt(_currentPageData, _currentPageOrigX + adjustForExtend, _currentPageOrigY + adjustForExtend, bitmap, bitmapWidth, bitmapHeight);

                tempDef.U = _currentPageOrigX;
                tempDef.V = _currentPageOrigY;
                tempDef.textureID = _currentPage;
                _currentPageOrigX += tempDef.width + 1;
                // take from pixels to points
                tempDef.width = tempDef.width / scaleFactor;
                tempDef.height = tempDef.height / scaleFactor;
                tempDef.U = tempDef.U / scaleFactor;
                tempDef.V = tempDef.V / scaleFactor;

                // Note: when `_outlineSize > 0`, `renderCharAt` already freed `bitmap`
                if (ownsBitmapBuf && _fontFreeType->getOutlineSize() <= 0.0f)
                {
                    delete[] bitmap;
                }
            }
        }
        else{
            if (ownsBitmapBuf && bitmap)
                delete[] bitmap;
            if (tempDef.xAdvance)
                tempDef.validDefinition = true;
            else
                tempDef.validDefinition = false;

            tempDef.isColor = false;
            tempDef.width = 0;
            tempDef.height = 0;
            tempDef.U = 0;
            tempDef.V = 0;
            tempDef.offsetX = 0;
            tempDef.offsetY = 0;
            tempDef.textureID = 0;
            _currentPageOrigX += 1;
        }

        _letterDefinitions[it.first] = tempDef;
        ++_layoutRevision;
    }

    if (touchedMonoPage)
    {
        const int updateHeight = static_cast<int>(_currentPageOrigY - startY) + _currLineHeight;
        if (updateHeight > 0)
        {
            unsigned char *data = nullptr;
            if (pixelFormat == PixelFormat::A8L8)
            {
                data = _currentPageData + CacheTextureWidth * (int)startY * 2;
            }
            else
            {
                data = _currentPageData + CacheTextureWidth * (int)startY;
            }
            _atlasTextures[_currentPage]->updateWithData(data, 0, startY, CacheTextureWidth, updateHeight);
        }
    }

    if (touchedColorPage && _currentColorPageData)
    {
        const int cSlot = ColorPageSlotBase + _currentColorPage;
        const int cUpdateH = static_cast<int>(_currentColorOrigY - colorStartY) + _currColorLineHeight;
        if (cUpdateH > 0 && _atlasTextures.count(cSlot))
        {
            unsigned char* cData = _currentColorPageData + CacheTextureWidth * static_cast<int>(colorStartY) * 4;
            _atlasTextures[cSlot]->updateWithData(cData, 0, colorStartY, CacheTextureWidth, cUpdateH);
        }
    }

    return true;
}

void GLFontAtlas::addTexture(GLTexture2D *texture, int slot)
{
    texture->addref();
    _atlasTextures[slot] = texture;
}

GLTexture2D* GLFontAtlas::getTexture(int slot)
{
	init();
    return _atlasTextures[slot];
}

void  GLFontAtlas::setLineHeight(float newHeight)
{
    _lineHeight = newHeight;
}

void GLFontAtlas::setAliasTexParameters()
{
	init();
    if (_antialiasEnabled)
    {
        _antialiasEnabled = false;
        for (const auto & tex : _atlasTextures)
        {
            tex.second->setAliasTexParameters();
        }
    }
}

void GLFontAtlas::setAntiAliasTexParameters()
{
	init();
    if (! _antialiasEnabled)
    {
        _antialiasEnabled = true;
        for (const auto & tex : _atlasTextures)
        {
            tex.second->setAntiAliasTexParameters();
        }
    }
}

void ParaEngine::GLFontAtlas::init()
{
	if (_inited)return;
	_inited = true;
	if (_fontFreeType)
	{
		_lineHeight = _font->getFontMaxHeight();
		_fontAscender = _fontFreeType->getFontAscender();
		auto texture = new (std::nothrow) GLTexture2D;
		_currentPage = 0;
		_currentPageOrigX = 0;
		_currentPageOrigY = 0;
		_letterEdgeExtend = 2;
		_letterPadding = 0;

		if (_fontFreeType->isDistanceFieldEnabled())
		{
			_letterPadding += 2 * FontFreeType::DistanceMapSpread;
		}
		_currentPageDataSize = CacheTextureWidth * CacheTextureHeight;
		auto outlineSize = _fontFreeType->getOutlineSize();
		if (outlineSize > 0)
		{
			_lineHeight += 2 * outlineSize;
			_currentPageDataSize *= 2;
		}

		_currentPageData = new (std::nothrow) unsigned char[_currentPageDataSize];
		memset(_currentPageData, 0, _currentPageDataSize);

		auto  pixelFormat = outlineSize > 0 ? PixelFormat::A8L8 : PixelFormat::A8;
		texture->initWithData(_currentPageData, _currentPageDataSize,
			pixelFormat, CacheTextureWidth, CacheTextureHeight, Size(CacheTextureWidth, CacheTextureHeight));

		if (_antialiasEnabled)
		{
			texture->setAntiAliasTexParameters();
		}
		else
		{
			texture->setAliasTexParameters();
		}

		addTexture(texture, 0);
		texture->Release();
	}

}

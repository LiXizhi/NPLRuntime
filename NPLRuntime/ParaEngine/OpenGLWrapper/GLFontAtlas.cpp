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


	if (_iconv)
	{
		iconv_close(_iconv);
		_iconv = nullptr;
	}
	_inited = false;
}

void GLFontAtlas::reset()
{
    releaseTextures();

    _currLineHeight = 0;
    _currentPage = 0;
    _currentPageOrigX = 0;
    _currentPageOrigY = 0;
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


void GLFontAtlas::addLetterDefinition(char16_t utf16Char, const FontLetterDefinition &letterDefinition)
{
    _letterDefinitions[utf16Char] = letterDefinition;
}

void GLFontAtlas::scaleFontLetterDefinition(float scaleFactor)
{
    for (auto&& fontDefinition : _letterDefinitions) {
        auto& letterDefinition = fontDefinition.second;
        letterDefinition.width *= scaleFactor;
        letterDefinition.height *= scaleFactor;
        letterDefinition.offsetX *= scaleFactor;
        letterDefinition.offsetY *= scaleFactor;
        letterDefinition.xAdvance *= scaleFactor;
    }
}

bool GLFontAtlas::getLetterDefinitionForChar(char16_t utf16Char, FontLetterDefinition &letterDefinition)
{
    auto outIterator = _letterDefinitions.find(utf16Char);

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

void GLFontAtlas::conversionU16TOGB2312(const std::u16string& u16Text, std::unordered_map<unsigned short, unsigned short>& charCodeMap)
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

void GLFontAtlas::findNewCharacters(const std::u16string& u16Text, std::unordered_map<unsigned short, unsigned short>& charCodeMap)
{
    std::u16string newChars;
    FT_Encoding charEncoding = _fontFreeType->getEncoding();

    //find new characters
    if (_letterDefinitions.empty())
    {
        // fixed #16169: new android project crash in android 5.0.2 device (Nexus 7) when use 3.12.
        // While using clang compiler with gnustl_static on android, the copy assignment operator of `std::u16string`
        // will affect the memory validity, it means after `newChars` is destroyed, the memory of `u16Text` holds
        // will be a dead region. `u16text` represents the variable in `Label::_utf16Text`, when somewhere
        // allocates memory by `malloc, realloc, new, new[]`, the generated memory address may be the same
        // as `Label::_utf16Text` holds. If doing a `memset` or other memory operations, the orignal `Label::_utf16Text`
        // will be in an unknown state. Meanwhile, a bunch lots of logic which depends on `Label::_utf16Text`
        // will be broken.

        // newChars = u16Text;

        // Using `append` method is a workaround for this issue. So please be carefuly while using the assignment operator
        // of `std::u16string`.
        newChars.append(u16Text);
    }
    else
    {
        auto length = u16Text.length();
        newChars.reserve(length);
        for (size_t i = 0; i < length; ++i)
        {
            auto outIterator = _letterDefinitions.find(u16Text[i]);
            if (outIterator == _letterDefinitions.end())
            {
                newChars.push_back(u16Text[i]);
            }
        }
    }

    if (!newChars.empty())
    {
        switch (charEncoding)
        {
        case FT_ENCODING_UNICODE:
        {
            for (auto u16Code : newChars)
            {
                charCodeMap[u16Code] = u16Code;
            }
            break;
        }
        case FT_ENCODING_GB2312:
        {
            conversionU16TOGB2312(newChars, charCodeMap);
            break;
        }
        default:
            OUTPUT_LOG("FontAtlas::findNewCharacters: Unsupported encoding:%d", charEncoding);
            break;
        }
    }
}

bool GLFontAtlas::prepareLetterDefinitions(const std::u16string& utf16Text)
{

	init();

	//OUTPUT_LOG("prepareLetterDefinitions %s\n", u8Str.c_str());

    if (_fontFreeType == nullptr)
    {
        return false;
    }
    std::unordered_map<unsigned short, unsigned short> codeMapOfNewChar;
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

    for (auto&& it : codeMapOfNewChar)
    {
        auto bitmap = _fontFreeType->getGlyphBitmap(it.second, bitmapWidth, bitmapHeight, tempRect, tempDef.xAdvance);
        if (bitmap && bitmapWidth > 0 && bitmapHeight > 0)
        {
            tempDef.validDefinition = true;
            tempDef.width = tempRect.size.width + _letterPadding + _letterEdgeExtend;
            tempDef.height = tempRect.size.height + _letterPadding + _letterEdgeExtend;
            tempDef.offsetX = tempRect.origin.x + adjustForDistanceMap - adjustForExtend;
            tempDef.offsetY = _fontAscender + tempRect.origin.y - adjustForDistanceMap - adjustForExtend;

            if (_currentPageOrigX + tempDef.width > CacheTextureWidth)
            {
                _currentPageOrigY += _currLineHeight;
                _currLineHeight = 0;
                _currentPageOrigX = 0;
                if (_currentPageOrigY + _lineHeight + _letterPadding + _letterEdgeExtend >= CacheTextureHeight)
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
            }
            glyphHeight = static_cast<int>(bitmapHeight) + _letterPadding + _letterEdgeExtend;
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
        }
        else{
            if (tempDef.xAdvance)
                tempDef.validDefinition = true;
            else
                tempDef.validDefinition = false;

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
    }

    unsigned char *data = nullptr;
    if (pixelFormat == PixelFormat::A8L8)
    {
        data = _currentPageData + CacheTextureWidth * (int)startY * 2;
    }
    else
    {
        data = _currentPageData + CacheTextureWidth * (int)startY;
    }
    _atlasTextures[_currentPage]->updateWithData(data, 0, startY, CacheTextureWidth, _currentPageOrigY - startY + _currLineHeight);

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

		/*if (_antialiasEnabled)
		{
			texture->setAntiAliasTexParameters();
		}
		else
		{
			texture->setAliasTexParameters();
		}*/

		addTexture(texture, 0);
		texture->Release();
	}

}

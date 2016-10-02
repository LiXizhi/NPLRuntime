/****************************************************************************
 Copyright (c) 2013      Zynga Inc.
 Copyright (c) 2013-2016 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef _COCOS2D_CCLABEL_H_
#define _COCOS2D_CCLABEL_H_

#include "ParaGLMac.h"

#include <unordered_map>

NS_CC_BEGIN

enum GlyphCollection
{
	DYNAMIC
};

enum TextVAlignment
{
    TOP,
    CENTER,
    BOTTOM
};


enum TextHAlignment
{
    LEFT,
    CENTER,
    RIGHT
};

class FontAtlas
{
public:
	void setAliasTexParameters();

    void prepareLetterDefinitions(const std::u16string& stringToRender) {}
    

    const std::unordered_map<ssize_t, Texture2D*>& getTextures() const { return _atlasTextures; }

private:
    std::unordered_map<ssize_t, Texture2D*> _atlasTextures;
};

typedef struct _ttfConfig
{
    std::string fontFilePath;
    float fontSize;

    GlyphCollection glyphs;
    const char *customGlyphs;

    bool distanceFieldEnabled;
    int outlineSize;

    bool italics;
    bool bold;
    bool underline;
    bool strikethrough;

    _ttfConfig(const std::string& filePath = "",float size = 12, const GlyphCollection& glyphCollection = GlyphCollection::DYNAMIC,
        const char *customGlyphCollection = nullptr, bool useDistanceField = false, int outline = 0,
               bool useItalics = false, bool useBold = false, bool useUnderline = false, bool useStrikethrough = false)
        : fontFilePath(filePath)
        , fontSize(size)
        , glyphs(glyphCollection)
        , customGlyphs(customGlyphCollection)
        , distanceFieldEnabled(useDistanceField)
        , outlineSize(outline)
        , italics(useItalics)
        , bold(useBold)
	, underline(useUnderline)
        , strikethrough(useStrikethrough)
    {
        if(outline > 0)
        {
            distanceFieldEnabled = false;
        }
    }
} TTFConfig;


struct FontLetterDefinition
{
    float U;
    float V;
    float width;
    float height;
    float offsetX;
    float offsetY;
    int textureID;
    bool validDefinition;
    int xAdvance;
};



class Label
{
public:
	Label();
	virtual ~Label();

	bool setTTFConfig(const TTFConfig& ttfConfig);

	FontAtlas* getFontAtlas() { return _fontAtlas; }

	void autorelease();


	virtual void setPosition(float x, float y);

    void setWidth(float width) { }
    float getWidth() const { return _labelWidth; }


    void setHeight(float height){  }
    float getHeight() const { return _labelHeight; }


    void setAlignment(TextHAlignment hAlignment) { }
    TextHAlignment getTextAlignment() const { return _hAlignment;}

    void computeStringNumLines() {}
    void computeAlignmentOffset();
    bool computeHorizontalKernings(const std::u16string& stringToRender);




    int _limitShowCount;
    std::vector<LetterInfo> _lettersInfo;

private:
	FontAtlas* _fontAtlas;

    float _labelHeight;
    float _labelWidth;

    TextHAlignment  _hAlignment;
};


class LabelTextFormatter
{
public:
    void createStringSprites(Label* label) {}
    void multilineText(Label* label) {}
    void alignText(Label* label) {}
}


NS_CC_END

#endif /*__COCOS2D_CCLABEL_H */


#ifndef _CCFont_h_
#define _CCFont_h_

/// @cond DO_NOT_SHOW

#include <string>

#include "CCType.h"

NS_CC_BEGIN

class FontAtlas;

class Font : public Ref
{
public:
    virtual FontAtlas* createFontAtlas() = 0;

    virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const = 0;
    
    virtual int getFontMaxHeight() const { return 0; }
};

NS_CC_END

/// @endcond
#endif


#ifndef _CCFontAtlasCache_h_
#define _CCFontAtlasCache_h_

#include <unordered_map>

#include "CCType.h"


NS_CC_BEGIN

class FontAtlas;
class Texture2D;

class FontAtlasCache
{  
public:
    static FontAtlas * getFontAtlasTTF(const TTFConfig & config);
     
    static bool releaseFontAtlas(FontAtlas *atlas);

    /** Removes cached data.
     It will purge the textures atlas and if multiple texture exist in one FontAtlas.
     */
    static void purgeCachedData();
    
private: 
    static std::string generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField);
    static std::unordered_map<std::string, FontAtlas *> _atlasMap;
};

NS_CC_END

#endif

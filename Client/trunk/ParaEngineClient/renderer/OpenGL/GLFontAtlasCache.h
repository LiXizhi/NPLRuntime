#pragma once
#include <unordered_map>
#include "GLType.h"

namespace ParaEngine
{
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
}
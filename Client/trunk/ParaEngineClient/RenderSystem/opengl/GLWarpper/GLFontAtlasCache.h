#pragma once
#include <unordered_map>
#include "GLType.h"
#include "GLFontAtlas.h"

namespace ParaEngine
{
	class GLTexture2D;

	class FontAtlasCache
	{
	public:
		static FontAtlasPtr getFontAtlasTTF(const TTFConfig & config);

		static bool releaseFontAtlas(FontAtlasPtr atlas);

		/** Removes cached data.
		 It will purge the textures atlas and if multiple texture exist in one FontAtlas.
		 */
		static void purgeCachedData();

	private:
		static std::string generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField);
		static std::unordered_map<std::string, FontAtlasPtr> _atlasMap;
	};
}
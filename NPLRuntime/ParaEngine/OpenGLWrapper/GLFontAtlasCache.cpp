#include "ParaEngine.h"
#include <iostream>
#include <sstream>

#include "GLFontAtlasCache.h"
#include "GLFontFreeType.h"
#include "GLFontAtlas.h"
#include "GLLabel.h"

namespace ParaEngine
{

	std::unordered_map<std::string, GLFontAtlas *> FontAtlasCache::_atlasMap;

	void FontAtlasCache::purgeCachedData()
	{
		for (auto & atlas : _atlasMap)
		{
			atlas.second->purgeTexturesAtlas();
		}
	}

	GLFontAtlas * FontAtlasCache::getFontAtlasTTF(const TTFConfig & config)
	{
		bool useDistanceField = config.distanceFieldEnabled;
		if (config.outlineSize > 0)
		{
			useDistanceField = false;
		}
		int fontSize = config.fontSize;
		auto contentScaleFactor = GL_CONTENT_SCALE_FACTOR();

		if (useDistanceField)
		{
			fontSize = GLLabel::DistanceFieldFontSize / contentScaleFactor;
		}

		auto atlasName = generateFontName(config.fontFilePath, fontSize, GlyphCollection::DYNAMIC, useDistanceField);
		atlasName.append("_outline_");
		std::stringstream ss;
		ss << config.outlineSize;
		atlasName.append(ss.str());

		auto it = _atlasMap.find(atlasName);

		if (it == _atlasMap.end())
		{
			auto font = FontFreeType::create(config.fontFilePath, fontSize, config.glyphs,
				config.customGlyphs, useDistanceField, config.outlineSize);
			if (font)
			{
				auto tempAtlas = font->createFontAtlas();
				if (tempAtlas)
				{
					OUTPUT_LOG("GLFontAtlasCache create :%s", atlasName.c_str());
					_atlasMap[atlasName] = tempAtlas;
					return _atlasMap[atlasName];
				}
			}
		}
		else
		{
			OUTPUT_LOG("GLFontAtlasCache hited :%s", atlasName.c_str());
			_atlasMap[atlasName]->addref();
			return _atlasMap[atlasName];
		}

		return nullptr;
	}


	std::string FontAtlasCache::generateFontName(const std::string& fontFileName, int size, GlyphCollection theGlyphs, bool useDistanceField)
	{
		std::string tempName(fontFileName);

		switch (theGlyphs)
		{
		case GlyphCollection::DYNAMIC:
			tempName.append("_DYNAMIC_");
			break;

		case GlyphCollection::NEHE:
			tempName.append("_NEHE_");
			break;

		case GlyphCollection::ASCII:
			tempName.append("_ASCII_");
			break;

		case GlyphCollection::CUSTOM:
			tempName.append("_CUSTOM_");
			break;

		default:
			break;
		}
		if (useDistanceField)
			tempName.append("df");
		// std::to_string is not supported on android, using std::stringstream instead.
		std::stringstream ss;
		ss << size;
		return  tempName.append(ss.str());
	}

	bool FontAtlasCache::releaseFontAtlas(GLFontAtlas *atlas)
	{
		if (nullptr != atlas)
		{
			for (auto &item : _atlasMap)
			{
				if (item.second == atlas)
				{
					if (atlas->GetRefCount() == 1)
					{
						_atlasMap.erase(item.first);
					}

					atlas->Release();

					return true;
				}
			}
		}

		return false;
	}
}
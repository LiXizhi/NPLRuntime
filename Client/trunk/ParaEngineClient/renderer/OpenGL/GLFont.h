#pragma once
#include <string>

#include "GLType.h"

namespace ParaEngine
{
	class FontAtlas;

	class Font : public CRefCountedOne
	{
	public:
		virtual FontAtlas* createFontAtlas() = 0;

		virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const = 0;

		virtual int getFontMaxHeight() const { return 0; }
	};
}
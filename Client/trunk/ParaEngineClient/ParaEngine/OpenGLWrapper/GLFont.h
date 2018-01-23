#pragma once
#include <string>

#include "GLType.h"

namespace ParaEngine
{
	class GLFontAtlas;

	class Font : public CRefCountedOne
	{
	public:
		virtual GLFontAtlas* createFontAtlas() = 0;

		virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const = 0;

		virtual int getFontMaxHeight() const { return 0; }
	};
}
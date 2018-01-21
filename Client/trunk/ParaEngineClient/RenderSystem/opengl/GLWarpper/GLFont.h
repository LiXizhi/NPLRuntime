#pragma once
#include <string>
#include <memory>

#include "GLType.h"

namespace ParaEngine
{
	class FontAtlas;
	class GLFont
	{
	public:
		virtual std::shared_ptr<FontAtlas> CreateFontAtlas() = 0;
		virtual int* GetHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const = 0;
		virtual int GetFontMaxHeight() const { return 0; }
	};

	using GLFontPtr = std::shared_ptr<GLFont>;
}
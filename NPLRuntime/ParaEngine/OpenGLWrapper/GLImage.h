#pragma once
#include "GLType.h"
#include "GLTexture2D.h"
#include "ParaImage.h"

namespace ParaEngine
{
	class GLImage : public ParaImage
	{
	public:
		bool	hasPremultipliedAlpha() const { return GLTexture2D::getPixelFormatInfoMap().at(_renderFormat).compressed; }
	};
}


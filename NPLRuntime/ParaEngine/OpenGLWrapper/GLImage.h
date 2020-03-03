#pragma once
#include "GLType.h"
#include "GLTexture2D.h"
#include "ParaImage.h"

namespace ParaEngine
{
	class GLImage : public ParaImage
	{
	public:
		bool	hasPremultipliedAlpha() const 
		{ 
			auto& infoMap = GLTexture2D::getPixelFormatInfoMap();
			if (infoMap.find(_renderFormat) != infoMap.end())
				return GLTexture2D::getPixelFormatInfoMap().at(_renderFormat).compressed;
			else
			{
				return false;
			}
		}
	};
}


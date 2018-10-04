#pragma once;
#include <Framework/Common/Image.hpp>

namespace IParaEngine
{
	class IImageParser
	{
	public:
		virtual ParaEngine::ImagePtr Parse(const unsigned char* buffer, size_t buffer_size) = 0;
	};
}
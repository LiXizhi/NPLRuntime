#pragma once;
#include "Framework/Interface/IImageParser.hpp"
namespace ParaEngine
{
	class S3TCParser : public IParaEngine::IImageParser
	{
	public:

		virtual ParaEngine::ImagePtr Parse(const unsigned char* buffer, size_t buffer_size) override;
	};
}

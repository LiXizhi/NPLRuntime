#pragma once;
#include "Framework/Common/Image.hpp"
#include "Framework/Interface/IImageParser.hpp"
namespace ParaEngine
{


	class ImageParser
	{


	public:
		enum class ImageFormat
		{
			Unkown = 0,
			JPG,
			PNG,
			S3TC,
			FormatCount
		};

	public:
		ImageParser();
		~ImageParser();
		static ImagePtr ParseImage(const unsigned char* buffer,size_t buffer_size);

	protected:
		ImagePtr Parse(const unsigned char* buffer, size_t buffer_size);
		ImageFormat DetectFormat(const unsigned char* buffer, size_t buffer_size);

	private:
		IParaEngine::IImageParser*  m_Parsers[(int)ImageFormat::FormatCount];
	};
}
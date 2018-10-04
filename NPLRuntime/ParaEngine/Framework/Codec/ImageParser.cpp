#include "Framework/Interface/IImageParser.hpp"
#include "ImageParser.h"

#include "JPGParser.h"
#include "PNGParser.h"
#include "S3TCParser.h"

using namespace ParaEngine;
using namespace IParaEngine;


ParaEngine::ImageParser::ImageParser()
{
	memset(m_Parsers, 0, sizeof(m_Parsers));
	m_Parsers[(int)ImageFormat::JPG] = new JPGParser;
	m_Parsers[(int)ImageFormat::PNG] = new PNGParser;
	m_Parsers[(int)ImageFormat::S3TC] = new S3TCParser;

}

ParaEngine::ImageParser::~ImageParser()
{
	for (int i =0;i<(int)ImageFormat::FormatCount;i++)
	{
		if (m_Parsers[i] != nullptr)
		{
			delete m_Parsers[i];
			m_Parsers[i] = nullptr;
		}
	}
}

ImageParser::ImageFormat ImageParser::DetectFormat(const unsigned char* buffer, size_t buffer_size)
{
	// isJPG
	if (buffer_size > 4)
	{
		static const unsigned char JPG_SOI[] = { 0xFF, 0xD8 };
		if (memcmp(JPG_SOI, buffer,sizeof(JPG_SOI)) == 0)
		{
			return ImageFormat::JPG;
		}
	}
	// isPNG
	if (buffer_size > 8)
	{
		static const unsigned char PNG_SIGNATURE[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
		if (memcmp(PNG_SIGNATURE, buffer, sizeof(PNG_SIGNATURE)) == 0)
		{
			return ImageFormat::PNG;
		}
	}
	// isS3TC
	if (buffer_size > 4)
	{
		static const char S3TC_SIGNATURE[] = {'D','D','S'};
		if (memcmp(S3TC_SIGNATURE, buffer, sizeof(S3TC_SIGNATURE)) == 0)
		{
			return ImageFormat::S3TC;
		}
	}
	// Unkown
	return ImageFormat::Unkown;
}

ImagePtr ImageParser::ParseImage(const unsigned char* buffer, size_t buffer_size)
{
	static ImageParser parser;
	return parser.Parse(buffer, buffer_size);
}

ImagePtr ImageParser::Parse(const unsigned char* buffer, size_t buffer_size)
{
	auto format = DetectFormat(buffer, buffer_size);
	if (format == ImageParser::ImageFormat::Unkown) return nullptr;

	auto parser = m_Parsers[(int)format];

	if (parser == nullptr) return nullptr;

	return parser->Parse(buffer, buffer_size);

}


#ifdef PLATFORM_MAC

#include "CCImage.h"

USING_NS_CC;

bool Image::initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti)
{
    return true;
}

bool Image::initWithImageData(const unsigned char * data, ssize_t dataLen)
{
	return true;
}

bool Image::saveToFile(const std::string &filename, bool isToRGB)
{
    return true;
}

#endif

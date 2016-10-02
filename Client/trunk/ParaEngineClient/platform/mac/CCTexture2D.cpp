


#ifdef PLATFORM_MAC

#include "CCTexture2D.h"

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

bool Texture2D::initWithImage(Image * image)
{
	return true;
}

bool Texture2D::initWithImage(Image * image, PixelFormat format)
{
	return true;
}

 /** Gets the width of the texture in pixels. */
int Texture2D::getPixelsWide() const
{
	return 0;
}

/** Gets the height of the texture in pixels. */
int  Texture2D::getPixelsHigh() const
{
	return 0;
}



bool Texture2D::initWithData(const void *data, ssize_t dataLen, Texture2D::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize)
{
	return true;
}

GLuint Texture2D::getName() const
{
    return _name;
}

void Texture2D::setTexParameters(const TexParams& texParams)
{

}

#endif

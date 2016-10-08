
#ifndef __CC_IMAGE_H__
#define __CC_IMAGE_H__

#include "CCType.h"

#include "CCTexture2D.h"

NS_CC_BEGIN





class Image : public Ref
{
public:
	bool initWithImageData(const unsigned char * data, ssize_t dataLen);
    bool initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
    bool saveToFile(const std::string &filename, bool isToRGB = true);

	int               getWidth()              { return _width; }
	int               getHeight()             { return _height; }

	Texture2D::PixelFormat getRenderFormat()  { return _renderFormat; }

	int getNumberOfMipmaps()    { return 0; }

private:
	Texture2D::PixelFormat _renderFormat;

	GLuint _name;

    float _width;
    float _height;
};

NS_CC_END

#endif

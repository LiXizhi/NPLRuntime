
#ifndef __CC_IMAGE_H__
#define __CC_IMAGE_H__


#include "CCType.h"


#include "CCTexture2D.h"

NS_CC_BEGIN




typedef struct _MipmapInfo
{
	unsigned char* address;
	int len;
	_MipmapInfo() :address(NULL), len(0) {}
}MipmapInfo;


class Image : public Ref
{
public:

	// config
	static bool supportsS3TC;
	static bool PNG_PREMULTIPLIED_ALPHA_ENABLED;
	static int MaxTextureSize;

public:
	/** Supported formats for Image */
	enum class Format
	{
		//! JPEG
		JPG,
		//! PNG
		PNG,
		//! TIFF
		TIFF,
		//! WebP
		WEBP,
		//! PVR
		PVR,
		//! ETC
		ETC,
		//! S3TC
		S3TC,
		//! ATITC
		ATITC,
		//! TGA
		TGA,
		//! Raw Data
		RAW_DATA,
		//! Unknown format
		UNKNOWN
	};

public:

	bool initWithImageData(const unsigned char * data, ssize_t dataLen);
    bool initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
    bool saveToFile(const std::string &filename, bool isToRGB = true);


	float               getWidth()              { return _width; }
	float               getHeight()             { return _height; }

	Texture2D::PixelFormat getRenderFormat()  { return _renderFormat; }

	int getNumberOfMipmaps()    { return _numberOfMipmaps; }


	const char* getFilePath() const { return ""; }
	unsigned char* getData() const { return _data; }
	int	getDataLen() const { return _dataLen;  }
	MipmapInfo* getMipmaps() { return _mipmaps; }
	bool	isCompressed() const { return false; }
	bool	hasPremultipliedAlpha() const { return Texture2D::getPixelFormatInfoMap().at(_renderFormat).compressed; }

protected:
	static const int MIPMAP_MAX = 16;

	Texture2D::PixelFormat _renderFormat;

    float _width;
    float _height;

	bool _hasPremultipliedAlpha;
	int _dataLen;
	unsigned char* _data;

	Format _fileType;

	int _numberOfMipmaps;
	MipmapInfo _mipmaps[MIPMAP_MAX];   // pointer to mipmap images



	bool isPng(const unsigned char * data, ssize_t dataLen);
	bool isEtc(const unsigned char * data, ssize_t dataLen);
	bool isS3TC(const unsigned char * data, ssize_t dataLen);
	bool isATITC(const unsigned char *data, ssize_t dataLen);
	bool isJpg(const unsigned char * data, ssize_t dataLen);
	bool isTiff(const unsigned char * data, ssize_t dataLen);
	bool isWebp(const unsigned char * data, ssize_t dataLen);
	bool isPvr(const unsigned char * data, ssize_t dataLen);
	Format detectFormat(const unsigned char *, ssize_t unpackedLen);

	bool initWithJpgData(const unsigned char* unpackedData, ssize_t unpackedLen);
	bool initWithPngData(const unsigned char* unpackedData, ssize_t unpackedLen);
	bool initWithS3TCData(const unsigned char* unpackedData, ssize_t unpackedLen);


	void premultipliedAlpha();


};

NS_CC_END

#endif

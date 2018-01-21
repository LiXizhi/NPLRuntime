#include <cassert>
#include "GLTexture2D.h"

#include "GLImage.h"
#include "GLProgram.h"
#include "GLProgramCache.h"

using namespace ParaEngine;

namespace {
	typedef GLTexture2D::PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
	static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
	{
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::BGRA8888, GLTexture2D::PixelFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::RGBA8888, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::RGBA4444, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::RGB5A1, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::RGB565, GLTexture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::RGB888, GLTexture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::A8, GLTexture2D::PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::I8, GLTexture2D::PixelFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::AI88, GLTexture2D::PixelFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),

#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::PVRTC2, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, false)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::PVRTC2A, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, true)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::PVRTC4, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::PVRTC4A, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, true)),
#endif

#ifdef GL_ETC1_RGB8_OES
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::ETC, GLTexture2D::PixelFormatInfo(GL_ETC1_RGB8_OES, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::S3TC_DXT1, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::S3TC_DXT3, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::S3TC_DXT5, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_ATC_RGB_AMD
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::ATC_RGB, GLTexture2D::PixelFormatInfo(GL_ATC_RGB_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::ATC_EXPLICIT_ALPHA, GLTexture2D::PixelFormatInfo(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD
		PixelFormatInfoMapValue(GLTexture2D::GLPixelFormat::ATC_INTERPOLATED_ALPHA, GLTexture2D::PixelFormatInfo(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
	};
}

//The PixpelFormat corresponding information
const GLTexture2D::PixelFormatInfoMap GLTexture2D::_pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
	TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

// If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
// Default is: RGBA8888 (32-bit textures)
GLTexture2D::GLPixelFormat GLTexture2D::g_defaultAlphaPixelFormat = GLTexture2D::GLPixelFormat::DEFAULT;

const GLTexture2D::PixelFormatInfoMap& GLTexture2D::getPixelFormatInfoMap()
{
	return _pixelFormatInfoTables;
}

int ccNextPOT(int x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

#define MAX(a,b) ((a>b)?a:b)


GLTexture2D::GLTexture2D()
	: _pixelFormat(GLTexture2D::GLPixelFormat::DEFAULT)
	, _pixelsWide(0)
	, _pixelsHigh(0)
	, _name(0)
	, _maxS(0.0)
	, _maxT(0.0)
	, _hasPremultipliedAlpha(false)
	, _hasMipmaps(false)
	, _shaderProgram(nullptr)
	, _antialiasEnabled(true)
	, _valid(true)
	, _alphaTexture(nullptr)
{
}

GLTexture2D::~GLTexture2D()
{

#if CC_ENABLE_CACHE_TEXTURE_DATA
	VolatileTextureMgr::removeTexture(this);
#endif
	_alphaTexture = nullptr;

	_shaderProgram = nullptr;

	//CC_SAFE_DELETE(_ninePatchInfo);

	if (_name)
	{
		GL::deleteTexture(_name);
	}
}

bool GLTexture2D::initWithImage(Image *image)
{
	return initWithImage(image, g_defaultAlphaPixelFormat);
}

bool GLTexture2D::initWithImage(Image *image, GLPixelFormat format)
{
	if (image == nullptr)
	{
		//OUTPUT_LOG("Texture2D. Can't create Texture. UIImage is nil");
		return false;
	}

	int imageWidth = image->getWidth();
	int imageHeight = image->getHeight();
	this->_filePath = image->getFilePath();


	int maxTextureSize = Image::MaxTextureSize;
	if (imageWidth > maxTextureSize || imageHeight > maxTextureSize)
	{
		//OUTPUT_LOG("WARNING: Image (%u x %u) is bigger than the supported %u x %u", imageWidth, imageHeight, maxTextureSize, maxTextureSize);
		return false;
	}

	unsigned char*   tempData = image->getData();
	Size             imageSize = Size((float)imageWidth, (float)imageHeight);
	GLPixelFormat      pixelFormat = ((GLPixelFormat::NONE == format) || (GLPixelFormat::AUTO == format)) ? image->getRenderFormat() : format;
	GLPixelFormat      renderFormat = image->getRenderFormat();
	size_t           tempDataLen = image->getDataLen();


	if (image->getNumberOfMipmaps() > 1)
	{
		if (pixelFormat != image->getRenderFormat())
		{
			//OUTPUT_LOG("WARNING: This image has more than 1 mipmaps and we will not convert the data format");
		}

		initWithMipmaps(image->getMipmaps(), image->getNumberOfMipmaps(), image->getRenderFormat(), imageWidth, imageHeight);

		// set the premultiplied tag
		_hasPremultipliedAlpha = image->hasPremultipliedAlpha();

		return true;
	}
	else if (image->isCompressed())
	{
		if (pixelFormat != image->getRenderFormat())
		{
			//OUTPUT_LOG("WARNING: This image is compressed and we can't convert it for now");
		}

		initWithData(tempData, tempDataLen, image->getRenderFormat(), imageWidth, imageHeight, imageSize);

		// set the premultiplied tag
		_hasPremultipliedAlpha = image->hasPremultipliedAlpha();

		return true;
	}
	else
	{
		unsigned char* outTempData = nullptr;
		size_t outTempDataLen = 0;

		pixelFormat = convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);

		initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);


		if (outTempData != nullptr && outTempData != tempData)
		{

			free(outTempData);
		}

		// set the premultiplied tag
		_hasPremultipliedAlpha = image->hasPremultipliedAlpha();

		return true;
	}
}

bool GLTexture2D::updateWithData(const void *data, int offsetX, int offsetY, int width, int height)
{
	if (_name)
	{
		GL::bindTexture2D(_name);
		const PixelFormatInfo& info = _pixelFormatInfoTables.at(_pixelFormat);
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, info.format, info.type, data);

		return true;
	}
	return false;
}

void GLTexture2D::setAntiAliasTexParameters()
{
	if (_antialiasEnabled)
	{
		return;
	}

	_antialiasEnabled = true;

	if (_name == 0)
	{
		return;
	}

	GL::bindTexture2D(_name);

	if (!_hasMipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	TexParams texParams = { (GLuint)(_hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR),GL_LINEAR,GL_NONE,GL_NONE };
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}


GLTexture2D::GLPixelFormat GLTexture2D::convertDataToFormat(const unsigned char* data, size_t dataLen, GLPixelFormat originFormat, GLPixelFormat format, unsigned char** outData, size_t* outDataLen)
{
	// don't need to convert
	if (format == originFormat || format == GLPixelFormat::AUTO)
	{
		*outData = (unsigned char*)data;
		*outDataLen = dataLen;
		return originFormat;
	}

	switch (originFormat)
	{
	case GLPixelFormat::I8:
		return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
	case GLPixelFormat::AI88:
		return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
	case GLPixelFormat::RGB888:
		return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
	case GLPixelFormat::RGBA8888:
		return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
	default:
		//OUTPUT_LOG("unsupported conversion from format %d to format %d", static_cast<int>(originFormat), static_cast<int>(format));
		*outData = (unsigned char*)data;
		*outDataLen = dataLen;
		return originFormat;
	}
}

int GLTexture2D::getNumberOfMipmaps()
{
	return 0;
}

 /** Gets the width of the texture in pixels. */
int GLTexture2D::getPixelsWide() const
{

	return _pixelsWide;

}

/** Gets the height of the texture in pixels. */
int  GLTexture2D::getPixelsHigh() const
{

	return _pixelsHigh;

}



bool GLTexture2D::initWithData(const void *data, size_t dataLen, GLTexture2D::GLPixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize)
{
	assert(dataLen>0 && pixelsWide>0 && pixelsHigh>0, "Invalid size");

	//if data has no mipmaps, we will consider it has only one mipmap
	MipmapInfo mipmap;
	mipmap.address = (unsigned char*)data;
	mipmap.len = static_cast<int>(dataLen);
	return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
}

bool GLTexture2D::initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, GLPixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
{

	//the pixelFormat must be a certain value
	assert(pixelFormat != GLPixelFormat::NONE && pixelFormat != GLPixelFormat::AUTO,"the \"pixelFormat\" param must be a certain value!");
	assert(pixelsWide>0 && pixelsHigh>0, "Invalid size");

	if (mipmapsNum <= 0)
	{
		//OUTPUT_LOG("WARNING: mipmap number is less than 1");
		return false;
	}


	if (_pixelFormatInfoTables.find(pixelFormat) == _pixelFormatInfoTables.end())
	{
		//OUTPUT_LOG("WARNING: unsupported pixelformat: %lx", (unsigned long)pixelFormat);
		return false;
	}

	const PixelFormatInfo& info = _pixelFormatInfoTables.at(pixelFormat);

	//Set the row align only when mipmapsNum == 1 and the data is uncompressed
	if (mipmapsNum == 1 && !info.compressed)
	{
		unsigned int bytesPerRow = pixelsWide * info.bpp / 8;

		if (bytesPerRow % 8 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
		}
		else if (bytesPerRow % 4 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		else if (bytesPerRow % 2 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		}
		else
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
	}
	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	if (_name != 0)
	{
		GL::deleteTexture(_name);
		_name = 0;
	}

	glGenTextures(1, &_name);
	GL::bindTexture2D(_name);

	if (mipmapsNum == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if CC_ENABLE_CACHE_TEXTURE_DATA
	if (_antialiasEnabled)
	{
		TexParams texParams = { (GLuint)(_hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR),GL_LINEAR,GL_NONE,GL_NONE };
		VolatileTextureMgr::setTexParameters(this, texParams);
	}
	else
	{
		TexParams texParams = { (GLuint)(_hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST),GL_NEAREST,GL_NONE,GL_NONE };
		VolatileTextureMgr::setTexParameters(this, texParams);
	}
#endif

	// clean possible GL error
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		//OUTPUT_LOG("OpenGL error 0x%04X in %s %s %d\n", err, __FILE__, __FUNCTION__, __LINE__);
	}

	// Specify OpenGL texture image
	int width = pixelsWide;
	int height = pixelsHigh;

	for (int i = 0; i < mipmapsNum; ++i)
	{
		unsigned char *data = mipmaps[i].address;
		GLsizei datalen = mipmaps[i].len;

		if (info.compressed)
		{
			//TODO:wangpeng ???
			///glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
		}

		if (i > 0 && (width != height || ccNextPOT(width) != width))
		{
			//OUTPUT_LOG("Texture2D. WARNING. Mipmap level %u is not squared. Texture won't render correctly. width=%d != height=%d", i, width, height);
		}

		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			//OUTPUT_LOG("Texture2D: Error uploading compressed texture level: %u . glError: 0x%04X", i, err);
			return false;
		}

		width = MAX(width >> 1, 1);
		height = MAX(height >> 1, 1);
	}

	_contentSize = Size((float)pixelsWide, (float)pixelsHigh);
	_pixelsWide = pixelsWide;
	_pixelsHigh = pixelsHigh;
	_pixelFormat = pixelFormat;
	_maxS = 1;
	_maxT = 1;

	_hasPremultipliedAlpha = false;
	_hasMipmaps = mipmapsNum > 1;

	// shader
	setGLProgram(GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE));
	return true;
}

void GLTexture2D::setGLProgram(GLProgramPtr shaderProgram)
{
	_shaderProgram = shaderProgram;
}


GLuint GLTexture2D::getName() const
{
    return _name;
}

void GLTexture2D::setTexParameters(const TexParams& texParams)
{

	GL::bindTexture2D(_name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT);

#if CC_ENABLE_CACHE_TEXTURE_DATA
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

void GLTexture2D::setAliasTexParameters()
{
	if (!_antialiasEnabled)
	{
		return;
	}

	_antialiasEnabled = false;

	if (_name == 0)
	{
		return;
	}

	GL::bindTexture2D(_name);

	if (!_hasMipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	TexParams texParams = { (GLuint)(_hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST),GL_NEAREST,GL_NONE,GL_NONE };
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

GLTexture2D::GLPixelFormat GLTexture2D::convertI8ToFormat(const unsigned char* data, size_t dataLen, GLPixelFormat format, unsigned char** outData, size_t* outDataLen)
{
	switch (format)
	{
	case GLPixelFormat::RGBA8888:
		*outDataLen = dataLen * 4;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertI8ToRGBA8888(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB888:
		*outDataLen = dataLen * 3;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertI8ToRGB888(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB565:
		*outDataLen = dataLen * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertI8ToRGB565(data, dataLen, *outData);
		break;
	case GLPixelFormat::AI88:
		*outDataLen = dataLen * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertI8ToAI88(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGBA4444:
		*outDataLen = dataLen * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertI8ToRGBA4444(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB5A1:
		*outDataLen = dataLen * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertI8ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != GLPixelFormat::AUTO && format != GLPixelFormat::I8)
		{
			OUTPUT_LOG("Can not convert image format PixelFormat::I8 to format ID:%d, we will use it's origin format PixelFormat::I8", static_cast<int>(format));
		}

		*outData = (unsigned char*)data;
		*outDataLen = dataLen;
		return GLPixelFormat::I8;
	}

	return format;
}

GLTexture2D::GLPixelFormat GLTexture2D::convertAI88ToFormat(const unsigned char* data, size_t dataLen, GLPixelFormat format, unsigned char** outData, size_t* outDataLen)
{
	switch (format)
	{
	case GLPixelFormat::RGBA8888:
		*outDataLen = dataLen * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToRGBA8888(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB888:
		*outDataLen = dataLen / 2 * 3;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToRGB888(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB565:
		*outDataLen = dataLen;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToRGB565(data, dataLen, *outData);
		break;
	case GLPixelFormat::A8:
		*outDataLen = dataLen / 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToA8(data, dataLen, *outData);
		break;
	case GLPixelFormat::I8:
		*outDataLen = dataLen / 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToI8(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGBA4444:
		*outDataLen = dataLen;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToRGBA4444(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB5A1:
		*outDataLen = dataLen;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertAI88ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != GLPixelFormat::AUTO && format != GLPixelFormat::AI88)
		{
			OUTPUT_LOG("Can not convert image format PixelFormat::AI88 to format ID:%d, we will use it's origin format PixelFormat::AI88", static_cast<int>(format));
		}

		*outData = (unsigned char*)data;
		*outDataLen = dataLen;
		return GLPixelFormat::AI88;
		break;
	}

	return format;
}

GLTexture2D::GLPixelFormat GLTexture2D::convertRGB888ToFormat(const unsigned char* data, size_t dataLen, GLPixelFormat format, unsigned char** outData, size_t* outDataLen)
{
	switch (format)
	{
	case GLPixelFormat::RGBA8888:
		*outDataLen = dataLen / 3 * 4;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToRGBA8888(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB565:
		*outDataLen = dataLen / 3 * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToRGB565(data, dataLen, *outData);
		break;
	case GLPixelFormat::A8:
		*outDataLen = dataLen / 3;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToA8(data, dataLen, *outData);
		break;
	case GLPixelFormat::I8:
		*outDataLen = dataLen / 3;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToI8(data, dataLen, *outData);
		break;
	case GLPixelFormat::AI88:
		*outDataLen = dataLen / 3 * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToAI88(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGBA4444:
		*outDataLen = dataLen / 3 * 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToRGBA4444(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB5A1:
		*outDataLen = dataLen;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGB888ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != GLPixelFormat::AUTO && format != GLPixelFormat::RGB888)
		{
			OUTPUT_LOG("Can not convert image format PixelFormat::RGB888 to format ID:%d, we will use it's origin format PixelFormat::RGB888", static_cast<int>(format));
		}

		*outData = (unsigned char*)data;
		*outDataLen = dataLen;
		return GLPixelFormat::RGB888;
	}
	return format;
}

GLTexture2D::GLPixelFormat GLTexture2D::convertRGBA8888ToFormat(const unsigned char* data, size_t dataLen, GLPixelFormat format, unsigned char** outData, size_t* outDataLen)
{

	switch (format)
	{
	case GLPixelFormat::RGB888:
		*outDataLen = dataLen / 4 * 3;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToRGB888(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB565:
		*outDataLen = dataLen / 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToRGB565(data, dataLen, *outData);
		break;
	case GLPixelFormat::A8:
		*outDataLen = dataLen / 4;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToA8(data, dataLen, *outData);
		break;
	case GLPixelFormat::I8:
		*outDataLen = dataLen / 4;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToI8(data, dataLen, *outData);
		break;
	case GLPixelFormat::AI88:
		*outDataLen = dataLen / 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToAI88(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGBA4444:
		*outDataLen = dataLen / 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToRGBA4444(data, dataLen, *outData);
		break;
	case GLPixelFormat::RGB5A1:
		*outDataLen = dataLen / 2;
		*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
		convertRGBA8888ToRGB5A1(data, dataLen, *outData);
		break;
	default:
		// unsupported conversion or don't need to convert
		if (format != GLPixelFormat::AUTO && format != GLPixelFormat::RGBA8888)
		{
			//OUTPUT_LOG("Can not convert image format PixelFormat::RGBA8888 to format ID:%d, we will use it's origin format PixelFormat::RGBA8888", static_cast<int>(format));
		}

		*outData = (unsigned char*)data;
		*outDataLen = dataLen;
		return GLPixelFormat::RGBA8888;
	}

	return format;
}


// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
void GLTexture2D::convertI8ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0; i < dataLen; ++i)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
void GLTexture2D::convertAI88ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
	}
}

// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
void GLTexture2D::convertI8ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0; i < dataLen; ++i)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
		*outData++ = 0xFF;        //A
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
void GLTexture2D::convertAI88ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*outData++ = data[i];     //R
		*outData++ = data[i];     //G
		*outData++ = data[i];     //B
		*outData++ = data[i + 1]; //A
	}
}

// IIIIIIII -> RRRRRGGGGGGBBBBB
void GLTexture2D::convertI8ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (int i = 0; i < dataLen; ++i)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00FC) << 3         //G
			| (data[i] & 0x00F8) >> 3;        //B
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRGGGGGGBBBBB
void GLTexture2D::convertAI88ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00FC) << 3         //G
			| (data[i] & 0x00F8) >> 3;        //B
	}
}

// IIIIIIII -> RRRRGGGGBBBBAAAA
void GLTexture2D::convertI8ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0; i < dataLen; ++i)
	{
		*out16++ = (data[i] & 0x00F0) << 8    //R
			| (data[i] & 0x00F0) << 4             //G
			| (data[i] & 0x00F0)                  //B
			| 0x000F;                             //A
	}
}

// IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
void GLTexture2D::convertAI88ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*out16++ = (data[i] & 0x00F0) << 8    //R
			| (data[i] & 0x00F0) << 4             //G
			| (data[i] & 0x00F0)                  //B
			| (data[i + 1] & 0x00F0) >> 4;          //A
	}
}

// IIIIIIII -> RRRRRGGGGGBBBBBA
void GLTexture2D::convertI8ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (int i = 0; i < dataLen; ++i)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00F8) << 3         //G
			| (data[i] & 0x00F8) >> 2         //B
			| 0x0001;                         //A
	}
}

// IIIIIIIIAAAAAAAA -> RRRRRGGGGGBBBBBA
void GLTexture2D::convertAI88ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i] & 0x00F8) << 3         //G
			| (data[i] & 0x00F8) >> 2         //B
			| (data[i + 1] & 0x0080) >> 7;    //A
	}
}

// IIIIIIII -> IIIIIIIIAAAAAAAA
void GLTexture2D::convertI8ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0; i < dataLen; ++i)
	{
		*out16++ = 0xFF00     //A
			| data[i];            //I
	}
}

// IIIIIIIIAAAAAAAA -> AAAAAAAA
void GLTexture2D::convertAI88ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 1; i < dataLen; i += 2)
	{
		*outData++ = data[i]; //A
	}
}

// IIIIIIIIAAAAAAAA -> IIIIIIII
void GLTexture2D::convertAI88ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
	{
		*outData++ = data[i]; //R
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
void GLTexture2D::convertRGB888ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = data[i];         //R
		*outData++ = data[i + 1];     //G
		*outData++ = data[i + 2];     //B
		*outData++ = 0xFF;            //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
void GLTexture2D::convertRGBA8888ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = data[i];         //R
		*outData++ = data[i + 1];     //G
		*outData++ = data[i + 2];     //B
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
void GLTexture2D::convertRGB888ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00FC) << 3     //G
			| (data[i + 2] & 0x00F8) >> 3;    //B
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
void GLTexture2D::convertRGBA8888ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00FC) << 3     //G
			| (data[i + 2] & 0x00F8) >> 3;    //B
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> AAAAAAAA
void GLTexture2D::convertRGB888ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //A =  (R*299 + G*587 + B*114 + 500) / 1000
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
void GLTexture2D::convertRGB888ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
void GLTexture2D::convertRGBA8888ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
void GLTexture2D::convertRGBA8888ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = data[i + 3]; //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
void GLTexture2D::convertRGB888ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
		*outData++ = 0xFF;
	}
}


// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
void GLTexture2D::convertRGBA8888ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
		*outData++ = data[i + 3];
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
void GLTexture2D::convertRGB888ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*out16++ = ((data[i] & 0x00F0) << 8           //R
			| (data[i + 1] & 0x00F0) << 4     //G
			| (data[i + 2] & 0xF0)            //B
			| 0x0F);                         //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
void GLTexture2D::convertRGBA8888ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
	{
		*out16++ = (data[i] & 0x00F0) << 8    //R
			| (data[i + 1] & 0x00F0) << 4         //G
			| (data[i + 2] & 0xF0)                //B
			| (data[i + 3] & 0xF0) >> 4;         //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
void GLTexture2D::convertRGB888ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00F8) << 3     //G
			| (data[i + 2] & 0x00F8) >> 2     //B
			| 0x01;                          //A
	}
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
void GLTexture2D::convertRGBA8888ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
{
	unsigned short* out16 = (unsigned short*)outData;
	for (size_t i = 0, l = dataLen - 2; i < l; i += 4)
	{
		*out16++ = (data[i] & 0x00F8) << 8    //R
			| (data[i + 1] & 0x00F8) << 3     //G
			| (data[i + 2] & 0x00F8) >> 2     //B
			| (data[i + 3] & 0x0080) >> 7;   //A
	}
}
// converter function end
//////////////////////////////////////////////////////////////////////////

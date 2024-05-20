#include "ParaEngine.h"
#include "GLTexture2D.h"

#include "GLImage.h"
#include "ParaImage.h"
#include "GLProgram.h"
#include "GLProgramCache.h"

#ifdef WIN32
	#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

using namespace ParaEngine;


namespace {
	typedef GLTexture2D::PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
	static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
	{
		PixelFormatInfoMapValue(PixelFormat::A8B8G8R8, GLTexture2D::PixelFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A8R8G8B8, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A4R4G4B4, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A1R5G5B5, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
		PixelFormatInfoMapValue(PixelFormat::R5G6B5, GLTexture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
		PixelFormatInfoMapValue(PixelFormat::R8G8B8, GLTexture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
		PixelFormatInfoMapValue(PixelFormat::A8, GLTexture2D::PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(PixelFormat::L8, GLTexture2D::PixelFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(PixelFormat::A8L8, GLTexture2D::PixelFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),

#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
		PixelFormatInfoMapValue(PixelFormat::PVRTC2, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, false)),
		PixelFormatInfoMapValue(PixelFormat::PVRTC2A, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, true)),
		PixelFormatInfoMapValue(PixelFormat::PVRTC4, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
		PixelFormatInfoMapValue(PixelFormat::PVRTC4A, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, true)),
#endif

#ifdef GL_ETC1_RGB8_OES
		PixelFormatInfoMapValue(PixelFormat::ETC1, GLTexture2D::PixelFormatInfo(GL_ETC1_RGB8_OES, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		PixelFormatInfoMapValue(PixelFormat::DXT1, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		PixelFormatInfoMapValue(PixelFormat::DXT3, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		PixelFormatInfoMapValue(PixelFormat::DXT5, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_ATC_RGB_AMD
		PixelFormatInfoMapValue(PixelFormat::ATC_RGB, GLTexture2D::PixelFormatInfo(GL_ATC_RGB_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
		PixelFormatInfoMapValue(PixelFormat::ATC_EXPLICIT_ALPHA, GLTexture2D::PixelFormatInfo(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD
		PixelFormatInfoMapValue(PixelFormat::ATC_INTERPOLATED_ALPHA, GLTexture2D::PixelFormatInfo(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
	};
}

//The PixpelFormat corresponding information
const GLTexture2D::PixelFormatInfoMap GLTexture2D::_pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
	TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

// If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
// Default is: RGBA8888 (32-bit textures)
PixelFormat GLTexture2D::g_defaultAlphaPixelFormat = PixelFormat::Unknown;

const GLTexture2D::PixelFormatInfoMap& GLTexture2D::getPixelFormatInfoMap()
{
	return _pixelFormatInfoTables;
}

int GLTexture2D::ccNextPOT(int x)
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
	: _pixelFormat(PixelFormat::Unknown)
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
	SAFE_RELEASE(_alphaTexture); // ETC1 ALPHA support.

	SAFE_RELEASE(_shaderProgram);

	//CC_SAFE_DELETE(_ninePatchInfo);

	if (_name)
	{
		GL::deleteTexture(_name);
	}
}

bool GLTexture2D::initWithImage(const ParaImage *image)
{
	return initWithImage(image, g_defaultAlphaPixelFormat);
}

bool GLTexture2D::initWithImage(const ParaImage *image_, PixelFormat format)
{
	if (image_ == nullptr)
	{
		OUTPUT_LOG("Texture2D. Can't create Texture. UIImage is nil");
		return false;
	}

	const GLImage* image = (const GLImage*)image_;


	int imageWidth = image->getWidth();
	int imageHeight = image->getHeight();
	this->_filePath = image->getFilePath();


	int maxTextureSize = GLImage::MaxTextureSize;
	if (imageWidth > maxTextureSize || imageHeight > maxTextureSize)
	{
		OUTPUT_LOG("WARNING: Image (%u x %u) is bigger than the supported %u x %u", imageWidth, imageHeight, maxTextureSize, maxTextureSize);
		return false;
	}

	unsigned char*   tempData = image->getData();
	Size             imageSize = Size((float)imageWidth, (float)imageHeight);
	PixelFormat      pixelFormat = ((PixelFormat::Unknown == format) || (PixelFormat::COUNT == format)) ? image->getRenderFormat() : format;
	PixelFormat      renderFormat = image->getRenderFormat();
	size_t           tempDataLen = image->getDataLen();


	if (image->getNumberOfMipmaps() > 1)
	{
		if (pixelFormat != image->getRenderFormat())
		{
			OUTPUT_LOG("WARNING: This image has more than 1 mipmaps and we will not convert the data format");
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
			OUTPUT_LOG("WARNING: This image is compressed and we can't convert it for now");
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

		pixelFormat = ParaImage::convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);

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
		PE_CHECK_GL_ERROR_DEBUG();

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



bool GLTexture2D::initWithData(const void *data, size_t dataLen, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize)
{
	PE_ASSERT2(dataLen>0 && pixelsWide>0 && pixelsHigh>0, "Invalid size");

	//if data has no mipmaps, we will consider it has only one mipmap
	MipmapInfo mipmap;
	mipmap.address = (unsigned char*)data;
	mipmap.len = static_cast<int>(dataLen);
	return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
}

bool GLTexture2D::initWithMipmaps(const MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
{

	//the pixelFormat must be a certain value
	PE_ASSERT2(pixelFormat != PixelFormat::COUNT && pixelFormat != PixelFormat::Unknown, "the \"pixelFormat\" param must be a certain value!");
	PE_ASSERT2(pixelsWide>0 && pixelsHigh>0, "Invalid size");

	if (mipmapsNum <= 0)
	{
		OUTPUT_LOG("WARNING: mipmap number is less than 1");
		return false;
	}


	if (_pixelFormatInfoTables.find(pixelFormat) == _pixelFormatInfoTables.end())
	{
		OUTPUT_LOG("WARNING: unsupported pixelformat: %lx", (unsigned long)pixelFormat);
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
		// note: GL_LINEAR_MIPMAP_NEAREST is the only GL_LINEAR that makes sense to use with mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
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
		OUTPUT_LOG("OpenGL error 0x%04X in %s %s %d\n", err, __FILE__, __FUNCTION__, __LINE__);
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
			// TODO: support decoding dds image from hardware?
			if (i == 0) {
				OUTPUT_LOG("Texture2D. WARNING. Compressed Texture found and will render as black");
			}
			// glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
		}

		if (i > 0 && (width != height || ccNextPOT(width) != width))
		{
			OUTPUT_LOG("Texture2D. WARNING. Mipmap level %u is not squared. Texture won't render correctly. width=%d != height=%d", i, width, height);
		}

		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			OUTPUT_LOG("Texture2D: Error uploading compressed texture level: %u . glError: 0x%04X", i, err);
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

void GLTexture2D::setGLProgram(GLProgram* shaderProgram)
{
	if(shaderProgram)
		shaderProgram->addref();
	SAFE_RELEASE(_shaderProgram);
	_shaderProgram = shaderProgram;
}


GLuint GLTexture2D::getName() const
{
	return _name;
}

void GLTexture2D::bind()
{
	GL::bindTexture2D(this->_name);
}
 
void GLTexture2D::bindN(GLuint textureUnit)
{
	GL::bindTexture2DN(textureUnit, this->_name);
}


void GLTexture2D::bind(GLTexture2D* pTex)
{
	GL::bindTexture2D(pTex == nullptr ? 0 : pTex->_name);
}

void GLTexture2D::bindN(GLTexture2D* pTex, GLuint textureUnit)
{
	GL::bindTexture2DN(textureUnit, pTex == nullptr ? 0 : pTex->_name);
}

GLuint GLTexture2D::getName(const GLTexture2D* pTex)
{
	return pTex == nullptr ? 0 : pTex->getName();
}

void GLTexture2D::setTexParameters(const TexParams& texParams)
{

	GL::bindTexture2D(_name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT);

	PE_CHECK_GL_ERROR_DEBUG();

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

	PE_CHECK_GL_ERROR_DEBUG();
}



void GLTexture2D::RendererRecreated()
{
	_name = 0;
}

// converter function end
//////////////////////////////////////////////////////////////////////////

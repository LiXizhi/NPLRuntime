

#include "ParaEngine.h"


#ifdef PLATFORM_MAC

#include "CCImage.h"


#include "platform/base/s3tc.h"


#include "jpeglib.h"
#include "png.h"



#define MAX(a,b) ((a>b)?a:b)

#define CC_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
    (unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
    ((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
    ((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
    ((unsigned)(unsigned char)(va) << 24))


USING_NS_CC;


// config
bool Image::supportsS3TC = false;
bool Image::PNG_PREMULTIPLIED_ALPHA_ENABLED = true;
int  Image::MaxTextureSize = 1024 * 8;

#ifndef CC_ENABLE_PREMULTIPLIED_ALPHA
#define CC_ENABLE_PREMULTIPLIED_ALPHA 1
#endif

typedef struct _PVRTexHeader
{
	unsigned int headerLength;
	unsigned int height;
	unsigned int width;
	unsigned int numMipmaps;
	unsigned int flags;
	unsigned int dataLength;
	unsigned int bpp;
	unsigned int bitmaskRed;
	unsigned int bitmaskGreen;
	unsigned int bitmaskBlue;
	unsigned int bitmaskAlpha;
	unsigned int pvrTag;
	unsigned int numSurfs;
} PVRv2TexHeader;


struct DDColorKey
{
	uint32_t colorSpaceLowValue;
	uint32_t colorSpaceHighValue;
};

struct DDSCaps
{
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
};

struct DDPixelFormat
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};


struct DDSURFACEDESC2
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;

	union
	{
		uint32_t pitch;
		uint32_t linearSize;
	} DUMMYUNIONNAMEN1;

	union
	{
		uint32_t backBufferCount;
		uint32_t depth;
	} DUMMYUNIONNAMEN5;

	union
	{
		uint32_t mipMapCount;
		uint32_t refreshRate;
		uint32_t srcVBHandle;
	} DUMMYUNIONNAMEN2;

	uint32_t alphaBitDepth;
	uint32_t reserved;
	uint32_t surface;

	union
	{
		DDColorKey ddckCKDestOverlay;
		uint32_t emptyFaceColor;
	} DUMMYUNIONNAMEN3;

	DDColorKey ddckCKDestBlt;
	DDColorKey ddckCKSrcOverlay;
	DDColorKey ddckCKSrcBlt;

	union
	{
		DDPixelFormat ddpfPixelFormat;
		uint32_t FVF;
	} DUMMYUNIONNAMEN4;

	DDSCaps ddsCaps;
	uint32_t textureStage;
};

#pragma pack(push,1)

struct S3TCTexHeader
{
	char fileCode[4];
	DDSURFACEDESC2 ddsd;
};
#pragma pack(pop)


bool Image::initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti)
{
	bool ret = false;
	do
	{
		CC_BREAK_IF(0 == width || 0 == height);

		_height = height;
		_width = width;
		_hasPremultipliedAlpha = preMulti;
		_renderFormat = Texture2D::PixelFormat::RGBA8888;

		// only RGBA8888 supported
		int bytesPerComponent = 4;
		_dataLen = height * width * bytesPerComponent;
		_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
		CC_BREAK_IF(!_data);
		memcpy(_data, data, _dataLen);

		ret = true;
	} while (0);

	return ret;

}

bool Image::initWithImageData(const unsigned char * data, ssize_t dataLen)
{

	bool ret = false;

	do
	{
		CC_BREAK_IF(!data || dataLen <= 0);

		unsigned char* unpackedData = nullptr;
		ssize_t unpackedLen = 0;

		unpackedData = const_cast<unsigned char*>(data);
		unpackedLen = dataLen;


		_fileType = detectFormat(unpackedData, unpackedLen);

		switch (_fileType)
		{
		case Format::PNG:
			ret = initWithPngData(unpackedData, unpackedLen);
			break;
		case Format::JPG:
			ret = initWithJpgData(unpackedData, unpackedLen);
			break;
		case Format::S3TC:
			ret = initWithS3TCData(unpackedData, unpackedLen);
			break;
		default:
			CCLOG("unsupported image format!");
		}

		if (unpackedData != data)
		{
			free(unpackedData);
		}
	} while (0);

	return ret;
}



bool Image::isPng(const unsigned char * data, ssize_t dataLen)
{
	if (dataLen <= 8)
	{
		return false;
	}

	static const unsigned char PNG_SIGNATURE[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

	return memcmp(PNG_SIGNATURE, data, sizeof(PNG_SIGNATURE)) == 0;
}


bool Image::isEtc(const unsigned char * data, ssize_t dataLen)
{
	return false;
}


bool Image::isS3TC(const unsigned char * data, ssize_t dataLen)
{

	S3TCTexHeader *header = (S3TCTexHeader *)data;

	if (strncmp(header->fileCode, "DDS", 3) != 0)
	{
		return false;
	}
	return true;
}

bool Image::isATITC(const unsigned char *data, ssize_t dataLen)
{
	return false;
}

bool Image::isJpg(const unsigned char * data, ssize_t dataLen)
{
	if (dataLen <= 4)
	{
		return false;
	}

	static const unsigned char JPG_SOI[] = { 0xFF, 0xD8 };

	return memcmp(data, JPG_SOI, 2) == 0;
}

bool Image::isTiff(const unsigned char * data, ssize_t dataLen)
{
	if (dataLen <= 4)
	{
		return false;
	}

	static const char* TIFF_II = "II";
	static const char* TIFF_MM = "MM";

	return (memcmp(data, TIFF_II, 2) == 0 && *(static_cast<const unsigned char*>(data) + 2) == 42 && *(static_cast<const unsigned char*>(data) + 3) == 0) ||
		(memcmp(data, TIFF_MM, 2) == 0 && *(static_cast<const unsigned char*>(data) + 2) == 0 && *(static_cast<const unsigned char*>(data) + 3) == 42);
}

bool Image::isWebp(const unsigned char * data, ssize_t dataLen)
{
	if (dataLen <= 12)
	{
		return false;
	}

	static const char* WEBP_RIFF = "RIFF";
	static const char* WEBP_WEBP = "WEBP";

	return memcmp(data, WEBP_RIFF, 4) == 0
		&& memcmp(static_cast<const unsigned char*>(data) + 8, WEBP_WEBP, 4) == 0;
}

bool Image::isPvr(const unsigned char * data, ssize_t dataLen)
{
	return false;
}

Image::Format Image::detectFormat(const unsigned char * data, ssize_t dataLen)
{
	if (isPng(data, dataLen))
	{
		return Format::PNG;
	}
	else if (isJpg(data, dataLen))
	{
		return Format::JPG;
	}
	else if (isTiff(data, dataLen))
	{
		return Format::TIFF;
	}
	else if (isWebp(data, dataLen))
	{
		return Format::WEBP;
	}
	else if (isPvr(data, dataLen))
	{
		return Format::PVR;
	}
	else if (isEtc(data, dataLen))
	{
		return Format::ETC;
	}
	else if (isS3TC(data, dataLen))
	{
		return Format::S3TC;
	}
	else if (isATITC(data, dataLen))
	{
		return Format::ATITC;
	}
	else
	{
		return Format::UNKNOWN;
	}
}


#define CC_USE_JPEG 1
namespace
{
	/*
	* ERROR HANDLING:
	*
	* The JPEG library's standard error handler (jerror.c) is divided into
	* several "methods" which you can override individually.  This lets you
	* adjust the behavior without duplicating a lot of code, which you might
	* have to update with each future release.
	*
	* We override the "error_exit" method so that control is returned to the
	* library's caller when a fatal error occurs, rather than calling exit()
	* as the standard error_exit method does.
	*
	* We use C's setjmp/longjmp facility to return control.  This means that the
	* routine which calls the JPEG library must first execute a setjmp() call to
	* establish the return point.  We want the replacement error_exit to do a
	* longjmp().  But we need to make the setjmp buffer accessible to the
	* error_exit routine.  To do this, we make a private extension of the
	* standard JPEG error handler object.  (If we were using C++, we'd say we
	* were making a subclass of the regular error handler.)
	*
	* Here's the extended error handler struct:
	*/
#if CC_USE_JPEG
	struct MyErrorMgr
	{
		struct jpeg_error_mgr pub;  /* "public" fields */
		jmp_buf setjmp_buffer;  /* for return to caller */
	};

	typedef struct MyErrorMgr * MyErrorPtr;

	/*
	* Here's the routine that will replace the standard error_exit method:
	*/

	METHODDEF(void)
		myErrorExit(j_common_ptr cinfo)
	{
		/* cinfo->err really points to a MyErrorMgr struct, so coerce pointer */
		MyErrorPtr myerr = (MyErrorPtr)cinfo->err;

		/* Always display the message. */
		/* We could postpone this until after returning, if we chose. */
		/* internal message function can't show error message in some platforms, so we rewrite it here.
		* edit it if has version conflict.
		*/
		//(*cinfo->err->output_message) (cinfo);
		char buffer[JMSG_LENGTH_MAX];
		(*cinfo->err->format_message) (cinfo, buffer);
		CCLOG("jpeg error: %s", buffer);

		/* Return control to the setjmp point */
		longjmp(myerr->setjmp_buffer, 1);
	}
#endif // CC_USE_JPEG
}

bool Image::initWithJpgData(const unsigned char * data, ssize_t dataLen)
{
#if CC_USE_WIC
	return decodeWithWIC(data, dataLen);
#else
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct MyErrorMgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1] = { 0 };
	unsigned long location = 0;

	bool ret = false;
	do
	{
		/* We set up the normal JPEG error routines, then override error_exit. */
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = myErrorExit;
		/* Establish the setjmp return context for MyErrorExit to use. */
		if (setjmp(jerr.setjmp_buffer))
		{
			/* If we get here, the JPEG code has signaled an error.
			* We need to clean up the JPEG object, close the input file, and return.
			*/
			jpeg_destroy_decompress(&cinfo);
			break;
		}

		/* setup decompression process and source, then read JPEG header */
		jpeg_create_decompress(&cinfo);

#ifndef CC_TARGET_QT5
		jpeg_mem_src(&cinfo, const_cast<unsigned char*>(data), dataLen);
#endif /* CC_TARGET_QT5 */

		/* reading the image header which contains image information */
#if (JPEG_LIB_VERSION >= 90)
		// libjpeg 0.9 adds stricter types.
		jpeg_read_header(&cinfo, TRUE);
#else
		jpeg_read_header(&cinfo, TRUE);
#endif

		// we only support RGB or grayscale
		if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
		{
			_renderFormat = Texture2D::PixelFormat::I8;
		}
		else
		{
			cinfo.out_color_space = JCS_RGB;
			_renderFormat = Texture2D::PixelFormat::RGB888;
		}

		/* Start decompression jpeg here */
		jpeg_start_decompress(&cinfo);

		/* init image info */
		_width = cinfo.output_width;
		_height = cinfo.output_height;

		_dataLen = cinfo.output_width*cinfo.output_height*cinfo.output_components;
		_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
		CC_BREAK_IF(!_data);

		/* now actually read the jpeg into the raw buffer */
		/* read one scan line at a time */
		while (cinfo.output_scanline < cinfo.output_height)
		{
			row_pointer[0] = _data + location;
			location += cinfo.output_width*cinfo.output_components;
			jpeg_read_scanlines(&cinfo, row_pointer, 1);
		}

		/* When read image file with broken data, jpeg_finish_decompress() may cause error.
		* Besides, jpeg_destroy_decompress() shall deallocate and release all memory associated
		* with the decompression object.
		* So it doesn't need to call jpeg_finish_decompress().
		*/
		//jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress(&cinfo);
		/* wrap up decompression, destroy objects, free pointers and close open files */
		ret = true;
	} while (0);

	return ret;
#endif
}

namespace
{
	typedef struct
	{
		const unsigned char * data;
		ssize_t size;
		int offset;
	}tImageSource;


	static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		tImageSource* isource = (tImageSource*)png_get_io_ptr(png_ptr);

		if ((int)(isource->offset + length) <= isource->size)
		{
			memcpy(data, isource->data + isource->offset, length);
			isource->offset += length;
		}
		else
		{
			png_error(png_ptr, "pngReaderCallback failed");
		}
	}
}

bool Image::initWithPngData(const unsigned char * data, ssize_t dataLen)
{
#if CC_USE_WIC
	return decodeWithWIC(data, dataLen);
#else
	// length of bytes to check if it is a valid png file
#define PNGSIGSIZE  8
	bool ret = false;
	png_byte        header[PNGSIGSIZE] = { 0 };
	png_structp     png_ptr = 0;
	png_infop       info_ptr = 0;

	do
	{
		// png header len is 8 bytes
		CC_BREAK_IF(dataLen < PNGSIGSIZE);

		// check the data is png or not
		memcpy(header, data, PNGSIGSIZE);
		CC_BREAK_IF(png_sig_cmp(header, 0, PNGSIGSIZE));

		// init png_struct
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		CC_BREAK_IF(!png_ptr);

		// init png_info
		info_ptr = png_create_info_struct(png_ptr);
		CC_BREAK_IF(!info_ptr);

#if (PARA_TARGET_PLATFORM != PARA_PLATFORM_BADA && PARA_TARGET_PLATFORM != PARA_PLATFORM_NACL && PARA_TARGET_PLATFORM != PARA_PLATFORM_TIZEN)
		CC_BREAK_IF(setjmp(png_jmpbuf(png_ptr)));
#endif

		// set the read call back function
		tImageSource imageSource;
		imageSource.data = (unsigned char*)data;
		imageSource.size = dataLen;
		imageSource.offset = 0;
		png_set_read_fn(png_ptr, &imageSource, pngReadCallback);

		// read png header info

		// read png file info
		png_read_info(png_ptr, info_ptr);

		_width = png_get_image_width(png_ptr, info_ptr);
		_height = png_get_image_height(png_ptr, info_ptr);
		png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

		//CCLOG("color type %u", color_type);

		// force palette images to be expanded to 24-bit RGB
		// it may include alpha channel
		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_palette_to_rgb(png_ptr);
		}
		// low-bit-depth grayscale images are to be expanded to 8 bits
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		{
			bit_depth = 8;
			png_set_expand_gray_1_2_4_to_8(png_ptr);
		}
		// expand any tRNS chunk data into a full alpha channel
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(png_ptr);
		}
		// reduce images with 16-bit samples to 8 bits
		if (bit_depth == 16)
		{
			png_set_strip_16(png_ptr);
		}

		// Expanded earlier for grayscale, now take care of palette and rgb
		if (bit_depth < 8)
		{
			png_set_packing(png_ptr);
		}
		// update info
		png_read_update_info(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);

		switch (color_type)
		{
		case PNG_COLOR_TYPE_GRAY:
			_renderFormat = Texture2D::PixelFormat::I8;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			_renderFormat = Texture2D::PixelFormat::AI88;
			break;
		case PNG_COLOR_TYPE_RGB:
			_renderFormat = Texture2D::PixelFormat::RGB888;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			_renderFormat = Texture2D::PixelFormat::RGBA8888;
			break;
		default:
			break;
		}

		// read png data
		png_size_t rowbytes;
		png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * _height);

		rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		_dataLen = rowbytes * _height;
		_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
		if (!_data)
		{
			if (row_pointers != nullptr)
			{
				free(row_pointers);
			}
			break;
		}

		for (unsigned short i = 0; i < _height; ++i)
		{
			row_pointers[i] = _data + i*rowbytes;
		}
		png_read_image(png_ptr, row_pointers);

		png_read_end(png_ptr, nullptr);

		// premultiplied alpha for RGBA8888
		if (PNG_PREMULTIPLIED_ALPHA_ENABLED && color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		{
			premultipliedAlpha();
		}

		if (row_pointers != nullptr)
		{
			free(row_pointers);
		}

		ret = true;
	} while (0);

	if (png_ptr)
	{
		png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
	}
	return ret;
#endif
}

namespace
{
	static uint32_t makeFourCC(char ch0, char ch1, char ch2, char ch3)
	{
		const uint32_t fourCC = ((uint32_t)(char)(ch0) | ((uint32_t)(char)(ch1) << 8) | ((uint32_t)(char)(ch2) << 16) | ((uint32_t)(char)(ch3) << 24));
		return fourCC;
	}
}

bool Image::initWithS3TCData(const unsigned char * data, ssize_t dataLen)
{
	const uint32_t FOURCC_DXT1 = makeFourCC('D', 'X', 'T', '1');
	const uint32_t FOURCC_DXT3 = makeFourCC('D', 'X', 'T', '3');
	const uint32_t FOURCC_DXT5 = makeFourCC('D', 'X', 'T', '5');

	/* load the .dds file */

	S3TCTexHeader *header = (S3TCTexHeader *)data;
	unsigned char *pixelData = static_cast<unsigned char*>(malloc((dataLen - sizeof(S3TCTexHeader)) * sizeof(unsigned char)));
	memcpy((void *)pixelData, data + sizeof(S3TCTexHeader), dataLen - sizeof(S3TCTexHeader));

	_width = header->ddsd.width;
	_height = header->ddsd.height;
	_numberOfMipmaps = MAX(1, header->ddsd.DUMMYUNIONNAMEN2.mipMapCount); //if dds header reports 0 mipmaps, set to 1 to force correct software decoding (if needed).
	_dataLen = 0;
	int blockSize = (FOURCC_DXT1 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC) ? 8 : 16;

	/* calculate the dataLen */

	int width = _width;
	int height = _height;

	if (supportsS3TC)  //compressed data length
	{
		_dataLen = dataLen - sizeof(S3TCTexHeader);
		_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
		memcpy((void *)_data, (void *)pixelData, _dataLen);
	}
	else                                               //decompressed data length
	{
		for (int i = 0; i < _numberOfMipmaps && (width || height); ++i)
		{
			if (width == 0) width = 1;
			if (height == 0) height = 1;

			_dataLen += (height * width * 4);

			width >>= 1;
			height >>= 1;
		}
		_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
	}

	/* if hardware supports s3tc, set pixelformat before loading mipmaps, to support non-mipmapped textures  */
	if (supportsS3TC)
	{   //decode texture through hardware

		if (FOURCC_DXT1 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
		{
			_renderFormat = Texture2D::PixelFormat::S3TC_DXT1;
		}
		else if (FOURCC_DXT3 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
		{
			_renderFormat = Texture2D::PixelFormat::S3TC_DXT3;
		}
		else if (FOURCC_DXT5 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
		{
			_renderFormat = Texture2D::PixelFormat::S3TC_DXT5;
		}
	}
	else { //will software decode
		_renderFormat = Texture2D::PixelFormat::RGBA8888;
	}

	/* load the mipmaps */

	int encodeOffset = 0;
	int decodeOffset = 0;
	width = _width;  height = _height;

	for (int i = 0; i < _numberOfMipmaps && (width || height); ++i)
	{
		if (width == 0) width = 1;
		if (height == 0) height = 1;

		int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;

		if (supportsS3TC)
		{   //decode texture through hardware
			_mipmaps[i].address = (unsigned char *)_data + encodeOffset;
			_mipmaps[i].len = size;
		}
		else
		{   //if it is not gles or device do not support S3TC, decode texture by software

			CCLOG("cocos2d: Hardware S3TC decoder not present. Using software decoder");

			int bytePerPixel = 4;
			unsigned int stride = width * bytePerPixel;

			std::vector<unsigned char> decodeImageData(stride * height);
			if (FOURCC_DXT1 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
			{
				s3tc_decode(pixelData + encodeOffset, &decodeImageData[0], width, height, S3TCDecodeFlag::DXT1);
			}
			else if (FOURCC_DXT3 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
			{
				s3tc_decode(pixelData + encodeOffset, &decodeImageData[0], width, height, S3TCDecodeFlag::DXT3);
			}
			else if (FOURCC_DXT5 == header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC)
			{
				s3tc_decode(pixelData + encodeOffset, &decodeImageData[0], width, height, S3TCDecodeFlag::DXT5);
			}

			_mipmaps[i].address = (unsigned char *)_data + decodeOffset;
			_mipmaps[i].len = (stride * height);
			memcpy((void *)_mipmaps[i].address, (void *)&decodeImageData[0], _mipmaps[i].len);
			decodeOffset += stride * height;
		}

		encodeOffset += size;
		width >>= 1;
		height >>= 1;
	}

	/* end load the mipmaps */

	if (pixelData != nullptr)
	{
		free(pixelData);
	};

	return true;
}

bool Image::saveToFile(const std::string &filename, bool isToRGB)

{
    return true;
}


void Image::premultipliedAlpha()
{
#if CC_ENABLE_PREMULTIPLIED_ALPHA == 0
	_hasPremultipliedAlpha = false;
	return;
#else
	CCASSERT(_renderFormat == Texture2D::PixelFormat::RGBA8888, "The pixel format should be RGBA8888!");

	unsigned int* fourBytes = (unsigned int*)_data;
	for (int i = 0; i < _width * _height; i++)
	{
		unsigned char* p = _data + i * 4;
		fourBytes[i] = CC_RGB_PREMULTIPLY_ALPHA(p[0], p[1], p[2], p[3]);
	}

	_hasPremultipliedAlpha = true;
#endif
}


#endif

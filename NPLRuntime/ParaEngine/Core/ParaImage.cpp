#include "ParaEngine.h"
#include "ParaImage.h"
#include "s3tc.h"
#include "RHISoftwareS3TC.h"
#include <limits>
#include "jpeglib.h"
#include "png.h"
#include "StringHelper.h"

#include <boost/gil/extension/numeric/sampler.hpp>
#include <boost/gil/extension/numeric/resample.hpp>

#include <boost/gil/extension/io/bmp.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/image_view_factory.hpp>

#ifdef WIN32
// just for compatibility with previous version of libpng.lib built by earlier version of visual studio 2015
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#endif


#define CC_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
    (unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
    ((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
    ((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
    ((unsigned)(unsigned char)(va) << 24))

#define CC_ENABLE_PREMULTIPLIED_ALPHA 0

namespace ParaEngine
{
	// config
#ifdef WIN32
	// it does not support decoding dds texture by hardware
	bool ParaImage::supportsS3TC = false;
#else
	bool ParaImage::supportsS3TC = false;
#endif
	bool ParaImage::PNG_PREMULTIPLIED_ALPHA_ENABLED = false;
	int  ParaImage::MaxTextureSize = 1024 * 8;

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

	ParaImage::ParaImage()
		: _width(0)
		, _height(0)
		, _hasPremultipliedAlpha(false)
		, _dataLen(0)
		, _data(nullptr)
		, _numberOfMipmaps(0)
	{

	}

	ParaImage::~ParaImage()
	{
		if (_data)
		{
			free(_data);
		}
		
	}

	template <typename AnyView>
	static void gray_image_hist(const AnyView& img_view, unsigned char* data)
	{
		for (typename AnyView::iterator it = img_view.begin(); it != img_view.end(); ++it)
		{
			auto color = *it;
			memcpy(data, &color, sizeof(color));
			data += sizeof(color);
		}
	}

	bool ParaImage::resize(int width, int height)
	{
		using namespace boost::gil;

		if (!_data)
			return false;

		auto old_width = (int)getWidth();
		auto old_height = (int)getHeight();

		if (width == old_width && height == old_height)
			return true;

		if (isCompressed())
			return false;

		auto format = getRenderFormat();

		int bytesPerComponent;

		switch (format)
		{
		case PixelFormat::L8:
			{
				bytesPerComponent = 1;
				_dataLen = height * width * bytesPerComponent;


				gray8c_view_t src_view = interleaved_view((std::size_t)old_width, (std::size_t)old_height, (const gray8_pixel_t*)_data, old_width * 1);
				gray8_image_t scale_img(width, height);
				auto scale_view = view(scale_img);

				resize_view(src_view, scale_view, bilinear_sampler());

				free(_data);
				_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));

				gray_image_hist(scale_view, _data);
			}
			break;
		case PixelFormat::R8G8B8:
			{
				bytesPerComponent = 3;
                _dataLen = height * width * bytesPerComponent;
				
				/*
				unsigned char *tempData = static_cast<unsigned char*>(malloc(old_width * old_height * 3 * sizeof(unsigned char)));
				
				for (int i = 0; i < old_width; ++i)
				{
					for (int j = 0; j < old_height; ++j)

					{
						tempData[(i * old_width + j) * 3] = _data[(i * old_width + j) * 4];
						tempData[(i * old_width + j) * 3 + 1] = _data[(i * old_width + j) * 4 + 1];
						tempData[(i * old_width + j) * 3 + 2] = _data[(i * old_width + j) * 4 + 2];
					}
				}
				*/

				rgb8c_view_t src_view = interleaved_view((std::size_t)old_width, (std::size_t)old_height, (const rgb8_pixel_t*)_data, old_width * 3);
				rgb8_image_t scale_img(width, height);
				auto scale_view = view(scale_img);

				resize_view(src_view, scale_view, bilinear_sampler());

				free(_data);
				//free(tempData);
				_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));

				gray_image_hist(scale_view, _data);
			}

			break;
		case PixelFormat::A8R8G8B8:
			{
				bytesPerComponent = 4;
				_dataLen = height * width * bytesPerComponent;

				rgba8c_view_t src_view = interleaved_view((std::size_t)old_width, (std::size_t)old_height, (const rgba8_pixel_t*)_data, old_width * 4);
				rgba8_image_t scale_img(width, height);
				auto scale_view = view(scale_img);

				resize_view(src_view, scale_view, bilinear_sampler());

				free(_data);
				_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));

				gray_image_hist(scale_view, _data);
			}
			
			break;
		default:
			return false;
		}

		_height = (float)height;
		_width = (float)width;

		return true;
	}

	bool ParaImage::initWithRawData(const unsigned char * data, size_t dataLen, int width, int height, int bitsPerComponent, bool preMulti)
	{
		bool ret = false;
		do
		{
			if (0 == width || 0 == height)
				break;

			_height = (float)height;
			_width = (float)width;
			_hasPremultipliedAlpha = preMulti;
			_renderFormat = PixelFormat::A8R8G8B8;

			// only RGBA8888 supported
			int bytesPerComponent = 4;
			_dataLen = height * width * bytesPerComponent;
			_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
			if (!_data)
				break;
			memcpy(_data, data, _dataLen);

			ret = true;
		} while (0);

		return ret;
	}

	int ParaImage::getBitsPrePixel() const
	{
		static int s[(size_t)PixelFormat::COUNT] =
		{
			-1, 24, 32, 32, 16, 16, 16, 16, 8, 8, 16, 16, 32, 32, 32, 32, 32, 64, 16, 8, 8, 16, 8, 16, 16, 32, 32, 32, 32, -1, -1, -1, -1, 4, 8, 8, 8, 8, -1, 32, 16, 32, 32, 32, 16, -1, -1, -1, 16, 32, 4, 4, 2, 2, 4, -1, -1, -1
		};

		return s[(size_t)_renderFormat];
	}

	bool ParaImage::initWithImageData(const unsigned char * data, size_t dataLen, bool preserveCompressed)
	{

		bool ret = false;

		do
		{
			if (!data || dataLen <= 0)
				break;

			unsigned char* unpackedData = nullptr;
			size_t unpackedLen = 0;

			unpackedData = const_cast<unsigned char*>(data);
			unpackedLen = dataLen;


			_fileType = detectFormat(unpackedData, unpackedLen);

			switch (_fileType)
			{
			case Format::PNG:
				ret = initWithPngData(unpackedData, unpackedLen);
				break;
			case Format::BMP:
				try
				{
					boost::iostreams::stream<boost::iostreams::array_source> stream(
						reinterpret_cast<const char*>(unpackedData), unpackedLen);
					auto info = boost::gil::read_image_info(stream, boost::gil::bmp_tag());
					const auto w = info._info._width;
					const auto h = info._info._height;
					if (w <= 0 || h <= 0 || w > MaxTextureSize || h > MaxTextureSize) break;
					stream.clear();
					stream.seekg(0);
					boost::gil::rgba8_image_t bitmap;
					boost::gil::read_and_convert_image(stream, bitmap, boost::gil::bmp_tag());
					auto pixels = boost::gil::const_view(bitmap);
					ret = initWithRawData(reinterpret_cast<const unsigned char*>(&pixels(0,0)),
						static_cast<size_t>(w) * h * 4, w, h, 32);
				}
				catch (const std::exception&)
				{
					ret = false;
				}
				break;
			case Format::JPG:
				ret = initWithJpgData(unpackedData, unpackedLen);
				break;
			case Format::S3TC:
				ret = initWithS3TCData(unpackedData, unpackedLen, preserveCompressed || supportsS3TC);
				break;
			default:
				OUTPUT_LOG("unsupported image format!");
			}

			if (unpackedData != data)
			{
				free(unpackedData);
			}
		} while (0);

		return ret;
	}

	bool ParaImage::isPng(const unsigned char * data, size_t dataLen)
	{
		if (dataLen <= 8)
		{
			return false;
		}

		static const unsigned char PNG_SIGNATURE[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

		return memcmp(PNG_SIGNATURE, data, sizeof(PNG_SIGNATURE)) == 0;
	}

	bool ParaImage::isEtc(const unsigned char * data, size_t dataLen)
	{
		return false;
	}

	bool ParaImage::isS3TC(const unsigned char * data, size_t dataLen)
	{
		return data && dataLen >= 4 && memcmp(data, "DDS ", 4) == 0;
	}

	bool ParaImage::isATITC(const unsigned char *data, size_t dataLen)
	{
		return false;
	}

	bool ParaImage::isJpg(const unsigned char * data, size_t dataLen)
	{
		if (dataLen <= 4)
		{
			return false;
		}

		static const unsigned char JPG_SOI[] = { 0xFF, 0xD8 };

		return memcmp(data, JPG_SOI, 2) == 0;
	}

	bool ParaImage::isTiff(const unsigned char * data, size_t dataLen)
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

	bool ParaImage::isWebp(const unsigned char * data, size_t dataLen)
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

	bool ParaImage::isPvr(const unsigned char * data, size_t dataLen)
	{
		return false;
	}

	ParaImage::Format ParaImage::detectFormat(const unsigned char * data, size_t dataLen)
	{
		if (dataLen >= 14 && data[0] == 'B' && data[1] == 'M')
			return Format::BMP;
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
			OUTPUT_LOG("jpeg error: %s", buffer);

			/* Return control to the setjmp point */
			longjmp(myerr->setjmp_buffer, 1);
		}
#endif // CC_USE_JPEG
	}

	bool ParaImage::initWithJpgData(const unsigned char * data, size_t dataLen)
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
			jpeg_mem_src(&cinfo, const_cast<unsigned char*>(data), (unsigned long)dataLen);
#endif /* CC_TARGET_QT5 */

			/* reading the image header which contains image information */
#if (JPEG_LIB_VERSION >= 90)
			// libjpeg 0.9 adds stricter types.
			jpeg_read_header(&cinfo, (boolean)1);
#else
			jpeg_read_header(&cinfo, TRUE);
#endif

			// we only support RGB or grayscale
			if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
			{
				_renderFormat = PixelFormat::L8;
			}
			else
			{
				cinfo.out_color_space = JCS_RGB;
				_renderFormat = PixelFormat::R8G8B8;
			}

			/* Start decompression jpeg here */
			jpeg_start_decompress(&cinfo);

			/* init image info */
			_width = (float)cinfo.output_width;
			_height = (float)cinfo.output_height;

			_dataLen = cinfo.output_width*cinfo.output_height*cinfo.output_components;
			_data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
			if (!_data)
				break;

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
			size_t size;
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

	bool ParaImage::initWithPngData(const unsigned char * data, size_t dataLen)
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
			if (dataLen < PNGSIGSIZE)
				break;

			// check the data is png or not
			memcpy(header, data, PNGSIGSIZE);
			if (png_sig_cmp(header, 0, PNGSIGSIZE))
				break;

			// init png_struct
			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
			if (!png_ptr) break;

			// init png_info
			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) break;

#if (PARA_TARGET_PLATFORM != PARA_PLATFORM_BADA && PARA_TARGET_PLATFORM != PARA_PLATFORM_NACL && PARA_TARGET_PLATFORM != PARA_PLATFORM_TIZEN)
			if (setjmp(png_jmpbuf(png_ptr))) break;
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

			//OUTPUT_LOG("color type %u", color_type);

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
				_renderFormat = PixelFormat::L8;
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				_renderFormat = PixelFormat::A8L8;
				break;
			case PNG_COLOR_TYPE_RGB:
				_renderFormat = PixelFormat::R8G8B8;
				break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				_renderFormat = PixelFormat::A8R8G8B8;
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
				row_pointers[i] = _data + i * rowbytes;
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


	bool ParaImage::initWithS3TCData(const unsigned char* data, size_t dataLen, bool preserveCompressed)
	{
		if (!data || dataLen < sizeof(S3TCTexHeader))
			return false;
		S3TCTexHeader header;
		memcpy(&header, data, sizeof(header));
		const auto& desc = header.ddsd;
		const uint32_t fourCC = desc.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC;
		PixelFormat format = PixelFormat::Unknown;
		if (fourCC == makeFourCC('D', 'X', 'T', '1')) format = PixelFormat::DXT1;
		else if (fourCC == makeFourCC('D', 'X', 'T', '3')) format = PixelFormat::DXT3;
		else if (fourCC == makeFourCC('D', 'X', 'T', '5')) format = PixelFormat::DXT5;
		else return false;

		uint32_t width = desc.width, height = desc.height;
		const uint32_t mipCount = (std::max)(1u, desc.DUMMYUNIONNAMEN2.mipMapCount);
		if (!width || !height || width > static_cast<uint32_t>(MaxTextureSize) ||
			height > static_cast<uint32_t>(MaxTextureSize) || mipCount > MIPMAP_MAX)
			return false;
		const size_t payloadSize = dataLen - sizeof(header);
		size_t encodedSize = 0, decodedSize = 0;
		for (uint32_t i = 0; i < mipCount; ++i)
		{
			const size_t bytes = RHISoftwareS3TC::ComputeCompressedMipSize(format, width, height);
			if (!bytes || bytes > payloadSize - encodedSize)
				return false;
			encodedSize += bytes;
			decodedSize += static_cast<size_t>(width) * height * 4;
			if (width == 1 && height == 1 && i + 1 < mipCount)
				return false;
			width = (std::max)(1u, width >> 1);
			height = (std::max)(1u, height >> 1);
		}
		const size_t outputSize = preserveCompressed ? encodedSize : decodedSize;
		if (outputSize > static_cast<size_t>((std::numeric_limits<int>::max)()))
			return false;
		unsigned char* output = static_cast<unsigned char*>(malloc(outputSize));
		if (!output)
			return false;
		const unsigned char* source = data + sizeof(header);
		if (preserveCompressed)
			memcpy(output, source, outputSize);
		width = desc.width; height = desc.height;
		size_t srcOffset = 0, dstOffset = 0;
		for (uint32_t i = 0; i < mipCount; ++i)
		{
			const size_t bytes = RHISoftwareS3TC::ComputeCompressedMipSize(format, width, height);
			const size_t mipSize = preserveCompressed ? bytes : static_cast<size_t>(width) * height * 4;
			if (!preserveCompressed)
			{
				std::vector<uint8_t> rgba;
				if (!RHISoftwareS3TC::DecodeToRGBA8(format, source + srcOffset, bytes, width, height, rgba))
				{
					free(output);
					return false;
				}
				memcpy(output + dstOffset, rgba.data(), mipSize);
			}
			_mipmaps[i].address = output + dstOffset;
			_mipmaps[i].len = static_cast<int>(mipSize);
			srcOffset += bytes;
			dstOffset += mipSize;
			width = (std::max)(1u, width >> 1);
			height = (std::max)(1u, height >> 1);
		}
		free(_data);
		_data = output;
		_dataLen = static_cast<int>(outputSize);
		_width = static_cast<float>(desc.width);
		_height = static_cast<float>(desc.height);
		_numberOfMipmaps = static_cast<int>(mipCount);
		_renderFormat = preserveCompressed ? format : PixelFormat::A8R8G8B8;
		_hasPremultipliedAlpha = false;
		return true;
	}


	namespace
	{
		bool SaveRawImageToBMPFile(const std::string& filePath, const unsigned char* data, int width, int height, PixelFormat fmt)
		{
			if (!data || width <= 0 || height <= 0)
				return false;
			if (fmt != PixelFormat::A8R8G8B8 && fmt != PixelFormat::R8G8B8)
				return false;

			FILE* fp = nullptr;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			std::wstring filePath16 = StringHelper::MultiByteToWideChar(filePath.c_str(), DEFAULT_FILE_ENCODING);
			fp = _wfopen(filePath16.c_str(), L"wb");
#else
			fp = fopen(filePath.c_str(), "wb");
#endif
			if (!fp)
				return false;

			const uint32_t rowStride = ((static_cast<uint32_t>(width) * 3u + 3u) & ~3u);
			const uint32_t imgBytes = rowStride * static_cast<uint32_t>(height);
			const uint32_t fileBytes = 54u + imgBytes;

			uint8_t header[54] = {};
			header[0] = 'B';
			header[1] = 'M';
			std::memcpy(&header[2], &fileBytes, 4);
			const uint32_t pixelOffset = 54u;
			std::memcpy(&header[10], &pixelOffset, 4);
			const uint32_t dibSize = 40u;
			std::memcpy(&header[14], &dibSize, 4);
			const int32_t bmpW = width;
			const int32_t bmpH = height; // bottom-up standard DIB
			std::memcpy(&header[18], &bmpW, 4);
			std::memcpy(&header[22], &bmpH, 4);
			const uint16_t planes = 1u;
			const uint16_t bpp = 24u;
			std::memcpy(&header[26], &planes, 2);
			std::memcpy(&header[28], &bpp, 2);
			std::memcpy(&header[34], &imgBytes, 4);

			if (fwrite(header, 1, 54, fp) != 54)
			{
				fclose(fp);
				return false;
			}

			const int srcStep = (fmt == PixelFormat::A8R8G8B8) ? 4 : 3;
			std::vector<uint8_t> rowBuf(rowStride, 0u);
			for (int y = height - 1; y >= 0; --y)
			{
				const unsigned char* srcRow = data + static_cast<size_t>(y) * static_cast<size_t>(width) * srcStep;
				for (int x = 0; x < width; ++x)
				{
					rowBuf[static_cast<size_t>(x) * 3u + 0u] = srcRow[x * srcStep + 2]; // B
					rowBuf[static_cast<size_t>(x) * 3u + 1u] = srcRow[x * srcStep + 1]; // G
					rowBuf[static_cast<size_t>(x) * 3u + 2u] = srcRow[x * srcStep + 0]; // R
				}
				if (fwrite(rowBuf.data(), 1, rowStride, fp) != rowStride)
				{
					fclose(fp);
					return false;
				}
			}

			fclose(fp);
			return true;
		}
	}

	bool ParaImage::saveToFile(const std::string &filename_, bool isToRGB)
	{
		std::string filename = filename_;
		if (!CParaFile::IsAbsolutePath(filename)) {
			filename = CParaFile::GetWritablePath() + filename;
		}

		size_t dotPos = filename.find_last_of('.');
		if (dotPos == std::string::npos)
			return false;

		std::string fileExtension = filename.substr(dotPos);
		for (char& c : fileExtension)
		{
			if (c >= 'A' && c <= 'Z')
				c = static_cast<char>(c - 'A' + 'a');
		}

		if (fileExtension == ".png")
		{
			bool toRGB = isToRGB;
			if (_renderFormat == PixelFormat::A8R8G8B8)
			{
				toRGB = false; //if true will ignore the value of alpha
			}
			return saveImageToPNG(filename, toRGB);
		}
		else if (fileExtension == ".jpg" || fileExtension == ".jpeg")
		{
			return saveImageToJPG(filename);
		}
		else if (fileExtension == ".dds")
		{
			const uint32_t width = static_cast<uint32_t>(_width);
			const uint32_t height = static_cast<uint32_t>(_height);
			std::vector<uint8_t> blocks;
			if (!RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT5, _renderFormat, _data,
				_dataLen, width, height, blocks, true))
				return false;

			S3TCTexHeader header = {};
			memcpy(header.fileCode, "DDS ", 4);
			auto& desc = header.ddsd;
			desc.size = sizeof(desc);
			desc.flags = 0x81007; // CAPS | HEIGHT | WIDTH | PIXELFORMAT | LINEARSIZE
			desc.width = width;
			desc.height = height;
			desc.DUMMYUNIONNAMEN1.linearSize = static_cast<uint32_t>(blocks.size());
			auto& pixel = desc.DUMMYUNIONNAMEN4.ddpfPixelFormat;
			pixel.size = sizeof(pixel);
			pixel.flags = 4; // DDPF_FOURCC
			pixel.fourCC = makeFourCC('D', 'X', 'T', '5');
			desc.ddsCaps.caps = 0x1000; // DDSCAPS_TEXTURE

			CParaFile file;
			return file.CreateNewFile(filename.c_str()) &&
				file.write(&header, sizeof(header)) == sizeof(header) &&
				file.write(blocks.data(), static_cast<int>(blocks.size())) == static_cast<int>(blocks.size());
		}
		else if (fileExtension == ".bmp")
		{
			return SaveRawImageToBMPFile(filename, _data, static_cast<int>(_width), static_cast<int>(_height), _renderFormat);
		}

		return false;
	}
    
    bool ParaImage::saveImageToPNG(const std::string& filePath, bool isToRGB)
    {
        bool ret = false;
        do
        {
            FILE *fp;
            png_structp png_ptr;
            png_infop info_ptr;
            png_colorp palette;
            png_bytep *row_pointers;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			std::wstring filePath16 = StringHelper::MultiByteToWideChar(filePath.c_str(), DEFAULT_FILE_ENCODING);
            fp = _wfopen(filePath16.c_str(), L"wb");
#else
			fp = fopen(filePath.c_str(), "wb");
#endif
            if(nullptr == fp)
                break;
            
            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            
            if (nullptr == png_ptr)
            {
                fclose(fp);
                break;
            }
            
            info_ptr = png_create_info_struct(png_ptr);
            if (nullptr == info_ptr)
            {
                fclose(fp);
                png_destroy_write_struct(&png_ptr, nullptr);
                break;
            }
#if (CC_TARGET_PLATFORM != CC_PLATFORM_BADA && CC_TARGET_PLATFORM != CC_PLATFORM_NACL && CC_TARGET_PLATFORM != CC_PLATFORM_TIZEN)
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                fclose(fp);
                png_destroy_write_struct(&png_ptr, &info_ptr);
                break;
            }
#endif
            png_init_io(png_ptr, fp);
            
            bool hasAlpha = true;
            if (!isToRGB && hasAlpha)
            {
                png_set_IHDR(png_ptr, info_ptr, this->_width, this->_height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            }
            else
            {
                png_set_IHDR(png_ptr, info_ptr, this->_width, this->_height, 8, PNG_COLOR_TYPE_RGB,
                             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            }
            
            palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof (png_color));
            png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
            
            png_write_info(png_ptr, info_ptr);
            
            png_set_packing(png_ptr);
            
            row_pointers = (png_bytep *)malloc(_height * sizeof(png_bytep));
            if(row_pointers == nullptr)
            {
                fclose(fp);
                png_destroy_write_struct(&png_ptr, &info_ptr);
                break;
            }
            
            int _width = this->_width;
            int _height = this->_height;
            
            if (!hasAlpha)
            {
                for (int i = 0; i < (int)_height; i++)
                {
                    row_pointers[i] = (png_bytep)_data + i * _width * 3;
                }
                
                png_write_image(png_ptr, row_pointers);
                
                free(row_pointers);
                row_pointers = nullptr;
            }
            else
            {
                if (isToRGB)
                {
                    unsigned char *tempData = static_cast<unsigned char*>(malloc(_width * _height * 3 * sizeof(unsigned char)));
                    if (nullptr == tempData)
                    {
                        fclose(fp);
                        png_destroy_write_struct(&png_ptr, &info_ptr);
                        
                        free(row_pointers);
                        row_pointers = nullptr;
                        break;
                    }
                    
                    for (int i = 0; i < _height; ++i)
                    {
                        for (int j = 0; j < _width; ++j)
                        {
                            tempData[(i * _width + j) * 3] = _data[(i * _width + j) * 4];
                            tempData[(i * _width + j) * 3 + 1] = _data[(i * _width + j) * 4 + 1];
                            tempData[(i * _width + j) * 3 + 2] = _data[(i * _width + j) * 4 + 2];
                        }
                    }
                    
                    for (int i = 0; i < (int)_height; i++)
                    {
                        row_pointers[i] = (png_bytep)tempData + i * _width * 3;
                    }
                    
                    png_write_image(png_ptr, row_pointers);
                    
                    free(row_pointers);
                    row_pointers = nullptr;
                    
                    if (tempData != nullptr)
                    {
                        free(tempData);
                    }
                }
                else
                {
                    for (int i = 0; i < (int)_height; i++)
                    {
                        row_pointers[i] = (png_bytep)_data + i * _width * 4;
                    }
                    
                    png_write_image(png_ptr, row_pointers);
                    
                    free(row_pointers);
                    row_pointers = nullptr;
                }
            }
            
            png_write_end(png_ptr, info_ptr);
            
            png_free(png_ptr, palette);
            palette = nullptr;
            
            png_destroy_write_struct(&png_ptr, &info_ptr);
            
            fclose(fp);
            
            ret = true;
        } while (0);
        return ret;
    }

    bool ParaImage::saveImageToJPG(const std::string& filePath)
    {
        cout << filePath << endl;
        bool ret = false;
        do
        {
            struct jpeg_compress_struct cinfo;
            struct jpeg_error_mgr jerr;
            FILE * outfile;                 /* target file */
            JSAMPROW row_pointer[1];        /* pointer to JSAMPLE row[s] */
            int     row_stride;          /* physical row width in image buffer */
            
            cinfo.err = jpeg_std_error(&jerr);
            /* Now we can initialize the JPEG compression object. */
            jpeg_create_compress(&cinfo);
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			std::wstring filePath16 = StringHelper::MultiByteToWideChar(filePath.c_str(), DEFAULT_FILE_ENCODING);
			outfile = _wfopen(filePath16.c_str(), L"wb");
#else
			outfile = fopen(filePath.c_str(), "wb");
#endif
            if((outfile) == nullptr)
                break;
            
            jpeg_stdio_dest(&cinfo, outfile);
            
            cinfo.image_width = _width;    /* image width and height, in pixels */
            cinfo.image_height = _height;
            cinfo.input_components = 3;       /* # of color components per pixel */
            cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */
            
            jpeg_set_defaults(&cinfo);
            jpeg_set_quality(&cinfo, 90, TRUE);
            
            jpeg_start_compress(&cinfo, TRUE);
            
            row_stride = _width * 3; /* JSAMPLEs per row in image_buffer */
            
            auto format = getRenderFormat();

            if (format == PixelFormat::R8G8B8)
            {
                while (cinfo.next_scanline < cinfo.image_height) {
                    row_pointer[0] = &_data[cinfo.next_scanline * row_stride];
                    (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
                }
            }
            else if (format == PixelFormat::A8R8G8B8)
            {
                unsigned char *tempData = static_cast<unsigned char*>(malloc(_width * _height * 3 * sizeof(unsigned char)));
                if (nullptr == tempData)
                {
                    jpeg_finish_compress(&cinfo);
                    jpeg_destroy_compress(&cinfo);
                    fclose(outfile);
                    break;
                }

                for (int i = 0; i < _height; ++i)
                {
                    for (int j = 0; j < _width; ++j)

                    {
                        tempData[(i * (int)_width + j) * 3] = _data[(i * (int)_width + j) * 4];
                        tempData[(i * (int)_width + j) * 3 + 1] = _data[(i * (int)_width + j) * 4 + 1];
                        tempData[(i * (int)_width + j) * 3 + 2] = _data[(i * (int)_width + j) * 4 + 2];
                    }
                }

                while (cinfo.next_scanline < cinfo.image_height)
                {
                    row_pointer[0] = &tempData[cinfo.next_scanline * row_stride];
                    (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
                }

                if (tempData != nullptr)
                {
                    free(tempData);
                }
            }

            jpeg_finish_compress(&cinfo);
            fclose(outfile);
            jpeg_destroy_compress(&cinfo);
            
            ret = true;
        } while (0);
        return ret;
    }

	void ParaImage::premultipliedAlpha()
	{
#if CC_ENABLE_PREMULTIPLIED_ALPHA == 0
		_hasPremultipliedAlpha = false;
		return;
#else
		// The pixel format should be RGBA8888!
		PE_ASSERT(_renderFormat == PixelFormat::A8R8G8B8);

		unsigned int* fourBytes = (unsigned int*)_data;
		for (int i = 0; i < _width * _height; i++)
		{
			unsigned char* p = _data + i * 4;
			fourBytes[i] = CC_RGB_PREMULTIPLY_ALPHA(p[0], p[1], p[2], p[3]);
		}

		_hasPremultipliedAlpha = true;
#endif
	}


	PixelFormat ParaImage::convertI8ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen)
	{
		switch (format)
		{
		case PixelFormat::A8R8G8B8:
			*outDataLen = dataLen * 4;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertI8ToRGBA8888(data, dataLen, *outData);
			break;
		case PixelFormat::R8G8B8:
			*outDataLen = dataLen * 3;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertI8ToRGB888(data, dataLen, *outData);
			break;
		case PixelFormat::R5G6B5:
			*outDataLen = dataLen * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertI8ToRGB565(data, dataLen, *outData);
			break;
		case PixelFormat::A8L8:
			*outDataLen = dataLen * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertI8ToAI88(data, dataLen, *outData);
			break;
		case PixelFormat::A4R4G4B4:
			*outDataLen = dataLen * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertI8ToRGBA4444(data, dataLen, *outData);
			break;
		case PixelFormat::A1R5G5B5:
			*outDataLen = dataLen * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertI8ToRGB5A1(data, dataLen, *outData);
			break;
		default:
			// unsupported conversion or don't need to convert
			if (format != PixelFormat::Unknown && format != PixelFormat::L8)
			{
				OUTPUT_LOG("Can not convert image format PixelFormat::I8 to format ID:%d, we will use it's origin format PixelFormat::I8", static_cast<int>(format));
			}

			*outData = (unsigned char*)data;
			*outDataLen = dataLen;
			return PixelFormat::L8;
		}

		return format;
	}

	PixelFormat ParaImage::convertAI88ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen)
	{
		switch (format)
		{
		case PixelFormat::A8R8G8B8:
			*outDataLen = dataLen * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToRGBA8888(data, dataLen, *outData);
			break;
		case PixelFormat::R8G8B8:
			*outDataLen = dataLen / 2 * 3;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToRGB888(data, dataLen, *outData);
			break;
		case PixelFormat::R5G6B5:
			*outDataLen = dataLen;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToRGB565(data, dataLen, *outData);
			break;
		case PixelFormat::A8:
			*outDataLen = dataLen / 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToA8(data, dataLen, *outData);
			break;
		case PixelFormat::L8:
			*outDataLen = dataLen / 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToI8(data, dataLen, *outData);
			break;
		case PixelFormat::A4R4G4B4:
			*outDataLen = dataLen;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToRGBA4444(data, dataLen, *outData);
			break;
		case PixelFormat::A1R5G5B5:
			*outDataLen = dataLen;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertAI88ToRGB5A1(data, dataLen, *outData);
			break;
		default:
			// unsupported conversion or don't need to convert
			if (format != PixelFormat::Unknown && format != PixelFormat::A8L8)
			{
				OUTPUT_LOG("Can not convert image format PixelFormat::AI88 to format ID:%d, we will use it's origin format PixelFormat::AI88", static_cast<int>(format));
			}

			*outData = (unsigned char*)data;
			*outDataLen = dataLen;
			return PixelFormat::A8L8;
			break;
		}

		return format;
	}

	PixelFormat ParaImage::convertRGB888ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen)
	{
		switch (format)
		{
		case PixelFormat::A8R8G8B8:
			*outDataLen = dataLen / 3 * 4;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToRGBA8888(data, dataLen, *outData);
			break;
		case PixelFormat::R5G6B5:
			*outDataLen = dataLen / 3 * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToRGB565(data, dataLen, *outData);
			break;
		case PixelFormat::A8:
			*outDataLen = dataLen / 3;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToA8(data, dataLen, *outData);
			break;
		case PixelFormat::L8:
			*outDataLen = dataLen / 3;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToI8(data, dataLen, *outData);
			break;
		case PixelFormat::A8L8:
			*outDataLen = dataLen / 3 * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToAI88(data, dataLen, *outData);
			break;
		case PixelFormat::A4R4G4B4:
			*outDataLen = dataLen / 3 * 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToRGBA4444(data, dataLen, *outData);
			break;
		case PixelFormat::A1R5G5B5:
			*outDataLen = dataLen;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGB888ToRGB5A1(data, dataLen, *outData);
			break;
		default:
			// unsupported conversion or don't need to convert
			if (format != PixelFormat::Unknown && format != PixelFormat::R8G8B8)
			{
				OUTPUT_LOG("Can not convert image format PixelFormat::RGB888 to format ID:%d, we will use it's origin format PixelFormat::RGB888", static_cast<int>(format));
			}

			*outData = (unsigned char*)data;
			*outDataLen = dataLen;
			return PixelFormat::R8G8B8;
		}
		return format;
	}

	PixelFormat ParaImage::convertRGBA8888ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen)
	{

		switch (format)
		{
		case PixelFormat::R8G8B8:
			*outDataLen = dataLen / 4 * 3;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToRGB888(data, dataLen, *outData);
			break;
		case PixelFormat::R5G6B5:
			*outDataLen = dataLen / 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToRGB565(data, dataLen, *outData);
			break;
		case PixelFormat::A8:
			*outDataLen = dataLen / 4;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToA8(data, dataLen, *outData);
			break;
		case PixelFormat::L8:
			*outDataLen = dataLen / 4;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToI8(data, dataLen, *outData);
			break;
		case PixelFormat::A8L8:
			*outDataLen = dataLen / 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToAI88(data, dataLen, *outData);
			break;
		case PixelFormat::A4R4G4B4:
			*outDataLen = dataLen / 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToRGBA4444(data, dataLen, *outData);
			break;
		case PixelFormat::A1R5G5B5:
			*outDataLen = dataLen / 2;
			*outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
			convertRGBA8888ToRGB5A1(data, dataLen, *outData);
			break;
		default:
			// unsupported conversion or don't need to convert
			if (format != PixelFormat::Unknown && format != PixelFormat::A8R8G8B8)
			{
				OUTPUT_LOG("Can not convert image format PixelFormat::RGBA8888 to format ID:%d, we will use it's origin format PixelFormat::RGBA8888", static_cast<int>(format));
			}

			*outData = (unsigned char*)data;
			*outDataLen = dataLen;
			return PixelFormat::A8R8G8B8;
		}

		return format;
	}


	// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
	void ParaImage::convertI8ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0; i < dataLen; ++i)
		{
			*outData++ = data[i];     //R
			*outData++ = data[i];     //G
			*outData++ = data[i];     //B
		}
	}

	// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
	void ParaImage::convertAI88ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
		{
			*outData++ = data[i];     //R
			*outData++ = data[i];     //G
			*outData++ = data[i];     //B
		}
	}

	// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
	void ParaImage::convertI8ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertAI88ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertI8ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertAI88ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertI8ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertAI88ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertI8ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertAI88ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertI8ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		unsigned short* out16 = (unsigned short*)outData;
		for (size_t i = 0; i < dataLen; ++i)
		{
			*out16++ = 0xFF00     //A
				| data[i];            //I
		}
	}

	// IIIIIIIIAAAAAAAA -> AAAAAAAA
	void ParaImage::convertAI88ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 1; i < dataLen; i += 2)
		{
			*outData++ = data[i]; //A
		}
	}

	// IIIIIIIIAAAAAAAA -> IIIIIIII
	void ParaImage::convertAI88ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 1; i < l; i += 2)
		{
			*outData++ = data[i]; //R
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
	void ParaImage::convertRGB888ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertRGBA8888ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
		{
			*outData++ = data[i];         //R
			*outData++ = data[i + 1];     //G
			*outData++ = data[i + 2];     //B
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
	void ParaImage::convertRGB888ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertRGBA8888ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertRGB888ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
		{
			*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //A =  (R*299 + G*587 + B*114 + 500) / 1000
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
	void ParaImage::convertRGB888ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
		{
			*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
	void ParaImage::convertRGBA8888ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
		{
			*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
	void ParaImage::convertRGBA8888ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
		{
			*outData++ = data[i + 3]; //A
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
	void ParaImage::convertRGB888ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 2; i < l; i += 3)
		{
			*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
			*outData++ = 0xFF;
		}
	}


	// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
	void ParaImage::convertRGBA8888ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData)
	{
		for (size_t i = 0, l = dataLen - 3; i < l; i += 4)
		{
			*outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
			*outData++ = data[i + 3];
		}
	}

	// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
	void ParaImage::convertRGB888ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertRGBA8888ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertRGB888ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
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
	void ParaImage::convertRGBA8888ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData)
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

	PixelFormat ParaImage::convertDataToFormat(const unsigned char* data, size_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, size_t* outDataLen)
	{
		// don't need to convert
		if (format == originFormat || format == PixelFormat::Unknown)
		{
			*outData = (unsigned char*)data;
			*outDataLen = dataLen;
			return originFormat;
		}

		switch (originFormat)
		{
		case PixelFormat::L8:
			return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
		case PixelFormat::A8L8:
			return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
		case PixelFormat::R8G8B8:
			return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
		case PixelFormat::A8R8G8B8:
			return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
		default:
			OUTPUT_LOG("unsupported conversion from format %d to format %d", static_cast<int>(originFormat), static_cast<int>(format));
			*outData = (unsigned char*)data;
			*outDataLen = dataLen;
			return originFormat;
		}
	}
}


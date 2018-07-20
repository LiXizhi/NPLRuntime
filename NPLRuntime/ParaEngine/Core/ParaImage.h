#pragma once


namespace ParaEngine
{
	typedef struct _MipmapInfo
	{
		unsigned char* address;
		int len;
		_MipmapInfo() :address(nullptr), len(0) {}
	}MipmapInfo;

	class ParaImage : public CRefCountedOne
	{
	public:

		ParaImage();
		virtual ~ParaImage();

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
		bool initWithImageData(const unsigned char * data, size_t dataLen);
		bool initWithRawData(const unsigned char * data, size_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
		bool saveToFile(const std::string &filename, bool isToRGB = true);

		float               getWidth() const { return _width; }
		float               getHeight() const { return _height; }

		PixelFormat getRenderFormat() const{ return _renderFormat; }

		int getNumberOfMipmaps() const { return _numberOfMipmaps; }

		int getBitsPrePixel() const;

		const char* getFilePath() const { return ""; }
		unsigned char* getData() const { return _data; }
		int	getDataLen() const { return _dataLen; }
		const MipmapInfo* getMipmaps() const { return _mipmaps; }
		bool	isCompressed() const { return false; }


		static PixelFormat convertDataToFormat(const unsigned char* data, size_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, size_t* outDataLen);

		static PixelFormat convertI8ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen);
		static PixelFormat convertAI88ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen);
		static PixelFormat convertRGB888ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen);
		static PixelFormat convertRGBA8888ToFormat(const unsigned char* data, size_t dataLen, PixelFormat format, unsigned char** outData, size_t* outDataLen);

		//I8 to XXX
		static void convertI8ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertI8ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertI8ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertI8ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertI8ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertI8ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData);

		//AI88 to XXX
		static void convertAI88ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertAI88ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertAI88ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertAI88ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertAI88ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertAI88ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertAI88ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData);

		//RGB888 to XXX
		static void convertRGB888ToRGBA8888(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGB888ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGB888ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGB888ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGB888ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGB888ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGB888ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData);

		//RGBA8888 to XXX
		static void convertRGBA8888ToRGB888(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGBA8888ToRGB565(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGBA8888ToI8(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGBA8888ToA8(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGBA8888ToAI88(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGBA8888ToRGBA4444(const unsigned char* data, size_t dataLen, unsigned char* outData);
		static void convertRGBA8888ToRGB5A1(const unsigned char* data, size_t dataLen, unsigned char* outData);

	protected:
		static const int MIPMAP_MAX = 16;

		float _width;
		float _height;

		bool _hasPremultipliedAlpha;
		int _dataLen;
		unsigned char* _data;

		PixelFormat _renderFormat;

		Format _fileType;

		int _numberOfMipmaps;
		MipmapInfo _mipmaps[MIPMAP_MAX];   // pointer to mipmap images

		bool isPng(const unsigned char * data, size_t dataLen);
		bool isEtc(const unsigned char * data, size_t dataLen);
		bool isS3TC(const unsigned char * data, size_t dataLen);
		bool isATITC(const unsigned char *data, size_t dataLen);
		bool isJpg(const unsigned char * data, size_t dataLen);
		bool isTiff(const unsigned char * data, size_t dataLen);
		bool isWebp(const unsigned char * data, size_t dataLen);
		bool isPvr(const unsigned char * data, size_t dataLen);
		Format detectFormat(const unsigned char *, size_t unpackedLen);

		bool initWithJpgData(const unsigned char* unpackedData, size_t unpackedLen);
		bool initWithPngData(const unsigned char* unpackedData, size_t unpackedLen);
		bool initWithS3TCData(const unsigned char* unpackedData, size_t unpackedLen);


		void premultipliedAlpha();
	};
}
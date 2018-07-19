#pragma once
#include "GLType.h"
#include <map>

namespace ParaEngine
{
	class GLImage;
	class GLProgram;
	typedef struct _MipmapInfo MipmapInfo;


	class GLTexture2D : public CRefCountedOne
	{
	public:

		struct PixelFormatInfo {

			PixelFormatInfo(GLenum anInternalFormat, GLenum aFormat, GLenum aType, int aBpp, bool aCompressed, bool anAlpha)
				: internalFormat(anInternalFormat)
				, format(aFormat)
				, type(aType)
				, bpp(aBpp)
				, compressed(aCompressed)
				, alpha(anAlpha)
			{}

			GLenum internalFormat;
			GLenum format;
			GLenum type;
			int bpp;
			bool compressed;
			bool alpha;
		};

		typedef std::map<PixelFormat, const PixelFormatInfo> PixelFormatInfoMap;


		typedef struct _TexParams {
			GLuint    minFilter;
			GLuint    magFilter;
			GLuint    wrapS;
			GLuint    wrapT;
		}TexParams;

		GLTexture2D();
		virtual ~GLTexture2D();

		bool initWithImage(GLImage * image);
		bool initWithImage(GLImage * image, PixelFormat format);



		int getNumberOfMipmaps();


		/** Gets the width of the texture in pixels. */
		int getPixelsWide() const;

		/** Gets the height of the texture in pixels. */
		int getPixelsHigh() const;


		bool initWithData(const void *data, size_t dataLen, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize);

		bool updateWithData(const void *data, int offsetX, int offsetY, int width, int height);

		GLuint getName() const;
		void setTexParameters(const TexParams& texParams);
		void setAliasTexParameters();
		void setAntiAliasTexParameters();

		static const PixelFormatInfoMap& getPixelFormatInfoMap();


		void bind();
		void bindN(GLuint textureUnit);

		void RendererRecreated();

	private:
		static PixelFormat g_defaultAlphaPixelFormat;

		/** pixel format of the texture */
		PixelFormat _pixelFormat;

		/** width in pixels */
		int _pixelsWide;

		/** height in pixels */
		int _pixelsHigh;

		/** texture name */
		GLuint _name;

		/** texture max S */
		GLfloat _maxS;

		/** texture max T */
		GLfloat _maxT;

		/** content size */
		Size _contentSize;

		/** whether or not the texture has their Alpha premultiplied */
		bool _hasPremultipliedAlpha;

		/** whether or not the texture has mip maps*/
		bool _hasMipmaps;

		/** shader program used by drawAtPoint and drawInRect */
		GLProgram* _shaderProgram;

		static const PixelFormatInfoMap _pixelFormatInfoTables;

		bool _antialiasEnabled;

		bool _valid;
		std::string _filePath;

		GLTexture2D* _alphaTexture;



		bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

		void setGLProgram(GLProgram* shaderProgram);

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
	};
}
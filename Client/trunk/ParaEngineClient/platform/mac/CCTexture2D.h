



#ifndef __CC_TEXTURE2d_H__
#define __CC_TEXTURE2d_H__



#include "CCType.h"

#include <map>



NS_CC_BEGIN

class Image;

class GLProgram;
typedef struct _MipmapInfo MipmapInfo;



class Texture2D : public Ref
{
public:
	enum class PixelFormat
	    {
	        //! auto detect the type
	        AUTO,
	        //! 32-bit texture: BGRA8888
	        BGRA8888,
	        //! 32-bit texture: RGBA8888
	        RGBA8888,
	        //! 24-bit texture: RGBA888
	        RGB888,
	        //! 16-bit texture without Alpha channel
	        RGB565,
	        //! 8-bit textures used as masks
	        A8,
	        //! 8-bit intensity texture
	        I8,
	        //! 16-bit textures used as masks
	        AI88,
	        //! 16-bit textures: RGBA4444
	        RGBA4444,
	        //! 16-bit textures: RGB5A1
	        RGB5A1,
	        //! 4-bit PVRTC-compressed texture: PVRTC4
	        PVRTC4,
	        //! 4-bit PVRTC-compressed texture: PVRTC4 (has alpha channel)
	        PVRTC4A,
	        //! 2-bit PVRTC-compressed texture: PVRTC2
	        PVRTC2,
	        //! 2-bit PVRTC-compressed texture: PVRTC2 (has alpha channel)
	        PVRTC2A,
	        //! ETC-compressed texture: ETC
	        ETC,
	        //! S3TC-compressed texture: S3TC_Dxt1
	        S3TC_DXT1,
	        //! S3TC-compressed texture: S3TC_Dxt3
	        S3TC_DXT3,
	        //! S3TC-compressed texture: S3TC_Dxt5
	        S3TC_DXT5,
	        //! ATITC-compressed texture: ATC_RGB
	        ATC_RGB,
	        //! ATITC-compressed texture: ATC_EXPLICIT_ALPHA
	        ATC_EXPLICIT_ALPHA,
	        //! ATITC-compressed texture: ATC_INTERPOLATED_ALPHA
	        ATC_INTERPOLATED_ALPHA,
	        //! Default texture format: AUTO
	        DEFAULT = AUTO,

	        NONE = -1
	    };


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

	typedef std::map<Texture2D::PixelFormat, const PixelFormatInfo> PixelFormatInfoMap;


	typedef struct _TexParams {
	        GLuint    minFilter;
	        GLuint    magFilter;
	        GLuint    wrapS;
	        GLuint    wrapT;
	    }TexParams;

	Texture2D();
	virtual ~Texture2D();

	bool initWithImage(Image * image);
	bool initWithImage(Image * image, PixelFormat format);



	int getNumberOfMipmaps();


	 /** Gets the width of the texture in pixels. */
    int getPixelsWide() const;

    /** Gets the height of the texture in pixels. */
    int getPixelsHigh() const;


	bool initWithData(const void *data, ssize_t dataLen, Texture2D::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize);

	bool updateWithData(const void *data, int offsetX, int offsetY, int width, int height);

	GLuint getName() const;
	void setTexParameters(const TexParams& texParams);
	void setAliasTexParameters();
	void setAntiAliasTexParameters();

	static const PixelFormatInfoMap& getPixelFormatInfoMap();

private:
	static Texture2D::PixelFormat g_defaultAlphaPixelFormat;

	/** pixel format of the texture */
	Texture2D::PixelFormat _pixelFormat;

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

	Texture2D* _alphaTexture;



	bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

	void setGLProgram(GLProgram* shaderProgram);

	static Texture2D::PixelFormat convertDataToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, ssize_t* outDataLen);

	static PixelFormat convertI8ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
	static PixelFormat convertAI88ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
	static PixelFormat convertRGB888ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen);
	static PixelFormat convertRGBA8888ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen);

	//I8 to XXX
	static void convertI8ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertI8ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertI8ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertI8ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertI8ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertI8ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

	//AI88 to XXX
	static void convertAI88ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertAI88ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertAI88ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertAI88ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertAI88ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertAI88ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertAI88ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

	//RGB888 to XXX
	static void convertRGB888ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGB888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGB888ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGB888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGB888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGB888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGB888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

	//RGBA8888 to XXX
	static void convertRGBA8888ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGBA8888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGBA8888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGBA8888ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGBA8888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGBA8888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData);
	static void convertRGBA8888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData);

};

NS_CC_END

#endif

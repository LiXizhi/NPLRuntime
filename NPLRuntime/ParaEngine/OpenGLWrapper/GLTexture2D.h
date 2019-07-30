#pragma once
#include "GLType.h"
#include "Framework/RenderSystem/RenderTypes.h"
#include <map>

namespace ParaEngine
{
	class GLImage;
	class ParaImage;
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

		bool initWithImage(const ParaImage * image);
		bool initWithImage(const ParaImage * image, PixelFormat format);



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

		static int ccNextPOT(int x);


		static void bind(GLTexture2D* pTex);
		static void bindN(GLTexture2D* pTex, GLuint textureUnit);
		static GLuint getName(const GLTexture2D* pTex);
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



		bool initWithMipmaps(const MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

		void setGLProgram(GLProgram* shaderProgram);

		
	};
}
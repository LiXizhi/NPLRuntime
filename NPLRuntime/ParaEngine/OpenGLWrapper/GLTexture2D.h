#pragma once
#include "GLType.h"
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Interface/Render/RHITypes.h"
#include <map>

namespace ParaEngine
{
	class GLImage;
	class ParaImage;
	class GLProgram;
	typedef struct _MipmapInfo MipmapInfo;


	class GLTexture2D : public CRefCountedOne, public IRHITexture
	{
	public:
		// IRHITexture implementation
		virtual uint32_t GetWidth() const override { return static_cast<uint32_t>(_pixelsWide); }
		virtual uint32_t GetHeight() const override { return static_cast<uint32_t>(_pixelsHigh); }
		virtual uint32_t GetMipLevels() const override { return _rhiTexture ? _rhiTexture->GetMipLevels() : (_hasMipmaps ? 2u : 1u); }
		virtual PixelFormat GetFormat() const override { return _pixelFormat; }
		virtual bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t rowPitch = 0) override
		{
			if (_rhiTexture)
				return _rhiTexture->UpdateSubresource(mipLevel, arraySlice, x, y, width, height, data, rowPitch);
			if (mipLevel == 0 && arraySlice == 0)
				return updateWithData(data, x, y, static_cast<int>(width), static_cast<int>(height));
			return false;
		}
		virtual void* GetNativeHandle() const override
		{
			return _rhiTexture ? _rhiTexture->GetNativeHandle() : reinterpret_cast<void*>(static_cast<uintptr_t>(_name));
		}
		virtual void ReleaseRHIResource() override { Release(); }
		IRHITexture* getRHITexture() const { return _rhiTexture; }

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

		bool initWithRHITexture(IRHITexture* rhiTex, bool takeOwnership = false);
		void detachRHITexture();

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
		IRHITexture* _rhiTexture;
		bool _ownsRHITexture;



		bool initWithMipmaps(const MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

		void setGLProgram(GLProgram* shaderProgram);

		
	};
}
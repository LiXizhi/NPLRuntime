#pragma once
#include "Framework/Interface/Render/ITexture.h"
#include "Framework/Common/Image.hpp"
#include "OpenGL.h"
namespace ParaEngine
{
	class Image;
	class TextureOpenGL : public IParaEngine::ITexture
	{
	public:
		TextureOpenGL();
		virtual ~TextureOpenGL() override;
		virtual bool CopyTo(ITexture* target) override;
		virtual bool StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ParaEngine::ETextureFilter filter) override;
		virtual uint32_t GetWidth() const override;
		virtual uint32_t GetHeight() const override;
		GLuint GetTextureID() const;

		virtual bool UpdateImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels) override;
		virtual ParaEngine::ImagePtr GetImage(uint32_t level) override;
		virtual ParaEngine::ETextureFilter GetMinFilter() const override;
		virtual ParaEngine::ETextureFilter GetMagFilter() const override;
		virtual bool SetMinFilter(ParaEngine::ETextureFilter type) override;
		virtual bool SetMagFilter(ParaEngine::ETextureFilter type) override;
		virtual ParaEngine::ETextureWrapMode GetAddressU() const override;
		virtual ParaEngine::ETextureWrapMode GetAddressV() const override;
		virtual bool SetAddressU(ParaEngine::ETextureWrapMode mode) override;
		virtual bool SetAddressV(ParaEngine::ETextureWrapMode mode) override;
	public:
		static TextureOpenGL* Create(uint32_t width,uint32_t height,EPixelFormat format,ETextureUsage usage);
		static TextureOpenGL* CreateWithImage(ImagePtr image);

	protected:
		virtual void OnRelease() override;
		bool UpdateImageUncomressed(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels);
		bool UpdateImageComressed(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels);

		static TextureOpenGL* CreateUnCompressedTextureWithImage(ImagePtr image);
		static TextureOpenGL* CreateComressedTextureWithImage(ImagePtr image);

	private:
		GLuint m_TextureID;
		uint32_t m_Width;
		uint32_t m_Height;
		EPixelFormat m_Format;
		GLenum m_GLFormat;
		GLenum m_GLDataType;
		GLenum m_GLPixelFomat;
		ETextureFilter m_MagFilter;
		ETextureFilter m_MinFilter;
		ETextureWrapMode m_AddressU;
		ETextureWrapMode m_AddressV;
		ETextureUsage m_Usage;
	};
}
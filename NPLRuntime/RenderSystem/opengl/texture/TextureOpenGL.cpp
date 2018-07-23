#include "TextureOpenGL.h"
using namespace ParaEngine;
using namespace IParaEngine;

ParaEngine::TextureOpenGL::TextureOpenGL()
	:m_TextureID(0)
	, m_Width(0)
	, m_Height(0)
	, m_Format(EPixelFormat::Unkonwn)
	, m_GLFormat(0)
	, m_MagFilter(ETextureFilter::Point)
	, m_MinFilter(ETextureFilter::Point)
	, m_AddressU(ETextureWrapMode::Clamp)
	, m_AddressV(ETextureWrapMode::Clamp)
	, m_Usage(ETextureUsage::Default)
{

}

ParaEngine::TextureOpenGL::~TextureOpenGL()
{

}


bool ParaEngine::TextureOpenGL::CopyTo(ITexture* target)
{
	return false;
}

bool ParaEngine::TextureOpenGL::StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ParaEngine::ETextureFilter filter)
{
	return false;
}

uint32_t ParaEngine::TextureOpenGL::GetWidth() const
{
	return m_Width;
}

uint32_t ParaEngine::TextureOpenGL::GetHeight() const
{
	return m_Height;
}

GLuint ParaEngine::TextureOpenGL::GetTextureID() const
{
	return m_TextureID;
}

bool ParaEngine::TextureOpenGL::UpdateImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height, m_GLFormat, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return true;
}

ParaEngine::ImagePtr ParaEngine::TextureOpenGL::GetImage(uint32_t level)
{
	uint32_t pitch = 0;
	Image::EImagePixelFormat format = Image::IPF_A8;

	GLint width = 0;
	GLint height = 0;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &height);
	switch (m_GLFormat)
	{
	case GL_RGB:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_R8G8B8;
	}
	break;
	case GL_RGBA:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_R8G8B8A8;
	}
	break;
	case GL_BGRA:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_B8G8R8A8;
	}
	break;
	case GL_ALPHA:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_A8;

	}
	break;
	case GL_LUMINANCE:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_L8;

	}
	break;
	case GL_LUMINANCE_ALPHA:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_A8L8;

	}
	break;
	default:
		break;
	}

	size_t buffer_size = pitch * height;
	unsigned char* buffer = new unsigned char[buffer_size];
	memset(buffer, 0, buffer_size);
	
	
	glGetTexImage(GL_TEXTURE_2D, level, m_GLFormat, GL_UNSIGNED_BYTE,buffer);
	glBindTexture(GL_TEXTURE_2D,0);

	auto img = std::make_shared<Image>();
	img->width = width;
	img->height = height;
	img->data = buffer;
	img->data_size = buffer_size;
	img->Format = format;
	return img;
}

ParaEngine::ETextureFilter ParaEngine::TextureOpenGL::GetMinFilter() const
{
	return m_MinFilter;
}

ParaEngine::ETextureFilter ParaEngine::TextureOpenGL::GetMagFilter() const
{
	return m_MagFilter;
}


bool ParaEngine::TextureOpenGL::SetMinFilter(ParaEngine::ETextureFilter type)
{
	if (type == m_MinFilter) return true;
	m_MinFilter = type;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	switch (m_MinFilter)
	{
	case ParaEngine::ETextureFilter::Point:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case ParaEngine::ETextureFilter::Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	default:
		break;
	}
	glBindTexture(GL_TEXTURE_2D,0);
	return true;
}


bool ParaEngine::TextureOpenGL::SetMagFilter(ParaEngine::ETextureFilter type)
{
	if (type == m_MagFilter) return true;
	m_MagFilter = type;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	switch (m_MagFilter)
	{
	case ParaEngine::ETextureFilter::Point:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case ParaEngine::ETextureFilter::Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

ParaEngine::ETextureWrapMode ParaEngine::TextureOpenGL::GetAddressU() const
{
	return m_AddressU;
}

ParaEngine::ETextureWrapMode ParaEngine::TextureOpenGL::GetAddressV() const
{
	return m_AddressV;
}


bool ParaEngine::TextureOpenGL::SetAddressU(ParaEngine::ETextureWrapMode mode)
{
	if (mode == m_AddressU) return true;
	m_AddressU = mode;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	switch (m_AddressU)
	{
	case ParaEngine::ETextureWrapMode::Clamp:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		break;
	case ParaEngine::ETextureWrapMode::Repeat:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		break;
	default:
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool ParaEngine::TextureOpenGL::SetAddressV(ParaEngine::ETextureWrapMode mode)
{
	if (mode == m_AddressV) return true;
	m_AddressV = mode;
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	switch (m_AddressV)
	{
	case ParaEngine::ETextureWrapMode::Clamp:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case ParaEngine::ETextureWrapMode::Repeat:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	default:
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}


TextureOpenGL* TextureOpenGL::Create(uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage)
{
	

	GLenum glFormat = 0;
	switch (format)
	{
	case EPixelFormat::R8G8B8:
		glFormat = GL_RGB;
		break;
	case EPixelFormat::A8R8G8B8:
		glFormat = GL_RGBA;
		break;
	case EPixelFormat::A8B8G8R8:
		glFormat = GL_BGRA;
		break;
	case EPixelFormat::A8:
		glFormat = GL_ALPHA;
		break;
	case EPixelFormat::L8:
		glFormat = GL_LUMINANCE;
		break;
	case EPixelFormat::A8L8:
		glFormat = GL_LUMINANCE_ALPHA;
	case EPixelFormat::D24S8:
		glFormat = GL_DEPTH24_STENCIL8;
		break;
	default:
		break;
	}
	if (glFormat == 0) return nullptr;

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, usage == ETextureUsage::DepthStencil ? GL_DEPTH_STENCIL : glFormat, usage == ETextureUsage::DepthStencil ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_BYTE,nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindTexture(GL_TEXTURE_2D, 0);

	
	TextureOpenGL* tex = new TextureOpenGL();

	tex->m_TextureID = textureID;
	tex->m_Width = width;
	tex->m_Height = height;
	tex->m_GLFormat = glFormat;
	tex->m_Usage = usage;
	tex->m_Format = format;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return tex;
}

ParaEngine::TextureOpenGL * ParaEngine::TextureOpenGL::CreateWithImage(ImagePtr image)
{
	if (image == nullptr) return nullptr;

	TextureOpenGL* tex = new TextureOpenGL();
	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	GLenum glformat = GL_RGB;
	EPixelFormat format = EPixelFormat::Unkonwn;
	switch (image->Format)
	{
	case  Image::IPF_L8:
		{
			glformat = GL_LUMINANCE;
			format = EPixelFormat::L8;
		}
		break;
	case  Image::IPF_A8:
		{
			glformat = GL_ALPHA;
			format = EPixelFormat::A8;
		}
		break;
	case  Image::IPF_A8L8:
		{
			glformat = GL_LUMINANCE_ALPHA;
			format = EPixelFormat::A8L8;
		}
		break;
	case  Image::IPF_R8G8B8:
		{
			glformat = GL_RGB;
			format = EPixelFormat::R8G8B8;
		}
		break;
	case  Image::IPF_R8G8B8A8:
		{
			glformat = GL_RGBA;
			format = EPixelFormat::A8R8G8B8;
		}
		break;

	default:
		break;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, glformat, image->width, image->height,0, glformat, GL_UNSIGNED_BYTE, image->data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindTexture(GL_TEXTURE_2D, 0);

	tex->m_TextureID = textureID;
	tex->m_Width = image->width;
	tex->m_Height = image->height;
	tex->m_Format = format;
	tex->m_GLFormat = glformat;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return tex;
}

void ParaEngine::TextureOpenGL::OnRelease()
{
	glDeleteTextures(1, &m_TextureID);
	m_TextureID = 0;
	m_Width = 0;
	m_Height = 0;
	m_Format = EPixelFormat::Unkonwn;
}

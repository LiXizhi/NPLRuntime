#include "TextureOpenGL.h"
using namespace ParaEngine;
using namespace IParaEngine;


bool IsComressedFormat(EPixelFormat format)
{

	if (format == EPixelFormat::DXT1 || format == EPixelFormat::DXT3 || format == EPixelFormat::DXT5)
	{
		return true;
	}
	else {
		return false;
	}
}


ParaEngine::TextureOpenGL::TextureOpenGL()
	:m_TextureID(0)
	, m_Width(0)
	, m_Height(0)
	, m_Format(EPixelFormat::Unkonwn)
	, m_GLFormat(0)
	, m_GLDataType(0)
	, m_GLPixelFomat(0)
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
	if (IsComressedFormat(m_Format))
	{
		return UpdateImageComressed(level, xoffset, yoffset, width, height, pixels);
	}
	else {
		return UpdateImageUncomressed(level, xoffset, yoffset, width, height, pixels);
	}
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
		pitch = 4 * sizeof(unsigned char) * width;
		format = Image::IPF_R8G8B8A8;
	}
	break;
	case GL_BGRA:
	{
		pitch = 4 * sizeof(unsigned char) * width;
		format = Image::IPF_B8G8R8A8;
	}
	break;
	case GL_ALPHA:
	{
		pitch = 1 * sizeof(unsigned char) * width;
		format = Image::IPF_A8;

	}
	break;
	case GL_LUMINANCE:
	{
		pitch = 1 * sizeof(unsigned char) * width;
		format = Image::IPF_L8;

	}
	break;
	case GL_LUMINANCE_ALPHA:
	{
		pitch = 2 * sizeof(unsigned char) * width;
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
	ImageMipmap mipmap;
	mipmap.width = width;
	mipmap.height = height;
	mipmap.offset = 0;
	mipmap.size = buffer_size;
	img->data = buffer;
	img->data_size = buffer_size;
	img->Format = format;
	img->mipmaps.push_back(mipmap);
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
	GLenum glDataType = 0;
	GLenum glPixelFormat = 0;
	switch (format)
	{
	case EPixelFormat::R8G8B8:
		glFormat = GL_RGB;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RGB;
		break;
	case EPixelFormat::A8R8G8B8:
		glFormat = GL_RGBA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RGBA;
		break;
	case EPixelFormat::A8B8G8R8:
		glFormat = GL_BGRA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_BGRA;
		break;
	case EPixelFormat::A8:
		glFormat = GL_ALPHA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_ALPHA;
		break;
	case EPixelFormat::L8:
		glFormat = GL_LUMINANCE;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RED;
		break;
	case EPixelFormat::A8L8:
		glFormat = GL_LUMINANCE_ALPHA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RG;
	case EPixelFormat::D24S8:
		glFormat = GL_DEPTH24_STENCIL8;
		glDataType = GL_UNSIGNED_INT_24_8;
		glPixelFormat = GL_DEPTH_STENCIL;
		break;
	default:
		return nullptr;
		break;
	}
	if (glFormat == 0) return nullptr;

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glPixelFormat, glDataType, nullptr);

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

	if (image->Format == Image::IPF_COMPRESSED_DXT1 ||
		image->Format == Image::IPF_COMPRESSED_DXT3 ||
		image->Format == Image::IPF_COMPRESSED_DXT5)
	{
		return CreateComressedTextureWithImage(image);
	}
	else {
		return CreateUnCompressedTextureWithImage(image);
	}

}

void ParaEngine::TextureOpenGL::OnRelease()
{
	glDeleteTextures(1, &m_TextureID);
	m_TextureID = 0;
	m_Width = 0;
	m_Height = 0;
	m_Format = EPixelFormat::Unkonwn;
}


ParaEngine::TextureOpenGL* ParaEngine::TextureOpenGL::CreateUnCompressedTextureWithImage(ImagePtr image)
{
	EPixelFormat format = EPixelFormat::Unkonwn;
	switch (image->Format)
	{
	case  Image::IPF_L8:
	{
		format = EPixelFormat::L8;
	}
	break;
	case  Image::IPF_A8:
	{
		format = EPixelFormat::A8;
	}
	break;
	case  Image::IPF_A8L8:
	{
		format = EPixelFormat::A8L8;
	}
	break;
	case  Image::IPF_R8G8B8:
	{
		format = EPixelFormat::R8G8B8;
	}
	break;
	case  Image::IPF_R8G8B8A8:
	{
		format = EPixelFormat::A8R8G8B8;
	}
	break;
	case Image::IPF_COMPRESSED_DXT1:
	{
		format = EPixelFormat::DXT1;
	}
	break;
	case Image::IPF_COMPRESSED_DXT3:
	{
		format = EPixelFormat::DXT3;
	}
	break;
	case Image::IPF_COMPRESSED_DXT5:
	{
		format = EPixelFormat::DXT5;
	}
	break;
	default:
		break;
	}

	TextureOpenGL* tex = TextureOpenGL::Create(image->mipmaps[0].width, image->mipmaps[0].height, format, ETextureUsage::Default);
	if (!tex)return nullptr;
	for (int i = 0; i < image->mipmaps.size(); i++)
	{
		tex->UpdateImage(i, 0, 0, image->mipmaps[i].width, image->mipmaps[i].height, ((unsigned char*)image->data) + image->mipmaps[i].offset);
	}
	return tex;
}

bool TextureOpenGL::UpdateImageComressed(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{
	
	uint32_t blockSize = 0;
	switch (m_Format)
	{
	case ParaEngine::EPixelFormat::DXT1:
		blockSize = 8;
		break;
	case ParaEngine::EPixelFormat::DXT3:
		blockSize = 16;
		break;
	case ParaEngine::EPixelFormat::DXT5:
		blockSize = 16;
		break;
	default:
		return false;
	}


	GLuint size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height, m_GLFormat, size, pixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return true;
}

bool TextureOpenGL::UpdateImageUncomressed(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{
	// flip vectical
	uint32_t bpp = 0;
	switch (m_GLFormat)
	{
	case GL_RGB:
		bpp = 3;
		break;
	case GL_RGBA:
	case GL_BGRA:
		bpp = 4;
		break;
	case GL_ALPHA:
	case GL_LUMINANCE:
		bpp = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		bpp = 2;
		break;
		break;
	default:
		break;
	}
	uint32_t pitch = width * bpp;
	const unsigned char* pSrc = pixels;
	unsigned char* pDest = new unsigned  char[height * pitch];
	for (int y = 0; y < height; y++)
	{
		memcpy(pDest + y * pitch, pSrc + (height - 1 - y)*pitch, pitch);
	}
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	uint32_t offy = m_Height - yoffset - height;

	glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, offy, width, height, m_GLFormat, GL_UNSIGNED_BYTE, pDest);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	delete[] pDest;
	return true;
}

TextureOpenGL* TextureOpenGL::CreateComressedTextureWithImage(ImagePtr image)
{

	GLenum glFormat = 0;
	GLenum glDataType = 0;
	GLenum glPixelFormat = 0;
	EPixelFormat format;
	switch (image->Format)
	{
	case Image::IPF_COMPRESSED_DXT1:
		glFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		format = EPixelFormat::DXT1;
		break;
	case Image::IPF_COMPRESSED_DXT3:
		glFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		format = EPixelFormat::DXT3;
		break;
	case Image::IPF_COMPRESSED_DXT5:
		glFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		format = EPixelFormat::DXT5;
		break;
	default:
		return nullptr;
		break;
	}
	if (glFormat == 0) return nullptr;

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);


	for (int i =0;i<image->mipmaps.size();i++)
	{

		glCompressedTexImage2D(GL_TEXTURE_2D, i, glFormat, image->mipmaps[i].width, image->mipmaps[i].height, 0, image->mipmaps[i].size, ((unsigned char*)image->data) + image->mipmaps[i].offset);

	}


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glBindTexture(GL_TEXTURE_2D, 0);


	TextureOpenGL* tex = new TextureOpenGL();

	tex->m_TextureID = textureID;
	tex->m_Width = image->mipmaps[0].width;
	tex->m_Height = image->mipmaps[0].height;
	tex->m_GLFormat = glFormat;
	tex->m_Usage = ETextureUsage::Default;
	tex->m_Format = format;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return tex;
}


#include "TextureOpenGL.h"
using namespace ParaEngine;
using namespace IParaEngine;

ParaEngine::TextureOpenGL::TextureOpenGL()
	:m_TextureID(0)
	,m_Width(0)
	,m_Height(0)
{

}

ParaEngine::TextureOpenGL::~TextureOpenGL()
{

}

void* ParaEngine::TextureOpenGL::Lock(unsigned int level, unsigned int& pitch, const ParaEngine::Rect* rect)
{
	return nullptr;
}

void ParaEngine::TextureOpenGL::Unlock(unsigned int level)
{
	
}

bool ParaEngine::TextureOpenGL::CopyTo(ITexture* target)
{
	return false;
}

bool ParaEngine::TextureOpenGL::StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ParaEngine::ETextureFilterType filter)
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

ParaEngine::TextureOpenGL * ParaEngine::TextureOpenGL::CreateWithImage(ImagePtr image)
{
	if (image == nullptr) return nullptr;

	TextureOpenGL* tex = new TextureOpenGL();
	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	GLenum format = GL_RGB;
	switch (image->Format)
	{
	case  Image::IPF_L8:
		format = GL_LUMINANCE;
		break;
	case  Image::IPF_A8:
		format = GL_ALPHA;
		break;
	case  Image::IPF_A8L8:
		format = GL_LUMINANCE_ALPHA;
		break;
	case  Image::IPF_R8G8B8:
		format = GL_RGB;
		break;
	case  Image::IPF_R8G8B8A8:
		format = GL_RGBA;
		break;

	default:
		break;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height,0, format, GL_UNSIGNED_BYTE, image->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	tex->m_TextureID = textureID;
	tex->m_Width = image->width;
	tex->m_Height = image->height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return tex;
}

void ParaEngine::TextureOpenGL::OnRelease()
{
	glDeleteTextures(1, &m_TextureID);
	m_TextureID = 0;
	m_Width = 0;
	m_Height = 0;
}

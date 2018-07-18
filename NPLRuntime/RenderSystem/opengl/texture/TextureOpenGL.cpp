#include "TextureOpenGL.h"

using namespace ParaEngine;
using namespace IParaEngine;

ParaEngine::TextureOpenGL::TextureOpenGL()
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
	return 0;
}

uint32_t ParaEngine::TextureOpenGL::GetHeight() const
{
	return 0;
}

void ParaEngine::TextureOpenGL::OnRelease()
{
	
}

#include "TextureD3D9.h"
using namespace ParaEngine;
using namespace IParaEngine;



ITexture* ParaEngine::TextureD3D9::Create(LPDIRECT3DDEVICE9 device,uint32_t width, uint32_t height, ETextureFormat format)
{
	TextureD3D9* texture = new TextureD3D9;
	if (texture->InnerCreate(device,width, height, format))
	{
		return texture;
	}
	delete texture;
	return nullptr;
}

void* ParaEngine::TextureD3D9::Lock(unsigned int level, unsigned int& pitch)
{
	assert(m_Texture);
	if (!m_Texture)return nullptr;

	D3DLOCKED_RECT lockedRect;
	if (m_Texture->LockRect(0, &lockedRect, NULL, 0) != S_OK) return nullptr;

	pitch = lockedRect.Pitch;
	return lockedRect.pBits;
}

void ParaEngine::TextureD3D9::Unlock()
{
	assert(m_Texture);
	if (!m_Texture)return;

	m_Texture->UnlockRect(0);
}

LPDIRECT3DSURFACE9 ParaEngine::TextureD3D9::GetSurface() const
{
	//return m_Texture;
}

LPDIRECT3DTEXTURE9 ParaEngine::TextureD3D9::GetTexture() const
{
	return m_Texture;
}

bool ParaEngine::TextureD3D9::InnerCreate(LPDIRECT3DDEVICE9 device,uint32_t width, uint32_t height, ETextureFormat format)
{
	D3DFORMAT d3dFormat = D3DFMT_UNKNOWN;
	switch (format)
	{
	case ParaEngine::A8R8G8B8:
		d3dFormat = D3DFMT_A8R8G8B8;
		break;
	case ParaEngine::ALPHA8:
		d3dFormat = D3DFMT_A8;
		break;
	case ParaEngine::X8R8G8B8:
		d3dFormat = D3DFMT_X8R8G8B8;
		break;
	default:
		break;
	}

	if (d3dFormat == D3DFMT_UNKNOWN)  return false;

	LPDIRECT3DTEXTURE9 deviceTexture;
	HRESULT hr = device->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, d3dFormat, D3DPOOL_MANAGED, &deviceTexture, NULL);
	if (hr != S_OK) return false;

	m_Texture = deviceTexture;
	m_Texture->GetSurfaceLevel(0, &m_Surface);
	m_Width = width;
	m_Height = height;
	m_Format = format;
	return true;
}

TextureD3D9::TextureD3D9()
	:m_Texture(NULL)
	,m_Surface(NULL)
	,m_Format(ETextureFormat::UNKONWN)
	,m_Width(0)
	,m_Height(0)
{

}

void TextureD3D9::OnRelease()
{
	if (m_Texture)
	{
		m_Surface->Release();
		m_Texture->Release();
	}
	m_Texture = NULL;
	m_Surface = NULL;
	m_Width = 0;
	m_Height = 0;
	m_Format = ETextureFormat::UNKONWN;
}

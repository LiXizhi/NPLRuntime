#include "TextureD3D9.h"
#include "d3dx9.h"
#include "D3DMapping.h"
#include "RenderDeviceD3D9.h"
using namespace ParaEngine;
using namespace IParaEngine;



ITexture* ParaEngine::TextureD3D9::Create(RenderDeviceD3D9* device,uint32_t width, uint32_t height, EPixelFormat format,ETextureUsage usage)
{
	TextureD3D9* texture = new TextureD3D9;
	texture->m_Device = device;
	if (texture->InnerCreate(width, height, format,usage))
	{
		return texture;
	}
	delete texture;
	return nullptr;
}

IParaEngine::ITexture* ParaEngine::TextureD3D9::Create(RenderDeviceD3D9* device, LPDIRECT3DSURFACE9 surface, ETextureUsage usage)
{
	if (surface == nullptr) return NULL;
	TextureD3D9* tex = new TextureD3D9;
	tex->m_Device = device;
	tex->m_Texture = nullptr;
	tex->m_Surface = surface;
	D3DSURFACE_DESC desc;
	tex->m_Surface->GetDesc(&desc);
	tex->m_Width = desc.Width;
	tex->m_Height = desc.Height;
	tex->m_Usage = usage;
	tex->m_Format = D3DMapping::toPixelFormat(desc.Format);
}

IParaEngine::ITexture* ParaEngine::TextureD3D9::Create(RenderDeviceD3D9* device,LPDIRECT3DTEXTURE9 texture)
{
	if (texture == nullptr) return NULL;

	LPDIRECT3DSURFACE9 surface = NULL;
	texture->GetSurfaceLevel(0, &surface);

	if (surface == nullptr) return nullptr;


	TextureD3D9* tex = new TextureD3D9;
	tex->m_Device = device;
	tex->m_Texture = texture;
	tex->m_Surface = surface;
	D3DSURFACE_DESC desc;
	tex->m_Surface->GetDesc(&desc);
	
	tex->m_Width = desc.Width;
	tex->m_Height = desc.Height;

	tex->m_Format = D3DMapping::toPixelFormat(desc.Format);

	return tex;
}

void* ParaEngine::TextureD3D9::Lock(unsigned int level, unsigned int& pitch, const ParaEngine::Rect* rect)
{
	assert(m_Texture);
	if (!m_Texture)return nullptr;

	D3DLOCKED_RECT lockedRect;
	RECT* dxRect = NULL;
	if (rect)
	{
		RECT r;
		r.left = rect->x;
		r.top = rect->y;
		r.right = rect->z;
		r.bottom = rect->w;
		dxRect = &r;
	}


	if (m_Texture->LockRect(0, &lockedRect, dxRect, 0) != S_OK) return nullptr;

	pitch = lockedRect.Pitch;
	return lockedRect.pBits;
}

void ParaEngine::TextureD3D9::Unlock(unsigned int level)
{
	assert(m_Texture);
	if (!m_Texture)return;

	m_Texture->UnlockRect(level);
}

LPDIRECT3DSURFACE9 ParaEngine::TextureD3D9::GetSurface() const
{
	return m_Surface;
}

LPDIRECT3DTEXTURE9 ParaEngine::TextureD3D9::GetTexture() const
{
	return m_Texture;
}

bool ParaEngine::TextureD3D9::CopyTo(ITexture* target)
{
	TextureD3D9* pTargetTexture = static_cast<TextureD3D9*>(target);
	HRESULT hr = D3DXLoadSurfaceFromSurface(pTargetTexture->GetSurface(), NULL, NULL, this->GetSurface(), NULL, NULL, D3DX_DEFAULT, 0);
	return hr == S_OK;
}

uint32_t ParaEngine::TextureD3D9::GetWidth()
{
	return m_Width;
}

uint32_t ParaEngine::TextureD3D9::GetHeight()
{
	return m_Height;
}

bool ParaEngine::TextureD3D9::StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ETextureFilterType filter)
{
	TextureD3D9* pTarget = static_cast<TextureD3D9*>(target);
	D3DTEXTUREFILTERTYPE d3dfilterType = D3DMapping::toD3DTextureFilter(filter);
	HRESULT hr = S_FALSE;

	RECT* dxSrcRect = NULL;
	RECT r_src;
	if (srcRect)
	{
		
		r_src.left = srcRect->x;
		r_src.top = srcRect->y;
		r_src.right = srcRect->z;
		r_src.bottom = srcRect->w;
		dxSrcRect = &r_src;
	}

	RECT* dxTargetRect = NULL;
	RECT r_target;
	if (targetRect)
	{
		r_target.left = targetRect->x;
		r_target.top = targetRect->y;
		r_target.right = targetRect->z;
		r_target.bottom = targetRect->w;
		dxTargetRect = &r_target;
	}
	hr = m_Device->GetDirect3DDevice9()->StretchRect(this->m_Surface, dxSrcRect, pTarget->m_Surface, dxTargetRect, d3dfilterType);

	return hr == S_OK;
}

bool ParaEngine::TextureD3D9::InnerCreate(uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage)
{
	D3DFORMAT d3dFormat = D3DMapping::toD3DFromat(format);


	if (d3dFormat == D3DFMT_UNKNOWN)  return false;


	DWORD d3dUsage = 0;

	switch (usage)
	{
	case ParaEngine::ETextureUsage::Default:
		d3dUsage = 0;
		break;
	case ParaEngine::ETextureUsage::RenderTarget:
		d3dUsage = D3DUSAGE_RENDERTARGET;
		break;
	case ParaEngine::ETextureUsage::DepthStencil:
		d3dUsage = D3DUSAGE_DEPTHSTENCIL;
		break;
	case ParaEngine::ETextureUsage::Dynamic:
		d3dUsage = D3DUSAGE_DYNAMIC;
		break;
	default:
		break;
	}


	LPDIRECT3DTEXTURE9 deviceTexture;
	LPDIRECT3DDEVICE9 d3dDevice = m_Device->GetDirect3DDevice9();
	HRESULT hr = d3dDevice->CreateTexture(width, height, 0, d3dUsage, d3dFormat, D3DPOOL_MANAGED, &deviceTexture, NULL);
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
	,m_Format(EPixelFormat::Unkonwn)
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
	m_Format = EPixelFormat::Unkonwn;
}

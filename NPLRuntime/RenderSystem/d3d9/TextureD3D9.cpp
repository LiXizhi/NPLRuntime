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

IParaEngine::ITexture* ParaEngine::TextureD3D9::CreateWithImage(RenderDeviceD3D9* device,ImagePtr image)
{
	EPixelFormat format = EPixelFormat::A8;
	switch (image->Format)
	{
	case Image::IPF_A8:
		format = EPixelFormat::A8;
		break;
	case Image::IPF_A8L8:
		format = EPixelFormat::A8L8;
		break;
	case Image::IPF_L8:
		format = EPixelFormat::L8;
		break;
	case Image::IPF_B8G8R8A8:
		format = EPixelFormat::A8B8G8R8;
		break;
	case Image::IPF_R8G8B8:
		format = EPixelFormat::R8G8B8;
		break;
	case Image::IPF_R8G8B8A8:
		format = EPixelFormat::A8R8G8B8;
		break;
	default:
		break;
	}
	auto tex = TextureD3D9::Create(device, image->mipmaps[0].width, image->mipmaps[0].height,format, ETextureUsage::Default);
	if (!tex) return nullptr;

	for (size_t i = 0; i < image->mipmaps.size(); i++)
	{
		tex->UpdateImage(i, 0, 0, image->mipmaps[i].width, image->mipmaps[i].height, (unsigned char*)image->data + image->mipmaps[i].offset);
	}
	
	return tex;
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
	return tex;
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

uint32_t ParaEngine::TextureD3D9::GetWidth() const
{
	return m_Width;
}

uint32_t ParaEngine::TextureD3D9::GetHeight() const
{
	return m_Height;
}

bool ParaEngine::TextureD3D9::StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ETextureFilter filter)
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



void update_image_rgb8(unsigned char* pDst, const unsigned char* pSrc,uint32_t width,uint32_t height)
{
	int index = 0;
	uint32_t bpp = 4;
	uint32_t pitch = width * bpp;
	for (int y = 0; y < height; y++)
	{
		
		for (int x = 0; x < width; x++)
		{
			int pos = y * 3 * width + 3 * x;
			pDst[index++] = pSrc[pos+2]; //B
			pDst[index++] = pSrc[pos+1]; //G
			pDst[index++] = pSrc[pos]; //R
			pDst[index++] = 0xff; //A 
		}
		index += pitch - (width * 4);
	}
}
void update_image_rgba8(unsigned char* pDst, const unsigned char* pSrc, uint32_t width, uint32_t height)
{
	int index = 0;
	uint32_t bpp = 4;
	uint32_t pitch = width * bpp;
	for (int y = 0; y < height; y++)
	{

		for (int x = 0; x < width; x++)
		{
			int pos = y * pitch + 4 * x;
			pDst[index++] =	pSrc[pos+2]; //B
			pDst[index++] =	pSrc[pos+1]; //G
			pDst[index++] =	pSrc[pos]; //R
			pDst[index++] =	pSrc[pos+3]; //A 
		}
		index += pitch - (width * 4);
	}
}

bool ParaEngine::TextureD3D9::UpdateImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{
	if (m_Texture == nullptr) return false;
	D3DLOCKED_RECT lockrect;
	RECT rect;
	rect.left = xoffset;
	rect.top = yoffset;
	rect.right = width;
	rect.bottom = height;
	HRESULT hr = m_Texture->LockRect(level, &lockrect,nullptr, 0);
	if (hr != S_OK) return false;
	unsigned char* pDst = (unsigned char*)lockrect.pBits;
	
	switch (m_Format)
	{
	case ParaEngine::EPixelFormat::A8R8G8B8:
		update_image_rgba8(pDst, pixels, width, height);
		break;
	case ParaEngine::EPixelFormat::R8G8B8:
		update_image_rgb8(pDst, pixels, width, height);
		break;
	case ParaEngine::EPixelFormat::A8B8G8R8:
		break;
	case ParaEngine::EPixelFormat::X8B8G8R8:
		break;
	case ParaEngine::EPixelFormat::L8:
		break;
	case ParaEngine::EPixelFormat::A8L8:
		break;
	default:
		break;
	}

	hr = m_Texture->UnlockRect(level);
	if (hr != S_OK) return false;
	return true;
}

ParaEngine::ImagePtr ParaEngine::TextureD3D9::GetImage(uint32_t level)
{
	return nullptr;
}

ParaEngine::ETextureFilter ParaEngine::TextureD3D9::GetMinFilter() const
{
	return ETextureFilter::Point;
}

ParaEngine::ETextureFilter ParaEngine::TextureD3D9::GetMagFilter() const
{
	return ETextureFilter::Point;
}

bool ParaEngine::TextureD3D9::SetMinFilter(ParaEngine::ETextureFilter type)
{
	return true;
}

bool ParaEngine::TextureD3D9::SetMagFilter(ParaEngine::ETextureFilter type)
{
	return true;
}

ParaEngine::ETextureWrapMode ParaEngine::TextureD3D9::GetAddressU() const
{
	return ETextureWrapMode::Repeat;
}

ParaEngine::ETextureWrapMode ParaEngine::TextureD3D9::GetAddressV() const
{
	return ETextureWrapMode::Clamp;
}

bool ParaEngine::TextureD3D9::SetAddressU(ParaEngine::ETextureWrapMode mode)
{
	return true;
}

bool ParaEngine::TextureD3D9::SetAddressV(ParaEngine::ETextureWrapMode mode)
{
	return true;
}


bool ParaEngine::TextureD3D9::InnerCreate(uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage)
{
	D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8;
	
	if (format != EPixelFormat::R8G8B8)
	{
		d3dFormat = D3DMapping::toD3DFromat(format);
	}

	if (d3dFormat == D3DFMT_UNKNOWN)  return false;


	DWORD d3dUsage = 0;
	D3DPOOL pool = D3DPOOL_DEFAULT;

	switch (usage)
	{
	case ParaEngine::ETextureUsage::Default:
		d3dUsage = 0;
		pool = D3DPOOL_MANAGED;
		break;
	case ParaEngine::ETextureUsage::RenderTarget:
		d3dUsage = D3DUSAGE_RENDERTARGET;
		pool = D3DPOOL_DEFAULT;
		break;
	case ParaEngine::ETextureUsage::DepthStencil:
		d3dUsage = D3DUSAGE_DEPTHSTENCIL;
		pool = D3DPOOL_DEFAULT;
		break;
	case ParaEngine::ETextureUsage::Dynamic:
		d3dUsage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;
		break;
	default:
		break;
	}


	LPDIRECT3DTEXTURE9 deviceTexture;
	LPDIRECT3DDEVICE9 d3dDevice = m_Device->GetDirect3DDevice9();
	HRESULT hr = d3dDevice->CreateTexture(width, height, 1, d3dUsage, d3dFormat, pool, &deviceTexture, NULL);
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

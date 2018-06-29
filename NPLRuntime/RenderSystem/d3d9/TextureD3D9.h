#pragma once
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Interface/Render/ITexture.h"
#include <d3d9.h>
namespace ParaEngine
{
	class TextureD3D9 : public IParaEngine::ITexture
	{
	public:
		static ITexture* Create(LPDIRECT3DDEVICE9 device,uint32_t width, uint32_t height, ETextureFormat format);
		TextureD3D9();
		virtual ~TextureD3D9() override = default;
		virtual void OnRelease();
		virtual void* Lock(unsigned int level, unsigned int& pitch) override;
		virtual void Unlock() override;

		LPDIRECT3DSURFACE9 GetSurface() const;
		LPDIRECT3DTEXTURE9 GetTexture() const;

	protected:
		LPDIRECT3DTEXTURE9 m_Texture;
		LPDIRECT3DSURFACE9 m_Surface;
		LPDIRECT3DDEVICE9 m_Device;
		ETextureFormat m_Format;
		uint32_t m_Width;
		uint32_t m_Height;

	private:
		bool InnerCreate(LPDIRECT3DDEVICE9 device,uint32_t width, uint32_t height, ETextureFormat format);
	};
};
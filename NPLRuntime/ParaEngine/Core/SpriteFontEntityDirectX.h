#pragma once
#include "SpriteFontEntity.h"

namespace ParaEngine
{
	struct SpriteFontEntityDirectX : public SpriteFontEntity
	{
	private:
		ID3DXSprite*  m_pD3DXSprite;

		/// font attribute
		union _FontAttribute
		{
			struct _fontGDI
			{
				LPD3DXFONT		m_pFont;
			}GDIFont;

			struct _fontBitmap
			{
				/// Number of characters per line in the image
				int				m_nCharsPerLine;
				/// Height of each letter
				int				m_nLetterHeight;
				/// Width of each letter
				int				m_nLetterWidth;
				LPDIRECT3DTEXTURE9  m_pTexture;
			}BMPFont;
		}FontAttibute;


	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::font; };
		/// how font texture is created
		enum _FontTextureType
		{
			/// GDI font, can be trueType
			sprite_font_GDI = 0,
			/// font from self-supplied bitmap file
			sprite_font_bitmap = 1
		}TextureType;
		
	public:
		SpriteFontEntityDirectX(const AssetKey& key);
		SpriteFontEntityDirectX();

		virtual ~SpriteFontEntityDirectX();

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		ID3DXSprite* GetFontSprite();
		LPD3DXFONT GetFont();

		virtual HRESULT DrawTextA(CSpriteRenderer* pSprite, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor);

		virtual HRESULT DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor);

	};
}
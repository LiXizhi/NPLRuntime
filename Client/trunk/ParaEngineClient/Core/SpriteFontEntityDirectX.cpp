//-----------------------------------------------------------------------------
// Class:	SpriteFontEntityDirectX
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12, 2014.8(cross platform)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef USE_DIRECTX_RENDERER
#include "SpriteRendererDirectX.h"
#include "SpriteFontEntity.h"
#include "StringHelper.h"
#include "memdebug.h"

using namespace ParaEngine;

SpriteFontEntityDirectX::SpriteFontEntityDirectX(const AssetKey& key)
:SpriteFontEntity(key)
{
	TextureType = sprite_font_GDI;
	m_pD3DXSprite = NULL;
	m_nWeight = FW_BOLD;
	memset(&FontAttibute, 0, sizeof(FontAttibute));
}
SpriteFontEntityDirectX::SpriteFontEntityDirectX()
{
	TextureType = sprite_font_GDI;
	m_pD3DXSprite = NULL;
	m_nWeight = FW_BOLD;
};

SpriteFontEntityDirectX::~SpriteFontEntityDirectX()
{
}

LPD3DXFONT SpriteFontEntityDirectX::GetFont()
{
	LoadAsset();
	return FontAttibute.GDIFont.m_pFont;
}

ID3DXSprite* SpriteFontEntityDirectX::GetFontSprite()
{
	LoadAsset();
	return m_pD3DXSprite;
}

HRESULT SpriteFontEntityDirectX::InitDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;

	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	// Initialize font
	if (TextureType == SpriteFontEntityDirectX::sprite_font_GDI)
	{
		string sFontName = TranslateFontName(m_sFontName);
		string sLocalFont = "fonts\\";
		sLocalFont += sFontName;
		sLocalFont += ".ttf";

		std::transform(sLocalFont.begin(), sLocalFont.end(), sLocalFont.begin(), std::tolower);

		if (CParaFile::DoesFileExist(sLocalFont.c_str(), false))
		{
			static map<string, bool> g_LoadedFonts;
			if (g_LoadedFonts.find(sLocalFont) == g_LoadedFonts.end())
			{
				g_LoadedFonts[sLocalFont] = true;
				int nAddedCount = AddFontResourceEx(sLocalFont.c_str(), FR_PRIVATE, 0);
				if (nAddedCount == 0)
				{
					OUTPUT_LOG("warning: failed loading font %s\n", sLocalFont.c_str());
				}
			}
		}


		// for GDI font
		HRESULT hr;
		int nHeight;
		// #define USE_LOGICAL_FONTSIZE
#ifdef USE_LOGICAL_FONTSIZE
		HDC hDC = GetDC(NULL);
		int nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY); // nLogPixelsY is 96 on my computer
		ReleaseDC(NULL, hDC);
		// nHeight = - m_nFontSize * nLogPixelsY / 72;
		nHeight = -MulDiv(m_nFontSize, nLogPixelsY, 72);
#else
		nHeight = -m_nFontSize;
#endif

		// see:  it is very tricky for cell height or character height. 
		// http://msdn.microsoft.com/en-us/library/dd183499.aspx
		// http://www.emdpi.com/fontsize.html
		hr = D3DXCreateFont(pd3dDevice,          // D3D device
			nHeight,               // Height
			0,                     // Width
			m_nWeight,             // Weight
			1,                     // MipLevels, 0 = autogen mipmaps
			FALSE,                 // Italic
			DEFAULT_CHARSET,       // CharSet
			OUT_TT_ONLY_PRECIS,    // OutputPrecision
			PROOF_QUALITY, // ANTIALIASED_QUALITY,       // Quality
			DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
			sFontName.c_str(),				// pFaceName
			&(FontAttibute.GDIFont.m_pFont));           // ppFont
		if (FAILED(hr))
		{
			OUTPUT_LOG("warning: font %s is not found. we will default to Arial font.", sFontName.c_str());

			if (FAILED(hr = D3DXCreateFont(pd3dDevice, nHeight, 0, m_nWeight, 0, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE,
				TEXT("Arial"), &(FontAttibute.GDIFont.m_pFont))))
			{
				OUTPUT_LOG("warning: font %s is not found. we will default to Arial font. but even Arial font is failed.", sFontName.c_str());
				// m_bIsInitialized = false;
				return hr;
			}
		}
		else
		{
			/*{
			HDC hDC = FontAttibute.GDIFont.m_pFont->GetDC();
			DWORD langInfo = GetFontLanguageInfo(hDC);
			if(langInfo == GCP_ERROR)
			{
			OUTPUT_LOG("text font contains error \n");
			}
			else if(langInfo & GCP_DIACRITIC)
			{
			OUTPUT_LOG("text font contains GCP_DIACRITIC\n");
			}
			ReleaseDC( NULL, hDC );
			}*/
		}
		//if( FAILED( hr = D3DXCreateSprite( pd3dDevice, &(m_pD3DXSprite)) ) )
		//{
		//	SAFE_RELEASE( (FontAttibute.GDIFont.m_pFont) );
		//	// m_bIsInitialized = false;
		//	OUTPUT_LOG("warning: D3DXCreateSprite failed in Font init()\n");
		//	return hr;
		//}
	}
	else
	{
		// for texture font
	}
	return S_OK;
}

HRESULT SpriteFontEntityDirectX::InvalidateDeviceObjects()
{
	if (TextureType == SpriteFontEntityDirectX::sprite_font_GDI)
	{
		// for GDI font
		if (FontAttibute.GDIFont.m_pFont)
			(FontAttibute.GDIFont.m_pFont)->OnLostDevice();
	}
	else
	{
	}
	if (m_pD3DXSprite)
		m_pD3DXSprite->OnLostDevice();
	return S_OK;
}

HRESULT SpriteFontEntityDirectX::RestoreDeviceObjects()
{
	if (TextureType == SpriteFontEntityDirectX::sprite_font_GDI)
	{
		// for GDI font
		if (FontAttibute.GDIFont.m_pFont)
			(FontAttibute.GDIFont.m_pFont)->OnResetDevice();
	}
	else
	{
	}
	if (m_pD3DXSprite)
		m_pD3DXSprite->OnResetDevice();
	return S_OK;
}

HRESULT SpriteFontEntityDirectX::DeleteDeviceObjects()
{
	m_bIsInitialized = false;

	if (TextureType == SpriteFontEntityDirectX::sprite_font_GDI)
	{
		// for GDI font
		SAFE_RELEASE((FontAttibute.GDIFont.m_pFont));
	}
	else
	{
		// for texture font: 
		SAFE_RELEASE((FontAttibute.BMPFont.m_pTexture));
	}
	SAFE_RELEASE(m_pD3DXSprite);
	return S_OK;
}

HRESULT ParaEngine::SpriteFontEntityDirectX::DrawTextA(CSpriteRenderer* pSprite, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD shadowColor)
{
	LPD3DXFONT pFontNode = GetFont();
	if (pFontNode != NULL && pSprite)
	{
		if (!strText || strText[0] == '\0')
			return false;
		std::string utf8Str = strText;
		std::u16string utf16Str;
		if (StringHelper::UTF8ToUTF16(utf8Str, utf16Str))
		{
			DrawTextW(pSprite, utf16Str.c_str(), (int)utf16Str.size(), rect, dwTextFormat, shadowColor);
		}
	}
	return S_OK;
}

HRESULT ParaEngine::SpriteFontEntityDirectX::DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD shadowColor)
{
	LPD3DXFONT pFontNode = GetFont();
	if (pFontNode != NULL && pSprite)
	{
		HRESULT res = pFontNode->DrawTextW(((CSpriteRendererDirectX*)pSprite)->GetSprite(), (LPCWSTR)strText, nCount, rect, dwTextFormat, shadowColor);
		return res;
	}
	return S_OK;
}

#endif
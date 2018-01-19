#pragma once
#include "AssetEntity.h"

#ifndef DT_TOP
/** text alignment */
#define DT_TOP                      0x00000000
#define DT_LEFT                     0x00000000
#define DT_CENTER                   0x00000001
#define DT_RIGHT                    0x00000002
#define DT_VCENTER                  0x00000004
#define DT_BOTTOM                   0x00000008
#define DT_WORDBREAK                0x00000010
#define DT_SINGLELINE               0x00000020
#define DT_EXPANDTABS               0x00000040
#define DT_TABSTOP                  0x00000080
#define DT_NOCLIP                   0x00000100
#define DT_EXTERNALLEADING          0x00000200
#define DT_CALCRECT                 0x00000400
#define DT_NOPREFIX                 0x00000800
#define DT_INTERNAL                 0x00001000
#endif

namespace ParaEngine
{
	class CSpriteRenderer;
	struct SpriteFontEntity : public AssetEntity
	{
	public:
		string m_sFontName;
		int	m_nFontSize;
		int	m_nWeight;

		// a mapping from a logical font name to the real font name(type face name)
		static map<string, string> g_mapFontNames;
	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::font; };
	public:
		SpriteFontEntity(const AssetKey& key);
		SpriteFontEntity();

		virtual ~SpriteFontEntity();
		
		virtual HRESULT DrawTextA(CSpriteRenderer* pSprite, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor){ return S_OK; };
		virtual HRESULT DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor){ return S_OK; };

		const std::string& GetFontName() const;
		int GetFontSize() const;

		/**
		* give an alias name to a given font name. The most common use of this function is to replace the "System" font
		* with a custom game font installed at "fonts/" folder.
		* @param sLocalName: a local file name like "System", "Default"
		* @param sTypeFaceName: the real type face name to use when creating the font.
		* please note that, the engine will search for the file "fonts/[sTypeFaceName].ttf", if this file exists, it will use that it, instead of the system installed font if any.
		* Note: game custom font files under "fonts/" must be named by their true font name (i.e. type face name), otherwise they will not be loaded properly.
		*/
		static void AddFontName(const string& sLocalName, const string& sTypeFaceName);

		/**
		* we will secretly translate the font name, according to font name mapping.
		* @return: input sLocalName is returned if no one is found, otherwise, we will return the new TypeFaceName
		*/
		static const string& TranslateFontName(const string& sLocalName);
	};
}

// chose an implementation as Texture Manager
#ifdef USE_DIRECTX_RENDERER
#include "SpriteFontEntityDirectX.h"
namespace ParaEngine{
	typedef AssetManager<SpriteFontEntity, SpriteFontEntityDirectX> SpriteFontAssetManager;
}
#elif defined(USE_OPENGL_RENDERER)
#include "SpriteFontEntityOpenGL.h"
namespace ParaEngine{
	typedef AssetManager<SpriteFontEntity, SpriteFontEntityOpenGL> SpriteFontAssetManager;
}
#else
namespace ParaEngine{
	typedef AssetManager<SpriteFontEntity, SpriteFontEntity> SpriteFontAssetManager;
}
#endif
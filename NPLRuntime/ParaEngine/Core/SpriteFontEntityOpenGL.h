#pragma once
#include "SpriteFontEntity.h"

namespace ParaEngine
{
	class CFontRendererOpenGL;

	/** rendering font */
	struct SpriteFontEntityOpenGL : public SpriteFontEntity
	{
	
	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::font; };
	public:
		SpriteFontEntityOpenGL(const AssetKey& key);
		SpriteFontEntityOpenGL();
		virtual ~SpriteFontEntityOpenGL();
		
		virtual HRESULT DrawTextA(CSpriteRenderer* pSprite, const char* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor);
		virtual HRESULT DrawTextW(CSpriteRenderer* pSprite, const char16_t* strText, int nCount, RECT* rect, DWORD dwTextFormat, DWORD textColor);

		virtual HRESULT InitDeviceObjects();

		virtual HRESULT DeleteDeviceObjects();

		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		virtual HRESULT RendererRecreated();;

		virtual void Cleanup();

	protected:
		// for text-related calculations 
		CFontRendererOpenGL*    m_fontRenderer;
	};
}
#pragma once
#include "VertexFVF.h"
#include "SpriteRenderer.h"

namespace ParaEngine
{
	class CSpriteRendererDirectX : public CSpriteRenderer
	{
	public:
		CSpriteRendererDirectX();
		virtual ~CSpriteRendererDirectX();
	public:
		/**
		* @param Flags: D3DXSPRITE_DONOTSAVESTATE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE
		*/
		virtual HRESULT Begin(DWORD Flags);

		virtual void End();

		virtual void FlushQuads();

		virtual HRESULT DrawRect(const RECT* pRect, Color color, float depth);

		virtual HRESULT DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c);

		virtual HRESULT SetTransform(const Matrix4& mat);
		
		virtual void InitDeviceObjects();

		virtual void RestoreDeviceObjects();

		virtual void InvalidateDeviceObjects();

		virtual void DeleteDeviceObjects();

	public:
		ID3DXSprite* GetSprite() const { return m_pSprite; }
	protected:
		// shared drawing sprite, for drawing all gui elements
		ID3DXSprite*			m_pSprite;
		VertexDeclarationPtr m_pLastDecl;
	};
}


#pragma once
#include "SpriteRenderer.h"

namespace ParaEngine
{
	class CEffectFile;

	class CSpriteRendererOpenGL : public CSpriteRenderer
	{
	public:
		CSpriteRendererOpenGL();
		virtual ~CSpriteRendererOpenGL();

		void Cleanup();

	public:
		virtual HRESULT Begin(DWORD Flags);
		virtual void End();
		virtual void FlushQuads();

		virtual bool BeginCustomShader();
		virtual void EndCustomShader();
		virtual void UpdateShader(bool bForceUpdate = false);
		virtual void SetNeedUpdateShader(bool bNeedUpdate);

		virtual HRESULT DrawRect(const RECT* pRect, Color color, float depth);

		virtual HRESULT DrawQuad(TextureEntity* pTexture, const RECT* pSrcRect, const Vector3* vCenter, const Vector3* pPosition, Color c);

		virtual void InitDeviceObjects();

		virtual void RestoreDeviceObjects();

		virtual void InvalidateDeviceObjects();

		virtual void DeleteDeviceObjects();

		/** whether to set text mode.*/
		virtual void SetTextMode(bool bIsTextMode = true);

		virtual void PrepareDraw();

	protected:
		virtual void DrawTriangles(const sprite_vertex* pVertices, int nTriangleCount);

	protected:
		Matrix4 m_proj;
		BOOL m_ready;
		/* Store the relevant caps to prevent multiple GetDeviceCaps calls */
		DWORD texfilter_caps;
		DWORD maxanisotropy;
		DWORD alphacmp_caps;

		ParaViewport m_viewport;
		ParaViewport m_lastViewport;
		// all sprites
		std::vector<SpriteQuad> m_sprites;
		// number of sprites to be drawn
		int m_sprite_count; 
		CEffectFile* m_pEffectFile;
		bool m_bNeedUpdateShader;
		/** current text mode */
		bool m_bIsTextMode;
		/** text mode on current shader*/
		bool m_bIsTextModeDevice;
	};
}


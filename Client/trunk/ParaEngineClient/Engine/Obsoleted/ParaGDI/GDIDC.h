#pragma once
#include <map>
using namespace std;
namespace ParaEngine
{
	class CGDIDC;
	struct TextureEntity;

	struct GDIDCElement{
		CGDIDC* pDC;
		int refcount;
	};
	class CGDIDC
	{
	public:
		CGDIDC(TextureEntity* pTexture);
		CGDIDC();
		~CGDIDC();
		void Release();

		HRESULT Begin();
		HRESULT End();
		HRESULT Clear(const Color &color);
		//these are internal graphic manipulating functions
		HRESULT DrawTriangleStrip(TextureEntity *pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertexData);
		HRESULT DrawTriangleStripPrim(LPDIRECT3DTEXTURE9 pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertexData);
		HRESULT DrawTriangleList(TextureEntity *pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertexData);
		void SetTransform(const Matrix4 *matTransform);
		void GetTransform(Matrix4 *matTransform);
		
		TextureEntity* GetRenderTarget();

		/** enable write alpha */
		void EnableWriteAlpha(bool bWriteAlpha);

	protected:
		asset_ptr<TextureEntity> m_pTextureEntity;
		LPDIRECT3DSURFACE9 m_pOldRenderSurface;
		
		Matrix4 m_transform;
	};
}
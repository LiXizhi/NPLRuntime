#pragma once
#include "AssetEntity.h"
#include "SpriteFontEntity.h"
#include "DynamicVertexBufferEntity.h"

namespace ParaEngine
{
#ifdef USE_DIRECTX_RENDERER

	class CBipedObject;
	
	//--------------------------------------------------------
	/// D3DXSpriteEntity distinguish one template from other
	//--------------------------------------------------------
	struct D3DXSpriteEntity : public AssetEntity
	{
	private:
		/// Vertex buffer for rendering a 2d object movie
		LPDIRECT3DVERTEXBUFFER9 m_pSpriteVB;

	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::spritevertex; };
		/// number of frames(quads) in the buffer.
		int		m_nFrames;
		/// how many rows and columns are there in the texture.
		int		m_nRow, m_nCol;

	public:
		D3DXSpriteEntity(const AssetKey& key)
			:AssetEntity(key)
		{
			m_pSpriteVB = NULL;
		}
		D3DXSpriteEntity()
		{
			m_pSpriteVB = NULL;
		};
		virtual ~D3DXSpriteEntity(){};

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		LPDIRECT3DVERTEXBUFFER9 GetVB(){
			LoadAsset();
			return m_pSpriteVB;
		}
	};

#endif
}
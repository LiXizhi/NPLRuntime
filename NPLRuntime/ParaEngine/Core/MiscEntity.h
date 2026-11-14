#pragma once
#include "AssetEntity.h"
#include "SpriteFontEntity.h"
#include "DynamicVertexBufferEntity.h"

namespace ParaEngine
{
#ifndef USE_NULL_RENDERER

	class CBipedObject;
	
	//--------------------------------------------------------
	/// D3DXSpriteEntity distinguish one template from other
	//--------------------------------------------------------
	struct D3DXSpriteEntity : public AssetEntity
	{
	private:
		/// Vertex buffer for rendering a 2d object movie
		VertexBufferDevicePtr_type m_pSpriteVB;

	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::spritevertex; };
		/// number of frames(quads) in the buffer.
		int		m_nFrames = 0;
		/// how many rows and columns are there in the texture.
		int		m_nRow = 1, m_nCol = 1;

	public:
		D3DXSpriteEntity(const AssetKey& key)
			:AssetEntity(key), m_pSpriteVB(nullptr)
		{
		}
		D3DXSpriteEntity()
			: m_pSpriteVB(nullptr)
		{
		}
		virtual ~D3DXSpriteEntity()
		{
			DeleteDeviceObjects();
		}

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		VertexBufferDevicePtr_type GetVB(){
			LoadAsset();
			return m_pSpriteVB;
		}
	};

#endif
}
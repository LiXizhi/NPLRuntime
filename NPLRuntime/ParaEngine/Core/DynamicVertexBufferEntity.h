#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"

namespace ParaEngine
{
	/** ID of dynamic vertex buffer asset. This ID can be used as index
	* in to the m_pDVBuffers array in ParaEngine asset manager. */
	enum DynamicVBAssetType
	{
		//D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL|D3DFVF_DIFFUSE
		DVB_XYZ_TEX1_NORM_DIF = 0,
		//D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL
		DVB_XYZ_TEX1_NORM,
		//D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_DIFFUSE
		DVB_XYZ_TEX1_DIF,
		//D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE
		DVB_XYZ_NORM_DIF,
		// float4, float4, float4 (world matrix for instancing)
		DVB_MATRIX,
		//D3DFVF_X
		//DVB_X,
		// number of dynamic buffer.
		DVB_COUNT
	};
}

#ifndef USE_NULL_RENDERER
#include "DynamicVertexBufferEntityOpenGL.h"
#else
namespace ParaEngine
{
	class DynamicVertexBufferEntity
	{
	public: 
		DynamicVertexBufferEntity():m_nUnitSize(0), m_dwDataFormat(0), m_dwBase(0), m_dwNextBase(0), m_dwFlush(0), m_dwDiscard(0)
		{

		}
		VertexBufferDevicePtr_type GetBuffer() { return nullptr; }
		int m_nUnitSize;
		uint32_t m_dwDataFormat;
		uint32_t m_dwBase;
		uint32_t m_dwNextBase;
		uint32_t m_dwFlush;
		uint32_t m_dwDiscard;
		int Lock(int,void**) { return 0; }
		void Unlock() {}
		bool IsMemoryBuffer() { return false; }
		int GetBaseVertex() { return 0; }
		void* GetBaseVertexPointer() { return nullptr; }
	};
}
#endif
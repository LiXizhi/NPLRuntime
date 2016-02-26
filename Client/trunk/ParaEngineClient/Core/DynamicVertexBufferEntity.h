#pragma once
#ifdef USE_DIRECTX_RENDERER
#include "DynamicVertexBufferEntityDirectX.h"
#elif defined(USE_OPENGL_RENDERER)
#include "DynamicVertexBufferEntityOpenGL.h"
#else
#include "DynamicVertexBufferEntityOpenGL.h"
#endif

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
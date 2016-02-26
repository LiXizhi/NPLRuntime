//----------------------------------------------------------------------
// Class:	Dynamic vertex buffer
// Authors:	LiXizhi
// Date:	2014.10.8
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef USE_DIRECTX_RENDERER
#include "DynamicVertexBufferEntityDirectX.h"

using namespace ParaEngine;

// define DOUBLE_PIPELINED_BUFFER to use the double pipeline dynamic buffer algorithm
// although it will not work for some cards. There is no significant performance difference though.
// #define DOUBLE_PIPELINED_BUFFER

/***************************************************************************
* DynamicVertexBufferEntityDirectX entity struct
* I used one large dynamic write-only vertex buffer for all ParaX models. This
* Dynamic buffer is called DynamicVertexBufferEntityDirectX which is a newly added
* type of asset in the asset manager. You can claim a range of vertex buffers
* by calling the lock and unlock method of this entity. An internal base pointer
* is maintained by this asset objects. AGP memory is allocated in a cyclic
* manner along the full buffer range (currently 1MB). Please read the Performance
* Optimization section in the DirectX9 document for why its reasons.
***************************************************************************/
DynamicVertexBufferEntityDirectX::DynamicVertexBufferEntityDirectX()
	:m_lpVB(NULL),
	m_dwBase(0),
	m_dwNextBase(0),
	// better be contains a factor of 3*4=12
#ifdef DOUBLE_PIPELINED_BUFFER
	// 24000=800KB, Please note that this number must be a multiple of 3
	m_dwDiscard(24000),
#else
	// 10000=400KB, Please note that this number must be a multiple of 3
	m_dwDiscard(10200),
#endif
	// normally 1000 vertex is a good choice for current TnL
	m_dwFlush(240),
	// default unit size.
	m_nUnitSize(sizeof(my_vertex))
{

};


HRESULT DynamicVertexBufferEntityDirectX::RestoreDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;

	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
	HRESULT hr = S_OK;
	switch (m_dwDataFormat)
	{
	case DVB_XYZ_TEX1_NORM_DIF:
		hr = pd3dDevice->CreateVertexBuffer(m_dwDiscard*m_nUnitSize,
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, mesh_vertex_normal_color::FVF,
			D3DPOOL_DEFAULT, &(m_lpVB), NULL);
		break;
	case DVB_XYZ_TEX1_NORM:
		hr = pd3dDevice->CreateVertexBuffer(m_dwDiscard*m_nUnitSize,
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, mesh_vertex_normal::FVF,
			D3DPOOL_DEFAULT, &(m_lpVB), NULL);
		break;
	case DVB_XYZ_TEX1_DIF:
		hr = pd3dDevice->CreateVertexBuffer(m_dwDiscard*m_nUnitSize,
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, SPRITEVERTEX::FVF,
			D3DPOOL_DEFAULT, &(m_lpVB), NULL);
		break;
	case DVB_XYZ_NORM_DIF:
		hr = pd3dDevice->CreateVertexBuffer(m_dwDiscard*m_nUnitSize,
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, bmax_vertex::FVF,
			D3DPOOL_DEFAULT, &(m_lpVB), NULL);
		break;
	case DVB_MATRIX:
		hr = pd3dDevice->CreateVertexBuffer(m_dwDiscard*m_nUnitSize,
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &(m_lpVB), NULL);
		break;
		/*case DVB_X:
		hr = pd3dDevice->CreateVertexBuffer(m_dwDiscard*m_nUnitSize,
		D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, 0,
		D3DPOOL_DEFAULT, &(m_lpVB), NULL);
		break;*/
	default:
		m_lpVB = NULL;
		hr = E_FAIL;
		OUTPUT_LOG("error: unsupported Dynamic Vertex Buffer format\n");
		PE_ASSERT(false);
		break;
	}

	if (m_lpVB == NULL || FAILED(hr))
	{
		OUTPUT_LOG("error: can not restore DynamicVertexBufferEntityDirectX. You may have run out of video memory. please restart or nothing will be rendered. \n");
	}
	return S_OK;
}


HRESULT DynamicVertexBufferEntityDirectX::InvalidateDeviceObjects()
{
	SAFE_RELEASE(m_lpVB);
	m_bIsInitialized = false;
	return S_OK;
}
//--------------------------------------------------------------------
/// nSizeOfData: number of vertices to be moved into the vertex buffer.
/// return the number of vertices locked
//--------------------------------------------------------------------
uint32 DynamicVertexBufferEntityDirectX::Lock(uint32 nSize, void** pVertices)
{
	LoadAsset();
	if (m_lpVB == 0)
		return 0;
#ifdef DOUBLE_PIPELINED_BUFFER
	//first we lock down the vertex buffer within our memory (hopefully AGP). We want to add a small chunk
	//of vertex information to the VB. Thus, we use the D3DLOCK_NOOVERWRITE flag. Should our VB no longer
	//have any room, we need to clean away the existing handle to the VB and create a new one with
	//D3DLOCK_DISCARD.
	UINT nSpaceLeft = m_dwDiscard - m_dwNextBase;
	if (nSpaceLeft > nSize)
	{
		// we will add a small chunk, Thus, we use the D3DLOCK_NOOVERWRITE flag.
		// I choose to use D3DLOCK_DISCARD tough for double pipeline TnLs
		if (FAILED(m_lpVB->Lock(m_dwNextBase * m_nUnitSize, nSize * m_nUnitSize,
			pVertices, D3DLOCK_DISCARD/*D3DLOCK_NOOVERWRITE*/)))
		{
			return 0;
		}
		m_dwBase = m_dwNextBase;
		m_dwNextBase += nSize;
		return nSize;
	}
	else if (nSpaceLeft < m_dwFlush)
	{
		m_dwNextBase = 0;
		return Lock(nSize, pVertices);
	}
	else
	{
		// we will lock nSpaceLeft, since it contains enough vertex
		// we should discard the buffer and start all over
		if (FAILED(m_lpVB->Lock(m_dwNextBase* m_nUnitSize, nSpaceLeft * m_nUnitSize,
			pVertices, D3DLOCK_DISCARD)))
		{
			return 0;
		}
		m_dwBase = m_dwNextBase;
		m_dwNextBase = 0;
		return nSpaceLeft;
	}
	// TODO see Bug fixed 2008.5.27 below: if the requested buffer is larger than the total dynamic buffer size, we will break the input. 
	return 0;
#else
	// Reusing one vertex buffer for multiple objects
	// No overwrite will be used if the vertices can fit into 
	//   the space remaining in the vertex buffer.
	DWORD dwLockFlags = D3DLOCK_NOOVERWRITE;

	// Check to see if the entire vertex buffer has been used up yet.
	if (nSize > m_dwDiscard - m_dwNextBase)
	{
		// No space remains. Start over from the beginning 
		//   of the vertex buffer.
		dwLockFlags = D3DLOCK_DISCARD;
		m_dwNextBase = 0;

		// Bug fixed 2008.5.27: if the requested buffer is larger than the total dynamic buffer size, we will break the input. 
		if (nSize > m_dwDiscard)
		{
			nSize = m_dwDiscard;
		}
	}

	// Lock the vertex buffer.
	if (FAILED(m_lpVB->Lock(m_dwNextBase * m_nUnitSize, nSize * m_nUnitSize,
		pVertices, dwLockFlags)))
		return 0;
	// Advance to the next position in the vertex buffer.
	m_dwBase = m_dwNextBase;
	m_dwNextBase += nSize;
	return nSize;
#endif
}

void DynamicVertexBufferEntityDirectX::Unlock()
{
	if (m_lpVB)
		m_lpVB->Unlock();
}

DWORD ParaEngine::DynamicVertexBufferEntityDirectX::GetBaseVertex()
{
	return m_dwBase;
}

bool ParaEngine::DynamicVertexBufferEntityDirectX::IsMemoryBuffer()
{
	return false;
}

IDirect3DVertexBuffer9* ParaEngine::DynamicVertexBufferEntityDirectX::GetBuffer()
{
	LoadAsset();
	return m_lpVB;
}

void* ParaEngine::DynamicVertexBufferEntityDirectX::GetBaseVertexPointer()
{
	return 0;
}

#endif
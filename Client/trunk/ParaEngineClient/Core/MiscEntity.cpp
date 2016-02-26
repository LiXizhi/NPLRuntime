//-----------------------------------------------------------------------------
// Class:	MiscEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef USE_DIRECTX_RENDERER
#include "MiscEntity.h"

using namespace ParaEngine;

HRESULT D3DXSpriteEntity::InitDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;

	HRESULT hr;
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	// Create a quad for rendering each tree
	if (FAILED(hr = pd3dDevice->CreateVertexBuffer(m_nFrames * 4 * sizeof(SPRITEVERTEX),
		D3DUSAGE_WRITEONLY, SPRITEVERTEX::FVF,
		D3DPOOL_MANAGED, &(m_pSpriteVB), NULL)))
	{
		OUTPUT_LOG("error: D3DXSpriteEntity::InitDeviceObjects CreateVertexBuffer failed\n");
		return hr;
	}

	// Copy quad mesh data into vertex buffer
	SPRITEVERTEX* v;
	m_pSpriteVB->Lock(0, 0, (void**)&v, 0);

	FLOAT width = 1.0f / m_nCol;
	FLOAT height = 1.0f / m_nRow;

	int nLeft = 0, nTop = 0;
	for (int i = 0; i<m_nFrames; i++)
	{
		int idx = i * 4 + 0;
		v[idx].p = Vector3(-0.5f, -0.5f, 0.0f);
		v[idx].tu = nLeft *width;
		v[idx].tv = (nTop + 1) *height;

		idx++;
		v[idx].p = Vector3(0.5f, -0.5f, 0.0f);
		v[idx].tu = width*(nLeft + 1);
		v[idx].tv = (nTop + 1) *height;

		idx++;
		v[idx].p = Vector3(-0.5f, 0.5f, 0.0f);
		v[idx].tu = width*nLeft;
		v[idx].tv = nTop *height;


		idx++;
		v[idx].p = Vector3(0.5f, 0.5f, 0.0f);
		v[idx].tu = width*(nLeft + 1);
		v[idx].tv = nTop *height;

		for (int j = 0; j<4; j++)
		{
			v[i * 4 + j].color = 0xffffffff;
		}

		if ((++nLeft) >= m_nCol)
		{
			nLeft = 0;
			nTop++;
		}
	}

	m_pSpriteVB->Unlock();
	return S_OK;
}


HRESULT D3DXSpriteEntity::DeleteDeviceObjects()
{
	m_bIsInitialized = false;

	SAFE_RELEASE(m_pSpriteVB);
	return S_OK;
}


#endif
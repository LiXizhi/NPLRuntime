//-----------------------------------------------------------------------------
// Class:	MiscEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifndef USE_NULL_RENDERER
#include "MiscEntity.h"
using namespace ParaEngine;

HRESULT D3DXSpriteEntity::InitDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;

	auto* pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice || m_nFrames <= 0 || m_nCol <= 0 || m_nRow <= 0)
		return E_FAIL;

	std::vector<SPRITEVERTEX> verts(static_cast<size_t>(m_nFrames) * 4u);
	SPRITEVERTEX* v = verts.data();

	FLOAT width = 1.0f / m_nCol;
	FLOAT height = 1.0f / m_nRow;

	int nLeft = 0, nTop = 0;
	for (int i = 0; i < m_nFrames; i++)
	{
		int idx = i * 4 + 0;
		v[idx].p = Vector3(-0.5f, -0.5f, 0.0f);
		v[idx].tu = nLeft * width;
		v[idx].tv = (nTop + 1) * height;

		idx++;
		v[idx].p = Vector3(0.5f, -0.5f, 0.0f);
		v[idx].tu = width * (nLeft + 1);
		v[idx].tv = (nTop + 1) * height;

		idx++;
		v[idx].p = Vector3(-0.5f, 0.5f, 0.0f);
		v[idx].tu = width * nLeft;
		v[idx].tv = nTop * height;

		idx++;
		v[idx].p = Vector3(0.5f, 0.5f, 0.0f);
		v[idx].tu = width * (nLeft + 1);
		v[idx].tv = nTop * height;

		for (int j = 0; j < 4; j++)
		{
			v[i * 4 + j].color = 0xffffffff;
		}

		if ((++nLeft) >= m_nCol)
		{
			nLeft = 0;
			nTop++;
		}
	}

	if (m_pSpriteVB)
	{
		m_pSpriteVB->ReleaseRHIResource();
		m_pSpriteVB = nullptr;
	}
	m_pSpriteVB = pRenderDevice->CreateBuffer(
		static_cast<uint32_t>(verts.size() * sizeof(SPRITEVERTEX)),
		false,
		ERHIBufferUsage::Static | ERHIBufferUsage::WriteOnly,
		verts.data());
	if (!m_pSpriteVB)
	{
		OUTPUT_LOG("error: D3DXSpriteEntity::InitDeviceObjects CreateBuffer failed\n");
		return E_FAIL;
	}
	return S_OK;
}


HRESULT D3DXSpriteEntity::DeleteDeviceObjects()
{
	m_bIsInitialized = false;

	if (m_pSpriteVB)
	{
		m_pSpriteVB->ReleaseRHIResource();
		m_pSpriteVB = nullptr;
	}
	return S_OK;
}


#endif
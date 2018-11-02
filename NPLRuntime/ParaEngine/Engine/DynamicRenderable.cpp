//-----------------------------------------------------------------------------
// Class:	CDynamicRenderable
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.27
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "ParaWorldAsset.h"
#include "DynamicRenderable.h"

#include "memdebug.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif

using namespace ParaEngine;

CDynamicRenderable::CDynamicRenderable(void)
{
	m_nFVF = mesh_vertex_normal::FVF;
	m_nVertexBufSize = 0;
	m_nIndexBufSize = 0;
}

CDynamicRenderable::~CDynamicRenderable(void)
{
}

bool ParaEngine::CDynamicRenderable::lock()
{
	// TODO: use a mutex, since our program is in single threaded, we need not do it at the moment. 
	return true;
}

bool ParaEngine::CDynamicRenderable::unlock()
{
	// TODO: use a mutex, since our program is in single threaded, we need not do it at the moment. 
	return true;
}

void ParaEngine::CDynamicRenderable::PrepareBuffers( int nVertexCount, int nIndexCount )
{
	if(m_nFVF == mesh_vertex_normal::FVF)
	{
		m_Positions.resize(nVertexCount);
		m_Normals.resize(nVertexCount);
		m_UVs.resize(nVertexCount);

		m_indices.resize(nIndexCount);

		m_nVertexBufSize = nVertexCount;
		m_nIndexBufSize = nIndexCount;
	}
	else
	{
		m_Positions.resize(0);
		m_Normals.resize(0);
		m_UVs.resize(0);
		m_indices.resize(0);

		m_nVertexBufSize = 0;
		m_nIndexBufSize = 0;
		OUTPUT_LOG("error: Unsupported FVF vertex format in CDynamicRenderable\n");
	}
}

HRESULT ParaEngine::CDynamicRenderable::DrawPrimitive( D3DPRIMITIVETYPE Type, UINT StartVertex, UINT PrimitiveCount )
{
	// currently we only support one type and one vertex format. 
	assert(Type == D3DPT_TRIANGLELIST);
	assert(m_nFVF == mesh_vertex_normal::FVF);

	auto pRenderDevice = CGlobals::GetRenderDevice();
	mesh_vertex_normal* vb_vertices = NULL;

	int nNumLockedVertice = 0;
	int nNumFinishedVertice = 0;
	int indexCount = PrimitiveCount*3;
	DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
	do
	{
		if( (nNumLockedVertice = pBufEntity->Lock((indexCount - nNumFinishedVertice), 
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice/3;

			for(int i=0;i<nLockedNum;++i)
			{
				for(int k=0; k<3; ++k)
				{
					int nVB = 3*i+k;
					vb_vertices[nVB].p = m_Positions[StartVertex+nVB];
					vb_vertices[nVB].n = m_Normals[StartVertex+nVB];
					vb_vertices[nVB].uv = m_UVs[StartVertex+nVB];
				}
			}

			pBufEntity->Unlock();

			CGlobals::GetRenderDevice()->DrawPrimitive(EPrimitiveType::TRIANGLELIST,pBufEntity->m_dwBase,nLockedNum);

			if((indexCount - nNumFinishedVertice) > nNumLockedVertice)
			{
				nNumFinishedVertice += nNumLockedVertice;
			}
			else
				break;
		}
		else 
			break;
	}while(1);

	return S_OK;
}

HRESULT ParaEngine::CDynamicRenderable::DrawIndexedPrimitive( D3DPRIMITIVETYPE Type, INT BaseVertexIndex,UINT MinIndex,UINT NumVertices, UINT indexStart,UINT PrimitiveCount )
{
	// currently we only support one type and one vertex format. 
	assert(Type == D3DPT_TRIANGLELIST);
	assert(m_nFVF == mesh_vertex_normal::FVF);

	auto pRenderDevice = CGlobals::GetRenderDevice();
	mesh_vertex_normal* vb_vertices = NULL;
	
	int nNumLockedVertice = 0;
	int nNumFinishedVertice = 0;
	int indexCount = PrimitiveCount*3;
	DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
	do
	{
		if( (nNumLockedVertice = pBufEntity->Lock((indexCount - nNumFinishedVertice), 
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice/3;

			for(int i=0;i<nLockedNum;++i)
			{
				for(int k=0; k<3; ++k)
				{
					int nVB = 3*i+k;
					unsigned short a = m_indices[indexStart + nNumFinishedVertice+nVB];

					vb_vertices[nVB].p = m_Positions[a];
					vb_vertices[nVB].n = m_Normals[a];
					vb_vertices[nVB].uv = m_UVs[a];
				}
			}

			pBufEntity->Unlock();

			CGlobals::GetRenderDevice()->DrawPrimitive(EPrimitiveType::TRIANGLELIST,pBufEntity->m_dwBase,nLockedNum);

			if((indexCount - nNumFinishedVertice) > nNumLockedVertice)
			{
				nNumFinishedVertice += nNumLockedVertice;
			}
			else
				break;
		}
		else 
			break;
	}while(1);

	return S_OK;
}

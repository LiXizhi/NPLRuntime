//----------------------------------------------------------------------
// Class:	Vertex Buffer Manager
// Authors:	LiXizhi
// Date:	2014.10.8
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "DynamicVertexBufferManager.h"

using namespace ParaEngine;

/**@def how many matrices are there in the matrix buffer for drawing instanced meshes.*/
#define MATRIX_INSTANCE_BUFFER_SIZE		4200

/**@def how many vertices are there in the character buffer for drawing software character meshes.
Quotes:
For static vertex buffers 1-4Mb is a good size to start with, but it might vary depending on the amount of your resources, local video and AGP memory available.
If amount of available video memory is low and a lot of resource swapping is expected, a smaller buffer size is a better choice.
For dynamic buffers you should not allocate buffers bigger than the data streamed to the card per frame. In most cases 256Kb-1Mb dynamic buffers provide a good starting point for performance tweaking.
@NOTE: note that this number must be a multiple of 3
*/
#define MATRIX_CHARACTER_BUFFER_SIZE	30600

ParaEngine::DynamicVertexBufferManager::DynamicVertexBufferManager()
{
	// set the unit size of each type of dynamic buffers.
	m_pDVBuffers[DVB_XYZ_TEX1_NORM_DIF].m_nUnitSize = sizeof(mesh_vertex_normal_color);
	m_pDVBuffers[DVB_XYZ_TEX1_NORM_DIF].m_dwDataFormat = DVB_XYZ_TEX1_NORM_DIF;

	// for characters
	m_pDVBuffers[DVB_XYZ_TEX1_NORM].m_nUnitSize = sizeof(mesh_vertex_normal);
	m_pDVBuffers[DVB_XYZ_TEX1_NORM].m_dwDataFormat = DVB_XYZ_TEX1_NORM;
	m_pDVBuffers[DVB_XYZ_TEX1_NORM].m_dwDiscard = MATRIX_CHARACTER_BUFFER_SIZE;

	m_pDVBuffers[DVB_XYZ_TEX1_DIF].m_nUnitSize = sizeof(SPRITEVERTEX);
	m_pDVBuffers[DVB_XYZ_TEX1_DIF].m_dwDataFormat = DVB_XYZ_TEX1_DIF;

	m_pDVBuffers[DVB_XYZ_NORM_DIF].m_nUnitSize = sizeof(bmax_vertex);
	m_pDVBuffers[DVB_XYZ_NORM_DIF].m_dwDataFormat = DVB_XYZ_NORM_DIF;

	// for mesh instancing
	m_pDVBuffers[DVB_MATRIX].m_nUnitSize = sizeof(MatrixVertex);
	m_pDVBuffers[DVB_MATRIX].m_dwDataFormat = DVB_MATRIX;
	m_pDVBuffers[DVB_MATRIX].m_dwDiscard = MATRIX_INSTANCE_BUFFER_SIZE;

	//m_pDVBuffers[DVB_X].m_nUnitSize = sizeof(float);
	//m_pDVBuffers[DVB_X].m_dwDataFormat = DVB_X;
}

DynamicVertexBufferEntity* ParaEngine::DynamicVertexBufferManager::GetDynamicBuffer(DynamicVBAssetType nBufferType)
{
	return &(m_pDVBuffers[nBufferType]);
}

HRESULT ParaEngine::DynamicVertexBufferManager::RestoreDeviceObjects()
{
	for (int i = 0; i < DVB_COUNT; i++)
	{
		m_pDVBuffers[i].RestoreDeviceObjects();
	}
	return S_OK;
}

HRESULT ParaEngine::DynamicVertexBufferManager::InvalidateDeviceObjects()
{
	for (int i = 0; i < DVB_COUNT; i++)
	{
		m_pDVBuffers[i].InvalidateDeviceObjects();
	}
	return S_OK;
}

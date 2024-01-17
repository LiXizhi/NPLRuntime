//-----------------------------------------------------------------------------
// Class:	ParaVoxelModel
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2024.1.17
// local obj = entity:GetInnerObject():GetPrimaryAsset():GetAttributeObject():GetChildAt(0):GetChild("VoxelModel")
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "DynamicAttributeField.h"
#include "ParaXModel.h"
#include "effect_file.h"
#include "StringHelper.h"
#include "ParaVoxelModel.h"

using namespace ParaEngine;

ParaVoxelModel::ParaVoxelModel()
{
	m_chunks.push_back(VoxelChunk(1));
	m_pRootNode = &(m_chunks[0][0]);
}

ParaVoxelModel::~ParaVoxelModel()
{
	m_chunks.clear();
}

int ParaEngine::ParaVoxelModel::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	return S_OK;
}


bool ParaVoxelModel::Load(const char* pBuffer, int nCount)
{
	return false;
}

bool ParaVoxelModel::Save(std::vector<char>& output)
{
	return false;
}

void ParaVoxelModel::SetBlock(int x, int y, int z, int level, int color)
{
}

int ParaVoxelModel::GetBlock(int x, int y, int z, int level)
{
	return 0;
}

bool ParaVoxelModel::RayPicking(const Vector3& origin, const Vector3& dir, Vector3& hitPos, int& hitColor, int level)
{
	return false;
}

void ParaVoxelModel::Optimize()
{
}

void ParaVoxelModel::Draw(SceneState* pSceneState)
{
	CBaseObject* pBaseObj = pSceneState->GetCurrentSceneObject();
	if (pBaseObj != NULL) pBaseObj->ApplyMaterial();


	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	int start = 0;
	int indexCount = 0;

	bmax_vertex* vb_vertices = NULL;
	bmax_vertex* ov = NULL;
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;
	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_NORM_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(bmax_vertex));

	do
	{
		indexCount = 3;
		if ((nNumLockedVertice = pBufEntity->Lock((indexCount - nNumFinishedVertice),
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice / 3;

			bmax_vertex origVertices[] = {
				{ Vector3(1, 0.5, 0), Vector3(0, 1, 0), 0xffffffff },
				{ Vector3(0, 0.5, 1), Vector3(0, 1, 0), 0xffffffff },
				{ Vector3(1, 0.5, 1), Vector3(0, 1, 0), 0xffffffff }
			};
			
			for (int i = 0; i < nLockedNum; ++i)
			{
				int nVB = 3 * i;
				for (int k = 0; k < 3; ++k, ++nVB)
				{
					uint16 a = k;
					bmax_vertex& out_vertex = vb_vertices[nVB];
					ov = origVertices + a;
					out_vertex.p = ov->p;
					out_vertex.n = ov->n;
					out_vertex.color = ov->color;
				}
			}
			
			pBufEntity->Unlock();

			if (pBufEntity->IsMemoryBuffer())
				RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
			else
				RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

			if ((indexCount - nNumFinishedVertice) > nNumLockedVertice)
			{
				nNumFinishedVertice += nNumLockedVertice;
			}
			else
				break;
		}
		else
			break;
	} while (1);
}
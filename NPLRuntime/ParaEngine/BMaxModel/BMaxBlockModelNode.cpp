//-----------------------------------------------------------------------------
// Class:Block max frame node
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date:	2015.10.9
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockDirection.h"
#include "BlockEngine/BlockWorldClient.h"
#include "ParaWorldAsset.h"
#include "BMaxParser.h"
#include "BMaxBlockModelNode.h"
using namespace ParaEngine;

ParaEngine::BMaxBlockModelNode::BMaxBlockModelNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_)
	:BMaxNode(pParser, x_, y_, z_, template_id_, block_data_), m_fFacing(0.f), bHasTransform(false)
{
	matLocalTrans.identity();
}

ParaEngine::BMaxBlockModelNode::~BMaxBlockModelNode()
{

}

const std::string& ParaEngine::BMaxBlockModelNode::GetFilename() const
{
	return m_sFilename;
}

void ParaEngine::BMaxBlockModelNode::SetFilename(const std::string& val)
{
	m_sFilename = val;
}

bool ParaEngine::BMaxBlockModelNode::HasTransform()
{
	return bHasTransform;
}

ParaEngine::Matrix4 ParaEngine::BMaxBlockModelNode::GetTransform()
{
	return matLocalTrans;
}

void ParaEngine::BMaxBlockModelNode::SetTransform(Matrix4& mat)
{
	bHasTransform = true;
	matLocalTrans = mat;
}

float ParaEngine::BMaxBlockModelNode::GetFacing() const
{
	return m_fFacing;
}

void ParaEngine::BMaxBlockModelNode::SetFacing(float val)
{
	m_fFacing = val;
}

bool ParaEngine::BMaxBlockModelNode::isSolid()
{
	return false;
}

int ParaEngine::BMaxBlockModelNode::TessellateBlock(BlockModel* tessellatedModel)
{
	if (m_sFilename.empty())
	return 0;
	
	CParaXModel * pModel = m_pParser->CreateGetRefModel(m_sFilename);
	if (pModel)
	{
		tessellatedModel->ClearVertices();
		m_pParaXModel = pModel;
		return tessellatedModel->GetVerticesCount();
	}
	return 0;

	// if (m_sFilename.empty())
	// 	return 0;
	
	// CParaXModel * pModel = m_pParser->CreateGetRefModel(m_sFilename);
	// if (pModel)
	// {
	// 	tessellatedModel->ClearVertices();
	// 	BlockVertexCompressed v;
	// 	memset(&v, 0, sizeof(v));

	// 	Matrix4 matLocalTrans;
	// 	bool m_bHasTransform = false;
	// 	if (m_fFacing != 0.f)
	// 	{
	// 		matLocalTrans.makeRot(Quaternion(Vector3::UNIT_Y, m_fFacing), Vector3::ZERO);
	// 		m_bHasTransform = true;
	// 	}
	// 	int nPasses = (int)pModel->passes.size();
	// 	int nTotalFaceCount = 0;
		
	// 	const Vector3& vCenter = m_pParser->GetCenterPos();
	// 	Vector3 vOffset((float)x - vCenter.x + BlockConfig::g_half_blockSize, (float)y, (float)z - vCenter.z + BlockConfig::g_half_blockSize);
		
	// 	for (int nPass = 0; nPass < nPasses; nPass++)
	// 	{
	// 		ModelRenderPass &p = pModel->passes[nPass];
	// 		if (pModel->showGeosets[p.geoset])
	// 		{
	// 			ModelVertex *ov = pModel->m_origVertices;
				
	// 			int nIndexCount = p.indexCount;
	// 			int nIndexOffset = p.indexStart;
	// 			// tricky here: we will assume 6 indices to be a single rectangle face in bmax model. 
	// 			int nFaceCount = nIndexCount / 6;
	// 			nTotalFaceCount += nFaceCount;
	// 			tessellatedModel->ReserveVertices(nTotalFaceCount*4);
	// 			for (int i = 0; i < nFaceCount; ++i)
	// 			{
	// 				for (int k = 0; k < 4;++k)
	// 				{
	// 					int nVB = i * 6 + k;
	// 					if (k == 3)
	// 						nVB += 2;
	// 					uint16 a = pModel->m_indices[nIndexOffset + nVB];
	// 					ov = pModel->m_origVertices + a;
	// 					if (m_bHasTransform)
	// 					{
	// 						v.SetPosition(ov->pos*matLocalTrans + vOffset);
	// 						v.SetNormal(ov->normal*matLocalTrans);
	// 					}
	// 					else
	// 					{
	// 						v.SetPosition(ov->pos + vOffset);
	// 						v.SetNormal(ov->normal);
	// 					}

	// 					v.SetTexcoord(ov->texcoords);
	// 					v.SetBlockColor(ov->color0);
	// 					tessellatedModel->AddVertex(v);
	// 				}
	// 			}
	// 		}
	// 	}
	// 	tessellatedModel->SetFaceCount(nTotalFaceCount);

	// 	return tessellatedModel->GetVerticesCount();
	// }
	// return 0;
}

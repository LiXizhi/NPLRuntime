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
	:BMaxNode(pParser, x_, y_, z_, template_id_, block_data_), bHasTransform(false), m_pParaXModel(NULL)
{
	m_bIsSolid = false;
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

CParaXModel* ParaEngine::BMaxBlockModelNode::GetParaXModel() 
{
	return m_pParaXModel;
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
}

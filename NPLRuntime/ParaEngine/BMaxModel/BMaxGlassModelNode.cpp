#include "ParaEngine.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockDirection.h"
#include "BlockEngine/BlockWorldClient.h"
#include "ParaWorldAsset.h"
#include "BMaxParser.h"
#include "BMaxGlassModelNode.h"
using namespace ParaEngine;

ParaEngine::BMaxGlassModelNode::BMaxGlassModelNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_)
	:BMaxNode(pParser, x_, y_, z_, template_id_, block_data_)
{
}

ParaEngine::BMaxGlassModelNode::~BMaxGlassModelNode()
{

}

int ParaEngine::BMaxGlassModelNode::TessellateBlock(BlockModel* tessellatedModel)
{
	int bone_index = GetBoneIndex();


	auto* pBlockWorld = BlockWorldClient::GetInstance();
	BlockTemplate* block_template = pBlockWorld ? pBlockWorld->GetBlockTemplate((uint16)template_id) : nullptr;
	if (!block_template)
		return tessellatedModel->GetVerticesCount();

	// glass models

	tessellatedModel->ClearVertices();
	BlockModel& blockModel = block_template->GetBlockModelByData(this->block_data);
	DWORD dwBlockColor = GetColor();
	//tessellatedModel->SetUniformLighting(true);
	tessellatedModel->CloneVertices(blockModel);

	BlockVertexCompressed* pVertices = tessellatedModel->GetVertices();
	int count = tessellatedModel->GetVerticesCount();
	const Vector3& vCenter = m_pParser->GetCenterPos();
	Vector3 vOffset((float)x - vCenter.x, (float)y, (float)z - vCenter.z);
	for (int k = 0; k < count; k++)
	{
		pVertices[k].OffsetPosition(vOffset);
		pVertices[k].SetBlockColor(dwBlockColor);
	}


	return tessellatedModel->GetVerticesCount();
}
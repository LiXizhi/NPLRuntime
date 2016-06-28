#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "ShapeAABB.h"
#include "SlopeModelProvider.h"
using namespace ParaEngine;
ParaEngine::CSlopeModelProvider::CSlopeModelProvider(BlockTemplate* pBlockTemplate)
	:CLinearModelProvider(pBlockTemplate, sizeof(mEdgeBlockModels) / sizeof(mEdgeBlockModels[0]) + sizeof(mCornerBlockModels) / sizeof(mCornerBlockModels[0]))
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(6);
	int block_index = 0;
	for (auto& model : mEdgeBlockModels){
		model.ClearVertices();
	}
	for (auto& model : mCornerBlockModels){
		model.ClearVertices();
	}
	/*
	                        /|
						   / |
						  /  |
						 /___|
	*/
	for (int i = 0; i < 4; ++i)
	{
		mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_btmLB + i);
	}

	for (int i = 0; i < 4; ++i)
	{
		mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightLB + i);
	}

	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightLT);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightRT);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_btmLB);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_btmRB);

	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkLT);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkLB);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkRB);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkRB);

	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtRT);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtLB);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtRB);
	mEdgeBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtRB);
	++block_index;
	/*
	                     |\
						 | \
						 |  \
						 |___\
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[0] = 1.0f - vertex.position[0];
		vertex.normal[0] = -vertex.normal[0];
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                      ____
						 |   /
						 |  /
						 | /
						 |/
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[1] = 1.0f - vertex.position[1];
		vertex.normal[1] = -vertex.normal[1];
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                  ____ 
					  \	  |   
					   \  |  
						\ | 
						 \|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[0] = 1.0f - vertex.position[0];
		vertex.normal[0] = -vertex.normal[0];
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
			_______
		   | \___/ |
		   | /   \ |
		   |/_____\|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		float x = vertex.position[0] - 1.0f;
		float z = vertex.position[2];
		float nor_x = vertex.normal[0];
		float nor_z = vertex.normal[2];
		vertex.position[0] = z;
		vertex.position[2] = -x;
		vertex.normal[0] = nor_z;
		vertex.normal[2] = -nor_x;
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
			_______
		   |  ___  |
		   |       |
		   |_______|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[2] = 1.0f - vertex.position[2];
		vertex.normal[2] = -vertex.normal[2];
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
			_______
		   |       |
		   |  ___  |
		   |_______|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[1] = 1.0f - vertex.position[1];
		vertex.normal[1] = -vertex.normal[1];
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
			_______
		   |\     /|
		   | \___/ |
		   |/_____\|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mEdgeBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[2] = 1.0f - vertex.position[2];
		vertex.normal[2] = -vertex.normal[2];
		mEdgeBlockModels[block_index].AddVertex(vertex);
	}
	block_index=0;
	//90degree is inner
	/*
	                        /|
						   / |
						  /__|
						 /___|
	*/
	for (int i = 0; i < 4; ++i)
	{
		mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_btmLB + i);
	}

	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightRT);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightLB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightRB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightRB);

	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_rightRT);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_leftLB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_leftRB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_leftRB);

	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkLT);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkLB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkRB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_bkRB);

	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtRT);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtLB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtRB);
	mCornerBlockModels[block_index].AddVertex(cube_mode, BlockModel::g_frtRB);
	++block_index;
	/*
	                     |\
						 | \
						 |__\
						 |___\
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[0] = 1.0f - vertex.position[0];
		vertex.normal[0] = -vertex.normal[0];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                      ____
						 |___/
						 |  /
						 | /
						 |/
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[1] = 1.0f - vertex.position[1];
		vertex.normal[1] = -vertex.normal[1];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                  ____ 
					  \___|   
					   \  |  
						\ | 
						 \|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[0] = 1.0f - vertex.position[0];
		vertex.normal[0] = -vertex.normal[0];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	//90degree is outer
	/*
	                  ____ 
					  \___|   
					   \  |  
						\ | 
						 \|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[2] = 1.0f - vertex.position[2];
		vertex.normal[2] = -vertex.normal[2];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                      ____
						 |___/
						 |  /
						 | /
						 |/
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[0] = 1.0f - vertex.position[0];
		vertex.normal[0] = -vertex.normal[0];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                     |\
						 | \
						 |__\
						 |___\
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[1] = 1.0f - vertex.position[1];
		vertex.normal[1] = -vertex.normal[1];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	++block_index;
	/*
	                        /|
						   / |
						  /__|
						 /___|
	*/
	for (int i = 0; i < 24; ++i)
	{
		BlockVertexCompressed vertex = mCornerBlockModels[block_index - 1].GetVertices()[i];
		vertex.position[0] = 1.0f - vertex.position[0];
		vertex.normal[0] = -vertex.normal[0];
		mCornerBlockModels[block_index].AddVertex(vertex);
	}
	block_index=0;
	
	for (auto& model : mEdgeBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
	}
	for (auto& model : mCornerBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
	}

	mCubeBlockModel = cube_mode;
}

ParaEngine::CSlopeModelProvider::~CSlopeModelProvider()
{}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	const int type_index = nIndex / 8;
	const int block_index = nIndex % 8;
	BlockModel * model = &mCubeBlockModel;
	switch (type_index)
	{
	case 0:
		model = mEdgeBlockModels;
		break;
	case 1:
		model = mCornerBlockModels;
		break;
	}
	return (nIndex<m_nModelCount) ? model[block_index] : model[0];
}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks /*= NULL*/)
{
	const int type_index = nBlockData / 8;
	const int block_index = nBlockData % 8;
	BlockModel * model = &mCubeBlockModel;
	switch (type_index)
	{
	case 0:
		model = mEdgeBlockModels;
		break;
	case 1:
		model = mCornerBlockModels;
		break;
	}
	return (nBlockData<m_nModelCount) ? model[block_index] : model[0];
}

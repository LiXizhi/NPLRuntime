#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "ShapeAABB.h"
#include "SlopeModelProvider.h"
using namespace ParaEngine;
ParaEngine::CSlopeModelProvider::CSlopeModelProvider(BlockTemplate* pBlockTemplate)
	:CLinearModelProvider(pBlockTemplate, sizeof(mEdgeBlockModels) / sizeof(mEdgeBlockModels[0]) + sizeof(mOuterCornerBlockModels) / sizeof(mOuterCornerBlockModels[0]) + sizeof(mInnerCornerBlockModels) / sizeof(mInnerCornerBlockModels[0]))
{
	_buildEdgeBlockModels();
	_builOuterCornerBlockModels();
	_buildInnerCornerBlockModels();
}

ParaEngine::CSlopeModelProvider::~CSlopeModelProvider()
{}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	const int type_index = nIndex / 8;
	const int block_index = nIndex % 8;
	BlockModel * model = mEdgeBlockModels;
	switch (type_index)
	{
	case 0:
		model = mEdgeBlockModels;
		break;
	case 1:
		model = mOuterCornerBlockModels;
		break;
	case 2:
		model = mInnerCornerBlockModels;
		break;
	}
	return (nIndex<m_nModelCount) ? model[block_index] : model[0];
}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks /*= NULL*/)
{
	const int type_index = nBlockData / 8;
	const int block_index = nBlockData % 8;
	BlockModel * model = mEdgeBlockModels;
	switch (type_index)
	{
	case 0:
		model = mEdgeBlockModels;
		break;
	case 1:
		model = mOuterCornerBlockModels;
		break;
	case 2:
		model = mInnerCornerBlockModels;
		break;
	}
	return (nBlockData<m_nModelCount) ? model[block_index] : model[0];
}

void ParaEngine::CSlopeModelProvider::_buildEdgeBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int block_index = 0;
	for (auto& model : mEdgeBlockModels){
		model.ClearVertices();
	}
	/*
	/|
	/ |
	/  |
	/___|
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topLT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftLB].position[0], cube_mode.Vertices()[BlockModel::g_leftLB].position[1], cube_mode.Vertices()[BlockModel::g_leftLB].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topLB].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRB].position[0], cube_mode.Vertices()[BlockModel::g_leftRB].position[1], cube_mode.Vertices()[BlockModel::g_leftRB].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i] = cube_mode.Vertices()[BlockModel::g_leftLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtLT] = cube_mode.Vertices()[BlockModel::g_frtLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkRT] = cube_mode.Vertices()[BlockModel::g_bkRB];
	++block_index;
	/*
	|\
	| \
	|  \
	|___\
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRB].position[0], cube_mode.Vertices()[BlockModel::g_rightRB].position[1], cube_mode.Vertices()[BlockModel::g_rightRB].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topRB].SetPosition(cube_mode.Vertices()[BlockModel::g_rightLB].position[0], cube_mode.Vertices()[BlockModel::g_rightLB].position[1], cube_mode.Vertices()[BlockModel::g_rightLB].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_rightLB + i] = cube_mode.Vertices()[BlockModel::g_rightLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtRT] = cube_mode.Vertices()[BlockModel::g_frtRB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkLT] = cube_mode.Vertices()[BlockModel::g_bkLB];
	++block_index;
	/*
	____
	|   /
	|  /
	| /
	|/
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmRB].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_rightLB + i] = cube_mode.Vertices()[BlockModel::g_rightLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtRB] = cube_mode.Vertices()[BlockModel::g_frtRT];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkLB] = cube_mode.Vertices()[BlockModel::g_bkLT];
	++block_index;
	/*
	____
	\	  |
	\  |
	\ |
	\|
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLT].position[0], cube_mode.Vertices()[BlockModel::g_frtLT].position[1], cube_mode.Vertices()[BlockModel::g_frtLT].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmLB].SetPosition(cube_mode.Vertices()[BlockModel::g_leftLT].position[0], cube_mode.Vertices()[BlockModel::g_leftLT].position[1], cube_mode.Vertices()[BlockModel::g_leftLT].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i] = cube_mode.Vertices()[BlockModel::g_leftLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtLB] = cube_mode.Vertices()[BlockModel::g_frtLT];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkRB] = cube_mode.Vertices()[BlockModel::g_bkRT];
	++block_index;
	/*
	_______
	| \___/ |
	| /   \ |
	|/_____\|
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmLB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRT].position[0], cube_mode.Vertices()[BlockModel::g_bkRT].position[1], cube_mode.Vertices()[BlockModel::g_bkRT].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmRB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLT].position[0], cube_mode.Vertices()[BlockModel::g_bkLT].position[1], cube_mode.Vertices()[BlockModel::g_bkLT].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i] = cube_mode.Vertices()[BlockModel::g_bkLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLB] = cube_mode.Vertices()[BlockModel::g_leftLT];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_rightRB] = cube_mode.Vertices()[BlockModel::g_rightRT];
	++block_index;
	/*
	_______
	|  ___  |
	|       |
	|_______|
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLT].position[0], cube_mode.Vertices()[BlockModel::g_frtLT].position[1], cube_mode.Vertices()[BlockModel::g_frtLT].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i] = cube_mode.Vertices()[BlockModel::g_frtLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftRB] = cube_mode.Vertices()[BlockModel::g_leftRT];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_rightLB] = cube_mode.Vertices()[BlockModel::g_rightLT];
	++block_index;
	/*
	_______
	|       |
	|  ___  |
	|_______|
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLB].position[0], cube_mode.Vertices()[BlockModel::g_frtLB].position[1], cube_mode.Vertices()[BlockModel::g_frtLB].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i] = cube_mode.Vertices()[BlockModel::g_frtLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftRT] = cube_mode.Vertices()[BlockModel::g_leftRB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_rightLT] = cube_mode.Vertices()[BlockModel::g_rightLB];
	++block_index;
	/*
	_______
	|\     /|
	| \___/ |
	|/_____\|
	*/
	mEdgeBlockModels[block_index] = cube_mode;
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topLT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRB].position[0], cube_mode.Vertices()[BlockModel::g_bkRB].position[1], cube_mode.Vertices()[BlockModel::g_bkRB].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topRT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLB].position[0], cube_mode.Vertices()[BlockModel::g_bkLB].position[1], cube_mode.Vertices()[BlockModel::g_bkLB].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i] = cube_mode.Vertices()[BlockModel::g_bkLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLT] = cube_mode.Vertices()[BlockModel::g_leftLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_rightRT] = cube_mode.Vertices()[BlockModel::g_rightRB];
	++block_index;

	for (auto& model : mEdgeBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_builOuterCornerBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int block_index = 0;
	for (auto& model : mOuterCornerBlockModels){
		model.ClearVertices();
	}
	//90degree is inner
	/*
	/|
	/ |
	/__|
	/___|
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	++block_index;
	//90degree is outer
	/*
	|\
	| \
	|__\
	|___\
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_leftRT].position[0], cube_mode.Vertices()[BlockModel::g_leftRT].position[1], cube_mode.Vertices()[BlockModel::g_leftRT].position[2]);
	++block_index;
	//90degree is inner
	/*
	____
	|___/
	|  /
	| /
	|/
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_btmLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLB].position[0], cube_mode.Vertices()[BlockModel::g_btmLB].position[1], cube_mode.Vertices()[BlockModel::g_btmLB].position[2]);
	++block_index;
	//90degree is outer
	/*
	____
	\___|
	\  |
	\ |
	\|
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_btmLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	++block_index;

	//90degree is outer
	/*
	/|
	/ |
	/__|
	/___|
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	++block_index;
	//90degree is inner
	/*
	|\
	| \
	|__\
	|___\
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLT].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRT].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLT].SetPosition(cube_mode.Vertices()[BlockModel::g_topLT].position[0], cube_mode.Vertices()[BlockModel::g_topLT].position[1], cube_mode.Vertices()[BlockModel::g_topLT].position[2]);
	++block_index;
	//90degree is outer
	/*
	____
	|___/
	|  /
	| /
	|/
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_btmLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmLT].position[0], cube_mode.Vertices()[BlockModel::g_btmLT].position[1], cube_mode.Vertices()[BlockModel::g_btmLT].position[2]);
	++block_index;
	//90degree is inner
	/*
	____
	\___|
	\  |
	\ |
	\|
	*/
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_btmLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_btmLB].normal[0], cube_mode.Vertices()[BlockModel::g_btmLB].normal[1], cube_mode.Vertices()[BlockModel::g_btmLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRB].SetPosition(cube_mode.Vertices()[BlockModel::g_btmRB].position[0], cube_mode.Vertices()[BlockModel::g_btmRB].position[1], cube_mode.Vertices()[BlockModel::g_btmRB].position[2]);
	++block_index;

	for (auto& model : mOuterCornerBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_buildInnerCornerBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int block_index = 0;
	for (auto& model : mInnerCornerBlockModels){
		model.ClearVertices();
	}
	//shift FRT to FRB
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_topRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	++block_index;
	//shift FRB to FRT
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_btmRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRT].position[0], cube_mode.Vertices()[BlockModel::g_frtRT].position[1], cube_mode.Vertices()[BlockModel::g_frtRT].position[2]);
	++block_index;
	//shift FLT to FLB
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLB].position[0], cube_mode.Vertices()[BlockModel::g_frtLB].position[1], cube_mode.Vertices()[BlockModel::g_frtLB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLB].position[0], cube_mode.Vertices()[BlockModel::g_frtLB].position[1], cube_mode.Vertices()[BlockModel::g_frtLB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLB].position[0], cube_mode.Vertices()[BlockModel::g_frtLB].position[1], cube_mode.Vertices()[BlockModel::g_frtLB].position[2]);
	++block_index;
	//shift FLB to FLT
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLT].position[0], cube_mode.Vertices()[BlockModel::g_frtLT].position[1], cube_mode.Vertices()[BlockModel::g_frtLT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_btmLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLT].position[0], cube_mode.Vertices()[BlockModel::g_frtLT].position[1], cube_mode.Vertices()[BlockModel::g_frtLT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtLT].position[0], cube_mode.Vertices()[BlockModel::g_frtLT].position[1], cube_mode.Vertices()[BlockModel::g_frtLT].position[2]);
	++block_index;
	//shift BRT to BRB
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRB].position[0], cube_mode.Vertices()[BlockModel::g_bkRB].position[1], cube_mode.Vertices()[BlockModel::g_bkRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_topLT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRB].position[0], cube_mode.Vertices()[BlockModel::g_bkRB].position[1], cube_mode.Vertices()[BlockModel::g_bkRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRB].position[0], cube_mode.Vertices()[BlockModel::g_bkRB].position[1], cube_mode.Vertices()[BlockModel::g_bkRB].position[2]);
	++block_index;
	//shift BRB to BRT
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRT].position[0], cube_mode.Vertices()[BlockModel::g_bkRT].position[1], cube_mode.Vertices()[BlockModel::g_bkRT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_btmLB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRT].position[0], cube_mode.Vertices()[BlockModel::g_bkRT].position[1], cube_mode.Vertices()[BlockModel::g_bkRT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkRT].position[0], cube_mode.Vertices()[BlockModel::g_bkRT].position[1], cube_mode.Vertices()[BlockModel::g_bkRT].position[2]);
	++block_index;
	//shift BLT to BLB
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLB].position[0], cube_mode.Vertices()[BlockModel::g_bkLB].position[1], cube_mode.Vertices()[BlockModel::g_bkLB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_topRT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLB].position[0], cube_mode.Vertices()[BlockModel::g_bkLB].position[1], cube_mode.Vertices()[BlockModel::g_bkLB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRT].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLB].position[0], cube_mode.Vertices()[BlockModel::g_bkLB].position[1], cube_mode.Vertices()[BlockModel::g_bkLB].position[2]);
	++block_index;
	//shift BLB to BLT
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_bkLB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLT].position[0], cube_mode.Vertices()[BlockModel::g_bkLT].position[1], cube_mode.Vertices()[BlockModel::g_bkLT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_btmRB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLT].position[0], cube_mode.Vertices()[BlockModel::g_bkLT].position[1], cube_mode.Vertices()[BlockModel::g_bkLT].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_rightRB].SetPosition(cube_mode.Vertices()[BlockModel::g_bkLT].position[0], cube_mode.Vertices()[BlockModel::g_bkLT].position[1], cube_mode.Vertices()[BlockModel::g_bkLT].position[2]);
	++block_index;

	for (auto& model : mInnerCornerBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}
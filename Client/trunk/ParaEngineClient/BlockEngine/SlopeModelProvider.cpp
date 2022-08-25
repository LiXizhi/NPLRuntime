#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "ShapeAABB.h"
#include "SlopeModelProvider.h"
using namespace ParaEngine;
ParaEngine::CSlopeModelProvider::CSlopeModelProvider(BlockTemplate* pBlockTemplate)
	:CLinearModelProvider(pBlockTemplate, sizeof(mEdgeBlockModels) / sizeof(mEdgeBlockModels[0]) + sizeof(mOuterCornerBlockModels) / sizeof(mOuterCornerBlockModels[0]) + sizeof(mInnerCornerBlockModels) / sizeof(mInnerCornerBlockModels[0]) +
		sizeof(mHEdgeBlockModels) / sizeof(mHEdgeBlockModels[0]) + sizeof(mHOuterCornerBlockModels) / sizeof(mHOuterCornerBlockModels[0]) + sizeof(mHInnerCornerBlockModels) / sizeof(mHInnerCornerBlockModels[0]))
{
	_buildEdgeBlockModels();
	_builOuterCornerBlockModels();
	_buildInnerCornerBlockModels();
	_buildHEdgeBlockModels();
	_buildHOuterCornerBlockModels();
	_buildHInnerCornerBlockModels();
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
	case 3:
		model = mHEdgeBlockModels;
		break;
	case 4:
		model = mHOuterCornerBlockModels;
		break;
	case 5:
		model = mHInnerCornerBlockModels;
		break;
	}
	return (nIndex<m_nModelCount) ? model[block_index] : model[0];
}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks /*= NULL*/)
{
	nBlockData = nBlockData & 0xff;

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
	case 3:
		model = mHEdgeBlockModels;
		break;
	case 4:
		model = mHOuterCornerBlockModels;
		break;
	case 5:
		model = mHInnerCornerBlockModels;
		break;
	}
	return (nBlockData<m_nModelCount) ? model[block_index] : model[0];
}

void ParaEngine::CSlopeModelProvider::_buildEdgeBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int block_index = 0;
	for (auto& model : mEdgeBlockModels) {
		model.ClearVertices();
	}
	
	mEdgeBlockModels[block_index] = cube_mode;
	std::vector<BlockVertexCompressed> _vertices = mEdgeBlockModels[block_index].Vertices();
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightLT].position[0], cube_mode.Vertices()[BlockModel::g_rightLT].position[1], cube_mode.Vertices()[BlockModel::g_rightLT].position[2]);
	for (int i = 0; i < 4; ++i)
		mEdgeBlockModels[block_index].Vertices()[BlockModel::g_topLB + i] = cube_mode.Vertices()[BlockModel::g_topLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_frtLT] = cube_mode.Vertices()[BlockModel::g_frtLB];
	mEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkRT] = cube_mode.Vertices()[BlockModel::g_bkRB];

	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(0, 3.14, 0),
		Vector3(3.14, 3.14, 0),
		Vector3(3.14, 0, 0),
		Vector3(3.14, 1.57, 0),
		Vector3(3.14, -1.57, 0),
		Vector3(0, -1.57, 0),
		Vector3(0, 1.57, 0)
	};
	auto _models = mEdgeBlockModels;

	BlockModel tempModel;
	tempModel.Clone(_models[block_index]);
	for (int i = 1; i < 8; i++) {
		Vector3 angles = angleArr[i];
		int block_index = i;
		_models[block_index] = cube_mode;
		for (int i = 0; i < 24; i++) {
			Vector3 pt = Vector3(tempModel.Vertices()[i].position[0], tempModel.Vertices()[i].position[1], tempModel.Vertices()[i].position[2]);
			Vector3 newPt = vec3RotateByPoint(Vector3(0.5, 0.5, 0.5), Vector3(pt.x, pt.y, pt.z), angles);
			_models[block_index].Vertices()[i].SetPosition(newPt.x, newPt.y, newPt.z);

			//OUTPUT_LOG("idx:%d ,pt_0_1:(%f,%f,%f)==(%f,%f,%f) ? %s\n\n", i, pt.x, pt.y, pt.z, newPt.x, newPt.y, newPt.z, (abs(pt.x - newPt.x) < 0.0000001 && abs(pt.y - newPt.y) < 0.0000001 && abs(pt.z - newPt.z) < 0.0000001) ? "true" : "false");
		}
	}

	for (auto& model : mEdgeBlockModels) {
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
	for (auto& model : mOuterCornerBlockModels) {
		model.ClearVertices();
	}
	
	mOuterCornerBlockModels[block_index] = cube_mode;
	for (int i = 0; i < 4; ++i)
	{
		mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
		//mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
		//mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
	}
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_bkRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	mOuterCornerBlockModels[block_index].Vertices()[BlockModel::g_frtLT].SetTexcoord(cube_mode.Vertices()[BlockModel::g_frtRT].texcoord[0], cube_mode.Vertices()[BlockModel::g_frtRT].texcoord[1]);
	
	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(0,3.14,0),
		Vector3(3.14,3.14,0),
		Vector3(3.14,0,0),
		Vector3(0,1.57,0),
		Vector3(0,-1.57,0),
		Vector3(3.14,1.57,0),
		Vector3(3.14,-1.57,0)
	};
	auto _models = mOuterCornerBlockModels;

	BlockModel tempModel;
	tempModel.Clone(_models[block_index]);
	for (int i = 1; i < 8; i++) {
		Vector3 angles = angleArr[i];
		int block_index = i;
		_models[block_index] = cube_mode;
		for (int i = 0; i < 24; i++) {
			Vector3 pt = Vector3(tempModel.Vertices()[i].position[0], tempModel.Vertices()[i].position[1], tempModel.Vertices()[i].position[2]);
			Vector3 newPt = vec3RotateByPoint(Vector3(0.5, 0.5, 0.5), Vector3(pt.x, pt.y, pt.z), angles);
			_models[block_index].Vertices()[i].SetPosition(newPt.x, newPt.y, newPt.z);

			//OUTPUT_LOG("idx:%d ,pt_0_1:(%f,%f,%f)==(%f,%f,%f) ? %s\n\n", i, pt.x, pt.y, pt.z, newPt.x, newPt.y, newPt.z, (abs(pt.x - newPt.x) < 0.0000001 && abs(pt.y - newPt.y) < 0.0000001 && abs(pt.z - newPt.z) < 0.0000001) ? "true" : "false");
		}
	}

	for (auto& model : mOuterCornerBlockModels) {
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
	for (auto& model : mInnerCornerBlockModels) {
		model.ClearVertices();
	}
	//shift FRT to FRB
	mInnerCornerBlockModels[block_index] = cube_mode;
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_topRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	mInnerCornerBlockModels[block_index].Vertices()[BlockModel::g_topLB].SetTexcoord(cube_mode.Vertices()[BlockModel::g_topRT].texcoord[0], cube_mode.Vertices()[BlockModel::g_topRT].texcoord[1]);

	//旧的 2-1和2-5明显不是内凸的，这里改了，应该没多少地方用到
	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(3.14,1.57,0),
		Vector3(0,1.57,0),
		Vector3(3.14,3.14,0),
		Vector3(0,3.14,0),
		Vector3(3.14,-1.57,0),
		Vector3(0,-1.57,0),
		Vector3(3.14,0,0)
	};

	BlockModel tempModel;
	tempModel.Clone(mInnerCornerBlockModels[block_index]);
	for (int i = 1; i < 8; i++) {
		Vector3 angles = angleArr[i];
		int block_index = i;
		mInnerCornerBlockModels[block_index] = cube_mode;
		for (int i = 0; i < 24; i++) {
			Vector3 pt = Vector3(tempModel.Vertices()[i].position[0], tempModel.Vertices()[i].position[1], tempModel.Vertices()[i].position[2]);
			Vector3 newPt = vec3RotateByPoint(Vector3(0.5, 0.5, 0.5), Vector3(pt.x, pt.y, pt.z), angles);
			mInnerCornerBlockModels[block_index].Vertices()[i].SetPosition(newPt.x, newPt.y, newPt.z);

			//OUTPUT_LOG("idx:%d ,pt_0_1:(%f,%f,%f)==(%f,%f,%f) ? %s\n\n", i, pt.x, pt.y, pt.z, newPt.x, newPt.y, newPt.z, (abs(pt.x - newPt.x) < 0.0000001 && abs(pt.y - newPt.y) < 0.0000001 && abs(pt.z - newPt.z) < 0.0000001) ? "true" : "false");
		}
	}

	for (auto& model : mInnerCornerBlockModels) {
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_buildHEdgeBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int block_index = 0;
	for (auto& model : mHEdgeBlockModels){
		model.ClearVertices();
	}
	
	mHEdgeBlockModels[block_index] = cube_mode;
	mHEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLB].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRB].position[0], cube_mode.Vertices()[BlockModel::g_rightRB].position[1], cube_mode.Vertices()[BlockModel::g_rightRB].position[2]);
	mHEdgeBlockModels[block_index].Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	for (int i = 0; i < 4; ++i)
		mHEdgeBlockModels[block_index].Vertices()[BlockModel::g_bkLB + i] = cube_mode.Vertices()[BlockModel::g_bkLB];
	mHEdgeBlockModels[block_index].Vertices()[BlockModel::g_topLT] = cube_mode.Vertices()[BlockModel::g_topLB];
	mHEdgeBlockModels[block_index].Vertices()[BlockModel::g_btmLB] = cube_mode.Vertices()[BlockModel::g_btmLT];

	//不全是对的，因为有重复，但是要兼容旧版
	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(0,1.57,0),
		Vector3(0,3.14,0),
		Vector3(0,-1.57,0),
		Vector3(0,1.57,-1.57),
		Vector3(0,-1.57,1.57),
		Vector3(-1.57,-1.57,0),
		Vector3(-1.57,1.57,0)
	};
	auto _models = mHEdgeBlockModels;

	BlockModel tempModel;
	tempModel.Clone(_models[block_index]);
	for (int i = 1; i < 8; i++) {
		Vector3 angles = angleArr[i];
		int block_index = i;
		_models[block_index] = cube_mode;
		for (int i = 0; i < 24; i++) {
			Vector3 pt = Vector3(tempModel.Vertices()[i].position[0], tempModel.Vertices()[i].position[1], tempModel.Vertices()[i].position[2]);
			Vector3 newPt = vec3RotateByPoint(Vector3(0.5, 0.5, 0.5), Vector3(pt.x, pt.y, pt.z), angles);
			_models[block_index].Vertices()[i].SetPosition(newPt.x, newPt.y, newPt.z);

			//OUTPUT_LOG("idx:%d ,pt_0_1:(%f,%f,%f)==(%f,%f,%f) ? %s\n\n", i, pt.x, pt.y, pt.z, newPt.x, newPt.y, newPt.z, (abs(pt.x - newPt.x) < 0.0000001 && abs(pt.y - newPt.y) < 0.0000001 && abs(pt.z - newPt.z) < 0.0000001) ? "true" : "false");
		}
	}

	for (auto& model : mHEdgeBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_buildHOuterCornerBlockModels()
{
}

void ParaEngine::CSlopeModelProvider::_buildHInnerCornerBlockModels()
{
}
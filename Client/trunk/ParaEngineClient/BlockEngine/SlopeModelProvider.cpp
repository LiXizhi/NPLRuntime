#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "ShapeAABB.h"
#include "SlopeModelProvider.h"
using namespace ParaEngine;
ParaEngine::CSlopeModelProvider::CSlopeModelProvider(BlockTemplate* pBlockTemplate)
	:CLinearModelProvider(pBlockTemplate, sizeof(mEdgeBlockModels) / sizeof(mEdgeBlockModels[0]) + 
		sizeof(mOuterCornerBlockModels) / sizeof(mOuterCornerBlockModels[0]) + 
		sizeof(mInnerCornerBlockModels) / sizeof(mInnerCornerBlockModels[0]) + 
		sizeof(mHEdgeBlockModels) / sizeof(mHEdgeBlockModels[0]) + 
		sizeof(mInnerCornerBlockModels2) / sizeof(mInnerCornerBlockModels2[0]) +
		sizeof(mOutCornerModels_1) / sizeof(mOutCornerModels_1[0]) + sizeof(mOutCornerModels_2) / sizeof(mOutCornerModels_2[0]))
{
	_buildEdgeBlockModels();
	_builOuterCornerBlockModels();
	_buildInnerCornerBlockModels();
	_buildHEdgeBlockModels();
	_buildInnerCornerBlockModels2();

	_buildOutCornerModels_1();
	_buildOutCornerModels_2();
}

ParaEngine::CSlopeModelProvider::~CSlopeModelProvider()
{}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	BlockModel * model = mEdgeBlockModels;
	int block_index = 0;
	if (nIndex >= 100) {//新加的用1000以上的，避开旧的影响
		nIndex -= 100;
		if (nIndex < 24) {
			block_index = nIndex;
			model = mOutCornerModels_1;
		}
		else if (nIndex < 24 + 8) {
			block_index = nIndex - 24;
			model = mOutCornerModels_2;
		}

		nIndex += 32;
	}
	else {
		const int type_index = nIndex / 8;
		block_index = nIndex % 8;
		
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
			model = mInnerCornerBlockModels2;
			break;
		}
	}
	
	return (nIndex<m_nModelCount) ? model[block_index] : model[0];
}

BlockModel& ParaEngine::CSlopeModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks /*= NULL*/)
{
	nBlockData = nBlockData & 0xff;

	BlockModel * model = mEdgeBlockModels;
	int block_index = 0;
	if (nBlockData >= 100) {//新加的用100以上的，避开旧的影响
		nBlockData -= 100;
		if (nBlockData < 24) {
			block_index = nBlockData;
			model = mOutCornerModels_1;
		}
		else if (nBlockData < 32) {
			block_index = nBlockData - 24;
			model = mOutCornerModels_2;
		}
		nBlockData += 32;
	}
	else {
		const int type_index = nBlockData / 8;
		block_index = nBlockData % 8;
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
			model = mInnerCornerBlockModels2;
			break;
		}
	}
	
	return (nBlockData<m_nModelCount) ? model[block_index] : model[0];
}

void ParaEngine::CSlopeModelProvider::_buildEdgeBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int modelNum = sizeof(mEdgeBlockModels) / sizeof(mEdgeBlockModels[0]);

	for (int i = 0; i < modelNum; i++) {
		mEdgeBlockModels[i].ClearVertices();
		mEdgeBlockModels[i] = cube_mode;
	}

	BlockModel tempModel = cube_mode;
	tempModel.Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightLT].position[0], cube_mode.Vertices()[BlockModel::g_rightLT].position[1], cube_mode.Vertices()[BlockModel::g_rightLT].position[2]);
	for (int i = 0; i < 4; ++i) {
		tempModel.Vertices()[BlockModel::g_topLB + i] = cube_mode.Vertices()[BlockModel::g_topLB];
		tempModel.Vertices()[BlockModel::g_topLB + i].SetNormal(0, 0, 0);
	}
		
	tempModel.Vertices()[BlockModel::g_frtLT] = cube_mode.Vertices()[BlockModel::g_frtLB];
	tempModel.Vertices()[BlockModel::g_bkRT] = cube_mode.Vertices()[BlockModel::g_bkRB];

	tempModel.Vertices()[BlockModel::g_frtLT].SetNormal(0, 0, 0);
	tempModel.Vertices()[BlockModel::g_bkRT].SetNormal(0, 0, 0);

	calculateModelNormalOfFace(tempModel, BlockModel::g_leftLB);//左侧变成了斜面，法线重新计算

	Vector3 angleArr[8] = { 
		Vector3(0,0,0),
		Vector3(0, 3.14f, 0),
		Vector3(3.14f, 3.14f, 0),
		Vector3(3.14f, 0, 0),
		Vector3(3.14f, 1.57f, 0),
		Vector3(3.14f, -1.57f, 0),
		Vector3(0, -1.57f, 0),
		Vector3(0, 1.57f, 0)
	};
	cloneAndRotateModels(tempModel, angleArr, mEdgeBlockModels, modelNum);

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
	int modelNum = sizeof(mOuterCornerBlockModels) / sizeof(mOuterCornerBlockModels[0]);

	for (int i = 0; i < modelNum; i++) {
		mOuterCornerBlockModels[i].ClearVertices();
		mOuterCornerBlockModels[i] = cube_mode;
	}

	BlockModel tempModel = cube_mode;

	for (int i = 0; i < 4; ++i)
	{
		tempModel.Vertices()[BlockModel::g_topLB + i].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
		//tempModel.Vertices()[BlockModel::g_leftLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
		//tempModel.Vertices()[BlockModel::g_frtLB + i].SetNormal(cube_mode.Vertices()[BlockModel::g_topLB].normal[0], cube_mode.Vertices()[BlockModel::g_topLB].normal[1], cube_mode.Vertices()[BlockModel::g_topLB].normal[2]);
	}
	tempModel.Vertices()[BlockModel::g_frtLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_leftRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_bkRT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	tempModel.Vertices()[BlockModel::g_frtLT].SetTexcoord(cube_mode.Vertices()[BlockModel::g_frtRT].texcoord[0], cube_mode.Vertices()[BlockModel::g_frtRT].texcoord[1]);
	
	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(0,3.14f,0),
		Vector3(3.14f,3.14f,0),
		Vector3(3.14f,0,0),
		Vector3(0,1.57f,0),
		Vector3(0,-1.57f,0),
		Vector3(3.14f,1.57f,0),
		Vector3(3.14f,-1.57f,0)
	};

	cloneAndRotateModels(tempModel, angleArr, mOuterCornerBlockModels, modelNum);

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
	int modelNum = sizeof(mInnerCornerBlockModels) / sizeof(mInnerCornerBlockModels[0]);

	for (int i = 0; i < modelNum; i++) {
		mInnerCornerBlockModels[i].ClearVertices();
		mInnerCornerBlockModels[i] = cube_mode;
	}

	BlockModel tempModel = cube_mode;
	//shift FRT to FRB
	tempModel = cube_mode;
	tempModel.Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	tempModel.Vertices()[BlockModel::g_topRB].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	tempModel.Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);

	tempModel.Vertices()[BlockModel::g_topRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	tempModel.Vertices()[BlockModel::g_topRT].SetTexcoord(1, 1);
	tempModel.Vertices()[BlockModel::g_topRB].SetTexcoord(1,1);

	auto vert = tempModel.Vertices()[BlockModel::g_frtRB];
	tempModel.AddVertex(vert);
	vert = tempModel.Vertices()[BlockModel::g_topLT];
	tempModel.AddVertex(vert);
	vert = tempModel.Vertices()[BlockModel::g_bkLT];
	vert.SetTexcoord(1, 0);
	tempModel.AddVertex(vert);
	vert = tempModel.Vertices()[BlockModel::g_frtRB];
	tempModel.AddVertex(vert);
	tempModel.SetFaceCount(tempModel.Vertices().size() / 4);

	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(3.14f,1.57f,0),
		Vector3(0,1.57f,0),
		Vector3(3.14f,3.14f,0),
		Vector3(0,3.14f,0),
		Vector3(3.14f,-1.57f,0),
		Vector3(0,-1.57f,0),
		Vector3(3.14f,0,0)
	};

	cloneAndRotateModels(tempModel, angleArr, mInnerCornerBlockModels, modelNum);

	for (auto& model : mInnerCornerBlockModels) {
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_buildInnerCornerBlockModels2()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int modelNum = sizeof(mInnerCornerBlockModels2) / sizeof(mInnerCornerBlockModels2[0]);

	for (int i = 0; i < modelNum; i++) {
		mInnerCornerBlockModels2[i].ClearVertices();
		mInnerCornerBlockModels2[i] = cube_mode;
	}

	BlockModel tempModel = cube_mode;
	//shift FRT to FRB
	tempModel = cube_mode;
	tempModel.Vertices()[BlockModel::g_frtRT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	tempModel.Vertices()[BlockModel::g_topRB].SetPosition(cube_mode.Vertices()[BlockModel::g_topRT].GetPosition());
	tempModel.Vertices()[BlockModel::g_rightLT].SetPosition(cube_mode.Vertices()[BlockModel::g_frtRB].position[0], cube_mode.Vertices()[BlockModel::g_frtRB].position[1], cube_mode.Vertices()[BlockModel::g_frtRB].position[2]);
	tempModel.Vertices()[BlockModel::g_topRB].SetTexcoord(cube_mode.Vertices()[BlockModel::g_topRT].GetTexcoord());
	
	// add another sloping face
	auto vert = tempModel.Vertices()[BlockModel::g_frtRB];
	tempModel.AddVertex(vert);
	vert = tempModel.Vertices()[BlockModel::g_topLB];
	tempModel.AddVertex(vert);
	vert = tempModel.Vertices()[BlockModel::g_topRT];
	tempModel.AddVertex(vert);
	vert = tempModel.Vertices()[BlockModel::g_frtRB];
	tempModel.AddVertex(vert);
	tempModel.SetFaceCount(tempModel.Vertices().size() / 4);

	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(3.14f,1.57f,0),
		Vector3(0,1.57f,0),
		Vector3(3.14f,3.14f,0),
		Vector3(0,3.14f,0),
		Vector3(3.14f,-1.57f,0),
		Vector3(0,-1.57f,0),
		Vector3(3.14f,0,0)
	};

	cloneAndRotateModels(tempModel, angleArr, mInnerCornerBlockModels2, modelNum);

	for (auto& model : mInnerCornerBlockModels2) {
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_buildHEdgeBlockModels()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int modelNum = sizeof(mHEdgeBlockModels) / sizeof(mHEdgeBlockModels[0]);

	for (int i = 0; i < modelNum; i++) {
		mHEdgeBlockModels[i].ClearVertices();
		mHEdgeBlockModels[i] = cube_mode;
	}

	BlockModel tempModel = cube_mode;
	
	tempModel = cube_mode;
	tempModel.Vertices()[BlockModel::g_leftLB].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRB].position[0], cube_mode.Vertices()[BlockModel::g_rightRB].position[1], cube_mode.Vertices()[BlockModel::g_rightRB].position[2]);
	tempModel.Vertices()[BlockModel::g_leftLT].SetPosition(cube_mode.Vertices()[BlockModel::g_rightRT].position[0], cube_mode.Vertices()[BlockModel::g_rightRT].position[1], cube_mode.Vertices()[BlockModel::g_rightRT].position[2]);
	for (int i = 0; i < 4; ++i)
		tempModel.Vertices()[BlockModel::g_bkLB + i] = cube_mode.Vertices()[BlockModel::g_bkLB];
	tempModel.Vertices()[BlockModel::g_topLT] = cube_mode.Vertices()[BlockModel::g_topLB];
	tempModel.Vertices()[BlockModel::g_btmLB] = cube_mode.Vertices()[BlockModel::g_btmLT];

	//不全是对的，因为有重复，但是要兼容旧版
	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(0,1.57f,0),
		Vector3(0,3.14f,0),
		Vector3(0,-1.57f,0),
		Vector3(0,1.57f,-1.57f),
		Vector3(0,-1.57f,1.57f),
		Vector3(-1.57f,-1.57f,0),
		Vector3(-1.57f,1.57f,0)
	};

	cloneAndRotateModels(tempModel, angleArr, mHEdgeBlockModels, modelNum);

	for (auto& model : mHEdgeBlockModels){
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::_buildOutCornerModels_1()
{
	BlockModel cube_mode;
	cube_mode.LoadModelByTexture(0);
	int modelNum = sizeof(mOutCornerModels_1) / sizeof(mOutCornerModels_1[0]);

	int faceCount = mOuterCornerBlockModels[0].GetFaceCount();
	for (int i = 0; i < modelNum; i++) {
		mOutCornerModels_1[i].ClearVertices();
		mOutCornerModels_1[i] = cube_mode;
		mOutCornerModels_1[i].SetFaceCount(faceCount);
	}

	BlockModel tempModel = cube_mode;
	int num = faceCount * 4;
	for (int i = 0; i < num; i++) {
		Vector3 pt;
		
		mOuterCornerBlockModels[0].Vertices()[i].GetPosition(pt);
		Vector3 newPt = vec3RotateByPoint(Vector3(0.5, 0.5, 0.5), Vector3(pt.x, pt.y, pt.z), Vector3(1.57f, 1.57f, 0));

		newPt.x = round(newPt.x);
		newPt.y = round(newPt.y);
		newPt.z = round(newPt.z);

		tempModel.Vertices()[i].SetPosition(newPt.x, newPt.y, newPt.z);
	}

	for (int i = tempModel.GetFaceCount() * 4-1; i >=num; i--) {
		tempModel.Vertices().pop_back();
	}
	tempModel.SetFaceCount(faceCount);

	Vector3 angleArr[24] = {
		//尖尖朝着y正方向
		Vector3(0,0,1.57f),
		Vector3(1.57f,0,1.57f),
		Vector3(3.14f,0,1.57f),
		Vector3(-1.57f,0,1.57f),
		//尖尖朝着y负方向
		Vector3(0,0,-1.57f),
		Vector3(1.57f,0,-1.57f),
		Vector3(3.14f,0,-1.57f),
		Vector3(-1.57f,0,-1.57f),


		//尖尖朝着x正方向
		Vector3(0,0,0),
		Vector3(1.57f,0,0),
		Vector3(3.14f,0,0),
		Vector3(-1.57f,0,0),
		//尖尖朝着x负方向
		Vector3(0,0,3.14f),
		Vector3(1.57f,0,3.14f),
		Vector3(3.14f,0,3.14f),
		Vector3(-1.57f,0,3.14f),

		//尖尖朝着z正方向
		Vector3(0,-1.57f,0),
		Vector3(1.57f,-1.57f,0),
		Vector3(3.14f,-1.57f,0),
		Vector3(-1.57f,-1.57f,0),
		//尖尖朝着z负方向
		Vector3(0,1.57f,3.14f),
		Vector3(1.57f,1.57f,3.14f),
		Vector3(3.14f,1.57f,3.14f),
		Vector3(-1.57f,1.57f,3.14f),
	};

	cloneAndRotateModels(tempModel, angleArr, mOutCornerModels_1, modelNum);

	for (auto& model : mOutCornerModels_1) {
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

static void _AddVertex(BlockModel& tmp, const Vector3 *pts, Vector3 &normal)
{
	static Vector2 texCoord[4] = {
		Vector2(0,1),
		Vector2(0,0),
		Vector2(1,0),
		Vector2(1,1),
	};
	for (int i = 0; i < 4; i++) {
		BlockVertexCompressed vert;
		auto &pt = pts[i];
		vert.SetPosition(pt);
		vert.SetNormal(normal);

		float u = texCoord[i].x;
		float v = texCoord[i].y;

		if (abs(abs(normal.x) - 1) < 0.01f) {
			if (abs(abs(pt.y) - 0.5f) < 0.01f) {
				v = 0.5f;
			}
			if (abs(abs(pt.z) - 0.5f) < 0.01f) {
				u = 0.5f;
			}
		}
		if (abs(abs(normal.z) - 1) < 0.01f) {
			if (abs(abs(pt.y) - 0.5f) < 0.01f) {
				v = 0.5f;
			}
			if (abs(abs(pt.x) - 0.5f) < 0.01f) {
				u = 0.5f;
			}
		}
		if (abs(abs(normal.y) - 1) < 0.01f) {
			if (abs(abs(pt.z) - 0.5f) < 0.01f) {
				v = 0.5f;
			}
			if (abs(abs(pt.x) - 0.5f) < 0.01f) {
				u = 0.5f;
			}
		}
		vert.SetTexcoord(u, v);
		tmp.AddVertex(vert);
	}
}

void ParaEngine::CSlopeModelProvider::_buildOutCornerModels_2()
{
	BlockModel template_1;
	{
		template_1.ClearVertices();

		//上面部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 1, 1);
			Vector3 pts[4] = {
				Vector3(0, 1, 0),
				Vector3(0, 0, 1),
				Vector3(1, 0, 0),
				Vector3(1, 0, 0)
			};
			_AddVertex(template_1, pts, normal);
		}
		//前面部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(0, 1, 0),
				Vector3(1, 0, 0),
				Vector3(1, 0, 0)
			};
			_AddVertex(template_1, pts, normal);
		}
		//左面部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 1),
				Vector3(0, 0, 1),
				Vector3(0, 1, 0),
				Vector3(0, 0, 0)
			};
			_AddVertex(template_1, pts, normal);
		}
		//下面部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, -1, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 1),
				Vector3(0, 0, 0),
				Vector3(1, 0, 0),
				Vector3(0, 0, 1)
			};
			_AddVertex(template_1, pts, normal);
		}
	}

	template_1.SetFaceCount(template_1.Vertices().size() / 4);

	Vector3 angleArr[8] = {
		Vector3(0,0,0),
		Vector3(0,1.57f,0),
		Vector3(0,3.14f,0),
		Vector3(0,-1.57f,0),
		
		Vector3(3.14f,0,0),
		Vector3(3.14f,1.57f,0),
		Vector3(3.14f,3.14f,0),
		Vector3(3.14f,-1.57f,0),
	};

	cloneAndRotateModels(template_1, angleArr, mOutCornerModels_2, sizeof(angleArr) / sizeof(angleArr[0]));

	for (auto& model : mOutCornerModels_2) {
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CSlopeModelProvider::cloneAndRotateModels(BlockModel &tempModel, Vector3 *angleArr, BlockModel *outModels, int len) {
	for (int i = 0; i < len; i++) {
		Vector3 angles = angleArr[i];
		int block_index = i;
		BlockModel & model = outModels[block_index + 0];
		model.ClearVertices();
		int faceNum = tempModel.GetFaceCount();
		for (int face = 0; face < faceNum; face++) {
			for (int j = 0;j<4; j++) {
				int idx = face * 4 + j;
				Vector3 pt;

				BlockVertexCompressed vert = tempModel.Vertices()[idx];
				vert.GetPosition(pt);
				Vector3 newPt = vec3RotateByPoint(Vector3(0.5f, 0.5f, 0.5f), pt, angles);

				newPt.x = round(newPt.x);
				newPt.y = round(newPt.y);
				newPt.z = round(newPt.z);

				vert.SetPosition(newPt.x, newPt.y, newPt.z);

				model.AddVertex(vert);
				
			}
			calculateModelNormalOfFace(model, face*4);

			//int tempFaceCount = model.GetFaceCount();
			//if (normal.positionEquals(Vector3(0, 0, 0))) {//表示这不是一个正常的面，直接剪裁掉
			//	int start = face * 4;//去掉这个面的四个顶点，并前移数组
			//	for (int v = start; v < tempFaceCount * 4 - 4; v++) {
			//		model.Vertices()[v] = model.Vertices()[v + 4];
			//	}
			//	for (int v = 0; v < 4; v++) {
			//		model.Vertices().pop_back();
			//	}
			//	tempFaceCount--;
			//}
			//model.SetFaceCount(tempFaceCount);
		}
	}
}

Vector3 ParaEngine::CSlopeModelProvider::calculateModelNormalOfFace(BlockModel &tempModel, int startIdxOfFace)
{
	int idx = startIdxOfFace + 0;
	Vector3 pt_0 = Vector3(tempModel.Vertices()[idx].position[0], tempModel.Vertices()[idx].position[1], tempModel.Vertices()[idx].position[2]);

	idx = startIdxOfFace + 1;
	Vector3 pt_1 = Vector3(tempModel.Vertices()[idx].position[0], tempModel.Vertices()[idx].position[1], tempModel.Vertices()[idx].position[2]);

	idx = startIdxOfFace + 2;
	Vector3 pt_2 = Vector3(tempModel.Vertices()[idx].position[0], tempModel.Vertices()[idx].position[1], tempModel.Vertices()[idx].position[2]);

	idx = startIdxOfFace + 3;
	Vector3 pt_3 = Vector3(tempModel.Vertices()[idx].position[0], tempModel.Vertices()[idx].position[1], tempModel.Vertices()[idx].position[2]);

	Vector3 dir0_1 = pt_0 - pt_1;
	Vector3 dir0_2 = pt_0 - pt_2;
	Vector3 dir0_3 = pt_0 - pt_3;

	Vector3 normal = Vector3(0, 0, 0);
	if (!dir0_1.positionEquals(normal) && !dir0_2.positionEquals(normal) && !dir0_1.positionEquals(dir0_2)) {
		normal = dir0_1.crossProduct(dir0_2);
	}
	else if (!dir0_1.positionEquals(normal) && !dir0_3.positionEquals(normal) && !dir0_1.positionEquals(dir0_3)) {
		normal = dir0_1.crossProduct(dir0_3);
	}
	else if (!dir0_2.positionEquals(normal) && !dir0_3.positionEquals(normal) && !dir0_2.positionEquals(dir0_3)) {
		normal = dir0_2.crossProduct(dir0_3);
	}
	
	normal.normalise();
	for (int i = 0; i < 4; i++) {
		int idx = startIdxOfFace + i;
		tempModel.Vertices()[idx].SetNormal(normal);
	}
	return normal;
}
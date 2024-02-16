#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "ShapeAABB.h"
#include "StairModelProvider.h"
using namespace ParaEngine;

ParaEngine::CStairModelProvider::CStairModelProvider(BlockTemplate* pBlockTemplate)
	:CLinearModelProvider(pBlockTemplate, sizeof(mBlockModels) / sizeof(mBlockModels[0]))
{
	_buildBlockModels();

}

ParaEngine::CStairModelProvider::~CStairModelProvider()
{}

BlockModel& ParaEngine::CStairModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	const int block_index = nIndex;


	return (nIndex < m_nModelCount) ? mBlockModels[block_index] : mBlockModels[0];
}


BlockModel& ParaEngine::CStairModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks /*= NULL*/)
{
	nBlockData = nBlockData & 0xff;

	const int block_index = nBlockData;


	return (nBlockData < m_nModelCount) ? mBlockModels[block_index] : mBlockModels[0];
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

void ParaEngine::CStairModelProvider::_buildBlockModels()
{
	BlockModel template_1;//https://s1.ax1x.com/2022/09/05/vTL1ln.png
	BlockModel template_2;//https://s1.ax1x.com/2022/09/05/vTj41J.png
	BlockModel template_3;//https://s1.ax1x.com/2022/09/05/vTxPM9.png

	//第一个形状
	template_1.ClearVertices();
	{
		//右边，完整正方形
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, 1);
			Vector3 pts[4] = {
				Vector3(1, 0, 1),
				Vector3(1, 1, 1),
				Vector3(0, 1, 1),
				Vector3(0, 0, 1)
			};
			_AddVertex(template_1, pts, normal);
		}

		//下面，完整正方形
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, -1, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(1, 0, 0),
				Vector3(1, 0, 1),
				Vector3(0, 0, 1)
			};
			_AddVertex(template_1, pts, normal);
		}

		//上面的右边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0, 1, 0.5f),
				Vector3(0, 1, 1),
				Vector3(1, 1, 1),
				Vector3(1, 1, 0.5f)
			};
			_AddVertex(template_1, pts, normal);
		}
		//前面的右下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0, 0.5f),
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 0.5f, 1),
				Vector3(1, 0, 1)
			};
			_AddVertex(template_1, pts, normal);
		}

		//左边的上半部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0, 0.5f, 0.5f),
				Vector3(0, 1, 0.5f),
				Vector3(1, 1, 0.5f),
				Vector3(1, 0.5f, 0.5f)
			};
			_AddVertex(template_1, pts, normal);
		}
		
		//后面的左上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0, 0.5f, 1),
				Vector3(0, 1, 1),
				Vector3(0, 1, 0.5f),
				Vector3(0, 0.5f, 0.5f)
			};
			_AddVertex(template_1, pts, normal);
		}

		//后面的左下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 1),
				Vector3(0, 0.5f, 1),
				Vector3(0, 0.5f, 0.5f),
				Vector3(0, 0, 0.5f)
			};
			_AddVertex(template_1, pts, normal);
		}

		//后面的右边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 0.5f),
				Vector3(0, 0.5f, 0.5f),
				Vector3(0, 0.5f, 0),
				Vector3(0, 0, 0)
			};
			_AddVertex(template_1, pts, normal);
		}

		//上面的左边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0, 0.5f, 0),
				Vector3(0, 0.5f, 0.5f),
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 0.5f, 0)
			};
			_AddVertex(template_1, pts, normal);
		}

		//前面的左边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0, 0),
				Vector3(1, 0.5f, 0),
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 0, 0.5f)
			};
			_AddVertex(template_1, pts, normal);
		}

		//前面的右上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 1, 0.5f),
				Vector3(1, 1, 1),
				Vector3(1, 0.5f, 1)
			};
			_AddVertex(template_1, pts, normal);
		}

		//左边的下半部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(0, 0.5f, 0),
				Vector3(1, 0.5f, 0),
				Vector3(1, 0, 0)
			};
			_AddVertex(template_1, pts, normal);
		}

	}

	//第2个形状
	template_2.ClearVertices();
	{
		//下面，完整正方形
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, -1, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(1, 0, 0),
				Vector3(1, 0, 1),
				Vector3(0, 0, 1)
			};
			_AddVertex(template_2, pts, normal);
		}

		//上面的左边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0,0.5f,0),
				Vector3(0, 0.5f, 1),
				Vector3(0.5f, 0.5f, 1),
				Vector3(0.5f, 0.5f, 0)
			};
			_AddVertex(template_2, pts, normal);
		}
		
		//前面的上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0.5f, 0.5f, 0),
				Vector3(0.5f, 1, 0),
				Vector3(1, 1, 0),
				Vector3(1, 0.5f, 0)
			};
			_AddVertex(template_2, pts, normal);
		}
		
		//左面上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0.5f, 0.5f, 0.5f),
				Vector3(0.5f,1, 0.5f),
				Vector3(0.5f,1,0),
				Vector3(0.5f, 0.5f,0)
			};
			_AddVertex(template_2, pts, normal);
		}
		
		//右面上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0.5f, 0),
				Vector3(1, 1, 0),
				Vector3(1, 1, 0.5f),
				Vector3(1, 0.5f, 0.5f)
			};
			_AddVertex(template_2, pts, normal);
		}
		
		//后面的上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, 1);
			Vector3 pts[4] = {
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 1, 0.5f),
				Vector3(0.5f, 1, 0.5f),
				Vector3(0.5f, 0.5f, 0.5f)
			};
			_AddVertex(template_2, pts, normal);
		}
		//后面的下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, 1);
			Vector3 pts[4] = {
				Vector3(1, 0, 1),
				Vector3(1, 0.5f, 1),
				Vector3(0, 0.5f, 1),
				Vector3(0,0 ,1)
			};
			_AddVertex(template_2, pts, normal);
		}

		//上面的右上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0.5f, 0.5f, 0.5f),
				Vector3(0.5f, 0.5f, 1),
				Vector3(1, 0.5f, 1),
				Vector3(1, 0.5f, 0.5f)
			};
			_AddVertex(template_2, pts, normal);
		}
		//上面的右下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0.5f, 1, 0),
				Vector3(0.5f, 1, 0.5f),
				Vector3(1, 1, 0.5f),
				Vector3(1, 1, 0)
			};
			_AddVertex(template_2, pts, normal);
		}
		//前面的下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(0, 0.5f, 0),
				Vector3(1, 0.5f, 0),
				Vector3(1, 0, 0)
			};
			_AddVertex(template_2, pts, normal);
		}
		//左面下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0,0,1),
				Vector3(0,0.5f,1),
				Vector3(0,0.5f,0),
				Vector3(0,0, 0)
			};
			_AddVertex(template_2, pts, normal);
		}
		//右面下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0, 0),
				Vector3(1, 0.5f, 0),
				Vector3(1, 0.5f, 1),
				Vector3(1, 0, 1)
			};
			_AddVertex(template_2, pts, normal);
		}
	}

	//第3个形状
	template_3.ClearVertices();
	{
		//下面，完整正方形
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, -1, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(1, 0, 0),
				Vector3(1, 0, 1),
				Vector3(0, 0, 1)
			};
			_AddVertex(template_3, pts, normal);
		}
		//左面，完整正方形
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(-1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0, 0, 1),
				Vector3(0, 1, 1),
				Vector3(0, 1, 0),
				Vector3(0, 0, 0)
			};
			_AddVertex(template_3, pts, normal);
		}

		//后边，完整正方形
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, 1);
			Vector3 pts[4] = {
				Vector3(1, 0, 1),
				Vector3(1, 1, 1),
				Vector3(0, 1, 1),
				Vector3(0, 0, 1)
			};
			_AddVertex(template_3, pts, normal);
		}

		//上面的左边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0, 1, 0),
				Vector3(0, 1, 1),
				Vector3(0.5f, 1, 1),
				Vector3(0.5f, 1, 0)
			};
			_AddVertex(template_3, pts, normal);
		}
		
		//前面的左边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0, 0, 0),
				Vector3(0, 1, 0),
				Vector3(0.5f, 1, 0),
				Vector3(0.5f, 0, 0)
			};
			_AddVertex(template_3, pts, normal);
		}
				
		//右面右边部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0, 0.5f),
				Vector3(1, 1, 0.5f),
				Vector3(1, 1, 1),
				Vector3(1, 0, 1)
			};
			_AddVertex(template_3, pts, normal);
		}
		//右面左上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(0.5f, 0.5f, 0),
				Vector3(0.5f, 1, 0),
				Vector3(0.5f, 1, 0.5f),
				Vector3(0.5f, 0.5f, 0.5f)
			};
			_AddVertex(template_3, pts, normal);
		}
		//右面左下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(1, 0, 0);
			Vector3 pts[4] = {
				Vector3(1, 0, 0),
				Vector3(1, 0.5f, 0),
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 0, 0.5f)
			};
			_AddVertex(template_3, pts, normal);
		}
		//上面的右上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0.5f, 1, 0.5f),
				Vector3(0.5f, 1, 1),
				Vector3(1, 1, 1),
				Vector3(1, 1, 0.5f)
			};
			_AddVertex(template_3, pts, normal);
		}
		//上面的右下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 1, 0);
			Vector3 pts[4] = {
				Vector3(0.5f, 0.5f, 0),
				Vector3(0.5f, 0.5f, 0.5f),
				Vector3(1, 0.5f, 0.5f),
				Vector3(1, 0.5f, 0)
			};
			_AddVertex(template_3, pts, normal);
		}

		//前面的右上部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0.5f, 0.5f, 0.5f),
				Vector3(0.5f, 1, 0.5f),
				Vector3(1, 1, 0.5f),
				Vector3(1, 0.5f, 0.5f)
			};
			_AddVertex(template_3, pts, normal);
		}
		//前面的右下部分
		{
			BlockVertexCompressed vertArr[4];
			Vector3 normal = Vector3(0, 0, -1);
			Vector3 pts[4] = {
				Vector3(0.5f, 0, 0),
				Vector3(0.5f, 0.5f, 0),
				Vector3(1, 0.5f, 0),
				Vector3(1, 0, 0)
			};
			_AddVertex(template_3, pts, normal);
		}
	}

	template_1.SetFaceCount(template_1.Vertices().size() / 4);
	//template_1.RecalculateFaceShapeAndSortFaces();
	//template_1.DumpToLog();
	template_2.SetFaceCount(template_2.Vertices().size() / 4);
	template_3.SetFaceCount(template_2.Vertices().size() / 4);

	//去翻转，这里是为了跟旧的模型一模一样，实际是有一些重复和缺失
	{
		Vector3 angles[5] = {
			Vector3(0,1.57f,0),
			Vector3(0,1.57f,0),
			Vector3(0,-1.57f,0),
			Vector3(0,0,0),
			Vector3(0,3.14f,0),
		};
		int startOutIdx = 0;
		cloneAndRotateModels(template_1, angles, mBlockModels, sizeof(angles) / sizeof(angles[0]), startOutIdx);
	}
	{
		Vector3 angles[4] = {
			Vector3(3.14f,1.57f,0),
			Vector3(3.14f,3.14f,0),
			Vector3(3.14f,-1.57f,0),
			Vector3(3.14f,0,0),
		};
		int startOutIdx = 10;
		cloneAndRotateModels(template_1, angles, mBlockModels, sizeof(angles) / sizeof(angles[0]), startOutIdx);
	}
	{
		Vector3 angles[4] = {
			Vector3(0,-1.57f,0),
			Vector3(0,3.14f,0),
			Vector3(0,1.57f,0),
			Vector3(0,0,0),
		};
		int startOutIdx = 5;
		cloneAndRotateModels(template_2, angles, mBlockModels, sizeof(angles) / sizeof(angles[0]), startOutIdx);
	}
	{
		Vector3 angles[4] = {
			Vector3(3.14f,0,0),
			Vector3(3.14f,-1.57f,0),
			Vector3(3.14f,3.14f,0),
			Vector3(3.14f,1.57f,0),
		};
		int startOutIdx = 14;
		cloneAndRotateModels(template_2, angles, mBlockModels, sizeof(angles) / sizeof(angles[0]), startOutIdx);
	}

	{
		Vector3 angles[6] = {
			Vector3(0,1.57f,0),
			Vector3(0,0,0),
			Vector3(0,-1.57f,0),
			Vector3(0,3.14f,0),
			Vector3(3.14f,3.14f,0),
			Vector3(3.14f,1.57f,0),
		};
		int startOutIdx = 18;
		cloneAndRotateModels(template_3, angles, mBlockModels, sizeof(angles) / sizeof(angles[0]), startOutIdx);
	}
	{
		Vector3 angles[1] = {
			Vector3(3.14f,3.14f,0),
		};
		int startOutIdx = 9;
		cloneAndRotateModels(template_3, angles, mBlockModels, sizeof(angles) / sizeof(angles[0]), startOutIdx);
	}

	for (auto& model : mBlockModels) {
		model.SetFaceCount(model.Vertices().size() / 4);
		model.SetUseAmbientOcclusion(false);
		model.SetUniformLighting(true);
	}
}

void ParaEngine::CStairModelProvider::cloneAndRotateModels(BlockModel &tempModel, Vector3 *angleArr, BlockModel *outModels, int len, int startOutIdx) {
	for (int i = 0; i < len; i++) {
		Vector3 angles = angleArr[i];
		int block_index = i;
		BlockModel & model = outModels[block_index + startOutIdx];
		model.ClearVertices();
		int faceNum = tempModel.GetFaceCount();
		for (int face = 0; face < faceNum; face++) {
			for (int j = 0; j < 4; j++) {
				int idx = face * 4 + j;
				Vector3 pt;

				BlockVertexCompressed vert = tempModel.Vertices()[idx];
				vert.GetPosition(pt);
				Vector3 newPt = vec3RotateByPoint(Vector3(0.5f, 0.5f, 0.5f), pt, angles);

				if (abs(newPt.x - 0.5f) < 0.01f) { newPt.x = 0.5f; }
				else if (abs(newPt.x + 0.5f) < 0.01f) { newPt.x = -0.5f; }
				else { newPt.x = round(newPt.x); }

				if (abs(newPt.y - 0.5f) < 0.01f) { newPt.y = 0.5f; }
				else if (abs(newPt.y + 0.5f) < 0.01f) { newPt.y = -0.5f; }
				else { newPt.y = round(newPt.y); }

				if (abs(newPt.z - 0.5f)< 0.01f) { newPt.z = 0.5f; }
				else if (abs(newPt.z + 0.5f)< 0.01f) { newPt.z = -0.5f; }
				else { newPt.z = round(newPt.z); }
				
				vert.SetPosition(newPt.x, newPt.y, newPt.z);

				vert.GetNormal(pt);
				newPt = vec3RotateByPoint(Vector3(0,0,0), pt, angles);
				newPt.normalise();
				newPt.x = round(newPt.x);
				newPt.y = round(newPt.y);
				newPt.z = round(newPt.z);
				vert.SetNormal(newPt.x, newPt.y, newPt.z);

				model.AddVertex(vert);
			}
		}
		model.SetFaceCount(faceNum);
		model.RecalculateFaceShapeAndSortFaces();
	}
}

Vector3 ParaEngine::CStairModelProvider::calculateModelNormalOfFace(BlockModel &tempModel, int startIdxOfFace)
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
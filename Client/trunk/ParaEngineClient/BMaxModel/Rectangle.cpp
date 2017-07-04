#include "ParaEngine.h"
#include "Rectangle.h"

namespace ParaEngine
{

	Rectangle::Rectangle(vector<BMaxNodePtr> nodes_, uint32 faceIndex_) : m_nodes(nodes_), faceIndex(faceIndex_)
	{
		InitOffsetTable();
	}

	Rectangle::~Rectangle()
	{
	}

	Vector3 Rectangle::GetOffsetByIndex(int nIndex)
	{
		return m_directionOffsetTable[nIndex];
	}

	void Rectangle::InitOffsetTable()
	{
		//top face
		m_directionOffsetTable[0] = Vector3(0, 0, 1);
		m_directionOffsetTable[1] = Vector3(1, 0, 0);
		m_directionOffsetTable[2] = Vector3(0, 0, -1);
		m_directionOffsetTable[3] = Vector3(-1, 0, 0);

		//front face
		m_directionOffsetTable[4] = Vector3(0, 1, 0);
		m_directionOffsetTable[5] = Vector3(1, 0, 0);
		m_directionOffsetTable[6] = Vector3(0, -1, 0);
		m_directionOffsetTable[7] = Vector3(-1, 0, 0);

		//bottom face
		m_directionOffsetTable[8] = Vector3(0, 0, -1);
		m_directionOffsetTable[9] = Vector3(1, 0, 0);
		m_directionOffsetTable[10] = Vector3(0, 0, 1);
		m_directionOffsetTable[11] = Vector3(-1, 0, 0);

		//left face
		m_directionOffsetTable[12] = Vector3(0, 1, 0);
		m_directionOffsetTable[13] = Vector3(0, 0, -1);
		m_directionOffsetTable[14] = Vector3(0, -1, 0);
		m_directionOffsetTable[15] = Vector3(0, 0, 1);

		//right face
		m_directionOffsetTable[16] = Vector3(0, 1, 0);
		m_directionOffsetTable[17] = Vector3(0, 0, 1);
		m_directionOffsetTable[18] = Vector3(0, -1, 0);
		m_directionOffsetTable[19] = Vector3(0, 0, -1);

		//back face
		m_directionOffsetTable[20] = Vector3(0, 1, 0);
		m_directionOffsetTable[21] = Vector3(-1, 0, 0);
		m_directionOffsetTable[22] = Vector3(0, -1, 0);
		m_directionOffsetTable[23] = Vector3(1, 0, 0);
	}

	void Rectangle::UpdateNode(BMaxNode* fromNode, BMaxNode* toNode, uint32 index)
	{
		if (fromNode)
		{
			m_nodes[index % 4] = fromNode;
		}

		if (toNode)
		{
			m_nodes[(index + 1) % 4] = toNode;
		}
	}

	void Rectangle::CloneNodes()
	{
		int startVertex = faceIndex * 4;
		for(int i = 0; i < 4; i++)
		{
			BlockModel *cube = m_nodes[i]->GetCube();
			BlockVertexCompressed *vertice = cube->GetVertices();
			m_retangleVertices[i] = vertice[startVertex + i];
		}
	}
	void Rectangle::ScaleVertices(float scale)
	{
		for (BlockVertexCompressed vertice : m_retangleVertices)
		{
			Vector3 pos;
			vertice.GetPosition(pos);
			pos[0] *= scale;
			pos[1] *= scale;
			pos[2] *= scale;
			vertice.SetPosition(pos[0], pos[1], pos[2]);
		}
	}
}
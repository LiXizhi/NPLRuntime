#include "ParaEngine.h"
#include "Rectangle.h"

namespace ParaEngine
{
	const Vector3 Rectangle::DirectionOffsetTable[24] =
	{
		//top face
		Vector3(0, 0, 1),
		Vector3(1, 0, 0),
		Vector3(0, 0, -1),
		Vector3(-1, 0, 0),

		//front face
		Vector3(0, 1, 0),
		Vector3(1, 0, 0),
		Vector3(0, -1, 0),
		Vector3(-1, 0, 0),

		//bottom face
		Vector3(0, 0, -1),
		Vector3(1, 0, 0),
		Vector3(0, 0, 1),
		Vector3(-1, 0, 0),

		//left face
		Vector3(0, 1, 0),
		Vector3(0, 0, -1),
		Vector3(0, -1, 0),
		Vector3(0, 0, 1),

		//right face
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(0, -1, 0),
		Vector3(0, 0, -1),

		//back face
		Vector3(0, 1, 0),
		Vector3(-1, 0, 0),
		Vector3(0, -1, 0),
		Vector3(1, 0, 0),
	};

	Rectangle::Rectangle(vector<BMaxNodePtr> &nodes_, uint32 faceIndex_) : m_nodes(nodes_), faceIndex(faceIndex_)
	{
		
	}



	Rectangle::~Rectangle()
	{
	}

	void Rectangle::InitOffsetTable()
	{
		
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
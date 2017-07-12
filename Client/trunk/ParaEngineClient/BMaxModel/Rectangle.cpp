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

	Rectangle::Rectangle(BMaxNodePtr nodes_[4], uint32 faceIndex_) : faceIndex(faceIndex_)
	{
		std::copy(nodes_, nodes_ + 4, m_nodes);
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
		for (BlockVertexCompressed &vertice : m_retangleVertices)
		{
			vertice.position[0] *= scale;
			vertice.position[1] *= scale;
			vertice.position[2] *= scale;
		}
	}
	int Rectangle::GetBoneIndexAt(int index)
	{
		return m_nodes[index]->GetBoneIndex();
	}
}
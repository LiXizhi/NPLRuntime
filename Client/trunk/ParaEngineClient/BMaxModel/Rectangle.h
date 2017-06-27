#pragma once
#include "BMaxNode.h"
#include "BlockEngine/BlockModel.h"

namespace ParaEngine
{
	class Rectangle : public CRefCounted
	{
	public:
		Rectangle(BMaxNodePtr *nodes_, uint32 faceIndex_);
		~Rectangle();
		void UpdateNode(BMaxNodePtr& fromNode, BMaxNodePtr&toNode, uint32 index);
		void CloneNodes();
		void ScaleVertices(float scale);

		BlockVertexCompressed *GetVertices()
		{
			return m_retangleVertices;
		}

		inline BMaxNode* GetFromNode(uint32 nIndex)
		{
			auto iter = m_nodes[nIndex % 4];
			return iter.get();
		}

		inline BMaxNode* GetToNode(uint32 nIndex)
		{
			auto iter = m_nodes[(nIndex + 1) % 4];
			return iter.get();
		}

		Vector3 GetOffsetByIndex(int nIndex);
	private:
		void InitOffsetTable();

	private:
		BMaxNodePtr* m_nodes;
		uint32 faceIndex;
		BlockVertexCompressed m_retangleVertices[4];
		std::vector<Vector3> m_directionOffsetTable;
	};

	typedef ref_ptr<Rectangle> RectanglePtr;
}



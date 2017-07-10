#pragma once
#include "BMaxNode.h"
#include "BlockEngine/BlockModel.h"

namespace ParaEngine
{
	class Rectangle : public CRefCounted
	{
	public:
		Rectangle(BMaxNodePtr nodes_[4], uint32 faceIndex_);
		~Rectangle();
		void UpdateNode(BMaxNode* fromNode, BMaxNode* toNode, uint32 index);
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

	private:
		void InitOffsetTable();

	public:
		static const Vector3 DirectionOffsetTable[24];

	private: 
		BMaxNodePtr m_nodes[4];
		uint32 faceIndex;
		BlockVertexCompressed m_retangleVertices[4];
		Vector3 m_directionOffsetTable[24];
	};

	typedef ref_ptr<Rectangle> RectanglePtr;
}



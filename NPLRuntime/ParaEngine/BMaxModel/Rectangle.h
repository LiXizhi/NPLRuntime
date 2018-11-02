#pragma once
#include "BMaxNode.h"
#include "BlockEngine/BlockModel.h"

namespace ParaEngine
{
	class Rectangle : public CRefCounted
	{
	public:
		Rectangle(BMaxNodePtr nodes_[4], uint32 faceIndex_);
		Rectangle(BMaxNode* node, uint32 faceIndex_);
		~Rectangle();
		/** set corner node*/
		void SetCornerNode(BMaxNode* node, uint32 index);
		void UpdateNode(BMaxNode* fromNode, BMaxNode* toNode, uint32 index);
		void CloneNodes();
		void ScaleVertices(float scale);
		int GetBoneIndexAt(int index);

		BlockVertexCompressed *GetVertices();

		BMaxNode* GetFromNode(uint32 nIndex);

		BMaxNode* GetToNode(uint32 nIndex);
	
	public:
		static const Vector3 DirectionOffsetTable[24];

	private: 
		/** value [0,5] representing one of the 6 faces. */
		uint32 m_faceIndex;
		/** 4 corner nodes of the rectangle, they could be the same node. */
		BMaxNodePtr m_nodes[4];
		/** the boundary rectangle vertices. */
		BlockVertexCompressed m_rectangleVertices[4];
	};

	typedef ref_ptr<Rectangle> RectanglePtr;
}



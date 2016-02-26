#pragma once

#include "BlockEngine/BlockCoordinate.h"
#include "BlockEngine/BlockDirection.h"

namespace ParaEngine
{
	class Bone;
	class BMaxParser;
	class BlockModel;
	struct BMaxFrameNode;

	/** base class for a block in bmax model */
	struct BMaxNode : public CRefCounted
	{
	public:
		BMaxNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_);
		virtual ~BMaxNode();
	public:
		inline uint32 GetIndex()
		{
			return (DWORD)x + ((DWORD)z << 8) + ((DWORD)y << 16);
		}
		virtual DWORD GetColor();
		virtual void SetColor(DWORD val);
		/** get the bone node interface if it is*/
		virtual BMaxFrameNode* ToBoneNode();
		/** if there are any bone associated with this node at the moment. */
		bool HasBoneWeight();
		/** return the index of first bone, return -1 if no bone is binded to this node*/
		virtual int GetBoneIndex();
		virtual void SetBoneIndex(int nIndex);

		/** get neighbor block by side id
		* @param nSize:
		*/
		BMaxNode* GetNeighbour(BlockDirection::Side nSize);
		virtual bool isSolid();

		/** 
		* @param tessellatedModel: generate block model vertices
		* @return vertices count
		*/
		virtual int TessellateBlock(BlockModel* tessellatedModel);

		void QueryNeighborBlockData(BMaxNode** pBlockData, int nFrom /*= 0*/, int nTo /*= 26*/);

		uint32 CalculateCubeAO(BMaxNode** neighborBlocks);

		int32_t GetAvgVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4);
	public:
		int16 x;
		int16 y;
		int16 z;
		int32 template_id;
		int32 block_data;
		int m_nBoneIndex;
	protected:
		BMaxParser* m_pParser;
		DWORD m_color;
	};
	typedef ref_ptr<BMaxNode> BMaxNodePtr;
}
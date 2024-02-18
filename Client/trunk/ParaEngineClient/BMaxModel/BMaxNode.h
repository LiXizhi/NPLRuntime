#pragma once

#include "BlockEngine/BlockCoordinate.h"
#include "BlockEngine/BlockDirection.h"

namespace ParaEngine
{
	class Bone;
	class BMaxParser;
	class BlockModel;
	class CParaXModel;
	struct BMaxFrameNode;

	/** base class for a block in bmax model */
	struct BMaxNode : public CRefCounted
	{
	public:
		BMaxNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_);
		virtual ~BMaxNode();

		enum FaceStatus
		{
			faceInvisible = 0,
			faceVisibleNotSign,
			faceVisibleSigned
		};

	public:
		inline uint64 GetIndex()
		{
			return (uint64)x + ((uint64)z << 16) + ((uint64)y << 32);
		}
		virtual DWORD GetColor();
		BlockModel *GetBlockModel();
		virtual CParaXModel *GetParaXModel();
		virtual bool HasTransform();
		virtual Matrix4 GetTransform();
		/** set block model weak reference. */
		void SetBlockModel(BlockModel* pModel);
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
		BMaxNode* GetNeighbourByOffset(Vector3 offset);
		inline bool isSolid() { return m_bIsSolid; };
		void setSolid(bool bValue);

		/** 
		* @param tessellatedModel: generate block model vertices
		* @return vertices count
		*/
		virtual int TessellateBlock(BlockModel* tessellatedModel);

		void QueryNeighborBlockData(BMaxNode** pBlockData, int nFrom /*= 0*/, int nTo /*= 26*/);

		uint32 CalculateCubeAO(BMaxNode** neighborBlocks);

		int32_t GetAvgVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4);

		void SetFaceVisible(int nIndex);
		void SetFaceUsed(int nIndex);
		bool IsFaceNotUse(int nIndex);

		int GetFaceShape(int nIndex);
	public:
		int16 x;
		int16 y;
		int16 z;
		int32 template_id;
		int32 block_data;
		int m_nBoneIndex;
		bool m_bIsSolid;
	protected:
		BMaxParser* m_pParser;
		DWORD m_color;
		/* weak reference to block model*/
		BlockModel * m_pBlockModel;
		FaceStatus m_facesStatus[6];
	};
	typedef ref_ptr<BMaxNode> BMaxNodePtr;
}
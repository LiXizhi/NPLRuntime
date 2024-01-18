#pragma once
#include "IAttributeFields.h"

namespace ParaEngine
{
	struct SceneState;

#pragma pack(push,1)
	/** a voxel octree data node
	* 1(isBlockMask)+3(color)+4(base)+8(children) = 16 bytes
	* a chunk contains at most 254 nodes = 16*254 = 4064 bytes ~= 4KB
	* 
	* if it is a leaf node or there are over 4 blocks in the 8 children, we will consider it as a block-typed node.
	* block are rendered as cube at its level of detail regardless of its completely full or only half full. 
	*/
	struct VoxelOctreeNode
	{
		VoxelOctreeNode();

		// 8 bits isBlockMask whether the 8 spaces in octree are blocks. 
		uint8_t isBlockMask;
		// 24 bits for color
		uint8_t colorRGB[3];
		uint32_t baseChunkOffset;
		union {
			uint8_t childOffsets[8];
			uint64_t childMask;
		};
		inline bool IsLeaf() { return childMask == 0xffffffffffffffff; };
		inline int GetBlockCountInMask() { 
			int count = 0;
			uint8_t n = isBlockMask;
			while (n) {
				n &= (n - 1);
				count++;
			}
			return count;
		};
		inline int IsBlock() { return IsLeaf() || (GetBlockCountInMask() >= 4); }
	};
#pragma pack(pop)

	/** a octree based sparse voxel model, with 8 bits color per node. 
	* The AABB of the model is always 1*1*1, one needs to transform the model when used. 
	* It uses extremely small memory, and can be dynamically changed. 
	*/
	class ParaVoxelModel : public IAttributeFields
	{
	public:
		ParaVoxelModel();
		virtual ~ParaVoxelModel();

		ATTRIBUTE_DEFINE_CLASS(ParaVoxelModel);

		ATTRIBUTE_METHOD1(ParaVoxelModel, SetBlock_s, char*) { cls->SetBlockCmd(p1); return S_OK; }
		ATTRIBUTE_METHOD(ParaVoxelModel, DumpOctree_s) { cls->DumpOctree(); return S_OK; }
		
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
			
	public:
		/** load the model from a binary buffer. */
		bool Load(const char* pBuffer, int nCount = -1);

		/** save the model to a binary buffer. */
		bool Save(std::vector<char>& output);

		/** set the block at the given position. 
		* @param x, y, z : the position of the block relative to level.
		* @param level: the level or model width. this should be a power of 2, like 1, 2, 4, 8, 16, ..., 1024, ...
		* @param color: the 24 bits color of the block. -1 if the block is empty.
		*/
		void SetBlock(uint32 x, uint32 y, uint32 z, int level, int color);
		/**
		* @param cmd: the command string to set the block. 
		* e.g. "7,7,7,8,254" means set the block at (7,7,7) at level 8 to color 254.
		*/
		void SetBlockCmd(const char* cmd);

		/** get the block color at the given position and level
		* @return -1 if the block is empty. or return 8 bits color of the block.
		*/
		int GetBlock(uint32 x, uint32 y, uint32 z, int level);

		/** ray picking at given level
		* @param level: the model level at which to pick. this should be a power of 2, like 1, 2, 4, 8, 16, ..., 1024, ...
		*/
		bool RayPicking(const Vector3& origin, const Vector3& dir, Vector3& hitPos, int& hitColor, int level = -1);

		/** draw blocks with dynamic level of detail with reference to the camera position in scene state.
		*/
		void Draw(SceneState* pSceneState);
		
	public:
		/** optimize the model to remove and merge octree node. */
		void Optimize();
		void OptimizeNode(VoxelOctreeNode* pNode);
		/** 
		* @param nodes: update nodes from nodes[nNodeCount - 1](smallest child) to nodes[0] (root node). 
		*/
		void UpdateNode(VoxelOctreeNode* nodes[], int nNodeCount);

		/** get the depth of the octree at the given level. 
		* e.g. LevelToDepth(1024) == 10
		*/
		inline int LevelToDepth(int level);

		int CreateGetFreeChunkIndex(int nMinFreeSize = 8);

		VoxelOctreeNode* CreateGetChildNode(VoxelOctreeNode* pNode, int nChildIndex);
		VoxelOctreeNode* GetChildNode(VoxelOctreeNode* pNode, int nChildIndex);

		void DumpOctree();
		void DumpOctreeNode(VoxelOctreeNode* pNode, int nDepth, int nChunkIndex, int offset);

		VoxelOctreeNode* GetRootNode();
	private:
		typedef std::vector<VoxelOctreeNode> VoxelChunk;
		std::vector< VoxelChunk* > m_chunks;
	};
}

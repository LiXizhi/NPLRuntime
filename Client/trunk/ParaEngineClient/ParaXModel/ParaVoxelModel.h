#pragma once
#include "IAttributeFields.h"

namespace ParaEngine
{
	struct SceneState;
	class VoxelChunk;
	struct VoxelOctreeNode;

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
		/** default to create a full leaf block */
		VoxelOctreeNode(uint8_t isBlockMask = 0xff);

		// 8 bits isBlockMask whether the 8 spaces in octree are blocks. 
		uint8_t isBlockMask;
		// 24 bits for color
		uint8_t colorRGB[3];
		// low 24 bits for base chunk offset, high 8 bits for voxel shape. 
		uint32_t baseChunkOffset;
		union {
			uint8_t childOffsets[8];
			uint64_t childMask;
		};

		static VoxelOctreeNode EmptyNode;
		static VoxelOctreeNode FullNode;
	public:
		inline void MakeEmpty() { isBlockMask = 0; };
		inline void MakeFullBlock() { isBlockMask = 0xff; };
		inline uint32 GetColor() { return colorRGB[0] | (colorRGB[1] << 8) | (colorRGB[2] << 16); };
		inline void SetColor(uint32 color) { colorRGB[0] = color & 0xff; colorRGB[1] = (color >> 8) & 0xff; colorRGB[2] = (color >> 16) & 0xff; };
		// only 24 bits are used
		inline int GetBaseChunkOffset() { return baseChunkOffset & 0xffffff; };
		inline void SetBaseChunkOffset(uint32_t value) { baseChunkOffset = value & 0xffffff; };
		inline void SetVoxelShape(uint32_t shape) {
			baseChunkOffset |= (shape << 24);
		}
		inline uint8_t GetVoxelShape() {
			return uint8_t(baseChunkOffset >> 24);
		}

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
		inline void MarkDeleted() {
			baseChunkOffset = 0xffffff;
		}
		inline void UnMarkDeleted() {
			baseChunkOffset = 0;
		}
		inline bool IsDeleted() {
			return baseChunkOffset == 0xffffff;
		}
	};
#pragma pack(pop)

	/** a chunk of at most 254 octree nodes. */
	class VoxelChunk : public std::vector<VoxelOctreeNode>
	{
	public:
		VoxelChunk():m_nSize(0), m_nTailFreeItemIndex(0), m_nHeadFreeItemIndex(0){
			resize(254);
			for (int i = 0; i < 254; i++) {
				(*this)[i].MarkDeleted();
			}
		};
		~VoxelChunk() {};

		void erase(int index) {
			(*this)[index].MarkDeleted();
			if (index == m_nTailFreeItemIndex - 1) {
				m_nTailFreeItemIndex--;
			}
			if (index < m_nHeadFreeItemIndex) {
				m_nHeadFreeItemIndex = index;
			}
			m_nSize--;
		};
		inline int GetUsedSize() { return m_nSize; };
		inline int GetFreeSize() { return 254 - m_nSize; };
		inline VoxelOctreeNode& SafeCreateNode(const VoxelOctreeNode* pCopyFromNode = NULL) {
			auto index = CreateNode(pCopyFromNode);
			if (index != 0xff) {
				return (*this)[index];
			}
			else {
				return (*this)[0];
			}
		}
		inline uint8_t CreateNode(const VoxelOctreeNode* pCopyFromNode = NULL) {
			if (m_nSize < 254) 
			{
				uint8_t index = m_nHeadFreeItemIndex;
				if(pCopyFromNode != 0)
					(*this)[m_nHeadFreeItemIndex] = *pCopyFromNode;
				else
					(*this)[m_nHeadFreeItemIndex].UnMarkDeleted();
				m_nSize++;
				
				if (index == m_nTailFreeItemIndex)
				{
					m_nTailFreeItemIndex++;
					m_nHeadFreeItemIndex++;
				}
				else
				{
					for (int i = index + 1; i <= m_nTailFreeItemIndex; i++) {
						if ((*this)[i].IsDeleted()) {
							m_nHeadFreeItemIndex = i;
							break;
						}
					}
				}
				return index;
			}
			else
			{
				return 0xff;
			}
		}
		
	private:
		uint8_t m_nTailFreeItemIndex;
		uint8_t m_nHeadFreeItemIndex;
		uint8_t m_nSize;
	};

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

		/** the color of the parent node is the average of its children.
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

		void RemoveNodeChildren(VoxelOctreeNode* pNode, int isBlockMask = 0xff);

		void DumpOctree();
		void DumpOctreeNode(VoxelOctreeNode* pNode, int nDepth, int nChunkIndex, int offset);

		VoxelOctreeNode* GetRootNode();
	private:
		std::vector< VoxelChunk* > m_chunks;
	};
}

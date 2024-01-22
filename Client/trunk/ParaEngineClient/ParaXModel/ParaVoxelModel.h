#pragma once
#include "IAttributeFields.h"

namespace ParaEngine
{
	struct SceneState;
	class VoxelChunk;
	struct VoxelOctreeNode;

#pragma pack(push,1)
	/** a voxel octree data node
	* 1(isBlockMask)+1(isChildMask)+2(color)+4(base)+8(children) = 16 bytes
	* a chunk contains at most 256 nodes = 16*256 = 4096 bytes = 4KB
	* 
	* A node is a block node, if it is a leaf node or there are over 4 blocks in the 8 children.
	* A block node is rendered as cube at its level of detail regardless of its completely full or only half full. 
	* 
	* if the node has both child nodes and non-child blocks, the color is the color of the non-child blocks.
	* if the node has only child nodes, the color is the average color of the its child blocks.
	*/
	struct VoxelOctreeNode
	{
		/** default to create a full leaf block */
		VoxelOctreeNode(uint8_t isBlockMask = 0xff);

		// 8 bits whether the 8 spaces in octree are blocks. 
		uint8_t isBlockMask;
		// 8 bits whether the 8 spaces in octree are non-leaf node. 
		uint8_t isChildMask;
		// 16 bits for color: 5 bits for each of the 3 color channels. the high 1 bit is not used.
		uint16_t colorRGB;
		// low 24 bits for base chunk offset, high 8 bits for voxel shape. 
		uint32_t baseChunkOffset;
		union {
			// for non-leaf node, this is the offset to the chunk. 
			uint8_t childOffsets[8]; 
			// for leaf node, this is the child's voxel shape.
			uint8_t childVoxelShape[8];
			// obsoleted
			uint64_t childMask; 
		};

		static VoxelOctreeNode EmptyNode;
		static VoxelOctreeNode FullNode;
	public:
		inline bool IsLeaf() { return childMask == 0xffffffffffffffff; };
		inline bool IsBlock() { return IsLeaf() || (GetBlockCountInMask() >= 4); }
		inline bool IsBlockAt(uint8_t index) { return isBlockMask & (1 << index);  }
		inline bool IsSolid() { return isBlockMask == 0xff; };
		inline bool IsEmpty() { return isBlockMask == 0x0; };
		inline bool IsFullySolid() { return IsSolid() && IsLeaf(); };

		inline uint32 GetColor32() { return (colorRGB&0x1f)<<3 | ((colorRGB&0x3e0) << 11) | ((colorRGB&0x7c00) << 19); };
		inline void SetColor32(uint32 color) { colorRGB = (uint16_t)((color & 0xf8) >> 3 | ((color & 0xf800) >> 6) | ((color & 0xf80000) >> 9)); };
		inline uint16 GetColor() { return colorRGB; };
		inline void SetColor(uint16 color) { colorRGB = color; };
		inline uint8_t GetColor0() { return (colorRGB & 0x1f); };
		inline uint8_t GetColor1() { return ((colorRGB & 0x3e0) >> 5); };
		inline uint8_t GetColor2() { return ((colorRGB & 0x7c00) >> 10); };
		inline void SetColor0(uint8_t value) { colorRGB = (colorRGB & 0x7fe0) | value; };
		inline void SetColor1(uint8_t value) { colorRGB = (colorRGB & 0x7c1f) | (value << 5); };
		inline void SetColor2(uint8_t value) { colorRGB = (colorRGB & 0x3ff) | (value << 10); };
		
		// only 24 bits are used
		inline int GetBaseChunkOffset() { return baseChunkOffset & 0xffffff; };
		inline void SetBaseChunkOffset(uint32_t value) { baseChunkOffset = value & 0xffffff; };
		// 8 bits for voxel shape, 6 bits is for each of the 6 sides of the cube. If a bit is 1, the side is connecting to a solid.
		inline void SetVoxelShape(uint8_t shape) {
			baseChunkOffset |= (shape << 24);
		}
		inline uint8_t GetVoxelShape() {
			return uint8_t(baseChunkOffset >> 24);
		}

		inline void MakeEmpty() { isBlockMask = 0; };
		inline void MakeFullBlock() { isBlockMask = 0xff; };

		inline int GetBlockCountInMask() { 
			int count = 0;
			uint8_t n = isBlockMask;
			while (n) {
				n &= (n - 1);
				count++;
			}
			return count;
		};
		
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

	// only used internally when traversing the octree
	struct TempVoxelOctreeNodeRef
	{
		TempVoxelOctreeNodeRef(VoxelOctreeNode* pNode, int32 x, int32 y, int32 z, int level) :pNode(pNode), x(x), y(y), z(z), level(level) {};
		TempVoxelOctreeNodeRef():pNode(NULL), x(0), y(0), z(0), level(0) {};
		VoxelOctreeNode* pNode;
		int32 x;
		int32 y;
		int32 z;
		int level;
	};


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
		ATTRIBUTE_METHOD1(ParaVoxelModel, PaintBlock_s, char*) { cls->PaintBlockCmd(p1); return S_OK; }
		
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

		void PaintBlock(uint32 x, uint32 y, uint32 z, int level, uint32_t color);
		void PaintBlockCmd(const char* cmd);

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
		
	protected:
		/** optimize the model to remove and merge octree node. */
		void Optimize();
		void OptimizeNode(VoxelOctreeNode* pNode);

		/** update isBlockMask, color, and voxel shape of the node, and also changing its parent and possible siblings.
		* one needs to call this function when a node is changed to update all the way to the root node.
		* @param nodes: update nodes from nodes[nNodeCount - 1](smallest child) to nodes[0] (root node). 
		*/
		void UpdateNode(TempVoxelOctreeNodeRef nodes[], int nNodeCount);
		void UpdateNodeShape(const TempVoxelOctreeNodeRef& node);
		
		/** get the depth of the octree at the given level. 
		* e.g. LevelToDepth(1024) == 10
		*/
		inline int LevelToDepth(int level);

		int CreateGetFreeChunkIndex(int nMinFreeSize = 8);

		VoxelOctreeNode* GetNode(int32 x, int32 y, int32 z, int level);
		VoxelOctreeNode* CreateGetChildNode(VoxelOctreeNode* pNode, int nChildIndex);
		inline VoxelOctreeNode* GetChildNode(VoxelOctreeNode* pNode, int nChildIndex);

		void RemoveNodeChildren(VoxelOctreeNode* pNode, uint8_t isBlockMask = 0xff);
		/** set color to the node and all of its children 
		* @return true if pNode is fully solid node.
		*/
		bool SetNodeColor(VoxelOctreeNode* pNode, uint32 color);

		void DumpOctree();
		void DumpOctreeNode(VoxelOctreeNode* pNode, int nDepth, int nChunkIndex, int offset);

		VoxelOctreeNode* GetRootNode();

		inline uint8_t GetOppositeSide(uint8_t nSide);
	private:
		std::vector< VoxelChunk* > m_chunks;
	};
}

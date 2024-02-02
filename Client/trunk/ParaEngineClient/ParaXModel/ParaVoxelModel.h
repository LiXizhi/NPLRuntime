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
	* A node is a block node, if it is a leaf node or there are 1 block in the 8 children.
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
		// lower 23 bits for base chunk offset, 
		// the 24th bit is for whether the node is a fully solid node, 
		// high 8 bits for voxel shape. 
		uint32_t baseChunkOffset;
		union {
			// for non-leaf node, this is the offset to the chunk. 
			uint8_t childOffsets[8];
			// for leaf node, this is the child's voxel shape.
			uint8_t childVoxelShape[8];
			// just for assignment
			uint64_t offsetAndShape;
		};

		static VoxelOctreeNode EmptyNode;
		static VoxelOctreeNode FullNode;
	public:
		inline bool IsLeaf() { return isChildMask == 0; };
		inline bool IsBlock() { return isBlockMask != 0x0; /*(GetBlockCountInMask() >= 4);*/ }
		inline bool IsBlockAt(uint8_t index) { return isBlockMask & (1 << index); }
		inline bool IsSolid() { return isBlockMask == 0xff; };
		// if the 4 child blocks on the given side are neither all empty or all blocks.
		// @param nSide: 0~5, one of the 6 sides of the cube.
		inline bool IsSideSplited(int nSide);
		/** return true if there is a child block that is not a child node*/
		inline bool HasNonChildNodeBlock() {
			return (~isChildMask) & isBlockMask;
		};
		inline bool IsEmpty() { return isBlockMask == 0x0; };
		// if fully opache. but it may contain child nodes with different colors.
		inline bool IsFullySolid() { return (baseChunkOffset & 0x800000); };
		inline void SetFullySolid(bool bOn) {
			baseChunkOffset = (baseChunkOffset & 0xff7fffff) | (bOn ? 0x800000 : 0);
		};
		inline bool IsChildAt(uint8_t index) { return isChildMask & (1 << index); }
		inline void SetChild(uint8_t index, uint8_t offset) {
			isChildMask |= (1 << index);
			childOffsets[index] = offset;
		};
		inline void RemoveChild(uint8_t index) { isChildMask &= ~(1 << index); childVoxelShape[index] = 0; };

		inline uint32 GetColor32() { return (colorRGB & 0x1f) << 3 | ((colorRGB & 0x3e0) << 6) | ((colorRGB & 0x7c00) << 9); };
		inline void SetColor32(uint32 color) { colorRGB = (uint16_t)((color & 0xf8) >> 3 | ((color & 0xf800) >> 6) | ((color & 0xf80000) >> 9)); };
		inline uint16 GetColor() { return colorRGB; };
		inline void SetColor(uint16 color) { colorRGB = color; };
		inline uint8_t GetColor0() { return (colorRGB & 0x1f); };
		inline uint8_t GetColor1() { return ((colorRGB & 0x3e0) >> 5); };
		inline uint8_t GetColor2() { return ((colorRGB & 0x7c00) >> 10); };
		inline void SetColor0(uint8_t value) { colorRGB = (colorRGB & 0x7fe0) | value; };
		inline void SetColor1(uint8_t value) { colorRGB = (colorRGB & 0x7c1f) | (value << 5); };
		inline void SetColor2(uint8_t value) { colorRGB = (colorRGB & 0x3ff) | (value << 10); };

		// only lower 23 bits are used, which is over 32GB data at most.
		inline int GetBaseChunkOffset() { return baseChunkOffset & 0x7fffff; };
		inline void SetBaseChunkOffset(uint32_t value) { baseChunkOffset = (baseChunkOffset & 0xff800000) | (value & 0x7fffff); };
		// 8 bits for voxel shape, 6 bits is for each of the 6 sides of the cube. If a bit is 1, the side is connecting to a solid.
		inline void SetVoxelShape(uint8_t shape) {
			baseChunkOffset = ((shape & 0x3f) << 24) | (baseChunkOffset & 0xffffff);
		}
		inline uint8_t GetVoxelShape() {
			return uint8_t((baseChunkOffset >> 24) & 0x3f);
		}
		/** a single bit to mark if the node has the same voxel shape as its 8 children.
		* we should render the node directly instead of its children if this bit is set.
		* this bit is on usually for leaf colored node that has not been splitted by neighbouring nodes.
		*/
		inline bool IsSingleShape() { return uint8_t((baseChunkOffset >> 24) & 0x40); };
		inline void SetSingleShape(bool bOn) {
			baseChunkOffset = (baseChunkOffset & 0xbfffffff) | (bOn ? 0x40000000 : 0);
		};

		inline void MakeEmpty() { isBlockMask = 0; SetFullySolid(false); };
		inline void MakeFullBlock() { isBlockMask = 0xff; SetFullySolid(true); };
		inline bool IsChildSameShapeAsParent();
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
		TempVoxelOctreeNodeRef(VoxelOctreeNode* pNode, int32 x, int32 y, int32 z, uint32 level, uint8 childIndex_ = 0) :pNode(pNode), x(x), y(y), z(z), level(level), childIndex(childIndex_) {};
		TempVoxelOctreeNodeRef() :pNode(NULL), x(0), y(0), z(0), level(0) {};
		VoxelOctreeNode* pNode;
		int32 x;
		int32 y;
		int32 z;
		uint32 level;
		uint8 childIndex;
	};
#define MAX_VOXEL_CHUNK_SIZE 256

	/** a chunk of at most MAX_VOXEL_CHUNK_SIZE(256) octree nodes. 
	* non-editable chunks are allocated with just enough memory to hold the static octree.
	* editable chunks are pre-allocated with 256 nodes to allow quick editing.
	*/
	class VoxelChunk : public std::vector<VoxelOctreeNode>
	{
	public:
		/* the capacity of the chunk. usually 256 for editable chunks. it can not be bigger than 256. */
		VoxelChunk(int nCapacity = MAX_VOXEL_CHUNK_SIZE) :m_nSize(0), m_nFirstFreeItemIndex(0) {
			resize(nCapacity);
			for (int i = 0; i < nCapacity; i++) {
				(*this)[i].MarkDeleted();
			}
		};
		~VoxelChunk() {};

		void MakeEditable() {
			auto nLastSize = size();
			if (nLastSize < MAX_VOXEL_CHUNK_SIZE)
			{
				resize(MAX_VOXEL_CHUNK_SIZE);
				for (int i = nLastSize; i < MAX_VOXEL_CHUNK_SIZE; i++) {
					(*this)[i].MarkDeleted();
				}
			}
		}

		void erase(int index) {
			(*this)[index].MarkDeleted();
			if (index < m_nFirstFreeItemIndex) {
				m_nFirstFreeItemIndex = index;
			}
			m_nSize--;
		};
		inline int GetUsedSize() { return m_nSize; };
		inline int GetFreeSize() { return (int)(size() - m_nSize); };
		inline VoxelOctreeNode& SafeCreateNode(const VoxelOctreeNode* pCopyFromNode = NULL) {
			auto index = CreateNode(pCopyFromNode);
			return (*this)[index];
		}
		int GetDiskSize() { 
			int nSize = (int)size();
			for (int i = nSize - 1; i >= m_nSize; i--) {
				if (!(*this)[i].IsDeleted()) {
					return i + 1;
				}
			}
			return m_nSize;
		};
		uint8_t CreateNode(const VoxelOctreeNode* pCopyFromNode = NULL) {
			int nCapacity = (int)size();
			if (m_nSize < nCapacity)
			{
				auto index = m_nFirstFreeItemIndex;
				if (pCopyFromNode != 0)
					(*this)[m_nFirstFreeItemIndex] = *pCopyFromNode;
				else
					(*this)[m_nFirstFreeItemIndex].UnMarkDeleted();
				m_nSize++;

				for (int i = index + 1; i < nCapacity; i++) {
					if ((*this)[i].IsDeleted()) {
						m_nFirstFreeItemIndex = i;
						return (uint8_t)index;
					}
				}
				m_nFirstFreeItemIndex = nCapacity;
				return (uint8_t)index;
			}
			assert(false);
			return 0;
		}
		void LoadFromBuffer(const char* pBuffer, int nSize)
		{
			resize(nSize);
			if (nSize > 0) {
				memcpy(data(), pBuffer, nSize * sizeof(VoxelOctreeNode));
			}
			m_nSize = 0;
			m_nFirstFreeItemIndex = nSize;
			for (int i = 0; i < nSize; i++) {
				if (!(*this)[i].IsDeleted()) {
					m_nSize++;
				}
				else if(i < m_nFirstFreeItemIndex){
					m_nFirstFreeItemIndex = i;
				}
			}
		}
	private:
		uint16_t m_nFirstFreeItemIndex;
		uint16_t m_nSize;
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

		ATTRIBUTE_METHOD1(ParaVoxelModel, LoadFromFile_s, char*) { cls->LoadFromFile(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVoxelModel, SaveToFile_s, char*) { cls->SaveToFile(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaVoxelModel, SetBlock_s, char*) { cls->SetBlockCmd(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVoxelModel, PaintBlock_s, char*) { cls->PaintBlockCmd(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVoxelModel, RunCommandList_s, char*) { cls->RunCommandList(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaVoxelModel, GetMinVoxelPixelSize_s, float*) { *p1 = cls->GetMinVoxelPixelSize(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVoxelModel, SetMinVoxelPixelSize_s, float) { cls->SetMinVoxelPixelSize(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaVoxelModel, IsEditable_s, bool*) { *p1 = cls->IsEditable(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVoxelModel, SetEditable_s, bool) { cls->SetEditable(p1); return S_OK; }

		ATTRIBUTE_METHOD(ParaVoxelModel, DumpOctree_s) { cls->DumpOctree(); return S_OK; }

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

	public:
		/** load the model from a binary buffer. */
		bool Load(const char* pBuffer, int nCount = -1);
		bool LoadFromFile(const char* filename);

		/** save the model to a binary buffer. */
		bool Save(std::vector<char>& output);
		bool SaveToFile(const char* filename);
		

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
		/** run voxel command list: it is in the format of cmd name, cmd param, cmd param, ...
		* e.g. "setblock 0,0,0,1,-1 level 8 color #ff0000 set 0,0,0,1,1,1,0,1,0"
		* @note: one can also call this function multiple times to separate a long command like:
		*   first send "offset 4,4,4 level 8 color #ff0000 setwithoffset", then "0,0,0,1,1,1,0,1,0".  it will cache previous states of cmd, level and color.
		*   this allows one to reuse long command list string with different offset, level, and color.
		* offset x,y,z: e.g. "offset 2,0,0", all position in the following commands will be offset by 2,0,0.
		* level l: e.g. "level 8", all positions in the following commands will be at level 8.
		* color c: e.g. "color #ff0000", "color -1", all colors in the following commands will be set to 0xff0000.
		* del x1,y1,z1,x2,y2,z2, ... : e.g. "del block at all given positions at predefined level. 
		* delwithoffset x1,y1,z1,x2,y2,z2, ... : e.g. "del block at all given positions at predefined level. 
		* set x1,y1,z1,x2,y2,z2, ... : e.g. "set block at all given positions at predefined level and color.
		* setwithoffset x1,y1,z1,x2,y2,z2, ... : e.g. "set block at all given positions at predefined offset, level, and color.
		* setxyzcolor x1,y1,z1,color1,x2,y2,z2,color2, ... : e.g. "set block at all given positions at predefined level.
		* setblock x,y,z,level,color,... : e.g. "setblock 0,0,0,1,-1". set the block at (0,0,0) at level 1 to empty.
		* setrect fromX,fromY,fromZ,toX,toY,toZ: e.g. "color 0;level 64;setrect 0,0,0,63,63,0". with current color and level.
		* paint x1,y1,z1,x2,y2,z2, ... : e.g. "paint block at all given positions at predefined level and color.
		* paintwithoffset x1,y1,z1,x2,y2,z2, ... : e.g. "paint block at all given positions at predefined level and color.
		* paintblock x,y,z,level,color,... : e.g. "paintblock 0,0,0,1,#ff0000". paint all blocks to red. 
		* paintxyzcolor x1,y1,z1,color1,x2,y2,z2,color2, ... : e.g. "paint block at all given positions at predefined level.
		* paintrect fromX,fromY,fromZ,toX,toY,toZ,color1,color2,...: e.g. "paintrect 0,0,0,63,63,0,#ff#ff00#ffff00(...64*64 color values)". 
		*    if for example fromX is bigger than toX, the rect will be painted flipped on x axis, this allow applys to y and z. 
		*/
		void RunCommandList(const char* cmd);

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

		/* how many pixels that the smallest voxel should occupy on screen. [1,4] are reasonable values.
		* the smaller the value, the finer the voxel model will be rendered.
		*/
		void SetMinVoxelPixelSize(float fMinVoxelPixelSize);
		float GetMinVoxelPixelSize();

		/** if the model is editable. when model is first loaded, its chunks only allocate enough memory to hold the static octree, without pre-allocating memory for the voxel chunks.
		* when the model is editable, we will pre-allocate memory for the voxel chunks to full size to allow quick editing.
		* usually this is false when model first loaded from disk, and set to true automatically when user edited the model.
		*/
		void SetEditable(bool bEditable);
		bool IsEditable();

	protected:
		/** optimize the model to remove and merge octree node for invisible nodes. */
		void Optimize();
		void OptimizeNode(VoxelOctreeNode* pNode);

		/**
		* update isBlockMask, color, and isFullySolid property for this and all of its parent nodes.
		* the above property has nothing to do with their neighouring nodes.
		* whenever a node is changed, calling this function immediately to update all the way to the root node, to ensure all these	properties are correct.
		*/
		void UpdateNodeParentsSolidityAndColor(TempVoxelOctreeNodeRef nodes[], int nNodeCount);
		void UpdateNodeParentsColor(TempVoxelOctreeNodeRef nodes[], int nNodeCount);

		/** suppose the node at the position is changed, call this function to update all affected blocks' shape in the scene.
		* @note: the block at the given position should be either a fully solid block or empty block.
		* this function is usually called when you just set a given node to empty or fully solid.
		*/
		void UpdateNodeShape(uint32 x, uint32 y, uint32 z, int level);

		/** update a given node and all of its child nodes that are adjacent to a given side of a fully solid or empty neighbour node.
		* @param isSolidOrEmpty: whether the neighbour node is fully solid or fully empty.
		* if the neighbour node is not fully solid or empty, this function should not be called.
		* Hence, we only call this function of the neighbour node that has just been set by SetBlock()
		*/
		void UpdateNodeAndChildShapeByNeighbour(int32 x, int32 y, int32 z, int level, int side, bool isSolidOrEmpty);
		void UpdateNodeShapeByNeighbour(int32 x, int32 y, int32 z, int level, int side, bool isSolidOrEmpty, bool IsSideSplited);

		/** we will try to merge or split fully solid node at the given node and all its parents, according to their neighouring nodes. */
		void MergeNodeAndParents(int32 x, int32 y, int32 z, int level);
		/** when a node is changed, call this function to make sure all affected fully solid nodes in the scene are merged or splitted. */
		void MergeNodeAndNeighbours(int32 x, int32 y, int32 z, int level);

		/** the input node should be a solid leaf node. we will try to split it into 8 child nodes if its neighbours has holes on its sides.
		* @return true if the node is splitted. false if the node is not splitted (should be merged instead).
		*/
		bool SplitSolidNode(TempVoxelOctreeNodeRef* node);
		/** check if a given node and all of its child nodes that are adjacent to a given side of node has holes on the side.
		*/
		bool HasHolesOnSide(int32 x, int32 y, int32 z, int level, int side);

		/** no recursion, update just this node and its neighour at the given level.
		* blocks at lower or higher levels are ignored.
		*/
		void UpdateNodeShapeAtLevel(VoxelOctreeNode* pNode, uint32 x, uint32 y, uint32 z, int level);
		/** no recursion, update just this node.
		* return true if neighour node is a block
		*/
		bool UpdateNodeShapeByNeighbourAtLevel(int32 x, int32 y, int32 z, int level, int side, bool isBlock);


		/** if the node at the given position and level is a block node. */
		bool IsBlock(int32 x, int32 y, int32 z, int level);

		/** get the depth of the octree at the given level.
		* e.g. LevelToDepth(1024) == 10
		*/
		inline int LevelToDepth(int level);

		int CreateGetFreeChunkIndex(int nMinFreeSize = 8);

		VoxelOctreeNode* GetNode(int32 x, int32 y, int32 z, int level);
		VoxelOctreeNode* CreateGetChildNode(VoxelOctreeNode* pNode, int nChildIndex);
		inline VoxelOctreeNode* GetChildNode(VoxelOctreeNode* pNode, int nChildIndex);
		void DeleteNode(uint32_t chunkIndex, uint8_t nodeIndex);

		void RemoveNodeChildren(VoxelOctreeNode* pNode, uint8_t isBlockMask = 0xff);
		/** set color to the node and all of its children
		* @return true if pNode is fully solid node.
		*/
		bool SetNodeAndChildColor(VoxelOctreeNode* pNode, uint32 color);

		void DumpOctree();
		void DumpOctreeNode(VoxelOctreeNode* pNode, int nDepth, int nChunkIndex, int offset, int x, int y, int z);

		VoxelOctreeNode* GetRootNode();

		inline uint8_t GetOppositeSide(uint8_t nSide);

		// return level of detail at the given distance from camera. 1 means a cube, the bigger the higher resolition. 
		// usually between [1, 12].  12 is max LOD depth allowed, which is 4096*4096*4096.
		int GetLodDepth(float fCameraObjectDist, float fScaling = 1.f);
	private:

		std::vector< VoxelChunk* > m_chunks;
		// this is just an estimate of the smallest chunk that has a node just deleted. It just help to make the CreateGetFreeChunkIndex() faster.
		uint32_t m_nFirstFreeChunkIndex;

		// how many pixels that the smallest voxel should occupy on screen. [1,4] are reasonable values. 
		// the smaller the value, the finer the voxel model will be rendered.
		float m_fMinVoxelPixelSize;
		// if the model is editable. when model is first loaded, its chunks only allocate enough memory to hold the static octree, without pre-allocating memory for the voxel chunks.
		// when the model is editable, we will pre-allocate memory for the voxel chunks to full size to allow quick editing.
		bool m_bIsEditable;
	};
}

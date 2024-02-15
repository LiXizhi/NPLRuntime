#pragma once
#include <vector>
#include <set>
#include "BlockTemplate.h"

namespace ParaEngine
{
	class VerticalChunkIterator;
	class BlockRegion;
	class BlockChunk;
	class BlockTemplate;
	class CBlockWorld;

	class Block
	{
	public:
		friend class BlockChunk;

		Block() :m_pTemplate(nullptr), m_blockData(0), m_nInstanceCount(0)
		{
		}
		Block(BlockTemplate *pTemplate, uint32_t nData = 0) :m_pTemplate(pTemplate), m_blockData(nData), m_nInstanceCount(0)
		{
		}

		inline BlockTemplate* GetTemplate()
		{
			return m_pTemplate;
		}

		inline uint16_t GetTemplateId()
		{
			return m_pTemplate->GetID();
		}

		inline int GetFaceShape(int nFaceIndex)
		{
			return m_pTemplate->GetFaceShape(nFaceIndex, m_blockData);
		}

		inline uint32_t GetUserData(){return m_blockData;}

		/* get the texture index into the main block template. */
		int32 GetTextureIndex();
	protected:
		inline void SetTemplate(BlockTemplate *pTemplate){
			m_pTemplate = pTemplate;
		}
		inline void SetUserData(uint32_t data){ 
			m_blockData = (uint16)data; 
		}
		void PushEmptySlotIndex(uint16 nIndex){
			m_pTemplate = NULL;
			m_blockData = nIndex;
			m_nInstanceCount = 0;
		}
		/* return the next empty slot index. */
		uint16 PopEmptySlot(){
			uint16 nNextEmptySlotIndex = m_blockData;
			m_blockData = 0;
			return nNextEmptySlotIndex;
		}
		
		bool IsEmptySlot(){
			return m_nInstanceCount == 0;
		}

		uint16 GetInstanceCount(){
			return m_nInstanceCount;
		}

		uint16 DecreaseInstanceCount(){
			--m_nInstanceCount;
			return m_nInstanceCount;
		}
		uint16 IncreaseInstanceCount(){
			++m_nInstanceCount;
			return m_nInstanceCount;
		}
	protected:
		BlockTemplate* m_pTemplate;
		uint16 m_blockData;
		uint16 m_nInstanceCount;
	};

	/** chunk data mask*/
	enum ChunkDataMask {
		ChunkDataMask_HasMaskData = 0x1<<31,
		ChunkDataMask_HasCustomData = 0x1<<30,
		ChunkDataMask_HasBlockData = 0x1<<29,
	};

	/**custom chunk data */
	enum ChunkCustomDataType {
		/** raw block index array. */
		ChunkCustomDataType_Blocks = 0,
		/** using skip 1 algorithm. */
		ChunkCustomDataType_Blocks_SkipOne,
		/** compress using delta. more space than SkipOne, but also much better than none-compressed */
		ChunkCustomDataType_Blocks_Delta,
		ChunkCustomDataType_Biomes,
		// compress using same integer algorithm
		ChunkCustomDataType_Biomes_SameInteger,
		ChunkCustomDataType_TimeStamp,
		// compress using same integer algorithm
		ChunkCustomDataType_TimeStamp_SameInteger,
		ChunkCustomDataType_Heightmap,
		ChunkCustomDataType_LightValues,
		ChunkCustomDataType_Tag,
		ChunkCustomDataType_Materials,
		// chunk data: following is 16*16*16 chunk data which may further contain custom data. 
		// this is always the last parent custom data since it does not specify size, and we will write to end of file.  
		ChunkCustomDataType_ChunksData = 100,
	};

	/** Block Light Data*/
	class LightData
	{
	public:
		inline LightData():m_value(0)
		{
		}

		uint8_t GetBrightness(bool isSunLight);

		void SetBrightness(uint8_t value,bool isSunLight);

		uint8_t GetMaxBrightness(CBlockWorld* pBlockWorld);

		/** load without dirty mask */
		void LoadBrightness(uint8_t sun, uint8_t torch);

		inline bool IsInfluencedBySun()
		{
			return (m_value & 0xF0) > 0;
		}
	
		inline bool IsZero(){ return m_value == 0; }
	private:
		//bit usage:[0,3] point light brightness,[4,7] sun light
		uint8 m_value;
	};

	/** Chunk is a 16*16*16 inside a region */
	class BlockChunk
	{
	public:
		/* 16*16*16 index for blocks (fixed sized at initialization). Index is -1 if block not exist. */
		std::vector<int16_t> m_blockIndices;
		
		/* set of indices of all light emitting blocks in current chunk. */
		std::set<uint16_t> m_lightBlockIndices;

		/** 16*16*16 of light data (fixed sized at initialization)*/
		std::vector<LightData> m_lightmapArray;

		/* indices, materialCount map of all blocks that has at least one external block material associated to its faces. */
		std::map<uint16_t, uint8_t> m_materialBlockIndices;

		/** key is face_index*4096+block_index; value is material id*/
		std::map<uint16_t, int32>  m_materialsKeyIdMap;

		// in world space 
		Uint16x3 m_minBlockId_ws;
		// in region space
		Uint16x3 m_minBlockId_rs;
		// in region space
		Uint16x3 m_chunkId_rs;

		/** total number of chunks */
		static int s_total_chunks;
	protected:
		// blocks pool that grows automatically as new blocks are added, removed. 
		std::vector<Block> m_blocks;
		uint32 m_nDirty;
		uint16 m_isBoundaryChunk;
		uint16 m_emptyBlockSlotIndex;
		BlockRegion*  m_ownerBlockRegion;
		int16_t m_packedChunkID;

		inline bool IsBoundaryChunk() const { return m_isBoundaryChunk>0; }
		void SetBoundaryChunk(bool val) { m_isBoundaryChunk = val ? 1:0; }
		/** find block in the m_blocks pool by template. return -1 if not found. */
		int16 FindBlock(BlockTemplate* pTemplate);
		int16 FindBlock(BlockTemplate* pTemplate, uint32 nData);
		void SetBlockEmpty(uint16_t nBlockIndex, Block& block);
		bool RecycleBlock(uint16 nIndex, Block& block);
		Block* CreateBlock(uint16_t nBlockIndex);
	public:
		BlockChunk(uint16_t nPackedChunkId, BlockRegion* pRegion);
		~BlockChunk();

		static int GetTotalChunksInMemory();

		// reserve blocks
		void ReserveBlocks(int nCount);

		/** get total number of memory bytes that this chunk occupies. for memory algorithm or stats.*/
		int GetTotalBytes();

		/** blocks higher than the highest solid block in the height map can sky. Note, the top most opaque block can not see the sky. 
		* @param x,y,z: in world space
		*/
		bool CanBlockSeeTheSkyWS(uint16 x, uint16 y, uint16 z);

		uint32 GetBlockCount();

		inline bool IsDirty() const { return m_nDirty > 0; }
		/** set dirty by block changes in this chunk */
		void SetDirty(bool val);
		/** set dirty by neighbor block change. */
		void SetDirtyByNeighbor();
		void SetLightDirty();
		/** whether this chunk is made dirty by neighbor other than content change. */
		bool IsDirtyByNeighbor();
		/** whether this chunk is made dirty by light than content change. */
		bool IsDirtyByLight();
		/** whether this chunk is made dirty by actual block changes. */
		bool IsDirtyByBlockChange();

		void SetLightingInitialized(bool bInitialized);
		bool IsLightingInitialized();
		// reset all blocks
		void Reset();

		/** the caller should ensure that nBlockIndex is valid. */
		Block& GetBlockByIndex(uint16_t nBlockIndex);

		/** get block 
		* @param nBlockIndex: [0,4096)
		*/
		Block* GetBlock(uint16_t nBlockIndex);
		
		/** load one or more blocks of a given block type. usually called when chunk is loaded from file 
		* return the number of blocks created. 
		*/
		int LoadBlocks(const std::vector<uint16_t>& blockIndices, BlockTemplate* pTemplate);

		Uint16x3 GetBlockPosRs(uint16 nBlockIndex);

		void UpdateHeightMapAtLoadTime(uint16_t nBlockIndex, bool isTransparent);

		void LoadBlock(uint16_t nBlockIndex, BlockTemplate* pTemplate);
		void SetBlockTemplate(uint16_t nBlockIndex, BlockTemplate* pTemplate);

		BlockTemplate* GetBlockTemplate(uint16_t nBlockIndex);
		void SetBlockData(uint16_t nBlockIndex, uint32 nData);
		/** set block template and data */
		void SetBlock(uint16_t nBlockIndex, BlockTemplate* pTemplate, uint32 nData);

		/** get container block world */
		CBlockWorld* GetBlockWorld();

		/** set block to empty
		* @param blockId_r: region space 
		*/
		bool SetBlockToAir(Uint16x3& blockId_r);

		/** remove light
		* @param blockId_r: region space 
		*/
		bool RemoveLight(Uint16x3& blockId_r);

		/** add light */
		void AddLight(Uint16x3& blockId_r);
		void AddLight(uint16 nPackedBlockID);

		/** get the light data. may return NULL. 
		* @param nIndex: parameters
		*/
		LightData* GetLightData(uint16_t nIndex);

		/** is any block influenced by sun light. */
		bool IsInfluenceBySunLight();

		//This is a approximate function to determine if block is visible
		//for non-transparent-liquid block return true if :
		//1. all neighbors are solid 
		//2. block is at the boundary of region
		//for transparent-liquid block return true if any neighbor is not the same
		//kind template.
		//@param index: packed chunk space block index [0,4096)
		bool IsVisibleBlock(int32_t index,Block* pBlock = NULL);

		/**
		@param pBlockData: Block* neighborBlocks[27] depending on nFrom, nTo;
		@param nFrom, nTo: default to query all 27 neighbor. Other frequently used ones are 1,6 (for six adjacent ones)
		*/
		void QueryNeighborBlockData(const Uint16x3& blockId_cs,Block** pBlockData, int nFrom = 0, int nTo = 26);

		/** return true if the nearby 4 chunk columns have been generated (and loaded) */
		bool IsNearbyChunksLoaded();

		/** return the blocks need to re notification */
		std::vector<Uint16x3> refreshBlockVisible(uint16_t blockTemplateId);

		void ClearAllLight();
		void ClearLightMap();

		/** check if a given block contains at least one block material */
		bool HasBlockMaterial(Uint16x3& blockId_r);
		bool HasBlockMaterial(uint16 nPackedBlockID);

		/** get material id for given block's given face
		* @param nFaceIndex: [0,5) 6 faces of a block 
		* @return -1 if not found
		*/
		int32 GetBlockFaceMaterial(uint16 nPackedBlockID, int16 nFaceIndex);

		/** Apply a given block material to a block face. 
		* @param nFaceIndex: [0,5) 6 faces of a block 
		* @param nMaterialID: global unique material id in CBlockMaterialManager
		*/
		void ApplyBlockMaterial(uint16 nPackedBlockID, int16 nFaceIndex, int32 nMaterialID);

		/** Remove a given block material from a block face.
		* @param nFaceIndex: if -1, we will remove all materials from the block
		* @param nMaterialID: global unique material id in CBlockMaterialManager
		*/
		void RemoveBlockMaterial(uint16 nPackedBlockID, int16 nFaceIndex);
	};
}
#pragma once
#include <stdint.h>
#include <vector>
#include <thread>
#include "BlockReadWriteLock.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "IAttributeFields.h"
#include "ChunkMaxHeight.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
namespace ParaEngine
{
	class VerticalChunkIterator;
	class BlockRegion;
	class BlockChunk;

	/**
	* 512*512 region
	*/
	class BlockRegion : public IAttributeFields
	{
	public:
		BlockRegion(int16_t regionX, int16_t regionZ, CBlockWorld* pBlockWorld);

		virtual ~BlockRegion();

		ATTRIBUTE_DEFINE_CLASS(BlockRegion);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(BlockRegion, GetRegionX_s, int*) { *p1 = cls->GetRegionX(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, GetRegionZ_s, int*) { *p1 = cls->GetRegionZ(); return S_OK; }

		ATTRIBUTE_METHOD1(BlockRegion, GetChunksLoaded_s, int*) { *p1 = cls->GetChunksLoaded(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, SetChunksLoaded_s, int) { cls->SetChunksLoaded((uint32)p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockRegion, IsLocked_s, bool*) { *p1 = cls->IsLocked(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, SetLocked_s, bool) { cls->SetLocked(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockRegion, IsModified_s, bool*) { *p1 = cls->IsModified(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, SetModified_s, bool) { cls->SetModified(p1); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, LoadFromFile_s, const char*) { cls->LoadFromFile(p1); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, SaveToFile_s, const char*) { cls->SaveToFile(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(BlockRegion, GetTag_s, const char**) { *p1 = cls->GetTag().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, SetTag_s, const char*) { cls->SetTag(p1); return S_OK; }


		ATTRIBUTE_METHOD(BlockRegion, DeleteAllBlocks_s) { cls->DeleteAllBlocks(); return S_OK; }
		
		ATTRIBUTE_METHOD1(BlockRegion, GetTotalBytes_s, int*) { *p1 = cls->GetTotalBytes(); return S_OK; }


		ATTRIBUTE_METHOD(BlockRegion, ClearAllLight_s) { cls->ClearAllLight(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockRegion, RefreshLightChunkColumns_s, Vector3) { cls->RefreshLightChunkColumns((uint16_t)p1.x, (uint16_t)p1.y, (uint16_t)p1.z); return S_OK; }

	public:
		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		/* if locked, all block access functions takes no effect. Since it is either being loaded or saved asynchronously. */
		bool IsLocked();
		void SetLocked(bool bLocked);

		/**
		@param x,z:  range in [0,512), y range in [0,256)
		@param bNeedUpdate: during loading from file, bNeedUpdate is false.
		*/
		void SetBlockTemplateByIndex(uint16_t x_rs, uint16_t y_rs, uint16_t z_rs, BlockTemplate* pTemplate);

		void RefreshBlockTemplateByIndex(uint16_t x_rs, uint16_t y_rs, uint16_t z_rs, BlockTemplate* pTemplate);

		uint32_t GetBlockTemplateIdByIndex(int16_t x, int16_t y, int16_t z);

		BlockTemplate* GetBlockTemplateByIndex(int16_t x, int16_t y, int16_t z);

		//user date is a uint32 value,
		//params are in region space,  x,z range in [0,512), y range in [0,256)
		void SetBlockUserDataByIndex(int16_t x, int16_t y, int16_t z, uint32_t data);

		uint32_t GetBlockUserDataByIndex(int16_t x, int16_t y, int16_t z);
		/** Remove a given block material from a block face.
		* @param nFaceIndex: if -1, we will remove all materials from the block
		* @param nMaterialID: global unique material id in CBlockMaterialManager
		*/
		bool SetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial);

		/** Remove a given block material from a block face.
		* @param nFaceIndex: if -1, we will remove all materials from the block
		* @return -1 if not found
		*/
		int32_t GetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId);
		
		/** set chunk column time stamp. usually 0 for non-generated. 1 for generated.
		this is usually called by world generators, so that we will not generate again next time we load the world. */
		void SetChunkColumnTimeStamp(uint16_t x_rs, uint16_t z_rs, uint16_t nTimeStamp);

		/** get the time stamp of for the given chunk column
		0 means not available, 1 means loaded before*/
		uint16_t GetChunkColumnTimeStamp(uint16_t x, uint16_t z);

		bool IntersectBlock(int16_t blockX, int16_t blockY, int16_t blockZ, uint32_t filter);

		//Get region x index
		inline int16_t GetRegionX() { return m_regionX; }

		//Get region Z index
		inline int16_t GetRegionZ() { return m_regionZ; }

		//Get the height of highest soiled block
		ChunkMaxHeight* GetHighestBlock(uint16_t blockIdX_rs, uint16_t blockIdZ_rs);

		//return neighbor state as a 32bit mask value.
		//if neighbor is not in current region we'll set the neighbor state as empty
		uint32_t QueryApproxNeighborBlockState(const Int16x3& blockPos);

		//return null if block not exist
		//note:do *not* hold a permanent reference of the return value,underlying
		//block address may change after SetBlockTemplate()!
		Block* GetBlock(uint16_t x_rs, uint16_t y_rs, uint16_t z_rs);

		void SaveToFile(const char* sFilename = NULL);

		void Load();

		// called every frame move 
		void OnFrameMove();

		void OnLoadWorldFinished();
		void OnUnloadWorld();

		void DeleteAllBlocks();

		void RefreshAllLightsInColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws);

		/** call this function when you make lots of changes to blocks when light is suspended. */
		void RefreshLightChunkColumns(uint16_t chunkX_ws, uint16_t chunkZ_ws, uint16_t chunksLength = 1);

		void GetBlocksInChunk(uint16_t chunkX_ws, uint16_t chunkZ_ws, uint16_t startChunkY, uint16_t endChunkY,
			uint32_t matchtype, const luabind::adl::object& result, int32_t& blockCount);
		void GetBlocksInChunk(uint16_t chunkX_ws, uint16_t chunkZ_ws, uint32_t verticalSectionFilter,
			uint32_t matchtype, const luabind::adl::object& result, int32_t& blockCount);

		const std::string& GetMapChunkData(uint32_t chunkX, uint32_t chunkZ, bool bIncludeInit, uint32_t verticalSectionFilter = 0xffff);

		void ApplyMapChunkData(uint32_t chunkX, uint32_t chunkZ, uint32_t verticalSectionFilter, const std::string& chunkData, const luabind::adl::object& output);

		// call this function when this chunk is modified
		void SetChunkDirty(uint16_t packedChunkID, bool isDirty);
		// this function is only called when neighbor block on the adjacent boundary to this chunk is dirty. 
		// This chunk may or may not be modified. 
		void SetChunkDirtyByNeighbor(uint16_t packedChunkID);
		void SetChunkLightDirty(uint16_t packedChunkID);

		inline uint16_t GetPackedRegionIndex()
		{
			return m_regionX + (m_regionZ << 6);
		}

		Block* GetBlock(uint16_t chunkId, Uint16x3& blockID_r);

		BlockChunk* GetChunk(uint16_t packedChunkID, bool createIfNotExist);

		/** whether modified. */
		bool IsModified();
		/** set modified. */
		void SetModified(bool bModified = true);

		uint32 GetChunksLoaded() const;
		void SetChunksLoaded(uint32 val);

		/** total number of bytes that this region occupies */
		int GetTotalBytes();
		void CalculateTotalBytes();

		/** get the world space position for the center block. */
		void GetCenterBlockWs(Uint16x3* pPos);
		const Uint16x3& GetMinBlockWs();
		const Uint16x3& GetMaxBlockWs();


		void ClearAllLight();

		/** refresh chunks include one type block*/
		void SetChunksDirtyByBlockTemplate(uint16_t templateId);

		/** this function is only valid, when the block region is locked. this function is extremely fast. and it does not do light calculation. */
		void LoadBlockAsync(uint16_t x_rs, uint16_t y_rs, uint16_t z_rs, uint16_t blockId, uint32_t userData);
	private:
		Block* CreateBlock(Uint16x3& blockID_r);

		//return false if block not exist
		bool SetBlockToAir(uint16_t chunkId, Uint16x3& blockId_r);


		BlockChunk* CreateNewChunk(uint16_t chunkIndex);

		bool IsChunkDirty(int16_t packedChunkID);

		void CheckNeighborChunkDirty(Uint16x3& blockId_rs);

		void SetNeighborChunkDirty(Uint16x3& neighborChunkId_ws);

		void UpdateBlockHeightMap(Uint16x3& blockId_rs, bool isRemove, bool isTransparent);

		CBlockWorld* GetBlockWorld();

		void LoadWorldThreadFunc();
		void LoadFromFile(const char* sFilename = NULL);

		void BeginRead();
		void EndRead();
		void BeginWrite();
		void EndWrite();

		/** get read write lock of this region. */
		BlockReadWriteLock& GetReadWriteLock();

		/** user defined custom data that can be set via scripting interface. */
		const std::string& GetTag();
		void SetTag(const std::string& sTag);
		
	private:
		void ParserFile(CParaFile* pFile);
		void ParserFile1_0(CParaFile* pFile);



		friend class BlockChunk;
		friend class RenderableChunk;

		typedef BlockChunk* BlockChunkPtr;
		// 32*32*16 chunks
		BlockChunkPtr* m_chunks;
		int GetChunksCount();

		/** 0 means not available, 1 means loaded before*/
		std::vector<unsigned char> m_chunkTimestamp;

		/** 512*512 biomes values*/
		std::vector<unsigned char> m_biomes;

		//store the highest block 
		//first is highest block,second is highest soiled block
		std::vector<ChunkMaxHeight> m_blockHeightMap;

		/** total number of bytes that this region occupies */
		int m_nTotalBytes;

		/** whether block is modified or not */
		bool m_bIsModified;

		Int16x3 m_minChunkId_ws;
		Int16x3 m_maxChunkId_ws;

		Uint16x3 m_minBlockId_ws;
		Uint16x3 m_maxBlockId_ws;

		int16_t m_regionX;
		int16_t m_regionZ;
		CBlockWorld* m_pBlockWorld;

		/* if locked, all block access functions takes no effect. Since it is either being loaded or saved asynchronously. */
		bool m_bIsLocked;
		/** read-write lock. */
		BlockReadWriteLock m_readWriteLock;

		std::thread m_thread;
		int32 m_nEventAsyncLoadWorldFinished;
		uint32 m_nChunksLoaded;

		std::string m_sName;

		/** user defined custom data that can be set via scripting interface. */
		std::string m_sTag;
	};


}
#pragma once
#include <vector>
#include <array>
#include <map>
#include <unordered_set>
#ifdef WIN32
#include "stdint.h"
#else
#include <stdint.h>
#endif
#include <bitset>
#include "IObjectScriptingInterface.h"
#include "IAttributeFields.h"

#include "util/mutex.h"
#include "BlockTemplate.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "RenderableChunk.h"
#include "BlockRenderTask.h"
#include "WorldInfo.h"
#include "BlockReadWriteLock.h"

namespace ParaEngine
{
	class BlockRegion;
	class RenderableChunk;
	class CBlockLightGridBase;
	class BlockRenderTask;
	struct BlockHeightValue;
	struct ChunkMaxHeight;


	/** base class for an instance of block world */
	class CBlockWorld : public IAttributeFields, IObjectScriptingInterface
	{
	public:
		CBlockWorld();
		virtual ~CBlockWorld();
	
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_BlockWorld; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "BlockWorld"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName);;
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);;
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount() { return 1; };
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CBlockWorld, GetBlockRenderMethod_s, int*)		{ *p1 = cls->GetBlockRenderMethod(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetBlockRenderMethod_s, int)	{ cls->SetBlockRenderMethod((BlockRenderMethod)p1); return S_OK; }

		ATTRIBUTE_METHOD(CBlockWorld, ResumeLightUpdate_s)	{ cls->ResumeLightUpdate(); return S_OK; }
		ATTRIBUTE_METHOD(CBlockWorld, SuspendLightUpdate_s)	{ cls->SuspendLightUpdate(); return S_OK; }

		ATTRIBUTE_METHOD(CBlockWorld, ResetAllLight_s) { cls->ResetAllLight(); return S_OK; }

		ATTRIBUTE_METHOD(CBlockWorld, LockWorld_s)	{ cls->LockWorld(); return S_OK; }
		ATTRIBUTE_METHOD(CBlockWorld, UnlockWorld_s)	{ cls->UnlockWorld(); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsLightUpdateSuspended_s, bool*)		{ *p1 = cls->IsLightUpdateSuspended(); return S_OK; }

		ATTRIBUTE_METHOD3(CBlockWorld, SetChunkColumnTimeStamp_s, float) { cls->SetChunkColumnTimeStamp((uint16_t)p1, (uint16_t)p2, (uint16_t)p3); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetRenderDist_s, int*)		{ *p1 = cls->GetRenderDist(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetRenderDist_s, int)	{ cls->SetRenderDist(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetDirtyColumnCount_s, int*)		{ *p1 = cls->GetDirtyColumnCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, GetDirtyBlockCount_s, int*)		{ *p1 = cls->GetDirtyBlockCount(); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsReadOnly_s, bool*)		{ *p1 = cls->IsReadOnly(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetReadOnly_s, bool)	{ cls->SetReadOnly(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsRemote_s, bool*)		{ *p1 = cls->IsRemote(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetIsRemote_s, bool)	{ cls->SetIsRemote(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsServerWorld_s, bool*)		{ *p1 = cls->IsServerWorld(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetIsServerWorld_s, bool)	{ cls->SetIsServerWorld(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsSaveLightMap_s, bool*)		{ *p1 = cls->IsSaveLightMap(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetSaveLightMap_s, bool)	{ cls->SaveLightMap(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsUseAsyncLoadWorld_s, bool*)		{ *p1 = cls->IsUseAsyncLoadWorld(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetUseAsyncLoadWorld_s, bool)	{ cls->SetUseAsyncLoadWorld(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetLightCalculationStep_s, int*)		{ *p1 = cls->GetLightCalculationStep(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetLightCalculationStep_s, int)	{ cls->SetLightCalculationStep(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsAsyncLightCalculation_s, bool*)		{ *p1 = cls->IsAsyncLightCalculation(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetAsyncLightCalculation_s, bool)	{ cls->SetAsyncLightCalculation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetMaxCacheRegionCount_s, int*)		{ *p1 = cls->GetMaxCacheRegionCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetMaxCacheRegionCount_s, int)	{ cls->SetMaxCacheRegionCount(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetNumOfLockedBlockRegion_s, int*)		{ *p1 = cls->GetNumOfLockedBlockRegion(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, GetNumOfBlockRegion_s, int*)		{ *p1 = cls->GetNumOfBlockRegion(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, GetTotalNumOfLoadedChunksInLockedBlockRegion_s, int*)		{ *p1 = cls->GetTotalNumOfLoadedChunksInLockedBlockRegion(); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, IsRenderBlocks_s, bool*)		{ *p1 = cls->IsRenderBlocks(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetRenderBlocks_s, bool)	{ cls->SetRenderBlocks(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetSunIntensity_s, float*)		{ *p1 = cls->GetSunIntensity(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetSunIntensity_s, float)	{ cls->SetSunIntensity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetMinWorldPos_s, Vector3*)		{ *p1 = Vector3((float)(cls->GetMinWorldPos().x), (float)(cls->GetMinWorldPos().y), (float)(cls->GetMinWorldPos().z)); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetMinWorldPos_s, Vector3)	{ Int32x3 v((int32)p1.x, (int32)p1.y, (int32)p1.z); cls->SetMinWorldPos(v); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, GetMaxWorldPos_s, Vector3*)	{ *p1 = Vector3((float)(cls->GetMaxWorldPos().x), (float)(cls->GetMaxWorldPos().y), (float)(cls->GetMaxWorldPos().z)); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetMaxWorldPos_s, Vector3)	{ Int32x3 v((int32)p1.x, (int32)p1.y, (int32)p1.z); cls->SetMaxWorldPos(v); return S_OK; }


		ATTRIBUTE_METHOD1(CBlockWorld, GetTotalChunksInMemory_s, int*)		{ *p1 = BlockChunk::GetTotalChunksInMemory(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, GetTotalRenderableChunksInMemory_s, int*)		{ *p1 = RenderableChunk::GetTotalRenderableChunks(); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockWorld, UseLinearTorchBrightness_s, bool)	{ cls->GenerateLightBrightnessTable(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, IsAutoPhysics_s, bool*)		{ *p1 = cls->IsAutoPhysics(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockWorld, SetAutoPhysics_s, bool)	{ cls->SetAutoPhysics(p1); return S_OK; }
		
	public:
		/** script call back type */
		enum CallBackType{
			Type_LoadBlockRegion = 0,
			Type_UnLoadBlockRegion,
			Type_GeneratorScript,
			Type_SaveRegionCallbackScript,
			Type_BeforeLoadBlockRegion,
		};
		DEFINE_SCRIPT_EVENT(CBlockWorld, BeforeLoadBlockRegion);
		DEFINE_SCRIPT_EVENT(CBlockWorld, LoadBlockRegion);
		DEFINE_SCRIPT_EVENT(CBlockWorld, UnLoadBlockRegion);
		/** NPL script to be called when a given region is flushed to disk*/
		DEFINE_SCRIPT_EVENT(CBlockWorld, SaveRegionCallbackScript);

		// when there is no block terrain to load from , this function will be used to generate new terrains. 
		DEFINE_SCRIPT_EVENT(CBlockWorld, GeneratorScript);

		/** called before block region is about to be loaded. If the function does not return 0, we will skip loading the block region. */
		int OnBeforeLoadBlockRegion(int x, int y);
		/** called when block region has just loaded. it will invoke the scripting interface if any. */
		int OnLoadBlockRegion(int x, int y);
		/** called when block region has just unloaded. it will invoke the scripting interface if any. */
		int OnUnLoadBlockRegion(int x, int y);
		/** called when block region has just saved. it will invoke the scripting interface if any. */
		int OnSaveBlockRegion(int x, int y);
	public:
		bool IsAutoPhysics();
		void SetAutoPhysics(bool bValue);
		/** get light grid */
		CBlockLightGridBase& GetLightGrid();

		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		//init block world
		virtual void EnterWorld(const string& sWorldDir, float x, float y, float z);

		//call this function when leave the block world
		virtual void LeaveWorld();

		void ClearAllBlockTemplates();

		/** how many chunk */
		void SetActiveChunkRadius(int nActiveChunkRadius = 12);

		void UpdateAllActiveChunks();

		/** minimum world position. mostly set to prevent running out of memory. default to 0,0,0. */
		const ParaEngine::Int32x3& GetMinWorldPos() const;
		void SetMinWorldPos(const ParaEngine::Int32x3& val);
		/** maximum world position. mostly set to prevent running out of memory. default to 0xffff,0xffff,0xffff. */
		const ParaEngine::Int32x3& GetMaxWorldPos() const;
		void SetMaxWorldPos(const ParaEngine::Int32x3& val);

		uint16_t GetMaxCacheRegionCount() const;
		void SetMaxCacheRegionCount(uint16_t val);

		/** how many lighting to calculate per tick for the lighting thread.
		* @param nTicks: default to 0. it will stop light calculation when some predefined lighting tasks is finished.
		* Otherwise, it will only stop either all tasks are finished or nTicks milliseconds have passed since it begins.
		*/
		virtual void SetLightCalculationStep(uint32 nTicks);
		virtual uint32 GetLightCalculationStep();

		/** get region object
		* @param x,y,z: in world space
		* @param rs_x,rs_y,rs_z: out: converted to region space.
		*/
		BlockRegion* GetRegion(uint16_t x, uint16_t y, uint16_t z, uint16_t& rs_x, uint16_t& rs_y, uint16_t& rs_z);
		BlockRegion* GetRegion(uint16_t region_x, uint16_t region_z);

		static float GetVerticalOffset();

		static void SetVerticalOffset(float offset);

		int32_t GetActiveChunkDim();

		int32_t GetActiveChunkDimY();

		bool IsInBlockWorld();

		/** whether it is readonly */
		void SetRenderBlocks(bool bValue);

		/** whether it is readonly */
		bool IsRenderBlocks();

		/** whether we will use async world loader. default to false. this is only true when region is first loaded. */
		bool IsUseAsyncLoadWorld() const;
		void SetUseAsyncLoadWorld(bool val);

		/** whether it is readonly */
		void SetReadOnly(bool bValue);

		/** whether it is readonly */
		bool IsReadOnly();

		/** if true, it will check dirty bit for all active chunks in the next the render framemove*/
		bool IsVisibleChunkDirty() const;
		void SetVisibleChunkDirty(bool val);

		/** whether it is a remote world. in such cases, we will stop loading from file. */
		void SetIsRemote(bool bValue);

		/** whether it is a remote world */
		bool IsRemote();

		/** whether it is a server world. in such cases, we will not unload region file. */
		void SetIsServerWorld(bool bValue);

		/** whether it is server world */
		bool IsServerWorld();

		/** if true, when rendering blocks, always group by chunk first and then by texture. if not we will group by texture first from all chunks.  */
		bool IsGroupByChunkBeforeTexture() const;
		void SetGroupByChunkBeforeTexture(bool val);

		/** when some thread is accessing the block world. It generally need to acquire this lock.
		* On client, the main thread will acquire the lock at beginning of scene/block animation/rendering and release it at the end.
		* So the main thread is occupied this block for a relative long time. Any other worker thread such as light calculation and terrain generation should wait for
		* this lock, and even it does successfully required the lock, they must ensure it is released immediately to not affecting the main rendering thread.
		*/
		ParaEngine::mutex& GetBlockWorldMutex();

		/** use scoped_lock where ever possible. this function is only called from scripting interface, where scoped_lock is not possible.
		* however script using this function must be very short and careful to call unlockworld in pairs;otherwise there will be dead lock.
		* Nested calls to this function in the same thread will not block but must be in pairs.
		*/
		void LockWorld();
		/** Unlock mutex */
		void UnlockWorld();
		/** get read/write locker of this world*/
		BlockReadWriteLock& GetReadWriteLock();

		/** whether to save light map to disk. Light map usually double the disk space,
		but increases map loading time, without the occasionally black blocks.
		*/
		bool IsSaveLightMap() const;
		void SaveLightMap(bool val);

		void SaveBlockTemplateData();

		void LoadBlockTemplateData();

		void SaveBlockMaterialData();
		void LoadBlockMaterialData();
		void SaveToFile(bool saveToTemp);

		/** return world info*/
		CWorldInfo& GetWorldInfo();

		//////////////////////////////////////////////////////////////////////////
		//template related function
		BlockTemplate* RegisterTemplate(uint16_t id, uint32_t attFlag, uint16_t category_id);

		/** return true if refresh world is required. */
		bool SetBlockVisible(uint16_t templateId, bool value, bool bRefreshWorld = true);
		/** refresh light and other attributes for all instances of given block template. */
		void RefreshBlockTemplate(uint16_t templateId);

		//set/get block id/data
		uint32_t SetBlockId(uint16_t x, uint16_t y, uint16_t z, uint32_t nBlockID);
		uint32_t GetBlockId(uint16_t x, uint16_t y, uint16_t z);
		uint32_t SetBlockData(uint16_t x, uint16_t y, uint16_t z, uint32_t nBlockData);
		uint32_t GetBlockData(uint16_t x, uint16_t y, uint16_t z);

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

		void LoadBlockAsync(uint16_t x, uint16_t y, uint16_t z, uint16_t blockId, uint32_t userData);

		//do *not* hold a permanent reference of return value,underlying memory address may change
		//@param id: template id;
		BlockTemplate* GetBlockTemplate(uint16_t id);

		//do *not* hold a permanent reference of return value,underlying memory address may change
		//@param x,y,z: world space block id
		BlockTemplate* GetBlockTemplate(uint16_t x, uint16_t y, uint16_t z);

		//do *not* hold a permanent reference of return value,underlying memory address may change
		BlockTemplate* GetBlockTemplate(Uint16x3& blockId_ws);

		//@param flag: can be one or more BlockAttrubiteFlag value.
		bool MatchTemplateAttribute(uint16_t templateId, BlockTemplate::BlockAttrubiteFlag flag);

		void SetTemplateTexture(uint16_t id, const char* textureName);
		void SetTemplatePhysicsProperty(uint16_t id, const char* property);
		const char* GetTemplatePhysicsProperty(uint16_t id);
		//get highest block, null if block not exist;
		//@return:first is highest soiled block, second is highest transparent block,second>= first
		ChunkMaxHeight* GetHighestBlock(uint16_t blockX_ws, uint16_t blockZ_ws);

		//get highest y values of current and neighbor blocks above terrain.
		//return terrain height if no block above ground.
		//@param result: int16_t[6] array contains the result height of {center,x,-x,z,-z,mask} 
		// each bit of mask indicate return value is block height or terrain height, 
		virtual void GetMaxBlockHeightWatchingSky(uint16_t blockX_ws, uint16_t blockZ_ws, ChunkMaxHeight* pResult);

		//////////////////////////////////////////////////////////////////////////
		//refresh all lights in chunk column,this function will recalc all point lights in a column
		void RefreshAllLightsInColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws);

		//called when block height map changed.This function will recalc sun light of block column
		void NotifyBlockHeightMapChanged(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, ChunkMaxHeight& prevBlockHeight);

		/** return the BlockChunk and the block index inside the chunk. */
		BlockIndex GetBlockIndex(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist = true);

		/** get chunk by world coordinates*/
		BlockChunk* GetChunk(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist = true);

		//////////////////////////////////////////////////////////////////////////
		//block manipulation function
		void SetBlockTemplateId(float x, float y, float z, uint16_t templateId);

		void SetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z, uint16_t templateId);

		uint16_t GetBlockTemplateId(float x, float y, float z);

		uint16_t GetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z);

		void SetBlockUserData(float x, float y, float z, uint32_t data);

		void SetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t data);

		uint32_t GetBlockUserData(float x, float y, float z);

		uint32_t GetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z);

		bool IsObstructionBlock(uint16_t x, uint16_t y, uint16_t z);


		//return null if block not exist
		//note:do *not* hold a permanent reference of the return value,underlying
		//block address may change after SetBlockTemplate()!
		Block* GetBlock(uint16_t x, uint16_t y, uint16_t z);
		Block* GetUnlockBlock(uint16_t x, uint16_t y, uint16_t z);

		//set chunk dirty will rebuild vertex buffer of the chunk
		void SetChunkDirty(Uint16x3& chunkId_ws, bool isDirty);
		void SetChunkLightDirty(Uint16x3& chunkId_ws);

		/** set chunk column time stamp. usually 0 for non-generated. 1 for generated.
		this is usually called by world generators, so that we will not generate again next time we load the world. */
		void SetChunkColumnTimeStamp(uint16_t x, uint16_t z, uint16_t nTimeStamp);
		/** get the time stamp of for the given chunk column
		*	-1 means region not loaded yet, 0 means not available, 1 means loaded before
		*/
		int32_t GetChunkColumnTimeStamp(uint16_t x, uint16_t z);

		/**
		* Checks if any of the chunks within radius blocks of the given block exist
		* @param x,y,z: center world block position
		* @param radius: radius
		*/
		bool DoChunksNearChunkExist(uint16_t x, uint16_t y, uint16_t z, uint16_t radius);

		/**
		* Checks between a min and max all the chunks in between actually exist.
		* @param all input in block position.
		*/
		bool CheckChunkColumnsExist(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);

		/**
		* Returns whether a chunk exists at chunk coordinates x, y
		*/
		bool ChunkColumnExists(uint16_t chunkX, uint16_t chunkZ);

		/** whether the chunk at the given world position is locked. a locked chunk is not loaded or being loaded or saved. 
		* when chunk is locked, all block set/get calls will take no effect. 
		*/
		bool IsChunkLocked(uint32 worldX, uint32 worldZ);

		/**
		* @param verticalSectionFilter: if not 0, we will ignore y value in startChunk_ws and endChunk_ws, but use this as a bitwise filter to y 
		*/
		int32_t GetBlocksInRegion(Uint16x3& startChunk_ws, Uint16x3& endChunk_ws, uint32_t matchType, const luabind::adl::object& result, uint32_t verticalSectionFilter = 0);

		/** get number of dirty chunk columns for light calculations*/
		int GetDirtyColumnCount();

		/** get the number of remaining dirty blocks */
		int GetDirtyBlockCount();

		/** ignore sub-block level physics during Pick() function. this is mostly used by camera collision */
		bool IsCubeModePicking();
		void SetCubeModePicking(bool bIsCubeModePicking);

		/** picking in block world */
		bool Pick(const Vector3& rayOrig, const Vector3& dir, float length, PickResult& result, uint32_t filter = 0xffffffff);

		/** find a block in the side direction that matched filter from block(x,y,z)
		* this function can be used to check for free space upward or download
		* @param side: 4 is top.  5 is bottom.
		* @param attrFilter:  attribute to match. 0 means air. default to any block
		* @param nCategoryID: -1 means any category_id.  default to -1
		* @return -1 if not found. otherwise distance to the first block that match in the side direction is returned.
		*/
		int FindFirstBlock(uint16_t x, uint16_t y, uint16_t z, uint16_t nSide = 4, uint32_t max_dist = 32, uint32_t attrFilter = 0xffffffff, int nCategoryID = -1);

		/** get the y pos of the first block of nBlockID, start searching from x, y, z in the side direction
		* @param side: 4 is top.  5 is bottom. default to 5, which uses heightmap for first block(can be a little faster. )
		*/
		int GetFirstBlock(uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide = 5, uint32_t max_dist = 32);

		/** this function is called, when the entire column is loaded
		*/
		bool RefreshChunkColumn(int16_t curChunkWX, int16_t curChunkWZ);
		/** is the chunk column in active chunk region. */
		bool IsChunkColumnInActiveRange(int16_t curChunkWX, int16_t curChunkWZ);

		/** get light brightness of current and 26 nearby block
		@param brightness: pointer to uint8_t blockBrightness[27];
		@param nSize: if 1, it will only get the center block. it is usually 27.
		@param nLightType: -1 means Max light. 0 means emissive block light, 1 means sun light. 2 means both. where brightness[i] is block light and brightness[i+nSize] is sunlight
		*/
		bool GetBlockBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nSize = 27, int nLightType = -1);

		/** if the block in not solid, we will simply return the block's brightness.
		if the block is solid, we will return the max brightness of the block's 6 neighbors.
		*/
		bool GetBlockMeshBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nLightType = 2);

		/** get the interpolated block brightness at the given position. This function ensures smooth light intensity when vPos moves in real world.
		@param vPos: real world position.
		@param pBrightness: output light value
		@return : light in [0,1] range
		*/
		float GetBlockBrightnessReal(const Vector3& vPos, float* pBrightness = NULL);
		float GetBlockBrightnessReal(Uint16x3& blockId_ws, float* pBrightness = NULL);


		//set global sun light intensity
		//@param intensity: [0,1]
		void SetSunIntensity(float intensity);

		float GetSunIntensity();

		//set light block dirty,called when add/remove any type of block
		//@lightValue: >0 if block is light
		void SetLightBlockDirty(Uint16x3& blockId_ws, bool isSunLight);

		/** get block light data by world coordinates*/
		LightData* GetLightData(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist = true);

		//call this function when loading blocks from file
		void SuspendLightUpdate();

		/** get the total number of locked bock regions. If this is not 0, it means that the world loading is still going on in background thread. */
		int GetNumOfLockedBlockRegion();
		int GetTotalNumOfLoadedChunksInLockedBlockRegion();
		int GetNumOfBlockRegion();

		//called when blocks loaded from file;
		void ResumeLightUpdate();

		bool IsLightUpdateSuspended();
		
		bool IsAsyncLightCalculation();
		void SetAsyncLightCalculation(bool val);

		/** @param brightness: must be in the range of [0,15] */
		float GetLightBrightnessFloat(uint8_t brightness);

		/** @param brightness: must be in the range of [0,15] */
		uint8_t GetLightBrightnessInt(uint8_t brightness);

		/** @param brightness: must be in the range of [0,15] */
		float GetLightBrightnessLinearFloat(uint8_t brightness);

		/** Creates the light to brightness table */
		void GenerateLightBrightnessTable(bool bUseLinearBrightness = false);

		//this function should be called every time when view center or camera position changed
		void OnViewCenterMove(float viewCenterX, float viewCenterY, float viewCenterZ);

		/** max number of blocks to render from eye to farthest one*/
		void SetRenderDist(int nValue);
		int GetRenderDist();

		/** get how blocks are preferred to be rendered. */
		BlockRenderMethod GetBlockRenderMethod();
		/** how blocks are preferred to be rendered. */
		virtual void SetBlockRenderMethod(BlockRenderMethod method);

		void GetCurrentCenterChunkId(Int16x3& oResult);

		const Uint16x3& GetEyeBlockId();
		void SetEyeBlockId(const Uint16x3& eyePos);

		const Uint16x3& GetEyeChunkId();

		const Int16x3 GetMinActiveChunkId();

		/** this function is called when region does not have matching disk file. */
		void OnGenerateTerrain(int nRegionX, int nRegionY, int nChunkX = -1, int nChunkZ = -1);

		/**		hight light select block
		@param nGroupID: 0 for animated selection, 1 for wire frame selection.
		*/
		void SelectBlock(uint16_t x, uint16_t y, uint16_t z, int nGroupID = 0);
		void DeselectBlock(uint16_t x, uint16_t y, uint16_t z, int nGroupID = 0);
		void UpdateSelectedBlockValue(uint16_t x, uint16_t y, uint16_t z, int nGroupID = 0);
		void UpdateSelectedBlockNearbyValues(uint16_t x, uint16_t y, uint16_t z, int nGroupID = 0);

		//add render task to render queueGe
		virtual void AddRenderTask(BlockRenderTask* pRenderTask);

		/**
		@param nGroupID: 0 for animated selection, 1 for wire frame selection.  if negative. [0, -nGroupID] are cleared
		*/
		void DeselectAllBlock(int nGroupID = -1);

		/** load region at the given position. this function is usually called only on server side.
		for client side, region is automatically loaded when camera moves. */
		BlockRegion* CreateGetRegion(uint16_t block_x, uint16_t block_y, uint16_t block_z);
		BlockRegion* CreateGetRegion(uint16_t region_x, uint16_t region_z);

		/** clear block render cache */
		virtual void ClearBlockRenderCache();

		// called each frame
		virtual void OnFrameMove();

		/* it will only return a valid chunk if input chunkPos is within current active chunk range usually around the current view center. 
		* only call this function from the main rendering thread. 
		* @return NULL is chunkPos does not exist in the current active range. or the chunk is currently dirty and can not be used. 
		*/
		RenderableChunk* GetRenderableChunk(const Int16x3& chunkPos);
		
		/** unload region from memory. return true if unloaded.*/
		bool UnloadRegion(uint16_t block_x, uint16_t block_y, uint16_t block_z, bool bAutoSave = true);



		void ResetAllLight();
	protected:
		/** removed given region from memory. */
		void UnloadRegion(BlockRegion* pRegion, bool bAutoSave = true);

		/**
		* @params : world chunk coordinates
		*/
		RenderableChunk& GetActiveChunk(uint16_t curChunkWX, uint16_t curChunkWY, uint16_t curChunkWZ);

		/** force reusing active chunk
		*/
		bool ReuseActiveChunk(int16_t curChunkWX, int16_t curChunkWY, int16_t curChunkWZ, BlockRegion* pRegion);

		//perform culling and fill up m_visibleChunks
		virtual void UpdateVisibleChunks(bool bIsShadowPass = false);

		//load/unload region on center view position
		virtual void UpdateRegionCache();

		//load/unload chunk on center view position
		virtual void UpdateActiveChunk();

		virtual void ClearOutOfRangeActiveChunkData();
	protected:
		std::string m_sName;
		int32_t m_activeChunkDim;
		int32_t m_activeChunkDimY;
		static float g_verticalOffset;
		// 自动物理地形
		bool m_bAutoPhysics;
		/** whether this world is active */
		bool m_isInWorld;

		/** if true, no auto world saving is done when user moving out of the region cache. */
		bool m_bReadOnlyWorld;

		/** a remote world connected to server. */
		bool m_bIsRemote;

		/** a server world. */
		bool m_bIsServerWorld;

		/** whether to save light map to disk. Light map usually double the disk space,
		but increases map loading time, without the occasionally black blocks. */
		bool m_bSaveLightMap;

		/** ignore sub-block level physics during Pick() function. this is mostly used by camera collision*/
		bool m_bCubeModePicking;

		/** if true, it will check dirty bit for all active chunks in the next the render framemove*/
		bool m_isVisibleChunkDirty;

		/** whether to render blocks*/
		bool m_bRenderBlocks;
		/** whether we will use async world loader. default to false. this is only true when region is first loaded. */
		bool m_bUseAsyncLoadWorld;

		/** if true, when rendering blocks, always group by chunk first and then by texture. if not we will group by texture first from all chunks.  */
		bool m_group_by_chunk_before_texture;

		/** if true, light value in block shader is linear. if false, it is pre-calculated to have a falloff curve.*/
		bool m_is_linear_torch_brightness;


		//active chunk is a toroidal 3D grid array hold render data for n chunks around eye position.
		//we will wrap the active chunk index when eye position change.
		//say active chunk range from [x,y,z] to [nx,nz,ny] at first when move to 
		//[x+1,y+1,z+1]~[nx+1,ny+1,nz+1] we keep all data from [x+1,y+1,z+1]~[nx,nz,ny]
		//and fill new data in the obsolete grids to avoid full refresh
		std::vector<RenderableChunk*> m_activeChunks;

		Int16x3 m_minActiveChunkId_ws;


		/** when some thread is accessing the block world. It generally need to acquire this lock.
		* On client, the main thread will acquire the lock at beginning of scene/block animation/rendering and release it at the end.
		* So the main thread is occupied this block for a relative long time. Any other worker thread such as light calculation and terrain generation should wait for
		* this lock, and even it does successfully required the lock, they must ensure it is released immediately to not affecting the main rendering thread.
		*/
		ParaEngine::mutex  m_blockworld_mutex;

		//current region position in world space
		int16_t m_curRegionIdX;
		int16_t m_curRegionIdZ;

		//current chunk position in world space
		Int16x3 m_curChunkIdW;
		Int16x3 m_lastChunkIdW;
		// for checking inside UpdateRegionCache(), 
		Int16x3 m_lastChunkIdW_RegionCache;
		Uint16x3 m_lastViewCheckIdW;

		//
		Uint16x3 m_curCenterBlockId;
		Uint16x3 m_curCamBlockId;
		Uint16x3 m_curCamChunkId;
		Uint16x3 m_selectBlockIdW;

		/** minimum world position. mostly set to prevent running out of memory. default to 0,0,0. */
		Int32x3 m_minWorldPos;
		/** maximum world position. mostly set to prevent running out of memory. default to 0xffff,0xffff,0xffff. */
		Int32x3 m_maxWorldPos;

		int m_minRegionX, m_minRegionZ;
		int m_maxRegionX, m_maxRegionZ;

		/** max number of blocks to render from eye to farthest one*/
		int m_nRenderDistance;

		/** the current block rendering method. */
		BlockRenderMethod m_dwBlockRenderMethod;

		float m_sunIntensity;

		//hold all lighting info of current active region
		CBlockLightGridBase* m_pLightGrid;

		//hold all visible chunks of current frame, weak references
		std::vector<RenderableChunk*> m_visibleChunks;
		std::vector<RenderableChunk*> m_tempDirtyChunks;

		// cache 4 regions at least.
		uint16_t m_maxCacheRegionCount;
		/** array of 64*64 regions. */
		typedef BlockRegion* BlockRegionPtr;
		BlockRegionPtr* m_pRegions;
		std::map<int, BlockRegion*> m_regionCache;

		//Block templates
		std::map<uint16_t, BlockTemplate*> m_blockTemplates;
		std::vector<BlockTemplate*> m_blockTemplatesArray;

		//save old data to revert
		struct BlockTemplateVisibleData
		{
			uint8_t torchLight;
			int32_t lightOpyValue;
			bool isTransparent;
		};
		std::map<uint16_t, BlockTemplateVisibleData> m_blockTemplateVisibleDatas;

		/* directory information about the world. */
		CWorldInfo m_worldInfo;

		/** Light to brightness conversion table. [0,1] float */
		float m_lightBrightnessTableFloat[17];
		/** Light to brightness conversion table. [0, 255] int */
		uint8_t m_lightBrightnessTableInt[17];
		float m_lightBrightnessLinearTableFloat[17];

		std::vector<CBlockSelectGroup> m_selectedBlockMap;

		/** read-write lock. */
		BlockReadWriteLock m_readWriteLock;

		friend class CMultiFrameBlockWorldRenderer;
	};
}
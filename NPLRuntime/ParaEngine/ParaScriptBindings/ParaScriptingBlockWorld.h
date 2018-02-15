//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.1
// Description:	API for world
//-----------------------------------------------------------------------------
#pragma once

#include "ParaScriptingGlobal.h"

namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

namespace ParaEngine
{
	class CBlockWorld;
}

namespace ParaScripting
{
	using namespace std;
	using namespace luabind;
	using namespace ParaEngine;
	class ParaTerrain;

	/**
	* @ingroup ParaWorld
	* 
	* Wrapper of internal CBlockWorld. may have multiple instance of the block world. 
	*/
	class PE_CORE_DECL ParaBlockWorld
	{
	public:
		ParaBlockWorld();
		~ParaBlockWorld();

	public:
		/** static function to create get a world instance */
		static object GetWorld(const object& sWorldName);

		/** get version */
		static int GetVersion(const object& pWorld);

		/** get block terrain manager's attribute object. */
		static ParaAttributeObject GetBlockAttributeObject(const object& pWorld);

	public:
		/** register blocks with given parameters
		@param params: it can be attFlag of int type. or it can be a table containing additional format such as
		{attFlag=number, modelName=string, etc. }
		*/
		static bool RegisterBlockTemplate(const object& pWorld, uint16_t templateId, const object& params);
		static bool RegisterBlockTemplate_(CBlockWorld* pWorld, uint16_t templateId, const object& params);

		/** set Block world's y offset in real world coordinate. */
		static void SetBlockWorldYOffset(const object& pWorld, float offset);

		/** call this function after all block templates has been registered to initialize the world 
		* note this function can be called multiple times to load different world with the same block templates. 
		* call LeaveWorld() before EnterWorld again. 
		* @param sWorldDir: world directory or world config file. 
		*/
		static void EnterWorld(const object& pWorld, const char* sWorldDir);

		/** call this function when leave the block world*/
		static void LeaveWorld(const object& pWorld);

		static void SaveBlockWorld(const object& pWorld, bool saveToTemp);

		/** load region at the given position. current implementation will load entire region rather than just chunk. 
		* one need to call load chunk before SetBlock/GetBlock api can be called in the region. 
		*/
		static void LoadRegion(const object& pWorld, uint16_t x, uint16_t y, uint16_t z);
		/** unload data for a given region from memory */
		static void UnloadRegion(const object& pWorld, uint16_t x, uint16_t y, uint16_t z);

		/** set block id set the given position. 
		* @param x,y,z: should be positive value
		* @param templateId: template id. specify 0 to delete a block.
		*/
		static void SetBlockId(const object& pWorld, uint16_t x, uint16_t y, uint16_t z, uint32_t templateId);

		/** get block id at the given block position. */
		static uint32_t GetBlockId(const object& pWorld, uint16_t x, uint16_t y, uint16_t z);

		/**
		* set per block user data 
		*/
		static void SetBlockData(const object& pWorld, uint16_t x, uint16_t y, uint16_t z, uint32_t data);

		/**
		* get per block user data 
		*/
		static uint32_t GetBlockData(const object& pWorld, uint16_t x, uint16_t y, uint16_t z);

		
		/** get block in [startChunk,endChunk]
		* @param result: in/out containing the result. 
		* @param startChunkY, endChunkY: if negative, and startChunkY == endChunkY, -startChunkY will be used as verticalSectionFilter (a bitwise filter).
		* @return {count,x{},y{},z{},tempId{}}
		*/
		static object GetBlocksInRegion(const object& pWorld, int32_t startChunkX, int32_t startChunkY, int32_t startChunkZ, int32_t endChunkX, int32_t endChunkY, int32_t endChunkZ,
			uint32_t matchType, const object& result);

		/** set current sun intensity in [0,1] range */
		static void SetBlockWorldSunIntensity(const object& pWorld, float value);

		/** find a block in the side direction that matched filter from block(x,y,z)
		* this function can be used to check for free space upward or download
		* @param side: 4 is top.  5 is bottom.
		* @param attrFilter:  attribute to match. 0 means air. default to any block
		* @param nCategoryID: -1 means any category_id.  default to -1
		* @return -1 if not found. otherwise distance to the first block that match in the side direction is returned.
		*/
		static int FindFirstBlock(const object& pWorld, uint16_t x, uint16_t y, uint16_t z, uint16_t nSide = 4, uint32_t max_dist = 32, uint32_t attrFilter = 0xffffffff, int nCategoryID = -1);

		/** get the y pos of the first block of nBlockID, start searching from x, y, z in the side direction
		*/
		static int GetFirstBlock(const object& pWorld, uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide = 4, uint32_t max_dist = 32);

	// following are client only functions. 
	public: 
		/** set the template texture.
		* only used on client side
		* This function is deprecated. use RegisterBlockTemplate instead.
		*/
		static void SetTemplateTexture(const object& pWorld, uint16_t templateId, const char* fileName);


		/** get visible chunk region
		* only used on client side
		* @return : world space chunk id {minX,minY,minZ,maxX,maxY,maxZ}
		*/
		static object GetVisibleChunkRegion(const object& pWorld, const object& result);

		/** ray origin should be positive value, ray direction should be normalized value
		* function is only used on client world
		@return:
		result["x"] = pickResult.X;
		result["y"] = pickResult.Y;
		result["z"] = pickResult.Z;
		result["blockX"] = pickResult.BlockX;
		result["blockY"] = pickResult.BlockY;
		result["blockZ"] = pickResult.BlockZ;
		result["side"] = pickResult.Side;
		result["length"] = pickResult.Distance;
		side value : 0 negativeX,1 positiveX,2 NZ,3 PZ,4 NY, 5PY
		length > fMaxDistance when no collision detected
		*/
		static object Pick(const object& pWorld, float rayX, float rayY, float rayZ, float dirX, float dirY, float dirZ, float fMaxDistance, const object& result, uint32_t filter = 0xffffffff);

		/**
		picking by current mouse position.
		only used on client world
		*/
		static object MousePick(const object& pWorld, float fMaxDistance, const object& result, uint32_t filter = 0xffffffff);


		/** add/remove block to/from highlight block list
		* only used on client side
		* @param x,y,z: world space block id;
		* @param isSelect : true to select block, false to de-select block
		* @param nGroupID: group id. 0 for highlight 1 for wireframe.
		*/
		static void SelectBlock(const object& pWorld, uint16_t x, uint16_t y, uint16_t z, bool isSelect);
		static void SelectBlock1(const object& pWorld, uint16_t x, uint16_t y, uint16_t z, bool isSelect, int nGroupID);

		/**
		* @param nGroupID: 0 for animated selection, 1 for wire frame selection.  -1 for all
		*/
		static void DeselectAllBlock1(const object& pWorld, int nGroupID);
		static void DeselectAllBlock(const object& pWorld);

		/** set damage block id
		* only used on client side
		* @param x,y,z :world space block id;
		*/
		static void SetDamagedBlock(const object& pWorld, uint16_t x, uint16_t y, uint16_t z);

		/** set damage block degree
		* @param damageDegree: [0,1] 0 means undamaged block,1 full damaged block
		*/
		static void SetDamageDegree(const object& pWorld, float damagedDegree);
	};
}
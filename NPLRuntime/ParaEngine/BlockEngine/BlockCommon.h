#pragma once

#include <vector>
#include <array>
#include <unordered_set>
#include <stdint.h>
#include <bitset>

#include "BlockConfig.h"
#include "BlockCoordinate.h"
#include "BlockIndex.h"

namespace ParaEngine
{
	class Block;
	class BlockChunk;
	class BlockRegion;
	class BlockLight;
	class RenderableChunk;
	class LightGrid;
	class BlockVertexCompressed;


	enum BlockRenderPass
	{
		BlockRenderPass_Opaque = 0,
		BlockRenderPass_AlphaTest,
		BlockRenderPass_AlphaBlended,
		// do not cast shadow, this is special in deferred shading. 
		BlockRenderPass_ReflectedWater,
	};

	struct CBlockSelectGroup
	{
	public:
		CBlockSelectGroup();

		std::map<int64_t, Uint16x3>& GetBlocks();

	public:
		std::map<int64_t, Uint16x3> m_blocks;
		float m_fScaling;
		bool m_bEnableBling;
		LinearColor m_color;
		bool m_bOnlyRenderClickableArea;
		bool m_bWireFrame;
	};

	enum RelativeBlockPos
	{
		rbp_center = 0,
		rbp_pX,
		rbp_nX,
		rbp_pY,
		rbp_nY,
		rbp_pZ,
		rbp_nZ,

		rbp_pXpY,
		rbp_pXnY,
		rbp_pXpZ,
		rbp_pXnZ,

		rbp_nXpY,
		rbp_nXnY,
		rbp_nXpZ,
		rbp_nXnZ,

		rbp_pYpZ,
		rbp_pYnZ,
		rbp_nYpZ,
		rbp_nYnZ,

		rbp_pXpYpZ,
		rbp_pXpYnZ,
		rbp_pXnYPz,
		rbp_pXnYnZ,
		rbp_nXpYpZ,
		rbp_nXpYnZ,
		rbp_nXnYPz,
		rbp_nXnYnZ,
	};

	enum DirectionFlag
	{
		dir_center = 0x0000,
		dir_x = 0x0001,
		dir_nx = 0x0002,
		dir_z = 0x0004,
		dir_nz = 0x0008,
		dir_y = 0x0010,
		dir_ny = 0x0020,

		dir_xy = dir_x | dir_y,
		dir_xny = dir_x | dir_ny,
		dir_xz = dir_x | dir_z,
		dir_xnz = dir_x | dir_nz,

		dir_nxy = dir_nx | dir_y,
		dir_nxny = dir_nx | dir_ny,
		dir_nxz = dir_nx | dir_z,
		dir_nxnz = dir_nx | dir_nz,
	};

	struct PickResult
	{
		float X;
		float Y;
		float Z;
		float Distance;
		uint16_t BlockX;
		uint16_t BlockY;
		uint16_t BlockZ;
		uint16_t Side;
	};

	enum BlockRenderMethod
	{
		BLOCK_RENDER_FIXED_FUNCTION = 0,
		BLOCK_RENDER_FAST_SHADER,
		BLOCK_RENDER_FANCY_SHADER,
	};

	enum BlockGroundIDEnum
	{
		BLOCK_GROUP_ID_HIGHLIGHT = 0,
		// scale 1.01
		BLOCK_GROUP_ID_WIREFRAME,
		// scale 1
		BLOCK_GROUP_ID_WIREFRAME1,
		BLOCK_GROUP_ID_MAX = 10,
	};

	/**
	* common static functions to block api, mostly standalone math API
	*/
	class BlockCommon
	{
	public:
		/** convert from block index to real world coordinate.
		* @param nSide: default to -1. -1 is the block center, 4 is top.  5 is bottom.
		* @return: the returned position is always the center of the block.
		*/
		static Vector3 ConvertToRealPosition(uint16 x, uint16 y, uint16 z, int nSide = -1);
		static float ConvertToRealY(uint16 y);

		/** convert from block x,y,z, to new x,y,z accordig to side. 
		* @param nSide: default to -1. -1 is the block center, 4 is top.  5 is bottom.
		*/
		static void GetBlockPosBySide(int& x, int& y, int& z, int nSide = 4);

		/** convert real world coordinate x,y,z to block index.
		*/
		static void ConvertToBlockIndex(float x,float y,float z, uint16& idx_x, uint16& idx_y, uint16& idx_z);

		/** get the block center, based on a real world position.
		*/
		static Vector3 GetBlockCenter(float x, float y, float z);

		/** get the block normal according to block side id
		* @param nSide: in range [0,5], where 4 up, 5 bottom, etc. */
		static Vector3 GetNormalBySide(int nSide);

		/** whether the two side is opposite side. 0,1 and 2, 3, and 4, 5 are opposite side.  */
		static bool IsOppositeSide(int nSide1, int nSide2);

		/** whether the two side form a corner side. 0,1 and 2, 3, and 4, 5 are not side. and anything equal is not corner side */
		static bool IsCornerSide(int nSide1, int nSide2);

		/** constrain pos in a AABB cubic space */
		static void ConstrainPos(Vector3& vMovePos,const Vector3& vMinPos,  const Vector3& vMaxPos);
		static void ConstrainPos(DVector3& vMovePos, const Vector3& vMinPos, const Vector3& vMaxPos);

		/** constrain pos outside a AABB cubic space. if there is AABB's side whose distance to point is smaller than fBoarderDist, then the point will be pushed to that side only. */
		static void ConstrainPosOuter(Vector3& vMovePos,const Vector3& vMinPos,  const Vector3& vMaxPos, float fBoarderDist);

		static const Int16x3 NeighborOfsTable[27];

		/// 96 relative block position of the 24 vertices. (each vertex has 4 rbp position)
		static const int32_t NeighborLightOrder[];
		/// relative block position of the six most close neighbors out of the 27 neighbor RBP positions. in block rendering order
		static const int32_t RBP_SixNeighbors[];

		// x, z direction vectors : east, south, west, north
		static int32 g_xzDirectionsConst[4][2];
	};
}

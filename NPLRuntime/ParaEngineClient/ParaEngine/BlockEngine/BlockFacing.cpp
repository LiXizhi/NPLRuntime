//-----------------------------------------------------------------------------
// Class:	Block facing
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.29
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockFacing.h"

namespace ParaEngine
{
	/**
	* Converts a side to the opposite side. This is the same as XOR'ing it with 1.
	*/
	int32 BlockFacing::oppositeSide[] = { 1, 0, 3, 2, 5, 4 };

	/**
	* gives the offset required for this axis to get the block at that side.
	*/
	int32 BlockFacing::offsetsXForSide[] = { 0, 0, 0, 0, -1, 1 };

	/**
	* gives the offset required for this axis to get the block at that side.
	*/
	int32 BlockFacing::offsetsYForSide[] = { -1, 1, 0, 0, 0, 0 };

	/**
	* gives the offset required for this axis to get the block at that side.
	*/
	int32 BlockFacing::offsetsZForSide[] = { 0, 0, -1, 1, 0, 0 };
}

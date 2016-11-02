//-----------------------------------------------------------------------------
// Class:	BlockDirection
// Authors:	leio, LiXizhi
// Emails:	
// Company: ParaEngine
// Date:	2015.9.24
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockDirection.h"

namespace ParaEngine
{
	const int BlockDirection::s_oppositeDirection[] = { Side::right, Side::left, Side::back, Side::front, Side::bottom, Side::top, Side::none };
	
	BlockDirection::BlockDirection()
	{
	}

	BlockDirection::~BlockDirection()
	{

	}
	
	Int32x3 BlockDirection::GetOffsetBySide(BlockDirection::Side side)
	{
		int dx = 0;
		int dy = 0;
		int dz = 0;
		switch (side)
		{
		case BlockDirection::left:
			dx = 1;
			break;
		case BlockDirection::right:
			dx = -1;
			break;
		case BlockDirection::front:
			dz = 1;
			break;
		case BlockDirection::back:
			dz = -1;
			break;
		case BlockDirection::top:
			dy = 1;
			break;
		case BlockDirection::bottom:
			dy = -1;
			break;
		default:
			break;
		}
		return Int32x3(dx, dy, dz);
	}

	BlockDirection::Side BlockDirection::GetBlockSide(int v)
	{
		if (v == 1)
		{
			return BlockDirection::left;
		}
		else if (v == 0)
		{
			return BlockDirection::right;
		}
		else if (v == 3)
		{
			return BlockDirection::front;
		}
		else if (v == 2)
		{
			return BlockDirection::back;
		}
		else if (v == 5)
		{
			return BlockDirection::top;
		}
		else if (v == 4)
		{
			return BlockDirection::bottom;
		}
		return BlockDirection::none;
	}

	BlockDirection::Side BlockDirection::GetOpSide(BlockDirection::Side side)
	{
		return (BlockDirection::Side)s_oppositeDirection[side];
	}
}
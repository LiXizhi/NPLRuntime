#pragma once
#include "BlockCoordinate.h"

namespace ParaEngine
{
	/**
	* helper functions for directions
	*/
	class BlockDirection
	{
	public:
		enum Side
		{
			left, right, front, back, top, bottom, none
		};
		BlockDirection();
		~BlockDirection();
		
		static Int32x3 GetOffsetBySide(Side side);
		static Side GetBlockSide(int v);
		static Side GetBlockSideAndLevelData(int v, int* pLevelData = NULL);
		static Side GetOpSide(Side side);
	public:
		static const int s_oppositeDirection[none+1];
	};
}
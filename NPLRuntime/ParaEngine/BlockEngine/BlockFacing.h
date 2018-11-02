#pragma once

namespace ParaEngine
{
	class BlockFacing
	{
	public:
		/**
		* Converts a side to the opposite side. This is the same as XOR'ing it with 1.
		*/
		static int32 oppositeSide[6];

		/**
		* gives the offset required for this axis to get the block at that side.
		*/
		static int32 offsetsXForSide[6];

		/**
		* gives the offset required for this axis to get the block at that side.
		*/
		static int32 offsetsYForSide[6];

		/**
		* gives the offset required for this axis to get the block at that side.
		*/
		static int32 offsetsZForSide[6];
	};
}

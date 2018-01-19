#pragma once

namespace ParaEngine
{
	/** chunk column's height map data */
	struct ChunkMaxHeight
	{
	public:
		int16_t m_nSolidHeight;
		int16_t m_nMaxHeight;

	public:
		ChunkMaxHeight(int16_t nSolidHeight=0, int16_t nMaxHeight=0);

		// y pos of the highest block
		int16_t GetMaxHeight();

		/* y pos of the first block whose opacity is bigger than 1 (first non-transparent block).
		*/
		int16_t GetMaxSolidHeight();

		void SetHeight(int16_t nHeight, bool bIsTransparent = false);

		/** only call this function at load time, since it can not erase height.
		use SetHeight if one wants to erase block height to new values.
		*/
		void AddBlockHeight(int16_t nHeight, bool bIsTransparent = false);

		/** clear all height 0*/
		void ClearHeight();
	};
}
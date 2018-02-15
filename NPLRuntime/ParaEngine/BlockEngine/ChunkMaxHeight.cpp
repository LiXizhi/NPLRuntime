//-----------------------------------------------------------------------------
// Class:	Chunk column's height map data
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.30
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ChunkMaxHeight.h"

using namespace ParaEngine;


ParaEngine::ChunkMaxHeight::ChunkMaxHeight(int16_t nSolidHeight, int16_t nMaxHeight) 
	: m_nSolidHeight(nSolidHeight), m_nMaxHeight(nMaxHeight)
{

}

int16_t ChunkMaxHeight::GetMaxHeight()
{
	return m_nMaxHeight;
}

int16_t ChunkMaxHeight::GetMaxSolidHeight()
{
	return m_nSolidHeight;
}

void ChunkMaxHeight::AddBlockHeight(int16_t nHeight, bool bIsTransparent /*= false*/)
{
	if (!bIsTransparent)
	{
		if (m_nSolidHeight < nHeight)
		{
			m_nSolidHeight = nHeight;
			if (m_nMaxHeight < nHeight)
				m_nMaxHeight = nHeight;
		}
	}
	else
	{
		if (m_nMaxHeight < nHeight)
			m_nMaxHeight = nHeight;
	}
}

void ChunkMaxHeight::SetHeight(int16_t nHeight, bool bIsTransparent /*= false*/)
{
	if (!bIsTransparent)
	{
		m_nSolidHeight = nHeight;
		m_nMaxHeight = nHeight;
	}
	else
	{
		m_nMaxHeight = nHeight;
		if (m_nSolidHeight > m_nMaxHeight)
			m_nSolidHeight = m_nMaxHeight;
	}
}

void ChunkMaxHeight::ClearHeight()
{
	m_nMaxHeight = 0;
	m_nSolidHeight = 0;
}

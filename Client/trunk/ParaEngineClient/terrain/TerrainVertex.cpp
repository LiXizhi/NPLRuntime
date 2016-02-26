#include "ParaEngine.h"
#include "TerrainVertex.h"

using namespace ParaTerrain;

TerrainVertex::TerrainVertex(int index, float x, float y, float z)
{
	m_Index = index;
	m_X = x;
	m_Y = y;
	m_Z = z;
}

TerrainVertex::~TerrainVertex()
{
}

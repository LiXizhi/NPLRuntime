//-----------------------------------------------------------------------------
// Class:	Coordinate
// Authors:	Clayman, LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2012.11.26
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "BlockCoordinate.h"

using namespace ParaEngine;


void ParaEngine::Int16x3::SetValue(int16_t v)
{
	x = v;
	y = v;
	z = v;
}

void ParaEngine::Int16x3::Add(Int16x3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

void ParaEngine::Int16x3::Add(int16_t value)
{
	x += value;
	y += value;
	z += value;
}

void ParaEngine::Int16x3::Add(const Int16x3& v0, const Int16x3& v1, Int16x3& oResult)
{
	oResult.x = v0.x + v1.x;
	oResult.y = v0.y + v1.y;
	oResult.z = v0.z + v1.z;
}

void ParaEngine::Int16x3::Add(const Int16x3& v0, int16_t v1, Int16x3& oResult)
{
	oResult.x = v0.x + v1;
	oResult.y = v0.y + v1;
	oResult.z = v0.z + v1;
}

void ParaEngine::Int16x3::Subtract(const Int16x3& value)
{
	x -= value.x;
	y -= value.y;
	z -= value.z;
}

void ParaEngine::Int16x3::Subtract(int16_t value)
{
	x -= value;
	y -= value;
	z -= value;
}

void ParaEngine::Int16x3::Subtract(const Int16x3& v0, const Int16x3& v1, Int16x3& oResult)
{
	oResult.x = v0.x - v1.x;
	oResult.y = v0.y - v1.y;
	oResult.z = v0.z - v1.z;
}

void ParaEngine::Int16x3::Subtract(const Int16x3& v0, int16_t v1, Int16x3& oResult)
{
	oResult.x = v0.x - v1;
	oResult.y = v0.y - v1;
	oResult.z = v0.z - v1;
}

void ParaEngine::Int16x3::Divide(Int16x3& value)
{
	x /= value.x;
	y /= value.y;
	z /= value.z;
}

void ParaEngine::Int16x3::Divide(int16_t value)
{
	x /= value;
	y /= value;
	z /= value;
}

void ParaEngine::Int16x3::DivideTo(const Int16x3& v0, const Int16x3 v1, Int16x3& oResult)
{
	oResult.x = v0.x / v1.x;
	oResult.y = v0.y / v1.y;
	oResult.z = v0.z / v1.z;
}

void ParaEngine::Int16x3::DivideTo(const Int16x3& v0, int16_t v1, Int16x3& oResult)
{
	oResult.x = v0.x / v1;
	oResult.y = v0.y / v1;
	oResult.z = v0.z / v1;
}

void ParaEngine::Int16x3::Mod(int value)
{
	x %= value;
	y %= value;
	z %= value;
}

void ParaEngine::Int16x3::Mod(const Int16x3& v0, int v1, Int16x3& oResult)
{
	oResult.x = v0.x % v1;
	oResult.y = v0.y % v1;
	oResult.z = v0.z % v1;
}

void ParaEngine::Int16x3::Abs()
{
	x = abs(x);
	y = abs(y);
	z = abs(z);
}

// ChunkLocation

ParaEngine::ChunkLocation::ChunkLocation(uint16_t chunkX, uint16_t chunkZ) :m_chunkX(chunkX), m_chunkZ(chunkZ)
{

}

ParaEngine::ChunkLocation::ChunkLocation(int chunkPackedPos)
{
	m_chunkX = chunkPackedPos >> 16;
	m_chunkZ = chunkPackedPos & 0xFFFF;
}

uint32_t ParaEngine::ChunkLocation::GetPackedChunkPos()  const
{
	return (m_chunkX << 16) + m_chunkZ;
}

uint32_t ParaEngine::ChunkLocation::FromChunkToPackedChunk(int chunkX, int chunkZ)
{
	return (chunkX << 16) + chunkZ;
}

uint32_t ParaEngine::ChunkLocation::GetCenterWorldX()  const
{
	return (m_chunkX << 4) + 8;
}
 
uint32_t ParaEngine::ChunkLocation::GetCenterWorldZ()  const
{
	return (m_chunkZ << 4) + 8;
}

bool ParaEngine::ChunkLocation::operator==(const ChunkLocation& r) const 
{
	return m_chunkX == r.m_chunkX && m_chunkZ == r.m_chunkZ;
}

int ParaEngine::ChunkLocation::DistanceToSquared(const ChunkLocation& coords) const
{
	int diffX = coords.m_chunkX - m_chunkX;
	int diffZ = coords.m_chunkZ - m_chunkZ;
	return diffX * diffX + diffZ * diffZ;
}

uint32 ParaEngine::Uint16x3::AbsDistanceTo(const Uint16x3& v) const 
{
	return abs((int32)x - (int32)v.x) + abs((int32)y - (int32)v.y) + abs((int32)z - (int32)v.z);
}

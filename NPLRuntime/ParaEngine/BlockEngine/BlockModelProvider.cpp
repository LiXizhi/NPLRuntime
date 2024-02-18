//-----------------------------------------------------------------------------
// Class: Block Model data provider
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2013.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"

#include "BlockModelProvider.h"

using namespace ParaEngine;


BlockModel& ParaEngine::IBlockModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	return (nIndex < (int)m_pBlockTemplate->m_block_models.size()) ? m_pBlockTemplate->m_block_models[nIndex] : m_pBlockTemplate->m_block_models[0];
}

BlockModel& ParaEngine::IBlockModelProvider::GetBlockModelByData(uint32 nData /*= 0*/)
{
	return GetBlockModel(0);
}

BlockModel& IBlockModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks/*=NULL*/)
{
	return GetBlockModel(GetModelIndex(pBlockManager, m_pBlockTemplate->GetID(), bx, by, bz, nBlockData, neighborBlocks));
}

int CGrassModelProvider::GetModelIndex(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks/*=NULL*/)
{
	// just make it random
	DWORD i = bx + by + bz + block_id;
	return (i + (i >> 8) + (i >> 16)) & 0xf;
}

int CLinearModelProvider::GetModelIndex(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks/*=NULL*/)
{
	nBlockData = nBlockData & 0xff;
	if (nBlockData < m_nModelCount)
		return nBlockData;
	else
		return 0;
}

BlockModel& ParaEngine::CLinearModelProvider::GetBlockModelByData(uint32 nBlockData)
{
	nBlockData = nBlockData & 0xff;
	return m_pBlockTemplate->GetBlockModel(nBlockData);
}

int ParaEngine::CLinearModelProvider::GetModelCount() const
{
	return m_nModelCount;
}

Vector3 ParaEngine::CLinearModelProvider::vec3Rotate(const Vector3 &pt, const Vector3 &angles)
{
	float X = pt.x,Y= pt.y,Z= pt.z;
	float a = angles.x, b = angles.y, c = angles.z;

	float x, y, z;
	// rotate around the X axis first
	if (abs(a)>0.01) {
		x = X, y = Y, z = Z;
		Y = y * cos(a) - z * sin(a);
		Z = y * sin(a) + z * cos(a);
	}	
	// And now around y
	if (abs(b) > 0.01) {
		x = X, y = Y, z = Z;
		X = x * cos(b) + z * sin(b);
		Z = -x * sin(b) + z * cos(b);
	}
	// Finally, around z
	if (abs(c) > 0.01) {
		x = X, y = Y, z = Z;
		X = x * cos(c) - y * sin(c);
		Y = x * sin(c) + y * cos(c);
	}
			
	return Vector3(X, Y, Z);
}

Vector3 ParaEngine::CLinearModelProvider::vec3RotateByPoint(const Vector3 &originPt, const Vector3 &pt, const Vector3 &angles)
{
	float ox = originPt.x, oy = originPt.y, oz = originPt.z;
	float X = pt.x, Y = pt.y, Z = pt.z;

	float x = X - ox;
	float y = Y - oy;
	float z = Z - oz;
	Vector3 ret = vec3Rotate(Vector3(x, y, z), angles);
	ret.x = ret.x + ox;
	ret.y = ret.y + oy;
	ret.z = ret.z + oz;

	return ret;
}
//-----------------------------------------------------------------------------
// Class: Wire Model data provider
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2013.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "WireModelProvider.h"

using namespace ParaEngine;
/** distance from vine model to surface */
#define WIRE_OFFGROUND_OFFSET  0.05f


ParaEngine::CWireModelProvider::CWireModelProvider(BlockTemplate* pBlockTemplate) :IBlockModelProvider(pBlockTemplate)
{
	m_block_model.LoadModelByTexture(0);
	m_block_model.LoadModel("vine2");
	m_block_model.SetCategoryID(m_pBlockTemplate->GetCategoryID());
	m_block_model.SetUsingSelfLighting(true);
	m_block_model.SetUseAmbientOcclusion(false);
	m_block_model.SetFaceCount(5);
	m_block_model_cube.LoadModelByTexture(0);

	BlockVertexCompressed* cube_vertices = m_block_model_cube.GetVertices();
	BlockVertexCompressed* vertices = m_block_model.GetVertices();

	const float fPosOffset = WIRE_OFFGROUND_OFFSET;
	//front face
	cube_vertices[4].SetPosition(0, 0, fPosOffset);
	cube_vertices[5].SetPosition(0, 1 + fPosOffset, fPosOffset);
	cube_vertices[6].SetPosition(1, 1 + fPosOffset, fPosOffset);
	cube_vertices[7].SetPosition(1, 0, fPosOffset);

	//left face
	cube_vertices[12].SetPosition(fPosOffset, 0, 1);
	cube_vertices[13].SetPosition(fPosOffset, 1 + fPosOffset, 1);
	cube_vertices[14].SetPosition(fPosOffset, 1 + fPosOffset, 0);
	cube_vertices[15].SetPosition(fPosOffset, 0, 0);

	//right face
	cube_vertices[16].SetPosition(1 - fPosOffset, 0, 0);
	cube_vertices[17].SetPosition(1 - fPosOffset, 1 + fPosOffset, 0);
	cube_vertices[18].SetPosition(1 - fPosOffset, 1 + fPosOffset, 1);
	cube_vertices[19].SetPosition(1 - fPosOffset, 0, 1);

	//back face
	cube_vertices[20].SetPosition(1, 0, 1 - fPosOffset);
	cube_vertices[21].SetPosition(1, 1 + fPosOffset, 1 - fPosOffset);
	cube_vertices[22].SetPosition(0, 1 + fPosOffset, 1 - fPosOffset);
	cube_vertices[23].SetPosition(0, 0, 1 - fPosOffset);
	cube_vertices[8] = cube_vertices[20];
	cube_vertices[9] = cube_vertices[21];
	cube_vertices[10] = cube_vertices[22];
	cube_vertices[11] = cube_vertices[23];

	for (int i = 1; i <= 4; ++i)
	{
		int offset = i * 4;
		vertices[offset].SetTexcoord(0, 0.5);
		vertices[offset + 1].SetTexcoord(1, 0.5);
		vertices[offset + 2].SetTexcoord(0, 1);
		vertices[offset + 3].SetTexcoord(0, 1);
	}

	m_block_model.SetAABB(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, 0.08f*BlockConfig::g_blockSize, BlockConfig::g_blockSize));
}

BlockModel& ParaEngine::CWireModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks/*=NULL*/)
{
	if (neighborBlocks)
	{
		const float fOffset = WIRE_OFFGROUND_OFFSET;
		BlockVertexCompressed* vertices = m_block_model.GetVertices();
		BlockVertexCompressed* cube_vertices = m_block_model_cube.GetVertices();

		// bottom uv;
		float u1 = ((neighborBlocks[rbp_nX] && (neighborBlocks[rbp_nX]->GetTemplate()->canProvidePower())) ||
			(!(neighborBlocks[rbp_nX] && neighborBlocks[rbp_nX]->GetTemplate()->isBlockNormalCube()) && neighborBlocks[rbp_nXnY] && (neighborBlocks[rbp_nXnY]->GetTemplate() == m_pBlockTemplate))) ? 0 : 0.3f;
		float v1 = ((neighborBlocks[rbp_nZ] && (neighborBlocks[rbp_nZ]->GetTemplate()->canProvidePower())) ||
			(!(neighborBlocks[rbp_nZ] && neighborBlocks[rbp_nZ]->GetTemplate()->isBlockNormalCube()) && neighborBlocks[rbp_nYnZ] && (neighborBlocks[rbp_nYnZ]->GetTemplate() == m_pBlockTemplate))) ? 0 : 0.3f;
		float u2 = ((neighborBlocks[rbp_pX] && (neighborBlocks[rbp_pX]->GetTemplate()->canProvidePower())) ||
			(!(neighborBlocks[rbp_pX] && neighborBlocks[rbp_pX]->GetTemplate()->isBlockNormalCube()) && neighborBlocks[rbp_pXnY] && (neighborBlocks[rbp_pXnY]->GetTemplate() == m_pBlockTemplate))) ? 1 : 0.7f;
		float v2 = ((neighborBlocks[rbp_pZ] && (neighborBlocks[rbp_pZ]->GetTemplate()->canProvidePower())) ||
			(!(neighborBlocks[rbp_pZ] && neighborBlocks[rbp_pZ]->GetTemplate()->isBlockNormalCube()) && neighborBlocks[rbp_nYpZ] && (neighborBlocks[rbp_nYpZ]->GetTemplate() == m_pBlockTemplate))) ? 1 : 0.7f;

		bool wall0 = false; bool wall1 = false; bool wall2 = false; bool wall3 = false;

		if (!(neighborBlocks[rbp_pY] && neighborBlocks[rbp_pY]->GetTemplate()->isBlockNormalCube()))
		{
			if (u1>0 && neighborBlocks[rbp_nXpY] && neighborBlocks[rbp_nXpY]->GetTemplate() == m_pBlockTemplate)
			{
				u1 = 0;
				wall0 = true;
			}
			if (v1>0 && neighborBlocks[rbp_pYnZ] && neighborBlocks[rbp_pYnZ]->GetTemplate() == m_pBlockTemplate)
			{
				v1 = 0;
				wall2 = true;
			}
			if (u2<1 && neighborBlocks[rbp_pXpY] && neighborBlocks[rbp_pXpY]->GetTemplate() == m_pBlockTemplate)
			{
				u2 = 1;
				wall1 = true;
			}
			if (v2<1 && neighborBlocks[rbp_pYpZ] && neighborBlocks[rbp_pYpZ]->GetTemplate() == m_pBlockTemplate)
			{
				v2 = 1;
				wall3 = true;
			}
		}
		bool bCrossTexture = true;
		int nFaceCount = 0;
		// bottom face position
		if (u1>0 && u2<1 && (v1 == 0 || v2 == 1)
			/*((v1 == 0 && v2 == 1) ||
			(v1 == 0 && neighborBlocks[rbp_pZ] && neighborBlocks[rbp_pZ]->GetTemplate()->isBlockNormalCube()) ||
			(v2 == 1 && neighborBlocks[rbp_nZ] && neighborBlocks[rbp_nZ]->GetTemplate()->isBlockNormalCube())) */
			)
		{
			// vertical
			bCrossTexture = false;
			v1 = 0; v2 = 1;
			u1 = 0; u2 = 1;

			vertices[1].SetPosition(u1 - fOffset, fOffset, v1 - fOffset);
			vertices[2].SetPosition(u2 + fOffset, fOffset, v1 - fOffset);
			vertices[3].SetPosition(u2 + fOffset, fOffset, v2 + fOffset);
			vertices[0].SetPosition(u1 - fOffset, fOffset, v2 + fOffset);
			nFaceCount++;
		}
		else
		{
			if (v1>0 && v2<1 && (u1 == 0 || u2 == 1)
				/*((u1 == 0 && u2 == 1)  ||
				(u1 == 0 && neighborBlocks[rbp_pX] && neighborBlocks[rbp_pX]->GetTemplate()->isBlockNormalCube())  ||
				(u2 == 1 && neighborBlocks[rbp_nX] && neighborBlocks[rbp_nX]->GetTemplate()->isBlockNormalCube()))*/
				)
			{
				// horizontal
				bCrossTexture = false;
				u1 = 0; u2 = 1;
				v1 = 0; v2 = 1;
			}
			vertices[0].SetPosition(u1 - fOffset, fOffset, v1 - fOffset);
			vertices[1].SetPosition(u2 + fOffset, fOffset, v1 - fOffset);
			vertices[2].SetPosition(u2 + fOffset, fOffset, v2 + fOffset);
			vertices[3].SetPosition(u1 - fOffset, fOffset, v2 + fOffset);
			nFaceCount++;
		}

		// now the bottom face
		int offset_x = (nBlockData == 0) ? 0 : 1;
		int offset_y = (bCrossTexture) ? 0 : 1;
		float u_offset = offset_x / 2.f;
		unsigned char nPowerStrength = (nBlockData == 0) ? 0xff : (nBlockData * 16 + 15);

		// four vertical faces
		if (wall0)
		{
			const int offset = 12;
			int nNextFaceVertexIndex = nFaceCount * 4;

			vertices[nNextFaceVertexIndex].SetPosition(cube_vertices[offset + 1]);
			vertices[nNextFaceVertexIndex + 1].SetPosition(cube_vertices[offset + 2]);
			vertices[nNextFaceVertexIndex + 2].SetPosition(cube_vertices[offset + 3]);
			vertices[nNextFaceVertexIndex + 3].SetPosition(cube_vertices[offset]);

			vertices[nNextFaceVertexIndex + 1].SetTexcoord(u_offset, 0.5);
			vertices[nNextFaceVertexIndex + 2].SetTexcoord(u_offset + 1 / 2.f, 0.5);
			vertices[nNextFaceVertexIndex + 3].SetTexcoord(u_offset + 1 / 2.f, 1);
			vertices[nNextFaceVertexIndex].SetTexcoord(u_offset, 1);

			vertices[nNextFaceVertexIndex].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 1].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 2].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 3].SetColorStrength(nPowerStrength);
			nFaceCount++;
		}


		if (wall1)
		{
			const int offset = 16;
			int nNextFaceVertexIndex = nFaceCount * 4;

			vertices[nNextFaceVertexIndex].SetPosition(cube_vertices[offset + 1]);
			vertices[nNextFaceVertexIndex + 1].SetPosition(cube_vertices[offset + 2]);
			vertices[nNextFaceVertexIndex + 2].SetPosition(cube_vertices[offset + 3]);
			vertices[nNextFaceVertexIndex + 3].SetPosition(cube_vertices[offset]);

			vertices[nNextFaceVertexIndex + 1].SetTexcoord(u_offset, 0.5);
			vertices[nNextFaceVertexIndex + 2].SetTexcoord(u_offset + 1 / 2.f, 0.5);
			vertices[nNextFaceVertexIndex + 3].SetTexcoord(u_offset + 1 / 2.f, 1);
			vertices[nNextFaceVertexIndex].SetTexcoord(u_offset, 1);

			vertices[nNextFaceVertexIndex].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 1].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 2].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 3].SetColorStrength(nPowerStrength);
			nFaceCount++;
		}


		if (wall3)
		{
			const int offset = 8;
			int nNextFaceVertexIndex = nFaceCount * 4;

			vertices[nNextFaceVertexIndex].SetPosition(cube_vertices[offset + 1]);
			vertices[nNextFaceVertexIndex + 1].SetPosition(cube_vertices[offset + 2]);
			vertices[nNextFaceVertexIndex + 2].SetPosition(cube_vertices[offset + 3]);
			vertices[nNextFaceVertexIndex + 3].SetPosition(cube_vertices[offset]);

			vertices[nNextFaceVertexIndex + 1].SetTexcoord(u_offset, 0.5);
			vertices[nNextFaceVertexIndex + 2].SetTexcoord(u_offset + 1 / 2.f, 0.5);
			vertices[nNextFaceVertexIndex + 3].SetTexcoord(u_offset + 1 / 2.f, 1);
			vertices[nNextFaceVertexIndex].SetTexcoord(u_offset, 1);

			vertices[nNextFaceVertexIndex].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 1].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 2].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 3].SetColorStrength(nPowerStrength);
			nFaceCount++;
		}


		if (wall2)
		{
			const int offset = 4;
			int nNextFaceVertexIndex = nFaceCount * 4;

			vertices[nNextFaceVertexIndex].SetPosition(cube_vertices[offset + 1]);
			vertices[nNextFaceVertexIndex + 1].SetPosition(cube_vertices[offset + 2]);
			vertices[nNextFaceVertexIndex + 2].SetPosition(cube_vertices[offset + 3]);
			vertices[nNextFaceVertexIndex + 3].SetPosition(cube_vertices[offset]);

			vertices[nNextFaceVertexIndex + 1].SetTexcoord(u_offset, 0.5);
			vertices[nNextFaceVertexIndex + 2].SetTexcoord(u_offset + 1 / 2.f, 0.5);
			vertices[nNextFaceVertexIndex + 3].SetTexcoord(u_offset + 1 / 2.f, 1);
			vertices[nNextFaceVertexIndex].SetTexcoord(u_offset, 1);

			vertices[nNextFaceVertexIndex].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 1].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 2].SetColorStrength(nPowerStrength);
			vertices[nNextFaceVertexIndex + 3].SetColorStrength(nPowerStrength);
			nFaceCount++;
		}

		u1 = (u1 + offset_x) / 2.f;
		u2 = (u2 + offset_x) / 2.f;
		v1 = (v1 + offset_y) / 2.f;
		v2 = (v2 + offset_y) / 2.f;

		vertices[0].SetTexcoord(u1, v1);
		vertices[1].SetTexcoord(u2, v1);
		vertices[2].SetTexcoord(u2, v2);
		vertices[3].SetTexcoord(u1, v2);

		vertices[0].SetColorStrength(nPowerStrength);
		vertices[1].SetColorStrength(nPowerStrength);
		vertices[2].SetColorStrength(nPowerStrength);
		vertices[3].SetColorStrength(nPowerStrength);

		m_block_model.SetFaceCount(nFaceCount);
	}
	else
	{
		m_block_model.SetFaceCount(5);
	}
	return m_block_model;
}

BlockModel& ParaEngine::CWireModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	// this fixed a bug for wire model face count predication. 
	m_block_model.SetFaceCount(5);
	return m_block_model;
}

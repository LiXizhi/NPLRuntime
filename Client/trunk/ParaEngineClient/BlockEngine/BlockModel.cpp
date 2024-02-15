//-----------------------------------------------------------------------------
// Class:	BlockModel
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2013.10
// desc	: 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "BlockConfig.h"
#include "ParaXModel/XFileHelper.h"
#include "ParaXModel/XFileStaticModelParser.h"
#include "BlockModel.h"
#include "BlockTemplate.h"
#include "BlockModelManager.h"

/** distance from vine model to surface */
#define VINE_OFFGROUND_OFFSET  0.02f
#define PLATE_OFFGROUND_OFFSET  0.0625f

namespace ParaEngine
{
	BlockModel::BlockModel(int32_t texFaceNum)
		:m_bUseAO(true), m_nFaceCount(6), m_bDisableFaceCulling(false), m_bUseSelfLighting(false), m_bIsCubeAABB(true), m_nTextureIndex(0), m_bUniformLighting(false)
	{
		memset(m_faceShape, 0, sizeof(m_faceShape));
		m_Vertices.resize(24);
		LoadModelByTexture(6);
		m_shapeAABB.SetMinMax(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
	}

	void BlockModel::LoadCubeModel()
	{
		m_bDisableFaceCulling = false;
		m_bUseAO = true;
		m_nFaceCount = 6;
		m_bUseSelfLighting = false;
		m_bIsCubeAABB = true;
		m_nTextureIndex = 0;
		m_bUniformLighting = false;
		m_Vertices.resize(24);
		LoadModelByTexture(6);
		m_shapeAABB.SetMinMax(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
	}

	void BlockModel::CloneRenderData(const BlockModel& from_block)
	{
		m_bUseAO = from_block.m_bUseAO;
		m_nFaceCount = from_block.m_nFaceCount;
		m_bDisableFaceCulling = from_block.m_bDisableFaceCulling;
		m_bUseSelfLighting = from_block.m_bUseSelfLighting;
		m_bIsCubeAABB = from_block.m_bIsCubeAABB;
		m_bUniformLighting = from_block.m_bUniformLighting;
		m_Vertices.resize(from_block.m_Vertices.size());
		memcpy(GetVertices(), from_block.GetVerticesConst(), sizeof(BlockVertexCompressed)*from_block.m_Vertices.size());
	}


	void BlockModel::Clone(const BlockModel& from_block)
	{
		CloneRenderData(from_block);
		m_shapeAABB = from_block.m_shapeAABB;
		m_nTextureIndex = from_block.m_nTextureIndex;
	}


	void BlockModel::LoadModelByTexture( int32_t texFaceNum )
	{
		//top face
		m_Vertices[g_topLB].SetPosition(0,1,0);
		m_Vertices[g_topLT].SetPosition(0,1,1);
		m_Vertices[g_topRT].SetPosition(1,1,1);
		m_Vertices[g_topRB].SetPosition(1,1,0);

		m_Vertices[g_topLB].SetNormal(0,1,0);
		m_Vertices[g_topLT].SetNormal(0,1,0);
		m_Vertices[g_topRT].SetNormal(0,1,0);
		m_Vertices[g_topRB].SetNormal(0,1,0);

		//front face
		m_Vertices[g_frtLB].SetPosition(0,0,0);
		m_Vertices[g_frtLT].SetPosition(0,1,0);
		m_Vertices[g_frtRT].SetPosition(1,1,0);
		m_Vertices[g_frtRB].SetPosition(1,0,0);

		m_Vertices[g_frtLB].SetNormal(0,0,-1);
		m_Vertices[g_frtLT].SetNormal(0,0,-1);
		m_Vertices[g_frtRT].SetNormal(0,0,-1);
		m_Vertices[g_frtRB].SetNormal(0,0,-1);

		//bottom face
		m_Vertices[g_btmLB].SetPosition(0,0,1);
		m_Vertices[g_btmLT].SetPosition(0,0,0);
		m_Vertices[g_btmRT].SetPosition(1,0,0);
		m_Vertices[g_btmRB].SetPosition(1,0,1);

		m_Vertices[g_btmLB].SetNormal(0,-1,0);
		m_Vertices[g_btmLT].SetNormal(0,-1,0);
		m_Vertices[g_btmRT].SetNormal(0,-1,0);
		m_Vertices[g_btmRB].SetNormal(0,-1,0);

		//left face
		m_Vertices[g_leftLB].SetPosition(0,0,1);
		m_Vertices[g_leftLT].SetPosition(0,1,1);
		m_Vertices[g_leftRT].SetPosition(0,1,0);
		m_Vertices[g_leftRB].SetPosition(0,0,0);

		m_Vertices[g_leftLB].SetNormal(-1,0,0);
		m_Vertices[g_leftLT].SetNormal(-1,0,0);
		m_Vertices[g_leftRT].SetNormal(-1,0,0);
		m_Vertices[g_leftRB].SetNormal(-1,0,0);

		//right face
		m_Vertices[g_rightLB].SetPosition(1,0,0);
		m_Vertices[g_rightLT].SetPosition(1,1,0);
		m_Vertices[g_rightRT].SetPosition(1,1,1);
		m_Vertices[g_rightRB].SetPosition(1,0,1);

		m_Vertices[g_rightLB].SetNormal(1,0,0);
		m_Vertices[g_rightLT].SetNormal(1,0,0);
		m_Vertices[g_rightRT].SetNormal(1,0,0);
		m_Vertices[g_rightRB].SetNormal(1,0,0);

		//back face
		m_Vertices[g_bkLB].SetPosition(1,0,1);
		m_Vertices[g_bkLT].SetPosition(1,1,1);
		m_Vertices[g_bkRT].SetPosition(0,1,1);
		m_Vertices[g_bkRB].SetPosition(0,0,1);

		m_Vertices[g_bkLB].SetNormal(0,0,1);
		m_Vertices[g_bkLT].SetNormal(0,0,1);
		m_Vertices[g_bkRT].SetNormal(0,0,1);
		m_Vertices[g_bkRB].SetNormal(0,0,1);

		//////////////////////////////////////////////////////////////////////////
		if(texFaceNum == 3)
		{
			//top face
			// this fixed a flickering bug when using point filter in texture sampler with UV wrapping or clamping. 
			float fUVEpsilon = 0.01f; 
			m_Vertices[g_topLB].SetTexcoord(0 + fUVEpsilon, 0.5f - fUVEpsilon);
			m_Vertices[g_topLT].SetTexcoord(0 + fUVEpsilon,0 + fUVEpsilon);
			m_Vertices[g_topRT].SetTexcoord(0.5f - fUVEpsilon,0 + fUVEpsilon);
			m_Vertices[g_topRB].SetTexcoord(0.5f - fUVEpsilon, 0.5f - fUVEpsilon);

			//front face
			m_Vertices[g_frtLB].SetTexcoord(0,1);
			m_Vertices[g_frtLT].SetTexcoord(0,0.5f);
			m_Vertices[g_frtRT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_frtRB].SetTexcoord(0.5f,1);

			//bottom face
			m_Vertices[g_btmLB].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_btmLT].SetTexcoord(0.5,0);
			m_Vertices[g_btmRT].SetTexcoord(1,0);
			m_Vertices[g_btmRB].SetTexcoord(1,0.5);

			//left face
			m_Vertices[g_leftLB].SetTexcoord(0,1);
			m_Vertices[g_leftLT].SetTexcoord(0,0.5f);
			m_Vertices[g_leftRT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_leftRB].SetTexcoord(0.5f,1);

			//right face
			m_Vertices[g_rightLB].SetTexcoord(0,1);
			m_Vertices[g_rightLT].SetTexcoord(0,0.5f);
			m_Vertices[g_rightRT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_rightRB].SetTexcoord(0.5f,1);

			//back face
			m_Vertices[g_bkLB].SetTexcoord(0,1);
			m_Vertices[g_bkLT].SetTexcoord(0,0.5f);
			m_Vertices[g_bkRT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_bkRB].SetTexcoord(0.5f,1);
		}
		else if(texFaceNum == 4)
		{
			float fUVEpsilon = 0.01f;
			//top face
			m_Vertices[g_topLB].SetTexcoord(0 + fUVEpsilon, 0.5f - fUVEpsilon);
			m_Vertices[g_topLT].SetTexcoord(0 + fUVEpsilon, 0 + fUVEpsilon);
			m_Vertices[g_topRT].SetTexcoord(0.5f - fUVEpsilon, 0 + fUVEpsilon);
			m_Vertices[g_topRB].SetTexcoord(0.5f - fUVEpsilon, 0.5f - fUVEpsilon);
			
			//front face
			m_Vertices[g_frtLB].SetTexcoord(0,1);
			m_Vertices[g_frtLT].SetTexcoord(0,0.5f);
			m_Vertices[g_frtRT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_frtRB].SetTexcoord(0.5f,1);

			//bottom face
			m_Vertices[g_btmLB].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_btmLT].SetTexcoord(0.5,0);
			m_Vertices[g_btmRT].SetTexcoord(1,0);
			m_Vertices[g_btmRB].SetTexcoord(1,0.5);

			//left face
			m_Vertices[g_leftLB].SetTexcoord(0.5f,1);
			m_Vertices[g_leftLT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_leftRT].SetTexcoord(1,0.5f);
			m_Vertices[g_leftRB].SetTexcoord(1,1);

			//right face
			m_Vertices[g_rightLB].SetTexcoord(0.5f,1);
			m_Vertices[g_rightLT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_rightRT].SetTexcoord(1,0.5f);
			m_Vertices[g_rightRB].SetTexcoord(1,1);

			//back face
			m_Vertices[g_bkLB].SetTexcoord(0,1);
			m_Vertices[g_bkLT].SetTexcoord(0,0.5f);
			m_Vertices[g_bkRT].SetTexcoord(0.5f,0.5f);
			m_Vertices[g_bkRB].SetTexcoord(0.5f,1);
		}
		else if(texFaceNum == 6)
		{
			//top face
			m_Vertices[g_topLB].SetTexcoord(0.5,0);
			m_Vertices[g_topLT].SetTexcoord(0.5,1);
			m_Vertices[g_topRT].SetTexcoord(0.625,1);
			m_Vertices[g_topRB].SetTexcoord(0.625,0);

			//front face
			m_Vertices[g_frtLB].SetTexcoord(0.5f,1);
			m_Vertices[g_frtLT].SetTexcoord(0.5f,0);
			m_Vertices[g_frtRT].SetTexcoord(0.375f,0);
			m_Vertices[g_frtRB].SetTexcoord(0.375f,1);

			//bottom face
			m_Vertices[g_btmLB].SetTexcoord(0.625f,0);
			m_Vertices[g_btmLT].SetTexcoord(0.625f,1);
			m_Vertices[g_btmRT].SetTexcoord(0.75f,1);
			m_Vertices[g_btmRB].SetTexcoord(0.75f,0);

			//left face
			m_Vertices[g_leftLB].SetTexcoord(0.125f,1);
			m_Vertices[g_leftLT].SetTexcoord(0.125f,0);
			m_Vertices[g_leftRT].SetTexcoord(0,0);
			m_Vertices[g_leftRB].SetTexcoord(0,1);

			//rgiht face
			m_Vertices[g_rightLB].SetTexcoord(0.375f,1);
			m_Vertices[g_rightLT].SetTexcoord(0.375f,0);
			m_Vertices[g_rightRT].SetTexcoord(0.25f,0);
			m_Vertices[g_rightRB].SetTexcoord(0.25f,1);

			//back face
			m_Vertices[g_bkLB].SetTexcoord(0.25f,1);
			m_Vertices[g_bkLT].SetTexcoord(0.25f,0);
			m_Vertices[g_bkRT].SetTexcoord(0.125f,0);
			m_Vertices[g_bkRB].SetTexcoord(0.125f,1);
		}
		else
		{
			//top face
			m_Vertices[g_topLB].SetTexcoord(0,1);
			m_Vertices[g_topLT].SetTexcoord(0,0);
			m_Vertices[g_topRT].SetTexcoord(1,0);
			m_Vertices[g_topRB].SetTexcoord(1,1);

			//front face
			m_Vertices[g_frtLB].SetTexcoord(0,1);
			m_Vertices[g_frtLT].SetTexcoord(0,0);
			m_Vertices[g_frtRT].SetTexcoord(1,0);
			m_Vertices[g_frtRB].SetTexcoord(1,1);

			//bottom face
			m_Vertices[g_btmLB].SetTexcoord(0,1);
			m_Vertices[g_btmLT].SetTexcoord(0,0);
			m_Vertices[g_btmRT].SetTexcoord(1,0);
			m_Vertices[g_btmRB].SetTexcoord(1,1);

			//left face
			m_Vertices[g_leftLB].SetTexcoord(0,1);
			m_Vertices[g_leftLT].SetTexcoord(0,0);
			m_Vertices[g_leftRT].SetTexcoord(1,0);
			m_Vertices[g_leftRB].SetTexcoord(1,1);

			//right face
			m_Vertices[g_rightLB].SetTexcoord(0,1);
			m_Vertices[g_rightLT].SetTexcoord(0,0);
			m_Vertices[g_rightRT].SetTexcoord(1,0);
			m_Vertices[g_rightRB].SetTexcoord(1,1);

			//back face
			m_Vertices[g_bkLB].SetTexcoord(0,1);
			m_Vertices[g_bkLT].SetTexcoord(0,0);
			m_Vertices[g_bkRT].SetTexcoord(1,0);
			m_Vertices[g_bkRB].SetTexcoord(1,1);
		}
	}

	void BlockModel::SetVertexShadow(int nIndex, unsigned char nShadowLevel)
	{
		m_Vertices[nIndex].SetColorStrength(255 - nShadowLevel);
	}

	uint32 CubeAmbientMaskMap[] = {
		BlockModel::evf_topFront | BlockModel::evf_topLeft | BlockModel::evf_NxyNz,				//g_topLB 
		BlockModel::evf_topLeft | BlockModel::evf_topBack | BlockModel::evf_Nxyz,				//g_topLT 
		BlockModel::evf_topRight | BlockModel::evf_topBack | BlockModel::evf_xyz,				//g_topRT 
		BlockModel::evf_topFront | BlockModel::evf_topRight | BlockModel::evf_xyNz,				//g_topRB 
		BlockModel::evf_LeftFront | BlockModel::evf_bottomFront | BlockModel::evf_NxNyNz,		//g_frtLB 
		BlockModel::evf_topFront | BlockModel::evf_LeftFront | BlockModel::evf_NxyNz,			//g_frtLT 
		BlockModel::evf_topFront | BlockModel::evf_rightFont | BlockModel::evf_xyNz,			//g_frtRT 
		BlockModel::evf_rightFont | BlockModel::evf_bottomFront | BlockModel::evf_xNyNz,		//g_frtRB 
		BlockModel::evf_bottomLeft | BlockModel::evf_bottomBack | BlockModel::evf_NxNyz,		//g_btmLB 
		BlockModel::evf_bottomFront | BlockModel::evf_bottomLeft | BlockModel::evf_NxNyNz,		//g_btmLT 
		BlockModel::evf_bottomFront | BlockModel::evf_bottomRight | BlockModel::evf_xNyNz,		//g_btmRT 
		BlockModel::evf_bottomRight | BlockModel::evf_bottomBack | BlockModel::evf_xNyz,		//g_btmRB 
		BlockModel::evf_leftBack | BlockModel::evf_bottomLeft | BlockModel::evf_NxNyz,			// g_leftLB 
		BlockModel::evf_topLeft | BlockModel::evf_leftBack | BlockModel::evf_Nxyz,				// g_leftLT 
		BlockModel::evf_topLeft | BlockModel::evf_LeftFront | BlockModel::evf_NxyNz,			// g_leftRT 
		BlockModel::evf_LeftFront | BlockModel::evf_bottomLeft | BlockModel::evf_NxNyNz,		// g_leftRB 
		BlockModel::evf_rightFont | BlockModel::evf_bottomRight | BlockModel::evf_xNyNz,		// g_rightLB
		BlockModel::evf_topRight | BlockModel::evf_rightFont | BlockModel::evf_xyNz,			// g_rightLT
		BlockModel::evf_topRight | BlockModel::evf_rightBack | BlockModel::evf_xyz,				// g_rightRT
		BlockModel::evf_rightBack | BlockModel::evf_bottomRight | BlockModel::evf_xNyz,			// g_rightRB
		BlockModel::evf_rightBack | BlockModel::evf_bottomBack | BlockModel::evf_xNyz,			// g_bkLB 
		BlockModel::evf_topBack | BlockModel::evf_rightBack | BlockModel::evf_xyz,				// g_bkLT 
		BlockModel::evf_topBack | BlockModel::evf_leftBack | BlockModel::evf_Nxyz,				// g_bkRT 
		BlockModel::evf_leftBack | BlockModel::evf_bottomBack | BlockModel::evf_NxNyz,			// g_bkRB 
	};

	void BlockModel::SetAOMask(uint32_t aoFlags)
	{
		if((aoFlags & evf_topFront) > 0)
		{
			m_Vertices[g_topLB].SetShadow();
			m_Vertices[g_topRB].SetShadow();
			m_Vertices[g_frtLT].SetShadow();
			m_Vertices[g_frtRT].SetShadow();
		}

		if((aoFlags & evf_topLeft) > 0)
		{
			m_Vertices[g_topLB].SetShadow();
			m_Vertices[g_topLT].SetShadow();
			m_Vertices[g_leftLT].SetShadow();
			m_Vertices[g_leftRT].SetShadow();
		}

		if((aoFlags & evf_topRight) > 0)
		{
			m_Vertices[g_topRB].SetShadow();
			m_Vertices[g_topRT].SetShadow();
			m_Vertices[g_rightLT].SetShadow();
			m_Vertices[g_rightRT].SetShadow();
		}

		if((aoFlags & evf_topBack) > 0)
		{
			m_Vertices[g_topLT].SetShadow();
			m_Vertices[g_topRT].SetShadow();
			m_Vertices[g_bkLT].SetShadow();
			m_Vertices[g_bkRT].SetShadow();
		}

		if((aoFlags & evf_LeftFront) > 0)
		{
			m_Vertices[g_leftRT].SetShadow();
			m_Vertices[g_leftRB].SetShadow();
			m_Vertices[g_frtLT].SetShadow();
			m_Vertices[g_frtLB].SetShadow();
		}

		if((aoFlags & evf_leftBack) > 0)
		{
			m_Vertices[g_leftLB].SetShadow();
			m_Vertices[g_leftLT].SetShadow();
			m_Vertices[g_bkRB].SetShadow();
			m_Vertices[g_bkRT].SetShadow();
		}

		if((aoFlags & evf_rightFont) > 0)
		{
			m_Vertices[g_frtRT].SetShadow();
			m_Vertices[g_frtRB].SetShadow();
			m_Vertices[g_rightLT].SetShadow();
			m_Vertices[g_rightLB].SetShadow();
		}

		if((aoFlags & evf_rightBack) > 0)
		{
			m_Vertices[g_rightRT].SetShadow();
			m_Vertices[g_rightRB].SetShadow();
			m_Vertices[g_bkLT].SetShadow();
			m_Vertices[g_bkLB].SetShadow();
		}

		if((aoFlags & evf_bottomFront) > 0)
		{
			m_Vertices[g_frtLB].SetShadow();
			m_Vertices[g_frtRB].SetShadow();
			m_Vertices[g_btmLT].SetShadow();
			m_Vertices[g_btmRT].SetShadow();
		}

		if((aoFlags & evf_bottomLeft) > 0)
		{
			m_Vertices[g_leftLB].SetShadow();
			m_Vertices[g_leftRB].SetShadow();
			m_Vertices[g_btmLT].SetShadow();
			m_Vertices[g_btmLB].SetShadow();
		}

		if((aoFlags & evf_bottomRight) > 0)
		{
			m_Vertices[g_rightLB].SetShadow();
			m_Vertices[g_rightRB].SetShadow();
			m_Vertices[g_btmRT].SetShadow();
			m_Vertices[g_btmRB].SetShadow();
		}

		if((aoFlags & evf_bottomBack) > 0)
		{
			m_Vertices[g_btmLB].SetShadow();
			m_Vertices[g_btmRB].SetShadow();
			m_Vertices[g_bkLB].SetShadow();
			m_Vertices[g_bkRB].SetShadow();
		}

		if((aoFlags & evf_xyz) > 0)
		{
			m_Vertices[g_topRT].SetShadow();
			m_Vertices[g_rightRT].SetShadow();
			m_Vertices[g_bkLT].SetShadow();
		}

		if((aoFlags & evf_xyNz) > 0)
		{
			m_Vertices[g_topRB].SetShadow();
			m_Vertices[g_rightLT].SetShadow();
			m_Vertices[g_frtRT].SetShadow();
		}

		if((aoFlags & evf_Nxyz) > 0)
		{
			m_Vertices[g_topLT].SetShadow();
			m_Vertices[g_leftLT].SetShadow();
			m_Vertices[g_bkRT].SetShadow();
		}

		if((aoFlags & evf_NxyNz) > 0)
		{
			m_Vertices[g_topLB].SetShadow();
			m_Vertices[g_frtLT].SetShadow();
			m_Vertices[g_leftRT].SetShadow();
		}

		if((aoFlags & evf_xNyz) > 0)
		{
			m_Vertices[g_btmRB].SetShadow();
			m_Vertices[g_rightRB].SetShadow();
			m_Vertices[g_bkLB].SetShadow();
		}

		if((aoFlags & evf_xNyNz) > 0)
		{
			m_Vertices[g_btmRT].SetShadow();
			m_Vertices[g_frtRB].SetShadow();
			m_Vertices[g_rightLB].SetShadow();
		}

		if((aoFlags & evf_NxNyz) > 0)
		{
			m_Vertices[g_btmLB].SetShadow();
			m_Vertices[g_leftLB].SetShadow();
			m_Vertices[g_bkRB].SetShadow();
		}

		if((aoFlags & evf_NxNyNz) > 0)
		{
			m_Vertices[g_btmLT].SetShadow();
			m_Vertices[g_frtLB].SetShadow();
			m_Vertices[g_leftRB].SetShadow();
		}
	}

	void BlockModel::SetVertexHeightScale(int nIndex, float scale)
	{
		m_Vertices[nIndex].SetHeightScale(scale);
	}

	void BlockModel::SetVerticalScale(EdgeVertexFlag vertex,float scale)
	{
		if(vertex == evf_xyz)
		{
			m_Vertices[g_topRT].SetHeightScale(scale);
			m_Vertices[g_rightRT].SetHeightScale(scale);
			m_Vertices[g_bkLT].SetHeightScale(scale);
		}
		else if(vertex == evf_Nxyz)
		{
			m_Vertices[g_topLT].SetHeightScale(scale);
			m_Vertices[g_leftLT].SetHeightScale(scale);
			m_Vertices[g_bkRT].SetHeightScale(scale);
		}
		else if(vertex == evf_xyNz)
		{
			m_Vertices[g_topRB].SetHeightScale(scale);
			m_Vertices[g_rightLT].SetHeightScale(scale);
			m_Vertices[g_frtRT].SetHeightScale(scale);
		}
		else if(vertex == evf_NxyNz)
		{
			m_Vertices[g_topLB].SetHeightScale(scale);
			m_Vertices[g_leftRT].SetHeightScale(scale);
			m_Vertices[g_frtLT].SetHeightScale(scale);
		}
	}

	void BlockModel::TranslateVertices( float dx, float dy, float dz )
	{
		int nVertexCount = GetVerticesCount();
		for (int i = 0; i<nVertexCount; ++i)
		{
			m_Vertices[i].OffsetPosition(dx, dy, dz);
		}
	}

	void BlockModel::SetCategoryID( DWORD nCategoryID )
	{
		int nVertexCount = GetVerticesCount();
		for (int i = 0; i<nVertexCount; ++i)
		{
			m_Vertices[i].SetCategoryID(nCategoryID);
		}
	}

	void BlockModel::SetFaceCount( int nFaceCount )
	{
		m_nFaceCount = nFaceCount;
	}


	int BlockModel::IncrementFaceCount(int nDelta /*= 1*/)
	{
		m_nFaceCount += nDelta;
		return m_nFaceCount;
	}


	void BlockModel::GetBoundingBoxVertices( Vector3 * pVertices, int* pNumber )
	{
		if(IsCubeAABB())
		{
			if(pNumber)
				*pNumber = 8;

			m_Vertices[g_btmLB].GetPosition(pVertices[0]);
			m_Vertices[g_btmLT].GetPosition(pVertices[1]);
			m_Vertices[g_btmRT].GetPosition(pVertices[2]);
			m_Vertices[g_btmRB].GetPosition(pVertices[3]);

			m_Vertices[g_topLB].GetPosition(pVertices[5]);
			m_Vertices[g_topLT].GetPosition(pVertices[4]);
			m_Vertices[g_topRT].GetPosition(pVertices[7]);
			m_Vertices[g_topRB].GetPosition(pVertices[6]);
		}
		else
		{
			BlockModel::GetBoundingBoxVertices(m_shapeAABB, pVertices, pNumber );
		}
	}

	void BlockModel::GetBoundingBoxVertices( CShapeAABB& aabb, Vector3 * pVertices, int* pNumber )
	{
		if(pNumber)
			*pNumber = 8;
		Vector3 vMin = aabb.GetMin();
		Vector3 vMax = aabb.GetMax();
		pVertices[0] = Vector3(vMin.x, vMin.y, vMax.z);
		pVertices[1] = vMin;
		pVertices[2] = Vector3(vMax.x, vMin.y, vMin.z);
		pVertices[3] = Vector3(vMax.x, vMin.y, vMax.z);

		pVertices[5] = Vector3(vMin.x, vMax.y, vMin.z);
		pVertices[4] = Vector3(vMin.x, vMax.y, vMax.z);
		pVertices[7] = vMax;
		pVertices[6] = Vector3(vMax.x, vMax.y, vMin.z);
	}

	void BlockModel::Transform( const Matrix4& mat )
	{
		int nVertexCount = GetFaceCount() * 4;
		for (int i=0;i<nVertexCount; ++i)
		{
			Vector3 v;
			m_Vertices[i].GetPosition(v);
			Vector3 vOut;
			vOut = v*mat;
			m_Vertices[i].SetPosition(vOut.x, vOut.y, vOut.z);
		}
	}

	void BlockModel::Transform( const Vector3& vOffset, float fScaling )
	{
		int nVertexCount = GetFaceCount() * 4;
		for (int i=0;i<nVertexCount; ++i)
		{
			Vector3 v;
			m_Vertices[i].GetPosition(v);
			v *= fScaling;
			v += vOffset;
			m_Vertices[i].SetPosition(v.x, v.y, v.z);
		}
	}

	void BlockModel::RemoveFace( int nFirstVertex )
	{
		m_Vertices[nFirstVertex+1].SetPosition(m_Vertices[nFirstVertex]);
		m_Vertices[nFirstVertex+2].SetPosition(m_Vertices[nFirstVertex]);
		m_Vertices[nFirstVertex+3].SetPosition(m_Vertices[nFirstVertex]);
	}

	BlockVertexCompressed* BlockModel::GetVertices()
	{
		return &(m_Vertices[0]);
	}

	const BlockVertexCompressed* BlockModel::GetVerticesConst() const
	{
		return &(m_Vertices[0]);
	}

	void BlockModel::LoadModel( const std::string& sModelName )
	{
		// TODO: move this code to external file. here we just hard code some basic 3D shape here
		if(sModelName == "shape_two_cross" || sModelName == "grass" || sModelName == "cross")
		{
			m_bUseAO = false;
			m_nFaceCount = 2;
			m_bDisableFaceCulling = true;

			m_Vertices[0].SetPosition(0,1,0);
			m_Vertices[1].SetPosition(1,1,1);
			m_Vertices[2].SetPosition(1,0,1);
			m_Vertices[3].SetPosition(0,0,0);

			m_Vertices[4].SetPosition(1,1,0);
			m_Vertices[5].SetPosition(0,1,1);
			m_Vertices[6].SetPosition(0,0,1);
			m_Vertices[7].SetPosition(1,0,0);

			m_Vertices[0].SetTexcoord(0,0);
			m_Vertices[1].SetTexcoord(1,0);
			m_Vertices[2].SetTexcoord(1,1);
			m_Vertices[3].SetTexcoord(0,1);

			m_Vertices[4].SetTexcoord(0,0);
			m_Vertices[5].SetTexcoord(1,0);
			m_Vertices[6].SetTexcoord(1,1);
			m_Vertices[7].SetTexcoord(0,1);
		}
		else if(sModelName == "slab_top" || sModelName == "slab_bottom")
		{
			float u, v;
			m_Vertices[g_frtRT].GetTexcoord(u, v);
			float left_u = 0, left_v = 0.5f;
			float right_u = 1, right_v = 0.5f;
			if(u == 0.5f)
			{
				right_u = 0.5f;
				left_v = 0.75f;
				right_v = 0.75f; 
			}

			//front face
			m_Vertices[g_frtLB].SetTexcoord(left_u, left_v);
			m_Vertices[g_frtRB].SetTexcoord(right_u, right_v);

			//back face
			m_Vertices[g_bkLB].SetTexcoord(left_u, left_v);
			m_Vertices[g_bkRB].SetTexcoord(right_u, right_v);

			//left face
			m_Vertices[g_leftLB].SetTexcoord(left_u, left_v);
			m_Vertices[g_leftRB].SetTexcoord(right_u, right_v);

			//right face
			m_Vertices[g_rightLB].SetTexcoord(left_u, left_v);
			m_Vertices[g_rightRB].SetTexcoord(right_u, right_v);

			if(sModelName == "slab_top")
			{
				//top face
				m_Vertices[g_topLB].SetPosition(0,0.5f,0);
				m_Vertices[g_topLT].SetPosition(0,0.5f,1);
				m_Vertices[g_topRT].SetPosition(1,0.5f,1);
				m_Vertices[g_topRB].SetPosition(1,0.5f,0);

				//front face
				m_Vertices[g_frtLT].SetPosition(0,0.5f,0);
				m_Vertices[g_frtRT].SetPosition(1,0.5f,0);

				//left face
				m_Vertices[g_leftLT].SetPosition(0,0.5f,1);
				m_Vertices[g_leftRT].SetPosition(0,0.5f,0);


				//right face
				m_Vertices[g_rightLT].SetPosition(1,0.5f,0);
				m_Vertices[g_rightRT].SetPosition(1,0.5f,1);

				//back face
				m_Vertices[g_bkLT].SetPosition(1,0.5f,1);
				m_Vertices[g_bkRT].SetPosition(0,0.5f,1);

				SetAABB(Vector3(0,0,0), Vector3(BlockConfig::g_blockSize,BlockConfig::g_blockSize*0.5f,BlockConfig::g_blockSize));
			}
			else
			{
				//front face
				m_Vertices[g_frtLB].SetPosition(0,0.5f,0);
				m_Vertices[g_frtRB].SetPosition(1,0.5f,0);

				//bottom face
				m_Vertices[g_btmLB].SetPosition(0,0.5f,1);
				m_Vertices[g_btmLT].SetPosition(0,0.5f,0);
				m_Vertices[g_btmRT].SetPosition(1,0.5f,0);
				m_Vertices[g_btmRB].SetPosition(1,0.5f,1);

				//left face
				m_Vertices[g_leftLB].SetPosition(0,0.5f,1);
				m_Vertices[g_leftRB].SetPosition(0,0.5f,0);


				//right face
				m_Vertices[g_rightLB].SetPosition(1,0.5f,0);
				m_Vertices[g_rightRB].SetPosition(1,0.5f,1);

				//back face
				m_Vertices[g_bkLB].SetPosition(1,0.5f,1);
				m_Vertices[g_bkRB].SetPosition(0,0.5f,1);

				SetAABB(Vector3(0,0.5f*BlockConfig::g_blockSize,0), Vector3(BlockConfig::g_blockSize,BlockConfig::g_blockSize,BlockConfig::g_blockSize));
			}
		}
		else if(sModelName.find("halfvine") == 0)
		{
			m_bUseAO = false;
			m_nFaceCount = 1;
			m_bDisableFaceCulling = true;

			const float fOffset = VINE_OFFGROUND_OFFSET;
			if(sModelName == "halfvine0")
			{
				//top face
				m_Vertices[g_topLB].SetPosition(0,1-fOffset,0);
				m_Vertices[g_topLT].SetPosition(0,1-fOffset,1);
				m_Vertices[g_topRT].SetPosition(1,1-fOffset,1);
				m_Vertices[g_topRB].SetPosition(1,1-fOffset,0);
				m_Vertices[0] = m_Vertices[g_topLB];
				m_Vertices[1] = m_Vertices[g_topLT];
				m_Vertices[2] = m_Vertices[g_topRT];
				m_Vertices[3] = m_Vertices[g_topRB];

				SetAABB(Vector3(0,0.92f*BlockConfig::g_blockSize,0), Vector3(BlockConfig::g_blockSize,BlockConfig::g_blockSize,BlockConfig::g_blockSize));
			}
			else if(sModelName == "halfvine1")
			{
				//front face
				m_Vertices[g_frtLB].SetPosition(0,0,fOffset);
				m_Vertices[g_frtLT].SetPosition(0,1,fOffset);
				m_Vertices[g_frtRT].SetPosition(1,1,fOffset);
				m_Vertices[g_frtRB].SetPosition(1,0,fOffset);
				m_Vertices[0] = m_Vertices[g_frtLB];
				m_Vertices[1] = m_Vertices[g_frtLT];
				m_Vertices[2] = m_Vertices[g_frtRT];
				m_Vertices[3] = m_Vertices[g_frtRB];
				SetAABB(Vector3(0,0,0), Vector3(BlockConfig::g_blockSize,BlockConfig::g_blockSize*0.5f,0.08f*BlockConfig::g_blockSize));
			}
			else if(sModelName == "halfvine2")
			{
				//bottom face
				m_Vertices[g_btmLB].SetPosition(0,fOffset,1);
				m_Vertices[g_btmLT].SetPosition(0,fOffset,0);
				m_Vertices[g_btmRT].SetPosition(1,fOffset,0);
				m_Vertices[g_btmRB].SetPosition(1,fOffset,1);
				m_Vertices[0] = m_Vertices[g_btmLB];
				m_Vertices[1] = m_Vertices[g_btmLT];
				m_Vertices[2] = m_Vertices[g_btmRT];
				m_Vertices[3] = m_Vertices[g_btmRB];

				SetAABB(Vector3(0,0,0), Vector3(BlockConfig::g_blockSize,0.08f*BlockConfig::g_blockSize,BlockConfig::g_blockSize));
			}
			else if(sModelName == "halfvine3")
			{
				//left face
				m_Vertices[g_leftLB].SetPosition(fOffset,0,1);
				m_Vertices[g_leftLT].SetPosition(fOffset,1,1);
				m_Vertices[g_leftRT].SetPosition(fOffset,1,0);
				m_Vertices[g_leftRB].SetPosition(fOffset,0,0);
				m_Vertices[0] = m_Vertices[g_leftLB];
				m_Vertices[1] = m_Vertices[g_leftLT];
				m_Vertices[2] = m_Vertices[g_leftRT];
				m_Vertices[3] = m_Vertices[g_leftRB];

				SetAABB(Vector3(0,0,0), Vector3(0.08f*BlockConfig::g_blockSize,BlockConfig::g_blockSize*0.5f,BlockConfig::g_blockSize));
			}
			else if(sModelName == "halfvine4")
			{
				//right face
				m_Vertices[g_rightLB].SetPosition(1-fOffset,0,0);
				m_Vertices[g_rightLT].SetPosition(1-fOffset,1,0);
				m_Vertices[g_rightRT].SetPosition(1-fOffset,1,1);
				m_Vertices[g_rightRB].SetPosition(1-fOffset,0,1);
				m_Vertices[0] = m_Vertices[g_rightLB];
				m_Vertices[1] = m_Vertices[g_rightLT];
				m_Vertices[2] = m_Vertices[g_rightRT];
				m_Vertices[3] = m_Vertices[g_rightRB];

				SetAABB(Vector3(0.92f*BlockConfig::g_blockSize,0,0), Vector3(BlockConfig::g_blockSize,BlockConfig::g_blockSize*0.5f,BlockConfig::g_blockSize));
			}
			else if(sModelName == "halfvine5")
			{	
				//back face
				m_Vertices[g_bkLB].SetPosition(1,0,1-fOffset);
				m_Vertices[g_bkLT].SetPosition(1,1,1-fOffset);
				m_Vertices[g_bkRT].SetPosition(0,1,1-fOffset);
				m_Vertices[g_bkRB].SetPosition(0,0,1-fOffset);
				m_Vertices[0] = m_Vertices[g_bkLB];
				m_Vertices[1] = m_Vertices[g_bkLT];
				m_Vertices[2] = m_Vertices[g_bkRT];
				m_Vertices[3] = m_Vertices[g_bkRB];
				SetAABB(Vector3(0,0,0.92f*BlockConfig::g_blockSize), Vector3(BlockConfig::g_blockSize,BlockConfig::g_blockSize*0.5f,BlockConfig::g_blockSize));
			}
		}
		else if (sModelName.find("plate") == 0)
		{
			// used as carpet

			const float fOffset = BlockConfig::g_blockSize/16.f;
			if (sModelName == "plate0")
			{
				//top face
				m_Vertices[g_topLB].SetPosition(0, fOffset, 0);
				m_Vertices[g_topLT].SetPosition(0, fOffset, 1);
				m_Vertices[g_topRT].SetPosition(1, fOffset, 1);
				m_Vertices[g_topRB].SetPosition(1, fOffset, 0);
				
				//front face
				m_Vertices[g_frtLT].SetPosition(0, fOffset, 0);
				m_Vertices[g_frtRT].SetPosition(1, fOffset, 0);

				//left face
				m_Vertices[g_leftLT].SetPosition(0, fOffset, 1);
				m_Vertices[g_leftRT].SetPosition(0, fOffset, 0);


				//right face
				m_Vertices[g_rightLT].SetPosition(1, fOffset, 0);
				m_Vertices[g_rightRT].SetPosition(1, fOffset, 1);

				//back face
				m_Vertices[g_bkLT].SetPosition(1, fOffset, 1);
				m_Vertices[g_bkRT].SetPosition(0, fOffset, 1);

				SetAABB(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, 0.08f*BlockConfig::g_blockSize, BlockConfig::g_blockSize));
				
			}
			else if (sModelName == "plate1")
			{
				//top face
				m_Vertices[g_topLT].SetPosition(0, 1, fOffset);
				m_Vertices[g_topRT].SetPosition(1, 1, fOffset);
				
				//bottom face
				m_Vertices[g_btmLB].SetPosition(0, 0, fOffset);
				m_Vertices[g_btmRB].SetPosition(1, 0, fOffset);


				//left face
				m_Vertices[g_leftLB].SetPosition(0, 0, fOffset);
				m_Vertices[g_leftLT].SetPosition(0, 1, fOffset);
				
				//right face
				m_Vertices[g_rightRT].SetPosition(1, 1, fOffset);
				m_Vertices[g_rightRB].SetPosition(1, 0, fOffset);

				//back face
				m_Vertices[g_bkLB].SetPosition(1, 0, fOffset);
				m_Vertices[g_bkLT].SetPosition(1, 1, fOffset);
				m_Vertices[g_bkRT].SetPosition(0, 1, fOffset);
				m_Vertices[g_bkRB].SetPosition(0, 0, fOffset);

				SetAABB(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, 0.08f*BlockConfig::g_blockSize));
			}
			else if (sModelName == "plate2")
			{
				//bottom face
				m_Vertices[g_btmLB].SetPosition(0, 1 - fOffset, 1);
				m_Vertices[g_btmLT].SetPosition(0, 1 - fOffset, 0);
				m_Vertices[g_btmRT].SetPosition(1, 1 - fOffset, 0);
				m_Vertices[g_btmRB].SetPosition(1, 1 - fOffset, 1);

				//front face
				m_Vertices[g_frtLB].SetPosition(0, 1 - fOffset, 0);
				m_Vertices[g_frtRB].SetPosition(1, 1 - fOffset, 0);

				//left face
				m_Vertices[g_leftLB].SetPosition(0, 1 - fOffset, 1);
				m_Vertices[g_leftRB].SetPosition(0, 1 - fOffset, 0);

				//right face
				m_Vertices[g_rightLB].SetPosition(1, 1 - fOffset, 0);
				m_Vertices[g_rightRB].SetPosition(1, 1 - fOffset, 1);

				//back face
				m_Vertices[g_bkLB].SetPosition(1, 1 - fOffset, 1);
				m_Vertices[g_bkRB].SetPosition(0, 1 - fOffset, 1);

				SetAABB(Vector3(0, 0.92f*BlockConfig::g_blockSize, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
			}
			else if (sModelName == "plate3")
			{
				//front face
				m_Vertices[g_frtRT].SetPosition(fOffset, 1, 0);
				m_Vertices[g_frtRB].SetPosition(fOffset, 0, 0);

				//top face
				m_Vertices[g_topRT].SetPosition(fOffset, 1, 1);
				m_Vertices[g_topRB].SetPosition(fOffset, 1, 0);

				//front face
				m_Vertices[g_frtRT].SetPosition(fOffset, 1, 0);
				m_Vertices[g_frtRB].SetPosition(fOffset, 0, 0);


				//bottom face
				m_Vertices[g_btmRT].SetPosition(fOffset, 0, 0);
				m_Vertices[g_btmRB].SetPosition(fOffset, 0, 1);

				//right face
				m_Vertices[g_rightLB].SetPosition(fOffset, 0, 0);
				m_Vertices[g_rightLT].SetPosition(fOffset, 1, 0);
				m_Vertices[g_rightRT].SetPosition(fOffset, 1, 1);
				m_Vertices[g_rightRB].SetPosition(fOffset, 0, 1);

				//back face
				m_Vertices[g_bkLB].SetPosition(fOffset, 0, 1);
				m_Vertices[g_bkLT].SetPosition(fOffset, 1, 1);

				SetAABB(Vector3(0, 0, 0), Vector3(0.08f*BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
			}
			else if (sModelName == "plate4")
			{
				//front face
				m_Vertices[g_frtLB].SetPosition(0, 0, 0);
				m_Vertices[g_frtLT].SetPosition(0, 1, 0);
				m_Vertices[g_frtRT].SetPosition(1, 1, 0);
				m_Vertices[g_frtRB].SetPosition(1, 0, 0);

				//top face
				m_Vertices[g_topLB].SetPosition(1 - fOffset, 1, 0);
				m_Vertices[g_topLT].SetPosition(1 - fOffset, 1, 1);
				
				//front face
				m_Vertices[g_frtLB].SetPosition(1 - fOffset, 0, 0);
				m_Vertices[g_frtLT].SetPosition(1 - fOffset, 1, 0);
				

				//bottom face
				m_Vertices[g_btmLB].SetPosition(1 - fOffset, 0, 1);
				m_Vertices[g_btmLT].SetPosition(1 - fOffset, 0, 0);

				//left face
				m_Vertices[g_leftLB].SetPosition(1 - fOffset, 0, 1);
				m_Vertices[g_leftLT].SetPosition(1 - fOffset, 1, 1);
				m_Vertices[g_leftRT].SetPosition(1 - fOffset, 1, 0);
				m_Vertices[g_leftRB].SetPosition(1 - fOffset, 0, 0);


				//back face
				m_Vertices[g_bkRT].SetPosition(1 - fOffset, 1, 1);
				m_Vertices[g_bkRB].SetPosition(1 - fOffset, 0, 1);
				
				SetAABB(Vector3(0.92f*BlockConfig::g_blockSize, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
			}
			else if (sModelName == "plate5")
			{
				//front face
				m_Vertices[g_frtLB].SetPosition(0, 0, 1 - fOffset);
				m_Vertices[g_frtLT].SetPosition(0, 1, 1 - fOffset);
				m_Vertices[g_frtRT].SetPosition(1, 1, 1 - fOffset);
				m_Vertices[g_frtRB].SetPosition(1, 0, 1 - fOffset);

				//top face
				m_Vertices[g_topLB].SetPosition(0, 1, 1 - fOffset);
				m_Vertices[g_topRB].SetPosition(1, 1, 1 - fOffset);

				//bottom face
				m_Vertices[g_btmLT].SetPosition(0, 0, 1 - fOffset);
				m_Vertices[g_btmRT].SetPosition(1, 0, 1 - fOffset);

				//left face
				m_Vertices[g_leftRT].SetPosition(0, 1, 1 - fOffset);
				m_Vertices[g_leftRB].SetPosition(0, 0, 1 - fOffset);

				//right face
				m_Vertices[g_rightLB].SetPosition(1, 0, 1 - fOffset);
				m_Vertices[g_rightLT].SetPosition(1, 1, 1 - fOffset);

				SetAABB(Vector3(0, 0, 0.92f*BlockConfig::g_blockSize), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
			}
		}
		else if(sModelName.find("vine") == 0)
		{
			// sModelName is 'vine[side][direction]', such as vine0 or vine01
			m_bUseAO = false;
			m_nFaceCount = 1;
			m_bDisableFaceCulling = true;

			if (sModelName.size() >= 5)
			{
				const float fOffset = VINE_OFFGROUND_OFFSET;
				int nSide = sModelName[4] - '0';
				if (nSide == 0)
				{
					//top face
					m_Vertices[g_topLB].SetPosition(0, 1 - fOffset, 0);
					m_Vertices[g_topLT].SetPosition(0, 1 - fOffset, 1);
					m_Vertices[g_topRT].SetPosition(1, 1 - fOffset, 1);
					m_Vertices[g_topRB].SetPosition(1, 1 - fOffset, 0);
					m_Vertices[g_topLB].SetNormal(-m_Vertices[g_topLB].GetNormal());
					m_Vertices[g_topLT].SetNormal(-m_Vertices[g_topLT].GetNormal());
					m_Vertices[g_topRT].SetNormal(-m_Vertices[g_topRT].GetNormal());
					m_Vertices[g_topRB].SetNormal(-m_Vertices[g_topRB].GetNormal());
					m_Vertices[0] = m_Vertices[g_topLB];
					m_Vertices[1] = m_Vertices[g_topLT];
					m_Vertices[2] = m_Vertices[g_topRT];
					m_Vertices[3] = m_Vertices[g_topRB];

					SetAABB(Vector3(0, 0.92f*BlockConfig::g_blockSize, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
				}
				else if (nSide == 1)
				{
					//front face
					m_Vertices[g_frtLB].SetPosition(0, 0, fOffset);
					m_Vertices[g_frtLT].SetPosition(0, 1, fOffset);
					m_Vertices[g_frtRT].SetPosition(1, 1, fOffset);
					m_Vertices[g_frtRB].SetPosition(1, 0, fOffset);
					m_Vertices[0] = m_Vertices[g_frtLB];
					m_Vertices[1] = m_Vertices[g_frtLT];
					m_Vertices[2] = m_Vertices[g_frtRT];
					m_Vertices[3] = m_Vertices[g_frtRB];
					SetAABB(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, 0.08f*BlockConfig::g_blockSize));
				}
				else if (nSide == 2)
				{
					//bottom face
					m_Vertices[g_btmLB].SetPosition(0, fOffset, 1);
					m_Vertices[g_btmLT].SetPosition(0, fOffset, 0);
					m_Vertices[g_btmRT].SetPosition(1, fOffset, 0);
					m_Vertices[g_btmRB].SetPosition(1, fOffset, 1);
					m_Vertices[g_btmLB].SetNormal(-m_Vertices[g_btmLB].GetNormal());
					m_Vertices[g_btmLT].SetNormal(-m_Vertices[g_btmLT].GetNormal());
					m_Vertices[g_btmRT].SetNormal(-m_Vertices[g_btmRT].GetNormal());
					m_Vertices[g_btmRB].SetNormal(-m_Vertices[g_btmRB].GetNormal());
					m_Vertices[0] = m_Vertices[g_btmLB];
					m_Vertices[1] = m_Vertices[g_btmLT];
					m_Vertices[2] = m_Vertices[g_btmRT];
					m_Vertices[3] = m_Vertices[g_btmRB];

					SetAABB(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, 0.08f*BlockConfig::g_blockSize, BlockConfig::g_blockSize));
				}
				else if (nSide == 3)
				{
					//left face
					m_Vertices[g_leftLB].SetPosition(fOffset, 0, 1);
					m_Vertices[g_leftLT].SetPosition(fOffset, 1, 1);
					m_Vertices[g_leftRT].SetPosition(fOffset, 1, 0);
					m_Vertices[g_leftRB].SetPosition(fOffset, 0, 0);
					m_Vertices[0] = m_Vertices[g_leftLB];
					m_Vertices[1] = m_Vertices[g_leftLT];
					m_Vertices[2] = m_Vertices[g_leftRT];
					m_Vertices[3] = m_Vertices[g_leftRB];

					SetAABB(Vector3(0, 0, 0), Vector3(0.08f*BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
				}
				else if (nSide == 4)
				{
					//right face
					m_Vertices[g_rightLB].SetPosition(1 - fOffset, 0, 0);
					m_Vertices[g_rightLT].SetPosition(1 - fOffset, 1, 0);
					m_Vertices[g_rightRT].SetPosition(1 - fOffset, 1, 1);
					m_Vertices[g_rightRB].SetPosition(1 - fOffset, 0, 1);
					m_Vertices[0] = m_Vertices[g_rightLB];
					m_Vertices[1] = m_Vertices[g_rightLT];
					m_Vertices[2] = m_Vertices[g_rightRT];
					m_Vertices[3] = m_Vertices[g_rightRB];

					SetAABB(Vector3(0.92f*BlockConfig::g_blockSize, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
				}
				else if (nSide == 5)
				{
					//back face
					m_Vertices[g_bkLB].SetPosition(1, 0, 1 - fOffset);
					m_Vertices[g_bkLT].SetPosition(1, 1, 1 - fOffset);
					m_Vertices[g_bkRT].SetPosition(0, 1, 1 - fOffset);
					m_Vertices[g_bkRB].SetPosition(0, 0, 1 - fOffset);
					m_Vertices[0] = m_Vertices[g_bkLB];
					m_Vertices[1] = m_Vertices[g_bkLT];
					m_Vertices[2] = m_Vertices[g_bkRT];
					m_Vertices[3] = m_Vertices[g_bkRB];
					SetAABB(Vector3(0, 0, 0.92f*BlockConfig::g_blockSize), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
				}
				if (sModelName.size() >= 6)
				{
					int nDir = sModelName[5] - '0';

					for (int i = 0; i < nDir; ++i)
					{
						float u0, v0, u, v;
						m_Vertices[0].GetTexcoord(u0, v0);
						m_Vertices[1].GetTexcoord(u, v);
						m_Vertices[0].SetTexcoord(u, v);
						m_Vertices[2].GetTexcoord(u, v);
						m_Vertices[1].SetTexcoord(u, v);
						m_Vertices[3].GetTexcoord(u, v);
						m_Vertices[2].SetTexcoord(u, v);
						m_Vertices[3].SetTexcoord(u0, v0);
					}
				}
			}
		}
		else if(sModelName.find("cross") == 0 && sModelName.size()>=5)
		{
			// sModelName such as "cross0/4", sharing a horizontal texture
			m_bUseAO = false;
			m_bDisableFaceCulling = true;
			m_nFaceCount = 2;
			m_Vertices[0].SetPosition(0,1,0);
			m_Vertices[1].SetPosition(1,1,1);
			m_Vertices[2].SetPosition(1,0,1);
			m_Vertices[3].SetPosition(0,0,0);

			m_Vertices[4].SetPosition(1,1,0);
			m_Vertices[5].SetPosition(0,1,1);
			m_Vertices[6].SetPosition(0,0,1);
			m_Vertices[7].SetPosition(1,0,0);

			m_Vertices[0].SetTexcoord(0,0);
			m_Vertices[1].SetTexcoord(1,0);
			m_Vertices[2].SetTexcoord(1,1);
			m_Vertices[3].SetTexcoord(0,1);

			m_Vertices[4].SetTexcoord(0,0);
			m_Vertices[5].SetTexcoord(1,0);
			m_Vertices[6].SetTexcoord(1,1);
			m_Vertices[7].SetTexcoord(0,1);

			int nOffset = sModelName[5] - '0';
			if(nOffset>=0 && nOffset<32)
			{
				float fStep = 1/4.f;
				float fFrom = nOffset * fStep;

				for (int i=0; i<8; ++i)
				{
					float u, v;
					m_Vertices[i].GetTexcoord(u, v);
					m_Vertices[i].SetTexcoord(fFrom + u*fStep, v);
				}
			}
		}
		else if(sModelName.find("seed") == 0 && sModelName.size()>=5)
		{
			// sModelName such as "seed0/4", sharing a horizontal texture
			// a flat face on ground
			m_bUseAO = false;
			m_bDisableFaceCulling = true;
			const float fOffset = VINE_OFFGROUND_OFFSET;

			m_nFaceCount = 1;
			m_Vertices[g_btmLB].SetPosition(0,fOffset,1);
			m_Vertices[g_btmLT].SetPosition(0,fOffset,0);
			m_Vertices[g_btmRT].SetPosition(1,fOffset,0);
			m_Vertices[g_btmRB].SetPosition(1,fOffset,1);
			m_Vertices[0] = m_Vertices[g_btmLB];
			m_Vertices[1] = m_Vertices[g_btmLT];
			m_Vertices[2] = m_Vertices[g_btmRT];
			m_Vertices[3] = m_Vertices[g_btmRB];

			int nOffset = sModelName[4] - '0';
			if(nOffset>=0 && nOffset<32)
			{
				float fStep = 1/4.f;
				float fFrom = nOffset * fStep;

				for (int i=0; i<4; ++i)
				{
					float u, v;
					m_Vertices[i].GetTexcoord(u, v);
					m_Vertices[i].SetTexcoord(fFrom + u*fStep, v);
				}
			}
			SetAABB(Vector3(0,0,0), Vector3(BlockConfig::g_blockSize,0.08f*BlockConfig::g_blockSize,BlockConfig::g_blockSize));
		}
		else if(sModelName == "cactus")
		{
			const float fPosOffset = 2.f/32.f;
			//front face
			m_Vertices[g_frtLB].SetPosition(0,0,fPosOffset);
			m_Vertices[g_frtLT].SetPosition(0,1,fPosOffset);
			m_Vertices[g_frtRT].SetPosition(1,1,fPosOffset);
			m_Vertices[g_frtRB].SetPosition(1,0,fPosOffset);

			//left face
			m_Vertices[g_leftLB].SetPosition(fPosOffset,0,1);
			m_Vertices[g_leftLT].SetPosition(fPosOffset,1,1);
			m_Vertices[g_leftRT].SetPosition(fPosOffset,1,0);
			m_Vertices[g_leftRB].SetPosition(fPosOffset,0,0);

			//right face
			m_Vertices[g_rightLB].SetPosition(1-fPosOffset,0,0);
			m_Vertices[g_rightLT].SetPosition(1-fPosOffset,1,0);
			m_Vertices[g_rightRT].SetPosition(1-fPosOffset,1,1);
			m_Vertices[g_rightRB].SetPosition(1-fPosOffset,0,1);

			//back face
			m_Vertices[g_bkLB].SetPosition(1,0,1-fPosOffset);
			m_Vertices[g_bkLT].SetPosition(1,1,1-fPosOffset);
			m_Vertices[g_bkRT].SetPosition(0,1,1-fPosOffset);
			m_Vertices[g_bkRB].SetPosition(0,0,1-fPosOffset);

			SetAABB(Vector3(fPosOffset*BlockConfig::g_blockSize,0,fPosOffset*BlockConfig::g_blockSize), Vector3((1-fPosOffset)*BlockConfig::g_blockSize,BlockConfig::g_blockSize,(1-fPosOffset)*BlockConfig::g_blockSize));
		}
	}

	void BlockModel::GetAABB( CShapeAABB* pOut ) const
	{
		*pOut = m_shapeAABB;
	}

	const CShapeAABB& BlockModel::GetAABB() const
	{
		return m_shapeAABB;
	}

	void BlockModel::SetAABB(const Vector3& vMin, const Vector3& vMax)
	{
		m_shapeAABB.SetMinMax(vMin, vMax);
		m_bIsCubeAABB = false;
	}

	bool BlockModel::IsCubeAABB()
	{
		return m_bIsCubeAABB;
	}

	void BlockModel::SetIsCubeAABB( bool bIsCube )
	{
		if (m_bIsCubeAABB != bIsCube)
		{
			m_bIsCubeAABB = bIsCube;
			if (m_bIsCubeAABB)
			{
				m_shapeAABB.SetMinMax(Vector3(0, 0, 0), Vector3(BlockConfig::g_blockSize, BlockConfig::g_blockSize, BlockConfig::g_blockSize));
			}
		}
	}

	int BlockModel::GetVerticesCount()
	{
		return m_nFaceCount*4;
	}
	
	void BlockModel::ReserveVertices(int nReservedSize /*= 24*/)
	{
		m_Vertices.reserve(nReservedSize);
	}

	void BlockModel::LoadModel(BlockTemplate* pTemplate, const std::string& filename, const Matrix4& mat, int nTextureIndex)
	{
		try
		{
			SetUseAmbientOcclusion(false);
			SetUniformLighting(true);
			using namespace XFile;
			Scene* pScene = BlockModelManager::GetInstance().GetXFile(filename);
			if (pScene)
			{
				m_nTextureIndex = nTextureIndex;
				Vector3 vMin = pScene->m_header.minExtent;
				vMin = vMin*mat;
				vMin += Vector3(0.5f, 0.5f, 0.5f);
				Vector3 vMax = pScene->m_header.maxExtent;
				vMax = vMax*mat;
				vMax += Vector3(0.5f, 0.5f, 0.5f);
				if (vMin.x > vMax.x)
					std::swap(vMin.x, vMax.x);
				if (vMin.y > vMax.y)
					std::swap(vMin.y, vMax.y);
				if (vMin.z > vMax.z)
					std::swap(vMin.z, vMax.z);
					
				if (vMin != Vector3::ZERO || vMax != Vector3::UNIT_SCALE)
				{
					SetAABB(vMin, vMax);
					// OUTPUT_LOG("%s %f %f %f, %f %f %f\n", filename.c_str(), vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z);
				}
					
				int nVertexCount = 0;
				int nIndexCount = 0;

				for (Mesh* mesh : pScene->mGlobalMeshes)
				{
					nVertexCount += mesh->mPositions.size();
					nIndexCount += mesh->mPosFaces.size() * 3;
				}
				bool bIsRectFaceModel = pScene->m_header.nModelFormat > 0;
				if (bIsRectFaceModel)
				{
					// we will count two triangles as a single rect face here. 
					m_nFaceCount = nIndexCount / 3 / 2;
					if (m_nFaceCount * 6 != nIndexCount)
					{
						OUTPUT_LOG("warn: only even number of triangle faces are supported for model:%s \n", filename.c_str());
					}
				}
				else
				{
					// because the index buffer is shared, custom block model always have the even number of face to be a infinit small face. 
					// this doubles the memory cost. In future, we should optimize the mesh more. 
					m_nFaceCount = nIndexCount / 3;
				}
				m_Vertices.resize(m_nFaceCount*4);

				BlockVertexCompressed* pCurVert = &(m_Vertices[0]);

				for (Mesh* mesh : pScene->mGlobalMeshes)
				{
					int nCount = mesh->mPositions.size();
					PE_ASSERT(nCount == mesh->mNormals.size());
					PE_ASSERT(nCount == mesh->mTexCoords[0].size());
					auto mPositions = mesh->mPositions;
					auto mNormals = mesh->mNormals;
					// prepare vertex for the sub mesh
					for (int i = 0; i < nCount; ++i)
					{
						// offset Y by 0.5
						Vector3 vPos = mPositions[i];
						vPos = vPos * mat;
						vPos.x += 0.5f; vPos.y += 0.5f; vPos.z += 0.5f;
						mPositions[i] = vPos;
						mNormals[i] = mNormals[i].TransformNormal(mat);
					}

					// prepare indices for the sub mesh
					if (!bIsRectFaceModel)
					{
						// non-rect face model will cost 4 times more memory to hold the triangle data. 
						// and draws 2 times more triangles than rect face models, since the event number of triangle is made invisible. 
						for (int i = 0; i < m_nFaceCount; ++i)
						{
							Face& face = mesh->mPosFaces[i];
							for (int nFace = 0; nFace < 3; ++nFace)
							{
								uint16 nVertexIndex = face.mIndices[nFace];
								BlockVertexCompressed& vert = (*pCurVert);
								pCurVert++;
								vert.SetPosition(mPositions[nVertexIndex]);
								vert.SetNormal(mNormals[nVertexIndex]);
								vert.SetTexcoord(mesh->mTexCoords[0][nVertexIndex]);
							}
							// make the even number of face disappear. 
							BlockVertexCompressed& vert = (*pCurVert);
							vert = *(pCurVert - 1);
							pCurVert++;
						}
					}
					else
					{
						// Note: the indices must be face1(0,1,2)  face2(2,0,3) 
						int nFaceIndex = 0;
						for (int i = 0; i < m_nFaceCount; ++i)
						{
							Face& face1 = mesh->mPosFaces[nFaceIndex++];
							Face& face2 = mesh->mPosFaces[nFaceIndex++];
							for (int nFace = 0; nFace < 3; ++nFace)
							{
								uint16 nVertexIndex = face1.mIndices[nFace];
								BlockVertexCompressed& vert = (*pCurVert);
								pCurVert++;
								vert.SetPosition(mPositions[nVertexIndex]);
								vert.SetNormal(mNormals[nVertexIndex]);
								vert.SetTexcoord(mesh->mTexCoords[0][nVertexIndex]);
							}
							BlockVertexCompressed& vert = (*pCurVert);
							pCurVert++;
							uint16 nVertexIndex = face2.mIndices[2];
							vert.SetPosition(mPositions[nVertexIndex]);
							vert.SetNormal(mNormals[nVertexIndex]);
							vert.SetTexcoord(mesh->mTexCoords[0][nVertexIndex]);
						}
					}
				}
			}
			SetCategoryID(pTemplate->GetCategoryID());
		}
		catch (...)
		{
		}
	}

	int BlockModel::GetTextureIndex() const
	{
		return m_nTextureIndex;
	}

	void BlockModel::SetTextureIndex(int val)
	{
		m_nTextureIndex = val;
	}

	void BlockModel::ClearVertices()
	{
		m_Vertices.clear();
		m_nFaceCount = 0;
	}

	int BlockModel::AddVertex(const BlockVertexCompressed& vertex)
	{
		m_Vertices.push_back(vertex);
		return m_Vertices.size() - 1;
	}

	int BlockModel::AddVertex(const BlockModel& from_block, int32 nVertexIndex)
	{
		m_Vertices.push_back(from_block.m_Vertices[nVertexIndex]);
		return m_Vertices.size() - 1;
	}

	void BlockModel::CloneVertices(const BlockModel& from_block)
	{
		m_nFaceCount = from_block.m_nFaceCount;
		int nVertexCount = m_nFaceCount * 4;
		m_Vertices.resize(nVertexCount);
		if (m_nFaceCount>0)
			memcpy(GetVertices(), from_block.GetVerticesConst(), sizeof(BlockVertexCompressed)*nVertexCount);
	}
	
	std::vector<BlockVertexCompressed>& BlockModel::Vertices()
	{
		return m_Vertices;
	}

	// count the number of bits set in v with Brian Kernighan's way 
	// http://graphics.stanford.edu/~seander/bithacks.html
	inline int CountBits(uint32 v)
	{
		int count = 0;
		while (v != 0)
		{
			count++;
			v &= v - 1;
		}
		return count;
	}

	unsigned char BlockModel::CalculateCubeVertexAOShadowLevel(int nIndex, uint32 aoFlags)
	{
		uint32 nShadowValues = (aoFlags & (CubeAmbientMaskMap[nIndex]));
		if (nShadowValues > 0)
		{
			return CountBits(nShadowValues)*45;
		}
		return 0;
	}
	void BlockModel::SetVertexShadowFromAOFlags(int nIndex, int nCubeIndex, uint32 aoFlags)
	{
		unsigned char nShadowLevel = 0;
		if (aoFlags > 0 && (nShadowLevel = CalculateCubeVertexAOShadowLevel(nCubeIndex, aoFlags)) != 0)
		{
			SetVertexShadow(nIndex, nShadowLevel);
		}
	}

	void BlockModel::SetColor(DWORD color)
	{
		int nVertexCount = GetFaceCount() * 4;
		for (int i = 0; i < nVertexCount; ++i)
		{
			m_Vertices[i].SetBlockColor(color);
		}
	}

	void BlockModel::SetVertexColor(int nIndex, DWORD color)
	{
		m_Vertices[nIndex].SetBlockColor(color);
	}

	void BlockModel::RecalculateNormals()
	{
		for (int i = 0; i < m_nFaceCount; ++i)
		{
			RecalculateNormalsOfRectFace(i * 4);
		}
	}

	void RoundNormal(float& f)
	{
		if (f > 0.99f)
			f = 1;
		else if (f < -0.99f)
			f = -1;
		else if (f < 0.01f && f > -0.01f)
			f = 0;
	}

	Vector3 BlockModel::RecalculateNormalsOfRectFace(int startIdxOfFace)
	{
		int idx = startIdxOfFace + 0;
		Vector3 pt_0 = Vector3(m_Vertices[idx].position[0], m_Vertices[idx].position[1], m_Vertices[idx].position[2]);

		idx = startIdxOfFace + 1;
		Vector3 pt_1 = Vector3(m_Vertices[idx].position[0], m_Vertices[idx].position[1], m_Vertices[idx].position[2]);

		idx = startIdxOfFace + 2;
		Vector3 pt_2 = Vector3(m_Vertices[idx].position[0], m_Vertices[idx].position[1], m_Vertices[idx].position[2]);

		idx = startIdxOfFace + 3;
		Vector3 pt_3 = Vector3(m_Vertices[idx].position[0], m_Vertices[idx].position[1], m_Vertices[idx].position[2]);

		Vector3 dir0_1 = pt_0 - pt_1;
		Vector3 dir0_2 = pt_0 - pt_2;
		Vector3 dir0_3 = pt_0 - pt_3;

		Vector3 normal = Vector3(0, 0, 0);
		if (!dir0_1.positionEquals(normal) && !dir0_2.positionEquals(normal) && !dir0_1.positionEquals(dir0_2)) {
			normal = dir0_1.crossProduct(dir0_2);
			normal.normalise();
		}
		else if (!dir0_1.positionEquals(normal) && !dir0_3.positionEquals(normal) && !dir0_1.positionEquals(dir0_3)) {
			normal = dir0_1.crossProduct(dir0_3);
			normal.normalise();
		}
		else if (!dir0_2.positionEquals(normal) && !dir0_3.positionEquals(normal) && !dir0_2.positionEquals(dir0_3)) {
			normal = dir0_2.crossProduct(dir0_3);
			normal.normalise();
		}
		RoundNormal(normal.x);
		RoundNormal(normal.y);
		RoundNormal(normal.z);

		for (int i = 0; i < 4; i++) {
			int idx = startIdxOfFace + i;
			m_Vertices[idx].SetNormal(normal);
		}
		return normal;
	}

	uint8 setFaceShapeByXY(float x, float y) 
	{
		uint8 shape = 0;
		if (x == 0 && y == 0)
			shape |= 1;
		else if (x == 1 && y == 0)
			shape |= 2;
		else if (x == 1 && y == 1)
			shape |= 4;
		else if (x == 0 && y == 1)
			shape |= 8;
		return shape;
	}
	uint8 setFaceShapeBy4Corners(BlockVertexCompressed* pVertices, int axis ,float axisValue) {
		uint8 shape = 0;
		if(axis == 0) {
			if (pVertices[0].position[0] == axisValue)
				shape |= setFaceShapeByXY(pVertices[0].position[1], pVertices[0].position[2]);
			if (pVertices[1].position[0] == axisValue)
				shape |= setFaceShapeByXY(pVertices[1].position[1], pVertices[1].position[2]);
			if (pVertices[2].position[0] == axisValue)
				shape |= setFaceShapeByXY(pVertices[2].position[1], pVertices[2].position[2]);
			if (pVertices[3].position[0] == axisValue)
				shape |= setFaceShapeByXY(pVertices[3].position[1], pVertices[3].position[2]);
		}
		else if(axis == 1) {
			if (pVertices[0].position[1] == axisValue)
				shape |= setFaceShapeByXY(pVertices[0].position[0], pVertices[0].position[2]);
			if (pVertices[1].position[1] == axisValue)
				shape |= setFaceShapeByXY(pVertices[1].position[0], pVertices[1].position[2]);
			if (pVertices[2].position[1] == axisValue)
				shape |= setFaceShapeByXY(pVertices[2].position[0], pVertices[2].position[2]);
			if (pVertices[3].position[1] == axisValue)
				shape |= setFaceShapeByXY(pVertices[3].position[0], pVertices[3].position[2]);
		}
		else if(axis == 2) {
			if (pVertices[0].position[2] == axisValue)
				shape |= setFaceShapeByXY(pVertices[0].position[0], pVertices[0].position[1]);
			if (pVertices[1].position[2] == axisValue)
				shape |= setFaceShapeByXY(pVertices[1].position[0], pVertices[1].position[1]);
			if (pVertices[2].position[2] == axisValue)
				shape |= setFaceShapeByXY(pVertices[2].position[0], pVertices[2].position[1]);
			if (pVertices[3].position[2] == axisValue)
				shape |= setFaceShapeByXY(pVertices[3].position[0], pVertices[3].position[1]);
		}
		return shape;
	}

	const static struct {
		Vector3 normal;
		uint8 axis;
		float axisValue;

	} faceNormals[6] = {
		{Vector3(0, 1, 0), 1, 1},
		{Vector3(0, 0, -1), 2, 0},
		{Vector3(0, -1, 0), 1, 0},
		{Vector3(-1, 0, 0), 0, 0},
		{Vector3(1, 0, 0), 0, 1},
		{Vector3(0, 0, 1), 2, 1}
	};

	void BlockModel::RecalculateFaceShapeAndSortFaces()
	{
		auto swapFaces = [this](int fromIndex, int toIndex) {
			// swap the face order
			BlockVertexCompressed temp[4];
			memcpy(temp, &m_Vertices[fromIndex], sizeof(BlockVertexCompressed) * 4);
			memcpy(&m_Vertices[fromIndex], &m_Vertices[toIndex], sizeof(BlockVertexCompressed) * 4);
			memcpy(&m_Vertices[toIndex], temp, sizeof(BlockVertexCompressed) * 4);
		};

		// sort the faces by the normal direction so that the first 6 faces are in the order of default cube model face order.
		for(int faceIndex = 0; faceIndex < 6; ++faceIndex)
		{
			m_faceShape[faceIndex] = 0;
			if (faceIndex < m_nFaceCount)
			{
				const auto& vNormal = faceNormals[faceIndex].normal;
				for (int j = 0; j < m_nFaceCount; ++j)
				{
					if (vNormal == Vector3(m_Vertices[j * 4].normal))
					{
						if (j != faceIndex) {
							swapFaces(faceIndex * 4, j * 4);
						}
						m_faceShape[faceIndex] = setFaceShapeBy4Corners(&m_Vertices[faceIndex * 4], faceNormals[faceIndex].axis, faceNormals[faceIndex].axisValue);
						break;
					}
				}
			}
		}
	}

	void BlockModel::DumpToLog()
	{
		OUTPUT_LOG("BlockModel: %d faces\n", m_nFaceCount);
		for (int i = 0; i < m_nFaceCount; ++i)
		{
			OUTPUT_LOG("Face %d: shape: %d\n", i, m_faceShape[i]);
			for (int j = 0; j < 4; ++j)
			{
				OUTPUT_LOG("  pos %d: %f %f %f\n", j, m_Vertices[i * 4 + j].position[0], m_Vertices[i * 4 + j].position[1], m_Vertices[i * 4 + j].position[2]);
				OUTPUT_LOG("  norm %d: %f %f %f\n", j, m_Vertices[i * 4 + j].normal[0], m_Vertices[i * 4 + j].normal[1], m_Vertices[i * 4 + j].normal[2]);
			}
		}
	}
}


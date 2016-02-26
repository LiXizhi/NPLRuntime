//-----------------------------------------------------------------------------
// Class:	BlockCommon
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date: 2014
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Globals.h"
#include "ShapeAABB.h"

#include <math.h>
#include <algorithm>
#include "BlockWorld.h"
#include "BlockCommon.h"
#include "BlockConfig.h"

/** @def define this to use double precision to convert real world block pos to block index. */
#define USE_DOUBLE_BLOCK_CONVERT

namespace ParaEngine
{
	const Int16x3 BlockCommon::NeighborOfsTable[27] =
	{
		Int16x3(0,0,0),			//0	 rbp_center 
		Int16x3(1,0,0),			//1	 rbp_pX	
		Int16x3(-1,0,0),			//2	 rbp_nX	
		Int16x3(0,1,0),			//3	 rbp_pY	
		Int16x3(0,-1,0),			//4	 rbp_nY	
		Int16x3(0,0,1),			//5	 rbp_pZ	
		Int16x3(0,0,-1),			//6	 rbp_nz	
		Int16x3(1,1,0),			//7	 rbp_pXpY	
		Int16x3(1,-1,0),			//8	 rbp_pXnY	
		Int16x3(1,0,1),			//9	 rbp_pXpZ	
		Int16x3(1,0,-1),			//10 rbp_pXnZ	
		Int16x3(-1,1,0),			//11 rbp_nXpY	
		Int16x3(-1,-1,0),		//12 rbp_nXnY	
		Int16x3(-1,0,1),			//13 rbp_nXpZ	
		Int16x3(-1,0,-1),		//14 rbp_nXnZ	
		Int16x3(0,1,1),			//15 rbp_pYpZ	
		Int16x3(0,1,-1),			//16 rbp_pYnZ	
		Int16x3(0,-1,1),			//17 rbp_nYpZ	
		Int16x3(0,-1,-1),		//18 rbp_nYnZ	
		Int16x3(1,1,1),			//19 rbp_pXpYpZ 
		Int16x3(1,1,-1),			//20 rbp_pXpYnZ 
		Int16x3(1,-1,1),			//21 rbp_pXnYPz 
		Int16x3(1,-1,-1),		//22 rbp_pXnYnZ 
		Int16x3(-1,1,1),			//23 rbp_nXpYpZ 
		Int16x3(-1,1,-1),		//24 rbp_nXpYnZ 
		Int16x3(-1,-1,1),		//25 rbp_nXnYPz 
		Int16x3(-1,-1,-1),		//26 rbp_nXnYnZ 
	};
	
	const int32_t BlockCommon::RBP_SixNeighbors[] = {
		rbp_pY,
		rbp_nZ,
		rbp_nY,
		rbp_nX,
		rbp_pX,
		rbp_pZ,
	};

	int32 BlockCommon::g_xzDirectionsConst[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

	const int32_t BlockCommon::NeighborLightOrder[] =
	{
		//topLB
		rbp_pY,
		rbp_nXpY,
		rbp_pYnZ,
		rbp_nXpYnZ,
		//topLT
		rbp_pY,
		rbp_nXpY,
		rbp_pYpZ,
		rbp_nXpYpZ,
		//topRT
		rbp_pY,
		rbp_pXpY,
		rbp_pYpZ,
		rbp_pXpYpZ,
		//topRB
		rbp_pY,
		rbp_pXpY,
		rbp_pYnZ,
		rbp_pXpYnZ,

		//frontLB
		rbp_nZ,
		rbp_nXnZ,
		rbp_nYnZ,
		rbp_nXnYnZ,
		//frontLT
		rbp_nZ,
		rbp_nXnZ,
		rbp_pYnZ,
		rbp_nXpYnZ,
		//frontRT
		rbp_nZ,
		rbp_pXnZ,
		rbp_pYnZ,
		rbp_pXpYnZ,
		//frontRB
		rbp_nZ,
		rbp_pXnZ,
		rbp_nYnZ,
		rbp_pXnYnZ,

		//bottomLB
		rbp_nY,
		rbp_nYpZ,
		rbp_nXnY,
		rbp_nXnYPz,
		//bottomLT
		rbp_nY,
		rbp_nYnZ,
		rbp_nXnY,
		rbp_nXnYnZ,
		//bottomRT
		rbp_nY,
		rbp_nYnZ,
		rbp_pXnY,
		rbp_pXnYnZ,
		//bottomRB
		rbp_nY,
		rbp_nYpZ,
		rbp_pXnY,
		rbp_pXnYPz,

		//leftLB
		rbp_nX,
		rbp_nXpZ,
		rbp_nXnY,
		rbp_nXnYPz,
		//leftLT
		rbp_nX,
		rbp_nXpZ,
		rbp_nXpY,
		rbp_nXpYpZ,
		//leftRT
		rbp_nX,
		rbp_nXnZ,
		rbp_nXpY,
		rbp_nXpYnZ,
		//leftRB
		rbp_nX,
		rbp_nXnZ,
		rbp_nXnY,
		rbp_nXnYnZ,

		//rightLB
		rbp_pX,
		rbp_pXnZ,
		rbp_pXnY,
		rbp_pXnYnZ,
		//rightLT
		rbp_pX,
		rbp_pXnZ,
		rbp_pXpY,
		rbp_pXpYnZ,
		//rightRT
		rbp_pX,
		rbp_pXpZ,
		rbp_pXpY,
		rbp_pXpYpZ,
		//rightRB
		rbp_pX,
		rbp_pXpZ,
		rbp_pXnY,
		rbp_pXnYPz,

		//backLB
		rbp_pZ,
		rbp_nYpZ,
		rbp_pXpZ,
		rbp_pXnYPz,
		//backLT
		rbp_pZ,
		rbp_pYpZ,
		rbp_pXpZ,
		rbp_pXpYpZ,
		//backRT
		rbp_pZ,
		rbp_pYpZ,
		rbp_nXpZ,
		rbp_nXpYpZ,
		//backRB
		rbp_pZ,
		rbp_nYpZ,
		rbp_nXpZ,
		rbp_nXnYPz,
	};
	void BlockCommon::GetBlockPosBySide( int& x, int& y, int& z, int nSide /*= 4*/ )
	{
		switch (nSide)
		{
		case 0:
			{
				x += 1;
				break;
			}
		case 1:
			{
				x -= 1;
				break;
			}
		case 2:
			{
				z += 1;
				break;
			}
		case 3:
			{
				z -= 1;
				break;
			}
		case 4:
			{
				y += 1;
				break;
			}
		case 5: 
			{
				y -= 1;
				break;
			}
		}
	}

	float BlockCommon::ConvertToRealY(uint16 y)
	{
		return (float)(y*BlockConfig::g_dBlockSize + CBlockWorld::GetVerticalOffset());
	}

	Vector3 BlockCommon::ConvertToRealPosition( uint16 x, uint16 y, uint16 z, int nSide /*= -1*/  )
	{
#ifdef USE_DOUBLE_BLOCK_CONVERT
		double real_x = x;
		double real_y = y;
		double real_z = z;
		const double blocksize = BlockConfig::g_dBlockSize;

		switch (nSide)
		{
		case -1:
			real_x += 0.5;
			real_y += 0.5;
			real_z += 0.5;
			break;
		case 0:
			real_x += 1;
			real_y += 0.5;
			real_z += 0.5;
			break;
		case 1:
			real_y += 0.5;
			real_z += 0.5;
			break;
		case 2:
			real_x += 0.5;
			real_y += 0.5;
			real_z += 1;
			break;
		case 3:
			real_x += 0.5;
			real_y += 0.5;
			break;
		case 4:
			real_x += 0.5;
			real_y += 1;
			real_z += 0.5;
			break;
		case 5:
			real_x += 0.5;
			real_z += 0.5;
			break;
		default: 
			break;
		}
		return Vector3((float)(real_x*blocksize), (float)(real_y*blocksize + CBlockWorld::GetVerticalOffset()), (float)(real_z*blocksize));
#else
		double real_x, real_y, real_z;

		const double blocksize = BlockConfig::g_blockSize;
		const double region_width = BlockConfig::g_regionSize;
		const double offset_y = CBlockWorld::GetVerticalOffset();

		uint16 region_x = (uint16)(x >> 9);
		uint16 region_z = (uint16)(z >> 9);
		uint16 x_orgin = region_x << 9;
		uint16 z_orgin = region_z << 9;

		// local index
		
		switch (nSide)
		{
		case -1:
			{
				double bx = (x - x_orgin + 0.5) * blocksize;
				double bz = (z - z_orgin + 0.5) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = (y + 0.5f)*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		case 0:
			{
				double bx = (x - x_orgin + 1) * blocksize;
				double bz = (z - z_orgin + 0.5) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = (y + 0.5f)*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		case 1:
			{
				double bx = (x - x_orgin) * blocksize;
				double bz = (z - z_orgin + 0.5) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = (y + 0.5f)*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		case 2:
			{
				double bx = (x - x_orgin + 0.5) * blocksize;
				double bz = (z - z_orgin + 1) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = (y + 0.5f)*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		case 3:
			{
				double bx = (x - x_orgin + 0.5) * blocksize;
				double bz = (z - z_orgin) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = (y + 0.5f)*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		case 4:
			{
				double bx = (x - x_orgin + 0.5) * blocksize;
				double bz = (z - z_orgin + 0.5) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = (y + 1)*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		case 5:
			{
				double bx = (x - x_orgin + 0.5) * blocksize;
				double bz = (z - z_orgin + 0.5) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = y*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		default: 
			{
				double bx = (x - x_orgin) * blocksize;
				double bz = (z - z_orgin) * blocksize;
				real_x = region_x * region_width + bx;
				real_y = y*blocksize + offset_y;
				real_z = region_z * region_width + bz;
				break;
			}
		}

		return Vector3((float)real_x, (float)real_y, (float)real_z);
#endif
	}

	void BlockCommon::ConvertToBlockIndex( float x,float y,float z, uint16& idx_x, uint16& idx_y, uint16& idx_z )
	{
#ifdef USE_DOUBLE_BLOCK_CONVERT
		const double blocksize_inverse = BlockConfig::g_dBlockSizeInverse;
		idx_x = (uint16)(x*blocksize_inverse);
		y -= CBlockWorld::GetVerticalOffset();
		idx_y = (uint16)((y > 0) ? (y*blocksize_inverse) : 0);
		idx_z = (uint16)(z*blocksize_inverse);
#else
		const double blocksize = BlockConfig::g_blockSize;
		const double region_width = BlockConfig::g_regionSize;
		double offset_y = CBlockWorld::GetVerticalOffset();

		idx_x = uint16(x / region_width);
		idx_z = uint16(z / region_width);
		double x_orgin = idx_x * region_width;
		double z_orgin = idx_z * region_width;
		// local index
		uint16 bx = uint16((x - x_orgin)/blocksize);
		uint16 bz = uint16((z - z_orgin)/blocksize);

		idx_x = (idx_x<<9) + bx;

		double ofsY = y - offset_y;
		if(ofsY < 0)ofsY = 0;
		idx_y = uint16(ofsY/blocksize);

		idx_z = (idx_z<<9) + bz;
#endif
	}

	Vector3 BlockCommon::GetBlockCenter( float x, float y, float z )
	{
		uint16 idx_x, idx_y, idx_z;
		ConvertToBlockIndex(x,y,z, idx_x, idx_y, idx_z);
		return ConvertToRealPosition(idx_x, idx_y, idx_z);
	}

	Vector3 BlockCommon::GetNormalBySide( int nSide )
	{
		switch(nSide)
		{
		case 0:
			return Vector3(1.f, 0, 0);
		case 1:
			return Vector3(-1.f, 0, 0);
		case 2:
			return Vector3(0, 0, 1.f);
		case 3:
			return Vector3(0, 0, -1.f);
		case 4:
			return Vector3(0, 1.f, 0);
		case 5:
			return Vector3(0, -1.f, 0);
		}
		return Vector3(0, 0, 0);
	}

	bool BlockCommon::IsOppositeSide( int nSide1, int nSide2 )
	{
		if(nSide1 == nSide2)
			return false;
		else if(nSide1 > nSide2)
		{
			int tmp;
			tmp = nSide1;
			nSide1 = nSide2;
			nSide2 = tmp;
		}
		return ((nSide1+1)==nSide2 && (nSide1%2 == 0));
	}

	bool BlockCommon::IsCornerSide( int nSide1, int nSide2 )
	{
		if(nSide1 == nSide2)
			return false;
		else if(nSide1 > nSide2)
		{
			int tmp;
			tmp = nSide1;
			nSide1 = nSide2;
			nSide2 = tmp;
		}
		return !((nSide1+1)==nSide2 && (nSide1%2 == 0));
	}

	void BlockCommon::ConstrainPos( Vector3& vMovePos,const Vector3& vMinPos, const Vector3& vMaxPos )
	{
		vMovePos.x = max(vMovePos.x, vMinPos.x);
		vMovePos.y = max(vMovePos.y, vMinPos.y);
		vMovePos.z = max(vMovePos.z, vMinPos.z);
		vMovePos.x = min(vMovePos.x, vMaxPos.x);
		vMovePos.y = min(vMovePos.y, vMaxPos.y);
		vMovePos.z = min(vMovePos.z, vMaxPos.z);
	}

	void BlockCommon::ConstrainPos(DVector3& vMovePos, const Vector3& vMinPos, const Vector3& vMaxPos)
	{
		vMovePos.x = max(vMovePos.x, (double)vMinPos.x);
		vMovePos.y = max(vMovePos.y, (double)vMinPos.y);
		vMovePos.z = max(vMovePos.z, (double)vMinPos.z);
		vMovePos.x = min(vMovePos.x, (double)vMaxPos.x);
		vMovePos.y = min(vMovePos.y, (double)vMaxPos.y);
		vMovePos.z = min(vMovePos.z, (double)vMaxPos.z);
	}

	void BlockCommon::ConstrainPosOuter( Vector3& vMovePos,const Vector3& vMinPos, const Vector3& vMaxPos, float fBoarderDist )
	{
		if( (vMinPos.x <= vMovePos.x && vMovePos.x <=vMaxPos.x) && 
			(vMinPos.y <= vMovePos.y && vMovePos.y <=vMaxPos.y) && 
			(vMinPos.z <= vMovePos.z && vMovePos.z <=vMaxPos.z) )
		{
			// vMinPos is inside the AABB
			bool bHasMoved;
			if((vMovePos.x - vMinPos.x) <=  fBoarderDist)
			{
				vMovePos.x = vMinPos.x;
				bHasMoved = true;
			}
			else if((vMaxPos.x-vMovePos.x) <= fBoarderDist)
			{
				vMovePos.x = vMaxPos.x;
				bHasMoved = true;
			}

			if((vMovePos.y - vMinPos.y) <=  fBoarderDist)
			{
				vMovePos.y = vMinPos.y;
				bHasMoved = true;
			}
			else if((vMaxPos.y-vMovePos.y) <= fBoarderDist)
			{
				vMovePos.y = vMaxPos.y;
				bHasMoved = true;
			}

			if((vMovePos.z - vMinPos.z) <=  fBoarderDist)
			{
				vMovePos.z = vMinPos.z;
				bHasMoved = true;
			}
			else if((vMaxPos.z-vMovePos.z) <= fBoarderDist)
			{
				vMovePos.z = vMaxPos.z;
				bHasMoved = true;
			}

			if( !bHasMoved)
			{
				vMovePos.x = ( (vMovePos.x - vMinPos.x) < (vMaxPos.x-vMovePos.x)) ? vMinPos.x : vMaxPos.x;
				vMovePos.y = ( (vMovePos.y - vMinPos.y) < (vMaxPos.y-vMovePos.y)) ? vMinPos.y : vMaxPos.y;
				vMovePos.z = ( (vMovePos.z - vMinPos.z) < (vMaxPos.z-vMovePos.z)) ? vMinPos.z : vMaxPos.z;
			}
		}
	}

	std::map<int64_t, Uint16x3>& CBlockSelectGroup::GetBlocks()
	{
		return m_blocks;
	}

	CBlockSelectGroup::CBlockSelectGroup() :m_fScaling(1.f), m_bEnableBling(false), m_color(0, 0, 0, 0), m_bOnlyRenderClickableArea(false), m_bWireFrame(false)
	{

	}

}
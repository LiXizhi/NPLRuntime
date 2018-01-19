//-----------------------------------------------------------------------------
// Class:	Server side light calculation
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.7
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockWorld.h"
#include "BlockLightGridServer.h"

namespace ParaEngine
{
	CBlockLightGridServer::CBlockLightGridServer(CBlockWorld* pBlockWorld)
		: CBlockLightGridBase(pBlockWorld), m_max_cells_per_frame(500), m_max_cells_left_per_frame(5000)
	{
	}

	CBlockLightGridServer::~CBlockLightGridServer()
	{
	}

	void CBlockLightGridServer::OnEnterWorld()
	{
	}

	void CBlockLightGridServer::OnLeaveWorld()
	{
	}

	void CBlockLightGridServer::OnWorldMove(uint16_t centerChunkX, uint16_t centerChunkZ)
	{
	}

	bool CBlockLightGridServer::GetBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nSize, int nLightType)
	{
		return false;
	}

	void CBlockLightGridServer::SetLightDirty(Uint16x3& blockId_ws, bool isSunLight)
	{
		
	}


	void CBlockLightGridServer::UpdateLighting()
	{
		// this function is called on each pre-render frame move to update light values if necessary. 
	}

	void CBlockLightGridServer::EmitLight(const Uint16x3& blockId_ws, uint8_t brightness, bool isSunLight)
	{
	}

	void CBlockLightGridServer::RefreshLight(const Uint16x3& blockId_ws)
	{
		RefreshLight(blockId_ws, true);
		RefreshLight(blockId_ws, false);
	}

	// call this function when the block's light value is no longer valid and need to recalculated. 
	// the old light value of the current cell is used to decide which blocks needs to be recalculated. 
	void CBlockLightGridServer::RefreshLight(const Uint16x3& blockId_ws, bool isSunLight)
	{
	}

	void CBlockLightGridServer::EmitSunLight(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, bool bInitialSet)
	{
		
	}

	void CBlockLightGridServer::AddDirtyColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
		
	}

	int CBlockLightGridServer::GetDirtyColumnCount()
	{
		return 0;
	}

}




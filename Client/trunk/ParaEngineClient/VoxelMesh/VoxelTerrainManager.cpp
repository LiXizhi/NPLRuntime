//-----------------------------------------------------------------------------
// Class:	VoxelTerrainManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "IsoSurfaceBuilder.h"
#include "VoxelTerrainManager.h"

using namespace ParaEngine;

VoxelTerrainManager::VoxelTerrainManager(void)
:m_pIsoSurfaceBuilder(NULL)
{
	/** how many cells does a grid have in one direction. it is usually 32*32*32. Some may like 64*64*64, which will cost quite a bit memory for data keeping.*/
	m_DefaultGridSize = 32;
	/** how to scale the grid to mesh the height map terrain grid.*/
	//m_DefaultGridScale = 2.9296875f;
	m_DefaultGridScale = 1.f;
}

VoxelTerrainManager::~VoxelTerrainManager(void)
{
	SAFE_DELETE(m_pIsoSurfaceBuilder);
}

IsoSurfaceBuilder* VoxelTerrainManager::GetIsoSurfaceBuilder()
{
	if(m_pDataGrid == 0)
	{
		m_pDataGrid = new DataGrid();
		m_pDataGrid->initialize(m_DefaultGridSize, m_DefaultGridSize, m_DefaultGridSize, m_DefaultGridScale, DataGrid::HAS_GRADIENT/* | DataGrid::HAS_COLOURS*/);
	}

	if(m_pIsoSurfaceBuilder == 0)
	{
		m_pIsoSurfaceBuilder  = new IsoSurfaceBuilder();
		m_pIsoSurfaceBuilder->initialize(m_pDataGrid.get(), IsoSurfaceBuilder::GEN_NORMALS| IsoSurfaceBuilder::GEN_TEX_COORDS);
		m_pIsoSurfaceBuilder->setFlipNormals(false);	
	}
	return m_pIsoSurfaceBuilder;
}
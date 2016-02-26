#pragma once
#include "AssetManager.h"
#include "DataGrid.h"
#include "VoxelTerrainEntity.h"

namespace ParaEngine
{
	class IsoSurfaceBuilder;

	/**
	* managing all voxel terrain entities used by any scenes
	*/
	class VoxelTerrainManager :	public AssetManager <VoxelTerrainEntity>
	{
	public:
		VoxelTerrainManager(void);
		~VoxelTerrainManager(void);

		/**
		* get the default IsoSurface builder. Create if does not exist. 
		*/
		IsoSurfaceBuilder* GetIsoSurfaceBuilder();

	private:
		/**
		* IsoSurface building for the current voxel mesh entity. This manager only allows editing or initializing one voxel mesh at a time. 
		*/
		IsoSurfaceBuilder* m_pIsoSurfaceBuilder;

		/**
		* A copy of datagrid for the current voxel mesh entity. This manager only allows editing or initializing one voxel mesh at a time. 
		*/
		DataGridPtr m_pDataGrid;

		/** how many cells does a grid have in one direction. it is usually 32*32*32. Some may like 64*64*64, which will cost quite a bit memory for data keeping.*/
		int m_DefaultGridSize;

		/** how to scale the grid to mesh the height map terrain grid.*/
		float m_DefaultGridScale;
	};

}

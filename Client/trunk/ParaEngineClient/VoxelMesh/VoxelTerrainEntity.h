#pragma once
#include "assetentity.h"
#include "DataGrid.h"

namespace ParaEngine
{
	class IsoSurfaceRenderable;
	/**
	* a voxel terrain entity uses a 3d grid to present a terrain mesh, such can have caves and overhangs on traditional height map terrains. 
	* Internally, I used matching cube algorithm to build triangles from a 3d grid. Please note that a voxel terrain needs to be perfectly aligned 
	* with the height map terrain and use same base texture at intersection points. There are some tricks to make this work. 
	* @remark: VoxelTerrainEntity does not use LOD, and it is basically an expensive highpoly mesh prebuilt from a 3d grid usually of size 32*32*32
	*/
	class VoxelTerrainEntity :	public AssetEntity
	{
	public:
		VoxelTerrainEntity(void);
		virtual ~VoxelTerrainEntity(void);

	public:

		/** load the voxel entity from file. the text file contains the grid data in the following format. 
		-- how many sells in the grid in x,y,z direction
		cellx = 32
		celly = 32
		cellz = 32
		-- index and value pairs. If an index is not defined, it is empty. 
		-- number of value pairs followed by index, value pairs on each line. 
		ValueCount = 32
		index,value;
		...
		index,value;
		*/
		void Init(const char* filename);

		/** Get the Iso surface object for rendering. The isosurface is not created until this function is called the first time. 
		*/
		IsoSurfaceRenderable* GetIsoSurface();

		/**
		* this is only created when a voxel entity is being manually edited by the user. In other cases, like reading from file, the data grid is NULL; 
		* the global grid in VoxelTerrainManager is used to save memory. 
		*/
		DataGridPtr m_pDataGrid;
	};
}


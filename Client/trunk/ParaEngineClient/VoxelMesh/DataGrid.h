#pragma once

#include "ShapeAABB.h"
#include "AssetEntity.h"

namespace ParaEngine
{
	class MetaWorldFragment;

	/**
	* Class providing a 3d grid of data values and methods for accessing and modifying it.
	* original code is partially from 2007 Martin Enge, based on code from DWORD, released into public domain. martin.enge@gmail.com
	*/
	struct DataGrid : public AssetEntity
	{
	public:
		virtual AssetType GetType(){return datagrid;};

		/// Flags describing what data is stored in the data grid.
		enum GridFlags
		{
			/// The data grid stores gradient vectors.
			HAS_GRADIENT = 0x01,
			/// The data grid stores colour values.
			HAS_COLOURS = 0x02,
			/// The data grid stores closest world fragment.
			HAS_WORLD_FRAGMENTS = 0x04
		};

		/// Constructor
		DataGrid();
		/// Virtual destructor
		virtual ~DataGrid();

		/** Initializes the data grid.
			@remarks
				This function should only be called once. The actual data in the grid is left uninitialized.
			@param numCellsX The number of grid cells along the x axis of the grid.
			@param numCellsY The number of grid cells along the y axis of the grid.
			@param numCellsZ The number of grid cells along the z axis of the grid.
			@param gridScale The scale of grid cells; this influences the position of grid vertices.
			@param flags Flags describing what data is stored in the data grid (see DataGrid::GridFlags). */
		void initialize(int numCellsX, int numCellsY, int numCellsZ, float gridScale, int flags);
		/// Returns the number of grid cells along the x axis.
		int getNumCellsX() const {return mNumCellsX; }
		/// Returns the number of grid cells along the y axis.
		int getNumCellsY() const {return mNumCellsY; }
		/// Returns the number of grid cells along the z axis.
		int getNumCellsZ() const {return mNumCellsZ; }
		/// Returns the grid scale (i.e. the distance along the axes between grid points).
		float getGridScale() const {return mGridScale; }
		/// Returns the grids position in space.
		Vector3 getPosition() {return mPosition;} const
		/// Sets the grids position in space.
		inline void setPosition(const Vector3 &pos)
		{
			mPosition = pos;
			float minX, minY, minZ, maxX, maxY, maxZ;
			minX = pos.x - mNumCellsX * mGridScale * 0.5f;
			minY = pos.y - mNumCellsY * mGridScale * 0.5f;
			minZ = pos.z - mNumCellsZ * mGridScale * 0.5f;
			maxX = pos.x + mNumCellsX * mGridScale * 0.5f;
			maxY = pos.y + mNumCellsY * mGridScale * 0.5f;
			maxZ = pos.z + mNumCellsZ * mGridScale * 0.5f;
			
			mBoundingBox.SetMinMax(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ));
		}

		/// Returns true if the grid stores gradient vectors.
		bool hasGradient() const {return (mGridFlags & HAS_GRADIENT) != 0; }
		/// Returns true if the grid stores colour values.
		bool hasColours() const {return (mGridFlags & HAS_COLOURS) != 0; }
		/// Returns true if the grid stores closest world fragment.
		bool hasMetaWorldFragments() const {return (mGridFlags & HAS_WORLD_FRAGMENTS) != 0; }

		/// Returns a pointer to the array of grid values.
		float* getValues() {return mValues; }
		/// Returns a pointer to the (const) array of grid vertices.
		const Vector3* getVertices() const {return mVertices; }
		/** Returns a pointer to the array of gradient vectors.
			@remarks
				The returned pointer is only valid if HAS_GRADIENT is set in DataGrid::mGridFlags. */
		Vector3* getGradient() {return mGradient; }
		/** Returns a pointer to the array of colour values.
			@remarks
				The returned pointer is only valid if HAS_COLOURS is set in DataGrid::mGridFlags. */
		Color* getColours() {return mColours; }
		/** Returns a pointer to the array of closest world fragments.
			@remarks
				The returned pointer is only valid if HAS_WORLD_FRAGMENT is set in DataGrid::mGridFlags. */
		std::pair<float, MetaWorldFragment*>* getMetaWorldFragments() {return mMetaWorldFragments; }

		/// Returns the bounding box of the grid.
		const CShapeAABB& getBoundingBox() const {return mBoundingBox; }
		const CShapeAABB& getBoxSize() const {return mBoxSize;}
		/// Returns the index of the specified grid point.
		int getGridIndex(int x, int y, int z) const {return z*(mNumCellsX + 1)*(mNumCellsY + 1) + y*(mNumCellsX + 1) + x; }
		/** Maps an axis aligned box to the grid points inside it.
			@remarks
				This function modifies the values of x0, y0, z0, x1, y1, and z1.
				(x0, y0, z0) will be the 'minimum' grid point <i>inside</i> the axis aligned box, and
				(x1, y1, z1) will be the 'maximum' grid point <i>inside</i> the axis aligned box.
				I.e. the points (x, y, z) for which (x0, y0, z0) <= (x, y, z) <= (x1, y1, z1) lie inside the axis aligned box.
			@returns
				false if the box is completely outside the grid, otherwise it returns true.
			@note
				If the function returns true, it is safe to use the values of (x0, y0, z0) and (x1, y1, z1) in a call to getGridIndex().
			@warning
				If the function returns false, the contents of x0, y0, z0, x1, y1, and z1 are undefined. */
		bool mapAABB(const CShapeAABB& aabb, int &x0, int &y0, int &z0, int &x1, int &y1, int &z1) const;
		/// Clears the data grid.
		void clear();

	protected:
		/// The number of grid cells along the x axis of the grid.
		int mNumCellsX;
		/// The number of grid cells along the y axis of the grid.
		int mNumCellsY;
		/// The number of grid cells along the z axis of the grid.
		int mNumCellsZ;
		/// Total number of grid points.
		int mNumGridPoints;
		/// The scale of grid cells; this influences the position of grid vertices.
		float mGridScale;
		/// Position of this Grid.
		Vector3 mPosition;
		/// Flags describing what data is stored in the data grid (see DataGrid::GridFlags).
		int mGridFlags;
		/// Data grid values.
		float* mValues;
		/// Vertex positions of the grid points.
		Vector3* mVertices;
		/** Gradient vectors of the grid.
			@remarks
				This array is only allocated if HAS_GRADIENT is set in DataGrid::mGridFlags. */
		Vector3* mGradient;
		/** Colour values of the grid.
			@remarks
				This array is only allocated if HAS_COLOURS is set in DataGrid::mGridFlags. */
		Color* mColours;
		/** Closest World Fragments of the grid.
			@remarks
				This array is only allocated if HAS_WORLD_FRAGMENTS is set in DataGrid::mGridFlags. */
		std::pair<float, MetaWorldFragment*>* mMetaWorldFragments;

		/// Bounding box of the grid.
		CShapeAABB mBoundingBox, mBoxSize;

		/** Initializes the position of grid points and the bounding box.
			@remarks
				In the default implementation, the grid points form a regular grid centered around (0, 0, 0).
				The distance along the axes between grid points is determined by mGridScale.
			@par
				This function is responsible for initializing mBoundingBox to fit around all grid points. */
		virtual void initializeVertices();

		void * lastHostObject;
	};

	/// Reference-counted shared pointer to a DataGrid.
	typedef asset_ptr<DataGrid> DataGridPtr;
}

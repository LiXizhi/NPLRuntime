#pragma once

#include "DataGrid.h"
#include <vector>

namespace ParaEngine
{
	/**
	* building triangles from 3d grid data. It uses the matching cubes algorithm. 
	*/
	class IsoSurfaceBuilder
	{
	public:
		friend class IsoSurfaceRenderable;
		/// Flags describing what data is generated for rendering the iso surface.
		enum SurfaceFlags
		{
			/// Generate vertex normals by interpolating the gradient stored in the data grid.
			GEN_NORMALS = 0x01,
			/// Generate vertex colours by interpolating the colours stored in the data grid.
			GEN_VERTEX_COLOURS = 0x02,
			/// Generate texture coordinates.
			GEN_TEX_COORDS = 0x04,
		};

		enum NormalType
		{
			/// Normals are calculated as a weighted average of face normals.
			NORMAL_WEIGHTED_AVERAGE,
			/// Normals are calculated as an average of face normals.
			NORMAL_AVERAGE,
			/// Normals are calculated by interpolating the gradient in the data grid.
			NORMAL_GRADIENT
		};

		/// Constructor
		IsoSurfaceBuilder();
		/// Virtual Destructor
		virtual ~IsoSurfaceBuilder();
		/** Initializes the iso surface.
			@remarks
				This function should only be called once.
			@param dataGridPtr Pointer to the data grid to use for iso surface generation.
			@param flags Flags describing what data is generated for rendering the iso surface (see IsoSurface::SurfaceFlags). */
		virtual void initialize(DataGrid* dg, int flags);
		/// Rebuilds the iso surface, and updates the IsoSurfaceRenderable.
		void update(IsoSurfaceRenderable * isr);
		/// Returns the pointer to the data grid.
		DataGrid* getDataGrid() {return mDataGrid.get(); }
		/// Returns the iso value of the surface.
		float getIsoValue() const {return mIsoValue; }
		/// Sets the iso value of the surface.
		void setIsoValue(float isoValue) {mIsoValue = isoValue; }
		/// Returns whether normals are flipped.
		bool getFlipNormals() const {return mFlipNormals; }
		/** Sets whether to flip normals.
			@remarks
				When flip normals is false (the default), the outside of the surface is where the values of the data
				grid are lower than the iso value. */
		void setFlipNormals(bool flipNormals) {mFlipNormals = flipNormals; }
		/// Gets the method used for normal generation.
		NormalType getNormalType() const {return mNormalType; }
		/// Sets the method used for normal generation.
		void setNormalType(NormalType normalType) {mNormalType = normalType; }

		/// The number of iso vertices, calculated on first call of getNumIsoVertices().
		int mNumIsoVertices;	
		/// Array with offsets to the different iso vertex groups.
		int isoVertexGroupOffsets[3];

		/// Returns the total number of iso vertices to be allocated.
		virtual int getNumIsoVertices();
		/// Creates and initializes the iso vertex index arrays of all grid cells.
		virtual void createGridCellIsoVertices();
		/// Builds the iso surface by looping through all grid cells generating triangles.
		virtual void buildIsoSurface();

	protected:

		/// Definition of a triangle in an iso surface.
		struct IsoTriangle
		{
			/// Iso vertex indices defining the triangle.
			int vertices[3];
		};

		/// Grid cell.
		struct GridCell
		{
			GridCell():isoVertices(NULL){};
			/** Indices of the eight corners of the cell in the data grid arrays.
			* <PRE>
			*       4---------5
			*      /.        /|
			*     / .       / |
			*    7---------6  |
			*    |  .      |  |
			*    |  0 . . .|. 1
			*    | ,       | /
			*    |,        |/
			*    3---------2 </PRE>
			*/
			int cornerIndices[8];
			/// Iso vertex indices of this grid cell.
			int* isoVertices;
		};

		typedef std::vector<int> IsoVertexVector;
		typedef std::vector<IsoTriangle> IsoTriangleVector;

		/// Reference-counted shared pointer to the data grid associated with this iso surface.
		DataGridPtr mDataGrid;
		/// Flags describing what data is generated for rendering the iso surface (see IsoSurface::SurfaceFlags).
		int mSurfaceFlags;
		/// Iso value of the surface, the default is 1.0.
		float mIsoValue;
		/// Flip normals of the surface, the default is false.
		bool mFlipNormals;
		/// The method used for normal generation.
		NormalType mNormalType;
		/** Hardware vertex buffer indices for all iso vertices.
			@remarks
				A value of ~0 means that the iso vertex is not used. During iso surface generation all
				indices are reset to this value. On the first use of an iso vertex, its parameters are
				calculated, and it is assigned the next index in the hardware vertex buffer. */
		int* mIsoVertexIndices;
		/** Positions of all iso vertices.
			@remarks
				Positions are valid only for used iso vertices. */
		Vector3* mIsoVertexPositions;
		/** Normals for all iso vertices.
			@remarks
				This array is only allocated if GEN_NORMALS is set in IsoSurface::mSurfaceFlags,
				and normals are valid only for used iso vertices. */
		Vector3* mIsoVertexNormals;
		/** Vertex colours for all iso vertices.
			@remarks
				This array is only allocated if GEN_VERTEX_COLOURS is set in IsoSurface::mSurfaceFlags,
				and colours are valid only for used iso vertices. */
		Color* mIsoVertexColours;
		/** Texture coordinates for all iso vertices.
			@remarks
				This array is only allocated if GEN_TEX_COORDS is set in IsoSurface::mSurfaceFlags,
				and texture coordinates are valid only for used iso vertices. */
		Vector2* mIsoVertexTexCoords;
		/// Array of grid cells.
		GridCell* mGridCells;
		/** Vector to which the indices of all used iso vertices are added.
			@remarks
				The iso vertex indices in this vector are iterated when filling the hardware vertex buffer. */
		IsoVertexVector mIsoVertices;
		/** Vector to which all generated iso triangles are added.
			@remarks
				This vector is iterated when filling the hardware index buffer. */
		IsoTriangleVector mIsoTriangles;

		/// ...
		static const int msEdgeTable[256];
		/// ...
		static const int msTriangleTable[256][16];
	//#include "IsoSurfaceBuilderTables.h"

		/** Creates the iso vertex arrays.
			@remarks
				The function calls the abstract function getNumIsoVertices() to get the length
				of the arrays to be created. Arrays are created according to the flags specified in
				IsoSurface::mSurfaceFlags. */
		void createIsoVertices();
		/** Creates and initializes the grid cells.
			@remarks
				This function initializes the corner index arrays of the grid cells, and calls
				the abstract function createGridCellIsoVertices() to create and initialize iso
				vertex index arrays of all grid cells. */
		void createGridCells();
		/// Destroys the grid cells, including their iso vertex index arrays.
		void destroyGridCells();
		/** Calculates properties of the iso vertex.
			@remarks
				If the properties of the iso vertex has already been calculated, the function
				returns immediately. Otherwise the iso vertex is initialized by calculating
				the necessary properties. Then it is assigned the next index in the hardware
				vertex buffer.
			@param isoVertex Index of the iso vertex to use.
			@param corner0 Index of the first data grid value associated with the iso vertex.
			@param corner1 Index of the second data grid value associated with the iso vertex.
			@returns
				The index passed in the isoVertex parameter. */
		int useIsoVertex(int isoVertex, int corner0, int corner1);
		/// ...
		void addIsoTriangle(const IsoTriangle& isoTriangle);
	};
}


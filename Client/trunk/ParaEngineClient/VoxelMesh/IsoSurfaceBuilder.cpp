//-----------------------------------------------------------------------------
// Class:	DataGrid
// Authors:	Ported by LiXizhi, original code by Copyright (c) 2007 Martin Enge. Based on code from DWORD, released into public domain. martin.enge@gmail.com
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DataGrid.h"
#include "IsoSurfaceRenderable.h"
#include "IsoSurfaceBuilder.h"
#include "TextureEntity.h"
#include "IsoSurfaceBuilderTables.h"
#include "memdebug.h"

using namespace ParaEngine;


IsoSurfaceBuilder::IsoSurfaceBuilder()
  : mIsoVertexIndices(0), mIsoVertexPositions(0), mIsoVertexNormals(0),
	mIsoVertexColours(0), mIsoVertexTexCoords(0), mNumIsoVertices(0)//, mSurfaceFlags(0)
{
}

IsoSurfaceBuilder::~IsoSurfaceBuilder()
{
	SAFE_DELETE_ARRAY(mIsoVertexIndices);
	SAFE_DELETE_ARRAY(mIsoVertexIndices);
	SAFE_DELETE_ARRAY(mIsoVertexPositions);
	SAFE_DELETE_ARRAY(mIsoVertexNormals);
	SAFE_DELETE_ARRAY(mIsoVertexColours);
	SAFE_DELETE_ARRAY(mIsoVertexTexCoords);

	destroyGridCells();
}

void IsoSurfaceBuilder::initialize(DataGrid* dataGridPtr, int flags)
{
	// Initialize members
	mDataGrid = dataGridPtr;
	mSurfaceFlags = flags;
	
	mIsoValue = 0.1f; // 0.2 used to be 0.2, should be slightly over 0.f

	mFlipNormals = false;
	mNormalType = NORMAL_AVERAGE;

	// Mask out surface options not supported by the supplied data grid.
	// TODO: Inform user of unsupported options?
	if (!mDataGrid->hasGradient())
		mSurfaceFlags &= ~GEN_NORMALS;

	if (!mDataGrid->hasColours())
		mSurfaceFlags &= ~GEN_VERTEX_COLOURS;


	createIsoVertices();
	createGridCells();
}

void IsoSurfaceBuilder::update(IsoSurfaceRenderable *surf)
{
	int count = getNumIsoVertices();

	// Clear vertex and triangle vectors before building new iso surface
	mIsoVertices.clear();
	mIsoTriangles.clear();

	// Mark all iso vertices as not being used
	for (int i = 0; i < count; ++i)
		mIsoVertexIndices[i] = ~0;

	// Build the iso surface
	buildIsoSurface();

	// Update the render operation
	surf->fillHardwareBuffers(this);
}

int IsoSurfaceBuilder::getNumIsoVertices()
{
	if (!mNumIsoVertices)
	{
		int x = mDataGrid->getNumCellsX();
		int y = mDataGrid->getNumCellsY();
		int z = mDataGrid->getNumCellsZ();

		// Initialize iso vertex group offsets
		isoVertexGroupOffsets[0] = 0;
		isoVertexGroupOffsets[1] = isoVertexGroupOffsets[0] + x*(y+1)*(z+1);
		isoVertexGroupOffsets[2] = isoVertexGroupOffsets[1] + (x+1)*y*(z+1);

		// Store the total number of iso vertices
		mNumIsoVertices = isoVertexGroupOffsets[2] + (x+1)*(y+1)*z;
	}

	return mNumIsoVertices;
}

void IsoSurfaceBuilder::createIsoVertices()
{
	int size = getNumIsoVertices();

	// Create mandatory hardware vertex buffer index and position arrays
	mIsoVertexIndices = new int[size];
	mIsoVertexPositions = new Vector3[size];

	if (mSurfaceFlags & GEN_NORMALS)
	{
		// Create optional normal array
		mIsoVertexNormals = new Vector3[size];
	}

	if (mSurfaceFlags & GEN_VERTEX_COLOURS)
	{
		// Create optional diffuse colour array 
		mIsoVertexColours = new Color[size];
	}

	if (mSurfaceFlags & GEN_TEX_COORDS)
	{
		// Create optional texture coordinates array
		mIsoVertexTexCoords = new Vector2[size];
	}
}


void IsoSurfaceBuilder::createGridCells()
{
	int x = mDataGrid->getNumCellsX();
	int y = mDataGrid->getNumCellsY();
	int z = mDataGrid->getNumCellsZ();

	mGridCells = new GridCell[x*y*z];
	GridCell* gridCell = mGridCells;

	// Initialize the corner index arrays of the grid cells.
	for (int k = 0; k < z; ++k)
	{
		for (int j = 0; j < y; ++j)
		{
			for (int i = 0; i < x; ++i)
			{
				gridCell->cornerIndices[0] = mDataGrid->getGridIndex(i, j, k);
				gridCell->cornerIndices[1] = mDataGrid->getGridIndex(i + 1, j, k);
				gridCell->cornerIndices[2] = mDataGrid->getGridIndex(i + 1, j, k + 1);
				gridCell->cornerIndices[3] = mDataGrid->getGridIndex(i, j, k + 1);
				gridCell->cornerIndices[4] = mDataGrid->getGridIndex(i, j + 1, k);
				gridCell->cornerIndices[5] = mDataGrid->getGridIndex(i + 1, j + 1, k);
				gridCell->cornerIndices[6] = mDataGrid->getGridIndex(i + 1, j + 1, k + 1);
				gridCell->cornerIndices[7] = mDataGrid->getGridIndex(i, j + 1, k + 1);

				++gridCell;
			}
		}
	}

	// Create and initialize the iso vertex index arrays of all grid cells.
	createGridCellIsoVertices();
}

void IsoSurfaceBuilder::destroyGridCells()
{
	int count = mDataGrid->getNumCellsX() * mDataGrid->getNumCellsY() * mDataGrid->getNumCellsZ();
	GridCell* gridCell = mGridCells;
	if(gridCell!=0)
	{
		// Delete the iso vertex array of all grid cells
		for (int i = 0; i < count; ++i)
		{
			SAFE_DELETE_ARRAY(gridCell->isoVertices);
			++gridCell;
		}

		// Delete the grid cell array
		SAFE_DELETE_ARRAY(mGridCells);
	}
}


void IsoSurfaceBuilder::createGridCellIsoVertices()
{
	int x = mDataGrid->getNumCellsX();
	int y = mDataGrid->getNumCellsY();
	int z = mDataGrid->getNumCellsZ();

	GridCell* gridCell = mGridCells;

	// Create and initialize the iso vertex index arrays of all grid cells.
	for (int k = 0; k < z; ++k)
	{
		for (int j = 0; j < y; ++j)
		{
			for (int i = 0; i < x; ++i)
			{
				gridCell->isoVertices = new int[12];

				// The following rather cryptic lines store the indices of the iso vertices
				// that are making up this grid cell.
				gridCell->isoVertices[0] = isoVertexGroupOffsets[0] + k*x*(y+1) + j*x + i;
				gridCell->isoVertices[1] = isoVertexGroupOffsets[2] + k*(x+1)*(y+1) + j*(x+1) + (i+1);
				gridCell->isoVertices[2] = isoVertexGroupOffsets[0] + (k+1)*x*(y+1) + j*x + i;
				gridCell->isoVertices[3] = isoVertexGroupOffsets[2] + k*(x+1)*(y+1) + j*(x+1) + i;

				gridCell->isoVertices[4] = isoVertexGroupOffsets[0] + k*x*(y+1) + (j+1)*x + i;
				gridCell->isoVertices[5] = isoVertexGroupOffsets[2] + k*(x+1)*(y+1) + (j+1)*(x+1) + (i+1);
				gridCell->isoVertices[6] = isoVertexGroupOffsets[0] + (k+1)*x*(y+1) + (j+1)*x + i;
				gridCell->isoVertices[7] = isoVertexGroupOffsets[2] + k*(x+1)*(y+1) + (j+1)*(x+1) + i;

				gridCell->isoVertices[8] = isoVertexGroupOffsets[1] + k*(x+1)*y + j*(x+1) + i;
				gridCell->isoVertices[9] = isoVertexGroupOffsets[1] + k*(x+1)*y + j*(x+1) + (i+1);
				gridCell->isoVertices[10] = isoVertexGroupOffsets[1] + (k+1)*(x+1)*y + j*(x+1) + (i+1);
				gridCell->isoVertices[11] = isoVertexGroupOffsets[1] + (k+1)*(x+1)*y + j*(x+1) + i;

				++gridCell;
			}
		}
	}
}

// Oh my god, I'm using a macro! But it does make this easier to read.
#define USE_ISO_VERTEX(i, a, b) useIsoVertex( \
			gridCell->isoVertices[i], \
			gridCell->cornerIndices[a], \
			gridCell->cornerIndices[b])

void IsoSurfaceBuilder::buildIsoSurface()
{
	int count =
		mDataGrid->getNumCellsX() *
		mDataGrid->getNumCellsY() *
		mDataGrid->getNumCellsZ();
	float* values = mDataGrid->getValues();
	GridCell* gridCell = mGridCells;

	// Loop through all grid cells
	for (int i = 0; i < count; ++i)
	{
		IsoTriangle isoTriangle;
		int flags = 0;

		// Flag the corners that are outside the iso surface
		if (values[gridCell->cornerIndices[0]] < mIsoValue) flags |= 1;
		if (values[gridCell->cornerIndices[1]] < mIsoValue) flags |= 2;
		if (values[gridCell->cornerIndices[2]] < mIsoValue) flags |= 4;
		if (values[gridCell->cornerIndices[3]] < mIsoValue) flags |= 8;
		if (values[gridCell->cornerIndices[4]] < mIsoValue) flags |= 16;
		if (values[gridCell->cornerIndices[5]] < mIsoValue) flags |= 32;
		if (values[gridCell->cornerIndices[6]] < mIsoValue) flags |= 64;
		if (values[gridCell->cornerIndices[7]] < mIsoValue) flags |= 128;

		// Optionally flip normals
		if (!mFlipNormals)
			flags = 0xFF - flags;

		// Find the vertices where the surface intersects the cube
		if (msEdgeTable[flags] &    1) USE_ISO_VERTEX( 0, 0, 1);
		if (msEdgeTable[flags] &    2) USE_ISO_VERTEX( 1, 1, 2);
		if (msEdgeTable[flags] &    4) USE_ISO_VERTEX( 2, 2, 3);
		if (msEdgeTable[flags] &    8) USE_ISO_VERTEX( 3, 3, 0);
		if (msEdgeTable[flags] &   16) USE_ISO_VERTEX( 4, 4, 5);
		if (msEdgeTable[flags] &   32) USE_ISO_VERTEX( 5, 5, 6);
		if (msEdgeTable[flags] &   64) USE_ISO_VERTEX( 6, 6, 7);
		if (msEdgeTable[flags] &  128) USE_ISO_VERTEX( 7, 7, 4);
		if (msEdgeTable[flags] &  256) USE_ISO_VERTEX( 8, 0, 4);
		if (msEdgeTable[flags] &  512) USE_ISO_VERTEX( 9, 1, 5);
		if (msEdgeTable[flags] & 1024) USE_ISO_VERTEX(10, 2, 6);
		if (msEdgeTable[flags] & 2048) USE_ISO_VERTEX(11, 3, 7);

		// Generate triangles for this cube
		for (int i = 0; msTriangleTable[flags][i] != -1; i += 3)
		{
			isoTriangle.vertices[0] = gridCell->isoVertices[msTriangleTable[flags][i]];
			isoTriangle.vertices[1] = gridCell->isoVertices[msTriangleTable[flags][i+1]];
			isoTriangle.vertices[2] = gridCell->isoVertices[msTriangleTable[flags][i+2]];
			addIsoTriangle(isoTriangle);
		}

		++gridCell;
	}
}

void ParaEngine::IsoSurfaceBuilder::addIsoTriangle(const IsoTriangle& isoTriangle)
{
	if ((mSurfaceFlags & GEN_NORMALS) && (mNormalType != NORMAL_GRADIENT))
	{
		Vector3 normal;
		normal = (mIsoVertexPositions[isoTriangle.vertices[1]] - mIsoVertexPositions[isoTriangle.vertices[0]]).crossProduct(mIsoVertexPositions[isoTriangle.vertices[2]] - mIsoVertexPositions[isoTriangle.vertices[0]]);

		switch (mNormalType)
		{
		case NORMAL_WEIGHTED_AVERAGE:
		{
			Vector3 tmp;
			tmp = normal.normalisedCopy();
			normal = tmp / normal.length();
			break;
		}

		case NORMAL_AVERAGE:
			normal.normalise();
			break;
		}

		mIsoVertexNormals[isoTriangle.vertices[0]] += normal;
		mIsoVertexNormals[isoTriangle.vertices[1]] += normal;
		mIsoVertexNormals[isoTriangle.vertices[2]] += normal;
	}

	mIsoTriangles.push_back(isoTriangle);
}

int ParaEngine::IsoSurfaceBuilder::useIsoVertex(int isoVertex, int corner0, int corner1)
{
	// Return the assigned hardware vertex buffer index if the iso vertex has already been used
	if (mIsoVertexIndices[isoVertex] != ~0)
		return isoVertex;

	// Calculate the transition of the iso vertex between the two corners
	float* values = mDataGrid->getValues();
	float t = (mIsoValue - values[corner0]) / (values[corner1] - values[corner0]);

	// Calculate the iso vertex position by interpolation
	const Vector3* vertices = mDataGrid->getVertices();
	mIsoVertexPositions[isoVertex] = vertices[corner0] + t*(vertices[corner1] - vertices[corner0]);

	if (mSurfaceFlags & GEN_NORMALS)
	{
		// Generate optional normal
		switch (mNormalType)
		{
		case NORMAL_WEIGHTED_AVERAGE:
		case NORMAL_AVERAGE:
			mIsoVertexNormals[isoVertex] = Vector3(0, 0, 0);
			break;

		case NORMAL_GRADIENT:
		{
								Vector3* gradient = mDataGrid->getGradient();
								if (mFlipNormals)
									mIsoVertexNormals[isoVertex] = gradient[corner0] + t*(gradient[corner1] - gradient[corner0]);
								else
									mIsoVertexNormals[isoVertex] = t*(gradient[corner0] - gradient[corner1]) - gradient[corner0];
		}
		}
	}

	if (mSurfaceFlags & GEN_VERTEX_COLOURS)
	{
		// Generate optional vertex colours by interpolation
		Color* colours = mDataGrid->getColours();
		LinearColor outColor = Math::Lerp(LinearColor(colours[corner0]), LinearColor(colours[corner1]), t);
		mIsoVertexColours[isoVertex] = outColor;
	}

	if (mSurfaceFlags & GEN_TEX_COORDS)
	{
		// Generate optional texture coordinates

		// use texture according to position in x,z plane, so it is like texture projection effect. 
		mIsoVertexTexCoords[isoVertex].x = mIsoVertexPositions[isoVertex].x;
		mIsoVertexTexCoords[isoVertex].y = mIsoVertexPositions[isoVertex].z;

		//mIsoVertexTexCoords[isoVertex][0] = mIsoVertexPositions[isoVertex].x;
		//mIsoVertexTexCoords[isoVertex][1] = mIsoVertexPositions[isoVertex].y;
		//mIsoVertexTexCoords[isoVertex][2] = mIsoVertexPositions[isoVertex].z;
	}

	// Assign the next index in the hardware vertex buffer to this iso vertex
	mIsoVertexIndices[isoVertex] = (int)mIsoVertices.size(); // TODO: Inefficient?
	mIsoVertices.push_back(isoVertex);

	return isoVertex;
}

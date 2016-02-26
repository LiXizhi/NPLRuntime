//-----------------------------------------------------------------------------
// Class:	DataGrid
// Authors:	Ported by LiXizhi, original code by Copyright (c) 2007 Martin Enge. Based on code from DWORD, released into public domain. martin.enge@gmail.com
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DataGrid.h"

using namespace ParaEngine;


DataGrid::DataGrid()
  : mValues(0), mVertices(0), mGradient(0), mColours(0), mMetaWorldFragments(0),
    mPosition(0,0,0)
{
}

DataGrid::~DataGrid()
{
	SAFE_DELETE_ARRAY(mValues);
	SAFE_DELETE_ARRAY(mVertices);
	SAFE_DELETE_ARRAY(mGradient);
	SAFE_DELETE_ARRAY(mColours);
	SAFE_DELETE_ARRAY(mMetaWorldFragments);
}

void DataGrid::initialize(int numCellsX, int numCellsY, int numCellsZ, float gridScale, int flags)
{
	// Initialize members
	mNumCellsX = numCellsX;
	mNumCellsY = numCellsY;
	mNumCellsZ = numCellsZ;
	mGridScale = gridScale;
	mGridFlags = flags;

	mNumGridPoints = (mNumCellsX + 1)*(mNumCellsY + 1)*(mNumCellsZ + 1);

	// Create mandatory value and grid vertex arrays
	mValues = new float[mNumGridPoints];
	mVertices = new Vector3[mNumGridPoints];

	if (hasGradient())
	{
		// Create optional gradient vector array
		mGradient = new Vector3[mNumGridPoints];
	}

	if (hasColours())
	{
		// Create optional colour value array
		mColours = new Color[mNumGridPoints];
	}
	
	if (hasMetaWorldFragments())
	{
		// Create optional MetaWorldFragment array
		mMetaWorldFragments = new std::pair<float, MetaWorldFragment*>[mNumGridPoints];
	}

	// Initialize the position of grid points and the bounding box
	initializeVertices();
}


void DataGrid::initializeVertices()
{
	Vector3 maximum = Vector3(
		0.5f*mGridScale*mNumCellsX,
		0.5f*mGridScale*mNumCellsY,
		0.5f*mGridScale*mNumCellsZ);

	Vector3* pVertex = mVertices;
	Vector3 position = -maximum;

	// Setup bounding box
	mBoundingBox.SetMinMax(-maximum, maximum);
	mBoxSize.SetMinMax(-maximum, maximum);

	// Initialize grid vertices
	for (int k = 0; k <= mNumCellsZ; ++k)
	{
		for (int j = 0; j <= mNumCellsY; ++j)
		{
			for (int i = 0; i <= mNumCellsX; ++i)
			{
				*pVertex++ = position;
				position.x += mGridScale;
			}
			position.x = -maximum.x;
			position.y += mGridScale;
		}
		position.y = -maximum.y;
		position.z += mGridScale;
	}
}

bool DataGrid::mapAABB(const CShapeAABB& aabb, int &x0, int &y0, int &z0, int &x1, int &y1, int &z1) const
{
	// x0
	if (aabb.GetMin().x <= mBoundingBox.GetMin().x)
		x0 = 0;
	else
	if (aabb.GetMin().x > mBoundingBox.GetMax().x)
		return false;
	else
		x0 = (int)::ceil((aabb.GetMin().x - mBoundingBox.GetMin().x) / mGridScale);

	// y0
	if (aabb.GetMin().y <= mBoundingBox.GetMin().y)
		y0 = 0;
	else
	if (aabb.GetMin().y > mBoundingBox.GetMax().y)
		return false;
	else
		y0 = (int)::ceil((aabb.GetMin().y - mBoundingBox.GetMin().y) / mGridScale);

	// z0
	if (aabb.GetMin().z <= mBoundingBox.GetMin().z)
		z0 = 0;
	else
	if (aabb.GetMin().z > mBoundingBox.GetMax().z)
		return false;
	else
		z0 = (int)::ceil((aabb.GetMin().z - mBoundingBox.GetMin().z) / mGridScale);

	// x1
	if (aabb.GetMax().x < mBoundingBox.GetMin().x)
		return false;
	else
	if (aabb.GetMax().x >= mBoundingBox.GetMax().x)
		x1 = mNumCellsX;
	else
		x1 = (int)::floor((aabb.GetMax().x - mBoundingBox.GetMin().x) / mGridScale);

	// y1
	if (aabb.GetMax().y < mBoundingBox.GetMin().y)
		return false;
	else
	if (aabb.GetMax().y >= mBoundingBox.GetMax().y)
		y1 = mNumCellsY;
	else
		y1 = (int)::floor((aabb.GetMax().y - mBoundingBox.GetMin().y) / mGridScale);

	// z1
	if (aabb.GetMax().z < mBoundingBox.GetMin().z)
		return false;
	else
	if (aabb.GetMax().z >= mBoundingBox.GetMax().z)
		z1 = mNumCellsZ;
	else
		z1 = (int)::floor((aabb.GetMax().z - mBoundingBox.GetMin().z) / mGridScale);

	return true;
}

void DataGrid::clear()
{
	float* value = mValues;
	Vector3* gradient = mGradient;
	Color* colour = mColours;
	std::pair<float, MetaWorldFragment*>* frags = mMetaWorldFragments;

	// Clear data grid
	for (int i = 0; i < mNumGridPoints; ++i)
	{
		*value++ = 0.0;

		if (hasGradient())
			*gradient++ = Vector3(0,0,0);

		if (hasColours())
			*colour++ = 0x0;

		if (hasMetaWorldFragments())
		{
			frags->first = 0.0;
			frags->second = 0;
			frags++;
		}
	}
}

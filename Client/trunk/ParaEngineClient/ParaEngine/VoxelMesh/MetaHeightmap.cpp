//-----------------------------------------------------------------------------
// Class:	MetaHeightmap
// Authors:	Ported by LiXizhi, original code by Copyright (c) 2007 Martin Enge. Based on code from DWORD, released into public domain. martin.enge@gmail.com
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "MetaHeightmap.h"
#include "DataGrid.h"
#include "terrain/GlobalTerrain.h"

using namespace ParaEngine;


MetaHeightmap::MetaHeightmap(MetaWorldFragment *wf, const TerrainRegion& t, float groundThreshold)
: MetaObject(wf, t.getCenter()), mTerrainTile(t), mGroundThreshold(groundThreshold), 
  mFallofRange(0), mGradient(0)
{
}

/// Adds this meta height map to the data grid.
void MetaHeightmap::updateDataGrid(DataGrid* dataGrid)
{
	float* values = dataGrid->getValues();
	const Vector3* vertices = dataGrid->getVertices();
	Vector3* gradient = dataGrid->getGradient();
	Color* colours = dataGrid->getColours();
	std::pair<float, MetaWorldFragment*>* worldFragments = dataGrid->getMetaWorldFragments();
	if(!mFallofRange)
	{
		mFallofRange = dataGrid->getGridScale();
		mGradient = mGroundThreshold / dataGrid->getGridScale();
	}
	Vector3 gridMin = dataGrid->getBoundingBox().GetMin();
	Vector3 gridCenter = dataGrid->getPosition();
	for (int z = 0; z <= dataGrid->getNumCellsZ(); ++z)
	{
		for (int x = 0; x <= dataGrid->getNumCellsX(); ++x)
		{
				
			Vector3 v = vertices[dataGrid->getGridIndex(x, dataGrid->getNumCellsY(), z)] + gridCenter;
			float h = mTerrainTile.getHeightAt(v.x, v.z);
			for (int y = 0; y <= dataGrid->getNumCellsY(); ++y)
			{
				int index = dataGrid->getGridIndex(x, y, z);
				Vector3 v2 = vertices[index]+gridCenter;
				float d = h-v2.y;
				float fieldStrength = 0;
				if(d <= -mFallofRange)
					continue;
				if(d >= mFallofRange*2.0f)
				{
					fieldStrength = 2.0f*mGroundThreshold;
				}
				else
				{
					fieldStrength =  (d + mFallofRange) * mGradient;
				}
				values[index] += fieldStrength;
				

				if (gradient)
					gradient[index] += v;/// Not sure about this...
				
				if(worldFragments)
				{
					if(fieldStrength > worldFragments[index].first)
					{
						worldFragments[index].first = fieldStrength;
						worldFragments[index].second = mMetaWorldFragment;
					}
				}

				//if (colours)
				//	colours[index] = Color::White;
			}
		}
	}
}

CShapeAABB MetaHeightmap::getAABB() const
{
	return mTerrainTile.getBoundingBox();
}

float MetaHeightmap::TerrainRegion::getHeightAt( float x, float z )
{
	// TODO: LXZ: add render offset and world space. , since it is in object space. 
	return CGlobals::GetGlobalTerrain()->GetElevation(x,z);
}

Vector3 MetaHeightmap::TerrainRegion::getCenter() const
{
	Vector3 v;
	m_aabb.GetCenter(v);
	return v;
}
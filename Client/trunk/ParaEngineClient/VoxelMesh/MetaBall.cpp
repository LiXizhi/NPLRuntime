//-----------------------------------------------------------------------------
// Class:	MetaBall
// Authors:	Ported by LiXizhi, original code by Copyright (c) 2007 Martin Enge. Based on code from DWORD, released into public domain. martin.enge@gmail.com
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DataGrid.h"
#include "MetaBall.h"
#include "DataGrid.h"
#include "MetaWorldFragment.h"

using namespace ParaEngine;

MetaBall::MetaBall(MetaWorldFragment *wf, const Vector3& position, float radius, bool excavating)
  : MetaObject(wf, position), mRadius(radius), mExcavating(excavating)
{
}

void MetaBall::updateDataGrid(DataGrid* dataGrid)
{
	CShapeAABB aabb = getAABB();

	int x0, y0, z0, x1, y1, z1;

	// Find the grid points this meta ball can possibly affect
	if (!dataGrid->mapAABB(aabb, x0, y0, z0, x1, y1, z1))
		return;

	float* values = dataGrid->getValues();
	const Vector3* vertices = dataGrid->getVertices();
	Vector3* gradient = dataGrid->getGradient();
	Color* colours = dataGrid->getColours();
	std::pair<float, MetaWorldFragment*>* worldFragments = dataGrid->getMetaWorldFragments();
	float fRadiusSq = mRadius*mRadius;
	for (int z = z0; z <= z1; ++z)
	{
		for (int y = y0; y <= y1; ++y)
		{
			for (int x = x0; x <= x1; ++x)
			{
				int index = dataGrid->getGridIndex(x, y, z);
				
				// http://www.geisswerks.com/ryan/BLOBS/blobs.html
/*				
				// Define math constants
				#ifndef M_SQRT1_2
				#define M_SQRT1_2 0.707106781186547524401
				#endif

				Vector3 v = (vertices[index] - mPosition);
				float r = v.length() * M_SQRT1_2 / mRadius;
				if (r > M_SQRT1_2)
					continue;
				values[index] += r*r*r*r - r*r + 0.25;
*/
				Vector3 v = (vertices[index] - mPosition + dataGrid->getPosition());

				float r2 = v.squaredLength() / (2.0f * fRadiusSq);
				if (r2 > 0.5f)
					continue;
				float currentFieldStrength = r2*r2 - r2 + 0.25f;

				if(mExcavating)
					values[index] -= currentFieldStrength;
				else
					values[index] += currentFieldStrength;
				

				if (gradient)
					gradient[index] += v / (fRadiusSq);
				
				if(worldFragments)
				{
					if(currentFieldStrength > worldFragments[index].first)
					{
						worldFragments[index].first = currentFieldStrength;
						worldFragments[index].second = mMetaWorldFragment;
					}
				}

				//if (colours)
				//	colours[index] = Color::White;
			}
		}
	}
}

CShapeAABB MetaBall::getAABB() const
{
	CShapeAABB aabb;
	aabb.SetMinMax(Vector3(mPosition - Vector3(mRadius, mRadius, mRadius)), Vector3(mPosition + Vector3(mRadius, mRadius, mRadius)));
	return aabb;
}

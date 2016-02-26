//-----------------------------------------------------------------------------
// Class:	TTerrain
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.12.8
// Revised: 2005.12.8
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Loader.h"
#include "TTerrain.h"

using namespace ParaEngine;

/** @def the smallest y scale. */
#define MIN_Y_SCALE		0.0001f


//////////////////////////////////////////////////////////////////////////
/// TTerrain class 
//////////////////////////////////////////////////////////////////////////

TTerrain::TTerrain()
:selection(NULL), heightfield(NULL),width(0), height(0),bIsNormalized(false)
{

}

TTerrain::~TTerrain()
{
	Cleanup();
}

void TTerrain::Cleanup()
{
	SAFE_DELETE_ARRAY(selection);
	SAFE_DELETE_ARRAY(heightfield);
}

bool  TTerrain::CreatePlane(int gridSize, float fHeight , float fSpacing)
{
	if(gridSize<=0)
		return false;

	fVertexSpacing = fSpacing;
	Cleanup();

	height = gridSize;
	width = gridSize;
	int nSize = height*width;
	heightfield = new float[nSize];
	if(heightfield)
	{
		for(int i=0;i<nSize;++i)
		{
			heightfield[i] = fHeight;
		}
		return true;
	}
	else
		OUTPUT_DEBUG("failed creating plane");
	return false;
}

bool  TTerrain::CreatePlane(const char* filename,float fSpacing)
{
	fVertexSpacing = fSpacing;
	Cleanup();

	if(SUCCEEDED(ParaTerrain::Loader::GetInstance()->LoadElevations(&heightfield, &height, filename)))
	{
		width = height;
		return true;
	}
	else
		Cleanup();
	return false;
}

float TTerrain::GetVertexSpacing()
{
	return fVertexSpacing;
}
void TTerrain::SetVertexSpacing(float fSpacing)
{
	fVertexSpacing = fSpacing;
}

int TTerrain::GetGridSize()
{
	return width;
}
float TTerrain::GetMinHeight()
{
	return fMin;
}
float TTerrain::GetMaxHeight()
{
	return fMax;
}

float TTerrain::GetYScale()
{
	return y_scale_factor;
}

void TTerrain::SetHeight(int x, int y, float fValue)
{
	heightfield[y*width+x] = fValue;
}

float TTerrain::GetHeight(int x, int y)
{
	return heightfield[y*width+x];
}

float TTerrain::GetHeightFinal(int x, int y)
{
	float fValue = heightfield[y*width+x];

	if(IsNormalized())
	{
		if(y_scale_factor==0)
			return fValue +fMin;
		else
			return (fValue*y_scale_factor+fMin);
	}
	return fValue;
}

bool TTerrain::IsNormalized()
{
	return bIsNormalized;
}
void TTerrain::SetNormalized(bool bSet)
{
	bIsNormalized = bSet;
}

void TTerrain::Normalize()
{
	if(bIsNormalized) 
		return;
	else
		bIsNormalized = !bIsNormalized;

	int    i, size;
	size = width * height;

	// recalculate fMin, fMax
	fMin = heightfield[0];
	fMax = fMin;
	for (i = 0; i < size; i++)
	{
		float value = heightfield[i];
		if(value < fMin)
			fMin = value;
		if(value > fMax)
			fMax = value;
	}
	y_scale_factor = fMax - fMin;
	if(y_scale_factor<=MIN_Y_SCALE)
		y_scale_factor = 0;

	// normalize
	for (i = 0; i < size; i++)
	{
		if(y_scale_factor==0)
			heightfield[i] = heightfield[i] -fMin;
		else
			heightfield[i] = (heightfield[i] -fMin)/y_scale_factor;
	}
}

void ParaEngine::TTerrain::NormalizeHeight( float& height )
{
	if(bIsNormalized) 
	{
		if(y_scale_factor==0)
			height = height -fMin;
		else
			height = (height -fMin)/y_scale_factor;
	}
}

void ParaEngine::TTerrain::SetMinHeight( float fMinHeight )
{
	if(GetMinHeight() < fMinHeight)
	{
		int    i, size;
		size = width * height;
		for (i = 0; i < size; ++i)
		{
			if( heightfield[i]< fMinHeight )
				heightfield[i] = fMinHeight;
		}
	}
}

void TTerrain::Recover()
{
	if(!bIsNormalized) 
		return;
	else
		bIsNormalized = !bIsNormalized;

	int    i, size;
	size = width * height;

	// recover
	if(y_scale_factor==0)
	{
		for (i = 0; i < size; ++i)
		{
			heightfield[i] = heightfield[i] +fMin;
		}
	}
	else
	{
		for (i = 0; i < size; ++i)
		{
			heightfield[i] = (heightfield[i]*y_scale_factor+fMin);
		}
	}
	
}

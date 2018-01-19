//-----------------------------------------------------------------------------
// Class:	MetaWorldFragment
// Authors:	Ported by LiXizhi, original code by Copyright (c) 2007 Martin Enge. Based on code from DWORD, released into public domain. martin.enge@gmail.com
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.1.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "MetaWorldFragment.h"
#include "DataGrid.h"
#include "MetaObject.h"
#include "TextureEntity.h"
#include "IsoSurfaceBuilder.h"
#include "IsoSurfaceRenderable.h"

using namespace ParaEngine;


//#define NUM_CELLS 30
//#define WIDTH 4.0
//#define SCALE 4.0/30.0


float MetaWorldFragment::mGridScale = 0;
float MetaWorldFragment::mSize = 0;
std::string MetaWorldFragment::mMaterialName = "";


MetaWorldFragment::MetaWorldFragment(const Vector3 &position, int ylevel)
: 	mSurf(NULL), mPosition(position), mYLevel(ylevel),m_bNeedUpdate(false)
{
}


MetaWorldFragment::~MetaWorldFragment()
{
	SAFE_DELETE(mSurf);
}

///Adds MetaObject to mObjs, and to mMoDataGrid
void MetaWorldFragment::addMetaObject(MetaObjectPtr mo)
{
	if(mo->getMetaWorldFragment() != this && mo->getMetaWorldFragment() != 0)
		addToWfList(mo->getMetaWorldFragment());
	mObjs.push_back(mo);
	SetNeedUpdate(true);
}

///Updates IsoSurface
void MetaWorldFragment::update(IsoSurfaceBuilder *builder)
{
	// only update if needed. 
	if(!m_bNeedUpdate)
		return;
	m_bNeedUpdate = false;

	if(!mSurf)
	{
		mSurf = new IsoSurfaceRenderable();
		mSurf->initialize(builder);
	}
	/// Zero data grid, then add the fields of objects to it.
	DataGrid * dg = builder->getDataGrid();
	dg->clear();
	for(std::vector<MetaObjectPtr>::iterator it = mObjs.begin(); it != mObjs.end(); ++it)
	{
		(*it)->updateDataGrid(dg);
	}
	builder->update(mSurf);
	mAabb = dg->getBoundingBox();
}

void MetaWorldFragment::addToWfList(MetaWorldFragment* wf)
{
	for(WfConstIter it = mAdjacentFragments.begin(); it != mAdjacentFragments.end(); ++it)
	{
		if( *it != this )
			mAdjacentFragments.push_back(*it);
	}
}

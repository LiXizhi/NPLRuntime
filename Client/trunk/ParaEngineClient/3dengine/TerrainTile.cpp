//-----------------------------------------------------------------------------
// Class:	CTerrainTile
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ShapeAABB.h"
#include "TerrainTile.h"

using namespace ParaEngine;

/** 2009.8.26: if defined, we will attach mesh object to the most suitable terrain tile
if not defined, we will always attach mesh object to the leaf terrain tile. 
if we load object asynchronously, the mesh size is not known when attached to scene, it is better to undeine this macro, 
otherwise, Find object by view object may be inconsistent.  
*/
// #define TERRAIN_QUAD_TREE_BEST_FIT


/** sub tile and its locations
^ Y
|______
|2 |3 |
-------
|0 |1 |
---------->x
*/
CTerrainTile::CTerrainTile()
{
	for(int i=0; i<MAX_NUM_SUBTILE; i++)
		m_subtiles[i] = NULL;
}

CTerrainTile::CTerrainTile(float x, float y, float r)
{
	m_fX=x; m_fY=y; m_fRadius=r;
	for(int i=0; i<MAX_NUM_SUBTILE; i++)
		m_subtiles[i] = NULL;
}

CTerrainTile::~CTerrainTile(void)
{
	Cleanup();
}


void CTerrainTile::AddVisitor(IGameObject* pObj, bool bCheckDuplicate)
{
	if(!bCheckDuplicate)
	{
		m_listVisitors.push_back(IGameObject::WeakPtr_type(pObj));
	}
	else
	{
		// delete object from the sentient object list.
		for (auto& visitor : m_listVisitors)
		{
			if (visitor.get() == pObj){
				return;
			}
		}
		m_listVisitors.push_back(IGameObject::WeakPtr_type(pObj));
	}
}

bool CTerrainTile::RemoveVisitor(IGameObject* pObject)
{
	VisitorList_type::iterator itCur,itEnd = m_listVisitors.end();
	for (itCur = m_listVisitors.begin();itCur!=itEnd;++itCur)
	{
		if ((*itCur).get() == pObject){
			m_listVisitors.erase(itCur);
			return true;
		}
	}
	return false;
}

void CTerrainTile::Cleanup()
{
	m_listFreespace.clear();
	m_listFreespace.shrink_to_fit();
	m_listSolidObj.clear();
	m_listSolidObj.shrink_to_fit();

	// clear name mapping.
	m_namemap.clear();

	m_listVisitors.clear();

	for(int i=0; i<MAX_NUM_SUBTILE; i++)
	{
		if(m_subtiles[i])
		{
			// implicitly calls m_subtiles[i]->Cleanup();
			delete m_subtiles[i];
			m_subtiles[i] = NULL;
		}
	}
}

void CTerrainTile::DestroyObjectByName(const char* sName)
{
	for( auto itCurCP = m_listSolidObj.begin(); itCurCP != m_listSolidObj.end(); )
	{
		if((*itCurCP)->GetIdentifier() == sName)
		{
			itCurCP = m_listSolidObj.erase(itCurCP);
		}
		else
			++itCurCP;
	}
	// erase from name mapping.
	map<string,CBaseObject*>::iterator iter = m_namemap.find(sName);
	if (iter!=m_namemap.end()) 
		m_namemap.erase(iter);
}
CBaseObject* CTerrainTile::GetObject(const string& sName)
{
	map<string,CBaseObject*>::iterator iter = m_namemap.find(sName);
	if (iter!=m_namemap.end()) 
		return iter->second;
	return NULL;
}

CBaseObject* CTerrainTile::SearchObject(const char* searchString, int search_mode, int reserved)
{
	if(searchString == NULL)
		return NULL;
	string matchPattern(searchString);
	if(search_mode == 0)
	{
		// first search using exact name mapping
		CBaseObject* obj = GetObject(searchString);
		if(obj!= NULL)
			return obj;
		/* search from solid objects.*/
		for (auto& pObject : m_listSolidObj)
		{
			if (Math::MatchString(matchPattern, pObject->GetIdentifier()))
			{
				return pObject;
			}
		}
	}
	return NULL;
}

/// get object position in the world space
void CTerrainTile::GetPosition(Vector3 *pV)	
{
	(*pV).x  = m_fX;
	(*pV).z  = m_fY;
	(*pV).y  = 0;
}

bool CTerrainTile::TestCollisionSphere(const Vector3* pvCenter, FLOAT radius)
{
	float fSize = m_fRadius + radius;

	Vector3 vCenter;	// center of this object
	GetPosition(&vCenter);
	Vector3 vPos = (*pvCenter) - vCenter;		// relative position

	//-- now test 
	/// Note: I only tested x,z, I care not about the y component, which is related to object's height
	if(((-fSize)>vPos.x) || (fSize<vPos.x) || 
		((-fSize)>vPos.z) || (fSize<vPos.z))
		return false;
	else
		return true;
}


float CTerrainTile::GetSphereCollisionDepth(Vector3* pvCenter, FLOAT radius, bool bSolveDepth)
{
	float fSize = m_fRadius + radius;

	Vector3 vCenter;	// center of this object
	GetPosition(&vCenter);
	Vector3 vPos = (*pvCenter) - vCenter;		// relative position
	
	//-- now test 
	// Note: I only tested x,z, I care not about the y component, which is related to object's height
	FLOAT min1 = min( (vPos.x + fSize),  (fSize-vPos.x) );
	FLOAT min2 = min( (vPos.z + fSize),  (fSize-vPos.z) );
	FLOAT fDepth = 0;			// depth of collision
	fDepth = min(min1, min2);

	return fDepth;
}

//------------------------------------------------------------------------------
// name: GetObjectCollisionDepth
/// desc: test if the circle in the y=0 plane collide with this object in the world view
/// Note: radius can be zero. We assume that test is done in the y=0 plane only
//------------------------------------------------------------------------------
float CTerrainTile::GetObjectCollisionDepth(CBaseObject* pObj)
{
	Vector3 v = pObj->GetPosition();
	return GetSphereCollisionDepth(&v, pObj->GetRadius(), false);
}


//------------------------------------------------------------------------------
// name: TestCollisionObject
/// desc: test if the object collide with this object in the world view. Currently
/// the pObj is regarded as a spheracal/circular object. mostly it's a biped object. And this
/// object can have shape of any type.
//------------------------------------------------------------------------------
bool CTerrainTile::TestCollisionObject(CBaseObject* pObj)
{
	Vector3 v = pObj->GetPosition();
	return TestCollisionSphere(&v, pObj->GetRadius());
}

/** 
desc: get the sub tile index which best tells the position of point given
sub tile and its locations.
^ Y
|______
|2 |3 |
-------
|0 |1 |
---------->x
*/
//------------------------------------------------------------------------------
int	CTerrainTile::GetSubTileIndexByPoint(FLOAT fAbsoluteX, FLOAT fAbsoluteY)
{
	if(fAbsoluteX<m_fX)
	{// 0, 2
		if(fAbsoluteY<m_fY)
			return 0;
		else
			return 2;
	}
	else
	{// 1,3
		if(fAbsoluteY<m_fY)
			return 1;
		else
			return 3;
	}
}
int	CTerrainTile::GetChildTileIndexByRect(FLOAT fAbsoluteX, FLOAT fAbsoluteY, FLOAT fPtWidth, FLOAT fPtHeight)
{
	float left = fAbsoluteX-fPtWidth/2, top = fAbsoluteY-fPtHeight/2, 
		right = fAbsoluteX+fPtWidth/2, bottom = fAbsoluteY+fPtHeight/2;

	CTerrainTile* pTile=this;
	int nIndex = 0;// bit indexes
	float x1,y1,x2,y2;
	/* sub tile0 */
	x1 = pTile->m_fX-pTile->m_fRadius;
	y1 = pTile->m_fY-pTile->m_fRadius;
	x2 = pTile->m_fX;
	y2 = pTile->m_fY;

	if((x1<left && left<x2 && y1<top && top<y2) &&
		(x1<right && right<x2 && y1<bottom && bottom<y2) )
	{
		nIndex |= 1;
	}
	/* sub tile1 */
	x1 += pTile->m_fRadius;
	y1 = y1;
	x2 += pTile->m_fRadius;
	y2 = y2;

	if((x1<left && left<x2 && y1<top && top<y2) &&
		(x1<right && right<x2 && y1<bottom && bottom<y2) )
	{
		nIndex |= 1<<1;
	}

	/* sub tile3 */
	x1 = x1;
	y1 += pTile->m_fRadius;
	x2 = x2;
	y2 += pTile->m_fRadius;

	if((x1<left && left<x2 && y1<top && top<y2) &&
		(x1<right && right<x2 && y1<bottom && bottom<y2) )
	{
		nIndex |= 1<<3;
	}

	/* sub tile2 */
	x1 -= pTile->m_fRadius;
	y1 = y1;
	x2 -= pTile->m_fRadius;
	y2 = y2;

	if((x1<left && left<x2 && y1<top && top<y2) &&
		(x1<right && right<x2 && y1<bottom && bottom<y2) )
	{
		nIndex |= 1<<2;
	}

	

	return nIndex;
}
/// nIndex: which tile to get. If the tile is blank, we  will create it.
CTerrainTile*	CTerrainTile::CreateSubTile(int nIndex)
{
	if(m_subtiles[nIndex] == NULL)
	{
		/// create the tile at position nIndexs
		/** sub tile and its locations.
		^ Y
		|______
		|2 |3 |
		-------
		|0 |1 |
		---------->x
		*/
		float fR = m_fRadius/2; // the new radius
		switch(nIndex)
		{
		case 0:
			m_subtiles[0] = new CTerrainTile(m_fX-fR, m_fY-fR, fR);
			break;
		case 1:
			m_subtiles[1] = new CTerrainTile(m_fX+fR, m_fY-fR, fR);
			break;
		case 2:
			m_subtiles[2] = new CTerrainTile(m_fX-fR, m_fY+fR, fR);
			break;
		case 3:
			m_subtiles[3] = new CTerrainTile(m_fX+fR, m_fY+fR, fR);
			break;
		default:
			break;
		}
	}
	return m_subtiles[nIndex];
}


CBaseObject* ParaEngine::CTerrainTile::GetLocalObject( const Vector3& vPos, const std::string& sName, float fEpsilon/*=0.01f*/ )
{
	CTerrainTile* pTile = this;

	/* search from solid objects.*/
	{
		for (auto& pObject : pTile->m_listSolidObj)
		{
			if (pObject->GetName() == sName)
			{
				Vector3 vItemPos = pObject->GetPosition();
				float fPosDiffSq = (vItemPos-vPos).squaredLength();
				if(fPosDiffSq<fEpsilon)
				{
					return pObject;
				}
			}
			
		}
	}

	/** search for free space objects */
	{
		for (auto& pObject : pTile->m_listFreespace)
		{
			if (pObject->GetName() == sName)
			{
				Vector3 vItemPos = pObject->GetPosition();
				float fPosDiffSq = (vItemPos - vPos).squaredLength();
				if(fPosDiffSq<fEpsilon)
				{
					return pObject;
				}
			}
		}
	}
	pTile = m_subtiles[GetSubTileIndexByPoint(vPos.x, vPos.z)];

	if(pTile)
	{
		return pTile->GetLocalObject(vPos, fEpsilon);
	}

	return NULL;
}

CBaseObject* CTerrainTile::GetLocalObject(const Vector3& vPos, float fEpsilon)
{
	CTerrainTile* pTile = this;

	/* search from solid objects.*/
	{
		for (auto& pObject : pTile->m_listSolidObj)
		{
			Vector3 vItemPos = pObject->GetPosition();
			float fPosDiffSq = (vItemPos - vPos).squaredLength();
			if(fPosDiffSq<fEpsilon)
			{
				return pObject;
			}
		}
	}

	/** search for free space objects */
	{
		for( auto& pObject : pTile->m_listFreespace)
		{
			Vector3 vItemPos = pObject->GetPosition();
			float fPosDiffSq = (vItemPos - vPos).squaredLength();
			if(fPosDiffSq<fEpsilon)
			{
				return pObject;
			}
		}
	}
	pTile = m_subtiles[GetSubTileIndexByPoint(vPos.x, vPos.z)];

	if(pTile)
	{
		return pTile->GetLocalObject(vPos, fEpsilon);
	}

	return NULL;
}

IAttributeFields* ParaEngine::CTerrainTile::GetChildAttributeObject(const std::string& sName)
{
	return IAttributeFields::GetChildAttributeObject(sName);
}

IAttributeFields* ParaEngine::CTerrainTile::GetChildAttributeObject(int nRowIndex, int nColumnIndex)
{
	if (nColumnIndex == 0)
		return (nRowIndex < MAX_NUM_SUBTILE) ? m_subtiles[nRowIndex] : NULL;
	else if (nColumnIndex == 1)
	{
		if (m_children.empty())
		{
			// create attribute model proxy on first use. 
			// exposing m_listSolidObj, m_listFreespace, m_listVisitors via attribute object
			m_children.reserve(3);
			m_children.push_back(new CArrayAttributeProxy< ObjectRefArray_type >(&m_listSolidObj, "listSolidObj"));
			m_children.push_back(new CArrayAttributeProxy< ObjectRefArray_type >(&m_listFreespace, "listFreespace"));
			m_children.push_back(new CArrayAttributeProxy< VisitorList_type >(&m_listVisitors, "listVisitors"));
		}
		if (nRowIndex < (int)m_children.size())
		{
			return m_children.at(nRowIndex);
		}
	}
	return NULL;
}

int ParaEngine::CTerrainTile::GetChildAttributeObjectCount(int nColumnIndex)
{
	if (nColumnIndex == 0)
		return MAX_NUM_SUBTILE;
	else if (nColumnIndex == 1)
		// exposing m_listSolidObj, m_listFreespace, m_listVisitors in second column
		return 3;
	else
		return 0;
}

int ParaEngine::CTerrainTile::GetChildAttributeColumnCount()
{
	return 2;
}

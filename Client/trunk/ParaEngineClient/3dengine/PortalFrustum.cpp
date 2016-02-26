//-----------------------------------------------------------------------------
// Class:	CPortalFrustum
// Authors:	LiXizhi, I ported some open source code by Eric Cha in its portal rendering plugin. 
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.9.9
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PortalNode.h"
#include "BaseObject.h"
#include "ShapeFrustum.h"
#include "PortalFrustum.h"

using namespace ParaEngine;

/////////////////////////////////////////////////////////////////
//
// plane class
//
/////////////////////////////////////////////////////////////////
ParaEngine::PCPlane::PCPlane()
:m_portal(0)
{
}

ParaEngine::PCPlane::PCPlane( const Plane & plane )
:Plane(plane), m_portal(0)
{
}

ParaEngine::PCPlane::PCPlane( const Vector3& rkNormal, const Vector3& rkPoint )
:Plane(rkPoint, rkNormal), m_portal(0)
{
}

ParaEngine::PCPlane::PCPlane( const Vector3& rkPoint0, const Vector3& rkPoint1, const Vector3& rkPoint2 )
:Plane(rkPoint0, rkPoint1, rkPoint2), m_portal(0)
{
}

void ParaEngine::PCPlane::SetPlane( const Plane & plane )
{
	(*this) = plane;
}

ParaEngine::PCPlane::~PCPlane()
{
}

/////////////////////////////////////////////////////////////////
//
// CPortalFrustum class
//
/////////////////////////////////////////////////////////////////
#define DEFAULT_PLANE_COUNT		128

CPortalFrustum::CPortalFrustum(void)
:m_CullingPlaneReservoir(DEFAULT_PLANE_COUNT), m_ActiveCullingPlanes(DEFAULT_PLANE_COUNT)
{
	m_vOrigin = Vector3(0,0,0);
}

CPortalFrustum::~CPortalFrustum(void)
{
	RemoveAllExtraCullingPlanes();
	{
		PCPlaneList::iterator pit = m_CullingPlaneReservoir.begin();
		while ( pit != m_CullingPlaneReservoir.end() )
		{
			PCPlane * plane = *pit;
			SAFE_DELETE(plane);
			pit++;
		}
		m_CullingPlaneReservoir.clear();
	}
}

int ParaEngine::CPortalFrustum::AddFrustum( CShapeFrustum * frustum, float fNearPlaneShiftDistance)
{
	for (int i=0;i<6;++i)
	{
		// add the plane created from the two portal corner points and the frustum location
		// to the  culling plane
		PCPlane * newPlane = getUnusedCullingPlane();
		const Plane& plane = frustum->planeFrustum[i];
		newPlane->Set(plane.a(), plane.b(), plane.c(), (i>0) ? plane.d : (plane.d + fNearPlaneShiftDistance));
		newPlane->SetPortal(NULL);
		m_ActiveCullingPlanes.push_back(newPlane); // tricky: we push to back to give it last priority
	}
	return 6;
}

int ParaEngine::CPortalFrustum::AddPortalCullingPlanes( CPortalNode * portal, bool bAutoDirection)
{
	bool bInverseDirection = false;

	Vector3 frustumToPortal = portal->getDerivedCP() - m_vOrigin;
	Vector3 portalDirection = portal->getDerivedDirection();
	float dotProduct = frustumToPortal.dotProduct(portalDirection);
	if ( dotProduct > 0 )
	{
		// portal is faced away from Frustum 
		bInverseDirection = true;
	}

	int addedcullingplanes = 0;
	// For portal Quads: Up to 4 planes can be added by the sides of a portal quad.
	// Each plane is created from 2 corners (world space) of the portal and the
	// frustum origin (world space).
	int i,j;
	Plane::Side pt0_side, pt1_side;
	bool visible;
	PCPlaneList::iterator pit;
	for (i=0;i<4;i++)
	{
		// first check if both corners are outside of one of the existing planes
		j = i+1;
		if (j > 3)
		{
			j = 0;
		}
		visible = true;
		pit = m_ActiveCullingPlanes.begin();
		while ( pit != m_ActiveCullingPlanes.end() )
		{
			PCPlane * plane = *pit;
			pt0_side = plane->getSide(portal->getDerivedCorner(i));
			pt1_side = plane->getSide(portal->getDerivedCorner(j));
			if (pt0_side == Plane::NEGATIVE_SIDE &&
				pt1_side == Plane::NEGATIVE_SIDE)
			{
				// the portal edge was actually completely culled by one of  culling planes
				visible = false;
			}
			pit++;
		}
		if (visible)
		{
			// add the plane created from the two portal corner points and the frustum location
			// to the  culling plane
			PCPlane * newPlane = getUnusedCullingPlane();
			if(!bInverseDirection)
				newPlane->Set(m_vOrigin, portal->getDerivedCorner(j), portal->getDerivedCorner(i));
			else
				newPlane->Set(m_vOrigin, portal->getDerivedCorner(i), portal->getDerivedCorner(j));

			newPlane->SetPortal(portal);
			m_ActiveCullingPlanes.push_front(newPlane); // tricky: we push to front to give it some priority
			addedcullingplanes++;
		}
	}
	// if we added ANY planes from the quad portal, we should add the plane of the
	// portal itself as an additional culling plane.
	if (addedcullingplanes > 0)
	{
		PCPlane * newPlane = getUnusedCullingPlane();
		if(!bInverseDirection)
			newPlane->Set(portal->getDerivedCorner(2), portal->getDerivedCorner(1), portal->getDerivedCorner(0));
		else
			newPlane->Set(portal->getDerivedCorner(2), portal->getDerivedCorner(0), portal->getDerivedCorner(1));
		newPlane->SetPortal(portal);
		m_ActiveCullingPlanes.push_front(newPlane); // tricky: we push to front to give it some priority
		addedcullingplanes++;
	}
	return addedcullingplanes;
}

void ParaEngine::CPortalFrustum::RemovePortalCullingPlanes( CPortalNode *portal )
{
	PCPlaneList::iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		if (plane->GetPortal() == portal)
		{
			// put the plane back in the reservoir
			if(m_CullingPlaneReservoir.full())
			{
				m_CullingPlaneReservoir.set_capacity(m_CullingPlaneReservoir.capacity()*2);
			}
			m_CullingPlaneReservoir.push_front(plane);
			// erase the entry from the active culling plane list
			pit = m_ActiveCullingPlanes.erase(pit);
		}
		else
		{
			pit++;
		}
	}
}

void ParaEngine::CPortalFrustum::RemoveAllExtraCullingPlanes( void )
{
	PCPlaneList::iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		// put the plane back in the reservoir
		if(m_CullingPlaneReservoir.full())
		{
			m_CullingPlaneReservoir.set_capacity(m_CullingPlaneReservoir.capacity()*2);
		}
		m_CullingPlaneReservoir.push_front(plane);
		// go to next entry
		pit++;
	}
	m_ActiveCullingPlanes.clear();
}

void ParaEngine::CPortalFrustum::setOrigin( const Vector3 & newOrigin )
{
	m_vOrigin = newOrigin;
}

bool ParaEngine::CPortalFrustum::isVisible( const CShapeAABB &bound ) const
{
	// Get centre of the box
	Vector3 centre;
	bound.GetCenter(centre);
	// Get the half-size of the box
	Vector3 halfSize;
	bound.GetExtents(halfSize);

	// For each extra active culling plane, see if the entire aabb is on the negative side
	// If so, object is not visible
	PCPlaneList::const_iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		Plane::Side xside = plane->getSide(centre, halfSize);
		if (xside == Plane::NEGATIVE_SIDE)
		{
			return false;
		}
		pit++;
	}
	return true;
}

bool ParaEngine::CPortalFrustum::isVisible( const CShapeSphere &bound ) const
{
	const Vector3 & vCenter = bound.GetCenter();
	float fRadius = bound.GetRadius();
	// For each extra active culling plane, see if the entire sphere is on the negative side
	// If so, object is not visible
	PCPlaneList::const_iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		Plane::Side xside = plane->getSide(vCenter);
		if (xside == Plane::NEGATIVE_SIDE)
		{
			float dist = plane->getDistance(vCenter);
			if (dist > fRadius)
			{
				return false;
			}
		}
		pit++;
	}
	return true;
}

/* isVisible() function for portals */
// NOTE: Everything needs to be updated spatially before this function is
//       called including portal corners, frustum planes, etc.
bool ParaEngine::CPortalFrustum::isVisible( CPortalNode * portal , bool bIgnorePortalDirection)
{
	// if portal isn't open, it's not visible
	if (!portal->IsOpen())
	{
		return false;
	}

	// if the frustum has no planes, just return true
	if (m_ActiveCullingPlanes.size() == 0)
	{
		return true;
	}
	// check if this portal is already in the list of active culling planes (avoid
	// infinite recursion case)
	PCPlaneList::const_iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		if (plane->GetPortal() == portal)
		{
			return false;
		}
		pit++;
	}
	
	// check if the portal norm is facing the frustum
	if(!bIgnorePortalDirection)
	{
		Vector3 frustumToPortal = portal->getDerivedCP() - m_vOrigin;
		Vector3 portalDirection = portal->getDerivedDirection();
		float dotProduct = frustumToPortal.dotProduct(portalDirection);
		if ( dotProduct > 0 )
		{
			// portal is faced away from Frustum 
			return false;
		}
	}
	
	// check against frustum culling planes
	bool visible_flag;

	// For each active culling plane, see if all portal points are on the negative 
	// side. If so, the portal is not visible
	pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		
		// set the visible flag to false
		visible_flag = false;
		// we have to check each corner of the portal
		for (int corner = 0; corner < 4; corner++)
		{
			Plane::Side side =plane->getSide(portal->getDerivedCorner(corner));
			if (side != Plane::NEGATIVE_SIDE)
			{
				visible_flag = true;
			}
		}
		// if the visible_flag is still false, then this plane
		// culled all the portal points
		if (visible_flag == false)
		{
			// ALL corners on negative side therefore out of view
			return false;
		}

		pit++;
	}
	// no plane culled all the portal points and the norm
	// was facing the frustum, so this portal is visible
	return true;
}

/* A 'more detailed' check for visibility of an AAB.  This function returns
none, partial, or full for visibility of the box.  This is useful for 
stuff like Octree leaf culling */
CPortalFrustum::Visibility ParaEngine::CPortalFrustum::getVisibility( const CShapeAABB & bound )
{
	// Null boxes always invisible
	if ( !bound.IsValid() )
		return NONE;

	// Get centre of the box
	Vector3 centre;
	bound.GetCenter(centre);
	// Get the half-size of the box
	Vector3 halfSize;
	bound.GetExtents(halfSize);

	bool all_inside = true;

	// For each active culling plane, see if the entire aabb is on the negative side
	// If so, object is not visible
	PCPlaneList::iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		Plane::Side xside = plane->getSide(centre, halfSize);
		if(xside == Plane::NEGATIVE_SIDE) 
		{
			return NONE;
		}
		// We can't return now as the box could be later on the negative side of a plane.
		if(xside == Plane::BOTH_SIDE) 
		{
			all_inside = false;
		}
		pit++;
	}

	if ( all_inside )
		return FULL;
	else
		return PARTIAL;
}

// get an unused PCPlane from the CullingPlane Reservoir
// note that this removes the PCPlane from the reservoir!
PCPlane * ParaEngine::CPortalFrustum::getUnusedCullingPlane( void )
{
	PCPlane * plane = 0;
	if (m_CullingPlaneReservoir.size() > 0)
	{
		PCPlaneList::iterator pit = m_CullingPlaneReservoir.begin();
		plane = *pit;
		m_CullingPlaneReservoir.erase(pit);
		return plane;
	}
	// no available planes! create one
	plane = new PCPlane;
	return plane;
}

bool ParaEngine::CPortalFrustum::CanSeeObject_PortalOnly( IViewClippingObject * pViewClippingObject ) const
{
	// if the frustum has no planes, just return true
	if (m_ActiveCullingPlanes.size() == 0)
	{
		return true;
	}
	PCPlaneList::const_iterator pit = m_ActiveCullingPlanes.begin();
	if((*pit)->GetPortal() == NULL)
		return true;

	Vector3 pVecBounds[8];
	int nCount;
	pViewClippingObject->GetRenderVertices(pVecBounds, &nCount);
	
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		if(plane->GetPortal())
		{
			int iPoint;
			for( iPoint = 0; iPoint < nCount; iPoint++ )
			{
				if( plane->getDistance(pVecBounds[iPoint]) >= 0.f)
				{
					// the point is inside the plane
					break;
				}
			}
			if( iPoint>= nCount)
				return false;
			++pit;
		}
		else
			break;
	}
	return true;
}

bool ParaEngine::CPortalFrustum::CanSeeObject( IViewClippingObject * pViewClippingObject ) const
{
	// if the frustum has no planes, just return true
	if (m_ActiveCullingPlanes.size() == 0)
	{
		return true;
	}

	Vector3 pVecBounds[8];
	int nCount;
	pViewClippingObject->GetRenderVertices(pVecBounds, &nCount);

	PCPlaneList::const_iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		PCPlane * plane = *pit;
		int iPoint;
		for( iPoint = 0; iPoint < nCount; iPoint++ )
		{
			if( plane->getDistance(pVecBounds[iPoint]) >= 0.f)
			{
				// the point is inside the plane
				break;
			}
		}
		if( iPoint>= nCount)
			return false;
		++pit;
	}
	return true;
}

bool ParaEngine::CPortalFrustum::CanSeeObject_CompleteCull( IViewClippingObject * pViewClippingObject, int nFullCullIndex) const
{
	// if the frustum has no planes, just return true
	if (m_ActiveCullingPlanes.size() == 0)
	{
		return true;
	}

	Vector3 pVecBounds[8];
	int nCount;
	pViewClippingObject->GetRenderVertices(pVecBounds, &nCount);

	int nIndex = 0;
	PCPlaneList::const_iterator pit = m_ActiveCullingPlanes.begin();
	while ( pit != m_ActiveCullingPlanes.end() )
	{
		if(nIndex < nFullCullIndex)
		{
			PCPlane * plane = *pit;
			int iPoint;
			for( iPoint = 0; iPoint < nCount; iPoint++ )
			{
				if( plane->getDistance(pVecBounds[iPoint]) >= 0.f)
				{
					// the point is inside the plane
					break;
				}
			}
			if( iPoint>= nCount)
				return false;
		}
		else
		{
			PCPlane * plane = *pit;
			int iPoint;
			for( iPoint = 0; iPoint < nCount; iPoint++ )
			{
				if( plane->getDistance(pVecBounds[iPoint]) < 0.f)
				{
					// the point is outside the plane
					return false;
				}
			}
		}
		++nIndex;
		++pit;
	}
	return true;
}

bool ParaEngine::CPortalFrustum::AddCullingPlane( const Plane& plane , bool bAddToFront)
{
	PCPlane * newPlane = getUnusedCullingPlane();
	newPlane->SetPlane(plane);
	if(m_ActiveCullingPlanes.full())
	{
		m_ActiveCullingPlanes.set_capacity(m_ActiveCullingPlanes.capacity()*2);
	}
	if(bAddToFront)
		m_ActiveCullingPlanes.push_front(newPlane);
	else
		m_ActiveCullingPlanes.push_back(newPlane);
	return true;
}
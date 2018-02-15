#pragma once
#include "ShapeAABB.h"
#include "ShapeSphere.h"
#include <boost/circular_buffer.hpp>
#include <deque>

namespace ParaEngine
{
	class CPortalNode;
	class CShapeFrustum;
	class CBaseObject;

	/** almost the same as Plane, except that it keeps a pointer to the containing portal. */
	class PCPlane : public Plane
	{
	public:
		/** Standard constructor */
		PCPlane();
		/** Alternative constructor */
		PCPlane(const Plane & plane);
		/** Alternative constructor */
		PCPlane(const Vector3& rkNormal, const Vector3& rkPoint);
		/** Alternative constructor */
		PCPlane(const Vector3& rkPoint0, const Vector3& rkPoint1, const Vector3& rkPoint2);

		/** Copy from a standard Plane */
		void SetPlane(const Plane & plane);

		/** Standard destructor */
		~PCPlane();

		/** Returns the Portal that was used to create this plane
		*/
		CPortalNode * GetPortal()
		{
			return m_portal;
		};

		/** Sets the Portal that was used to create this plane
		*/
		void SetPortal( CPortalNode *o )
		{
			m_portal = o;
		};

	protected:

		///Portal used to create this plane.
		CPortalNode *m_portal;
	};

	typedef boost::circular_buffer < PCPlane * > PCPlaneList;

	/**
	Specialized frustum shaped culling volume that has culling planes created from portals 
	This isn't really a traditional "frustum", but more a collection of	extra culling planes used by the Scene Manager 
	for supplementing the camera culling by creating extra culling planes from visible portals.  Since portals are 
	4 sided, the extra culling planes tend to form frustums (pyramids) but nothing in the code really assumes that 
	the culling planes are frustums.  They are just treated as planes. 
	*/
	class CPortalFrustum
	{
	public:
		// visibility types
		enum Visibility
		{
			NONE,
			PARTIAL,
			FULL
		};

		CPortalFrustum(void);
		~CPortalFrustum(void);
	public:
		// set the origin value
		void setOrigin(const Vector3 & newOrigin);

		/* isVisible function for aabb */
		bool isVisible( const CShapeAABB &bound) const;

		/* return true if we can see the view clipping scene object. it will use the OBB of the view clipping object. */
		bool CanSeeObject( IViewClippingObject * pViewClippingObject) const;
		/** same as above, except that it will only test again portal planes and pass all camera frustum automatically. 
		* this is used when you already know that the object is inside the camera frustum.*/
		bool CanSeeObject_PortalOnly( IViewClippingObject * pViewClippingObject) const;

		/** this will test agains a special frustum used by GetObjectsByScreenRect. It is similar to CanSeeObject, except that 
		* all culling planes after the nIndex is tested with full culling. For example, we can test if all objects are completely 
		* in front of a given set of planes. In GetObjectsByScreenRect(), the near and far planes are inserted last, so that 
		* objects must be completely inside the near and far plane in order to pass the test. 
		* @param nFullCullIndex: if -1, objects must be completely inside the culling planes*/
		bool CanSeeObject_CompleteCull( IViewClippingObject * pViewClippingObject, int nFullCullIndex=-1) const;

		/* isVisible function for sphere */
		bool isVisible( const CShapeSphere &bound) const;
		/* isVisible() function for portals 
		* @param bIgnorePortalDirection: default to true. This should be set to false, if you are testing within a zone that may contain other zones. 
		* such as the root zone. however, for all convex zones that does not contain other zones, one can set this to true. 
		*/
		bool isVisible (CPortalNode * portal, bool bIgnorePortalDirection = true);

		/* more detailed check for visibility of an AABB */
		Visibility getVisibility(const CShapeAABB & bound);

		// calculate extra culling planes from portal and camera 
		// origin and add to list of extra culling planes
		// @param bAutoDirection: it will correct portal direction, so that it does not face away from the camera eye
		int AddPortalCullingPlanes(CPortalNode * portal, bool bAutoDirection = true);

		/** add a standard culling plane 
		* @param bAddToFront: whether add to front of the queue. the front culling planes will be tested first. 
		*/
		bool AddCullingPlane(const Plane& plane, bool bAddToFront = true);

		/**
		* add 6 planes from a standard camera frustum. 
		* planes from the frustum will not be used during object culling. 
		* However, it will prevent redundant portal culling planes from being added. 
		* @param fNearPlaneShiftDistance: the near plane to be shifted when added
		*/
		int AddFrustum(CShapeFrustum * frustum, float fNearPlaneShiftDistance = 0.f);

		// remove extra culling planes created from the given portal
		void RemovePortalCullingPlanes(CPortalNode *portal);

		// remove all extra culling planes
		void RemoveAllExtraCullingPlanes(void);

		// get an unused PCPlane from the CullingPlane Reservoir
		PCPlane * getUnusedCullingPlane(void);
		
	private:
		Vector3 m_vOrigin;
		PCPlaneList m_ActiveCullingPlanes;
		PCPlaneList m_CullingPlaneReservoir;
	};
}

#pragma once
#include "BaseObject.h"
#include <list>
#include <map>

namespace ParaEngine
{
	class CBaseObject;
	class CPortalNode;
	struct SceneState;

	/**
	* a zone in portal rendering. I will assume that no zone node will contain other zone nodes. 
	* zone nodes are connected by zone portals. portals may connect 2 zones, or just a single zone with the outer world (root zone).
	* if we associate a convex shape with a zone, the scene manager can automatically determine objects in the zone. 
	*/
	class CZoneNode : public CBaseObject
	{
	public:
		CZoneNode(void);
		virtual ~CZoneNode(void);
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::ZoneNode;};

		virtual string ToString(DWORD nMethod);

		virtual DVector3 GetPosition();
		virtual void SetPosition(const DVector3 & v);

		/** assume that the object is a sphere, it will return false if the sphere is completely out of the 6 frustum planes of the camera 
		* @param fFarPlaneDistance: this value will replace the far plane. 
		*/
		virtual bool TestCollisionSphere(CBaseCamera* pCamera, float fFarPlaneDistance);
		/** check if the object's original shape can be seen via a camera. it will return false if the object is completely out of the 6 frustum planes of the camera
		*/
		virtual bool TestCollision(CBaseCamera* pCamera);

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CZoneNode;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CZoneNode"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CZoneNode, GetZonePlanes_s, const char**)	{*p1 = cls->GetZonePlanes(); return S_OK;}
		ATTRIBUTE_METHOD1(CZoneNode, SetZonePlanes_s, const char*)	{cls->SetZonePlanes(p1); return S_OK;}

		ATTRIBUTE_METHOD(CZoneNode, AutoGenZonePlanes_s)	{cls->AutoGenZonePlanes(); return S_OK;}
		ATTRIBUTE_METHOD(CZoneNode, AutoFillZone_s)	{cls->AutoFillZone(); return S_OK;}

	public:
		/** automatically generate 6 zone planes according to the bounding box of the zone node. */
		void AutoGenZonePlanes();

		/** automatically fill the zone with static mesh objects that are within the zone's convex space. it will not remove items that are already in the zone.*/
		void AutoFillZone();

		/** set or get zone planes  as a string 
		* @param sPlanes: if this is "", the zone will not be able to automatically determine which mobile objects are in it. 
		* or it can be "x1,y1,z1;x2,y2,z2;x3,y3,z3;" each three value is a point in local space denoting a plane of the bounding volume. 
		* if NULL, it will call AutoGenPortalPoints() to set the points. 
		*/
		void SetZonePlanes(const char* sPlanes);

		/** set or get zone planes as a string */
		const char* GetZonePlanes();


		/** Adds an Scene Node to this Zone.
		the Scene Manager automatically determines which objects are inside a zoom and calls this function to add a node to the zone.  
		*/
		void addNode( CBaseObject * pNode);

		/** Removes all references to a SceneNode from this Zone.
		*/
		void removeNode( CBaseObject * pNode );

		/* Add a portal to the zone
		*/
		void addPortal( CPortalNode * );

		/* Remove a portal from the zone
		*/
		void removePortal( CPortalNode * );

		/** check whether the zone contains a given object. 
		* @param pNode: the node to test, it will use the bounding box of the node. 
		* @param bStrict: if true, it will test if node's bounding box is fully inside the shape.
		* @return: false means that object is outside the zone, or the object intersects with the zone. true means object is fully contained inside the zone
		*/
		bool Contains(CBaseObject * pNode, bool bStrict);

		/** check whether the zone contains a given object.  
		* @param vPos: is a world position
		*/
		bool Contains(const Vector3& vPos);

		/** return planes of the zone. we can create from basic shapes (box, cones or raw planes) */
		vector<Plane>& GetPlanes() {return m_planes;};

		/** it only draws an arrow, when the scene's show local light parameter is true. */
		virtual HRESULT Draw( SceneState * sceneState);
	private:
		/** zones are convex mesh formed by planes. these are planes in local coordinates*/
		vector<Plane>  m_planes;
		
		/// current position in the map
		DVector3           m_vPos;
	};
}

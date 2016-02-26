#pragma once
#include <vector>
#include "TileObject.h"

namespace ParaEngine
{
	using namespace std;
	class CZoneNode;
	struct SceneState;

	/**
	* Portal rendering: portal class
	* a portal can be connected to one or two zones. 
	*/
	class CPortalNode :	public CTileObject
	{
	public:
		CPortalNode(void);
		virtual ~CPortalNode(void);

		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::PortalNode;};
		virtual string ToString(DWORD nMethod);
		

		/** assume that the object is a sphere, it will return false if the sphere is completely out of the 6 frustum planes of the camera 
		* @param fFarPlaneDistance: this value will replace the far plane. 
		*/
		virtual bool TestCollisionSphere(CBaseCamera* pCamera, float fFarPlaneDistance);
		/** check if the object's original shape can be seen via a camera. it will return false if the object is completely out of the 6 frustum planes of the camera
		*/
		virtual bool TestCollision(CBaseCamera* pCamera);

		/**
		* get all the bounding vertices in world space
		*/
		virtual void GetVertices(Vector3 * pVertices, int* nNumber);

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CPortalNode;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CPortalNode"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CPortalNode, GetHomeZone_s, const char**)	{*p1 = cls->GetHomeZone(); return S_OK;}
		ATTRIBUTE_METHOD1(CPortalNode, SetHomeZone_s, const char*)	{cls->SetHomeZone(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CPortalNode, GetTargetZone_s, const char**)	{*p1 = cls->GetTargetZone(); return S_OK;}
		ATTRIBUTE_METHOD1(CPortalNode, SetTargetZone_s, const char*)	{cls->SetTargetZone(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CPortalNode, GetPortalPoints_s, const char**)	{*p1 = cls->GetPortalPoints(); return S_OK;}
		ATTRIBUTE_METHOD1(CPortalNode, SetPortalPoints_s, const char*)	{cls->SetPortalPoints(p1); return S_OK;}

		ATTRIBUTE_METHOD(CPortalNode, AutoGenPortalPoints_s)	{cls->AutoGenPortalPoints(); return S_OK;}
	public:
		/** Get the other Zone that the Portal connects to.
		* @param pCurZone: it will return the first zone that is different from pCurZone. Default to NULL.
		*/
		CZoneNode * GetZone(CZoneNode * pCurZone = NULL);
		
		/** get the number of zones connected. It can be 0, 1 or 2. */
		int GetZoneCount();

		/* open the portal */
		void Open();
		/* close the portal */
		void Close();
		/* check if portal is open */
		bool IsOpen() {return m_bOpen;}

		/** Set the coordinates of one of the portal corners in local space
		*/
		void SetCorner( int nIndex, const Vector3& vPos);

		/** Get the coordinates of one of the portal corners in local space
		*/
		Vector3 & getCorner( int nIndex);

		/** Get the Derived (world render coordinates) of one of the portal corners, assume that they are up to date. 
		*/
		Vector3 & getDerivedCorner( int nIndex);

		/** Get the direction of the portal in world coordinates
		*/
		Vector3 & getDerivedDirection( void );

		/** Get the position (centerpoint) of the portal in world coordinates
		*/
		Vector3 & getDerivedCP( void );

		/** Update the derived values
		*/
		void updateDerivedValues();


		/* Calculate the local direction and radius of the portal
		*/
		void calcDirectionAndRadius( void );

		/** it only draws an arrow, when the scene's show local light parameter is true. */
		virtual HRESULT Draw( SceneState * sceneState);

		/** this function is called to update the render frame number of this object. 
		* please note that, the frame number increases by 1 every frame. This function is called at the earliest scene culling stage. 
		* the object may be culled in later rendering stages. so one should not do much computation in this place. 
		* in most cases, we only need to cache some derived data such as render coordinates for use in later rendering processes. 
		*/
		virtual void UpdateFrameNumber(int nFrameNumber);

		/** set the home zone name. if the zone does not exist, it will be created. */
		void SetHomeZone(const char* sName);
		const char* GetHomeZone();

		/** set the target zone name. if the zone does not exist, it will be created. */
		void SetTargetZone(const char* sName);
		const char* GetTargetZone();

		/** set or get portal point as a string 
		* @param sVertices: it is coordinates of the 4 quad vertices, "x1,y1,z1;x2,y2,z2;x3,y3,z3;x4,y4,z4;". if NULL, it will call AutoGenPortalPoints() to set the points. 
		*/
		void SetPortalPoints(const char* sVertices);
		/** set or get portal point as a string */
		const char* GetPortalPoints();

		/** automatically generate 4 portal points according to the bounding box of the portal node. */
		void AutoGenPortalPoints();
	private:
		// flag open or closed
		bool m_bOpen;

		/// flag indicating whether or not local values are up-to-date
		bool m_LocalsUpToDate;

		/// Corners of the quad portal - coordinates are relative to the scene Node. 
		/// NOTE: there are 4 corners if the portal is a quad type
		Vector3 m_Corners[4];
		/// Direction ("Norm") of the portal - 
		// NOTE: For a Quad portal, determined by the 1st 3 corners.
		// NOTE: For AABB & SPHERE portals, we only have "inward" or "outward" cases.
		//       To indicate "outward", the Direction is UNIT_Z
		//		 to indicate "inward", the Direction is NEGATIVE_UNIT_Z
		Vector3 m_Direction;
		// Local Center point of the portal
		Vector3 m_LocalCP;

		////////////////////////////////////////////////////////////
		// cached per frame data to speed up calculation
		/// world render coordinates Corners of the portal. it is updated and cached each frame. 
		Vector3 m_DerivedCorners[4];
		/// Derived (world coordinates) direction of the portal
		// NOTE: Only applicable for a Quad portal
		Vector3 m_DerivedDirection;
		/// Derived (world coordinates) of portal (center point)
		Vector3 m_DerivedCP;

		
	};
}


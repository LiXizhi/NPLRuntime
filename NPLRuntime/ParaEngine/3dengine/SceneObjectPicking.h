#pragma once

#include "SceneObject.h"

namespace ParaEngine
{
	/** any mesh object in the scene. Usually for selection during scene editing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingMesh;
	/** mesh object that is clickable (associated with scripts). Usually for game playing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingNPCMesh;
	/** biped object.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingBiped;
	/** any object in the scene except for the current player. Usually for selection during scene editing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingNotPlayer;
	/** any object in the scene except. Usually for selection during scene editing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingAll;
	/** only light object in the scene except. Usually for selection during scene editing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingLight;
	/** all global objects, such as global character and mesh. This is usually for game mode.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingGlobal;
	/** any objects, including mesh and characters. but not including helper objects, such as light.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingAnyObject;
	/** any mesh or physical mesh that contains action XRef scripts.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingActionMesh;
	/** pick objects according to the filter in the last call of SetPickingFilter() function .*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingByObjectTypes;
	/** pick objects according to the physics group in last call of SetPickingPhysicsFilter() function.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingByPhysicsGroup;

	/** Set the picking filter to be used with PickingByObjectTypes function filter
	* @param filter: bitwise fields of _SceneObjectType, such as CBaseObject::ZoneNode | CBaseObject::PortalNode
	*/
	extern void SetPickingFilter(DWORD filter);

	/** Set the picking filter to be used with PickingByPhysicsGroup function filter
	* @param filter: bitwise fields of group 0-31. so that 0xffffffff means matches any physics group. 
	*/
	extern void SetPickingPhysicsFilter(DWORD filter);

	/** it stands for an object intersecting with a ray. it is used for ray picking. */
	struct PickedObject
	{
		/// approximated distance
		float m_fRayDist;
		/// the smallest value of the object's bounding box's extents
		float m_fMinObjExtent;

		CBaseObject* m_pObj;
	public:
		PickedObject(float fRayDist, float fMinObjExtent,  CBaseObject* pObj)
			:m_fRayDist(fRayDist), m_fMinObjExtent(fMinObjExtent), m_pObj(pObj){}
			PickedObject()
				:m_fRayDist(0), m_fMinObjExtent(0), m_pObj(NULL){}
				bool IsValid(){return m_pObj!=NULL;};
	};
}
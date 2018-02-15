#pragma once
#include "BaseObject.h"

namespace ParaEngine
{
	class CShapeOBB;
	class CShapeAABB;
	
	/**
	* It is used for object-level view culling.
	* A view clipping object is an AABB (axis aligned bounding box) which could transform itself according to 
	* a local transform matrix, a global position and a facing value, so that the transformed object 
	* can be used for efficient object-level view culling in ParaEngine.
	* Currently the MeshEntity and mesh object uses this class as their default view culling object. 
	*/
	class CViewCullingObject :	public IViewClippingObject
	{
	public:
		CViewCullingObject(void);
		virtual ~CViewCullingObject(void);
	private:
	
		/// AABB info
		Vector3            m_vMin;
		Vector3            m_vMax;
	
		/// AABB which is transformed
		CShapeAABB*			m_pShapeAABB;

		/// current position in the map
		DVector3           m_vPos;

		/// the OBB shape in world coordinate
		CShapeOBB*			m_pShapeOBB;
		/** the matrix that could transform AABB to its global position in the 3D world */
		Matrix4 m_mWorldTransform;

		/// radius
		FLOAT					m_fRadius;
		float					m_bound_fY;
	public:
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::ViewCullingObject;};

		virtual float GetDepth(){return m_bound_fY;};
		virtual float GetWidth(){return m_bound_fY;};
		virtual FLOAT GetHeight(){return m_bound_fY;};

		/**
		* Set the object shape to Sphere and sphere parameters
		* @param fRadius: radius
		*/
		virtual void SetRadius(FLOAT fRadius);
	
		/**
		* @see SetRadius(FLOAT fRadius);
		*/
		virtual FLOAT GetRadius(){return m_fRadius;};

		virtual DVector3 GetPosition();
	
		/** Set AABB information. It is possibly attained from a mesh entity. */
		virtual void SetAABB(const Vector3 *vMin, const Vector3 *vMax);

		/** get the oriented bounding box in world space. One may need to shift by the rendering origin if used on that purposes.
		* @param obb: [out]
		*/
		virtual void GetOBB(CShapeOBB* obb);
		virtual void GetAABB(CShapeAABB* aabb);

		/**
		* Set a local transform matrix, a global position and a facing value, according to which the object will be transformed
		* on demand, so that the transformed object can be used for efficient object-level view culling in ParaEngine.
		* Please note, that the input pointers must be valid until the ClearTransform is called
		*/
		virtual void SetTransform(Matrix4* pLocalTransform, const DVector3& pGlobalPos, float* pRotation);

		/**
		* this function is called by the game engine for rough object level clipped.
		* We assume that the view culling object is a sphere.
		*/
		virtual void GetRenderVertices(Vector3 * pVertices, int* nNumber);

		/** get world transform */
		virtual Matrix4* GetWorldTransform(Matrix4& pOut, int nRenderNumber = 0);
	private: 
		/** update the OBB according to current transform and location. */
		void UpdateOBB();
		void UpdateAABB();
	};
}
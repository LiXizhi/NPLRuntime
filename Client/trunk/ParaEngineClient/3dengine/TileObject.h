#pragma once
#include "BaseObject.h"
#include "math/ShapeAABB.h"

namespace ParaEngine
{
	/** Tile Object have position and bounding rect and can usually be attached to quad-tree terrain tile.
	* Most scene object like MeshObject, BipedObject derives from this class. 
	*/
	class CTileObject : public CBaseObject
	{
	public:
		CTileObject();
		virtual ~CTileObject();
		ATTRIBUTE_DEFINE_CLASS(CTileObject);

		/** if true, we will attach this object to quad-tree terrain tile according to its attributes when adding to the scene
		* if false, we will automatically attach it to a container object named after its class name.
		*/
		virtual bool IsTileObject() { return true; };

		virtual DVector3 GetPosition();
		virtual void SetPosition(const DVector3& v);

		/**
		* Set the object shape to box and bounding box parameters
		* if fFacing is 0, it is a axis aligned bounding box (AABB). The bounding sphere is set to
		* radius of bounding box's the smallest spherical container
		* @param fOBB_X: object bounding box.x
		* @param fOBB_Y: object bounding box.y
		* @param fOBB_Z: object bounding box.z
		* @param fFacing: rotation of the bounding box around the y axis.
		*/
		virtual void SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing);

		/**
		* @see SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing);
		*/
		virtual void GetBoundingBox(float* fOBB_X, float* fOBB_Y, float* fOBB_Z, float* fFacing);

		/**
		* Set the object shape to rect (a plane) and rect parameters
		* if fFacing is 0, it is a axis aligned bounding box (AABB).
		* @param fWidth: plane width or x
		* @param fHeight: plane height or y.
		* @param fFacing: rotation of the plane around the y axis.
		*/
		virtual void SetBoundRect(float fWidth, float fHeight, float fFacing);
		/**
		* @see SetBoundRect(float fWidth ,float fHeight, float fFacing);
		*/
		virtual void GetBoundRect(float* fWidth, float* fHeight, float* fFacing);

		/**
		* Set the object shape to Sphere and sphere parameters
		* @param fRadius: radius
		*/
		virtual void SetRadius(float fRadius);

		/**
		* @see SetRadius(float fRadius);
		*/
		virtual float GetRadius();


		/** get object Yaw: is facing the positive x axis, if yaw is 0.
		*/
		virtual float GetYaw();
		virtual void SetYaw(float fValue);
		

		/** get object height y axis: only for object that has a Height property, usually is height of the bounding box */
		virtual float GetHeight();
		virtual void SetHeight(float fHeight);

		/** get object width: x axis */
		virtual float GetWidth();
		virtual void SetWidth(float fWidth);

		/** get object depth: z axis */
		virtual float GetDepth();
		virtual void SetDepth(float fDepth);

		/** Set AABB information. It is possibly attained from a mesh entity. */
		virtual void SetAABB(const Vector3 *vMin, const Vector3 *vMax);

		virtual Vector3 GetLocalAABBCenter();

		virtual void GetOBB(CShapeOBB* obb);
		virtual void GetAABB(CShapeAABB* aabb);

		/**
		* get the center of the object in world space
		*/
		virtual DVector3 GetObjectCenter();
		/**
		* set the center of the object in world space
		*/
		virtual void SetObjectCenter(const DVector3 & v);

		virtual void GetVerticesWithOrigin(const Vector3* vOrigin, Vector3 * pVertices, int* nNumber);
	protected:
		/** position is always at the bottom center of the bounding shape. */
		DVector3 m_vPos;

		/** bounding box of containing object. This is the local aabb, local center position is usually at (0, height/2, 0). */
		CShapeAABB m_aabb;

		/** yaw or facing */
		float m_fYaw;

		/** radius of m_aabb */
		float m_fRadius;
	};
}



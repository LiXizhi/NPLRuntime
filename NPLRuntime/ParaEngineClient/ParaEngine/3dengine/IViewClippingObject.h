#pragma once
#include "IRefObject.h"

namespace ParaEngine
{
	class IBatchedElementDraw;
	class CShapeOBB;
	class CShapeAABB;
	class CBaseCamera;
	struct SceneState;
	
	/** object shape */
	enum ObjectShape
	{
		_ObjectShape_Circle = 0,
		_ObjectShape_Sphere,
		_ObjectShape_Rectangular,
		_ObjectShape_Box,
		_ObjectShape_NX_dynamic, /// the shape of the object is associated with a dynamic actor defined in Novodex physics engine
		_ObjectShape_NX_static,	/// the shape of the object is associated with a static actor defined in Novodex physics engine
		_ObjectShape_NX_kinematicBox,	/// the shape of the object is associated with a kinematic box shaped actor defined in Novodex physics engine
	};

	struct AssetEntity;

	/** Pure interface for CBaseObject (3D scene object)
	* It defines basic shapes and collision detection. 
	*/
	class IViewClippingObject : public IRefObject
	{
	public:
		virtual ~IViewClippingObject(){};

		/** get the object shape */
		virtual ObjectShape GetObjectShape(){return _ObjectShape_Box;};

		/** set the object shape */
		virtual void SetObjectShape(ObjectShape shape){};

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
		virtual void SetBoundRect(float fWidth ,float fHeight, float fFacing){};
		/**
		* @see SetBoundRect(float fWidth ,float fHeight, float fFacing);
		*/
		virtual void GetBoundRect(float* fWidth ,float* fHeight, float* fFacing);

		/**
		* Set the object shape to Sphere and sphere parameters
		* @param fRadius: radius
		*/
		virtual void SetRadius(float fRadius){};
	
		/**
		* @see SetRadius(float fRadius);
		*/
		virtual float GetRadius(){return 0.f;};
	
		/**
		* get all the vertices which can represent the object's shape. This is used for view clipping.
		* for sphere shaped object:
		*	8 vertices of a axis aligned bounding box of the sphere is returned
		* for box shaped object:
		*	The eight points of the bounding box is returned.
		* for rect shaped object:
		*	The 4 points of the bounding box is returned.
		* for circle shaped object:
		*	The 4 points of the bounding box of the circle is returned.
		* @params pVertices: out parameter for storing the vertices, must be at least Vector3[8]
		* @params nNumber: out parameter storing the number of vertices returned
		*/
		virtual void GetVertices(Vector3 * pVertices, int* nNumber);

		/** same as GetVertices(). Except that the vertices returned are for rendering, not for physics.*/
		virtual void GetRenderVertices(Vector3 * pVertices, int* nNumber);

		virtual void GetVerticesWithOrigin( const Vector3* vOrigin, Vector3 * pVertices, int* nNumber);
		/** 
		* get object position in the world space. The position of an object is the bottom center of the object. 
		* most scene object will return the position using the following rules
		*  project the center of the shape to the bottom plane of the shape, the
		*  projection point at the bottom is returned as the object's world position.
		* so it is different from the center of the object. 
		* @see GetObjectCenter(Vector3 *pV)
		*/
		virtual DVector3 GetPosition();
		
		/** set the object position. */
		virtual void SetPosition(const DVector3& v);

		/** Rotate the object.This only takes effects on objects having 3D orientation, such as
		* static mesh and physics mesh. The orientation is computed in the following way: first rotate around x axis, 
		* then around y, finally z axis.
		* @param x: rotation around the x axis.
		* @param y: rotation around the y axis.
		* @param z: rotation around the z axis.
		*/
		virtual void Rotate(float x, float y, float z);

		/** set rotation using a quaternion. */
		virtual void SetRotation(const Quaternion& quat);
		/** get rotation using a quaternion */
		virtual void GetRotation(Quaternion* quat);

		/** set the scale of the object. This function takes effects on both character object and mesh object. 
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);

		/** get the scaling. */
		virtual float GetScaling();

		/** get the offset(translation) of the object used for rendering.
		* the rendering offset will translate the object from the world coordinate system to 
		* the scene's rendering coordinate system.This will make the components of world transform 
		* matrix consistent.(i.e. of the same magnitude.) hence it will correct floating point 
		* calculation imprecisions, due to large object coordinates, such as (20000.01).
		* Notes: both the render function and the camera will use the render offset, instead of 
		* GetPosition() to get the position of the render object. This function will internally call
		* the virtual function GetPosition().
		* @return: the offset returned.
		* @see: GetRenderOrigin().*/
		virtual Vector3 GetRenderOffset();

		/**
		* get the center of the object in world space
		*/
		virtual DVector3 GetObjectCenter();
		/**
		* set the center of the object in world space
		*/
		virtual void SetObjectCenter(const DVector3 & v);

		/** get object facing: only for object that has a facing property like biped, not bound facing */
		virtual void GetFacing3D( Vector3 *pV );

		/** same as GetYaw and SetYaw
		* get object facing: only for object that has a facing property like biped, not bound facing
		* The object is facing the positive x axis, if facing is 0.
		*/
		inline float GetFacing() { return GetYaw(); };
		inline void SetFacing(float fFacing) { SetYaw(fFacing); };

		/** get object Yaw: is facing the positive x axis, if yaw is 0.
		*/
		virtual float GetYaw(){ return 0.f; };
		virtual void SetYaw(float fFacing){};

		/** get object pitch: is facing the positive x axis, if yaw is 0.
		*/
		virtual float GetPitch(){ return 0.f; };
		virtual void SetPitch(float fFacing){};

		/** get object roll: is facing the positive x axis, if yaw is 0.
		*/
		virtual float GetRoll(){ return 0.f; };
		virtual void SetRoll(float fFacing){};


		/** get object height y axis: only for object that has a Height property, usually is height of the bounding box */
		virtual float GetHeight(){return 0.f;};
		virtual void SetHeight(float fHeight){};

		/** get object width: x axis */
		virtual float GetWidth(){return 0.f;};
		virtual void SetWidth(float fWidth){};

		/** get object depth: z axis */
		virtual float GetDepth(){return 0.f;};
		virtual void SetDepth(float fDepth){};
	
		/* return the normal at the terrain position where the object is situated at its x,z plane.*/
		virtual Vector3 GetNormal(){return Vector3(0,1.f,0);};
		virtual void SetNormal(const Vector3 & pNorm){};

		/** get the oriented bounding box in world space. One may need to shift by the rendering origin if used on that purposes.
		* @param obb: [out]
		*/
		virtual void GetOBB(CShapeOBB* obb);
		virtual void GetAABB(CShapeAABB* aabb);

		
		/** Set local AABB information. It is possibly attained from a mesh entity. */
		virtual void SetAABB(const Vector3 *vMin, const Vector3 *vMax);
		
		virtual Vector3 GetLocalAABBCenter();

		/**
		* Set a local transform matrix, a global position and a facing value, according to which the object will be transformed
		* on demand, so that the transformed object can be used for efficient object-level view culling in ParaEngine.
		* Please note, that the input pointers must be valid until the ClearTransform is called
		*/
		virtual void SetTransform(Matrix4* pLocalTransform, const DVector3& pGlobalPos, float* pRotation);
		virtual void SetTransform(Matrix4* pWorldTransform);

		/**
		* return the world matrix of the object for rendering
		* @param out: the output.
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return: same as out. or NULL if not exists.
		*/
		virtual Matrix4* GetRenderMatrix(Matrix4& out, int nRenderNumber = 0);

		/** get world transform */
		virtual Matrix4* GetWorldTransform(Matrix4& pOut, int nRenderNumber = 0);

		/// -- collision detection 
		/** 
		* test if the sphere collides with this object in the world coordinate.
		* this function is called by the game engine for object level clipping. The more accurate method will be slower.
		* We assume that the view culling object is a box transformed from the AABB with the temporary transform information.
		* @param pvCenter: center of the sphere to test against
		* @param radius: the radius of the sphere. It can be zero.
		* @param nMethod: whether this object will be treated as a 2D cylinder, a 3D sphere or the original shape. 
		*	- nMethod=0: The object is treated as its original shape.This may be slow.
		*	- nMethod=1: both the incoming sphere and the object are treated as 2D circles. This is the fastest method, and is
		*		useful for rough object-level culling, even for 3D scenes;
		*	- nMethod=2: both the incoming sphere and the object are treated as 3D sphere. This is a all very method, and is
		*		useful for rough object-level culling for 3D scenes;
		*	- nMethod=3: reserved
		*/
		virtual bool TestCollisionSphere(const Vector3* pvCenter, float radius, BYTE nMethod = 1);
		/** assume that the object is a sphere, it will return false if the sphere is completely out of the 6 frustum planes of the camera 
		* @param fFarPlaneDistance: this value will replace the far plane. 
		*/
		virtual bool TestCollisionSphere(CBaseCamera* pCamera, float fFarPlaneDistance);

		/** assume that the object is a sphere, it will return false if the swept sphere along the given direction is completely out of the 6 frustum planes of the camera 
		this function is usually used to test whether we need to render this object as shadow caster, where pvDirection is the current sun direction. 
		*/
		virtual bool TestShadowSweptSphere(CBaseCamera* pCamera, const Vector3* pvDirection);

		/** check if the object's original shape can be seen via a camera. it will return false if the object is completely out of the 6 frustum planes of the camera
		*/
		virtual bool TestCollision(CBaseCamera* pCamera);

		virtual bool TestCollisionObject(IViewClippingObject* pObj);
		virtual bool TestCollisionRay(const Vector3& vPickRayOrig, const Vector3& vPickRayDir, float* fDistance);
		virtual float GetSphereCollisionDepth(Vector3* pvCenter, float radius, bool bSolveDepth = false);
		virtual float GetObjectCollisionDepth(IViewClippingObject* pObj);
	
		/** get the distance from the object's surface to a given point. 
		* the object is always treated as a sphere. If the point is not inside the object, a positive value is always returned.
		* otherwise, a negative value will be returned. */
		virtual float GetObjectToPointDistance(const Vector3* pPoint);

		/** debugging only: draw bounding box */
		virtual void	DrawBoundingBox(SceneState * sceneState, DWORD color);

		/// draw the occlusion object
		virtual void	DrawOcclusionObject(SceneState * sceneState);

		/** this function is called whenever debugging drawing is enabled. */
		virtual void DebugDraw(IBatchedElementDraw* pDebugDraw);
	};
}
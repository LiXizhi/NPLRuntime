#pragma once

#include "PEtypes.h"
#include <functional>

namespace ParaEngine {

	struct IParaPhysicsRigidbody;
	struct IParaPhysicsConstraint;

	struct RayCastHitResult
	{
		float m_fDistance;
		PARAVECTOR3 m_vHitPointWorld;
		PARAVECTOR3 m_vHitNormalWorld;
	};

	struct IParaPhysicsWorld
	{
		/// return pointer to the low level physics engine world object. 
		virtual void* get() = 0;

		// Pointer to this class will be invalid after the call
		virtual void Release() = 0;

		/** draw the entire physics debug world.*/
		virtual void DebugDrawWorld() = 0;

		/** step simulation
		* @param fDeltaTime: in seconds.
		*/
		virtual bool StepSimulation(float fDeltaTime, int maxSubSteps = 1, float fixedTimeStep = 1.0f / 60.0f) = 0;

		virtual void SetGravity(const PARAVECTOR3 &gravity) = 0;
		virtual void GetGravity(PARAVECTOR3& gravity) const = 0;

		virtual void AddRigidBody(IParaPhysicsRigidbody *body) = 0;
		virtual void AddRigidBody(IParaPhysicsRigidbody *body, int group, int mask) = 0;
		virtual void RemoveRigidBody(IParaPhysicsRigidbody *body) = 0;

		virtual int GetNumBodies() const = 0;
		virtual IParaPhysicsRigidbody* GetBodyByIndex(int index) = 0;

		virtual IParaPhysicsRigidbody* RaycastClosestShape(const PARAVECTOR3& vOrigin
				, const PARAVECTOR3& vDirection
				, DWORD dwType
				, RayCastHitResult& hit
				, int dwGroupMask
				, float fSensorRange) = 0;


		virtual void AddConstraint(IParaPhysicsConstraint *constraint, bool disableCollisionsBetweenLinkedBodies = false) = 0;
		virtual void RemoveConstraint(IParaPhysicsConstraint *constraint) = 0;
		virtual int GetNumConstraints() const = 0;
		virtual IParaPhysicsConstraint * GetConstraint(int index) = 0;
	};

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct IParaPhysicsShape
	{
		/// get user data associated with the shape
		virtual void* GetUserData() = 0;
		virtual void SetUserData(void* pData) = 0;

		/// return pointer to the low level physics engine shape object. 
		virtual void* get() = 0;

		// Pointer to this class will be invalid after the call
		virtual void Release() = 0;



		virtual void SetLocalScaling(const PARAVECTOR3& scaling) = 0;
		virtual void GetLocalScaling(PARAVECTOR3& scaling) = 0;
		virtual void CalculateLocalInertia(float mass, PARAVECTOR3& inertia) = 0;
	};

	struct IParaPhysicsCompoundShape : public IParaPhysicsShape
	{
		/* add */
		virtual void AddChildShape(const PARAMATRIX3x3& rotation
			, const PARAVECTOR3& origin
			, IParaPhysicsShape* pShape) = 0;

		/* remove */
		virtual void RemoveChildShape(IParaPhysicsShape* pShape) = 0;

		/* remove by index */
		virtual void RemoveChildByIndex(int index) = 0;

		/* get num of child */
		virtual int GetNumChild() = 0;

		/* get child by index */
		virtual IParaPhysicsShape* GetChildShape(int index) = 0;

		/* get transform of child by index */
		virtual void GetChildTransform(int index, PARAMATRIX3x3& rotation, PARAVECTOR3& origin) = 0;

		/* set a new transform for a child, and update internal data structures */
		virtual void UpdateChildTransform(int index
			, const PARAMATRIX3x3& rotation
			, const PARAVECTOR3& origin
			, bool shouldRecalculateLocalAabb) = 0;
	};

	struct IParaPhysicsTriangleMeshShape : public IParaPhysicsShape
	{

	};

	struct IParaPhysicsScalingTriangleMeshShape : public IParaPhysicsShape
	{
		virtual IParaPhysicsTriangleMeshShape* getChildShape() = 0;
	};


	struct IParaPhysicsBody
	{
		/// return pointer to the low level physics engine body object. 
		virtual void* get() = 0;

		// Pointer to this class will be invalid after the call
		virtual void Release() = 0;

		/// get user data associated with the body
		virtual void* GetUserData() = 0;
		virtual void SetUserData(void* pData) = 0;

		virtual int GetInternalType() const = 0;

		// don't remove any body in this callback, because it will be called any times in one tick
		typedef std::function<void(IParaPhysicsBody* bodyA, IParaPhysicsBody* bodyB, float dis, const PARAVECTOR3& posA, const PARAVECTOR3& posB, const PARAVECTOR3& normalOnB)> ContactCallBack;
		virtual void SetContactCallBack(const ContactCallBack& cb) = 0;
		virtual void OnContact(IParaPhysicsBody* bodyA, IParaPhysicsBody* bodyB, float dis, const PARAVECTOR3& posA, const PARAVECTOR3& posB, const PARAVECTOR3& normalOnB) = 0;
	};

	struct IParaPhysicsRigidbody : public IParaPhysicsBody
	{

		// is this rigidbody added to physics world?
		virtual bool isInWorld() const = 0;

		// convert this body to kinematic body
		virtual void Convert2Kinematic() = 0;
	};

	struct ParaPhysicsMotionStateDesc
	{
		// synchronizes world transform from physics to user (input param)
		// physics engine only calls the update of worldtransform for active objects
		typedef std::function<void(const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin)> UpdateTransformCallBack;
		UpdateTransformCallBack cb;

		// synchronizes world transform from user to physics (output param)
		typedef std::function<void(const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin)> SetTransformFunction;
		SetTransformFunction setTransform;

	};

	/* Create descriptor for a rigid body. so that we can create it. */
	struct ParaPhysicsRigidbodyDesc
	{
		// Local Inertia, this is optional
		PARAVECTOR3* m_pInertia;
		/// if mass is zero, it is a static mesh.
		float m_mass;
		///Storage for the translation
		PARAVECTOR3 m_origin;
		///Storage for the rotation
		PARAMATRIX3x3 m_rotation;

		struct CenterOfMassOffset
		{
			CenterOfMassOffset(const PARAVECTOR3& origin, const PARAMATRIX3x3& rotation)
				: m_offset_origin(origin)
				, m_offset_rotation(rotation)
			{
			}

			PARAVECTOR3 m_offset_origin;
			PARAMATRIX3x3 m_offset_rotation;
		};

		// set center of mass offset , this is optional
		CenterOfMassOffset* m_pCenterOfMassOffset;
		
		// group id
		short m_group;
		// collision mask id,  -1 to collide with all meshes.
		int m_mask;
		// the collision shape of the rigid body. it should be the pointer returned by one of the CreateXXXShape() functions. 
		IParaPhysicsShape* m_pShape;

		void setLocalInertia(const PARAVECTOR3& inertia)
		{
			m_pInertia = new PARAVECTOR3(inertia);
		}

		void setCenterOfMassOffset(const PARAVECTOR3& origin, const PARAMATRIX3x3& rotation)
		{
			m_pCenterOfMassOffset = new CenterOfMassOffset(origin, rotation);
		}

		ParaPhysicsRigidbodyDesc()
			: m_mass(0.f)
			, m_group(1)
			, m_mask(-1)
			, m_pShape(nullptr)
			, m_pInertia(nullptr)
			, m_pCenterOfMassOffset(nullptr)
		{
			SAFE_DELETE(m_pInertia);
			SAFE_DELETE(m_pCenterOfMassOffset);
		}
	};

	/** Create descriptor for triangle mesh shape. so that we can create it.
	*/
	struct ParaPhysicsTriangleMeshDesc
	{
		DWORD m_numVertices;
		DWORD m_numTriangles;
		DWORD m_pointStrideBytes;
		DWORD m_triangleStrideBytes;
		void* m_points;
		void* m_triangles;
		DWORD m_flags;
	};

	struct IParaPhysicsConstraint
	{
		/// return pointer to the low level physics engine constraint object. 
		virtual void* get() = 0;

		// Pointer to this class will be invalid after the call
		virtual void Release() = 0;

		/// get user data associated with the constraint
		virtual void* GetUserData() = 0;
		virtual void SetUserData(void* pData) = 0;

		virtual bool isEnabled() const = 0;
		virtual void setEnabled(bool enabled) = 0;
	};

	struct IParaPhysicsPoint2PointConstraint : public IParaPhysicsConstraint
	{

	};

	struct IParaPhysicsHingeConstraint : public IParaPhysicsConstraint
	{

	};

	struct IParaPhysicsSliderConstraint : public IParaPhysicsConstraint
	{

	};

	struct IParaPhysicsConeTwistConstraint : public IParaPhysicsConstraint
	{

	};

	struct IParaPhysicsGeneric6DofSpringConstraint : public IParaPhysicsConstraint
	{

	};

	
}

#pragma once

#include "btBulletDynamicsCommon.h"
#include "Physics/IParaPhysics.h"

#include <set>
#include <list>

#define CONVERT_PARAVECTOR3(_vector) (PARAVECTOR3((float)(_vector).getX(), (float)(_vector).getY(), (float)(_vector).getZ()))

#define CONVERT_BTVECTOR3(_vector) (btVector3(btScalar((_vector).x), btScalar((_vector).y), btScalar((_vector).z)))

#define CONVERT_TO_TRANSFORM(_matrix, _vector) \
btTransform(btMatrix3x3(\
		btScalar((_matrix)._11), btScalar((_matrix)._21), btScalar((_matrix)._31),\
		btScalar((_matrix)._12), btScalar((_matrix)._22), btScalar((_matrix)._32),\
		btScalar((_matrix)._13), btScalar((_matrix)._23), btScalar((_matrix)._33)\
	)\
, btVector3(\
		btScalar((_vector).x), btScalar((_vector).y), btScalar((_vector).z)\
))

/*
matRot = btMatrix3x3(
	btScalar(offset.m_offset_rotation._11), btScalar(offset.m_offset_rotation._21), btScalar(offset.m_offset_rotation._31),
	btScalar(offset.m_offset_rotation._12), btScalar(offset.m_offset_rotation._22), btScalar(offset.m_offset_rotation._32),
	btScalar(offset.m_offset_rotation._13), btScalar(offset.m_offset_rotation._23), btScalar(offset.m_offset_rotation._33)
);

vOrigin = btVector3(btScalar(offset.m_offset_origin.x), btScalar(offset.m_offset_origin.y), btScalar(offset.m_offset_origin.z));
centerOfMassOffset = btTransform(matRot, vOrigin);
*/


namespace ParaEngine {

	struct BulletWorld : public IParaPhysicsWorld
	{
		BulletWorld() ;
		virtual ~BulletWorld();

		virtual void* get() { return m_pWorld; };
		virtual void Release() { delete this; };

		virtual void DebugDrawWorld();

		/** step simulation
		* @param fDeltaTime: in seconds.
		*/
		virtual bool StepSimulation(float fDeltaTime, int maxSubSteps = 1, float fixedTimeStep = 1.0f / 60.0f);

		virtual void SetGravity(const PARAVECTOR3 &gravity);
		virtual void GetGravity(PARAVECTOR3& gravity) const;

		virtual void AddRigidBody(IParaPhysicsRigidbody *body);
		virtual void AddRigidBody(IParaPhysicsRigidbody *body, int group, int mask);
		virtual void RemoveRigidBody(IParaPhysicsRigidbody *body);

		virtual int GetNumBodies() const;
		virtual IParaPhysicsRigidbody* GetBodyByIndex(int index);

		virtual IParaPhysicsRigidbody* RaycastClosestShape(const PARAVECTOR3& vOrigin
				, const PARAVECTOR3& vDirection
				, DWORD dwType
				, RayCastHitResult& hit
				, int dwGroupMask
				, float fSensorRange);

		virtual void AddConstraint(IParaPhysicsConstraint *constraint, bool disableCollisionsBetweenLinkedBodies = false);
		virtual void RemoveConstraint(IParaPhysicsConstraint *constraint);
		virtual int GetNumConstraints() const;
		virtual IParaPhysicsConstraint * GetConstraint(int index);

		btDiscreteDynamicsWorld* m_pWorld;
		btBroadphaseInterface*	m_broadphase;
		btCollisionDispatcher*	m_dispatcher;
		btConstraintSolver*	m_solver;
		btDefaultCollisionConfiguration* m_collisionConfiguration;
	};


	ATTRIBUTE_ALIGNED16(struct) ParaPhysicsMotionState : public btMotionState
	{
		btTransform m_graphicsWorldTrans;
		btTransform	m_centerOfMassOffset;
		btTransform m_startWorldTrans;

		ParaPhysicsMotionStateDesc::UpdateTransformCallBack m_cb;

		BT_DECLARE_ALIGNED_ALLOCATOR();


		ParaPhysicsMotionState(const btTransform& startTrans = btTransform::getIdentity()
			, const btTransform& centerOfMassOffset = btTransform::getIdentity()
			, ParaPhysicsMotionStateDesc::UpdateTransformCallBack cb = nullptr);

		/// synchronizes world transform from user to physics
		/// this func only Bullet will call 
		virtual void getWorldTransform(btTransform& centerOfMassWorldTrans) const;

		///synchronizes world transform from physics to user
		///Bullet only calls the update of worldtransform for active objects
		virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans);

		// synchronizes world transform from user to physics
		void setWorldTransform(const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin);
	};
}

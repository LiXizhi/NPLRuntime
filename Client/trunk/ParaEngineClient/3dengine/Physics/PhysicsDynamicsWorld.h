#pragma once

#include "Physics/PhysicsObject.h"

namespace ParaEngine {
	class CPhysicsShape;
	class CPhysicsRigidBody;
	class CPhysicsConstraint;

	class CPhysicsDynamicsWorld : public CPhysicsObject
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsDynamicsWorld(IParaPhysicsWorld* pWorld);
	public:
		virtual ~CPhysicsDynamicsWorld();

		IParaPhysicsWorld* get();

		ATTRIBUTE_DEFINE_CLASS(CPhysicsDynamicsWorld);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CPhysicsDynamicsWorld, GetGravity_s, Vector3*) { cls->GetGravity(*((PARAVECTOR3*)p1)); return S_OK; }
		ATTRIBUTE_METHOD1(CPhysicsDynamicsWorld, SetGravity_s, Vector3) { cls->SetGravity(*((PARAVECTOR3*)&p1)); return S_OK; }

		/** step simulation
		* @param fDeltaTime: in seconds.
		*/
		bool StepSimulation(float fDeltaTime, int maxSubSteps = 1, float fixedTimeStep = 1.0f / 60.0f);

		void SetGravity(const PARAVECTOR3 &gravity);
		void GetGravity(PARAVECTOR3& gravity) const;

		void AddRigidBody(CPhysicsRigidBody *body);
		void AddRigidBody(CPhysicsRigidBody *body, int group, int mask);
		void RemoveRigidBody(CPhysicsRigidBody *body);

		int GetNumBodies() const;
		CPhysicsRigidBody* GetBodyByIndex(int index);

		CPhysicsRigidBody* RaycastClosestShape(const PARAVECTOR3& vOrigin
			, const PARAVECTOR3& vDirection
			, DWORD dwType
			, RayCastHitResult& hit
			, int dwGroupMask
			, float fSensorRange);

		/** draw the entire physics debug world.*/
		void DebugDrawWorld();

		void AddConstraint(CPhysicsConstraint *constraint, bool disableCollisionsBetweenLinkedBodies = false);
		void RemoveConstraint(CPhysicsConstraint *constraint);
		int GetNumConstraints() const;
		CPhysicsConstraint * GetConstraint(int index);

	private:
		IParaPhysicsWorld* m_pWorld;
	};
}

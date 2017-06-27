#include "ParaEngine.h"
#include "Physics/PhysicsDynamicsWorld.h"
#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsConstraint.h"

namespace ParaEngine {
	CPhysicsDynamicsWorld::CPhysicsDynamicsWorld(IParaPhysicsWorld* pWorld)
		: m_pWorld(pWorld)
	{

	}

	IParaPhysicsWorld* CPhysicsDynamicsWorld::get()
	{
		return m_pWorld;
	}

	CPhysicsDynamicsWorld::~CPhysicsDynamicsWorld()
	{
		if (m_pWorld)
		{
			// release all constraint
			for (int i = 0; i < m_pWorld->GetNumConstraints(); i++)
			{
				auto obj = m_pWorld->GetConstraint(i);
				if (obj)
				{
					auto pConstraint = static_cast<CPhysicsConstraint*>(obj->GetUserData());
					if (pConstraint)
					{
						pConstraint->Release();
					}
				}
			}

			// release all bodies
			for (int i = 0; i < m_pWorld->GetNumBodies(); i++)
			{
				auto obj = m_pWorld->GetBodyByIndex(i);
				if (obj)
				{
					auto pBody = static_cast<CPhysicsBody*>(obj->GetUserData());
					if (pBody)
					{
						pBody->Release();
					}
				}
			}
			m_pWorld->Release();
		}
	}

	bool CPhysicsDynamicsWorld::StepSimulation(float fDeltaTime, int maxSubSteps, float fixedTimeStep)
	{
		assert(m_pWorld);

		return m_pWorld->StepSimulation(fDeltaTime, maxSubSteps, fixedTimeStep);
	}

	void CPhysicsDynamicsWorld::SetGravity(const PARAVECTOR3 &gravity)
	{
		assert(m_pWorld);
		m_pWorld->SetGravity(gravity);
	}

	void CPhysicsDynamicsWorld::GetGravity(PARAVECTOR3& gravity) const
	{
		assert(m_pWorld);
		m_pWorld->GetGravity(gravity);
	}

	void CPhysicsDynamicsWorld::AddRigidBody(CPhysicsRigidBody *body)
	{
		assert(m_pWorld && body && body->get());

		auto obj = static_cast<IParaPhysicsRigidbody*>(body->get());
		m_pWorld->AddRigidBody(obj);
		body->addref();
	}

	int CPhysicsDynamicsWorld::GetNumBodies() const
	{
		assert(m_pWorld);
		return m_pWorld->GetNumBodies();
	}

	CPhysicsRigidBody* CPhysicsDynamicsWorld::GetBodyByIndex(int index)
	{
		assert(m_pWorld);
		auto obj = m_pWorld->GetBodyByIndex(index);

		if (obj)
		{
			return static_cast<CPhysicsRigidBody*>(obj->GetUserData());
		}

		return nullptr;
	}

	void CPhysicsDynamicsWorld::AddRigidBody(CPhysicsRigidBody *body, int group, int mask)
	{
		assert(m_pWorld && body && body->get());
		auto obj = static_cast<IParaPhysicsRigidbody*>(body->get());
		m_pWorld->AddRigidBody(obj, group, mask);
		body->addref();
	}

	void CPhysicsDynamicsWorld::RemoveRigidBody(CPhysicsRigidBody *body)
	{
		assert(m_pWorld && body && body->get());

		// must be to remove constraint first
		assert(!body->isConstrainted());

		auto obj = static_cast<IParaPhysicsRigidbody*>(body->get());
		m_pWorld->RemoveRigidBody(obj);
		body->Release();
	}

	CPhysicsRigidBody* CPhysicsDynamicsWorld::RaycastClosestShape(const PARAVECTOR3& vOrigin
		, const PARAVECTOR3& vDirection
		, DWORD dwType
		, RayCastHitResult& hit
		, int dwGroupMask
		, float fSensorRange)
	{
		assert(m_pWorld);

		auto obj = m_pWorld->RaycastClosestShape(vOrigin, vDirection, dwType, hit, dwGroupMask, fSensorRange);
		if (obj)
		{
			return static_cast<CPhysicsRigidBody*>(obj->GetUserData());
		}
		
		return nullptr;
	}

	void CPhysicsDynamicsWorld::DebugDrawWorld()
	{
		assert(m_pWorld);
		m_pWorld->DebugDrawWorld();
	}

	void CPhysicsDynamicsWorld::AddConstraint(CPhysicsConstraint *constraint, bool disableCollisionsBetweenLinkedBodies)
	{
		assert(m_pWorld && constraint && constraint->get());
		m_pWorld->AddConstraint(constraint->get(), disableCollisionsBetweenLinkedBodies);
		constraint->addref();
	}

	void CPhysicsDynamicsWorld::RemoveConstraint(CPhysicsConstraint *constraint)
	{
		assert(m_pWorld && constraint && constraint->get());
		m_pWorld->RemoveConstraint(constraint->get());
		constraint->Release();
	}

	int CPhysicsDynamicsWorld::GetNumConstraints() const
	{
		assert(m_pWorld);
		return m_pWorld->GetNumConstraints();
	}

	CPhysicsConstraint* CPhysicsDynamicsWorld::GetConstraint(int index)
	{
		assert(m_pWorld);
		auto obj = m_pWorld->GetConstraint(index);

		if (obj)
		{
			return static_cast<CPhysicsConstraint*>(obj->GetUserData());
		}

		return nullptr;
	}

	int CPhysicsDynamicsWorld::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CPhysicsObject::InstallFields(pClass, bOverride);

		PE_ASSERT(pClass != NULL);

		pClass->AddField("gravity"
			, FieldType_Vector3
			, SetGravity_s
			, GetGravity_s
			, nullptr, nullptr, bOverride);


		return S_OK;
	}
}
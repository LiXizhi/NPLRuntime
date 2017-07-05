#include "ParaEngine.h"
#include "Physics/PhysicsDynamicsWorld.h"
#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsConstraint.h"

#include <luabind/object.hpp>
#include "ParaScriptingPhysics.h"

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

		pClass->AddField("numBodies"
			, FieldType_Int
			, nullptr
			, GetNumBodies_s
			, nullptr, nullptr, bOverride);

		pClass->AddField("numConstraints"
			, FieldType_Int
			, nullptr
			, GetNumConstraints_s
			, nullptr, nullptr, bOverride);

		pClass->AddField("AddConstraint"
			, FieldType_function
			, AddConstraint_s
			, nullptr
			, nullptr, nullptr, bOverride);

		pClass->AddField("RaycastClosestShape"
			, FieldType_function
			, RaycastClosestShape_s
			, nullptr
			, nullptr, nullptr, bOverride);

		return S_OK;
	}


	/* remove child object*/
	bool CPhysicsDynamicsWorld::RemoveChildAttributeObjcet(IAttributeFields* pChild)
	{
		if (!pChild)
			return false;

		auto classId = pChild->GetAttributeClassID();
		if (classId == ATTRIBUTE_CLASSID_CPhysicsRigidBody)
		{
			auto pBody = static_cast<CPhysicsRigidBody*>(pChild);
			RemoveRigidBody(pBody);
			return true;
		}

		if (classId >= ATTRIBUTE_CLASSID_CPhysicsConstraintMin && classId <= ATTRIBUTE_CLASSID_CPhysicsConstraintMax)
		{
			auto constraint = static_cast<CPhysicsConstraint*>(pChild);
			RemoveConstraint(constraint);
			return true;
		}

		return false;
	}

	bool CPhysicsDynamicsWorld::AddChildAttributeObject(IAttributeFields* pChild, int nRowIndex, int nColumnIndex)
	{
		(void)nRowIndex;
		(void)nColumnIndex;

		if (!pChild)
			return false;

		auto classId = pChild->GetAttributeClassID();
		if (classId == ATTRIBUTE_CLASSID_CPhysicsRigidBody)
		{
			auto pBody = static_cast<CPhysicsRigidBody*>(pChild);
			AddRigidBody(pBody);

			return true;
		}

		if (classId >= ATTRIBUTE_CLASSID_CPhysicsConstraintMin && classId <= ATTRIBUTE_CLASSID_CPhysicsConstraintMax)
		{
			auto constraint = static_cast<CPhysicsConstraint*>(pChild);
			AddConstraint(constraint);
			return true;
		}

		return false;
	}

	int CPhysicsDynamicsWorld::GetChildAttributeColumnCount()
	{
		return 2;
	}

	int CPhysicsDynamicsWorld::GetChildAttributeObjectCount(int nColumnIndex)
	{
		switch (nColumnIndex)
		{
		case 0:
			return GetNumBodies();
			break;
		case 1:
			return GetNumConstraints();
			break;
		default:
			return 0;
			break;
		}
	}

	IAttributeFields* CPhysicsDynamicsWorld::GetChildAttributeObject(int nRowIndex, int nColumnIndex)
	{
		switch (nColumnIndex)
		{
		case 0:
			return GetBodyByIndex(nRowIndex);
			break;
		case 1:
			return GetConstraint(nRowIndex);
			break;
		default:
			return nullptr;
			break;
		}
	}


	HRESULT CPhysicsDynamicsWorld::AddConstraint_s(CPhysicsDynamicsWorld* cls, const luabind::object& in, luabind::object& out)
	{
		if (luabind::type(in) != LUA_TTABLE)
			return S_FALSE;

		CPhysicsConstraint* pConstraint = nullptr;
		ParaScripting::ReadLuaTablePhysicsConstraint(in, "constraint", pConstraint);
		if (!pConstraint)
			return S_FALSE;
		
		bool disableCollisionsBetweenLinkedBodies = false;
		ParaScripting::ReadLuaTableBoolean(in, "disableCollisionsBetweenLinkedBodies", disableCollisionsBetweenLinkedBodies);

		cls->AddConstraint(pConstraint
			, disableCollisionsBetweenLinkedBodies);

		return S_OK;
	}

	HRESULT CPhysicsDynamicsWorld::RaycastClosestShape_s(CPhysicsDynamicsWorld* cls, const luabind::object& in, luabind::object& out)
	{
		if (luabind::type(in) != LUA_TTABLE)
			return S_FALSE;

		PARAVECTOR3 vOrigin;
		if (!ParaScripting::ReadLuaTableVector3(in, "origin", vOrigin))
			return S_FALSE;

		PARAVECTOR3 vDirection;
		if (!ParaScripting::ReadLuaTableVector3(in, "dir", vDirection))
				return S_FALSE;

		DWORD dwType;
		if (!ParaScripting::ReadLuaTableDWORD(in, "type", dwType))
			return S_FALSE;

		int dwGroupMask;
		if (!ParaScripting::ReadLuaTableInt(in, "mask", dwGroupMask))
			return S_FALSE;

		float fSensorRange;
		if (!ParaScripting::ReadLuaTableFloat(in, "sensorRange", fSensorRange))
			return S_FALSE;

		RayCastHitResult hit;
		auto pBody = cls->RaycastClosestShape(vOrigin, vDirection, dwType, hit, dwGroupMask, fSensorRange);
		if (pBody)
		{

		}
		else
		{

		}

		return S_OK;
	}
}
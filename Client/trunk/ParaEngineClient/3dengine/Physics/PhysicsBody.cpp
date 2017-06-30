#include "ParaEngine.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsShape.h"


namespace ParaEngine {

	CPhysicsBody::CPhysicsBody(IParaPhysicsBody* pBody)
		: m_pBody(pBody)
	{
		pBody->SetUserData(this);
	}

	CPhysicsBody::~CPhysicsBody()
	{
		SAFE_RELEASE(m_pBody);
	}

	int CPhysicsBody::GetInternalType() const
	{
		assert(m_pBody);
		return m_pBody->GetInternalType();
	}

	IParaPhysicsBody* CPhysicsBody::get()
	{
		return m_pBody;
	}

	void CPhysicsBody::SetContactCallBack(const ContactCallBack& cb)
	{
		if (cb)
		{
			m_pBody->SetContactCallBack([&cb](IParaPhysicsBody* bodyObjA, IParaPhysicsBody* bodyObjB, float dis, const PARAVECTOR3& posA, const PARAVECTOR3& posB, const PARAVECTOR3& normalOnB)
			{
				auto bodyA = static_cast<CPhysicsBody*>(bodyObjA->GetUserData());
				auto bodyB = static_cast<CPhysicsBody*>(bodyObjB->GetUserData());

				assert(bodyA && bodyB);

				cb(bodyA, bodyB, dis, posA, posB, normalOnB);
			});
		}
		else
		{
			m_pBody->SetContactCallBack(nullptr);
		}
	}

	int CPhysicsBody::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CPhysicsObject::InstallFields(pClass, bOverride);

		pClass->AddField("InternalType"
			, FieldType_Int
			, nullptr
			, GetInternalType_s
			, nullptr, nullptr, bOverride);

		return S_OK;
	}

	CPhysicsRigidBody::CPhysicsRigidBody(IParaPhysicsRigidbody* pBody, CPhysicsShape* pShape)
		: CPhysicsBody(pBody)
		, m_pShape(pShape)
		, m_pConstraint(nullptr)
	{
	}

	
	CPhysicsRigidBody::~CPhysicsRigidBody()
	{
		
	}

	int CPhysicsRigidBody::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CPhysicsBody::InstallFields(pClass, bOverride);

		PE_ASSERT(pClass != NULL);

		pClass->AddField("isConstrainted"
			, FieldType_Bool
			, nullptr
			, isConstrainted_s
			, nullptr, nullptr, bOverride);

		pClass->AddField("isInWorld"
			, FieldType_Bool
			, nullptr
			, isInWorld_s
			, nullptr, nullptr, bOverride);

		pClass->AddField("Convert2Kinematic"
			, FieldType_void
			, Convert2Kinematic_s
			, nullptr
			, nullptr, nullptr, bOverride);

		return S_OK;
	}

	void CPhysicsRigidBody::SetConstraint(CPhysicsConstraint* p)
	{
		m_pConstraint = p;
	}

	bool CPhysicsRigidBody::isConstrainted()
	{
		return m_pConstraint != nullptr;
	}

	bool CPhysicsRigidBody::isInWorld() const
	{
		assert(m_pBody);
		auto pBody = static_cast<IParaPhysicsRigidbody*>(m_pBody);
		return pBody->isInWorld();
	}

	void CPhysicsRigidBody::Convert2Kinematic()
	{
		assert(m_pBody);
		auto pBody = static_cast<IParaPhysicsRigidbody*>(m_pBody);
		return pBody->Convert2Kinematic();
	}
}
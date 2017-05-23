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

	CPhysicsRigidBody::CPhysicsRigidBody(IParaPhysicsRigidbody* pBody, CPhysicsShape* pShape)
		: CPhysicsBody(pBody)
		, m_pShape(pShape)
		, m_pConstraint(nullptr)
	{
	}

	
	CPhysicsRigidBody::~CPhysicsRigidBody()
	{
		
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
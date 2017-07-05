#include "ParaEngine.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsConstraint.h"

namespace ParaEngine {
	CPhysicsConstraint::CPhysicsConstraint(IParaPhysicsConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB)
		: m_pConstraint(pConstraint)
		, m_rbA(rbA)
		, m_rbB(rbB)
	{
		assert(pConstraint && rbA);

		m_pConstraint->SetUserData(this);
		rbA->SetConstraint(this);
		if (rbB)
			rbB->SetConstraint(this);
	}

	CPhysicsConstraint::~CPhysicsConstraint()
	{
		m_rbA->SetConstraint(nullptr);
		if (m_rbA)
		{
			m_rbA->SetConstraint(nullptr);
		}

		SAFE_RELEASE(m_pConstraint);

		/*
		assert(!((m_rbA && m_rbA->isInWorld())
			|| (m_rbB && m_rbB->isInWorld())));
			*/
	}

	IParaPhysicsConstraint* CPhysicsConstraint::get()
	{
		return m_pConstraint;
	}

	bool CPhysicsConstraint::isEnabled() const
	{
		assert(m_pConstraint);
		return m_pConstraint->isEnabled();
	}

	void CPhysicsConstraint::setEnabled(bool enabled)
	{
		assert(m_pConstraint);
		m_pConstraint->setEnabled(enabled);
	}

	int CPhysicsConstraint::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CPhysicsObject::InstallFields(pClass, bOverride);

		pClass->AddField("Enabled"
			, FieldType_Bool
			, setEnabled_s
			, isEnabled_s
			, nullptr, nullptr, bOverride);

		return S_OK;
	}

	CPhysicsP2PConstraint::CPhysicsP2PConstraint(IParaPhysicsPoint2PointConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB)
		: CPhysicsConstraint(pConstraint, rbA, rbB)
	{

	}

	CPhysicsP2PConstraint::~CPhysicsP2PConstraint()
	{

	}

	CPhysicsHingeConstraint::CPhysicsHingeConstraint(IParaPhysicsHingeConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB)
		: CPhysicsConstraint(pConstraint, rbA, rbB)
	{

	}

	CPhysicsHingeConstraint::~CPhysicsHingeConstraint()
	{

	}

	CPhysicsSliderConstraint::CPhysicsSliderConstraint(IParaPhysicsSliderConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB)
		: CPhysicsConstraint(pConstraint, rbA, rbB)
	{

	}

	CPhysicsSliderConstraint::~CPhysicsSliderConstraint()
	{

	}

	CPhysicsConeTwistConstraint::CPhysicsConeTwistConstraint(IParaPhysicsConeTwistConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB)
		: CPhysicsConstraint(pConstraint, rbA, rbB)
	{

	}

	CPhysicsConeTwistConstraint::~CPhysicsConeTwistConstraint()
	{

	}

	CPhysicsGeneric6DofSpringConstraint::CPhysicsGeneric6DofSpringConstraint(IParaPhysicsGeneric6DofSpringConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB)
		: CPhysicsConstraint(pConstraint, rbA, rbB)
	{

	}

	CPhysicsGeneric6DofSpringConstraint::~CPhysicsGeneric6DofSpringConstraint()
	{

	}

}
#pragma once

#include "Physics/PhysicsObject.h"

namespace ParaEngine {

	class CPhysicsRigidBody;

	class CPhysicsConstraint : public CPhysicsObject
	{
		friend class CPhysicsWorld;
	protected:
		CPhysicsConstraint(IParaPhysicsConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsConstraint();

		IParaPhysicsConstraint* get();

		ATTRIBUTE_DEFINE_CLASS(CPhysicsConstraint);

		bool isEnabled() const;
		void setEnabled(bool enabled);

	protected:
		IParaPhysicsConstraint* m_pConstraint;

		ref_ptr<CPhysicsRigidBody> m_rbA;
		ref_ptr<CPhysicsRigidBody> m_rbB;
	};


	class CPhysicsP2PConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsWorld;
	protected:
		CPhysicsP2PConstraint(IParaPhysicsPoint2PointConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsP2PConstraint();
		ATTRIBUTE_DEFINE_CLASS(CPhysicsP2PConstraint);
	};

	class CPhysicsHingeConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsWorld;
	protected:
		CPhysicsHingeConstraint(IParaPhysicsHingeConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsHingeConstraint();
		ATTRIBUTE_DEFINE_CLASS(CPhysicsHingeConstraint);
	};

	class CPhysicsSliderConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsWorld;
	protected:
		CPhysicsSliderConstraint(IParaPhysicsSliderConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsSliderConstraint();
		ATTRIBUTE_DEFINE_CLASS(CPhysicsSliderConstraint);
	};

	class CPhysicsConeTwistConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsWorld;
	protected:
		CPhysicsConeTwistConstraint(IParaPhysicsConeTwistConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsConeTwistConstraint();
		ATTRIBUTE_DEFINE_CLASS(CPhysicsConeTwistConstraint);
	};
	
	class CPhysicsGeneric6DofSpringConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsWorld;
	protected:
		CPhysicsGeneric6DofSpringConstraint(IParaPhysicsGeneric6DofSpringConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsGeneric6DofSpringConstraint();
		ATTRIBUTE_DEFINE_CLASS(CPhysicsGeneric6DofSpringConstraint);
	};

	
}
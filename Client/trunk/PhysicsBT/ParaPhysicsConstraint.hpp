#pragma once

#include "btBulletDynamicsCommon.h"
#include "Physics/IParaPhysics.h"

namespace ParaEngine {

	// use template instead of Virtual Inheritance
	template< class Base >
	struct _BulletConstraint : public Base
	{
		/// pointer to the low level physics engine shape object. 
		btTypedConstraint* m_pConstraint;

		/// keep some user data here
		void* m_pUserData;

		_BulletConstraint() : m_pConstraint(nullptr)
		{
		};

		virtual ~_BulletConstraint()
		{
			SAFE_DELETE(m_pConstraint);
		};

		virtual void* GetUserData()
		{
			return m_pUserData;
		};

		virtual void SetUserData(void* pData)
		{
			m_pUserData = pData;
		};

		virtual void* get()
		{
			return m_pConstraint;
		};

		virtual void Release()
		{
			delete this;
		};

		virtual bool isEnabled() const
		{
			assert(m_pConstraint);
			return m_pConstraint->isEnabled();
		};

		virtual void setEnabled(bool enabled)
		{
			assert(m_pConstraint);
			m_pConstraint->setEnabled(enabled);
		}
	};

	struct BulletPoint2PointConstraint : public _BulletConstraint<IParaPhysicsPoint2PointConstraint>
	{

	};

	struct BulletHingeConstraint : public _BulletConstraint<IParaPhysicsHingeConstraint>
	{

	};

	struct BulletSliderConstraint : public _BulletConstraint<IParaPhysicsSliderConstraint>
	{

	};

	struct BulletConeTwistConstraint : public _BulletConstraint<IParaPhysicsConeTwistConstraint>
	{

	};

	struct BulletGeneric6DofSpringConstraint : public _BulletConstraint<IParaPhysicsGeneric6DofSpringConstraint>
	{

	};
}
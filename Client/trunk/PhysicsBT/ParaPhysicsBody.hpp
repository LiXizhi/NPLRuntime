#pragma once

#include "btBulletDynamicsCommon.h"
#include "Physics/IParaPhysics.h"

namespace ParaEngine {
	// use template instead of Virtual Inheritance
	template< class Base >
	struct _BulletBody : public Base
	{
		virtual void* get() 
		{ 
			return m_pBody;
		};

		virtual void Release() 
		{
			delete this;
		};

		virtual void* GetUserData() 
		{ 
			return m_userData; 
		};

		virtual void SetUserData(void* pData)
		{ 
			m_userData = pData;
		}

		int GetInternalType() const
		{
			assert(m_pBody);
			return m_pBody->getInternalType();
		}

		_BulletBody()
			: m_pBody(nullptr)
			, m_userData(nullptr)
		{
		};


		virtual ~_BulletBody()
		{
			SAFE_DELETE(m_pBody);
		};

		btCollisionObject* m_pBody;
		void* m_userData;
	};

	struct BulletRigidbody : public _BulletBody<IParaPhysicsRigidbody>
	{
		BulletRigidbody()
		{

		}

		virtual ~BulletRigidbody()
		{
			if (m_pBody)
			{
				auto pBody = static_cast<btRigidBody*>(m_pBody);
				auto state = pBody->getMotionState();
				if (state)
					delete state;
			}
		}

		virtual bool isInWorld() const
		{
			auto pBody = static_cast<btRigidBody*>(m_pBody);
			return pBody->isInWorld();
		};

		// convert this body to kinematic body
		virtual void Convert2Kinematic()
		{
			auto pBody = static_cast<btRigidBody*>(m_pBody);
			pBody->setCollisionFlags(pBody->getCollisionFlags() |
				btCollisionObject::CF_KINEMATIC_OBJECT);

			pBody->setActivationState(DISABLE_DEACTIVATION);
		};
	};
}

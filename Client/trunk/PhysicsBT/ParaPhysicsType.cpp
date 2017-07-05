#include "stdafx.h"
#include "ParaPhysicsType.h"

namespace ParaEngine {

	BulletWorld::BulletWorld()
		: m_pWorld(nullptr)
		, m_solver(nullptr)
		, m_broadphase(nullptr)
		, m_dispatcher(nullptr)
		, m_collisionConfiguration(nullptr)

	{

	}

	BulletWorld::~BulletWorld()
	{
		//delete dynamics world
		SAFE_DELETE(m_pWorld);

		//delete solver
		SAFE_DELETE(m_solver);

		//delete broad phase
		SAFE_DELETE(m_broadphase);

		//delete dispatcher
		SAFE_DELETE(m_dispatcher);

		SAFE_DELETE(m_collisionConfiguration);
	}

	void BulletWorld::DebugDrawWorld()
	{
		if (m_pWorld)
		{
			m_pWorld->debugDrawWorld();
		}
	}

	bool BulletWorld::StepSimulation(float fDeltaTime, int maxSubSteps, float fixedTimeStep)
	{
		m_pWorld->stepSimulation(fDeltaTime, maxSubSteps, btScalar(fixedTimeStep));


		auto numManifolds = m_dispatcher->getNumManifolds();
		for (decltype(numManifolds) i = 0; i < numManifolds; i++)
		{
			auto contactManifold = m_dispatcher->getManifoldByIndexInternal(i);
			auto objA = contactManifold->getBody0();
			auto objB = contactManifold->getBody1();

			auto numContacts = contactManifold->getNumContacts();
			for (decltype(numContacts) j = 0; j < numContacts; j++)
			{
				auto& pt = contactManifold->getContactPoint(j);
				auto dis = pt.getDistance();
				if (dis < 0.0f)
				{
					const  btVector3& ptA = pt.getPositionWorldOnA();
					const  btVector3& ptB = pt.getPositionWorldOnB();
					const  btVector3& normalOnB = pt.m_normalWorldOnB;

					PARAVECTOR3 posA = CONVERT_PARAVECTOR3(ptA);
					PARAVECTOR3 posB = CONVERT_PARAVECTOR3(ptB);
					PARAVECTOR3 normal = CONVERT_PARAVECTOR3(normalOnB);

					auto bodyA = static_cast<IParaPhysicsBody*>(objA->getUserPointer());
					auto bodyB = static_cast<IParaPhysicsBody*>(objB->getUserPointer());

					assert(bodyA && bodyB);

					bodyA->OnContact(bodyA, bodyB, dis, posA, posB, normal);
					bodyB->OnContact(bodyA, bodyB, dis, posA, posB, normal);
				}
			}
		}

		return true;
	}

	void BulletWorld::SetGravity(const PARAVECTOR3 &gravity)
	{
		assert(m_pWorld);

		m_pWorld->setGravity(CONVERT_BTVECTOR3(gravity));
	}

	void BulletWorld::GetGravity(PARAVECTOR3& gravity) const
	{
		assert(m_pWorld);

		auto tmp = m_pWorld->getGravity();
		gravity = CONVERT_PARAVECTOR3(tmp);
	}

	void BulletWorld::AddConstraint(IParaPhysicsConstraint *constraint, bool disableCollisionsBetweenLinkedBodies)
	{
		assert(m_pWorld && constraint && constraint->get());
		m_pWorld->addConstraint(static_cast<btTypedConstraint*>(constraint->get()), disableCollisionsBetweenLinkedBodies);
	}

	void BulletWorld::RemoveConstraint(IParaPhysicsConstraint *constraint)
	{
		assert(m_pWorld && constraint && constraint->get());
		m_pWorld->removeConstraint(static_cast<btTypedConstraint*>(constraint->get()));
	}

	int BulletWorld::GetNumConstraints() const
	{
		assert(m_pWorld);
		return m_pWorld->getNumConstraints();
	}

	IParaPhysicsConstraint* BulletWorld::GetConstraint(int index)
	{
		assert(m_pWorld);
		auto obj = m_pWorld->getConstraint(index);
		if (obj)
		{
			return static_cast<IParaPhysicsConstraint*>(obj->getUserConstraintPtr());
		}

		return nullptr;
	}

	void BulletWorld::AddRigidBody(IParaPhysicsRigidbody *body)
	{
		assert(m_pWorld && body && body->get());
		m_pWorld->addRigidBody(static_cast<btRigidBody*>(body->get()));
	}

	void BulletWorld::AddRigidBody(IParaPhysicsRigidbody *body, int group, int mask)
	{
		assert(m_pWorld && body && body->get());
		m_pWorld->addRigidBody(static_cast<btRigidBody*>(body->get()), group, mask);
	}

	void BulletWorld::RemoveRigidBody(IParaPhysicsRigidbody *body)
	{
		assert(m_pWorld && body && body->get());
		m_pWorld->removeRigidBody(static_cast<btRigidBody*>(body->get()));
	}

	int BulletWorld::GetNumBodies() const
	{
		assert(m_pWorld);
		return m_pWorld->getNumCollisionObjects();
	}

	IParaPhysicsRigidbody* BulletWorld::GetBodyByIndex(int index)
	{
		assert(m_pWorld);
		auto obj = m_pWorld->getCollisionObjectArray()[index];
		auto body = static_cast<IParaPhysicsRigidbody*>(obj->getUserPointer());
		return body;
	}

	IParaPhysicsRigidbody* BulletWorld::RaycastClosestShape(const PARAVECTOR3& vOrigin
		, const PARAVECTOR3& vDirection
		, DWORD dwType
		, RayCastHitResult& hit
		, int dwGroupMask
		, float fSensorRange)
	{
		btVector3 vFrom = CONVERT_BTVECTOR3(vOrigin);
		btVector3 vTo = CONVERT_BTVECTOR3(vDirection);

		if (fSensorRange < 0.f)
			fSensorRange = 200.f;
		vTo = vFrom + vTo * fSensorRange;

		if ((vFrom - vTo).fuzzyZero())
			return nullptr;

		btCollisionWorld::ClosestRayResultCallback cb(vFrom, vTo);

		cb.m_collisionFilterMask = dwGroupMask;
		// filter back faces. Added by LiXizhi 2010.3. In bullet, triangle seems to be double sided by default.  
		cb.m_flags = 1;// btTriangleRaycastCallback::kF_FilterBackfaces;

		m_pWorld->rayTest(vFrom, vTo, cb);

		if (cb.hasHit())
		{
			hit.m_vHitPointWorld = CONVERT_PARAVECTOR3(cb.m_hitPointWorld);
			hit.m_vHitNormalWorld = CONVERT_PARAVECTOR3(cb.m_hitNormalWorld.normalize());
			hit.m_fDistance = cb.m_hitPointWorld.distance(cb.m_rayFromWorld);

			return static_cast<IParaPhysicsRigidbody*>(cb.m_collisionObject->getUserPointer());
		}
		else
		{
			hit.m_vHitPointWorld = CONVERT_PARAVECTOR3(vTo);
			hit.m_vHitNormalWorld = CONVERT_PARAVECTOR3(btVector3(1.0, 0.0, 0.0));
			return nullptr;
		}
	}


	ParaPhysicsMotionState::ParaPhysicsMotionState(const btTransform& startTrans, const btTransform& centerOfMassOffset, ParaPhysicsMotionStateDesc::UpdateTransformCallBack cb)
		: m_graphicsWorldTrans(startTrans)
		, m_centerOfMassOffset(centerOfMassOffset)
		, m_startWorldTrans(startTrans)
		, m_cb(cb)
	{

	}

	void ParaPhysicsMotionState::getWorldTransform(btTransform& centerOfMassWorldTrans) const
	{
		centerOfMassWorldTrans = m_graphicsWorldTrans * m_centerOfMassOffset.inverse();
	}

	void ParaPhysicsMotionState::setWorldTransform(const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin)
	{
		btMatrix3x3 matRot(
			btScalar(rotation._11), btScalar(rotation._21), btScalar(rotation._31),
			btScalar(rotation._12), btScalar(rotation._22), btScalar(rotation._32),
			btScalar(rotation._13), btScalar(rotation._23), btScalar(rotation._33)
		);
		btVector3 vOrigin(btScalar(origin.x), btScalar(origin.y), btScalar(origin.z));
		m_graphicsWorldTrans = btTransform(matRot, vOrigin);
	}

	void ParaPhysicsMotionState::setWorldTransform(const btTransform& centerOfMassWorldTrans)
	{
		m_graphicsWorldTrans = centerOfMassWorldTrans * m_centerOfMassOffset;

		if (m_cb)
		{
			const auto& btBasis = centerOfMassWorldTrans.getBasis();
			const auto& btOrigin = centerOfMassWorldTrans.getOrigin();

			PARAMATRIX3x3 rotation;
			PARAVECTOR3 origin;

			rotation._11 = (float)btBasis.getRow(0).getX();
			rotation._21 = (float)btBasis.getRow(0).getY();
			rotation._31 = (float)btBasis.getRow(0).getZ();

			rotation._12 = (float)btBasis.getRow(1).getX();
			rotation._22 = (float)btBasis.getRow(1).getY();
			rotation._32 = (float)btBasis.getRow(1).getZ();

			rotation._13 = (float)btBasis.getRow(2).getX();
			rotation._23 = (float)btBasis.getRow(2).getY();
			rotation._33 = (float)btBasis.getRow(2).getZ();

			origin.x = (float)btOrigin.getX();
			origin.y = (float)btOrigin.getY();
			origin.z = (float)btOrigin.getZ();

			m_cb(rotation, origin);
		}
	}
}
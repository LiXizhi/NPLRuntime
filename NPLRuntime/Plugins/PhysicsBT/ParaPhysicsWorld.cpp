//-----------------------------------------------------------------------------
// Class:	ParaEngine physics wrapper for the bullet physics engine DLL
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.2.24
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ParaPhysicsWorld.h"

// @def using motion state is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#define USE_MOTIONSTATE 1

using namespace ParaEngine;

BulletPhysicsShape::BulletPhysicsShape()
	:m_pShape(NULL), m_indexVertexArrays(NULL), m_triangleIndices(NULL), m_vertices(NULL)
{
}

BulletPhysicsShape::~BulletPhysicsShape()
{
	SAFE_DELETE(m_pShape);
	SAFE_DELETE(m_indexVertexArrays);
	SAFE_DELETE_ARRAY(m_triangleIndices);
	SAFE_DELETE_ARRAY(m_vertices);
}

void ParaEngine::BulletPhysicsShape::Release()
{
	delete this;
}

ParaEngine::BulletPhysicsActor::BulletPhysicsActor(btRigidBody* pActor) : m_pActor(pActor)
{

}

ParaEngine::BulletPhysicsActor::~BulletPhysicsActor()
{
	if (m_pActor && m_pActor->getMotionState())
	{
		delete m_pActor->getMotionState();
	}
	SAFE_DELETE(m_pActor);
}

PARAMATRIX* ParaEngine::BulletPhysicsActor::GetWorldTransform(PARAMATRIX* pOut)
{
	static PARAMATRIX s_OutputMatrix;
	if (pOut == NULL)
		pOut = &s_OutputMatrix;
	btTransform& transform = m_pActor->getWorldTransform();
	transform.getOpenGLMatrix((float*)pOut);
	return pOut;
}

PARAVECTOR3 ParaEngine::BulletPhysicsActor::GetOrigin() { 
	btTransform& transform = m_pActor->getWorldTransform();
	PARAVECTOR3 vOrigin = (PARAVECTOR3&)transform.getOrigin();
	return vOrigin;
}

void ParaEngine::BulletPhysicsActor::SetWorldTransform(const PARAMATRIX* pMatrix)
{
#ifdef USE_MOTIONSTATE
	static btTransform transform;
	transform.setFromOpenGLMatrix((float*)pMatrix);
	m_pActor->getMotionState()->setWorldTransform(transform);
	if (IsKinematicObject()) m_pActor->setCenterOfMassTransform(transform);
#else
	m_pActor->getWorldTransform().setFromOpenGLMatrix((float*)pMatrix);
#endif
}

void ParaEngine::BulletPhysicsActor::Release()
{
	delete this;
}

void ParaEngine::BulletPhysicsActor::ApplyCentralImpulse(const PARAVECTOR3& impulse)
{
	m_pActor->setActivationState(ACTIVE_TAG);
	m_pActor->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}
void BulletPhysicsActor::Activate()
{
	m_pActor->activate();
}
bool BulletPhysicsActor::IsActive()
{
	return m_pActor->isActive();
}
bool BulletPhysicsActor::IsKinematicObject()
{
	return m_pActor->isKinematicObject();
}
bool BulletPhysicsActor::IsStaticObject()
{
	return m_pActor->isStaticObject();
}
bool BulletPhysicsActor::IsStaticOrKinematicObject()
{
	return m_pActor->isStaticOrKinematicObject();
}
float BulletPhysicsActor::GetMass()
{
	float mass = m_pActor->getInvMass();
	return mass == 0.0f ? 0.0f : (1 / mass);
}
void BulletPhysicsActor::SetMass(float mass)
{
	m_pActor->setMassProps(mass, m_pActor->getLocalInertia());
}
PARAVECTOR3 BulletPhysicsActor::GetLocalInertia()
{
	const btVector3& inertia = m_pActor->getLocalInertia();
	return PARAVECTOR3((float)inertia.getX(), (float)inertia.getY(), (float)inertia.getZ());
}
void BulletPhysicsActor::SetLocalInertia(const PARAVECTOR3& inertia)
{
	m_pActor->setMassProps(GetMass(), btVector3(inertia.x, inertia.y, inertia.z));
}
PARAVECTOR3 BulletPhysicsActor::GetGravity()
{
	const btVector3& gravity = m_pActor->getGravity();
	return PARAVECTOR3((float)gravity.getX(), (float)gravity.getY(), (float)gravity.getZ());
}
void BulletPhysicsActor::SetGravity(const PARAVECTOR3& gravity)
{
	m_pActor->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
	// m_pActor->applyGravity();
}
// [0, 1]
void BulletPhysicsActor::SetLinearDamping(float damping)
{
	m_pActor->setDamping(damping, m_pActor->getAngularDamping());
	// m_pActor->applyDamping(1.0f);
}
float BulletPhysicsActor::GetLinearDamping()
{
	return m_pActor->getLinearDamping();
}
void BulletPhysicsActor::SetAngularDamping(float damping)
{
	m_pActor->setDamping(m_pActor->getLinearDamping(), damping);
	// m_pActor->applyDamping(1.0f);
}
float BulletPhysicsActor::GetAngularDamping()
{
	return m_pActor->getAngularDamping();
}
PARAVECTOR3 BulletPhysicsActor::GetLinearFactor()
{
	const btVector3& factor = m_pActor->getLinearFactor();
	return PARAVECTOR3((float)factor.getX(), (float)factor.getY(), (float)factor.getZ());
}
void BulletPhysicsActor::SetLinearFactor(const PARAVECTOR3& factor)
{
	m_pActor->setLinearFactor(btVector3(factor.x, factor.y, factor.z));
}
PARAVECTOR3 BulletPhysicsActor::GetAngularFactor()
{
	const btVector3& factor = m_pActor->getAngularFactor();
	return PARAVECTOR3((float)factor.getX(), (float)factor.getY(), (float)factor.getZ());
}
void BulletPhysicsActor::SetAngularFactor(const PARAVECTOR3& factor)
{
	m_pActor->setAngularFactor(btVector3(factor.x, factor.y, factor.z));
}
PARAVECTOR3 BulletPhysicsActor::GetLinearVelocity()
{
	const btVector3& velocity = m_pActor->getLinearVelocity();
	return PARAVECTOR3((float)velocity.getX(), (float)velocity.getY(), (float)velocity.getZ());
}
void BulletPhysicsActor::SetLinearVelocity(const PARAVECTOR3& velocity)
{
	m_pActor->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}
PARAVECTOR3 BulletPhysicsActor::GetAngularVelocity()
{
	const btVector3& velocity = m_pActor->getAngularVelocity();
	return PARAVECTOR3((float)velocity.getX(), (float)velocity.getY(), (float)velocity.getZ());
}
void BulletPhysicsActor::SetAngularVelocity(const PARAVECTOR3& velocity)
{
	m_pActor->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}
int BulletPhysicsActor::GetFlags()
{
	return m_pActor->getFlags();
}
void BulletPhysicsActor::SetFlags(int flags)
{
	m_pActor->setFlags(flags);
}
int BulletPhysicsActor::GetActivationState()
{
	return m_pActor->getActivationState();
}
void BulletPhysicsActor::SetActivationState(int state)
{
	m_pActor->setActivationState(state);
}
float BulletPhysicsActor::GetDeactivationTime()
{
	return m_pActor->getDeactivationTime();
}
void BulletPhysicsActor::SetDeactivationTime(float time)
{
	m_pActor->setDeactivationTime(time);
}
float BulletPhysicsActor::GetRestitution()
{
	return m_pActor->getRestitution();
}
void BulletPhysicsActor::SetRestitution(float restitution)
{
	m_pActor->setRestitution(restitution);
}
float BulletPhysicsActor::GetFriction()
{
	return m_pActor->getFriction();
}
void BulletPhysicsActor::SetFriction(float friction)
{
	m_pActor->setFriction(friction);
}
float BulletPhysicsActor::GetRollingFriction()
{
	return m_pActor->getRollingFriction();
}
void BulletPhysicsActor::SetRollingFriction(float friction)
{
	m_pActor->setRollingFriction(friction);
}
float BulletPhysicsActor::GetSpinningFriction()
{
	return m_pActor->getSpinningFriction();
}
void BulletPhysicsActor::SetSpinningFriction(float friction)
{
	m_pActor->setSpinningFriction(friction);
}
float BulletPhysicsActor::GetContactStiffness()
{
	return m_pActor->getContactStiffness();
}
void BulletPhysicsActor::SetContactStiffness(float stiffness)
{
	m_pActor->setContactStiffnessAndDamping(stiffness, m_pActor->getContactDamping());
}
float BulletPhysicsActor::GetContactDamping()
{
	return m_pActor->getContactDamping();
}
void BulletPhysicsActor::SetContactDamping(float damping)
{
	m_pActor->setContactStiffnessAndDamping(m_pActor->getContactStiffness(), damping);
}
int BulletPhysicsActor::GetIslandTag()
{
	return m_pActor->getIslandTag();
}
void BulletPhysicsActor::SetIslandTag(int flags)
{
	m_pActor->setIslandTag(flags);
}
int BulletPhysicsActor::GetCompanionId()
{
	return m_pActor->getCompanionId();
}
void BulletPhysicsActor::SetCompanionId(int id)
{
	m_pActor->setCompanionId(id);
}
float BulletPhysicsActor::GetHitFraction()
{
	return m_pActor->getHitFraction();
}
void BulletPhysicsActor::SetHitFraction(float fraction)
{
	m_pActor->setHitFraction(fraction);
}
int BulletPhysicsActor::GetCollisionFlags()
{
	return m_pActor->getCollisionFlags();
}
void BulletPhysicsActor::SetCollisionFlags(int flags)
{
	m_pActor->setCollisionFlags(flags);
}
float BulletPhysicsActor::GetCcdSweptSphereRadius()
{
	return m_pActor->getCcdSweptSphereRadius();
}
void BulletPhysicsActor::SetCcdSweptSphereRadius(float radius)
{
	m_pActor->setCcdSweptSphereRadius(radius);
}
float BulletPhysicsActor::GetCcdMotionThreshold()
{
	return m_pActor->getCcdMotionThreshold();
}
void BulletPhysicsActor::SetCcdMotionThreshold(float threshold)
{
	m_pActor->setCcdMotionThreshold(threshold);
}

//////////////////////////////////////////////////////////////////////////
// BulletPhysicsConstraint Implementation
//////////////////////////////////////////////////////////////////////////

BulletPhysicsConstraint::BulletPhysicsConstraint(btTypedConstraint* pConstraint, ParaPhysicsConstraintType type,
	IParaPhysicsActor* pActorA, IParaPhysicsActor* pActorB)
	: m_pConstraint(pConstraint), m_type(type), m_pActorA(pActorA), m_pActorB(pActorB)
{
}

BulletPhysicsConstraint::~BulletPhysicsConstraint()
{
	SAFE_DELETE(m_pConstraint);
}

void BulletPhysicsConstraint::Release()
{
	delete this;
}

bool BulletPhysicsConstraint::IsEnabled()
{
	return m_pConstraint ? m_pConstraint->isEnabled() : false;
}

void BulletPhysicsConstraint::SetEnabled(bool enabled)
{
	if (m_pConstraint)
		m_pConstraint->setEnabled(enabled);
}

float BulletPhysicsConstraint::GetBreakingThreshold()
{
	return m_pConstraint ? m_pConstraint->getBreakingImpulseThreshold() : 0.0f;
}

void BulletPhysicsConstraint::SetBreakingThreshold(float threshold)
{
	if (m_pConstraint)
		m_pConstraint->setBreakingImpulseThreshold(threshold);
}

float BulletPhysicsConstraint::GetHingeAngle()
{
	if (m_type == ConstraintType_Hinge && m_pConstraint)
	{
		btHingeConstraint* hinge = static_cast<btHingeConstraint*>(m_pConstraint);
		return hinge->getHingeAngle();
	}
	return 0.0f;
}

void BulletPhysicsConstraint::SetHingeLimit(float low, float high, float softness, float biasFactor, float relaxationFactor)
{
	if (m_type == ConstraintType_Hinge && m_pConstraint)
	{
		btHingeConstraint* hinge = static_cast<btHingeConstraint*>(m_pConstraint);
		hinge->setLimit(low, high, softness, biasFactor, relaxationFactor);
	}
}

void BulletPhysicsConstraint::EnableHingeMotor(bool enable, float targetVelocity, float maxImpulse)
{
	if (m_type == ConstraintType_Hinge && m_pConstraint)
	{
		btHingeConstraint* hinge = static_cast<btHingeConstraint*>(m_pConstraint);
		hinge->enableAngularMotor(enable, targetVelocity, maxImpulse);
	}
}

float BulletPhysicsConstraint::GetSliderPosition()
{
	if (m_type == ConstraintType_Slider && m_pConstraint)
	{
		btSliderConstraint* slider = static_cast<btSliderConstraint*>(m_pConstraint);
		return slider->getLinearPos();
	}
	return 0.0f;
}

void BulletPhysicsConstraint::SetSliderLimit(float lowerLimit, float upperLimit)
{
	if (m_type == ConstraintType_Slider && m_pConstraint)
	{
		btSliderConstraint* slider = static_cast<btSliderConstraint*>(m_pConstraint);
		slider->setLowerLinLimit(lowerLimit);
		slider->setUpperLinLimit(upperLimit);
	}
}

void BulletPhysicsConstraint::EnableSliderMotor(bool enable, float targetVelocity, float maxForce)
{
	if (m_type == ConstraintType_Slider && m_pConstraint)
	{
		btSliderConstraint* slider = static_cast<btSliderConstraint*>(m_pConstraint);
		slider->setPoweredLinMotor(enable);
		slider->setTargetLinMotorVelocity(targetVelocity);
		slider->setMaxLinMotorForce(maxForce);
	}
}

//////////////////////////////////////////////////////////////////////////
// BulletPhysicsVehicle Implementation
//////////////////////////////////////////////////////////////////////////

BulletPhysicsVehicle::BulletPhysicsVehicle(btRaycastVehicle* pVehicle, btVehicleRaycaster* pRaycaster, IParaPhysicsActor* pChassisActor)
	: m_pVehicle(pVehicle), m_pRaycaster(pRaycaster), m_pChassisActor(pChassisActor)
{
}

BulletPhysicsVehicle::~BulletPhysicsVehicle()
{
	SAFE_DELETE(m_pVehicle);
	SAFE_DELETE(m_pRaycaster);
}

void BulletPhysicsVehicle::Release()
{
	delete this;
}

int BulletPhysicsVehicle::AddWheel(const ParaPhysicsWheelDesc& wheelDesc)
{
	if (!m_pVehicle)
		return -1;

	btVector3 connectionPoint(wheelDesc.m_connectionPoint.x, wheelDesc.m_connectionPoint.y, wheelDesc.m_connectionPoint.z);
	btVector3 wheelDirection(wheelDesc.m_wheelDirection.x, wheelDesc.m_wheelDirection.y, wheelDesc.m_wheelDirection.z);
	btVector3 wheelAxle(wheelDesc.m_wheelAxle.x, wheelDesc.m_wheelAxle.y, wheelDesc.m_wheelAxle.z);

	btWheelInfo& wheel = m_pVehicle->addWheel(
		connectionPoint,
		wheelDirection,
		wheelAxle,
		wheelDesc.m_suspensionRestLength,
		wheelDesc.m_wheelRadius,
		m_tuning,
		wheelDesc.m_isFrontWheel
	);

	wheel.m_suspensionStiffness = wheelDesc.m_suspensionStiffness;
	wheel.m_wheelsDampingCompression = wheelDesc.m_wheelsDampingCompression;
	wheel.m_wheelsDampingRelaxation = wheelDesc.m_wheelsDampingRelaxation;
	wheel.m_frictionSlip = wheelDesc.m_frictionSlip;
	wheel.m_rollInfluence = wheelDesc.m_rollInfluence;

	return m_pVehicle->getNumWheels() - 1;
}

int BulletPhysicsVehicle::GetNumWheels()
{
	return m_pVehicle ? m_pVehicle->getNumWheels() : 0;
}

void BulletPhysicsVehicle::SetSteeringValue(float steering, int wheelIndex)
{
	if (m_pVehicle && wheelIndex >= 0 && wheelIndex < m_pVehicle->getNumWheels())
		m_pVehicle->setSteeringValue(steering, wheelIndex);
}

float BulletPhysicsVehicle::GetSteeringValue(int wheelIndex)
{
	if (m_pVehicle && wheelIndex >= 0 && wheelIndex < m_pVehicle->getNumWheels())
		return m_pVehicle->getSteeringValue(wheelIndex);
	return 0.0f;
}

void BulletPhysicsVehicle::ApplyEngineForce(float force, int wheelIndex)
{
	if (m_pVehicle && wheelIndex >= 0 && wheelIndex < m_pVehicle->getNumWheels())
		m_pVehicle->applyEngineForce(force, wheelIndex);
}

void BulletPhysicsVehicle::SetBrake(float brake, int wheelIndex)
{
	if (m_pVehicle && wheelIndex >= 0 && wheelIndex < m_pVehicle->getNumWheels())
		m_pVehicle->setBrake(brake, wheelIndex);
}

PARAMATRIX* BulletPhysicsVehicle::GetWheelTransform(int wheelIndex, PARAMATRIX* pOut)
{
	static PARAMATRIX s_OutputMatrix;
	if (pOut == NULL)
		pOut = &s_OutputMatrix;
	
	if (m_pVehicle && wheelIndex >= 0 && wheelIndex < m_pVehicle->getNumWheels())
	{
		m_pVehicle->updateWheelTransform(wheelIndex, true);
		const btTransform& transform = m_pVehicle->getWheelTransformWS(wheelIndex);
		transform.getOpenGLMatrix((float*)pOut);
	}
	return pOut;
}

void BulletPhysicsVehicle::UpdateWheelTransform(int wheelIndex, bool interpolatedTransform)
{
	if (m_pVehicle && wheelIndex >= 0 && wheelIndex < m_pVehicle->getNumWheels())
		m_pVehicle->updateWheelTransform(wheelIndex, interpolatedTransform);
}

float BulletPhysicsVehicle::GetCurrentSpeedKmHour()
{
	return m_pVehicle ? m_pVehicle->getCurrentSpeedKmHour() : 0.0f;
}

PARAVECTOR3 BulletPhysicsVehicle::GetForwardVector()
{
	if (m_pVehicle)
	{
		const btVector3& fwd = m_pVehicle->getForwardVector();
		return PARAVECTOR3(fwd.x(), fwd.y(), fwd.z());
	}
	return PARAVECTOR3(0, 0, 1);
}

void BulletPhysicsVehicle::ResetSuspension()
{
	if (m_pVehicle)
		m_pVehicle->resetSuspension();
}


//
// Physics World
//
CParaPhysicsWorld::CParaPhysicsWorld()
	: m_dynamicsWorld(NULL), m_collisionWorld(NULL), m_broadphase(NULL), m_dispatcher(NULL), m_solver(NULL), m_collisionConfiguration(NULL), m_bInvertFaceWinding(false)
{
	m_exit = true;
#ifdef WIN32
	// m_bInvertFaceWinding = true;
#endif
}

CParaPhysicsWorld::~CParaPhysicsWorld()
{

}

void CParaPhysicsWorld::Release()
{
	delete this;
}

bool CParaPhysicsWorld::InitPhysics()
{
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration);

	/*btVector3 worldMin(0,-1000.f,0);
	btVector3 worldMax(40000.f,1000.f,40000.f);
	m_broadphase = new btAxisSweep3(worldMin,worldMax);*/

	m_broadphase = new btDbvtBroadphase();

	m_solver = new btSequentialImpulseConstraintSolver();
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

	// m_dynamicsWorld->setGravity(btVector3(0, 0, 0));

	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->setDebugDrawer(&m_physics_debug_draw);
	}
	m_exit = false;
	return true;
}

bool CParaPhysicsWorld::StepSimulation(float fDeltaTime)
{
	m_dynamicsWorld->stepSimulation(fDeltaTime, 3, 1 / 60.f);
	return true;
}

bool CParaPhysicsWorld::ExitPhysics()
{
	m_exit = true;
	if (m_dynamicsWorld == 0)
		return true;

	//cleanup in the reverse order of creation/initialization
	while (!m_vehicles.empty())
	{
		ReleaseVehicle(*(m_vehicles.begin()));
	}

	while (!m_constraints.empty())
	{
		ReleaseConstraint(*(m_constraints.begin()));
	}

	while (!m_actors.empty())
	{
		ReleaseActor(*(m_actors.begin()));
	}

	//remove the rigid bodies from the dynamics world and delete them
	int i;
	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	{
		IParaPhysicsShape_Array_Type::iterator itCur, itEnd = m_collisionShapes.end();
		for (itCur = m_collisionShapes.begin(); itCur != itEnd; ++itCur)
		{
			IParaPhysicsShape* shape = (*itCur);
			shape->Release();
		}
		m_collisionShapes.clear();
	}

	//delete dynamics world
	SAFE_DELETE(m_dynamicsWorld);

	//delete solver
	SAFE_DELETE(m_solver);

	//delete broad phase
	SAFE_DELETE(m_broadphase);

	//delete dispatcher
	SAFE_DELETE(m_dispatcher);

	SAFE_DELETE(m_collisionConfiguration);

	return true;
}

IParaPhysicsShape* CParaPhysicsWorld::CreateSimpleShape(const ParaPhysicsSimpleShapeDesc& shapeDesc)
{
	BulletSimpleShape* pShape = new BulletSimpleShape();
	if (shapeDesc.m_shape == "box") 
	{   // aabb
		pShape->m_pShape = new btBoxShape(btVector3(shapeDesc.m_halfWidth, shapeDesc.m_halfHeight, shapeDesc.m_halfLength));
	}
	else if (shapeDesc.m_shape == "sphere")
	{
		pShape->m_pShape = new btSphereShape(shapeDesc.m_halfWidth);
	}
	else if (shapeDesc.m_shape == "capsule")
	{
		pShape->m_pShape = new btCapsuleShape(shapeDesc.m_halfWidth, (shapeDesc.m_halfHeight - shapeDesc.m_halfWidth) * 2);
	}
	else 
	{
		pShape->m_pShape = NULL;
	}
	if (pShape->m_pShape == NULL) return NULL;

	pShape->m_pShape->setUserPointer(pShape);

	m_collisionShapes.insert(pShape);
	
	return pShape;
}

IParaPhysicsShape* CParaPhysicsWorld::CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc)
{
	BulletPhysicsShape*  pShape = new BulletPhysicsShape();

	pShape->m_triangleIndices = new int32[12 * meshDesc.m_numTriangles];
	if (meshDesc.m_triangleStrideBytes == 12)
	{
		// 32 bits index
		int32* dest = pShape->m_triangleIndices;
		int32* src = (int32*)(meshDesc.m_triangles);
		if (m_bInvertFaceWinding)
		{
			int nFaceCount = meshDesc.m_numTriangles;
			for (int i = 0; i < nFaceCount; ++i)
			{
				// change the triangle winding order
				*dest = *src; ++src;
				*(dest + 2) = *src; ++src;
				*(dest + 1) = *src; ++src;
				dest += 3;
			}
		}
		else
		{
			memcpy(dest, src, sizeof(int32) * meshDesc.m_numTriangles * 3);
		}
	}
	else
	{
		// 16 bits index
		int32* dest = pShape->m_triangleIndices;
		int16* src = (int16*)(meshDesc.m_triangles);
		if (m_bInvertFaceWinding)
		{
			int nFaceCount = meshDesc.m_numTriangles;
			for (int i = 0; i < nFaceCount; ++i)
			{
				*dest = *src; ++src;
				*(dest + 2) = *src; ++src;
				*(dest + 1) = *src; ++src;
				dest += 3;
			}
		}
		else
		{
			int nFaceCount = meshDesc.m_numTriangles;
			for (int i = 0; i < nFaceCount; ++i)
			{
				*dest = *src; ++src;
				*(dest + 1) = *src; ++src;
				*(dest + 2) = *src; ++src;
				dest += 3;
			}
		}
	}

	pShape->m_vertices = new btScalar[meshDesc.m_pointStrideBytes * meshDesc.m_numVertices / sizeof(btScalar)];
	memcpy(pShape->m_vertices, meshDesc.m_points, meshDesc.m_pointStrideBytes * meshDesc.m_numVertices);

	pShape->m_indexVertexArrays = new btTriangleIndexVertexArray(meshDesc.m_numTriangles,
		(int*)(pShape->m_triangleIndices),
		12,
		meshDesc.m_numVertices, pShape->m_vertices, meshDesc.m_pointStrideBytes);

	bool useQuantizedAabbCompression = true;

	pShape->m_pShape = new btBvhTriangleMeshShape(pShape->m_indexVertexArrays, useQuantizedAabbCompression);
	pShape->m_pShape->setUserPointer(pShape);
	m_collisionShapes.insert(pShape);

	return pShape;
}

void CParaPhysicsWorld::ReleaseShape(IParaPhysicsShape* pShape)
{
	m_collisionShapes.erase((BulletPhysicsShape*)pShape);
	pShape->Release();
}

IParaPhysicsActor* CParaPhysicsWorld::CreateActor(const ParaPhysicsActorDesc& actorDesc)
{
	//rigid body is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (actorDesc.m_mass != 0.f);

	// btCollisionShape* shape = (static_cast<BulletPhysicsShape*>(actorDesc.m_pShape))->m_pShape;
	btCollisionShape* shape = (btCollisionShape*)actorDesc.m_pShape->get();
	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(actorDesc.m_mass, localInertia);

	btMatrix3x3 matRot(
		actorDesc.m_rotation._11, actorDesc.m_rotation._21, actorDesc.m_rotation._31,
		actorDesc.m_rotation._12, actorDesc.m_rotation._22, actorDesc.m_rotation._32,
		actorDesc.m_rotation._13, actorDesc.m_rotation._23, actorDesc.m_rotation._33
		);
	btVector3 vOrigin(actorDesc.m_origin.x, actorDesc.m_origin.y, actorDesc.m_origin.z);
	btTransform startTransform(matRot, vOrigin);

#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(actorDesc.m_mass,myMotionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);

#else
	btRigidBody* body = new btRigidBody(actorDesc.m_mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif

	// create as static object
	if (!isDynamic){
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	}

	// short collisionFilterGroup = isDynamic? short(btBroadphaseProxy::DefaultFilter) : short(btBroadphaseProxy::StaticFilter);
	// short collisionFilterMask = isDynamic? 	short(btBroadphaseProxy::AllFilter) : 	short(btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);

	// please note: group_mask is 2^group_id secretly
	short nGroupMask = 1 << actorDesc.m_group;
	m_dynamicsWorld->addRigidBody(body, nGroupMask, actorDesc.m_mask);

	BulletPhysicsActor* pActor = new BulletPhysicsActor(body);
	
	body->setUserPointer(pActor);

	m_actors.insert(pActor);
	return pActor;
}

void CParaPhysicsWorld::ReleaseActor(IParaPhysicsActor* pActor)
{
	// 先查找避免二次释放, 世界退出自动释放全部, 但上层引用无法感知造成二次释放出错
	if (m_actors.find(pActor) != m_actors.end())
	{
		btCollisionObject* colObj = (btCollisionObject*)(pActor->get());
		if (m_exit)
		{
			m_actors.erase(pActor);
			m_dynamicsWorld->removeCollisionObject(colObj);
			pActor->Release();
		}
		else 
		{
			CParaContactResultCallback callback;
			m_dynamicsWorld->contactTest(colObj, callback);

			m_actors.erase(pActor);
			m_dynamicsWorld->removeCollisionObject(colObj);
			pActor->Release();

			auto it = callback.m_colObj1List.begin();
			while (it != callback.m_colObj1List.end())
			{
				const btCollisionObject* colObj = *it;
				colObj->activate();
				it++;
			}
		}
	}
}

IParaPhysicsActor* ParaEngine::CParaPhysicsWorld::RaycastClosestShape(const PARAVECTOR3& vOrigin, const PARAVECTOR3& vDirection, DWORD dwType, RayCastHitResult& hit, short dwGroupMask, float fSensorRange)
{
	btVector3 vFrom(vOrigin.x, vOrigin.y, vOrigin.z);
	btVector3 vTo(vDirection.x, vDirection.y, vDirection.z);
	dwGroupMask = dwGroupMask ^ (1 << IParaPhysicsGroup::BLOCK);  // 屏蔽地块组
	if (fSensorRange < 0.f)
		fSensorRange = 200.f;
	vTo = vFrom + vTo * fSensorRange;

	btCollisionWorld::ClosestRayResultCallback cb(vFrom, vTo);
	// cb.m_collisionFilterGroup = 0;
	cb.m_collisionFilterMask = dwGroupMask;
	// filter back faces. Added by LiXizhi 2010.3. In bullet, triangle seems to be double sided by default.  
	cb.m_flags = 1;// btTriangleRaycastCallback::kF_FilterBackfaces;

	m_dynamicsWorld->rayTest(vFrom, vTo, cb);
	if (cb.hasHit())
	{
		hit.m_vHitPointWorld = CONVERT_PARAVECTOR3(cb.m_hitPointWorld);
		hit.m_vHitNormalWorld = CONVERT_PARAVECTOR3(cb.m_hitNormalWorld.normalize());
		hit.m_fDistance = cb.m_hitPointWorld.distance(cb.m_rayFromWorld);
		return (IParaPhysicsActor*)(cb.m_collisionObject->getUserPointer());
	}
	else
	{
		hit.m_vHitPointWorld = CONVERT_PARAVECTOR3(vTo);
		hit.m_vHitNormalWorld = PARAVECTOR3(1.0f, 1.0f, 1.0f); //CONVERT_PARAVECTOR3(btVector3(1.0, 0.0, 0.0));
		return NULL;
	}
}

void ParaEngine::CParaPhysicsWorld::SetDebugDrawer(IParaDebugDraw* debugDrawer)
{
	m_physics_debug_draw.SetParaDebugDrawInterface(debugDrawer);
}


IParaDebugDraw* ParaEngine::CParaPhysicsWorld::GetDebugDrawer()
{
	return m_physics_debug_draw.GetParaDebugDrawInterface();
}

void ParaEngine::CParaPhysicsWorld::DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color)
{

}

void ParaEngine::CParaPhysicsWorld::DebugDrawWorld()
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->debugDrawWorld();
	}
}

void ParaEngine::CParaPhysicsWorld::SetDebugDrawMode(int debugMode)
{
	m_physics_debug_draw.setDebugMode(debugMode);
}

int ParaEngine::CParaPhysicsWorld::GetDebugDrawMode()
{
	return m_physics_debug_draw.getDebugMode();
}

//////////////////////////////////////////////////////////////////////////
// Constraint/Joint Implementation
//////////////////////////////////////////////////////////////////////////

IParaPhysicsConstraint* CParaPhysicsWorld::CreateConstraint(const ParaPhysicsConstraintDesc& constraintDesc)
{
	if (!m_dynamicsWorld || !constraintDesc.m_pActorA)
		return nullptr;

	btRigidBody* bodyA = static_cast<btRigidBody*>(constraintDesc.m_pActorA->get());
	btRigidBody* bodyB = constraintDesc.m_pActorB ? static_cast<btRigidBody*>(constraintDesc.m_pActorB->get()) : nullptr;

	if (!bodyA)
		return nullptr;

	btTypedConstraint* constraint = nullptr;

	btVector3 pivotA(constraintDesc.m_pivotInA.x, constraintDesc.m_pivotInA.y, constraintDesc.m_pivotInA.z);
	btVector3 axisA(constraintDesc.m_axisInA.x, constraintDesc.m_axisInA.y, constraintDesc.m_axisInA.z);
	btVector3 pivotB(constraintDesc.m_pivotInB.x, constraintDesc.m_pivotInB.y, constraintDesc.m_pivotInB.z);
	btVector3 axisB(constraintDesc.m_axisInB.x, constraintDesc.m_axisInB.y, constraintDesc.m_axisInB.z);

	switch (constraintDesc.m_type)
	{
	case ConstraintType_PointToPoint:
	{
		if (bodyB)
		{
			constraint = new btPoint2PointConstraint(*bodyA, *bodyB, pivotA, pivotB);
		}
		else
		{
			constraint = new btPoint2PointConstraint(*bodyA, pivotA);
		}
		break;
	}
	case ConstraintType_Hinge:
	{
		if (bodyB)
		{
			constraint = new btHingeConstraint(*bodyA, *bodyB, pivotA, pivotB, axisA, axisB);
		}
		else
		{
			constraint = new btHingeConstraint(*bodyA, pivotA, axisA);
		}
		btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint);
		if (constraintDesc.m_lowLimit != 0.0f || constraintDesc.m_highLimit != 0.0f)
		{
			hinge->setLimit(constraintDesc.m_lowLimit, constraintDesc.m_highLimit, 
				constraintDesc.m_softness, constraintDesc.m_biasFactor, constraintDesc.m_relaxationFactor);
		}
		if (constraintDesc.m_enableMotor)
		{
			hinge->enableAngularMotor(true, constraintDesc.m_motorTargetVelocity, constraintDesc.m_maxMotorImpulse);
		}
		break;
	}
	case ConstraintType_Slider:
	{
		btTransform frameA, frameB;
		frameA.setIdentity();
		frameA.setOrigin(pivotA);
		frameB.setIdentity();
		frameB.setOrigin(pivotB);

		if (bodyB)
		{
			constraint = new btSliderConstraint(*bodyA, *bodyB, frameA, frameB, true);
		}
		else
		{
			constraint = new btSliderConstraint(*bodyA, frameA, true);
		}
		btSliderConstraint* slider = static_cast<btSliderConstraint*>(constraint);
		slider->setLowerLinLimit(constraintDesc.m_lowLimit);
		slider->setUpperLinLimit(constraintDesc.m_highLimit);
		if (constraintDesc.m_enableMotor)
		{
			slider->setPoweredLinMotor(true);
			slider->setTargetLinMotorVelocity(constraintDesc.m_motorTargetVelocity);
			slider->setMaxLinMotorForce(constraintDesc.m_maxMotorImpulse);
		}
		break;
	}
	case ConstraintType_ConeTwist:
	{
		btTransform frameA, frameB;
		frameA.setIdentity();
		frameA.setOrigin(pivotA);
		frameB.setIdentity();
		frameB.setOrigin(pivotB);

		if (bodyB)
		{
			constraint = new btConeTwistConstraint(*bodyA, *bodyB, frameA, frameB);
		}
		else
		{
			constraint = new btConeTwistConstraint(*bodyA, frameA);
		}
		break;
	}
	case ConstraintType_Generic6Dof:
	{
		btTransform frameA, frameB;
		frameA.setIdentity();
		frameA.setOrigin(pivotA);
		frameB.setIdentity();
		frameB.setOrigin(pivotB);

		if (bodyB)
		{
			constraint = new btGeneric6DofConstraint(*bodyA, *bodyB, frameA, frameB, true);
		}
		else
		{
			constraint = new btGeneric6DofConstraint(*bodyA, frameA, true);
		}
		break;
	}
	case ConstraintType_Fixed:
	{
		btTransform frameA, frameB;
		frameA.setIdentity();
		frameA.setOrigin(pivotA);
		frameB.setIdentity();
		frameB.setOrigin(pivotB);

		if (bodyB)
		{
			constraint = new btFixedConstraint(*bodyA, *bodyB, frameA, frameB);
		}
		break;
	}
	default:
		return nullptr;
	}

	if (!constraint)
		return nullptr;

	if (constraintDesc.m_breakingThreshold > 0)
	{
		constraint->setBreakingImpulseThreshold(constraintDesc.m_breakingThreshold);
	}

	m_dynamicsWorld->addConstraint(constraint, constraintDesc.m_disableCollisionsBetweenBodies);

	BulletPhysicsConstraint* pConstraint = new BulletPhysicsConstraint(constraint, constraintDesc.m_type,
		constraintDesc.m_pActorA, constraintDesc.m_pActorB);
	constraint->setUserConstraintPtr(pConstraint);
	m_constraints.insert(pConstraint);

	return pConstraint;
}

void CParaPhysicsWorld::ReleaseConstraint(IParaPhysicsConstraint* pConstraint)
{
	if (!pConstraint)
		return;

	auto it = m_constraints.find(pConstraint);
	if (it != m_constraints.end())
	{
		btTypedConstraint* btConstraint = static_cast<btTypedConstraint*>(pConstraint->get());
		if (btConstraint && m_dynamicsWorld)
		{
			m_dynamicsWorld->removeConstraint(btConstraint);
		}
		m_constraints.erase(it);
		pConstraint->Release();
	}
}

//////////////////////////////////////////////////////////////////////////
// Vehicle Implementation
//////////////////////////////////////////////////////////////////////////

IParaPhysicsVehicle* CParaPhysicsWorld::CreateVehicle(IParaPhysicsActor* pChassisActor)
{
	if (!m_dynamicsWorld || !pChassisActor)
		return nullptr;

	btRigidBody* chassis = static_cast<btRigidBody*>(pChassisActor->get());
	if (!chassis)
		return nullptr;

	// Deactivate chassis deactivation so the vehicle stays active
	chassis->setActivationState(DISABLE_DEACTIVATION);

	btRaycastVehicle::btVehicleTuning tuning;
	btVehicleRaycaster* raycaster = new btDefaultVehicleRaycaster(m_dynamicsWorld);
	btRaycastVehicle* vehicle = new btRaycastVehicle(tuning, chassis, raycaster);

	// Never deactivate the vehicle
	vehicle->setCoordinateSystem(0, 1, 2);

	m_dynamicsWorld->addVehicle(vehicle);

	BulletPhysicsVehicle* pVehicle = new BulletPhysicsVehicle(vehicle, raycaster, pChassisActor);
	m_vehicles.insert(pVehicle);

	return pVehicle;
}

void CParaPhysicsWorld::ReleaseVehicle(IParaPhysicsVehicle* pVehicle)
{
	if (!pVehicle)
		return;

	auto it = m_vehicles.find(pVehicle);
	if (it != m_vehicles.end())
	{
		btRaycastVehicle* btVehicle = static_cast<btRaycastVehicle*>(pVehicle->get());
		if (btVehicle && m_dynamicsWorld)
		{
			m_dynamicsWorld->removeVehicle(btVehicle);
		}
		m_vehicles.erase(it);
		pVehicle->Release();
	}
}

//-----------------------------------------------------------------------------
// Class:	ParaEngine physics wrapper for the bullet physics engine DLL
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.2.24
//-----------------------------------------------------------------------------
#include "PluginAPI.h"
#include "ParaPhysicsWorld.h"

/// @def using motion state is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
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

void ParaEngine::BulletPhysicsActor::Release()
{
	delete this;
}
//
// Physics World
//
CParaPhysicsWorld::CParaPhysicsWorld()
	: m_dynamicsWorld(NULL), m_collisionWorld(NULL), m_broadphase(NULL), m_dispatcher(NULL), m_solver(NULL), m_collisionConfiguration(NULL), m_bInvertFaceWinding(false)
{
#ifdef WIN32
	m_bInvertFaceWinding = true;
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

	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->setDebugDrawer(&m_physics_debug_draw);
	}
	return true;
}

bool CParaPhysicsWorld::StepSimulation(float fDeltaTime)
{
	m_dynamicsWorld->stepSimulation(fDeltaTime);
	return true;
}

bool CParaPhysicsWorld::ExitPhysics()
{
	if (m_dynamicsWorld == 0)
		return true;

	//cleanup in the reverse order of creation/initialization
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
		BulletPhysicsShape_Array_Type::iterator itCur, itEnd = m_collisionShapes.end();
		for (itCur = m_collisionShapes.begin(); itCur != itEnd; ++itCur)
		{
			BulletPhysicsShape* shape = (*itCur);
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


IParaPhysicsShape* CParaPhysicsWorld::CreateTriangleMeshShap(const ParaPhysicsTriangleMeshDesc& meshDesc)
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

	btCollisionShape* shape = (static_cast<BulletPhysicsShape*>(actorDesc.m_pShape))->m_pShape;
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
	short nGroupMask = 1;
	for (int i = 0; i < actorDesc.m_group; ++i)
	{
		nGroupMask *= 2;
	}

	m_dynamicsWorld->addRigidBody(body, nGroupMask, actorDesc.m_mask);

	BulletPhysicsActor* pActor = new BulletPhysicsActor(body);
	body->setUserPointer(pActor);

	m_actors.insert(pActor);

	return pActor;
}

void CParaPhysicsWorld::ReleaseActor(IParaPhysicsActor* pActor)
{
	m_dynamicsWorld->removeCollisionObject((btCollisionObject*)(pActor->get()));
	pActor->Release();
	m_actors.erase((BulletPhysicsActor*)pActor);
}

IParaPhysicsActor* ParaEngine::CParaPhysicsWorld::RaycastClosestShape(const PARAVECTOR3& vOrigin, const PARAVECTOR3& vDirection, DWORD dwType, RayCastHitResult& hit, short dwGroupMask, float fSensorRange)
{
	btVector3 vFrom(vOrigin.x, vOrigin.y, vOrigin.z);
	btVector3 vTo(vDirection.x, vDirection.y, vDirection.z);

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
		btVector3 tem(1.0, 0.0, 0.0);
		hit.m_vHitNormalWorld = CONVERT_PARAVECTOR3(tem);
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
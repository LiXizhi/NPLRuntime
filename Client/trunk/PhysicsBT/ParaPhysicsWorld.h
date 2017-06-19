#pragma once

#include "btBulletDynamicsCommon.h"
#include "IParaPhysics.h"
#include "PhysicsDebugDraw.h"

#include <set>
#include <list>

#define CONVERT_BTVECTOR3(x) *((btVector3*)&(x))

#define CONVERT_PARAVECTOR3(x) *((PARAVECTOR3*)&(x))



/**
NxScene <--------> m_dynamicsWorld, m_collisionWorld
NxTriangleMeshDesc  <--------> btCollisionShape
NxActor  <--------> btRigidBody
*/
namespace ParaEngine
{
	/** it is represent a shape that can be used to create various actors in the scene. */
	struct BulletPhysicsShape : public IParaPhysicsShape
	{
		BulletPhysicsShape();
		~BulletPhysicsShape();

		virtual void* GetUserData() {return m_pUserData;};
		virtual void SetUserData(void* pData) {m_pUserData = pData;};

		virtual void* get() {return m_pShape;};
		virtual void Release();

		/// pointer to the low level physics engine shape object. 
		btCollisionShape* m_pShape;
		btTriangleIndexVertexArray* m_indexVertexArrays;
		int32* m_triangleIndices;
		btScalar* m_vertices;

		/// keep some user data here
		void* m_pUserData;
	};

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct BulletPhysicsActor :IParaPhysicsActor
	{
		BulletPhysicsActor(btRigidBody* pActor);
		~BulletPhysicsActor();
		virtual void* GetUserData() {return m_pUserData;};
		virtual void SetUserData(void* pData) {m_pUserData = pData;};
		virtual void* get() {return m_pActor;};
		virtual void Release();

		/// pointer to the low level physics engine actor(rigid body). 
		btRigidBody* m_pActor;
		/// keep some user data here
		void* m_pUserData;
	};

	using namespace std;

	class CParaPhysicsWorld :public IParaPhysics
	{
	public:
		typedef std::set<BulletPhysicsActor*> BulletPhysicsActor_Map_Type;
		typedef std::set<BulletPhysicsShape*> BulletPhysicsShape_Array_Type;
		CParaPhysicsWorld();
		virtual ~CParaPhysicsWorld();

		/** create and initialize all physics lib and create the default scene(world) */
		virtual bool InitPhysics();

		/** step simulation 
		* @param fDeltaTime: in seconds. 
		*/
		virtual bool StepSimulation(float fDeltaTime);

		/** cleanup all physics entities. 
		*/
		virtual bool ExitPhysics();

		/** all this to unload the IPhysics. Pointer to this class will be invalid after the call
		*/
		virtual void Release();

		/** create a triangle shape.
		* @return: the triangle shape pointer is returned. 
		*/
		virtual IParaPhysicsShape* CreateTriangleMeshShap(const ParaPhysicsTriangleMeshDesc& meshDesc);

		/** release a physics shape */
		virtual void ReleaseShape(IParaPhysicsShape* pShape);

		/** create an physics actor(rigid body) in the current world.*/
		virtual IParaPhysicsActor* CreateActor(const ParaPhysicsActorDesc& meshDesc);

		/** release a physics actor. */
		virtual void ReleaseActor(IParaPhysicsActor* pActor);
		

		/** ray cast a given group. */
		virtual IParaPhysicsActor* RaycastClosestShape(const PARAVECTOR3& vOrigin, const PARAVECTOR3& vDirection, DWORD dwType, RayCastHitResult& hit, short dwGroupMask, float fSensorRange);

		/** set the debug draw object for debugging physics world. */
		virtual void	SetDebugDrawer(IParaDebugDraw*	debugDrawer);

		/** Get the debug draw object for debugging physics world. */
		virtual IParaDebugDraw*	GetDebugDrawer();

		/** draw a given object. */
		virtual void DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color);

		/** draw the entire physics debug world.*/
		virtual void DebugDrawWorld();

		/** bitwise of PhysicsDebugDrawModes */
		virtual void	SetDebugDrawMode(int debugMode);

		/** bitwise of PhysicsDebugDrawModes */
		virtual int		GetDebugDrawMode();
	public:
		/** get a pointer to physics scene object */
		virtual btDynamicsWorld* GetScene()
		{
			return m_dynamicsWorld;
		}

	protected:
		btBroadphaseInterface*	m_broadphase;
		btCollisionDispatcher*	m_dispatcher;
		btConstraintSolver*	m_solver;
		btDefaultCollisionConfiguration* m_collisionConfiguration;

		btDiscreteDynamicsWorld* m_dynamicsWorld;
		btCollisionWorld* m_collisionWorld;

		///keep the collision shapes, for deletion/cleanup
		BulletPhysicsShape_Array_Type	m_collisionShapes;

		// keep all actors
		BulletPhysicsActor_Map_Type m_actors;

		CPhysicsDebugDraw m_physics_debug_draw;
		bool m_bInvertFaceWinding;
	};
}
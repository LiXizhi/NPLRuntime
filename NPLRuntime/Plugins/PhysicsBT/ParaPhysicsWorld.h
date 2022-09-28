#pragma once

#include "btBulletCollisionCommon.h"
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

		virtual void* get() {return m_pShape;};
		virtual void Release();

		/// pointer to the low level physics engine shape object. 
		btCollisionShape* m_pShape;
		btTriangleIndexVertexArray* m_indexVertexArrays;
		int32* m_triangleIndices;
		btScalar* m_vertices;
	};

	struct BulletSimpleShape : public IParaPhysicsShape
	{
		BulletSimpleShape(): m_pShape(NULL) {}
		~BulletSimpleShape() { SAFE_DELETE(m_pShape); }

		virtual void* get() {return m_pShape;};
		virtual void Release() { delete this; }

		btCollisionShape* m_pShape;
	};

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct BulletPhysicsActor : public IParaPhysicsActor
	{
		BulletPhysicsActor(btRigidBody* pActor);
		~BulletPhysicsActor();

		// 设置获取物理矩阵  float[16]
		virtual PARAMATRIX* GetWorldTransform(PARAMATRIX* pOut);
		virtual void SetWorldTransform(const PARAMATRIX* pMatrix);

		virtual void* get() {return m_pActor;};
		virtual void Release();
		virtual void ApplyCentralImpulse(const PARAVECTOR3& impulse);
		virtual PARAVECTOR3 GetOrigin();

		virtual void Activate();
		virtual bool IsActive();
		virtual bool IsStaticObject();
		virtual bool IsKinematicObject();
		virtual bool IsStaticOrKinematicObject();
		
		virtual float GetMass();
		virtual void SetMass(float mass);
		virtual PARAVECTOR3 GetLocalInertia();
		virtual void SetLocalInertia(const PARAVECTOR3& inertia);
		virtual PARAVECTOR3 GetGravity();
		virtual void SetGravity(const PARAVECTOR3& gravity);
		virtual float GetLinearDamping();
		virtual void SetLinearDamping(float damping);
		virtual float GetAngularDamping();
		virtual void SetAngularDamping(float damping);
		virtual PARAVECTOR3 GetLinearFactor();
		virtual void SetAngularFactor(const PARAVECTOR3& factor);
		virtual PARAVECTOR3 GetAngularFactor();
		virtual void SetLinearFactor(const PARAVECTOR3& factor);
		virtual PARAVECTOR3 GetLinearVelocity();
		virtual void SetLinearVelocity(const PARAVECTOR3& velocity);
		virtual PARAVECTOR3 GetAngularVelocity();
		virtual void SetAngularVelocity(const PARAVECTOR3& velocity);
		virtual int GetFlags();
		virtual void SetFlags(int flags);
		virtual int GetActivationState();
		virtual void SetActivationState(int state);
		virtual float GetDeactivationTime();
		virtual void SetDeactivationTime(float time);
		virtual float GetRestitution();
		virtual void SetRestitution(float restitution);
		virtual float GetFriction();
		virtual void SetFriction(float friction); 
		virtual float GetRollingFriction();
		virtual void SetRollingFriction(float friction); 
		virtual float GetSpinningFriction();
		virtual void SetSpinningFriction(float friction);
		virtual float GetContactStiffness();
		virtual void SetContactStiffness(float stiffness);
		virtual float GetContactDamping();
		virtual void SetContactDamping(float damping);
		virtual int GetIslandTag();
		virtual void SetIslandTag(int flags);
		virtual int GetCompanionId();
		virtual void SetCompanionId(int id);
		virtual float GetHitFraction();
		virtual void SetHitFraction(float fraction);
		virtual int GetCollisionFlags();
		virtual void SetCollisionFlags(int flags);
		virtual float GetCcdSweptSphereRadius();
		virtual void SetCcdSweptSphereRadius(float radius);
		virtual float GetCcdMotionThreshold();
		virtual void SetCcdMotionThreshold(float threshold);
		
		btRigidBody* m_pActor;
	};

	using namespace std;

	class CParaPhysicsWorld :public IParaPhysics
	{
	public:
		// typedef std::set<BulletPhysicsActor*> BulletPhysicsActor_Map_Type;
		// typedef std::set<BulletPhysicsShape*> BulletPhysicsShape_Array_Type;
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
		virtual IParaPhysicsShape* CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc);
		virtual IParaPhysicsShape* CreateSimpleShape(const ParaPhysicsSimpleShapeDesc& shapeDesc);

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
		IParaPhysicsShape_Array_Type	m_collisionShapes;

		// keep all actors
		IParaPhysicsActor_Map_Type m_actors;

		CPhysicsDebugDraw m_physics_debug_draw;
		bool m_bInvertFaceWinding;
		bool m_exit;
	};

	class CParaContactResultCallback: public btCollisionWorld::ContactResultCallback
	{
	public:
		virtual	btScalar addSingleResult(btManifoldPoint& cp,	const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
		{
			const btCollisionObject* colObj0 = colObj0Wrap->getCollisionObject();
			const btCollisionObject* colObj1 = colObj1Wrap->getCollisionObject();
			m_colObj0List.push_back(colObj0);
			m_colObj1List.push_back(colObj1);
			return btScalar(0.f);
		}

		std::vector<const btCollisionObject*> m_colObj0List;
		std::vector<const btCollisionObject*> m_colObj1List;
	};
}
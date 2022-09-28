#pragma once
#include "PEtypes.h"
#include <set>
#include <list>
#include <string>

namespace ParaEngine
{
	class IParaDebugDraw;

	enum PhysicsDebugDrawModes
	{
		PDDM_NoDebug=0,
		PDDM_DrawWireframe = 1,
		PDDM_DrawAabb=2,
		PDDM_DrawFeaturesText=4,
		PDDM_DrawContactPoints=8,
		PDDM_NoDeactivation=16,
		PDDM_NoHelpText = 32,
		PDDM_DrawText=64,
		PDDM_ProfileTimings = 128,
		PDDM_EnableSatComparison = 256,
		PDDM_DisableBulletLCP = 512,
		PDDM_EnableCCD = 1024,
		PDDM_DrawConstraints = (1 << 11),
		PDDM_DrawConstraintLimits = (1 << 12),
		PDDM_FastWireframe = (1<<13),
		PDDM_MAX_DEBUG_DRAW_MODE
	};

	/** Create descriptor for triangle mesh shape. so that we can create it.
	*/
	struct ParaPhysicsTriangleMeshDesc
	{
		DWORD m_numVertices;
		DWORD m_numTriangles;
		DWORD m_pointStrideBytes;
		DWORD m_triangleStrideBytes;
		void* m_points;
		void* m_triangles;
		DWORD m_flags;
	};

	struct ParaPhysicsSimpleShapeDesc
	{
		std::string m_shape;
		float m_halfWidth;     // box.x  sphere.r  capsule.r
		float m_halfHeight;    // box.y	 sphere.r  capsule.h(胶囊圆柱高, 含半球高)
		float m_halfLength;    // box.z  sphere.r  capsule.r
	};

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct IParaPhysicsShape
	{
		IParaPhysicsShape()
		{
			m_pUserData = NULL;
		}

		/// get user data associated with the shape
		virtual void* GetUserData() { return m_pUserData; }
		virtual void SetUserData(void* pData) { m_pUserData = pData; }
		
		/// return pointer to the low level physics engine shape object. 
		virtual void* get() = 0;
		virtual void Release() = 0;

		void* m_pUserData;
	};

	// 物理模型分组
	enum IParaPhysicsGroup
	{
		// 默认, 动态物理组
		DEFAULT = 0,  // 1 << 0
		// 静态物理组
		STATIC = 1,
		// 地块组
		BLOCK = 15,
	};

	/** Create descriptor for a physics actor. so that we can create it.
	*/
	struct ParaPhysicsActorDesc
	{
		ParaPhysicsActorDesc():m_mass(0.f), m_group(1), m_mask(-1), m_pShape(NULL) {}
		///Storage for the translation
		PARAVECTOR3 m_origin;

		///Storage for the rotation
		PARAMATRIX3x3 m_rotation;

		/// if mass is zero, it is a static mesh. 
		float m_mass;

		/// group id
		short m_group;

		/// collision mask id, -1 to collide with all meshes. specify (-1 ^ m_group) to exclude collision of the same type. 
		short m_mask;

		/// the collision shape of the actor. it should be the pointer returned by one of the CreateXXXShape() functions. 
		IParaPhysicsShape* m_pShape;
	};

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct IParaPhysicsActor
	{
		IParaPhysicsActor() 
		{
			m_pUserData = NULL;
		}
		~IParaPhysicsActor() 
		{
		}
		/// get user data associated with the shape
		virtual void* GetUserData() { return m_pUserData; }
		virtual void SetUserData(void* pData) { m_pUserData = pData; }

		// 设置获取物理矩阵
		virtual PARAMATRIX* GetWorldTransform(PARAMATRIX* pOut) { return pOut; }
		virtual void SetWorldTransform(const PARAMATRIX* pMatrix) {}

		/// return pointer to the low level physics engine shape object. 
		virtual void* get() = 0;
		virtual void Release() = 0;
		virtual void ApplyCentralImpulse(const PARAVECTOR3& impulse) {}
		virtual PARAVECTOR3 GetOrigin() { return PARAVECTOR3(); }

		virtual void Activate() = 0;
		virtual bool IsActive() = 0;
		virtual bool IsStaticObject() = 0;
		virtual bool IsKinematicObject() = 0;
		virtual bool IsStaticOrKinematicObject() = 0;
		
		virtual float GetMass() = 0;
		virtual void SetMass(float mass) = 0;
		virtual PARAVECTOR3 GetLocalInertia() = 0;
		virtual void SetLocalInertia(PARAVECTOR3& inertia) = 0;
		virtual PARAVECTOR3 GetGravity() = 0;
		virtual void SetGravity(PARAVECTOR3& gravity) = 0;
		virtual float GetLinearDamping() = 0;
		virtual void SetLinearDamping(float damping) = 0;
		virtual float GetAngularDamping() = 0;
		virtual void SetAngularDamping(float damping) = 0;
		virtual PARAVECTOR3 GetLinearFactor() = 0;
		virtual void SetAngularFactor(PARAVECTOR3& factor) = 0;
		virtual PARAVECTOR3 GetAngularFactor() = 0;
		virtual void SetLinearFactor(PARAVECTOR3& factor) = 0;
		virtual PARAVECTOR3 GetLinearVelocity() = 0;
		virtual void SetLinearVelocity(PARAVECTOR3& velocity) = 0;
		virtual PARAVECTOR3 GetAngularVelocity() = 0;
		virtual void SetAngularVelocity(PARAVECTOR3& velocity) = 0;
		virtual int GetFlags() = 0;
		virtual void SetFlags(int flags) = 0;
		virtual int GetActivationState() = 0;
		virtual void SetActivationState(int state) = 0;
		virtual float GetDeactivationTime() = 0;
		virtual void SetDeactivationTime(float time) = 0;
		virtual float GetRestitution() = 0;
		virtual void SetRestitution(float restitution) = 0;
		virtual float GetFriction() = 0;
		virtual void SetFriction(float friction) = 0; 
		virtual float GetRollingFriction() = 0;
		virtual void SetRollingFriction(float friction) = 0; 
		virtual float GetSpinningFriction() = 0;
		virtual void SetSpinningFriction(float friction) = 0;
		virtual float GetContactStiffness() = 0;
		virtual void SetContactStiffness(float stiffness) = 0;
		virtual float GetContactDamping() = 0;
		virtual void SetContactDamping(float damping) = 0;
		virtual int GetIslandTag() = 0;
		virtual void SetIslandTag(int flags) = 0;
		virtual int GetCompanionId() = 0;
		virtual void SetCompanionId(int id) = 0;
		virtual float GetHitFraction() = 0;
		virtual void SetHitFraction(float fraction) = 0;
		virtual int GetCollisionFlags() = 0;
		virtual void SetCollisionFlags(int flags) = 0;
		virtual float GetCcdSweptSphereRadius() = 0;
		virtual void SetCcdSweptSphereRadius(float radius) = 0;
		virtual float GetCcdMotionThreshold() = 0;
		virtual void SetCcdMotionThreshold(float threshold) = 0;

		void* m_pUserData;
	};

	struct RayCastHitResult
	{
		float m_fDistance;
		PARAVECTOR3 m_vHitPointWorld;
		PARAVECTOR3 m_vHitNormalWorld;
	};

	/** ParaPhysics core interface. 
	*/
	class IParaPhysics
	{
	public:
		/** create and initialize all physics lib and create the default scene(world) */
		virtual bool InitPhysics() = 0;

		/** step simulation 
		* @param fDeltaTime: in seconds. 
		*/
		virtual bool StepSimulation(float fDeltaTime) = 0;

		/** cleanup all physics entities. 
		*/
		virtual bool ExitPhysics() = 0;

		/** all this to unload the IPhysics. Pointer to this class will be invalid after the call
		*/
		virtual void Release() = 0;

		/** create a triangle shape.
		* @return: the triangle shape pointer is returned. 
		*/
		virtual IParaPhysicsShape* CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc) = 0;

		virtual IParaPhysicsShape* CreateSimpleShape(const ParaPhysicsSimpleShapeDesc& shapeDesc) { return NULL; }

		/** release a physics shape */
		virtual void ReleaseShape(IParaPhysicsShape *pShape) = 0;

		/** create an physics actor(rigid body) in the current world.*/
		virtual IParaPhysicsActor* CreateActor(const ParaPhysicsActorDesc& meshDesc) = 0;

		/** release a physics actor */
		virtual void ReleaseActor(IParaPhysicsActor* pActor) = 0;

		/** ray cast a given group. */
		virtual IParaPhysicsActor* RaycastClosestShape(const PARAVECTOR3& vOrigin, const PARAVECTOR3& vDirection, DWORD dwType, RayCastHitResult& hit, short dwGroupMask, float fSensorRange) = 0;

		/** set the debug draw object for debugging physics world. */
		virtual void	SetDebugDrawer(IParaDebugDraw*	debugDrawer) = 0;
		
		/** Get the debug draw object for debugging physics world. */
		virtual IParaDebugDraw*	GetDebugDrawer() = 0;

		/** draw a given object. */
		virtual void DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color) = 0;

		/** draw the entire physics debug world.*/
		virtual void DebugDrawWorld() = 0;

		/** bitwise of PhysicsDebugDrawModes */
		virtual void	SetDebugDrawMode(int debugMode) = 0;

		/** bitwise of PhysicsDebugDrawModes */
		virtual int		GetDebugDrawMode() = 0;
	};

	typedef std::set<IParaPhysicsActor*> IParaPhysicsActor_Map_Type;
	typedef std::set<IParaPhysicsShape*> IParaPhysicsShape_Array_Type;
}
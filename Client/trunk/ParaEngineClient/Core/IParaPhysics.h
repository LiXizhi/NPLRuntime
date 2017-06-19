#pragma once
#include "PEtypes.h"

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

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct IParaPhysicsShape
	{
		/// get user data associated with the shape
		virtual void* GetUserData() = 0;
		virtual void SetUserData(void* pData) = 0;
		
		/// return pointer to the low level physics engine shape object. 
		virtual void* get() = 0;

		virtual void Release() = 0;
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
		/// get user data associated with the shape
		virtual void* GetUserData() = 0;
		virtual void SetUserData(void* pData) = 0;

		/// return pointer to the low level physics engine shape object. 
		virtual void* get() = 0;

		virtual void Release() = 0;
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
		virtual IParaPhysicsShape* CreateTriangleMeshShap(const ParaPhysicsTriangleMeshDesc& meshDesc) = 0;

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
}
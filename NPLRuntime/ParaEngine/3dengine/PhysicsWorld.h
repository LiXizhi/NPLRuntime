#pragma once
#include "IParaPhysics.h"

#include <list>
#include <unordered_map>
#include <string>

/** different physics engine has different winding order. */
// #define INVERT_PHYSICS_FACE_WINDING

namespace ParaEngine
{
	struct MeshEntity;
	struct ParaXEntity;
	class CShapeAABB;
	class BlockModel;

	class CPhysicsBlockShape
	{
	public:
		CPhysicsBlockShape(): m_shape(nullptr) {}
		~CPhysicsBlockShape() {  }

		static const char* GetStdCubeHash() { return "std_cube_hash"; }
		bool IsStdCube() { return m_hash == GetStdCubeHash(); }

		IParaPhysicsShape* m_shape; 
		std::string m_hash;
	};

	class CPhysicsBlock
	{
	public:
		CPhysicsBlock(uint64_t id, uint32_t key)
		{
			m_id = id;
			m_key = key;
			m_frameId = 0;
			m_actor = nullptr;
			m_world = nullptr;
		}
		
		~CPhysicsBlock() 
		{
			Unload();
		}

		uint64_t GetID() { return m_id; }

		void SetFrameId(int16_t frameId) { m_frameId = frameId; }
		int16_t GetFrameId() { return m_frameId; }
		void SetKey(uint32_t key) { m_key = key; }
		uint32_t GetKey() { return m_key; }

		static uint64_t PackID(uint16_t bx, uint16_t by, uint16_t bz)
		{
			return (((uint64)by)<<32) + (((uint64)bx) << 16)  + bz;
		}

		static void UnPackID(uint64_t id, uint16_t& bx, uint16_t& by, uint16_t& bz)
		{	
			bz = (uint16_t)(id & 0xffff);
			bx = (uint16_t)((id >> 16) & 0xffff);
			by = (uint16_t)((id >> 32) & 0xffff);
		}

		std::shared_ptr<CPhysicsBlockShape> GetShape(BlockModel& model, IParaPhysics* world);

		static std::unordered_map<uint32_t, uint16_t>* GetShapeIndexMap() 
		{
			static std::unordered_map<uint32_t, uint16_t> s_physics_block_shape_index_map;
			return &s_physics_block_shape_index_map;
		}

		static std::vector<std::shared_ptr<CPhysicsBlockShape>>* GetShapeList() 
		{
			static std::vector<std::shared_ptr<CPhysicsBlockShape>> s_physics_block_shape_list;
			return &s_physics_block_shape_list;
		}

		bool IsLoaded() { return m_actor != nullptr; }
		IParaPhysicsActor* GetActor() { return m_actor; }

		void Load(BlockModel& model, IParaPhysics* world);
		void Unload();
	private:
		uint64_t m_id;   
		uint32_t m_key;
		IParaPhysicsActor* m_actor;  
		IParaPhysics* m_world;
		int16_t m_frameId;
	};

	/**
	* The global physics scene (NxScene) and physics SDK is encapsulated in a member object of scene manager. 
	* It is called CPhysicsWorld. The environment simulator can retrieve the physics scene from this object. 
	* Other physics actor scene object can access this global physics objects via the scene manager.
	*/
	class CPhysicsWorld : public IAttributeFields
	{
	public:
		class TriangleMeshShape;
		typedef std::list<TriangleMeshShape*> TriangleMeshShape_Map_Type;

		struct SubMeshPhysicsShape
		{
		public:
			SubMeshPhysicsShape():m_nPhysicsGroup(0), m_pShape(NULL){};
			SubMeshPhysicsShape(IParaPhysicsShape* pShape, int nPhysicsGroup=0):m_nPhysicsGroup(nPhysicsGroup), m_pShape(pShape){};
			IParaPhysicsShape* m_pShape;
			int m_nPhysicsGroup;
		};
		/**
		* the triangle mesh shape.
		* since we need to create separate physics mesh with different scaling factors even for the same mesh model,
		* we will need the mesh entity plus the scaling vector to fully specify the mesh object
		*/
		class TriangleMeshShape
		{
		public:
			union {
				MeshEntity* m_pMeshEntity;	/// the mesh entity
				ParaXEntity* m_pParaXEntity;	/// the parax entity
			};
			
			Vector3 m_vScale;		/// the scaling factor
			std::vector<SubMeshPhysicsShape> m_pShapes;
		
			TriangleMeshShape ():m_pMeshEntity(NULL), m_vScale (1.f,1.f,1.f){}
		};

	public:
		void SetActorPhysicsProperty(IParaPhysicsActor* actor, const char* property);
		const char* GetActorPhysicsProperty(IParaPhysicsActor* actor);

	public:
		CPhysicsWorld(void);
		virtual ~CPhysicsWorld(void);
	
		ATTRIBUTE_DEFINE_CLASS(CPhysicsWorld);

		/**
		* Init the physics scene
		*/
		void InitPhysics();

		/** 
		* ExitPhysics() calls NxPhysicsSDK::releaseScene() which deletes all the objects in the scene and then 
		* deletes the scene itself.  It then calls NxPhysicsSDK::release() which shuts down the SDK.
		* ExitPhysics() is also called after glutMainLoop() to shut down the SDK before exiting.
		*/
		void ExitPhysics();

		/** First call ExitPhysics(), then InitPhysics() */
		void ResetPhysics();

		/** Start the physics for some time advances
		* @params dTime: advances in seconds */
		void StepSimulation(double dTime);

		/**
		* create a static actor in the physical world from a mesh entity.
		* @params ppMesh: the mesh entity, it must contain non-material faces. 
		* @params globalMat: the facing around the y axis.
		* @params nShapeGroup: default to 0, must be smaller than 32. 
			please see groups Mask used to filter shape objects. See #NxShape::setGroup
			group 0 means physics object that will block the player, but not the camera, such as small stones, thin posts, trees, etc. 
			group 1 means physics object that will block the camera and player, such as building walls, big tree trunks, etc. 
			group 2 means physics object that will block the camera only. 
		* @params pOutputPhysicsActor: in case the ppMesh contains multiple actors, it will be returned and added to pOutputPhysicsActor. if this is NULL, only the first physics actor will be added and returned. 
		* @params pUserData: the user data associated with each physics actor, this is usually its parent BaseObject. 
		* @return the first physics actor that is added in ppMesh. 
		*/
		IParaPhysicsActor* CreateStaticMesh(MeshEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup = 0, vector<IParaPhysicsActor*>* pOutputPhysicsActor = NULL, void* pUserData=NULL);
		IParaPhysicsActor* CreateStaticMesh(ParaXEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup = 0, vector<IParaPhysicsActor*>* pOutputPhysicsActor = NULL, void* pUserData = NULL);

		IParaPhysicsActor* CreateDynamicMesh(CBaseObject* obj);
		IParaPhysicsActor* CreateDynamicShape(CBaseObject* obj);
		
		std::shared_ptr<CPhysicsBlock> LoadPhysicsBlock(uint16_t bx, uint16_t by, uint16_t bz);
		void LoadPhysicsBlock(CShapeAABB* aabb, int16_t frameId = 0, float extend = 0.5f);

		/** release an actor by calling this function. */
		void ReleaseActor(IParaPhysicsActor* pActor);

		//////////////////////////////////////////////////////////////////////////
		// Constraint/Joint APIs
		//////////////////////////////////////////////////////////////////////////

		/** Create a constraint/joint between two physics actors
		* @param constraintType: type of constraint (see ParaPhysicsConstraintType)
		* @param pActorA: first rigid body (required)
		* @param pActorB: second rigid body (optional, nullptr for world constraint)
		* @param pivotInA: pivot point in local space of body A
		* @param axisInA: axis in local space of body A (for hinge/slider)
		* @param pivotInB: pivot point in local space of body B
		* @param axisInB: axis in local space of body B (for hinge/slider)
		* @param disableCollision: whether to disable collision between connected bodies
		* @return: pointer to the created constraint, or nullptr on failure
		*/
		IParaPhysicsConstraint* CreateConstraint(int constraintType, IParaPhysicsActor* pActorA, IParaPhysicsActor* pActorB,
			const Vector3& pivotInA, const Vector3& axisInA, const Vector3& pivotInB, const Vector3& axisInB,
			bool disableCollision = true);

		/** Release a constraint */
		void ReleaseConstraint(IParaPhysicsConstraint* pConstraint);

		/** Set constraint properties from a Lua table string */
		void SetConstraintProperty(IParaPhysicsConstraint* pConstraint, const char* property);

		/** Get constraint properties as a Lua table string */
		const char* GetConstraintProperty(IParaPhysicsConstraint* pConstraint);

		//////////////////////////////////////////////////////////////////////////
		// Vehicle/Wheel APIs
		//////////////////////////////////////////////////////////////////////////

		/** Create a ray cast vehicle attached to a dynamic physics actor
		* @param pChassisActor: the rigid body to use as chassis (must be dynamic)
		* @return: pointer to the created vehicle, or nullptr on failure
		*/
		IParaPhysicsVehicle* CreateVehicle(IParaPhysicsActor* pChassisActor);

		/** Release a vehicle */
		void ReleaseVehicle(IParaPhysicsVehicle* pVehicle);

		/** Add a wheel to a vehicle from a Lua table string configuration */
		int AddWheelToVehicle(IParaPhysicsVehicle* pVehicle, const char* wheelConfig);

		//////////////////////////////////////////////////////////////////////////
		// Actor-based Lookup APIs (for game objects to query their physics)
		//////////////////////////////////////////////////////////////////////////

		/** Get all constraints associated with a given actor
		* @param pActor: the physics actor to query
		* @param outConstraints: output vector to receive constraint pointers
		* @return: number of constraints found
		*/
		int GetConstraintsByActor(IParaPhysicsActor* pActor, std::vector<IParaPhysicsConstraint*>& outConstraints);

		/** Get constraint by actor and index
		* @param pActor: the physics actor
		* @param index: index among constraints associated with this actor
		* @return: constraint pointer or nullptr
		*/
		IParaPhysicsConstraint* GetConstraintByActor(IParaPhysicsActor* pActor, int index);

		/** Get number of constraints associated with an actor */
		int GetConstraintCountByActor(IParaPhysicsActor* pActor);

		/** Release all constraints associated with an actor */
		void ReleaseConstraintsByActor(IParaPhysicsActor* pActor);

		/** Get vehicle associated with an actor (actor is the chassis)
		* @param pChassisActor: the chassis rigid body
		* @return: vehicle pointer or nullptr
		*/
		IParaPhysicsVehicle* GetVehicleByActor(IParaPhysicsActor* pChassisActor);

		/** Release vehicle by chassis actor */
		void ReleaseVehicleByActor(IParaPhysicsActor* pChassisActor);

		/**	whether to do dynamic simulation. It is turned off by default, which only provide basic collision detection. 
		*/
		void SetDynamicsSimulationEnabled(bool bEnable);
		bool IsDynamicsSimulationEnabled();

	public:
		/** get the physics interface. create one if one does not exist. */
		IParaPhysics* GetPhysicsInterface();

	public:
		/** the main physic interface. */
		IParaPhysics*     m_pPhysicsWorld;

		/// all shapes used to composed the physical world. There may be multiple object using the same shape
		TriangleMeshShape_Map_Type  m_listMeshShapes;
		IParaPhysicsActor_Map_Type m_mapDynamicActors;
		std::unordered_map<uint64_t, std::shared_ptr<CPhysicsBlock>> m_mapPhysicsBlocks;

		/// all constraints/joints created in the physics world
		IParaPhysicsConstraint_Set_Type m_constraints;

		/// all vehicles created in the physics world
		IParaPhysicsVehicle_Set_Type m_vehicles;

		/// whether to do dynamic simulation. It is turned off by default, which only provide basic collision detection. 
		bool m_bRunDynamicSimulation;
	};
}
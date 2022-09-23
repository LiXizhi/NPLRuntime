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
	class BlockModel;

	class CPhysicsBlock
	{
	public:
		class CPhysicsBlockShape
		{
		public:
			CPhysicsBlockShape(): m_shape(nullptr) {}
			~CPhysicsBlockShape() {  }

			IParaPhysicsShape* m_shape; 
			std::string m_hash;
		};

		CPhysicsBlock(uint64_t id)
		{
			m_id = id;
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

		IParaPhysicsShape* GetShape(uint32_t key, BlockModel& model, IParaPhysics* world);

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

		void Load(IParaPhysics* world);
		void Unload();
	private:
		uint64_t m_id;   
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

		std::shared_ptr<CPhysicsBlock> LoadPhysicsBlock(uint16_t bx, uint16_t by, uint16_t bz);
		void LoadPhysicsBlock(CShapeAABB* aabb, int16_t frameId = 0, float extend = 0.5f);

		/** release an actor by calling this function. */
		void ReleaseActor(IParaPhysicsActor* pActor);

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
		
		/// whether to do dynamic simulation. It is turned off by default, which only provide basic collision detection. 
		bool m_bRunDynamicSimulation;
	};
}
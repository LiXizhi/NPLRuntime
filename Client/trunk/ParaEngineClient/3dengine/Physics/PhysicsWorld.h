#pragma once
#include "Physics/IParaPhysics.h"

#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsDynamicsWorld.h"
#include "Physics/PhysicsConstraint.h"

#include <unordered_map>  

namespace ParaEngine {

	/**
	* The global physics scene (NxScene) and physics SDK is encapsulated in a member object of scene manager.
	* It is called CPhysicsWorld. The environment simulator can retrieve the physics scene from this object.
	* Other physics actor scene object can access this global physics objects via the scene manager.
	*/
	class CPhysicsWorld : public IAttributeFields
	{
	public:
		CPhysicsWorld(void);
		virtual ~CPhysicsWorld(void);

		ATTRIBUTE_DEFINE_CLASS(CPhysicsWorld);

		/** get the physics interface. create one if one does not exist. */
		IParaPhysics* GetPhysicsInterface();

		/**
		* Init the physics scene
		*/
		void InitPhysics();

		/* release physics scene*/
		void ExitPhysics();

		/** release physics world, then create it*/
		void ResetPhysics();

		/** Start the physics for some time advances
		* @params dTime: advances in seconds */
		void StepSimulation(double dTime);


		/**	whether to do dynamic simulation. It is turned off by default, which only provide basic collision detection.
		*/
		void SetDynamicsSimulationEnabled(bool bEnable);
		bool IsDynamicsSimulationEnabled() const;


		CPhysicsRigidBody* CreateBoundsBody(CBaseObject* pObject, const Matrix4& globalMat, uint32 nShapeGroup = 0, vector<CPhysicsRigidBody::WeakPtr>* pOutputPhysicsActor = nullptr, void* pUserData = nullptr);
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
		
		CPhysicsRigidBody* CreateStaticMesh(MeshEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup = 0, vector<CPhysicsRigidBody::WeakPtr>* pOutputPhysicsActor = nullptr, void* pUserData = nullptr);
		CPhysicsRigidBody* CreateStaticMesh(ParaXEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup = 0, vector<CPhysicsRigidBody::WeakPtr>* pOutputPhysicsActor = nullptr, void* pUserData = nullptr);

		/* create a box shape*/
		CPhysicsShape* CreateBoxShape(const PARAVECTOR3& boxHalfExtents);

		/* create a sphere shape*/
		CPhysicsShape* CreateSphereShape(float radius);

		/* create a capsule shape*/
		CPhysicsShape* CreateCapsuleShapeY(float radius, float height);
		CPhysicsShape* CreateCapsuleShapeX(float radius, float height);
		CPhysicsShape* CreateCapsuleShapeZ(float radius, float height);

		/* create a cylinder shape*/
		CPhysicsShape* CreateCylinderShapeY(const PARAVECTOR3& halfExtents);
		CPhysicsShape* CreateCylinderShapeX(const PARAVECTOR3& halfExtents);
		CPhysicsShape* CreateCylinderShapeZ(const PARAVECTOR3& halfExtents);


		/* create a cone shape*/
		CPhysicsShape* CreateConeShapeY(float radius, float height);
		CPhysicsShape* CreateConeShapeX(float radius, float height);
		CPhysicsShape* CreateConeShapeZ(float radius, float height);
	
		/* create a Convex Hull shape*/
		CPhysicsShape* CreateConvexHullShape(const PARAVECTOR3* points, int numPoints);

		/* create a MultiSphere shape*/
		CPhysicsShape* CreateMultiSphereShape(const PARAVECTOR3* positions, const float* radi, int numSpheres);

		/* create a Compound shape*/
		CPhysicsCompoundShape* CreateCompoundShape(bool enableDynamicAabbTree);

		/* create a static plane shape*/
		CPhysicsShape* CreateStaticPlaneShape(const PARAVECTOR3& planeNormal, float planeConstant);

		/** create a triangle shape.
		* @return: the triangle shape pointer is returned.
		*/
		CPhysicsTriangleMeshShape* CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc);

		// The ScaledTriangleMeshShape allows to instance a scaled version of an existing TriangleMeshShape
		CPhysicsScaledTriangleMeshShape* CreateScaledTriangleMeshShape(CPhysicsTriangleMeshShape* pTriangleMeshShape, const PARAVECTOR3& localScaling);

		/*  create a rigid body */
		CPhysicsRigidBody* CreateRigidbody(const ParaPhysicsRigidbodyDesc& desc, ParaPhysicsMotionStateDesc* motionStateDesc = nullptr);


		/* create a Point to Point Constraint */
		CPhysicsP2PConstraint* CreatePoint2PointConstraint(CPhysicsRigidBody* rbA, const PARAVECTOR3& pivotInA);
		CPhysicsP2PConstraint* CreatePoint2PointConstraint(CPhysicsRigidBody* rbA
			, CPhysicsRigidBody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB);

		/* create a Hinge Constraint */
		CPhysicsHingeConstraint* CreateHingeConstraint(CPhysicsRigidBody* rbA
			, CPhysicsRigidBody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB
			, const PARAVECTOR3& axisInA
			, const PARAVECTOR3& axisInB
			, bool useReferenceFrameA = false);
		CPhysicsHingeConstraint* CreateHingeConstraint(CPhysicsRigidBody* rbA
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& axisInA
			, bool useReferenceFrameA = false);
		 CPhysicsHingeConstraint* CreateHingeConstraint(CPhysicsRigidBody* rbA
			, CPhysicsRigidBody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useReferenceFrameA = false);
		 CPhysicsHingeConstraint* CreateHingeConstraint(CPhysicsRigidBody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, bool useReferenceFrameA = false);

		 /* create a Slider Constraint */
		 CPhysicsSliderConstraint* CreateSliderConstraint(CPhysicsRigidBody* rbA
			 , CPhysicsRigidBody* rbB
			 , const PARAVECTOR3& rbAOrigin
			 , const PARAMATRIX3x3& rbARotation
			 , const PARAVECTOR3& rbBOrigin
			 , const PARAMATRIX3x3& rbBRotation
			 , bool useLinearReferenceFrameA);
		 CPhysicsSliderConstraint* CreateSliderConstraint(CPhysicsRigidBody* rbB
			 , const PARAVECTOR3& rbBOrigin
			 , const PARAMATRIX3x3& rbBRotation
			 , bool useLinearReferenceFrameA);

		 /* can be used to simulate ragdoll joints */
		 CPhysicsConeTwistConstraint* CreateConeTwistConstraint(CPhysicsRigidBody* rbA
			 , const PARAVECTOR3& rbAOrigin
			 , const PARAMATRIX3x3& rbARotation);
		 CPhysicsConeTwistConstraint* CreateConeTwistConstraint(CPhysicsRigidBody* rbA
			 , CPhysicsRigidBody* rbB
			 , const PARAVECTOR3& rbAOrigin
			 , const PARAMATRIX3x3& rbARotation
			 , const PARAVECTOR3& rbBOrigin
			 , const PARAMATRIX3x3& rbBRotation);

		 /* Generic 6 DOF constraint that allows to set spring motors to any translational and rotational DOF. */
		 CPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(CPhysicsRigidBody* rbA
			 , CPhysicsRigidBody* rbB
			 , const PARAVECTOR3& rbAOrigin
			 , const PARAMATRIX3x3& rbARotation
			 , const PARAVECTOR3& rbBOrigin
			 , const PARAMATRIX3x3& rbBRotation
			 , bool useLinearReferenceFrameA);
		 CPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(CPhysicsRigidBody* rbB
			 , const PARAVECTOR3& rbBOrigin
			 , const PARAMATRIX3x3& rbBRotation
			 , bool useLinearReferenceFrameB);
	
		/* get current world, if not call InitPhysics it will return nullptr*/
		CPhysicsDynamicsWorld* GetCurrentWorld();

	private:
		// 
		void onAfterCheckContact();
	private:

		struct SubMeshPhysicsShape
		{
		public:
			SubMeshPhysicsShape() :m_nPhysicsGroup(0){};
			SubMeshPhysicsShape(CPhysicsShape* pShape, int nPhysicsGroup = 0) :m_nPhysicsGroup(nPhysicsGroup), m_pShape(pShape) {};
			CPhysicsShape::WeakPtr m_pShape;
			int m_nPhysicsGroup;
		};

		/**
		* the triangle mesh shape.
		* since we need to create separate physics mesh with different scaling factors even for the same mesh model,
		* we will need the mesh entity plus the scaling vector to fully specify the mesh object
		*/
		struct TriangleMeshShape
		{
		public:
			std::vector<SubMeshPhysicsShape> m_pShapes;
		};

		std::unordered_map<void*, TriangleMeshShape> m_meshShapesCache;

		TriangleMeshShape* GetShapsInCache(void* p);
		TriangleMeshShape& BuildCache(void* p);

		/** the main physic interface. */
		IParaPhysics*     m_pPhysicsInstance;

		/* current physic world pointer */
		ref_ptr<CPhysicsDynamicsWorld> m_pWorld;

		/// whether to do dynamic simulation. It is turned off by default, which only provide basic collision detection. 
		bool m_bRunDynamicSimulation;
	};
}
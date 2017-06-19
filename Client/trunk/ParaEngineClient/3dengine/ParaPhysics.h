#pragma once
#include "Physics/IParaPhysics.h"

namespace ParaEngine
{
	class CParaPhysicsImp : public IParaPhysics
	{
	public:
		CParaPhysicsImp();

	public:
		/** create and initialize all physics lib and create the default scene(world) */
		virtual IParaPhysicsWorld* InitPhysics(ParaPhysicsWorldType ppwt = ParaPhysicsWorldType::PPWT_Dynamics);

		virtual void Release();

		virtual IParaPhysicsTriangleMeshShape* CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc);

		virtual IParaPhysicsShape* CreateBoxShape(const PARAVECTOR3& boxHalfExtents);

		virtual IParaPhysicsShape* CreateCylinderShape(const PARAVECTOR3& boxHalfExtents);

		/* create a sphere shape*/
		virtual IParaPhysicsShape* CreateSphereShape(float radius);

		virtual IParaPhysicsShape* CreateCapsuleShapeY(float radius, float height);
		virtual IParaPhysicsShape* CreateCapsuleShapeX(float radius, float height);
		virtual IParaPhysicsShape* CreateCapsuleShapeZ(float radius, float height);

		/* create a cylinder shape*/
		virtual IParaPhysicsShape* CreateCylinderShapeY(const PARAVECTOR3& halfExtents) { return nullptr; };
		virtual IParaPhysicsShape* CreateCylinderShapeX(const PARAVECTOR3& halfExtents) { return nullptr; };
		virtual IParaPhysicsShape* CreateCylinderShapeZ(const PARAVECTOR3& halfExtents) { return nullptr; };

		/* create a Cone shape*/
		virtual IParaPhysicsShape* CreateConeShapeY(float radius, float height) { return nullptr; };
		virtual IParaPhysicsShape* CreateConeShapeX(float radius, float height) { return nullptr; };
		virtual IParaPhysicsShape* CreateConeShapeZ(float radius, float height) { return nullptr; };



		/* create a Convex Hull shape*/
		virtual IParaPhysicsShape* CreateConvexHullShape(const PARAVECTOR3* points, int numPoints) { return nullptr; };

		/* create a MultiSphere shape*/
		virtual IParaPhysicsShape* CreateMultiSphereShape(const PARAVECTOR3* positions, const float* radi, int numSpheres) { return nullptr; };

		/* create a Compound shape*/
		virtual IParaPhysicsCompoundShape* CreateCompoundShape(bool enableDynamicAabbTree) { return nullptr;  };


		/* create a static plane shape*/
		virtual IParaPhysicsShape* CreateStaticPlaneShape(const PARAVECTOR3& planeNormal, float planeConstant) { return nullptr;  };

		// The ScaledTriangleMeshShape allows to instance a scaled version of an existing TriangleMeshShape
		virtual IParaPhysicsScalingTriangleMeshShape* CreateScaledTriangleMeshShape(IParaPhysicsTriangleMeshShape* pTriangleMeshShape, const PARAVECTOR3& localScaling) { return nullptr; };

		/*  create a rigid body */
		virtual IParaPhysicsRigidbody* CreateRigidbody(const ParaPhysicsRigidbodyDesc& desc, ParaPhysicsMotionStateDesc* motionStateDesc = nullptr) { return nullptr; };

		/* get current world, if not call InitPhysics it will return nullptr*/
		//virtual IParaPhysicsWorld* GetCurrentWorld() { return nullptr; };

		/* create a Point to Point Constraint */
		virtual IParaPhysicsPoint2PointConstraint* CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA, const PARAVECTOR3& pivotInA) { return nullptr;  };
		virtual IParaPhysicsPoint2PointConstraint* CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB) 
		{
			return nullptr; 
		};


		/* create a Hinge Constraint */
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB
			, const PARAVECTOR3& axisInA
			, const PARAVECTOR3& axisInB
			, bool useReferenceFrameA = false) 
		{
			return nullptr;
		};
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& axisInA
			, bool useReferenceFrameA = false) 
		{
			return nullptr;
		};
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useReferenceFrameA = false)
		{
			return nullptr;
		};
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, bool useReferenceFrameA = false) 
		{
			return nullptr;
		};

		/* create a Slider Constraint */
		virtual IParaPhysicsSliderConstraint* CreateSliderConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA)
		{
			return nullptr;
		};
		virtual IParaPhysicsSliderConstraint* CreateSliderConstraint(IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA)
		{
			return nullptr;
		};

		/* can be used to simulate ragdoll joints */
		virtual IParaPhysicsConeTwistConstraint* CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation)
		{
			return nullptr;
		};
		virtual IParaPhysicsConeTwistConstraint* CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation)
		{
			return nullptr;
		};

		virtual IParaPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA)
		{
			return nullptr;
		};
		virtual IParaPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameB)
		{
			return nullptr;
		};

		virtual void SetDebugDrawer(IParaDebugDraw* debugDrawer);

		virtual IParaDebugDraw* GetDebugDrawer();

		virtual void DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color);

		virtual void SetDebugDrawMode(int debugMode);

		virtual int GetDebugDrawMode();
	protected:
		IParaDebugDraw* m_pDebugDrawer;
		int m_nDebugMode;
	};
}
	


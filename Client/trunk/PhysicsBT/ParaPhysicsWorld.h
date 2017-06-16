#pragma once

#include "ParaPhysicsType.h"
#include "PhysicsDebugDraw.h"

namespace ParaEngine {
	class CParaPhysicsWorld :public IParaPhysics
	{
	public:
		CParaPhysicsWorld();
		virtual ~CParaPhysicsWorld();

		/** create and initialize all physics lib and create the default scene(world) */
		virtual IParaPhysicsWorld* InitPhysics(ParaPhysicsWorldType ppwt = ParaPhysicsWorldType::PPWT_Dynamics);

		/** all this to unload the IPhysics. Pointer to this class will be invalid after the call
		*/
		virtual void Release();

		/** set the debug draw object for debugging physics world. */
		virtual void	SetDebugDrawer(IParaDebugDraw*	debugDrawer);

		/** Get the debug draw object for debugging physics world. */
		virtual IParaDebugDraw*	GetDebugDrawer();

		/** draw a given object. */
		virtual void DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color);

		/** bitwise of PhysicsDebugDrawModes */
		virtual void	SetDebugDrawMode(int debugMode);

		/** bitwise of PhysicsDebugDrawModes */
		virtual int		GetDebugDrawMode();

		/* create a box shape*/
		virtual IParaPhysicsShape* CreateBoxShape(const PARAVECTOR3& boxHalfExtents);

		/* create a sphere shape*/
		virtual IParaPhysicsShape* CreateSphereShape(float radius);

		/* create a Capsule shape*/
		virtual IParaPhysicsShape* CreateCapsuleShapeY(float radius, float height);
		virtual IParaPhysicsShape* CreateCapsuleShapeX(float radius, float height);
		virtual IParaPhysicsShape* CreateCapsuleShapeZ(float radius, float height);

		/* create a cylinder shape*/
		virtual IParaPhysicsShape* CreateCylinderShapeY(const PARAVECTOR3& halfExtents);
		virtual IParaPhysicsShape* CreateCylinderShapeX(const PARAVECTOR3& halfExtents);
		virtual IParaPhysicsShape* CreateCylinderShapeZ(const PARAVECTOR3& halfExtents);

		/* create a Cone shape*/
		virtual IParaPhysicsShape* CreateConeShapeY(float radius, float height);
		virtual IParaPhysicsShape* CreateConeShapeX(float radius, float height);
		virtual IParaPhysicsShape* CreateConeShapeZ(float radius, float height);

		/* create a MultiSphere shape*/
		IParaPhysicsShape* CreateMultiSphereShape(const PARAVECTOR3* positions, const float* radi, int numSpheres);

		/* create a Compound shape*/
		virtual IParaPhysicsCompoundShape* CreateCompoundShape(bool enableDynamicAabbTree);

		/* create a Convex Hull shape*/
		virtual IParaPhysicsShape* CreateConvexHullShape(const PARAVECTOR3* points, int numPoints);

		/* create a static plane shape*/
		virtual IParaPhysicsShape* CreateStaticPlaneShape(const PARAVECTOR3& planeNormal, float planeConstant);

		/** create a triangle shape.
		* @return: the triangle shape pointer is returned.
		*/
		virtual IParaPhysicsTriangleMeshShape* CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc);

		// The ScaledTriangleMeshShape allows to instance a scaled version of an existing TriangleMeshShape
		virtual IParaPhysicsScalingTriangleMeshShape* CreateScaledTriangleMeshShape(IParaPhysicsTriangleMeshShape* pTriangleMeshShape, const PARAVECTOR3& localScaling);

		/*  create a rigid body */
		virtual IParaPhysicsRigidbody* CreateRigidbody(const ParaPhysicsRigidbodyDesc& desc, ParaPhysicsMotionStateDesc* motionStateDesc = nullptr);

		
		/* create a Point to Point Constraint */
		virtual IParaPhysicsPoint2PointConstraint* CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA, const PARAVECTOR3& pivotInA);
		virtual IParaPhysicsPoint2PointConstraint* CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB);

		/* create a Hinge Constraint */
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB
			, const PARAVECTOR3& axisInA
			, const PARAVECTOR3& axisInB
			, bool useReferenceFrameA = false);
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& axisInA
			, bool useReferenceFrameA = false);
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useReferenceFrameA = false);
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, bool useReferenceFrameA = false);

		/* create a Slider Constraint */
		virtual IParaPhysicsSliderConstraint* CreateSliderConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA);
		virtual IParaPhysicsSliderConstraint* CreateSliderConstraint(IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA);

		/* can be used to simulate ragdoll joints */
		virtual IParaPhysicsConeTwistConstraint* CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation);
		virtual IParaPhysicsConeTwistConstraint* CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation);

		/* Generic 6 DOF constraint that allows to set spring motors to any translational and rotational DOF. */
		virtual IParaPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA);
		virtual IParaPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameB);
		
		/* get current world, if not call InitPhysics it will return nullptr*/
		//virtual IParaPhysicsWorld* GetCurrentWorld();
	private:

		CPhysicsDebugDraw m_physics_debug_draw;
		bool m_bInvertFaceWinding;
	};
}
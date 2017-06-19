#pragma once

#include "Physics/IParaPhysicsType.h"
#include "Physics/IParaPhysicsStruct.h"

namespace ParaEngine {

	class IParaDebugDraw;

	/** ParaPhysics core interface.
	*/
	class IParaPhysics
	{
	public:
		/** create and initialize all physics lib and create the default scene(world) */
		virtual IParaPhysicsWorld* InitPhysics(ParaPhysicsWorldType ppwt = ParaPhysicsWorldType::PPWT_Dynamics) = 0;

				/** all this to unload the IPhysics. Pointer to this class will be invalid after the call
		*/
		virtual void Release() = 0;

		/* create a box shape*/
		virtual IParaPhysicsShape* CreateBoxShape(const PARAVECTOR3& boxHalfExtents) = 0;

		/* create a sphere shape*/
		virtual IParaPhysicsShape* CreateSphereShape(float radius) = 0;

		/* create a capsule shape*/
		virtual IParaPhysicsShape* CreateCapsuleShapeY(float radius, float height) = 0;
		virtual IParaPhysicsShape* CreateCapsuleShapeX(float radius, float height) = 0;
		virtual IParaPhysicsShape* CreateCapsuleShapeZ(float radius, float height) = 0;

		/* create a cylinder shape*/
		virtual IParaPhysicsShape* CreateCylinderShapeY(const PARAVECTOR3& halfExtents) = 0;
		virtual IParaPhysicsShape* CreateCylinderShapeX(const PARAVECTOR3& halfExtents) = 0;
		virtual IParaPhysicsShape* CreateCylinderShapeZ(const PARAVECTOR3& halfExtents) = 0;

		/* create a cone shape*/
		virtual IParaPhysicsShape* CreateConeShapeY(float radius, float height) = 0;
		virtual IParaPhysicsShape* CreateConeShapeX(float radius, float height) = 0;
		virtual IParaPhysicsShape* CreateConeShapeZ(float radius, float height) = 0;

		/* create a Convex Hull shape*/
		virtual IParaPhysicsShape* CreateConvexHullShape(const PARAVECTOR3* points, int numPoints) = 0;

		/* create a MultiSphere shape*/
		virtual IParaPhysicsShape* CreateMultiSphereShape(const PARAVECTOR3* positions, const float* radi, int numSpheres) = 0;

		/* create a Compound shape*/
		virtual IParaPhysicsCompoundShape* CreateCompoundShape(bool enableDynamicAabbTree) = 0;

		/* create a static plane shape*/
		virtual IParaPhysicsShape* CreateStaticPlaneShape(const PARAVECTOR3& planeNormal, float planeConstant) = 0;

		/** create a triangle shape.
		* @return: the triangle shape pointer is returned.
		*/
		virtual IParaPhysicsTriangleMeshShape* CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc) = 0;

		// The ScaledTriangleMeshShape allows to instance a scaled version of an existing TriangleMeshShape
		virtual IParaPhysicsScalingTriangleMeshShape* CreateScaledTriangleMeshShape(IParaPhysicsTriangleMeshShape* pTriangleMeshShape, const PARAVECTOR3& localScaling) = 0;

		/*  create a rigid body */
		virtual IParaPhysicsRigidbody* CreateRigidbody(const ParaPhysicsRigidbodyDesc& desc, ParaPhysicsMotionStateDesc* motionStateDesc = nullptr) = 0;

		
		/* create a Point to Point Constraint */
		virtual IParaPhysicsPoint2PointConstraint* CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA, const PARAVECTOR3& pivotInA) = 0;
		virtual IParaPhysicsPoint2PointConstraint* CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB) = 0;

		/* create a Hinge Constraint */
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& pivotInB
			, const PARAVECTOR3& axisInA
			, const PARAVECTOR3& axisInB
			, bool useReferenceFrameA = false) = 0;
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& pivotInA
			, const PARAVECTOR3& axisInA
			, bool useReferenceFrameA = false) = 0;
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useReferenceFrameA = false) = 0;
		virtual IParaPhysicsHingeConstraint* CreateHingeConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, bool useReferenceFrameA = false) = 0;
		
		/* create a Slider Constraint */
		virtual IParaPhysicsSliderConstraint* CreateSliderConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA) = 0;
		virtual IParaPhysicsSliderConstraint* CreateSliderConstraint(IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA) = 0;


		/* can be used to simulate ragdoll joints */
		virtual IParaPhysicsConeTwistConstraint* CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation) = 0;
		virtual IParaPhysicsConeTwistConstraint* CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation) = 0;

		/* Generic 6 DOF constraint that allows to set spring motors to any translational and rotational DOF. */
		virtual IParaPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbA
			, IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbAOrigin
			, const PARAMATRIX3x3& rbARotation
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameA) = 0;
		virtual IParaPhysicsGeneric6DofSpringConstraint* CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbB
			, const PARAVECTOR3& rbBOrigin
			, const PARAMATRIX3x3& rbBRotation
			, bool useLinearReferenceFrameB) = 0;


		
		/* get current world, if not call InitPhysics it will return nullptr*/
		//virtual IParaPhysicsWorld* GetCurrentWorld() = 0;

		/** set the debug draw object for debugging physics world. */
		virtual void SetDebugDrawer(IParaDebugDraw*	debugDrawer) = 0;

		/** Get the debug draw object for debugging physics world. */
		virtual IParaDebugDraw*	GetDebugDrawer() = 0;

		/** draw a given object. */
		virtual void DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color) = 0;

		/** bitwise of PhysicsDebugDrawModes */
		virtual void SetDebugDrawMode(int debugMode) = 0;

		/** bitwise of PhysicsDebugDrawModes */
		virtual int	GetDebugDrawMode() = 0;
	};
}
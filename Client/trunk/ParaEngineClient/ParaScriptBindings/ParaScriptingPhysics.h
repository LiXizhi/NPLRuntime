#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"

extern "C"
{
#include <lua.h>
}

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsDynamicsWorld.h"
#include "Physics/PhysicsConstraint.h"

#include <ParaScriptingConvert.h>

namespace ParaEngine
{
	struct ParaPhysicsMotionStateDesc;
}

namespace ParaScripting
{
	class PE_CORE_DECL ParaPhysicsFactory
	{
	public:
		typedef ParaEngine::weak_ptr<ParaEngine::IObject, ParaEngine::CPhysicsFactory> WeakPtr_type;
		// a pointer to the object
		WeakPtr_type m_pObj;

		ParaPhysicsFactory() {};
		ParaPhysicsFactory(ParaEngine::CPhysicsFactory* pObj);
		~ParaPhysicsFactory();

		ParaEngine::CPhysicsFactory* get() const {
			return m_pObj.get();
		};

		/**
		* check if the object is valid
		*/
		bool IsValid() const { return m_pObj; };

		/** get the attribute object associated with an object. */
		ParaAttributeObject* GetAttributeObject();
		/** for API exportation*/
		void GetAttributeObject_(ParaAttributeObject& output);

		ParaAttributeObject* GetCurrentWorld();

		enum ObjectType
		{
			OT_MIN		= 0,
			OT_Shape	= OT_MIN,
			OT_Rigidbody,
			OT_Constraint,
			OT_Invalid,
			OT_MAX		= OT_Invalid,
		};

		enum ConstraintType
		{
			CT_MIN			= 0,
			CT_Point2Point	= CT_MIN,
			CT_Hinge,
			CT_Slider,
			CT_ConeTwist,
			CT_Generic6DofSpring,
			CT_Invalid,
			CT_MAX = CT_Invalid,
		};

		enum ShapeType
		{
			ST_MIN			= 0,
			ST_Box			= ST_MIN,
			ST_Sphere,
			ST_CapsuleY,
			ST_CapsuleX,
			ST_CapsuleZ,
			ST_CylinderY,
			ST_CylinderX,
			ST_CylinderZ,
			ST_ConeY,
			ST_ConeX,
			ST_ConeZ,
			ST_ConvexHull,
			ST_Compound,
			ST_StaticPlane,
			ST_TriangleMesh,
			ST_ScaledTriangleMesh,
			ST_Invalid,
			ST_MAX = ST_Invalid,
		};

		ParaAttributeObject* CreateShape(ShapeType shapeType, const object& params);

		ParaAttributeObject* CreateConstraint(ConstraintType cType, const object& params);

		ParaAttributeObject* CreateRigidbody(const object& params);
		ParaAttributeObject* CreateRigidbody2(const object& params, const object& callback);

		ParaAttributeObject* CreatePhysicsObject(ObjectType oType, int subType, const object& params);
	private:
		ParaAttributeObject* CreateRigidbody_(const object& params, ParaPhysicsMotionStateDesc* motionStateDesc);


		/* create a box shape*/
		ParaAttributeObject* CreateBoxShape(const object& boxHalfExtents);

		/* create a sphere shape*/
		ParaAttributeObject* CreateSphereShape(const object& radius);

		/* create a capsule shape*/
		ParaAttributeObject* CreateCapsuleShapeY(const object& params);
		ParaAttributeObject* CreateCapsuleShapeX(const object& params);
		ParaAttributeObject* CreateCapsuleShapeZ(const object& params);

		/* create a cylinder shape*/
		ParaAttributeObject* CreateCylinderShapeY(const object& halfExtents);
		ParaAttributeObject* CreateCylinderShapeX(const object& halfExtents);
		ParaAttributeObject* CreateCylinderShapeZ(const object& halfExtents);

		/* create a cone shape*/
		ParaAttributeObject* CreateConeShapeY(const object& params);
		ParaAttributeObject* CreateConeShapeX(const object& params);
		ParaAttributeObject* CreateConeShapeZ(const object& params);

		/* create a Convex Hull shape*/
		ParaAttributeObject* CreateConvexHullShape(const object& params);

		/* create a Compound shape*/
		ParaAttributeObject* CreateCompoundShape(const object& enableDynamicAabbTree);

		/* create a static plane shape*/
		ParaAttributeObject* CreateStaticPlaneShape(const object& params);

		/** create a triangle shape.
		* @return: the triangle shape pointer is returned.
		*/
		ParaAttributeObject* CreateTriangleMeshShape(const object& params);

		// The ScaledTriangleMeshShape allows to instance a scaled version of an existing TriangleMeshShape
		ParaAttributeObject* CreateScaledTriangleMeshShape(const object& params);

		/* create a Point to Point Constraint */
		ParaAttributeObject* CreatePoint2PointConstraint(const object& params);

		/* create a Hinge Constraint */
		ParaAttributeObject* CreateHingeConstraint(const object& params);

		/* create a Slider Constraint */
		ParaAttributeObject* CreateSliderConstraint(const object& params);

		/* can be used to simulate ragdoll joints */
		ParaAttributeObject* CreateConeTwistConstraint(const object& params);

		/* Generic 6 DOF constraint that allows to set spring motors to any translational and rotational DOF. */
		ParaAttributeObject* CreateGeneric6DofSpringConstraint(const object& params);

		typedef ParaAttributeObject* (ParaPhysicsFactory::*CreateShapeFunc)(const object& params);
		static CreateShapeFunc m_pCrateShapeFunc[ShapeType::ST_MAX];

		typedef ParaAttributeObject* (ParaPhysicsFactory::*CreateConstraintFunc)(const object& params);
		static CreateConstraintFunc m_pCreateConstraintFunc[ConstraintType::CT_MAX];
	};
}
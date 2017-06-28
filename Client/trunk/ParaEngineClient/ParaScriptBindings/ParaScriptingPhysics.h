#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"

namespace ParaEngine
{
	class CPhysicsDynamicsWorld;
	class CPhysicsFactory;
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
		ParaAttributeObject GetAttributeObject();
		/** for API exportation*/
		void GetAttributeObject_(ParaAttributeObject& output);

		ParaAttributeObject GetCurrentWorld();


		enum ShapeType
		{
			MIN			= 0,
			Box			= 0,
			Sphere,
			CapsuleY,
			CapsuleX,
			CapsuleZ,
			CylinderY,
			CylinderX,
			CylinderZ,
			ConeY,
			ConeX,
			ConeZ,
			ConvexHull,
			Compound,
			StaticPlane,
			Invalid,
			MAX = Invalid,
		};

		ParaAttributeObject CreateShape(ShapeType shapeType, const object& params);


	private:
		/* create a box shape*/
		ParaAttributeObject CreateBoxShape(const object& boxHalfExtents);

		/* create a sphere shape*/
		ParaAttributeObject CreateSphereShape(const object& radius);

		/* create a capsule shape*/
		ParaAttributeObject CreateCapsuleShapeY(const object& params);
		ParaAttributeObject CreateCapsuleShapeX(const object& params);
		ParaAttributeObject CreateCapsuleShapeZ(const object& params);

		/* create a cylinder shape*/
		ParaAttributeObject CreateCylinderShapeY(const object& halfExtents);
		ParaAttributeObject CreateCylinderShapeX(const object& halfExtents);
		ParaAttributeObject CreateCylinderShapeZ(const object& halfExtents);

		/* create a cone shape*/
		ParaAttributeObject CreateConeShapeY(const object& params);
		ParaAttributeObject CreateConeShapeX(const object& params);
		ParaAttributeObject CreateConeShapeZ(const object& params);

		/* create a Convex Hull shape*/
		ParaAttributeObject CreateConvexHullShape(const object& params);

		/* create a Compound shape*/
		ParaAttributeObject CreateCompoundShape(const object& enableDynamicAabbTree);

		/* create a static plane shape*/
		ParaAttributeObject CreateStaticPlaneShape(const object& params);

		typedef ParaAttributeObject (ParaPhysicsFactory::*CreateShapeFunc)(const object& params);
		static CreateShapeFunc m_pCrateShapeFunc[ShapeType::MAX];
	};
}
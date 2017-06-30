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

namespace ParaEngine
{
	class CPhysicsDynamicsWorld;
	class CPhysicsFactory;
	class CPhysicsRigidBody;
	class CPhysicsConstraint;
	class CPhysicsShape;
	struct ParaPhysicsMotionStateDesc;
}

namespace ParaScripting
{
	inline bool Object2Vector3(const luabind::object& o, PARAVECTOR3& output)
	{
		if (luabind::type(o) == LUA_TTABLE
			&& luabind::type(o[1]) == LUA_TNUMBER
			&& luabind::type(o[2]) == LUA_TNUMBER
			&& luabind::type(o[3]) == LUA_TNUMBER)
		{
			output.x = luabind::object_cast<float>(o[1]);
			output.y = luabind::object_cast<float>(o[2]);
			output.z = luabind::object_cast<float>(o[3]);

			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool Object2Matrix3x3(const luabind::object& o, PARAMATRIX3x3& output)
	{
		if (luabind::type(o) != LUA_TTABLE)
			return false;

		for (int x = 0; x < 3; x++)
			for (int y = 0; y < 3; y++)
			{
				int index = x * 3 + y + 1;
				if (luabind::type(o[index]) != LUA_TNUMBER)
					return false;

				output.m[x][y] = luabind::object_cast<float>(o[index]);
			}

		return true;
	}

	inline void Vector32Object(const PARAVECTOR3& v, luabind::object& o)
	{
		(o)[1] = (v).x; (o)[2] = (v).y; (o)[3] = (v).z;
	}

	inline void Matrix3x32Object(const PARAMATRIX3x3& matrix, luabind::object& o)
	{
		for (int x = 0; x < 3; x++)
			for (int y = 0; y < 3; y++)
			{
				int index = x * 3 + y + 1; 
				(o)[index] = (matrix).m[x][y]; 
			}
	}

	inline void Object2RigidBody(const luabind::object& o, ParaAttributeObject*& rb)
	{
		(rb) = nullptr; 
		if (luabind::type((o)) == LUA_TUSERDATA)
		{
			auto f = luabind::object_cast<ParaAttributeObject*>((o));
			if (f->IsValid())
			{
				auto classId = f->GetClassID(); 
				if (classId == ATTRIBUTE_CLASSID_CPhysicsRigidBody)
				{
					(rb) = f; 
				}
			}
		}
	}

	typedef std::function<bool(ATTRIBUTE_CLASSID_TABLE classType)> CheckClassFunc;
	inline IAttributeFields* Object2AttributeFields(const object& o, CheckClassFunc f)
	{
		if (type(o) != LUA_TUSERDATA)
			return nullptr;

		auto pAttObj = object_cast<ParaAttributeObject*>(o);
		if (!pAttObj || !pAttObj->IsValid())
			return nullptr;

		auto classId = pAttObj->GetClassID();
		if (f((ATTRIBUTE_CLASSID_TABLE)classId))
			return pAttObj->m_pAttribute.get();
		else
			return nullptr;

	}

	inline IAttributeFields* Object2AttributeFields(const object& o, ATTRIBUTE_CLASSID_TABLE classType)
	{
		return Object2AttributeFields(o, [classType](ATTRIBUTE_CLASSID_TABLE thisType)->bool
		{
			return classType == thisType;
		});
	}

	template<class T>
	bool Object2PhysicsObject(const object& o, T*& out, CheckClassFunc f)
	{
		auto p = Object2AttributeFields(o, f);
		if (!p)
			return false;

		out = static_cast<T*>(p);
		return true;
	}

	inline bool ReadLuaPhysicsShape(const object& o, CPhysicsShape*& out)
	{
		return Object2PhysicsObject(o, out, [](ATTRIBUTE_CLASSID_TABLE classType)
		{
			return (classType >= ATTRIBUTE_CLASSID_CPhysicsShapeMin
				&& classType <= ATTRIBUTE_CLASSID_CPhysicsShapeMax);
		});
	}

	inline bool ReadLuaPhysicsRigidBody(const object& o, CPhysicsRigidBody*& out)
	{
		return Object2PhysicsObject(o, out, [](ATTRIBUTE_CLASSID_TABLE classType)
		{
			return classType == ATTRIBUTE_CLASSID_CPhysicsRigidBody;
		});
	}

	inline bool ReadLuaPhysicsConstraint(const object& o, CPhysicsConstraint*& out)
	{
		return Object2PhysicsObject(o, out, [](ATTRIBUTE_CLASSID_TABLE classType)
		{
			return (classType >= ATTRIBUTE_CLASSID_CPhysicsConstraintMin
				&& classType <= ATTRIBUTE_CLASSID_CPhysicsConstraintMax);
		});
	}


	template<class T, int luaType>
	bool ReadLuaValue(const luabind::object& o, T& out)
	{
		if (luabind::type(o) != luaType)
			return false;

		out = luabind::object_cast<T>(o);

		return true;
	}

#define ReadLuaBoolean  ReadLuaValue<bool, LUA_TBOOLEAN>
#define ReadLuaInt ReadLuaValue<int, LUA_TNUMBER>
#define ReadLuaUInt ReadLuaValue<unsigned int, LUA_TNUMBER>
#define ReadLuaFloat ReadLuaValue<float, LUA_TNUMBER>
#define ReadLuaDouble ReadLuaValue<double, LUA_TNUMBER>
#define ReadLuaString ReadLuaValue<const char*, LUA_TSTRING>

	template<class T, class FuncType>
	bool ReadLuaTable(const luabind::object& o
		, const char* key
		, T& out
		, FuncType ConvertFunc)
	{
		auto& sub = o[key];
		return ConvertFunc(sub, out);
	}
	

#define ReadLuaTableBoolean(p1, p2, p3) ReadLuaTable<bool>((p1), (p2), (p3), ParaScripting::ReadLuaBoolean)
#define ReadLuaTableInt(p1, p2, p3) ReadLuaTable<int>((p1), (p2), (p3), ParaScripting::ReadLuaInt)
#define ReadLuaTableUInt(p1, p2, p3) ReadLuaTable<unsigned int>((p1), (p2), (p3), ParaScripting::ReadLuaUInt)
#define ReadLuaTableFloat(p1, p2, p3) ReadLuaTable<float>((p1), (p2), (p3), ParaScripting::ReadLuaFloat)
#define ReadLuaTableDouble(p1, p2, p3) ReadLuaTable<double>((p1), (p2), (p3), ParaScripting::ReadLuaDouble)
#define ReadLuaTableString(p1, p2, p3) ReadLuaTable<const char*>((p1), (p2), (p3), ParaScripting::ReadLuaString)
#define ReadLuaTableVector3(p1, p2, p3) ReadLuaTable<PARAVECTOR3>((p1), (p2), (p3), ParaScripting::Object2Vector3)
#define ReadLuaTableMatrix3x3(p1, p2, p3) ReadLuaTable<PARAMATRIX3x3>((p1), (p2), (p3), ParaScripting::Object2Matrix3x3)
#define ReadLuaTablePhysicsShape(p1, p2, p3) ReadLuaTable<CPhysicsShape*>((p1), (p2), (p3), ParaScripting::ReadLuaPhysicsShape)
#define ReadLuaTablePhysicsRigidBody(p1, p2, p3) ReadLuaTable<CPhysicsRigidBody*>((p1), (p2), (p3), ParaScripting::ReadLuaPhysicsRigidBody)
#define ReadLuaTablePhysicsConstraint(p1, p2, p3) ReadLuaTable<CPhysicsConstraint*>((p1), (p2), (p3), ParaScripting::ReadLuaPhysicsConstraint)

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
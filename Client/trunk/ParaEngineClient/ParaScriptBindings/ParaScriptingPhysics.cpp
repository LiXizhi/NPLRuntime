#include "ParaEngine.h"
#include "ParaScriptingPhysics.h"
#include "Physics/PhysicsFactory.h"
#include "Physics/PhysicsObject.h"
#include "Physics/PhysicsDynamicsWorld.h"

extern "C"
{
	#include <lua.h>
}

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

namespace ParaScripting
{
	ParaPhysicsFactory::ParaPhysicsFactory(ParaEngine::CPhysicsFactory* pObj)
		: m_pObj(pObj)
	{
	}

	ParaPhysicsFactory::~ParaPhysicsFactory()
	{

	}

	/** get the attribute object associated with an object. */
	ParaAttributeObject ParaPhysicsFactory::GetAttributeObject()
	{
		return ParaAttributeObject(m_pObj);
	}

	/** for API exportation*/
	void ParaPhysicsFactory::GetAttributeObject_(ParaAttributeObject& output)
	{
		output = GetAttributeObject();
	}

	ParaAttributeObject ParaPhysicsFactory::GetCurrentWorld()
	{
		if (IsValid())
		{
			return ParaAttributeObject(m_pObj->GetCurrentWorld());
		}
		else
		{
			return ParaAttributeObject();
		}
	}

	ParaPhysicsFactory::CreateShapeFunc ParaPhysicsFactory::m_pCrateShapeFunc[ParaPhysicsFactory::ShapeType::MAX] =
	{
		&ParaPhysicsFactory::CreateBoxShape
		, &ParaPhysicsFactory::CreateSphereShape

		, &ParaPhysicsFactory::CreateCapsuleShapeY
		, &ParaPhysicsFactory::CreateCapsuleShapeX
		, &ParaPhysicsFactory::CreateCapsuleShapeZ

		, &ParaPhysicsFactory::CreateCylinderShapeY
		, &ParaPhysicsFactory::CreateCylinderShapeX
		, &ParaPhysicsFactory::CreateCylinderShapeZ

		, &ParaPhysicsFactory::CreateConeShapeY
		, &ParaPhysicsFactory::CreateConeShapeX
		, &ParaPhysicsFactory::CreateConeShapeZ

		, &ParaPhysicsFactory::CreateConvexHullShape

		, &ParaPhysicsFactory::CreateCompoundShape

		, &ParaPhysicsFactory::CreateStaticPlaneShape
	};

	ParaAttributeObject ParaPhysicsFactory::CreateStaticPlaneShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		return ParaAttributeObject();
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCompoundShape(const object& enableDynamicAabbTree)
	{
		if (type(enableDynamicAabbTree) != LUA_TBOOLEAN)
			return ParaAttributeObject();

		auto obj = m_pObj->CreateCompoundShape(object_cast<bool>(enableDynamicAabbTree));
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateConvexHullShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		std::vector<PARAVECTOR3> positions;
		for (luabind::iterator itCur(params), itEnd; itCur != itEnd; ++itCur)
		{
			const object& item = *itCur;
			if (type(item) == LUA_TTABLE)
			{
				float x = object_cast<float>(item[1]);
				float y = object_cast<float>(item[2]);
				float z = object_cast<float>(item[3]);
				positions.push_back(PARAVECTOR3(x, y, z));
			}
		}

		auto obj = m_pObj->CreateConvexHullShape(&positions[0], positions.size());
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateShape(ShapeType shapeType, const object& params)
	{
		if (!IsValid())
			return ParaAttributeObject();

		if (shapeType >= ShapeType::MIN && shapeType < ShapeType::MAX)
			return (this->*(m_pCrateShapeFunc[shapeType]))(params);
		else
			return ParaAttributeObject();
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCylinderShapeY(const object& halfExtents)
	{
		if (type(halfExtents) != LUA_TTABLE)
			return ParaAttributeObject();

		float x = object_cast<float>(halfExtents[1]);
		float y = object_cast<float>(halfExtents[2]);
		float z = object_cast<float>(halfExtents[3]);

		auto obj = m_pObj->CreateCylinderShapeY(PARAVECTOR3(x, y, z));
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCylinderShapeX(const object& halfExtents)
	{
		if (type(halfExtents) != LUA_TTABLE)
			return ParaAttributeObject();

		float x = object_cast<float>(halfExtents[1]);
		float y = object_cast<float>(halfExtents[2]);
		float z = object_cast<float>(halfExtents[3]);

		auto obj = m_pObj->CreateCylinderShapeX(PARAVECTOR3(x, y, z));
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCylinderShapeZ(const object& halfExtents)
	{
		if (type(halfExtents) != LUA_TTABLE)
			return ParaAttributeObject();

		float x = object_cast<float>(halfExtents[1]);
		float y = object_cast<float>(halfExtents[2]);
		float z = object_cast<float>(halfExtents[3]);

		auto obj = m_pObj->CreateCylinderShapeZ(PARAVECTOR3(x, y, z));
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateBoxShape(const object& boxHalfExtents)
	{
		if (type(boxHalfExtents) != LUA_TTABLE)
			return ParaAttributeObject();

		float x = object_cast<float>(boxHalfExtents[1]);
		float y = object_cast<float>(boxHalfExtents[2]);
		float z = object_cast<float>(boxHalfExtents[3]);

		auto obj = m_pObj->CreateBoxShape(PARAVECTOR3(x, y, z));
		return ParaAttributeObject(obj);
	}
	
	ParaAttributeObject ParaPhysicsFactory::CreateSphereShape(const object& radius)
	{
		if (type(radius) != LUA_TNUMBER)
			return ParaAttributeObject();

		auto obj = m_pObj->CreateSphereShape(object_cast<float>(radius));
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCapsuleShapeY(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		float radius = object_cast<float>(params["radius"]);
		float height = object_cast<float>(params["height"]);

		auto obj = m_pObj->CreateCapsuleShapeY(radius, height);
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCapsuleShapeX(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		float radius = object_cast<float>(params["radius"]);
		float height = object_cast<float>(params["height"]);

		auto obj = m_pObj->CreateCapsuleShapeX(radius, height);
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateCapsuleShapeZ(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		float radius = object_cast<float>(params["radius"]);
		float height = object_cast<float>(params["height"]);

		auto obj = m_pObj->CreateCapsuleShapeZ(radius, height);
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateConeShapeY(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		float radius = object_cast<float>(params["radius"]);
		float height = object_cast<float>(params["height"]);

		auto obj = m_pObj->CreateConeShapeY(radius, height);
		return ParaAttributeObject(obj);
	}


	ParaAttributeObject ParaPhysicsFactory::CreateConeShapeX(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		float radius = object_cast<float>(params["radius"]);
		float height = object_cast<float>(params["height"]);

		auto obj = m_pObj->CreateConeShapeX(radius, height);
		return ParaAttributeObject(obj);
	}

	ParaAttributeObject ParaPhysicsFactory::CreateConeShapeZ(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return ParaAttributeObject();

		float radius = object_cast<float>(params["radius"]);
		float height = object_cast<float>(params["height"]);

		auto obj = m_pObj->CreateConeShapeZ(radius, height);
		return ParaAttributeObject(obj);
	}
}
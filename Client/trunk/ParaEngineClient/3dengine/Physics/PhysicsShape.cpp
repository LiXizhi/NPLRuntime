#include "ParaEngine.h"
#include "Physics/PhysicsShape.h"

#include <luabind/object.hpp>
#include "ParaScriptingPhysics.h"

namespace ParaEngine {
	CPhysicsShape::CPhysicsShape(IParaPhysicsShape* pShape)
		: m_pShape(pShape)
	{
		pShape->SetUserData(this);
	}

	CPhysicsShape::~CPhysicsShape()
	{
		SAFE_RELEASE(m_pShape);
	}

	IParaPhysicsShape* CPhysicsShape::get()
	{
		return m_pShape;
	}

	CPhysicsCompoundShape::CPhysicsCompoundShape(IParaPhysicsCompoundShape* pShape)
		: CPhysicsShape(pShape)
	{

	}

	int CPhysicsCompoundShape::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CPhysicsShape::InstallFields(pClass, bOverride);

		pClass->AddField("NumChild"
			, FieldType_Int
			, nullptr
			, GetNumChild_s
			, nullptr, nullptr, bOverride);


		pClass->AddField("AddChildShape"
			, FieldType_function
			, AddChildShape_s
			, nullptr
			, nullptr, nullptr, bOverride);

		pClass->AddField("RemoveChildShape"
			, FieldType_function
			, RemoveChildShape_s
			, nullptr
			, nullptr, nullptr, bOverride);

		pClass->AddField("RemoveChildByIndex"
			, FieldType_function
			, RemoveChildByIndex_s
			, nullptr
			, nullptr, nullptr, bOverride);

		pClass->AddField("GetChildShape"
			, FieldType_function
			, GetChildShape_s
			, nullptr
			, nullptr, nullptr, bOverride);

		pClass->AddField("GetChildTransform"
			, FieldType_function
			, GetChildTransform_s
			, nullptr
			, nullptr, nullptr, bOverride);

		pClass->AddField("UpdateChildTransform"
			, FieldType_function
			, UpdateChildTransform_s
			, nullptr
			, nullptr, nullptr, bOverride);

		return S_OK;
	}

	HRESULT CPhysicsCompoundShape::AddChildShape_s(CPhysicsCompoundShape* cls, const luabind::object& in, luabind::object& out)
	{
		if (luabind::type(in) != LUA_TTABLE)
			return S_FALSE;

		CPhysicsShape* pShape = nullptr;
		ParaScripting::ReadLuaTablePhysicsShape(in, "shape", pShape);
		if (!pShape)
			return S_FALSE;

		PARAMATRIX3x3 mRotation;
		if (!ParaScripting::ReadLuaTableMatrix3x3(in, "rotation", mRotation))
			return S_FALSE;

		PARAVECTOR3 vOrigin;
		if (!ParaScripting::ReadLuaTableVector3(in, "origin", vOrigin))
			return S_FALSE;

		cls->AddChildShape(mRotation, vOrigin, pShape);

		return S_OK;
	}

	HRESULT CPhysicsCompoundShape::RemoveChildShape_s(CPhysicsCompoundShape* cls, const luabind::object& in, luabind::object& out)
	{
		CPhysicsShape* pShape = nullptr;
		ParaScripting::ReadLuaPhysicsShape(in, pShape);
		if (!pShape)
			return S_FALSE;

		cls->RemoveChildShape(pShape);

		return S_OK;
	}

	HRESULT CPhysicsCompoundShape::RemoveChildByIndex_s(CPhysicsCompoundShape* cls, const luabind::object& in, luabind::object& out)
	{
		int index;
		if (!ParaScripting::ReadLuaInt(in, index))
			return S_FALSE;

		cls->RemoveChildByIndex(index);

		return S_OK;
	}
	
	HRESULT CPhysicsCompoundShape::GetChildShape_s(CPhysicsCompoundShape* cls, const luabind::object& in, luabind::object& out)
	{
		int index;
		if (!ParaScripting::ReadLuaInt(in, index))
			return S_FALSE;

		auto pShape = cls->GetChildShape(index);

		if (!pShape)
			return S_FALSE;

		out = luabind::object(in.interpreter(), pShape);

		return S_OK;
	}

	HRESULT CPhysicsCompoundShape::GetChildTransform_s(CPhysicsCompoundShape* cls, const luabind::object& in, luabind::object& out)
	{
		int index;
		if (!ParaScripting::ReadLuaInt(in, index))
			return S_FALSE;

		PARAMATRIX3x3 m;
		PARAVECTOR3 v;

		cls->GetChildTransform(index, m, v);

		out = luabind::newtable(in.interpreter());

		luabind::object origin = luabind::newtable(in.interpreter());
		ParaScripting::Vector32Object(v, origin);

		luabind::object rotation = luabind::newtable(in.interpreter());
		ParaScripting::Matrix3x32Object(m, rotation);

		out["origin"] = origin;
		out["rotation"] = rotation;

		return S_OK;
	}

	HRESULT CPhysicsCompoundShape::UpdateChildTransform_s(CPhysicsCompoundShape* cls, const luabind::object& in, luabind::object& out)
	{
		if (luabind::type(in) != LUA_TTABLE)
			return S_FALSE;

		int index;
		if (!ParaScripting::ReadLuaTableInt(in, "index", index))
			return S_FALSE;

		PARAMATRIX3x3 m;
		if (!ParaScripting::ReadLuaTableMatrix3x3(in, "rotation", m))
			return S_FALSE;

		PARAVECTOR3 v;
		if (!ParaScripting::ReadLuaTableVector3(in, "origin", v))
			return S_FALSE;

		bool shouldRecalculateLocalAabb;
		if (!ParaScripting::ReadLuaTableBoolean(in, "shouldRecalculateLocalAabb", shouldRecalculateLocalAabb))
			return S_FALSE;

		cls->UpdateChildTransform(index, m, v, shouldRecalculateLocalAabb);

		return S_OK;
	}

	CPhysicsCompoundShape::~CPhysicsCompoundShape()
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());

		// release all children
		for (int i = 0; i < thisShape->GetNumChild(); i++)
		{
			auto childShapeObj = thisShape->GetChildShape(i);
			if (childShapeObj)
			{
				auto childShape = static_cast<CPhysicsShape*>(childShapeObj->GetUserData());
				if (childShape)
				{
					childShape->Release();
				}
			}
		}
	}

	void CPhysicsCompoundShape::RemoveChildShape(CPhysicsShape* pShape)
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		thisShape->RemoveChildShape(pShape->get());
		pShape->Release();
	}

	/* remove by index */
	void CPhysicsCompoundShape::RemoveChildByIndex(int index)
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		auto childShapeObj = thisShape->GetChildShape(index);
		thisShape->RemoveChildByIndex(index);

		if (childShapeObj)
		{
			auto childShape = static_cast<CPhysicsShape*>(childShapeObj->GetUserData());
			if (childShape)
			{
				childShape->Release();
			}
		}
	}

	int CPhysicsCompoundShape::GetNumChild()
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		return thisShape->GetNumChild();
	}

	/* get child by index */
	CPhysicsShape* CPhysicsCompoundShape::GetChildShape(int index)
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		auto childShapeObj = thisShape->GetChildShape(index);

		if (childShapeObj)
		{
			return static_cast<CPhysicsShape*>(childShapeObj->GetUserData());
		}
		else
		{
			return nullptr;
		}
	}

	/* get transform of child by index */
	void CPhysicsCompoundShape::GetChildTransform(int index, PARAMATRIX3x3& rotation, PARAVECTOR3& origin)
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		thisShape->GetChildTransform(index, rotation, origin);
	}

	void CPhysicsCompoundShape::UpdateChildTransform(int index
		, const PARAMATRIX3x3& rotation
		, const PARAVECTOR3& origin
		, bool shouldRecalculateLocalAabb)
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		thisShape->UpdateChildTransform(index, rotation, origin, shouldRecalculateLocalAabb);
	}

	void CPhysicsCompoundShape::AddChildShape(const PARAMATRIX3x3& rotation
		, const PARAVECTOR3& origin
		, CPhysicsShape* pShape)
	{
		auto thisShape = static_cast<IParaPhysicsCompoundShape*>(get());
		auto pShapeObj = pShape->get();
		thisShape->AddChildShape(rotation, origin, pShapeObj);
		pShape->addref();
	}

	CPhysicsTriangleMeshShape::CPhysicsTriangleMeshShape(IParaPhysicsTriangleMeshShape* pShape)
		: CPhysicsShape(pShape)
	{

	}

	CPhysicsTriangleMeshShape::~CPhysicsTriangleMeshShape()
	{

	}

	CPhysicsScaledTriangleMeshShape::CPhysicsScaledTriangleMeshShape(IParaPhysicsScalingTriangleMeshShape* pShape, CPhysicsShape* pChild)
		: CPhysicsShape(pShape)
		, m_pChild(pChild)
	{

	}

	CPhysicsScaledTriangleMeshShape::~CPhysicsScaledTriangleMeshShape()
	{

	}

	CPhysicsTriangleMeshShape* CPhysicsScaledTriangleMeshShape::GetChildShape()
	{
		auto thisShape = static_cast<IParaPhysicsScalingTriangleMeshShape*>(get());
		thisShape->getChildShape();

		if (thisShape)
		{
			auto ret = static_cast<CPhysicsTriangleMeshShape*>(thisShape->GetUserData());
			assert(m_pChild.get() == ret);

			return ret;
		}
		return nullptr;
	}
}
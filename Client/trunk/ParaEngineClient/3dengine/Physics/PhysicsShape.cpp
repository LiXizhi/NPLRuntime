#include "ParaEngine.h"
#include "Physics/PhysicsShape.h"

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
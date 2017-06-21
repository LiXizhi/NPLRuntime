#pragma once

#include "Physics/PhysicsObject.h"

namespace ParaEngine {
	class CPhysicsShape : public CPhysicsObject
	{
		friend class CPhysicsFactory;
	public:
		virtual ~CPhysicsShape();

		IParaPhysicsShape* get();

		ATTRIBUTE_DEFINE_CLASS(CPhysicsShape);


		typedef weak_ptr<IObject, CPhysicsShape>  WeakPtr;
	protected:
		CPhysicsShape(IParaPhysicsShape* pShape);

	private:
		IParaPhysicsShape* m_pShape;
	};

	class CPhysicsCompoundShape : public CPhysicsShape
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsCompoundShape(IParaPhysicsCompoundShape* pShape);
	public:
		virtual ~CPhysicsCompoundShape();

		ATTRIBUTE_DEFINE_CLASS(CPhysicsCompoundShape);

		/* add */
		void AddChildShape(const PARAMATRIX3x3& rotation
			, const PARAVECTOR3& origin
			, CPhysicsShape* pShape);

		/* remove */
		void RemoveChildShape(CPhysicsShape* pShape);

		/* remove by index */
		void RemoveChildByIndex(int index);

		/* get num of child */
		int GetNumChild();

		/* get child by index */
		CPhysicsShape* GetChildShape(int index);

		/* get transform of child by index */
		void GetChildTransform(int index, PARAMATRIX3x3& rotation, PARAVECTOR3& origin);

		/* set a new transform for a child, and update internal data structures */
		void UpdateChildTransform(int index
			, const PARAMATRIX3x3& rotation
			, const PARAVECTOR3& origin
			, bool shouldRecalculateLocalAabb);
	};


	class CPhysicsTriangleMeshShape : public CPhysicsShape
	{
		friend class CPhysicsFactory;
	public:
		virtual ~CPhysicsTriangleMeshShape();

		ATTRIBUTE_DEFINE_CLASS(CPhysicsScaledTriangleMeshShape);

	protected:
		CPhysicsTriangleMeshShape(IParaPhysicsTriangleMeshShape* pShape);
	};


	class CPhysicsScaledTriangleMeshShape : public CPhysicsShape
	{
		friend class CPhysicsFactory;
	public:
		virtual ~CPhysicsScaledTriangleMeshShape();

		ATTRIBUTE_DEFINE_CLASS(CPhysicsScaledTriangleMeshShape);


		CPhysicsTriangleMeshShape* GetChildShape();
	protected:
		CPhysicsScaledTriangleMeshShape(IParaPhysicsScalingTriangleMeshShape* pShape, CPhysicsShape* pChild);

	private:
		ref_ptr<CPhysicsShape> m_pChild;
	};
}
#pragma once

#include "btBulletDynamicsCommon.h"
#include "Physics/IParaPhysics.h"

namespace ParaEngine {

	// use template instead of Virtual Inheritance
	template< class Base >
	struct _BulletShape : public Base
	{
		_BulletShape() : m_pShape(nullptr)
		{
		};

		virtual ~_BulletShape()
		{
			SAFE_DELETE(m_pShape);
		};

		virtual void* GetUserData()
		{
			return m_pUserData;
		};

		virtual void SetUserData(void* pData)
		{
			m_pUserData = pData;
		};

		virtual void* get()
		{
			return m_pShape;
		};

		virtual void Release()
		{
			delete this;
		};


		virtual void SetLocalScaling(const PARAVECTOR3& scaling)
		{
			assert(m_pShape);
			m_pShape->setLocalScaling(btVector3(btScalar(scaling.x), btScalar(scaling.y), btScalar(scaling.z)));
		}

		virtual void GetLocalScaling(PARAVECTOR3& scaling)
		{
			assert(m_pShape);
			const auto& value = m_pShape->getLocalScaling();

			scaling.x = (float)value.getX();
			scaling.y = (float)value.getY();
			scaling.z = (float)value.getZ();
		}

		virtual void CalculateLocalInertia(float mass, PARAVECTOR3& inertia)
		{
			assert(m_pShape);
			btVector3 localInertia(0, 0, 0);
			m_pShape->calculateLocalInertia(btScalar(mass), localInertia);

			inertia.x = (float)localInertia.getX();
			inertia.y = (float)localInertia.getY();
			inertia.z = (float)localInertia.getZ();
		}

		/// pointer to the low level physics engine shape object. 
		btCollisionShape* m_pShape;

		/// keep some user data here
		void* m_pUserData;
	};

	typedef _BulletShape<IParaPhysicsShape> BulletShape;


	struct BulletCompoundShape : public _BulletShape<IParaPhysicsCompoundShape>
	{
		BulletCompoundShape()
		{
		};
		virtual ~BulletCompoundShape()
		{
		};

		virtual void AddChildShape(const PARAMATRIX3x3& rotation
			, const PARAVECTOR3& origin
			, IParaPhysicsShape* pShape)
		{
			assert(m_pShape);

			if (!pShape)
				return;

			auto pChild = static_cast<btCollisionShape*>(pShape->get());

			if (!pChild)
				return;


			btMatrix3x3 matRot(
				btScalar(rotation._11), btScalar(rotation._21), btScalar(rotation._31),
				btScalar(rotation._12), btScalar(rotation._22), btScalar(rotation._32),
				btScalar(rotation._13), btScalar(rotation._23), btScalar(rotation._33)
			);
			btVector3 vOrigin(btScalar(origin.x), btScalar(origin.y), btScalar(origin.z));
			btTransform startTransform(matRot, vOrigin);

			(static_cast<btCompoundShape*>(m_pShape))->addChildShape(startTransform, pChild);
		};

		virtual void RemoveChildShape(IParaPhysicsShape* pShape)
		{
			assert(m_pShape && pShape);
			auto pbtShape = static_cast<btCollisionShape*>(pShape->get());
			(static_cast<btCompoundShape*>(m_pShape))->removeChildShape(pbtShape);
		}

		virtual void RemoveChildByIndex(int index)
		{
			assert(m_pShape && index >= 0);
			(static_cast<btCompoundShape*>(m_pShape))->removeChildShapeByIndex(index);
		};

		virtual int GetNumChild()
		{
			assert(m_pShape);
			return (static_cast<btCompoundShape*>(m_pShape))->getNumChildShapes();
		};

		virtual IParaPhysicsShape* GetChildShape(int index)
		{
			assert(m_pShape && index >= 0);
			auto pbtShape = (static_cast<btCompoundShape*>(m_pShape))->getChildShape(index);
			if (!pbtShape)
				return nullptr;

			auto pShape = pbtShape->getUserPointer();
			return static_cast<IParaPhysicsShape*>(pShape);
		};

		virtual void GetChildTransform(int index, PARAMATRIX3x3& rotation, PARAVECTOR3& origin)
		{
			assert(m_pShape && index >= 0);

			const auto& transform = (static_cast<btCompoundShape*>(m_pShape))->getChildTransform(index);
			const auto& btBasis = transform.getBasis();
			const auto& btOrigin = transform.getOrigin();

			rotation._11 = (float)btBasis.getRow(0).getX();
			rotation._21 = (float)btBasis.getRow(0).getY();
			rotation._31 = (float)btBasis.getRow(0).getZ();

			rotation._12 = (float)btBasis.getRow(1).getX();
			rotation._22 = (float)btBasis.getRow(1).getY();
			rotation._32 = (float)btBasis.getRow(1).getZ();

			rotation._13 = (float)btBasis.getRow(2).getX();
			rotation._23 = (float)btBasis.getRow(2).getY();
			rotation._33 = (float)btBasis.getRow(2).getZ();

			origin.x = (float)btOrigin.getX();
			origin.y = (float)btOrigin.getY();
			origin.z = (float)btOrigin.getZ();
		};

		virtual void UpdateChildTransform(int index
			, const PARAMATRIX3x3& rotation
			, const PARAVECTOR3& origin
			, bool shouldRecalculateLocalAabb)
		{
			assert(m_pShape && index >= 0);

			btMatrix3x3 matRot(
				btScalar(rotation._11), btScalar(rotation._21), btScalar(rotation._31),
				btScalar(rotation._12), btScalar(rotation._22), btScalar(rotation._32),
				btScalar(rotation._13), btScalar(rotation._23), btScalar(rotation._33)
			);
			btVector3 vOrigin(btScalar(origin.x), btScalar(origin.y), btScalar(origin.z));
			btTransform startTransform(matRot, vOrigin);

			(static_cast<btCompoundShape*>(m_pShape))->updateChildTransform(index, startTransform, shouldRecalculateLocalAabb);
		};
	};

	/** it is represent a shape that can be used to create various actors in the scene. */
	struct BulletTriangleMeshShape : public _BulletShape<IParaPhysicsTriangleMeshShape>
	{
		BulletTriangleMeshShape()
			: m_indexVertexArrays(nullptr)
			, m_triangleIndices(nullptr)
			, m_vertices(nullptr)
		{
		};

		virtual ~BulletTriangleMeshShape()
		{
			SAFE_DELETE(m_indexVertexArrays);
			SAFE_DELETE_ARRAY(m_triangleIndices);
			SAFE_DELETE_ARRAY(m_vertices);
		};

		btTriangleIndexVertexArray* m_indexVertexArrays;
		int32* m_triangleIndices;
		btScalar* m_vertices;
	};

	struct BulletScalingTriangleMeshShape : public _BulletShape<IParaPhysicsScalingTriangleMeshShape>
	{
		virtual IParaPhysicsTriangleMeshShape* getChildShape()
		{
			assert(m_pShape);
			auto pShape = (static_cast<btScaledBvhTriangleMeshShape*>(m_pShape));
			auto pChild = pShape->getChildShape();

			if (pChild)
			{
				return static_cast<IParaPhysicsTriangleMeshShape*>(pChild->getUserPointer());
			}
			return nullptr;
		}
	};
}
#include "ParaEngine.h"
#include "ParaScriptingPhysics.h"
#include "Physics/PhysicsFactory.h"
#include "Physics/PhysicsObject.h"
#include "Physics/PhysicsDynamicsWorld.h"


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
	ParaAttributeObject* ParaPhysicsFactory::GetAttributeObject()
	{
		if (IsValid())
			return new ParaAttributeObject(m_pObj);
		else
			return nullptr;
	}

	/** for API exportation*/
	void ParaPhysicsFactory::GetAttributeObject_(ParaAttributeObject& output)
	{
		output = *GetAttributeObject();
	}

	ParaAttributeObject* ParaPhysicsFactory::GetCurrentWorld()
	{
		if (IsValid())
		{
			return new ParaAttributeObject(m_pObj->GetCurrentWorld());
		}
		else
		{
			return nullptr;
		}
	}

	ParaAttributeObject* ParaPhysicsFactory::CreatePhysicsObject(ObjectType oType, int subType, const object& params)
	{
		if (!IsValid())
			return nullptr;

		switch (oType)
		{
		case ObjectType::OT_Shape:
			return CreateShape((ShapeType)subType, params);
			break;
		case ObjectType::OT_Rigidbody:
			return CreateRigidbody(params);
			break;
		case ObjectType::OT_Constraint:
			return CreateConstraint((ConstraintType)subType, params);
			break;
		default:
			return nullptr;
			break;
		}
	}

	ParaPhysicsFactory::CreateConstraintFunc ParaPhysicsFactory::m_pCreateConstraintFunc[ParaPhysicsFactory::ConstraintType::CT_MAX] =
	{
		&ParaPhysicsFactory::CreatePoint2PointConstraint
		, &ParaPhysicsFactory::CreateHingeConstraint
		, &ParaPhysicsFactory::CreateSliderConstraint
		, &ParaPhysicsFactory::CreateConeTwistConstraint
		, &ParaPhysicsFactory::CreateGeneric6DofSpringConstraint
	};

	ParaAttributeObject* ParaPhysicsFactory::CreateGeneric6DofSpringConstraint(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		CPhysicsGeneric6DofSpringConstraint* obj = nullptr;

		auto& rbA = params["rbA"];
		ParaAttributeObject* rigidBodyA;
		Object2RigidBody(rbA, rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		auto& useLinearReferenceFrameA = params["useLinearReferenceFrameA"];
		if (type(useLinearReferenceFrameA) != LUA_TBOOLEAN)
			return nullptr;
		auto bUseLinearReferenceFrameA = object_cast<bool>(useLinearReferenceFrameA);

		auto& rbAOrigin = params["rbAOrigin"];
		PARAVECTOR3 v3RbAOrigin;
		if (!Object2Vector3(rbAOrigin, v3RbAOrigin))
			return nullptr;

		auto& rbARotation = params["rbARotation"];
		PARAMATRIX3x3 mRbARotation;
		if (!Object2Matrix3x3(rbARotation, mRbARotation))
			return nullptr;

		auto& rbB = params["rbB"];
		ParaAttributeObject* rigidBodyB;
		Object2RigidBody(rbB, rigidBodyB);

		if (rigidBodyB)
		{
			auto& rbBOrigin = params["rbBOrigin"];
			PARAVECTOR3 v3RbBOrigin;
			if (!Object2Vector3(rbBOrigin, v3RbBOrigin))
				return nullptr;

			auto& rbBRotation = params["rbBRotation"];
			PARAMATRIX3x3 mRbBRotation;
			if (!Object2Matrix3x3(rbBRotation, mRbBRotation))
				return nullptr;

			obj = m_pObj->CreateGeneric6DofSpringConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, static_cast<CPhysicsRigidBody*>(rigidBodyB->m_pAttribute.get())
				, v3RbAOrigin
				, mRbARotation
				, v3RbBOrigin
				, mRbBRotation
				, bUseLinearReferenceFrameA);
		}
		else
		{
			obj = m_pObj->CreateGeneric6DofSpringConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, v3RbAOrigin
				, mRbARotation
				, bUseLinearReferenceFrameA);
		}

		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConeTwistConstraint(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		CPhysicsConeTwistConstraint* obj = nullptr;

		auto& rbA = params["rbA"];
		ParaAttributeObject* rigidBodyA;
		Object2RigidBody(rbA, rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		auto& rbAOrigin = params["rbAOrigin"];
		PARAVECTOR3 v3RbAOrigin;
		if (!Object2Vector3(rbAOrigin, v3RbAOrigin))
			return nullptr;

		auto& rbARotation = params["rbARotation"];
		PARAMATRIX3x3 mRbARotation;
		if (!Object2Matrix3x3(rbARotation, mRbARotation))
			return nullptr;

		auto& rbB = params["rbB"];
		ParaAttributeObject* rigidBodyB;
		Object2RigidBody(rbB, rigidBodyB);

		if (rigidBodyB)
		{
			auto& rbBOrigin = params["rbBOrigin"];
			PARAVECTOR3 v3RbBOrigin;
			if (!Object2Vector3(rbBOrigin, v3RbBOrigin))
				return nullptr;

			auto& rbBRotation = params["rbBRotation"];
			PARAMATRIX3x3 mRbBRotation;
			if (!Object2Matrix3x3(rbBRotation, mRbBRotation))
				return nullptr;

			obj = m_pObj->CreateConeTwistConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, static_cast<CPhysicsRigidBody*>(rigidBodyB->m_pAttribute.get())
				, v3RbAOrigin
				, mRbARotation
				, v3RbBOrigin
				, mRbBRotation);
		}
		else
		{
			obj = m_pObj->CreateConeTwistConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, v3RbAOrigin
				, mRbARotation);
		}

		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateSliderConstraint(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		CPhysicsSliderConstraint* obj = nullptr;

		auto& rbA = params["rbA"];
		ParaAttributeObject* rigidBodyA;
		Object2RigidBody(rbA, rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		auto& useLinearReferenceFrameA = params["useLinearReferenceFrameA"];
		if (type(useLinearReferenceFrameA) != LUA_TBOOLEAN)
			return nullptr;
		auto bUseLinearReferenceFrameA = object_cast<bool>(useLinearReferenceFrameA);

		auto& rbAOrigin = params["rbAOrigin"];
		PARAVECTOR3 v3RbAOrigin;
		if (!Object2Vector3(rbAOrigin, v3RbAOrigin))
			return nullptr;

		auto& rbARotation = params["rbARotation"];
		PARAMATRIX3x3 mRbARotation;
		if (!Object2Matrix3x3(rbARotation, mRbARotation))
			return nullptr;

		auto& rbB = params["rbB"];
		ParaAttributeObject* rigidBodyB;
		Object2RigidBody(rbB, rigidBodyB);

		if (rigidBodyB)
		{
			auto& rbBOrigin = params["rbBOrigin"];
			PARAVECTOR3 v3RbBOrigin;
			if (!Object2Vector3(rbBOrigin, v3RbBOrigin))
				return nullptr;

			auto& rbBRotation = params["rbBRotation"];
			PARAMATRIX3x3 mRbBRotation;
			if (!Object2Matrix3x3(rbBRotation, mRbBRotation))
				return nullptr;

			obj = m_pObj->CreateSliderConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, static_cast<CPhysicsRigidBody*>(rigidBodyB->m_pAttribute.get())
				, v3RbAOrigin
				, mRbARotation
				, v3RbBOrigin
				, mRbBRotation
				, bUseLinearReferenceFrameA);
		}
		else
		{
			obj = m_pObj->CreateSliderConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, v3RbAOrigin
				, mRbARotation
				, bUseLinearReferenceFrameA);
		}

		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateHingeConstraint(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		CPhysicsHingeConstraint* obj = nullptr;

		auto& rbA = params["rbA"];
		ParaAttributeObject* rigidBodyA;
		Object2RigidBody(rbA, rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		auto& useReferenceFrameA = params["useReferenceFrameA"];
		bool bUseReferenceFrameA = false;
		if (type(useReferenceFrameA) == LUA_TBOOLEAN)
			bUseReferenceFrameA = object_cast<bool>(useReferenceFrameA);

		auto& rbB = params["rbB"];
		ParaAttributeObject* rigidBodyB;
		Object2RigidBody(rbB, rigidBodyB);

		auto& pivotInA = params["pivotInA"];
		PARAVECTOR3 v3PivotInA;
		bool bUsePivotInA = Object2Vector3(pivotInA, v3PivotInA);

		if (bUsePivotInA)
		{
			auto& axisInA = params["axisInA"];
			PARAVECTOR3 v3AxisInA;
			if (!Object2Vector3(axisInA, v3AxisInA))
				return nullptr;

			if (rigidBodyB)
			{
				auto& pivotInB = params["pivotInB"];
				PARAVECTOR3 v3PivotInB;
				bool bUsePivotInB = Object2Vector3(pivotInB, v3PivotInB);

				if (bUsePivotInB)
				{
					auto& axisInB = params["axisInB"];
					PARAVECTOR3 v3AxisInB;
					if (!Object2Vector3(axisInB, v3AxisInB))
						return nullptr;

					obj = m_pObj->CreateHingeConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
						, static_cast<CPhysicsRigidBody*>(rigidBodyB->m_pAttribute.get())
						, v3PivotInA
						, v3PivotInB
						, v3AxisInA
						, v3PivotInB
						, bUseReferenceFrameA);
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				obj = m_pObj->CreateHingeConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
					, v3PivotInA
					, v3AxisInA
					, bUseReferenceFrameA);
			}
		}
		else
		{
			auto& rbAOrigin = params["rbAOrigin"];
			PARAVECTOR3 v3RbAOrigin;
			if (!Object2Vector3(rbAOrigin, v3RbAOrigin))
				return nullptr;

			auto& rbARotation = params["rbARotation"];
			PARAMATRIX3x3 mRbARotation;
			if (!Object2Matrix3x3(rbARotation, mRbARotation))
				return nullptr;

			if (rigidBodyB)
			{
				auto& rbBOrigin = params["rbBOrigin"];
				PARAVECTOR3 v3RbBOrigin;
				if (!Object2Vector3(rbBOrigin, v3RbBOrigin))
					return nullptr;

				auto& rbBRotation = params["rbBRotation"];
				PARAMATRIX3x3 mRbBRotation;
				if (!Object2Matrix3x3(rbBRotation, mRbBRotation))
					return nullptr;

				obj = m_pObj->CreateHingeConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
					, static_cast<CPhysicsRigidBody*>(rigidBodyB->m_pAttribute.get())
					, v3RbAOrigin
					, mRbARotation
					, v3RbBOrigin
					, mRbBRotation
					, bUseReferenceFrameA);
			}
			else
			{
				obj = m_pObj->CreateHingeConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
					, v3RbAOrigin
					, mRbARotation
					, bUseReferenceFrameA);
			}
		}
		
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreatePoint2PointConstraint(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		auto& rbA = params["rbA"];
		ParaAttributeObject* rigidBodyA;
		Object2RigidBody(rbA, rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		auto& pivotInA = params["pivotInA"];
		PARAVECTOR3 v3PivotInA;
		if (!Object2Vector3(pivotInA, v3PivotInA))
			return nullptr;

		CPhysicsP2PConstraint* obj = nullptr;

		auto& rbB = params["rbB"];
		ParaAttributeObject* rigidBodyB;
		Object2RigidBody(rbB, rigidBodyB);
		if (rigidBodyB)
		{
			auto& pivotInB = params["pivotInB"];
			PARAVECTOR3 v3PivotInB;
			if (!Object2Vector3(pivotInB, v3PivotInB))
				return nullptr;

			obj = m_pObj->CreatePoint2PointConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, static_cast<CPhysicsRigidBody*>(rigidBodyB->m_pAttribute.get())
				, v3PivotInA
				, v3PivotInB);
		}	
		else
		{
			obj = m_pObj->CreatePoint2PointConstraint(static_cast<CPhysicsRigidBody*>(rigidBodyA->m_pAttribute.get())
				, v3PivotInA);
		}

		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConstraint(ConstraintType cType, const object& params)
	{
		if (cType >= ConstraintType::CT_MIN && cType < ConstraintType::CT_MAX)
			return (this->*(m_pCreateConstraintFunc[cType]))(params);
		else
			return nullptr;
	}

	ParaPhysicsFactory::CreateShapeFunc ParaPhysicsFactory::m_pCrateShapeFunc[ParaPhysicsFactory::ShapeType::ST_MAX] =
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

		, &ParaPhysicsFactory::CreateTriangleMeshShape

		, &ParaPhysicsFactory::CreateScaledTriangleMeshShape
	};

	ParaAttributeObject* ParaPhysicsFactory::CreateScaledTriangleMeshShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		auto& triangleMeshShape = params["triangleMeshShape"];
		if (type(triangleMeshShape) != LUA_TUSERDATA)
			//return ParaAttributeObject();
			return nullptr;

		auto shapeObj = object_cast<ParaAttributeObject*>(triangleMeshShape);
		if (!shapeObj->IsValid() || shapeObj->GetClassID() != ATTRIBUTE_CLASSID_CPhysicsTriangleMeshShape)
			//return ParaAttributeObject();
			return nullptr;

		auto& localScaling = params["localScaling"];
		PARAVECTOR3 v3;
		if (!Object2Vector3(localScaling, v3))
			//return ParaAttributeObject();
			return nullptr;

		auto pShape = static_cast<CPhysicsTriangleMeshShape*>(shapeObj->m_pAttribute.get());
		auto obj = m_pObj->CreateScaledTriangleMeshShape(pShape, v3);

		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateTriangleMeshShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		auto& flags = params["flags"];
		if (type(flags) != LUA_TNUMBER)
			//return ParaAttributeObject();
			return nullptr;

		auto& points = params["points"];
		if (type(points) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		auto& triangles = params["triangles"];
		if (type(triangles) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		DWORD dwFlag = object_cast<DWORD>(flags);

		std::vector<PARAVECTOR3> vecPoints;
		for (luabind::iterator itCur(points), itEnd; itCur != itEnd; ++itCur)
		{
			const object& item = *itCur;

			auto size = vecPoints.size();
			vecPoints.resize(size + 1);
			if (!Object2Vector3(item, vecPoints[size]))
				//return ParaAttributeObject();
				return nullptr;
		}

		std::vector<WORD> vecIndices;
		for (luabind::iterator itCur(points), itEnd; itCur != itEnd; ++itCur)
		{
			const object& item = *itCur;
			if (type(item) == LUA_TNUMBER)
			{
				vecIndices.push_back(object_cast<WORD>(item));
			}
		}

		if (vecIndices.size() % 3 != 0)
			//return ParaAttributeObject();
			return nullptr;

		ParaPhysicsTriangleMeshDesc trimeshDesc;
		trimeshDesc.m_numVertices = vecPoints.size();
		trimeshDesc.m_numTriangles = vecIndices.size() / 3;
		trimeshDesc.m_pointStrideBytes = sizeof(PARAVECTOR3);
		trimeshDesc.m_triangleStrideBytes = 3 * sizeof(int16);
		trimeshDesc.m_points = &vecPoints[0];
		trimeshDesc.m_triangles = &vecIndices[0];
		trimeshDesc.m_flags = 0;

		auto obj = m_pObj->CreateTriangleMeshShape(trimeshDesc);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateStaticPlaneShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		auto& planeNormal = params["planeNormal"];
		PARAVECTOR3 v3;
		if (!Object2Vector3(planeNormal, v3))
			//return ParaAttributeObject();
			return nullptr;


		auto& planeConstant = params["planeConstant"];
		if (type(planeConstant) != LUA_TNUMBER)
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateStaticPlaneShape(v3, object_cast<float>(planeConstant));
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCompoundShape(const object& enableDynamicAabbTree)
	{
		if (type(enableDynamicAabbTree) != LUA_TBOOLEAN)
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateCompoundShape(object_cast<bool>(enableDynamicAabbTree));
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConvexHullShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		std::vector<PARAVECTOR3> positions;
		for (luabind::iterator itCur(params), itEnd; itCur != itEnd; ++itCur)
		{
			const object& item = *itCur;
			auto size = positions.size();
			positions.resize(size + 1);
			if (!Object2Vector3(item, positions[size]))
				//return ParaAttributeObject();
				return nullptr;
		}

		auto obj = m_pObj->CreateConvexHullShape(&positions[0], positions.size());
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateShape(ShapeType shapeType, const object& params)
	{

		if (shapeType >= ShapeType::ST_MIN && shapeType < ShapeType::ST_MAX)
			return (this->*(m_pCrateShapeFunc[shapeType]))(params);
		else
			return nullptr;
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCylinderShapeY(const object& halfExtents)
	{
		PARAVECTOR3 v3;

		if (!Object2Vector3(halfExtents, v3))
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateCylinderShapeY(v3);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCylinderShapeX(const object& halfExtents)
	{
		PARAVECTOR3 v3;

		if (!Object2Vector3(halfExtents, v3))
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateCylinderShapeX(v3);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCylinderShapeZ(const object& halfExtents)
	{
		PARAVECTOR3 v3;

		if (!Object2Vector3(halfExtents, v3))
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateCylinderShapeZ(v3);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateBoxShape(const object& boxHalfExtents)
	{
		PARAVECTOR3 v3;

		if (!Object2Vector3(boxHalfExtents, v3))
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateBoxShape(v3);
		return new ParaAttributeObject(obj);
	}
	
	ParaAttributeObject* ParaPhysicsFactory::CreateSphereShape(const object& radius)
	{
		if (type(radius) != LUA_TNUMBER)
			//return ParaAttributeObject();
			return nullptr;

		auto obj = m_pObj->CreateSphereShape(object_cast<float>(radius));
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCapsuleShapeY(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		auto& radius = params["radius"];
		if (type(radius) == LUA_TNUMBER)
			fRadius = object_cast<float>(radius);

		auto& height = params["height"];
		if (type(height) == LUA_TNUMBER)
			fHeight = object_cast<float>(height);


		auto obj = m_pObj->CreateCapsuleShapeY(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCapsuleShapeX(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		auto& radius = params["radius"];
		if (type(radius) == LUA_TNUMBER)
			fRadius = object_cast<float>(radius);

		auto& height = params["height"];
		if (type(height) == LUA_TNUMBER)
			fHeight = object_cast<float>(height);

		auto obj = m_pObj->CreateCapsuleShapeX(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCapsuleShapeZ(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		auto& radius = params["radius"];
		if (type(radius) == LUA_TNUMBER)
			fRadius = object_cast<float>(radius);

		auto& height = params["height"];
		if (type(height) == LUA_TNUMBER)
			fHeight = object_cast<float>(height);

		auto obj = m_pObj->CreateCapsuleShapeZ(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConeShapeY(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		auto& radius = params["radius"];
		if (type(radius) == LUA_TNUMBER)
			fRadius = object_cast<float>(radius);

		auto& height = params["height"];
		if (type(height) == LUA_TNUMBER)
			fHeight = object_cast<float>(height);

		auto obj = m_pObj->CreateConeShapeY(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}


	ParaAttributeObject* ParaPhysicsFactory::CreateConeShapeX(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		auto& radius = params["radius"];
		if (type(radius) == LUA_TNUMBER)
			fRadius = object_cast<float>(radius);

		auto& height = params["height"];
		if (type(height) == LUA_TNUMBER)
			fHeight = object_cast<float>(height);

		auto obj = m_pObj->CreateConeShapeX(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConeShapeZ(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		auto& radius = params["radius"];
		if (type(radius) == LUA_TNUMBER)
			fRadius = object_cast<float>(radius);

		auto& height = params["height"];
		if (type(height) == LUA_TNUMBER)
			fHeight = object_cast<float>(height);

		auto obj = m_pObj->CreateConeShapeZ(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateRigidbody_(const object& params, ParaPhysicsMotionStateDesc* motionStateDesc)
	{
		if (type(params) != LUA_TTABLE)
			//return ParaAttributeObject();
			return nullptr;

		// check shape
		ParaAttributeObject* shapeObj = nullptr;
		{
			auto& shape = params["shape"];
			if (type(shape) != LUA_TUSERDATA)
				//return ParaAttributeObject();
				return nullptr;

			shapeObj = object_cast<ParaAttributeObject*>(shape);
			auto classId = shapeObj->GetClassID();
			if (!(classId >= ATTRIBUTE_CLASSID_CPhysicsShapeMin && classId <= ATTRIBUTE_CLASSID_CPhysicsShapeMax))
				//return ParaAttributeObject();
				return nullptr;
		}


		float fMass = 0.0f;
		auto& mass = params["mass"];
		if (type(mass) == LUA_TNUMBER)
			fMass = object_cast<float>(mass);

		PARAVECTOR3 vOrigin;
		auto& origin = params["origin"];
		Object2Vector3(origin, vOrigin);

		PARAVECTOR3 vInertia;
		auto& inertia = params["inertia"];
		bool bUseInertia = Object2Vector3(inertia, vInertia);


		PARAMATRIX3x3 mRotation;
		auto& rotation = params["rotation"];
		Object2Matrix3x3(rotation, mRotation);

		short nGroup = 0;
		auto& group = params["group"];
		if (type(group) == LUA_TNUMBER)
			nGroup = object_cast<short>(group);

		int nMask = -1;
		auto& mask = params["mask"];
		if (type(mask) == LUA_TNUMBER)
			nMask = object_cast<int>(mask);


		bool bUseCenterOfMassOffset = false;
		auto& centerOfMassOffset = params["centerOfMassOffset"];
		PARAVECTOR3 vCenterOfMassOffsetOrigin;
		PARAMATRIX3x3 mCenterOfMassOffsetRotation;
		if (type(centerOfMassOffset) == LUA_TTABLE)
		{
			bUseCenterOfMassOffset = true;
			auto& origin = centerOfMassOffset["origin"];
			bUseCenterOfMassOffset &= Object2Vector3(origin, vCenterOfMassOffsetOrigin);

			if (bUseCenterOfMassOffset)
			{
				auto& rotation = params["rotation"];
				bUseCenterOfMassOffset &= Object2Matrix3x3(rotation, mCenterOfMassOffsetRotation);
			}
		}

		ParaPhysicsRigidbodyDesc desc;
		desc.m_group = nGroup;
		desc.m_mask = nMask;
		desc.m_mass = fMass;
		desc.m_origin = vOrigin;
		desc.m_rotation = mRotation;

		if (bUseInertia)
			desc.setLocalInertia(vInertia);
		if (bUseCenterOfMassOffset)
			desc.setCenterOfMassOffset(vCenterOfMassOffsetOrigin, mCenterOfMassOffsetRotation);

		auto pShape = static_cast<CPhysicsShape*>(shapeObj->m_pAttribute.get());
		desc.m_pShape = pShape->get();

		return new ParaAttributeObject(m_pObj->CreateRigidbody(desc, motionStateDesc));
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateRigidbody(const object& params)
	{
		auto& callback = params["callback"];

		return CreateRigidbody2(params, callback);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateRigidbody2(const object& params, const object& callback)
	{
		if (type(callback) == LUA_TFUNCTION)
		{
			ParaPhysicsMotionStateDesc motionStateDesc;
			motionStateDesc.cb = [callback](const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin)
			{
				object oRotation = newtable(callback.interpreter());
				Matrix3x32Object(rotation, oRotation);

				object oOrigin = newtable(callback.interpreter());
				Vector32Object(origin, oOrigin);

				try 
				{
					call_function<void>(callback, boost::ref(oRotation), boost::ref(oOrigin));
				}
				catch (luabind::error& e)
				{
					OUTPUT_LOG("LUA throw error: err_msg[%s]\n", lua_tostring(e.state(), -1));
				}
			};

			return CreateRigidbody_(params, &motionStateDesc);
		}
		else
		{
			return CreateRigidbody_(params, nullptr);
		}
	}
}
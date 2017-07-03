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

		CPhysicsRigidBody* rigidBodyA = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbA", rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		bool bUseLinearReferenceFrameA;
		if (!ReadLuaTableBoolean(params, "useLinearReferenceFrameA", bUseLinearReferenceFrameA))
			return nullptr;

		PARAVECTOR3 v3RbAOrigin;
		if (!ReadLuaTableVector3(params, "rbAOrigin", v3RbAOrigin))
			return nullptr;

		PARAMATRIX3x3 mRbARotation;
		if (!ReadLuaTableMatrix3x3(params, "rbARotation", mRbARotation))
			return nullptr;

		CPhysicsRigidBody* rigidBodyB = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbB", rigidBodyB);

		if (rigidBodyB)
		{

			PARAVECTOR3 v3RbBOrigin;
			if (!ReadLuaTableVector3(params, "rbBOrigin", v3RbBOrigin))
				return nullptr;

			PARAMATRIX3x3 mRbBRotation;
			if (!ReadLuaTableMatrix3x3(params, "rbBRotation", mRbBRotation))
				return nullptr;

			obj = m_pObj->CreateGeneric6DofSpringConstraint(rigidBodyA
				, rigidBodyB
				, v3RbAOrigin
				, mRbARotation
				, v3RbBOrigin
				, mRbBRotation
				, bUseLinearReferenceFrameA);
		}
		else
		{
			obj = m_pObj->CreateGeneric6DofSpringConstraint(rigidBodyA
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

		CPhysicsRigidBody* rigidBodyA = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbA", rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		PARAVECTOR3 v3RbAOrigin;
		if (!ReadLuaTableVector3(params, "rbAOrigin", v3RbAOrigin))
			return nullptr;

		PARAMATRIX3x3 mRbARotation;
		if (!ReadLuaTableMatrix3x3(params, "rbARotation", mRbARotation))
			return nullptr;

		CPhysicsRigidBody* rigidBodyB = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbB", rigidBodyB);

		if (rigidBodyB)
		{
			PARAVECTOR3 v3RbBOrigin;
			if (!ReadLuaTableVector3(params, "rbBOrigin", v3RbBOrigin))
				return nullptr;

			PARAMATRIX3x3 mRbBRotation;
			if (!ReadLuaTableMatrix3x3(params, "rbBRotation", mRbBRotation))
				return nullptr;

			obj = m_pObj->CreateConeTwistConstraint(rigidBodyA
				, rigidBodyB
				, v3RbAOrigin
				, mRbARotation
				, v3RbBOrigin
				, mRbBRotation);
		}
		else
		{
			obj = m_pObj->CreateConeTwistConstraint(rigidBodyA
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

		CPhysicsRigidBody* rigidBodyA = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbA", rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		bool bUseLinearReferenceFrameA;
		if (!ReadLuaTableBoolean(params, "useLinearReferenceFrameA", bUseLinearReferenceFrameA))
			return nullptr;

		PARAVECTOR3 v3RbAOrigin;
		if (!ReadLuaTableVector3(params, "rbAOrigin", v3RbAOrigin))
			return nullptr;

		PARAMATRIX3x3 mRbARotation;
		if (!ReadLuaTableMatrix3x3(params, "rbARotation", mRbARotation))
			return nullptr;

		CPhysicsRigidBody* rigidBodyB = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbB", rigidBodyB);

		if (rigidBodyB)
		{
			PARAVECTOR3 v3RbBOrigin;
			if (!ReadLuaTableVector3(params, "rbBOrigin", v3RbBOrigin))
				return nullptr;

			PARAMATRIX3x3 mRbBRotation;
			if (!ReadLuaTableMatrix3x3(params, "rbBRotation", mRbBRotation))
				return nullptr;

			obj = m_pObj->CreateSliderConstraint(rigidBodyA
				, rigidBodyB
				, v3RbAOrigin
				, mRbARotation
				, v3RbBOrigin
				, mRbBRotation
				, bUseLinearReferenceFrameA);
		}
		else
		{
			obj = m_pObj->CreateSliderConstraint(rigidBodyA
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

		CPhysicsRigidBody* rigidBodyA = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbA", rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		bool bUseReferenceFrameA;
		if (!ReadLuaTableBoolean(params, "useReferenceFrameA", bUseReferenceFrameA))
			bUseReferenceFrameA = false;

		CPhysicsRigidBody* rigidBodyB = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbB", rigidBodyB);


		PARAVECTOR3 v3PivotInA;
		bool bUsePivotInA = ReadLuaTableVector3(params, "pivotInA", v3PivotInA);

		if (bUsePivotInA)
		{
			PARAVECTOR3 v3AxisInA;
			if (!ReadLuaTableVector3(params, "axisInA", v3AxisInA))
				return nullptr;


			if (rigidBodyB)
			{
				PARAVECTOR3 v3PivotInB;
				bool bUsePivotInB = ReadLuaTableVector3(params, "pivotInB", v3PivotInB);


				if (bUsePivotInB)
				{
					PARAVECTOR3 v3AxisInB;
					if (!ReadLuaTableVector3(params, "axisInB", v3AxisInB))
						return nullptr;

					obj = m_pObj->CreateHingeConstraint(rigidBodyA
						, rigidBodyB
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
				obj = m_pObj->CreateHingeConstraint(rigidBodyA
					, v3PivotInA
					, v3AxisInA
					, bUseReferenceFrameA);
			}
		}
		else
		{
			PARAVECTOR3 v3RbAOrigin;
			if (!ReadLuaTableVector3(params, "rbAOrigin", v3RbAOrigin))
				return nullptr;

			PARAMATRIX3x3 mRbARotation;
			if (!ReadLuaTableMatrix3x3(params, "rbARotation", mRbARotation))
				return nullptr;

			if (rigidBodyB)
			{
				PARAVECTOR3 v3RbBOrigin;
				if (!ReadLuaTableVector3(params, "rbBOrigin", v3RbBOrigin))
					return nullptr;

				PARAMATRIX3x3 mRbBRotation;
				if (!ReadLuaTableMatrix3x3(params, "rbBRotation", mRbBRotation))
					return nullptr;

				obj = m_pObj->CreateHingeConstraint(rigidBodyA
					, rigidBodyB
					, v3RbAOrigin
					, mRbARotation
					, v3RbBOrigin
					, mRbBRotation
					, bUseReferenceFrameA);
			}
			else
			{
				obj = m_pObj->CreateHingeConstraint(rigidBodyA
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

		CPhysicsP2PConstraint* obj = nullptr;

		CPhysicsRigidBody* rigidBodyA = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbA", rigidBodyA);
		if (!rigidBodyA)
			return nullptr;

		PARAVECTOR3 v3PivotInA;
		if (!ReadLuaTableVector3(params, "pivotInA", v3PivotInA))
			return nullptr;

		CPhysicsRigidBody* rigidBodyB = nullptr;
		ReadLuaTablePhysicsRigidBody(params, "rbB", rigidBodyB);

		if (rigidBodyB)
		{
			PARAVECTOR3 v3PivotInB;
			if (!ReadLuaTableVector3(params, "pivotInB", v3PivotInB))
				return nullptr;

			obj = m_pObj->CreatePoint2PointConstraint(rigidBodyA
				, rigidBodyB
				, v3PivotInA
				, v3PivotInB);
		}	
		else
		{
			obj = m_pObj->CreatePoint2PointConstraint(rigidBodyA
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
			return nullptr;

		CPhysicsTriangleMeshShape* pShape = nullptr;
		if (!ReadLuaTablePhysicsTriangleMeshShape(params, "triangleMeshShape", pShape))
			return nullptr;

		PARAVECTOR3 v3;
		if (!ReadLuaTableVector3(params, "localScaling", v3))
			return nullptr;

		auto obj = m_pObj->CreateScaledTriangleMeshShape(pShape, v3);

		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateTriangleMeshShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		DWORD flags;
		if (!ReadLuaTableDWORD(params, "flags", flags))
			return nullptr;

		std::vector<PARAVECTOR3> vecPoints;
		if (!ReadLuaTableVector3Array(params, "points", vecPoints))
			return nullptr;
	

		std::vector<WORD> vecIndices;
		if (!ReadLuaTableWordArray(params, "triangles", vecIndices))
			return nullptr;

		if (vecIndices.size() % 3 != 0)
			return nullptr;

		ParaPhysicsTriangleMeshDesc trimeshDesc;
		trimeshDesc.m_numVertices = vecPoints.size();
		trimeshDesc.m_numTriangles = vecIndices.size() / 3;
		trimeshDesc.m_pointStrideBytes = sizeof(PARAVECTOR3);
		trimeshDesc.m_triangleStrideBytes = 3 * sizeof(int16);
		trimeshDesc.m_points = &vecPoints[0];
		trimeshDesc.m_triangles = &vecIndices[0];
		trimeshDesc.m_flags = flags;

		auto obj = m_pObj->CreateTriangleMeshShape(trimeshDesc);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateStaticPlaneShape(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		PARAVECTOR3 v3;
		if (!ReadLuaTableVector3(params, "planeNormal", v3))
			return nullptr;

		float planeConstant;
		if (!ReadLuaTableFloat(params, "planeConstant", planeConstant))
			return nullptr;

		auto obj = m_pObj->CreateStaticPlaneShape(v3, planeConstant);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCompoundShape(const object& enableDynamicAabbTree)
	{
		bool bEnableDynamicAabbTree;
		if (!ReadLuaBoolean(enableDynamicAabbTree, bEnableDynamicAabbTree))
			return nullptr;

		auto obj = m_pObj->CreateCompoundShape(bEnableDynamicAabbTree);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConvexHullShape(const object& params)
	{
		std::vector<PARAVECTOR3> positions;
		if (!ReadLuaVector3Array(params, positions) || positions.size() == 0)
			return nullptr;

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
			return nullptr;

		auto obj = m_pObj->CreateCylinderShapeY(v3);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCylinderShapeX(const object& halfExtents)
	{
		PARAVECTOR3 v3;
		if (!Object2Vector3(halfExtents, v3))
			return nullptr;

		auto obj = m_pObj->CreateCylinderShapeX(v3);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCylinderShapeZ(const object& halfExtents)
	{
		PARAVECTOR3 v3;
		if (!Object2Vector3(halfExtents, v3))
			return nullptr;

		auto obj = m_pObj->CreateCylinderShapeZ(v3);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateBoxShape(const object& boxHalfExtents)
	{
		PARAVECTOR3 v3;
		if (!Object2Vector3(boxHalfExtents, v3))
			return nullptr;

		auto obj = m_pObj->CreateBoxShape(v3);
		return new ParaAttributeObject(obj);
	}
	
	ParaAttributeObject* ParaPhysicsFactory::CreateSphereShape(const object& radius)
	{

		float fRadius;
		if (!ReadLuaFloat(radius, fRadius))
			return nullptr;

		auto obj = m_pObj->CreateSphereShape(fRadius);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCapsuleShapeY(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		ReadLuaTableFloat(params, "radius", fRadius);
		ReadLuaTableFloat(params, "height", fHeight);


		auto obj = m_pObj->CreateCapsuleShapeY(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCapsuleShapeX(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		ReadLuaTableFloat(params, "radius", fRadius);
		ReadLuaTableFloat(params, "height", fHeight);

		auto obj = m_pObj->CreateCapsuleShapeX(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateCapsuleShapeZ(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		ReadLuaTableFloat(params, "radius", fRadius);
		ReadLuaTableFloat(params, "height", fHeight);

		auto obj = m_pObj->CreateCapsuleShapeZ(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConeShapeY(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		ReadLuaTableFloat(params, "radius", fRadius);
		ReadLuaTableFloat(params, "height", fHeight);

		auto obj = m_pObj->CreateConeShapeY(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}


	ParaAttributeObject* ParaPhysicsFactory::CreateConeShapeX(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		ReadLuaTableFloat(params, "radius", fRadius);
		ReadLuaTableFloat(params, "height", fHeight);

		auto obj = m_pObj->CreateConeShapeX(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateConeShapeZ(const object& params)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		float fRadius = 0.f, fHeight = 0.f;

		ReadLuaTableFloat(params, "radius", fRadius);
		ReadLuaTableFloat(params, "height", fHeight);

		auto obj = m_pObj->CreateConeShapeZ(fRadius, fHeight);
		return new ParaAttributeObject(obj);
	}

	ParaAttributeObject* ParaPhysicsFactory::CreateRigidbody_(const object& params, ParaPhysicsMotionStateDesc* motionStateDesc)
	{
		if (type(params) != LUA_TTABLE)
			return nullptr;

		// check shape
		CPhysicsShape* pShape = nullptr;
		if (!ReadLuaTablePhysicsShape(params, "shape", pShape))
			return nullptr;

		float fMass = 0.0f;
		ReadLuaTableFloat(params, "mass", fMass);

		PARAVECTOR3 vOrigin;
		ReadLuaTableVector3(params, "origin", vOrigin);


		PARAVECTOR3 vInertia;
		bool bUseInertia = ReadLuaTableVector3(params, "inertia", vInertia);


		PARAMATRIX3x3 mRotation;
		ReadLuaTableMatrix3x3(params, "rotation", mRotation);


		short nGroup = 0;
		ReadLuaTableShort(params, "group", nGroup);
	
		int nMask = -1;
		ReadLuaTableInt(params, "mask", nMask);
	

		bool bUseCenterOfMassOffset = false;
		auto& centerOfMassOffset = params["centerOfMassOffset"];
		PARAVECTOR3 vCenterOfMassOffsetOrigin;
		PARAMATRIX3x3 mCenterOfMassOffsetRotation;
		if (type(centerOfMassOffset) == LUA_TTABLE)
		{
			bUseCenterOfMassOffset = true;
			bUseCenterOfMassOffset &= ReadLuaTableVector3(centerOfMassOffset, "origin", vCenterOfMassOffsetOrigin);

			if (bUseCenterOfMassOffset)
			{
				bUseCenterOfMassOffset &= ReadLuaTableMatrix3x3(centerOfMassOffset, "rotation", mCenterOfMassOffsetRotation);
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
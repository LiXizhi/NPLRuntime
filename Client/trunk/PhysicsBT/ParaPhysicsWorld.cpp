#include "stdafx.h"
#include "ParaPhysicsWorld.h"
#include "IParaDebugDraw.h"
#include "ParaPhysicsShape.hpp"
#include "ParaPhysicsBody.hpp"
#include "ParaPhysicsConstraint.hpp"

#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"

namespace ParaEngine {
	CParaPhysicsWorld::CParaPhysicsWorld()
		: m_bInvertFaceWinding(false)
	{
#ifdef WIN32
		m_bInvertFaceWinding = true;
#endif
	}

	CParaPhysicsWorld::~CParaPhysicsWorld()
	{

	}

	void CParaPhysicsWorld::Release()
	{
		delete this;
	}

	IParaPhysicsWorld* CParaPhysicsWorld::InitPhysics(ParaPhysicsWorldType ppwt)
	{
		auto pWorld = new BulletWorld();

		pWorld->m_collisionConfiguration = new btDefaultCollisionConfiguration();
		pWorld->m_dispatcher = new	btCollisionDispatcher(pWorld->m_collisionConfiguration);
		pWorld->m_broadphase = new btDbvtBroadphase();
		pWorld->m_solver = new btSequentialImpulseConstraintSolver();

		btDiscreteDynamicsWorld* dynamicsWorld;

		switch (ppwt)
		{
		case ParaEngine::PPWT_Dynamics:
			dynamicsWorld = new btDiscreteDynamicsWorld(pWorld->m_dispatcher, pWorld->m_broadphase, pWorld->m_solver, pWorld->m_collisionConfiguration);
			break;
		case ParaEngine::PPWT_SoftRigidDynamics:
			dynamicsWorld = new btSoftRigidDynamicsWorld(pWorld->m_dispatcher, pWorld->m_broadphase, pWorld->m_solver, pWorld->m_collisionConfiguration);
			break;
		default:
			dynamicsWorld = new btDiscreteDynamicsWorld(pWorld->m_dispatcher, pWorld->m_broadphase, pWorld->m_solver, pWorld->m_collisionConfiguration);
			break;
		}

		if (dynamicsWorld)
		{
			m_physics_debug_draw.setDebugMode(PhysicsDebugDrawModes::PDDM_MAX_DEBUG_DRAW_MODE);
			dynamicsWorld->setDebugDrawer(&m_physics_debug_draw);
		}
		
		pWorld->m_pWorld = dynamicsWorld;
		
		return pWorld;
	}


	void CParaPhysicsWorld::SetDebugDrawer(IParaDebugDraw* debugDrawer)
	{
		m_physics_debug_draw.SetParaDebugDrawInterface(debugDrawer);
	}

	IParaDebugDraw* CParaPhysicsWorld::GetDebugDrawer()
	{
		return m_physics_debug_draw.GetParaDebugDrawInterface();
	}

	void CParaPhysicsWorld::DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color)
	{

	}

	void CParaPhysicsWorld::SetDebugDrawMode(int debugMode)
	{
		m_physics_debug_draw.setDebugMode(debugMode);
	}

	int CParaPhysicsWorld::GetDebugDrawMode()
	{
		return m_physics_debug_draw.getDebugMode();
	}


	IParaPhysicsShape* CParaPhysicsWorld::CreateBoxShape(const PARAVECTOR3& boxHalfExtents)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btBoxShape(btVector3(btScalar(boxHalfExtents.x)
			, btScalar(boxHalfExtents.y)
			, btScalar(boxHalfExtents.z)));
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateSphereShape(float radius)
	{
		auto pShape = new BulletShape();

		pShape->m_pShape = new btSphereShape(btScalar(radius));
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}


	IParaPhysicsShape* CParaPhysicsWorld::CreateCapsuleShapeY(float radius, float height)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btCapsuleShape(btScalar(radius), btScalar(height));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateCapsuleShapeX(float radius, float height)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btCapsuleShapeX(btScalar(radius), btScalar(height));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateCapsuleShapeZ(float radius, float height)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btCapsuleShapeZ(btScalar(radius), btScalar(height));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateCylinderShapeY(const PARAVECTOR3& halfExtents)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btCylinderShape(btVector3(btScalar(halfExtents.x)
			, btScalar(halfExtents.y)
			, btScalar(halfExtents.z)));
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateCylinderShapeX(const PARAVECTOR3& halfExtents)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btCylinderShapeX(btVector3(btScalar(halfExtents.x)
			, btScalar(halfExtents.y)
			, btScalar(halfExtents.z)));
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateCylinderShapeZ(const PARAVECTOR3& halfExtents)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btCylinderShapeZ(btVector3(btScalar(halfExtents.x)
			, btScalar(halfExtents.y)
			, btScalar(halfExtents.z)));
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateConeShapeY(float radius, float height)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btConeShape(btScalar(radius), btScalar(height));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateConeShapeX(float radius, float height)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btConeShapeX(btScalar(radius), btScalar(height));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateConeShapeZ(float radius, float height)
	{
		auto pShape = new BulletShape();
		pShape->m_pShape = new btConeShapeZ(btScalar(radius), btScalar(height));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}


	IParaPhysicsShape* CParaPhysicsWorld::CreateMultiSphereShape(const PARAVECTOR3* positions, const float* radi, int numSpheres)
	{
		assert(positions && radi && numSpheres > 0);

		btVector3* pPositions = nullptr;
		btScalar* pRadi = nullptr;

		auto pShape = new BulletShape();

		if (sizeof(btScalar) == sizeof(float))
		{
			pRadi = (btScalar*)radi;
			pPositions = new btVector3[numSpheres];

			for (int i = 0; i < numSpheres; i++)
			{
				auto& dstPos = pPositions[i];
				auto& srcPos = positions[i];
				dstPos = btVector3(btScalar(srcPos.x), btScalar(srcPos.y), btScalar(srcPos.z));
			}

			pShape->m_pShape = new btMultiSphereShape(pPositions, pRadi, numSpheres);

			delete[] pPositions;
		}
		else
		{
			pRadi = new btScalar[numSpheres];
			pPositions = new btVector3[numSpheres];

			for (int i = 0; i < numSpheres; i++)
			{
				auto& dstPos = pPositions[i];
				auto& srcPos = positions[i];
				dstPos = btVector3(btScalar(srcPos.x), btScalar(srcPos.y), btScalar(srcPos.z));

				pRadi[i] = btScalar(radi[i]);
			}

			delete[] pPositions;
			delete[] pRadi;
		}

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}


	IParaPhysicsCompoundShape* CParaPhysicsWorld::CreateCompoundShape(bool enableDynamicAabbTree)
	{
		auto pShape = new BulletCompoundShape();
		pShape->m_pShape = new btCompoundShape(enableDynamicAabbTree);

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateConvexHullShape(const PARAVECTOR3* points, int numPoints)
	{
		auto pShape = new BulletShape();

		if (!points || numPoints == 0)
		{
			pShape->m_pShape = new btConvexHullShape();
		}
		else
		{
			if (sizeof(btScalar) == sizeof(float))
			{
				const int stride = sizeof(PARAVECTOR3);
				pShape->m_pShape = new btConvexHullShape((btScalar*)points, numPoints, stride);
			}
			else
			{
				const int stride = sizeof(btVector3);
				auto btPoints = new btVector3[numPoints];
				for (int i = 0; i < numPoints; i++)
				{
					btPoints[i].setX(btScalar(points[i].x));
					btPoints[i].setY(btScalar(points[i].y));
					btPoints[i].setZ(btScalar(points[i].z));
				}

				pShape->m_pShape = new btConvexHullShape((btScalar*)btPoints, numPoints, stride);

				delete[] btPoints;
			}
		}

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsShape* CParaPhysicsWorld::CreateStaticPlaneShape(const PARAVECTOR3& planeNormal, float planeConstant)
	{
		auto pShape = new BulletShape();

		pShape->m_pShape = new btStaticPlaneShape(btVector3(btScalar(planeNormal.x), btScalar(planeNormal.y), btScalar(planeNormal.z))
			, btScalar(planeConstant));

		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsRigidbody* CParaPhysicsWorld::CreateRigidbody(const ParaPhysicsRigidbodyDesc& desc, ParaPhysicsMotionStateDesc* motionStateDesc)
	{
		//rigid body is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (desc.m_mass != 0.f);

		btCollisionShape* shape = static_cast<btCollisionShape*>(desc.m_pShape->get());
		assert(shape != nullptr);

		btVector3 localInertia(0.f, 0.f, 0.f);
		if (isDynamic)
		{
			if (desc.m_pInertia)
			{
				const auto& inertia = *desc.m_pInertia;
				localInertia = btVector3(btScalar(inertia.x), btScalar(inertia.y), btScalar(inertia.z));
			}
			else
			{
				shape->calculateLocalInertia(desc.m_mass, localInertia);
			}
		}

		btMatrix3x3 matRot(
			btScalar(desc.m_rotation._11), btScalar(desc.m_rotation._21), btScalar(desc.m_rotation._31),
			btScalar(desc.m_rotation._12), btScalar(desc.m_rotation._22), btScalar(desc.m_rotation._32),
			btScalar(desc.m_rotation._13), btScalar(desc.m_rotation._23), btScalar(desc.m_rotation._33)
		);
		btVector3 vOrigin(btScalar(desc.m_origin.x), btScalar(desc.m_origin.y), btScalar(desc.m_origin.z));
		btTransform startTransform(matRot, vOrigin);

		btTransform centerOfMassOffset;
		if (desc.m_pCenterOfMassOffset)
		{
			const auto& offset = *desc.m_pCenterOfMassOffset;
			matRot = btMatrix3x3(
				btScalar(offset.m_offset_rotation._11), btScalar(offset.m_offset_rotation._21), btScalar(offset.m_offset_rotation._31),
				btScalar(offset.m_offset_rotation._12), btScalar(offset.m_offset_rotation._22), btScalar(offset.m_offset_rotation._32),
				btScalar(offset.m_offset_rotation._13), btScalar(offset.m_offset_rotation._23), btScalar(offset.m_offset_rotation._33)
			);

			vOrigin = btVector3(btScalar(offset.m_offset_origin.x), btScalar(offset.m_offset_origin.y), btScalar(offset.m_offset_origin.z));
			centerOfMassOffset = btTransform(matRot, vOrigin);
		}
		else
		{
			centerOfMassOffset = btTransform::getIdentity();
		}

		btRigidBody* body = nullptr;
		if (motionStateDesc)
		{
			auto pMotionState = new ParaPhysicsMotionState(startTransform, centerOfMassOffset, motionStateDesc->cb);

			motionStateDesc->setTransform = [pMotionState](const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin)
			{
				pMotionState->setWorldTransform(rotation, origin);
			};

			body = new btRigidBody(desc.m_mass, pMotionState, shape, localInertia);
		}
		else
		{
			body = new btRigidBody(desc.m_mass, nullptr, shape, localInertia);
			body->setWorldTransform(startTransform);

			if (desc.m_pCenterOfMassOffset)
				body->setCenterOfMassTransform(centerOfMassOffset);
		}

		BulletRigidbody* pBody = new BulletRigidbody();
		pBody->m_pBody = body;
		body->setUserPointer(pBody);

		return pBody;
	}

	IParaPhysicsTriangleMeshShape* CParaPhysicsWorld::CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc)
	{
		BulletTriangleMeshShape* pShape = new BulletTriangleMeshShape();

		pShape->m_triangleIndices = new int32[12 * meshDesc.m_numTriangles];
		if (meshDesc.m_triangleStrideBytes == 12)
		{
			// 32 bits index
			int32* dest = pShape->m_triangleIndices;
			int32* src = (int32*)(meshDesc.m_triangles);
			if (m_bInvertFaceWinding)
			{
				int nFaceCount = meshDesc.m_numTriangles;
				for (int i = 0; i < nFaceCount; ++i)
				{
					// change the triangle winding order
					*dest = *src; ++src;
					*(dest + 2) = *src; ++src;
					*(dest + 1) = *src; ++src;
					dest += 3;
				}
			}
			else
			{
				memcpy(dest, src, sizeof(int32) * meshDesc.m_numTriangles * 3);
			}
		}
		else
		{
			// 16 bits index
			int32* dest = pShape->m_triangleIndices;
			int16* src = (int16*)(meshDesc.m_triangles);
			if (m_bInvertFaceWinding)
			{
				int nFaceCount = meshDesc.m_numTriangles;
				for (int i = 0; i < nFaceCount; ++i)
				{
					*dest = *src; ++src;
					*(dest + 2) = *src; ++src;
					*(dest + 1) = *src; ++src;
					dest += 3;
				}
			}
			else
			{
				int nFaceCount = meshDesc.m_numTriangles;
				for (int i = 0; i < nFaceCount; ++i)
				{
					*dest = *src; ++src;
					*(dest + 1) = *src; ++src;
					*(dest + 2) = *src; ++src;
					dest += 3;
				}
			}
		}

		pShape->m_vertices = new btScalar[meshDesc.m_pointStrideBytes * meshDesc.m_numVertices / sizeof(btScalar)];
		memcpy(pShape->m_vertices, meshDesc.m_points, meshDesc.m_pointStrideBytes * meshDesc.m_numVertices);

		pShape->m_indexVertexArrays = new btTriangleIndexVertexArray(meshDesc.m_numTriangles,
			(int*)(pShape->m_triangleIndices),
			12,
			meshDesc.m_numVertices, pShape->m_vertices, meshDesc.m_pointStrideBytes);

		bool useQuantizedAabbCompression = true;

		pShape->m_pShape = new btBvhTriangleMeshShape(pShape->m_indexVertexArrays, useQuantizedAabbCompression);
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsScalingTriangleMeshShape* CParaPhysicsWorld::CreateScaledTriangleMeshShape(IParaPhysicsTriangleMeshShape* pTriangleMeshShape, const PARAVECTOR3& localScaling)
	{
		auto pbtTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(pTriangleMeshShape->get());

		if (!pbtTriangleMeshShape)
			return nullptr;

		BulletScalingTriangleMeshShape* pShape = new BulletScalingTriangleMeshShape();

		btVector3 scaling = CONVERT_BTVECTOR3(localScaling);

		pShape->m_pShape = new btScaledBvhTriangleMeshShape(pbtTriangleMeshShape, scaling);
		pShape->m_pShape->setUserPointer(pShape);

		return pShape;
	}

	IParaPhysicsPoint2PointConstraint* CParaPhysicsWorld::CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA, const PARAVECTOR3& pivotInA)
	{
		auto pConstraint = new BulletPoint2PointConstraint();

		pConstraint->m_pConstraint = new btPoint2PointConstraint(*static_cast<btRigidBody*>(rbA->get())
			, CONVERT_BTVECTOR3(pivotInA));

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsPoint2PointConstraint* CParaPhysicsWorld::CreatePoint2PointConstraint(IParaPhysicsRigidbody* rbA
		, IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& pivotInA
		, const PARAVECTOR3& pivotInB)
	{
		auto pConstraint = new BulletPoint2PointConstraint();

		pConstraint->m_pConstraint = new btPoint2PointConstraint(*static_cast<btRigidBody*>(rbA->get())
			, *static_cast<btRigidBody*>(rbB->get())
			, CONVERT_BTVECTOR3(pivotInA)
			, CONVERT_BTVECTOR3(pivotInB));

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsHingeConstraint* CParaPhysicsWorld::CreateHingeConstraint(IParaPhysicsRigidbody* rbA
		, IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& pivotInA
		, const PARAVECTOR3& pivotInB
		, const PARAVECTOR3& axisInA
		, const PARAVECTOR3& axisInB
		, bool useReferenceFrameA)
	{
		auto pConstraint = new BulletHingeConstraint();

		pConstraint->m_pConstraint = new btHingeConstraint(*static_cast<btRigidBody*>(rbA->get())
			, *static_cast<btRigidBody*>(rbB->get())
			, CONVERT_BTVECTOR3(pivotInA)
			, CONVERT_BTVECTOR3(pivotInB)
			, CONVERT_BTVECTOR3(axisInA)
			, CONVERT_BTVECTOR3(axisInB)
			, useReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsHingeConstraint* CParaPhysicsWorld::CreateHingeConstraint(IParaPhysicsRigidbody* rbA
		, const PARAVECTOR3& pivotInA
		, const PARAVECTOR3& axisInA
		, bool useReferenceFrameA)
	{
		auto pConstraint = new BulletHingeConstraint();

		pConstraint->m_pConstraint = new btHingeConstraint(*static_cast<btRigidBody*>(rbA->get())
			, CONVERT_BTVECTOR3(pivotInA)
			, CONVERT_BTVECTOR3(axisInA)
			, useReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsHingeConstraint* CParaPhysicsWorld::CreateHingeConstraint(IParaPhysicsRigidbody* rbA
		, IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useReferenceFrameA)
	{
		auto pConstraint = new BulletHingeConstraint();

		pConstraint->m_pConstraint = new btHingeConstraint(*static_cast<btRigidBody*>(rbA->get())
			, *static_cast<btRigidBody*>(rbB->get())
			, CONVERT_TO_TRANSFORM(rbARotation, rbAOrigin)
			, CONVERT_TO_TRANSFORM(rbBRotation, rbBOrigin)
			, useReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsHingeConstraint* CParaPhysicsWorld::CreateHingeConstraint(IParaPhysicsRigidbody* rbA
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, bool useReferenceFrameA)
	{
		auto pConstraint = new BulletHingeConstraint();

		pConstraint->m_pConstraint = new btHingeConstraint(*static_cast<btRigidBody*>(rbA->get())
			, CONVERT_TO_TRANSFORM(rbARotation, rbAOrigin)
			, useReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsSliderConstraint* CParaPhysicsWorld::CreateSliderConstraint(IParaPhysicsRigidbody* rbA
		, IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameA)
	{
		auto pConstraint = new BulletSliderConstraint();

		pConstraint->m_pConstraint = new btSliderConstraint(*static_cast<btRigidBody*>(rbA->get())
			, *static_cast<btRigidBody*>(rbB->get())
			, CONVERT_TO_TRANSFORM(rbARotation, rbAOrigin)
			, CONVERT_TO_TRANSFORM(rbBRotation, rbBOrigin)
			, useLinearReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsSliderConstraint* CParaPhysicsWorld::CreateSliderConstraint(IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameA)
	{
		auto pConstraint = new BulletSliderConstraint();

		pConstraint->m_pConstraint = new btSliderConstraint(*static_cast<btRigidBody*>(rbB->get())
			, CONVERT_TO_TRANSFORM(rbBRotation, rbBOrigin)
			, useLinearReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsConeTwistConstraint* CParaPhysicsWorld::CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation)
	{
		auto pConstraint = new BulletConeTwistConstraint();

		pConstraint->m_pConstraint = new btConeTwistConstraint(*static_cast<btRigidBody*>(rbA->get())
			, CONVERT_TO_TRANSFORM(rbARotation, rbAOrigin));

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsConeTwistConstraint* CParaPhysicsWorld::CreateConeTwistConstraint(IParaPhysicsRigidbody* rbA
		, IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation)
	{
		auto pConstraint = new BulletConeTwistConstraint();

		pConstraint->m_pConstraint = new btConeTwistConstraint(*static_cast<btRigidBody*>(rbA->get())
			, *static_cast<btRigidBody*>(rbB->get())
			, CONVERT_TO_TRANSFORM(rbARotation, rbAOrigin)
			, CONVERT_TO_TRANSFORM(rbBRotation, rbBOrigin));

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsGeneric6DofSpringConstraint* CParaPhysicsWorld::CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbA
		, IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameA)
	{
		auto pConstraint = new BulletGeneric6DofSpringConstraint();

		pConstraint->m_pConstraint = new btGeneric6DofSpringConstraint(*static_cast<btRigidBody*>(rbA->get())
			, *static_cast<btRigidBody*>(rbB->get())
			, CONVERT_TO_TRANSFORM(rbARotation, rbAOrigin)
			, CONVERT_TO_TRANSFORM(rbBRotation, rbBOrigin)
			, useLinearReferenceFrameA);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}

	IParaPhysicsGeneric6DofSpringConstraint* CParaPhysicsWorld::CreateGeneric6DofSpringConstraint(IParaPhysicsRigidbody* rbB
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameB)
	{
		auto pConstraint = new BulletGeneric6DofSpringConstraint();
		pConstraint->m_pConstraint = new btGeneric6DofSpringConstraint(*static_cast<btRigidBody*>(rbB->get())
			, CONVERT_TO_TRANSFORM(rbBRotation, rbBOrigin)
			, useLinearReferenceFrameB);

		pConstraint->m_pConstraint->setUserConstraintPtr(pConstraint);

		return pConstraint;
	}
}
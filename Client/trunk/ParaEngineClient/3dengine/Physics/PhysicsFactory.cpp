//-----------------------------------------------------------------------------
// Class:	CPhysicsFactory
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.7 revised 2010.2.25(PhysX is replaced by bullet)
// Note: serialization functions for ParaX model.
//-----------------------------------------------------------------------------
/**
current PhysX version is 2.8.1
Ageia now uses a free licence for commercial use of its physics engine since version 2.6.2 in Nov,2006. Below are SDK installation guide and application release notes.
- copy [AgeiaDir]/SDKs/XXX.* to [paraengine]/Physics/ : One can skip ./Docs directory.
- copy [AgeiaDir]/Bin/Win32/PhysXLoader.dll to [paraengine]/
- Some helper cooking files at [paraengine]/Physics/XXX.cpp(h)
- [NX_SUPPORT_MESH_SCALE is not working. This step is skipped] in Nxp.h at the ageia include folder, uncomment a line to define NX_SUPPORT_MESH_SCALE
Release:
- During End User installation. Run the [AgeiaDir]/Redistributable/ in slient mode to install all necessary DLLs to the user's computer.
since 2.7.0, it has a much smaller installer which can be downloaded at SDK site.
- Add (NVidia)Ageia Copyright Text to about of ParaEngine.
- Send a copy of the commercial product to Ageia.
*/

/**
Currently ParaEngine only supports Ageia/Novodex Physics Engine 2.X ( free license). ODE support is in plan.
Below are the matching concept between these two simulation system.
Novodex				<-> ODE
nxScene				<-> quad-tree based space: space = dQuadTreeSpaceCreate();...;  dSpaceDestroy(space);
TriangleMeshShape	<-> dTriMeshDataID Data = dGeomTriMeshDataCreate();
dGeomTriMeshDataBuildSimple(Data, (dReal*)Vertices, VertexCount, Indices, IndexCount);
// another difference is that in ODE, the user needs to keep the memory of Vertices valid; whereas in Novodex it is not.
static actor		<-> TriMesh = dCreateTriMesh(space, Data, 0, 0, 0); // triangle mesh geometry in a space is similar to static actor in a novodex scene
dGeomSetPosition(TriMesh, 0, 0, 1.0); // one can set position and orientation in it.
ray					<-> Ray = dCreateRay(space, 0.9);
ray collision		<-> dSpaceCollide2(...);
*/
#include "ParaEngine.h"

#include "PluginManager.h"
#include "PluginAPI.h"

#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "Physics/PhysicsFactory.h"
#include "Physics/PhysicsDynamicsWorld.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsShape.h"
#include "ParaPhysics.h"

#include "IParaEngineApp.h"
#include "memdebug.h"

namespace ParaEngine {
	/** the bullet physics engine plugin dll file path */
#ifdef _DEBUG
	const char* PHYSICS_DLL_FILE_PATH = "PhysicsBT_d.dll";
#else
	const char* PHYSICS_DLL_FILE_PATH = "PhysicsBT.dll";
#endif

	CPhysicsFactory::CPhysicsFactory()
		: m_bRunDynamicSimulation(false)
		, m_pPhysicsInstance(nullptr)
	{
	}

	CPhysicsFactory::~CPhysicsFactory(void)
	{
		SAFE_RELEASE(m_pPhysicsInstance);
	}

	void CPhysicsFactory::InitPhysics()
	{
		IParaPhysics* pPhysics = GetPhysicsInterface();
		if (pPhysics == nullptr)
		{
			OUTPUT_LOG("error: failed loading physics lib at %s \n", PHYSICS_DLL_FILE_PATH);
			return;
		}

		auto pWorldInterface = pPhysics->InitPhysics();
		if (!pWorldInterface)
			return;

		m_pWorld = new CPhysicsDynamicsWorld(pWorldInterface);
		m_pWorld->AddToAutoReleasePool();

		pPhysics->SetDebugDrawer(CGlobals::GetScene()->GetDebugDrawer());
	}

	void CPhysicsFactory::ResetPhysics()
	{
		ExitPhysics();
		InitPhysics();
	}

	void CPhysicsFactory::ExitPhysics()
	{
		m_pWorld.reset();
		m_meshShapesCache.clear();
	}

	void CPhysicsFactory::StepSimulation(double dTime)
	{
		if (IsDynamicsSimulationEnabled())
		{
			if (m_pWorld)
			{
				PERF1("Dynamic Physics");
				m_pWorld->StepSimulation((float)dTime);
				onAfterCheckContact();
			}
		}
	}

	void CPhysicsFactory::onAfterCheckContact()
	{
	}

	void CPhysicsFactory::SetDynamicsSimulationEnabled(bool bEnable)
	{
		m_bRunDynamicSimulation = bEnable;
	}

	bool CPhysicsFactory::IsDynamicsSimulationEnabled() const
	{
		return m_bRunDynamicSimulation;
	}

	// iOS does not support dynamically loaded dll, hence we will use statically linked plugin. 
#if defined(STATIC_PLUGIN_PHYSICS_BT)
	extern ClassDescriptor* PhysicsBT_GetClassDesc();
#endif

	IParaPhysics* CPhysicsFactory::GetPhysicsInterface()
	{
		if (m_pPhysicsInstance)
			return m_pPhysicsInstance;

#ifdef STATIC_PLUGIN_PHYSICS_BT
		ClassDescriptor* pClassDesc = PhysicsBT_GetClassDesc();
		if (pClassDesc && (strcmp(pClassDesc->ClassName(), "IParaPhysics") == 0))
		{
			m_pPhysicsWorld = (IParaPhysics*)pClassDesc->Create();
			OUTPUT_LOG("physics BT engine loaded statically\n");
		}
#else
		DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(PHYSICS_DLL_FILE_PATH);
		if (pPluginEntity == 0)
		{
			// load the plug-in if it has never been loaded before. 
			pPluginEntity = CGlobals::GetPluginManager()->LoadDLL("", PHYSICS_DLL_FILE_PATH);
		}

		if (pPluginEntity != 0)
		{
			for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
			{
				ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

				if (pClassDesc && (strcmp(pClassDesc->ClassName(), "IParaPhysics") == 0))
				{
					m_pPhysicsInstance = (IParaPhysics*)pClassDesc->Create();
				}
			}
		}
#endif
		if (m_pPhysicsInstance == nullptr)
		{
			OUTPUT_LOG("note: no physics engine found, null physics is used \n");
			m_pPhysicsInstance = new CParaPhysicsImp();
		}
		return m_pPhysicsInstance;
	}


	CPhysicsShape* CPhysicsFactory::CreateBoxShape(const PARAVECTOR3& boxHalfExtents)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateBoxShape(boxHalfExtents);
		if (!pShapeObj)
			return nullptr;

		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateSphereShape(float radius)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateSphereShape(radius);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateCapsuleShapeY(float radius, float height)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCapsuleShapeY(radius, height);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateCapsuleShapeX(float radius, float height)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCapsuleShapeX(radius, height);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateCapsuleShapeZ(float radius, float height)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCapsuleShapeZ(radius, height);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateCylinderShapeY(const PARAVECTOR3& halfExtents)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCylinderShapeY(halfExtents);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateCylinderShapeX(const PARAVECTOR3& halfExtents)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCylinderShapeX(halfExtents);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateCylinderShapeZ(const PARAVECTOR3& halfExtents)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCylinderShapeZ(halfExtents);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateConeShapeY(float radius, float height)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateConeShapeY(radius, height);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateConeShapeX(float radius, float height)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateConeShapeX(radius, height);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateConeShapeZ(float radius, float height)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateConeShapeZ(radius, height);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateConvexHullShape(const PARAVECTOR3* points, int numPoints)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateConvexHullShape(points, numPoints);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateMultiSphereShape(const PARAVECTOR3* positions, const float* radi, int numSpheres)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateMultiSphereShape(positions, radi, numSpheres);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsCompoundShape* CPhysicsFactory::CreateCompoundShape(bool enableDynamicAabbTree)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateCompoundShape(enableDynamicAabbTree);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsCompoundShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsShape* CPhysicsFactory::CreateStaticPlaneShape(const PARAVECTOR3& planeNormal, float planeConstant)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateStaticPlaneShape(planeNormal, planeConstant);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsTriangleMeshShape* CPhysicsFactory::CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateTriangleMeshShape(meshDesc);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsTriangleMeshShape(pShapeObj);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsScaledTriangleMeshShape* CPhysicsFactory::CreateScaledTriangleMeshShape(CPhysicsTriangleMeshShape* pTriangleMeshShape, const PARAVECTOR3& localScaling)
	{
		auto pShapeObj = m_pPhysicsInstance->CreateScaledTriangleMeshShape(static_cast<IParaPhysicsTriangleMeshShape*>(pTriangleMeshShape->get()), localScaling);
		if (!pShapeObj)
			return nullptr;
		auto pShape = new CPhysicsScaledTriangleMeshShape(pShapeObj, pTriangleMeshShape);
		pShape->AddToAutoReleasePool();

		return pShape;
	}

	CPhysicsRigidBody* CPhysicsFactory::CreateRigidbody(const ParaPhysicsRigidbodyDesc& desc, ParaPhysicsMotionStateDesc* motionStateDesc)
	{
		CPhysicsShape* pShape = nullptr;

		if (desc.m_pShape)
		{
			pShape = static_cast<CPhysicsShape*>(desc.m_pShape->GetUserData());
		}

		if (!pShape)
			return nullptr;

		auto pBodyObj = m_pPhysicsInstance->CreateRigidbody(desc, motionStateDesc);
		if (!pBodyObj)
			return nullptr;

		auto pBody = new CPhysicsRigidBody(pBodyObj, pShape);
		pBody->AddToAutoReleasePool();

		return pBody;
	}

	CPhysicsDynamicsWorld* CPhysicsFactory::GetCurrentWorld()
	{
		return m_pWorld.get();
	}

	CPhysicsFactory::TriangleMeshShape* CPhysicsFactory::GetShapsInCache(void* p)
	{
		auto mapIt = m_meshShapesCache.find(p);
		if (mapIt == m_meshShapesCache.end())
			return nullptr;

		auto& shapes = mapIt->second.m_pShapes;
		for (auto vecIt = shapes.begin(); vecIt != shapes.end(); vecIt++)
		{
			if (vecIt->m_pShape.get() == nullptr)
			{
				// this shape has been release, so we remove current cache
				m_meshShapesCache.erase(mapIt);
				return nullptr;
			}
		}

		return &mapIt->second;
	}

	CPhysicsFactory::TriangleMeshShape& CPhysicsFactory::BuildCache(void* p)
	{
		m_meshShapesCache.insert(std::pair<void*, TriangleMeshShape>(p, TriangleMeshShape()));
		return m_meshShapesCache[p];
	}

	CPhysicsRigidBody* CPhysicsFactory::CreateStaticMesh(MeshEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup, vector<CPhysicsRigidBody::WeakPtr>* pOutputPhysicsActor, void* pUserData)
	{
		if (!ppMesh->IsValid() || ppMesh->GetMesh() == nullptr || m_pWorld == 0)
			return nullptr;
	
		/** Get the scaling factor from globalMat.
		* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
		* it is assumed that components of globalMat satisfies the following equation:
		* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
		* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
		* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
		*/

		float fScalingX, fScalingY, fScalingZ;
		Math::GetMatrixScaling(globalMat, &fScalingX, &fScalingY, &fScalingZ);
		bool bScaling = Vector3(fScalingX - 1.0f, fScalingY - 1.0f, fScalingZ - 1.0f).squaredLength() > FLT_TOLERANCE;
		CPhysicsRigidBody* pFirstBody = nullptr;

		int nMeshGroup = -1;

		auto cache = GetShapsInCache(ppMesh);

		if (cache)
		{
			auto& shapes = cache->m_pShapes;
			for (auto it = shapes.begin(); it != shapes.end(); it++)
			{
				auto shape = it->m_pShape.get();
				nMeshGroup = it->m_nPhysicsGroup;

				if (bScaling)
				{
					shape = CreateScaledTriangleMeshShape(static_cast<CPhysicsTriangleMeshShape*>(shape), PARAVECTOR3(fScalingX, fScalingY, fScalingZ));
				}

				// Generate report
				if (CGlobals::WillGenReport())
				{
					CGlobals::GetReport()->SetValue("physics counts", CGlobals::GetReport()->GetValue("physics counts") + 1);
				}

				ParaPhysicsRigidbodyDesc bodyDesc;
				bodyDesc.m_group = (int16)((nMeshGroup == 0) ? (nShapeGroup) : nMeshGroup);
				bodyDesc.m_mask = -1;
				// this ensures a static object. 
				bodyDesc.m_mass = 0.f;
				// set global world position
				bodyDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42, globalMat._43);
				bodyDesc.m_pShape = shape->get();

				// remove the scaling factor from the rotation matrix
				for (int i = 0; i<3; ++i)
				{
					bodyDesc.m_rotation.m[0][i] = globalMat.m[0][i] / fScalingX;
					bodyDesc.m_rotation.m[1][i] = globalMat.m[1][i] / fScalingY;
					bodyDesc.m_rotation.m[2][i] = globalMat.m[2][i] / fScalingZ;
				}

				auto body = CreateRigidbody(bodyDesc);
				if (body)
				{
					m_pWorld->AddRigidBody(body);

					if (pUserData != nullptr)
					{
						body->SetUserData(pUserData);
					}

					if (pOutputPhysicsActor != nullptr)
					{
						pOutputPhysicsActor->push_back(CPhysicsRigidBody::WeakPtr(body));
					}

					if (pFirstBody == nullptr)
					{
						pFirstBody = body;
					}
				}
			}

			return pFirstBody;
		}

		
		// get physics mesh data to pSysMesh with only vertex position vector
		DWORD dwNumVx = 0;
		Vector3 * verts = nullptr;
		DWORD dwNumFaces = 0;
		WORD* indices = nullptr;
		

		auto mesh = ppMesh->GetMesh();
		bool bFirst = true;

		auto& triangleMeshShape = BuildCache(ppMesh);
		
		while ((nMeshGroup = mesh->GetNextPhysicsGroupID(nMeshGroup)) >= 0)
		{
			SAFE_DELETE_ARRAY(indices);

			HRESULT result;
			if (verts)
			{
				result = mesh->ClonePhysicsMesh(nullptr, nullptr, &dwNumFaces, &indices, &nMeshGroup);
			}
			else
			{
				result = mesh->ClonePhysicsMesh(&dwNumVx, &verts, &dwNumFaces, &indices, &nMeshGroup);
			}

			if (!SUCCEEDED(result) || dwNumFaces == 0)
			{
				if (bFirst)
				{
					SAFE_DELETE_ARRAY(verts);
					SAFE_DELETE_ARRAY(indices);
					// physics mesh can not be loaded either because of an internal error or the mesh does not contain any physics faces.
					return nullptr;
				}
				else
				{
					continue;
				}
			}

			if (bFirst)
			{
				bFirst = false;
			}

			// Create descriptor for triangle mesh
			ParaPhysicsTriangleMeshDesc trimeshDesc;
			trimeshDesc.m_numVertices = dwNumVx;
			trimeshDesc.m_numTriangles = dwNumFaces;
			trimeshDesc.m_pointStrideBytes = sizeof(PARAVECTOR3);
			trimeshDesc.m_triangleStrideBytes = 3 * sizeof(int16);
			trimeshDesc.m_points = verts;
			trimeshDesc.m_triangles = indices;
			trimeshDesc.m_flags = 0;

			CPhysicsShape* shape = CreateTriangleMeshShape(trimeshDesc);

			triangleMeshShape.m_pShapes.push_back(SubMeshPhysicsShape(shape, nMeshGroup));

			if (bScaling)
			{
				shape = CreateScaledTriangleMeshShape(static_cast<CPhysicsTriangleMeshShape*>(shape), PARAVECTOR3(fScalingX, fScalingY, fScalingZ));
			}

			// Generate report
			if (CGlobals::WillGenReport())
			{
				CGlobals::GetReport()->SetValue("physics counts", CGlobals::GetReport()->GetValue("physics counts") + 1);
			}

			ParaPhysicsRigidbodyDesc bodyDesc;
			bodyDesc.m_group = (int16)((nMeshGroup == 0) ? (nShapeGroup) : nMeshGroup);
			bodyDesc.m_mask = -1;
			// this ensures a static object. 
			bodyDesc.m_mass = 0.f;
			// set global world position
			bodyDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42, globalMat._43);
			bodyDesc.m_pShape = shape->get();

			// remove the scaling factor from the rotation matrix
			for (int i = 0; i<3; ++i)
			{
				bodyDesc.m_rotation.m[0][i] = globalMat.m[0][i] / fScalingX;
				bodyDesc.m_rotation.m[1][i] = globalMat.m[1][i] / fScalingY;
				bodyDesc.m_rotation.m[2][i] = globalMat.m[2][i] / fScalingZ;
			}

			auto body = CreateRigidbody(bodyDesc);
			if (body)
			{
				m_pWorld->AddRigidBody(body);

				if (pUserData != nullptr)
				{
					body->SetUserData(pUserData);
				}

				if (pOutputPhysicsActor != nullptr)
				{
					pOutputPhysicsActor->push_back(CPhysicsRigidBody::WeakPtr(body));
				}

				if (pFirstBody == nullptr)
				{
					pFirstBody = body;
				}
			}
		}

		SAFE_DELETE_ARRAY(verts);
		SAFE_DELETE_ARRAY(indices);

		return pFirstBody;
	}

	CPhysicsRigidBody* CPhysicsFactory::CreateBoundsBody(CBaseObject* pObject, const Matrix4& globalMat, uint32 nShapeGroup, vector<CPhysicsRigidBody::WeakPtr>* pOutputPhysicsActor, void* pUserData)
	{
		if (m_pWorld == 0 || pObject == nullptr)
			return nullptr;

		float fScalingX, fScalingY, fScalingZ;
		Math::GetMatrixScaling(globalMat, &fScalingX, &fScalingY, &fScalingZ);

		bool bScaling = Vector3(fScalingX - 1.0f, fScalingY - 1.0f, fScalingZ - 1.0f).squaredLength() > FLT_TOLERANCE;

		Vector3 pVecBounds[8];
		int nNumVertices;
		pObject->GetRenderVertices(pVecBounds, &nNumVertices);
		const PARAVECTOR3* bounds = ((PARAVECTOR3*)pVecBounds);

		auto pShape = CreateConvexHullShape(bounds, nNumVertices);

		ParaPhysicsRigidbodyDesc bodyDesc;
		bodyDesc.m_group = (int16)nShapeGroup;
		bodyDesc.m_mask = -1;
		// this ensures a static object. 
		bodyDesc.m_mass = 1.f;
		// set global world position
		bodyDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42, globalMat._43);
		bodyDesc.m_pShape = pShape->get();

		ParaPhysicsMotionStateDesc msDesc;
		msDesc.cb = [pObject](const PARAMATRIX3x3& rotation, const PARAVECTOR3& origin)
		{
			pObject->SetPosition(DVector3(origin.x, origin.y, origin.z));
		};

		// remove the scaling factor from the rotation matrix
		for (int i = 0; i<3; ++i)
		{
			bodyDesc.m_rotation.m[0][i] = globalMat.m[0][i] / fScalingX;
			bodyDesc.m_rotation.m[1][i] = globalMat.m[1][i] / fScalingY;
			bodyDesc.m_rotation.m[2][i] = globalMat.m[2][i] / fScalingZ;
		}

		auto body = CreateRigidbody(bodyDesc, &msDesc);

		if (body)
		{
			m_pWorld->AddRigidBody(body);

			if (pUserData != nullptr)
			{
				body->SetUserData(pUserData);
			}

			if (pOutputPhysicsActor != nullptr)
			{
				pOutputPhysicsActor->push_back(CPhysicsRigidBody::WeakPtr(body));
			}
		}

		return body;
	}

	CPhysicsRigidBody* CPhysicsFactory::CreateStaticMesh(ParaXEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup, vector<CPhysicsRigidBody::WeakPtr>* pOutputPhysicsActor, void* pUserData)
	{
		if (!ppMesh->IsValid() || ppMesh->GetModel() == nullptr || m_pWorld == 0)
			return nullptr;

		/** Get the scaling factor from globalMat.
		* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
		* it is assumed that components of globalMat satisfies the following equation:
		* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
		* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
		* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
		*/
		float fScalingX, fScalingY, fScalingZ;
		Math::GetMatrixScaling(globalMat, &fScalingX, &fScalingY, &fScalingZ);

		bool bScaling = Vector3(fScalingX - 1.0f, fScalingY - 1.0f, fScalingZ - 1.0f).squaredLength() > FLT_TOLERANCE;
		CPhysicsRigidBody* pFirstBody = nullptr;

		int nMeshGroup = -1;

		auto cache = GetShapsInCache(ppMesh);

		if (cache)
		{
			auto& shapes = cache->m_pShapes;
			for (auto it = shapes.begin(); it != shapes.end(); it++)
			{
				auto shape = it->m_pShape.get();
				nMeshGroup = it->m_nPhysicsGroup;

				if (bScaling)
				{
					shape = CreateScaledTriangleMeshShape(static_cast<CPhysicsTriangleMeshShape*>(shape), PARAVECTOR3(fScalingX, fScalingY, fScalingZ));
				}

				// Generate report
				if (CGlobals::WillGenReport())
				{
					CGlobals::GetReport()->SetValue("physics counts", CGlobals::GetReport()->GetValue("physics counts") + 1);
				}

				ParaPhysicsRigidbodyDesc bodyDesc;
				bodyDesc.m_group = (int16)((nMeshGroup == 0) ? (nShapeGroup) : nMeshGroup);
				bodyDesc.m_mask = -1;
				// this ensures a static object. 
				bodyDesc.m_mass = 0.f;
				// set global world position
				bodyDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42, globalMat._43);
				bodyDesc.m_pShape = shape->get();

				// remove the scaling factor from the rotation matrix
				for (int i = 0; i<3; ++i)
				{
					bodyDesc.m_rotation.m[0][i] = globalMat.m[0][i] / fScalingX;
					bodyDesc.m_rotation.m[1][i] = globalMat.m[1][i] / fScalingY;
					bodyDesc.m_rotation.m[2][i] = globalMat.m[2][i] / fScalingZ;
				}

				auto body = CreateRigidbody(bodyDesc);
				if (body)
				{
					m_pWorld->AddRigidBody(body);

					if (pUserData != nullptr)
					{
						body->SetUserData(pUserData);
					}

					if (pOutputPhysicsActor != nullptr)
					{
						pOutputPhysicsActor->push_back(CPhysicsRigidBody::WeakPtr(body));
					}

					if (pFirstBody == nullptr)
					{
						pFirstBody = body;
					}
				}
			}

			return pFirstBody;
		}

		// get physics mesh data to pSysMesh with only vertex position vector
		DWORD dwNumVx = 0;
		Vector3 * verts = nullptr;
		DWORD dwNumFaces = 0;
		DWORD* indices = nullptr;

		auto mesh = ppMesh->GetModel();
		bool bFirst = true;

		auto& triangleMeshShape = BuildCache(ppMesh);

		while ((nMeshGroup = mesh->GetNextPhysicsGroupID(nMeshGroup)) >= 0)
		{
			SAFE_DELETE_ARRAY(indices);

			HRESULT result;
			if (verts)
			{
				result = mesh->ClonePhysicsMesh(nullptr, nullptr, &dwNumFaces, &indices, &nMeshGroup);
			}
			else
			{
				result = mesh->ClonePhysicsMesh(&dwNumVx, &verts, &dwNumFaces, &indices, &nMeshGroup);
			}

			if (!SUCCEEDED(result) || dwNumFaces == 0)
			{
				if (bFirst)
				{
					SAFE_DELETE_ARRAY(verts);
					SAFE_DELETE_ARRAY(indices);
					// physics mesh can not be loaded either because of an internal error or the mesh does not contain any physics faces.
					return nullptr;
				}
				else
				{
					continue;
				}
			}

			if (bFirst)
			{
				bFirst = false;
			}

			// Create descriptor for triangle mesh
			ParaPhysicsTriangleMeshDesc trimeshDesc;
			trimeshDesc.m_numVertices = dwNumVx;
			trimeshDesc.m_numTriangles = dwNumFaces;
			trimeshDesc.m_pointStrideBytes = sizeof(PARAVECTOR3);
			trimeshDesc.m_triangleStrideBytes = 3 * sizeof(int32);
			trimeshDesc.m_points = verts;
			trimeshDesc.m_triangles = indices;
			trimeshDesc.m_flags = 0;

			CPhysicsShape* shape = CreateTriangleMeshShape(trimeshDesc);

			triangleMeshShape.m_pShapes.push_back(SubMeshPhysicsShape(shape, nMeshGroup));

			if (bScaling)
			{
				shape = CreateScaledTriangleMeshShape(static_cast<CPhysicsTriangleMeshShape*>(shape), PARAVECTOR3(fScalingX, fScalingY, fScalingZ));
			}


			// Generate report
			if (CGlobals::WillGenReport())
			{
				CGlobals::GetReport()->SetValue("physics counts", CGlobals::GetReport()->GetValue("physics counts") + 1);
			}

			ParaPhysicsRigidbodyDesc bodyDesc;
			bodyDesc.m_group = (int16)((nMeshGroup == 0) ? (nShapeGroup) : nMeshGroup);
			bodyDesc.m_mask = -1;
			// this ensures a static object. 
			bodyDesc.m_mass = 0.f;
			// set global world position
			bodyDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42, globalMat._43);
			bodyDesc.m_pShape = shape->get();

			// remove the scaling factor from the rotation matrix
			for (int i = 0; i<3; ++i)
			{
				bodyDesc.m_rotation.m[0][i] = globalMat.m[0][i] / fScalingX;
				bodyDesc.m_rotation.m[1][i] = globalMat.m[1][i] / fScalingY;
				bodyDesc.m_rotation.m[2][i] = globalMat.m[2][i] / fScalingZ;
			}

			auto body = CreateRigidbody(bodyDesc);
			if (body)
			{
				m_pWorld->AddRigidBody(body);

				if (pUserData != nullptr)
				{
					body->SetUserData(pUserData);
				}

				if (pOutputPhysicsActor != nullptr)
				{
					pOutputPhysicsActor->push_back(CPhysicsRigidBody::WeakPtr(body));
				}

				if (pFirstBody == nullptr)
				{
					pFirstBody = body;
				}
			}
		}

		SAFE_DELETE_ARRAY(verts);
		SAFE_DELETE_ARRAY(indices);

		return pFirstBody;
	}

	CPhysicsP2PConstraint* CPhysicsFactory::CreatePoint2PointConstraint(CPhysicsRigidBody* rbA, const PARAVECTOR3& pivotInA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pConstraintObj = m_pPhysicsInstance->CreatePoint2PointConstraint(pBodyObjA, pivotInA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsP2PConstraint(pConstraintObj, rbA, nullptr);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsP2PConstraint* CPhysicsFactory::CreatePoint2PointConstraint(CPhysicsRigidBody* rbA
		, CPhysicsRigidBody* rbB
		, const PARAVECTOR3& pivotInA
		, const PARAVECTOR3& pivotInB)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreatePoint2PointConstraint(pBodyObjA, pBodyObjB, pivotInA, pivotInB);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsP2PConstraint(pConstraintObj, rbA, rbB);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsHingeConstraint* CPhysicsFactory::CreateHingeConstraint(CPhysicsRigidBody* rbA
		, CPhysicsRigidBody* rbB
		, const PARAVECTOR3& pivotInA
		, const PARAVECTOR3& pivotInB
		, const PARAVECTOR3& axisInA
		, const PARAVECTOR3& axisInB
		, bool useReferenceFrameA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateHingeConstraint(pBodyObjA, pBodyObjB, pivotInA, pivotInB, axisInA, axisInB, useReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsHingeConstraint(pConstraintObj, rbA, rbB);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsHingeConstraint* CPhysicsFactory::CreateHingeConstraint(CPhysicsRigidBody* rbA
		, const PARAVECTOR3& pivotInA
		, const PARAVECTOR3& axisInA
		, bool useReferenceFrameA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateHingeConstraint(pBodyObjA, pivotInA, axisInA, useReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsHingeConstraint(pConstraintObj, rbA, nullptr);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsHingeConstraint* CPhysicsFactory::CreateHingeConstraint(CPhysicsRigidBody* rbA
		, CPhysicsRigidBody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useReferenceFrameA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateHingeConstraint(pBodyObjA, pBodyObjB, rbAOrigin, rbARotation, rbBOrigin, rbBRotation, useReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsHingeConstraint(pConstraintObj, rbA, rbB);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsHingeConstraint* CPhysicsFactory::CreateHingeConstraint(CPhysicsRigidBody* rbA
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, bool useReferenceFrameA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateHingeConstraint(pBodyObjA, rbAOrigin, rbARotation, useReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsHingeConstraint(pConstraintObj, rbA, nullptr);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsSliderConstraint* CPhysicsFactory::CreateSliderConstraint(CPhysicsRigidBody* rbA
		, CPhysicsRigidBody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateSliderConstraint(pBodyObjA, pBodyObjB, rbAOrigin, rbARotation, rbBOrigin, rbBRotation, useLinearReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsSliderConstraint(pConstraintObj, rbA, rbB);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsSliderConstraint* CPhysicsFactory::CreateSliderConstraint(CPhysicsRigidBody* rbB
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameA)
	{
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateSliderConstraint(pBodyObjB, rbBOrigin, rbBRotation, useLinearReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsSliderConstraint(pConstraintObj, rbB, nullptr);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsConeTwistConstraint* CPhysicsFactory::CreateConeTwistConstraint(CPhysicsRigidBody* rbA
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateConeTwistConstraint(pBodyObjA, rbAOrigin, rbARotation);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsConeTwistConstraint(pConstraintObj, rbA, nullptr);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsConeTwistConstraint* CPhysicsFactory::CreateConeTwistConstraint(CPhysicsRigidBody* rbA
		, CPhysicsRigidBody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateConeTwistConstraint(pBodyObjA, pBodyObjB, rbAOrigin, rbARotation, rbBOrigin, rbBRotation);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsConeTwistConstraint(pConstraintObj, rbA, rbB);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsGeneric6DofSpringConstraint* CPhysicsFactory::CreateGeneric6DofSpringConstraint(CPhysicsRigidBody* rbA
		, CPhysicsRigidBody* rbB
		, const PARAVECTOR3& rbAOrigin
		, const PARAMATRIX3x3& rbARotation
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameA)
	{
		auto pBodyObjA = static_cast<IParaPhysicsRigidbody*>(rbA->get());
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateGeneric6DofSpringConstraint(pBodyObjA, pBodyObjB, rbAOrigin, rbARotation, rbBOrigin, rbBRotation, useLinearReferenceFrameA);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsGeneric6DofSpringConstraint(pConstraintObj, rbA, rbB);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	CPhysicsGeneric6DofSpringConstraint* CPhysicsFactory::CreateGeneric6DofSpringConstraint(CPhysicsRigidBody* rbB
		, const PARAVECTOR3& rbBOrigin
		, const PARAMATRIX3x3& rbBRotation
		, bool useLinearReferenceFrameB)
	{
		auto pBodyObjB = static_cast<IParaPhysicsRigidbody*>(rbB->get());
		auto pConstraintObj = m_pPhysicsInstance->CreateGeneric6DofSpringConstraint(pBodyObjB, rbBOrigin, rbBRotation, useLinearReferenceFrameB);
		if (!pConstraintObj)
			return nullptr;

		auto pConstraint = new CPhysicsGeneric6DofSpringConstraint(pConstraintObj, rbB, nullptr);
		pConstraint->AddToAutoReleasePool();

		return pConstraint;
	}

	
}
//-----------------------------------------------------------------------------
// Class:	CPhysicsWorld
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
#include "ParaPhysics.h"
#include "PhysicsWorld.h"

#include "IParaEngineApp.h"
#include "memdebug.h"

using namespace ParaEngine;

/** the bullet physics engine plugin dll file path */
#ifdef _DEBUG
const char* PHYSICS_DLL_FILE_PATH = "PhysicsBT_d.dll";
#else
const char* PHYSICS_DLL_FILE_PATH = "PhysicsBT.dll";
#endif

CPhysicsWorld::CPhysicsWorld()
: m_pPhysicsWorld(NULL), m_bRunDynamicSimulation(false)
{
}

CPhysicsWorld::~CPhysicsWorld(void)
{
	ExitPhysics();

	SAFE_RELEASE(m_pPhysicsWorld);
}

void CPhysicsWorld::InitPhysics()
{
	IParaPhysics* pPhysics = GetPhysicsInterface();
	if(pPhysics == 0)
	{
		OUTPUT_LOG("error: failed loading physics lib at %s \n", PHYSICS_DLL_FILE_PATH);
		return;
	}
	pPhysics->InitPhysics();
	pPhysics->SetDebugDrawer(CGlobals::GetScene()->GetDebugDrawer());
}


void CPhysicsWorld::ExitPhysics()
{
	if (m_pPhysicsWorld) 
	{
		m_pPhysicsWorld->ExitPhysics();
	}
	
	// clear all shapes
	TriangleMeshShape_Map_Type::iterator itCurCP, itEndCP = m_listMeshShapes.end();

	for( itCurCP = m_listMeshShapes.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		delete * itCurCP;
	}
	m_listMeshShapes.clear();
}

void CPhysicsWorld::ResetPhysics()
{
	ExitPhysics();
	InitPhysics();
}

void CPhysicsWorld::StepSimulation(double dTime)
{
	if(IsDynamicsSimulationEnabled())
	{
		if(m_pPhysicsWorld)
		{
			PERF1("Dynamic Physics");
			m_pPhysicsWorld->StepSimulation((float)dTime);
		}
	}
}


/**
* please note that multiple mesh actor may be created. 
*/
IParaPhysicsActor* ParaEngine::CPhysicsWorld::CreateStaticMesh( MeshEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup /*= 0*/, vector<IParaPhysicsActor*>* pOutputPhysicsActor /*= NULL*/, void* pUserData/*=NULL*/ )
{
	if(!ppMesh->IsValid() || ppMesh->GetMesh() == 0 || m_pPhysicsWorld==0)
		return NULL;
	TriangleMeshShape * MeshShape = NULL;
	
	/** Get the scaling factor from globalMat.
	* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
	* it is assumed that components of globalMat satisfies the following equation:
	* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
	* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
	* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
	*/
	float fScalingX,fScalingY,fScalingZ;
	Math::GetMatrixScaling(globalMat, &fScalingX,&fScalingY,&fScalingZ);
	
	// TODO: use a sorted set to store the shape list. it is more efficient
	TriangleMeshShape_Map_Type::iterator itCurCP, itEndCP = m_listMeshShapes.end();

	for( itCurCP = m_listMeshShapes.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		if((*itCurCP)->m_pMeshEntity == ppMesh)
		{
			// keep mesh for every scale level
			Vector3 vScale = (*itCurCP)->m_vScale;
			if(Vector3(vScale.x-fScalingX,vScale.y-fScalingY,vScale.z-fScalingZ).squaredLength() < FLT_TOLERANCE)
			{
				MeshShape = (*itCurCP);
				break;
			}			
		}
	}

	if(MeshShape == NULL)
	{
		// get physics mesh data to pSysMesh with only vertex position vector
		DWORD dwNumVx=0;
		Vector3 * verts = NULL;
		DWORD dwNumFaces = 0;
		WORD* indices = NULL;
		if(ppMesh->GetMesh() == 0)
			return NULL;
		int nMeshGroup = -1;
		
		while( (nMeshGroup = ppMesh->GetMesh()->GetNextPhysicsGroupID(nMeshGroup)) >=0 )
		{
			SAFE_DELETE_ARRAY(indices);
			if(!SUCCEEDED(ppMesh->GetMesh()->ClonePhysicsMesh((verts==NULL)? &dwNumVx : NULL,(verts==NULL)? (&verts) : NULL,&dwNumFaces,&indices, &nMeshGroup)) 
				|| dwNumFaces==0 )
			{
				if(MeshShape == NULL)
				{
					SAFE_DELETE_ARRAY(verts);
					SAFE_DELETE_ARRAY(indices);
					// physics mesh can not be loaded either because of an internal error or the mesh does not contain any physics faces.
					return NULL; 
				}
				else
					continue;
			}

			/// if the shape has not been created from MeshEntity, we will create it, and add to the shape list for reuse later
			if(MeshShape == NULL)
			{
				MeshShape = new TriangleMeshShape();
				MeshShape->m_pMeshEntity  = ppMesh;
				MeshShape->m_vScale = Vector3(fScalingX, fScalingY, fScalingZ);
				m_listMeshShapes.push_back(MeshShape);

				// scale the vertex if necessary
				if(Vector3(fScalingX-1.0f,fScalingY-1.0f,fScalingZ-1.0f).squaredLength() > FLT_TOLERANCE)
				{
					for( DWORD i = 0; i < dwNumVx; ++ i )
					{
						// scale each vertex before hand
						verts[i].x *= fScalingX;
						verts[i].y *= fScalingY;
						verts[i].z *= fScalingZ;
					}
				}
			}

			// Create descriptor for triangle mesh
			ParaPhysicsTriangleMeshDesc trimeshDesc;
			trimeshDesc.m_numVertices		= dwNumVx;
			trimeshDesc.m_numTriangles		= dwNumFaces;
			trimeshDesc.m_pointStrideBytes	= sizeof(PARAVECTOR3);
			trimeshDesc.m_triangleStrideBytes = 3 * sizeof(int16);
			trimeshDesc.m_points			= verts;
			trimeshDesc.m_triangles			= indices;
			trimeshDesc.m_flags				= 0;

			// Cooking from memory

			MeshShape->m_pShapes.push_back(SubMeshPhysicsShape(m_pPhysicsWorld->CreateTriangleMeshShap(trimeshDesc), nMeshGroup));
		}
		SAFE_DELETE_ARRAY(verts);
		SAFE_DELETE_ARRAY(indices);
	}
	/// Create the static actor
	if(MeshShape != NULL && !(MeshShape->m_pShapes.empty()))
	{
		// Generate report
		if(CGlobals::WillGenReport())
		{
			CGlobals::GetReport()->SetValue("physics counts",CGlobals::GetReport()->GetValue("physics counts")+1);
		}
		IParaPhysicsActor* pFirstActor = NULL;
		int nMeshCount = (int)MeshShape->m_pShapes.size();
		for(int i=0; i<nMeshCount; ++i)
		{
			ParaPhysicsActorDesc ActorDesc;
			int nPhysicsGroup = MeshShape->m_pShapes[i].m_nPhysicsGroup;
			ActorDesc.m_group = (int16)((nPhysicsGroup==0) ? (nShapeGroup) : nPhysicsGroup);
			ActorDesc.m_mask = -1;
			// this ensures a static object. 
			ActorDesc.m_mass = 0.f;
			ActorDesc.m_pShape = MeshShape->m_pShapes[i].m_pShape;
			// set global world position
			ActorDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42,globalMat._43);

			// remove the scaling factor from the rotation matrix
			for (int i=0;i<3;++i)
			{
				ActorDesc.m_rotation.m[0][i] = globalMat.m[0][i]/fScalingX;
				ActorDesc.m_rotation.m[1][i] = globalMat.m[1][i]/fScalingY;
				ActorDesc.m_rotation.m[2][i] = globalMat.m[2][i]/fScalingZ;
			}

			IParaPhysicsActor* pActor = m_pPhysicsWorld->CreateActor(ActorDesc);
			if(pActor!=0)
			{
				if(pUserData!=NULL)
				{
					pActor->SetUserData(pUserData);
				}
				if(pOutputPhysicsActor!=NULL)
				{
					pOutputPhysicsActor->push_back(pActor);
				}
				if(pFirstActor == NULL)
				{
					pFirstActor = pActor;
				}
			}
		}
		return pFirstActor;
	}

	return NULL;
}

IParaPhysicsActor* ParaEngine::CPhysicsWorld::CreateStaticMesh(ParaXEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup /*= 0*/, vector<IParaPhysicsActor*>* pOutputPhysicsActor /*= NULL*/, void* pUserData /*= NULL*/)
{
	if (!ppMesh->IsValid() || ppMesh->GetModel() == 0 || m_pPhysicsWorld == 0)
		return NULL;
	TriangleMeshShape * MeshShape = NULL;

	/** Get the scaling factor from globalMat.
	* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
	* it is assumed that components of globalMat satisfies the following equation:
	* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
	* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
	* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
	*/
	float fScalingX, fScalingY, fScalingZ;
	Math::GetMatrixScaling(globalMat, &fScalingX, &fScalingY, &fScalingZ);

	// TODO: use a sorted set to store the shape list. it is more efficient
	TriangleMeshShape_Map_Type::iterator itCurCP, itEndCP = m_listMeshShapes.end();

	for (itCurCP = m_listMeshShapes.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		if ((*itCurCP)->m_pParaXEntity == ppMesh)
		{
			// keep mesh for every scale level
			Vector3 vScale = (*itCurCP)->m_vScale;
			if (Vector3(vScale.x - fScalingX, vScale.y - fScalingY, vScale.z - fScalingZ).squaredLength() < FLT_TOLERANCE)
			{
				MeshShape = (*itCurCP);
				break;
			}
		}
	}

	if (MeshShape == NULL)
	{
		// get physics mesh data to pSysMesh with only vertex position vector
		DWORD dwNumVx = 0;
		Vector3 * verts = NULL;
		DWORD dwNumFaces = 0;
		WORD* indices = NULL;
		if (ppMesh->GetModel() == 0)
			return NULL;
		int nMeshGroup = -1;

		while ((nMeshGroup = ppMesh->GetModel()->GetNextPhysicsGroupID(nMeshGroup)) >= 0)
		{
			SAFE_DELETE_ARRAY(indices);
			if (!SUCCEEDED(ppMesh->GetModel()->ClonePhysicsMesh((verts == NULL) ? &dwNumVx : NULL, (verts == NULL) ? (&verts) : NULL, &dwNumFaces, &indices, &nMeshGroup))
				|| dwNumFaces == 0)
			{
				if (MeshShape == NULL)
				{
					SAFE_DELETE_ARRAY(verts);
					SAFE_DELETE_ARRAY(indices);
					// physics mesh can not be loaded either because of an internal error or the mesh does not contain any physics faces.
					return NULL;
				}
				else
					continue;
			}

			/// if the shape has not been created from MeshEntity, we will create it, and add to the shape list for reuse later
			if (MeshShape == NULL)
			{
				MeshShape = new TriangleMeshShape();
				MeshShape->m_pParaXEntity = ppMesh;
				MeshShape->m_vScale = Vector3(fScalingX, fScalingY, fScalingZ);
				m_listMeshShapes.push_back(MeshShape);

				// scale the vertex if necessary
				if (Vector3(fScalingX - 1.0f, fScalingY - 1.0f, fScalingZ - 1.0f).squaredLength() > FLT_TOLERANCE)
				{
					for (DWORD i = 0; i < dwNumVx; ++i)
					{
						// scale each vertex before hand
						verts[i].x *= fScalingX;
						verts[i].y *= fScalingY;
						verts[i].z *= fScalingZ;
					}
				}
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

			// Cooking from memory

			MeshShape->m_pShapes.push_back(SubMeshPhysicsShape(m_pPhysicsWorld->CreateTriangleMeshShap(trimeshDesc), nMeshGroup));
		}
		SAFE_DELETE_ARRAY(verts);
		SAFE_DELETE_ARRAY(indices);
	}
	/// Create the static actor
	if (MeshShape != NULL && !(MeshShape->m_pShapes.empty()))
	{
		// Generate report
		if (CGlobals::WillGenReport())
		{
			CGlobals::GetReport()->SetValue("physics counts", CGlobals::GetReport()->GetValue("physics counts") + 1);
		}
		IParaPhysicsActor* pFirstActor = NULL;
		int nMeshCount = (int)MeshShape->m_pShapes.size();
		for (int i = 0; i<nMeshCount; ++i)
		{
			ParaPhysicsActorDesc ActorDesc;
			int nPhysicsGroup = MeshShape->m_pShapes[i].m_nPhysicsGroup;
			ActorDesc.m_group = (int16)((nPhysicsGroup == 0) ? (nShapeGroup) : nPhysicsGroup);
			ActorDesc.m_mask = -1;
			// this ensures a static object. 
			ActorDesc.m_mass = 0.f;
			ActorDesc.m_pShape = MeshShape->m_pShapes[i].m_pShape;
			// set global world position
			ActorDesc.m_origin = PARAVECTOR3(globalMat._41, globalMat._42, globalMat._43);

			// remove the scaling factor from the rotation matrix
			for (int i = 0; i<3; ++i)
			{
				ActorDesc.m_rotation.m[0][i] = globalMat.m[0][i] / fScalingX;
				ActorDesc.m_rotation.m[1][i] = globalMat.m[1][i] / fScalingY;
				ActorDesc.m_rotation.m[2][i] = globalMat.m[2][i] / fScalingZ;
			}

			IParaPhysicsActor* pActor = m_pPhysicsWorld->CreateActor(ActorDesc);
			if (pActor != 0)
			{
				if (pUserData != NULL)
				{
					pActor->SetUserData(pUserData);
				}
				if (pOutputPhysicsActor != NULL)
				{
					pOutputPhysicsActor->push_back(pActor);
				}
				if (pFirstActor == NULL)
				{
					pFirstActor = pActor;
				}
			}
		}
		return pFirstActor;
	}

	return NULL;
}

void CPhysicsWorld::ReleaseActor(IParaPhysicsActor* pActor)
{
	if(m_pPhysicsWorld)
	{
		m_pPhysicsWorld->ReleaseActor(pActor);
		if(CGlobals::WillGenReport())
		{
			CGlobals::GetReport()->SetValue("physics counts",CGlobals::GetReport()->GetValue("physics counts")-1);
		}
	}
}

// iOS does not support dynamically loaded dll, hence we will use statically linked plugin. 
#if defined(STATIC_PLUGIN_PHYSICS_BT)
extern ClassDescriptor* PhysicsBT_GetClassDesc();
#endif

IParaPhysics* CPhysicsWorld::GetPhysicsInterface()
{
	if(m_pPhysicsWorld)
		return m_pPhysicsWorld;

#ifdef STATIC_PLUGIN_PHYSICS_BT
	ClassDescriptor* pClassDesc = PhysicsBT_GetClassDesc();
	if (pClassDesc && (strcmp(pClassDesc->ClassName(), "IParaPhysics") == 0))
	{
		m_pPhysicsWorld = (IParaPhysics*)pClassDesc->Create();
		OUTPUT_LOG("physics BT engine loaded statically\n");
	}
#else
	DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(PHYSICS_DLL_FILE_PATH);
	if(pPluginEntity==0)
	{
		// load the plug-in if it has never been loaded before. 
		pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", PHYSICS_DLL_FILE_PATH);
	}

	if(pPluginEntity!=0)
	{
		for (int i=0; i < pPluginEntity->GetNumberOfClasses(); ++i)
		{
			ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

			if(pClassDesc && (strcmp(pClassDesc->ClassName(), "IParaPhysics") == 0))
			{
				m_pPhysicsWorld = (IParaPhysics*) pClassDesc->Create();
			}
		}
	}
#endif
	if (m_pPhysicsWorld == nullptr)
	{
		OUTPUT_LOG("note: no physics engine found, null physics is used \n");
		m_pPhysicsWorld = new CParaPhysicsImp();
	}
	return m_pPhysicsWorld;
}

void ParaEngine::CPhysicsWorld::SetDynamicsSimulationEnabled( bool bEnable )
{
	m_bRunDynamicSimulation = bEnable;
}

bool ParaEngine::CPhysicsWorld::IsDynamicsSimulationEnabled()
{
	return m_bRunDynamicSimulation;
}

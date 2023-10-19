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

#include "BaseObject.h"
#include "ShapeAABB.h"

#include "BlockEngine/BlockChunk.h"
#include "BlockEngine/BlockRegion.h"
#include "BlockEngine/BlockWorldClient.h"
#include "util/StringHelper.h"
#include "NPL/NPLHelper.h"

using namespace ParaEngine;

/** the bullet physics engine plugin dll file path */
#if defined(WIN32)
#define DLL_FILE_EXT  "dll"
#elif defined(PLATFORM_MAC)
#define DLL_FILE_EXT "dylib"
#else
#define DLL_FILE_EXT "so"
#endif


#if defined(_DEBUG) && defined(WIN32)
const char* PHYSICS_DLL_FILE_PATH = ("PhysicsBT_d." DLL_FILE_EXT);
#else
const char* PHYSICS_DLL_FILE_PATH = ("PhysicsBT." DLL_FILE_EXT);
#endif


std::shared_ptr<CPhysicsBlockShape> CPhysicsBlock::GetShape(BlockModel& model, IParaPhysics* world)
{
	uint32_t key = GetKey();
	auto shapeIndexMap = GetShapeIndexMap();
	auto shapeList = GetShapeList();
	auto it = shapeIndexMap->find(key);
	if (it != shapeIndexMap->end()) return (*shapeList)[it->second];
	std::shared_ptr<CPhysicsBlockShape> pShape = std::make_shared<CPhysicsBlockShape>();

	int pointCount = model.GetVerticesCount();
	int faceCount = model.GetFaceCount();
	int triangleCount = faceCount * 2;
	int pointStrideBytes = sizeof(Vector3);
	BlockVertexCompressed* vertices = model.GetVertices();
	std::string source(pointCount * pointStrideBytes, '\0');

	Vector3* pVertices = new Vector3[pointCount];
	for (int i = 0; i < pointCount; i++)
	{
		pVertices[i].x = vertices[i].position[0];
		pVertices[i].y = vertices[i].position[1];
		pVertices[i].z = vertices[i].position[2];
		// 顶点字符串化
		const char* srcStr = (const char*)(&(pVertices[i]));
		const int startStrIndex = i * pointStrideBytes;
		for (int j = 0; j < pointStrideBytes; j++)
		{
			source[startStrIndex + j] = *srcStr;
			srcStr++;
		}
	}

	uint16_t index = 0;
	uint16_t* pIndices = new uint16_t[triangleCount * 3];
	bool stdCubeFaces[6] = { false, false, false, false, false, false }; // x 0, 1, y 0, 1 z 0, 1
	bool isStdCubeShape = faceCount == 6;
	for (int i = 0; i < faceCount; i++)
	{
		uint16_t indexOfs = 4 * i;
		Vector3 pts[4] = { pVertices[indexOfs + 0], pVertices[indexOfs + 1], pVertices[indexOfs + 2], pVertices[indexOfs + 3] };
		if (pts[0] != pts[1] && pts[0] != pts[3] && pts[1] != pts[3])
		{
			pIndices[index++] = indexOfs + 0;
			pIndices[index++] = indexOfs + 1;
			pIndices[index++] = indexOfs + 3;
		}
		if (pts[1] != pts[2] && pts[1] != pts[3] && pts[2] != pts[3])
		{
			pIndices[index++] = indexOfs + 1;
			pIndices[index++] = indexOfs + 2;
			pIndices[index++] = indexOfs + 3;
		}
		// TODO 识别正方体模型
		if (isStdCubeShape)
		{
			// 一个面四个顶点两两不等
			isStdCubeShape = (pts[0] != pts[1]) && (pts[0] != pts[2]) && (pts[0] != pts[3]) && (pts[1] != pts[2]) && (pts[1] != pts[3]) && (pts[2] != pts[3]);
			bool tmpStdCubeFaces[6];
			tmpStdCubeFaces[0] = (pts[0].x == 0) && (pts[1].x == 0) && (pts[2].x == 0) && (pts[3].x == 0);
			tmpStdCubeFaces[1] = (pts[0].x == 1) && (pts[1].x == 1) && (pts[2].x == 1) && (pts[3].x == 1);
			tmpStdCubeFaces[2] = (pts[0].y == 0) && (pts[1].y == 0) && (pts[2].y == 0) && (pts[3].y == 0);
			tmpStdCubeFaces[3] = (pts[0].y == 1) && (pts[1].y == 1) && (pts[2].y == 1) && (pts[3].y == 1);
			tmpStdCubeFaces[4] = (pts[0].z == 0) && (pts[1].z == 0) && (pts[2].z == 0) && (pts[3].z == 0);
			tmpStdCubeFaces[5] = (pts[0].z == 1) && (pts[1].z == 1) && (pts[2].z == 1) && (pts[3].z == 1);
			int stdCubeFace = -1;
			for (int i = 0; i < 6 && isStdCubeShape; i++)
			{
				if (tmpStdCubeFaces[i])
				{
					if (stdCubeFace < 0)
					{
						stdCubeFace = i;
					}
					else
					{
						isStdCubeShape = false;
					}
				}
			}
			if (isStdCubeShape && stdCubeFace >= 0)
			{
				stdCubeFaces[stdCubeFace] = true;
			}
			else
			{
				isStdCubeShape = false;
			}
		}
	}
	for (int i = 0; i < 6 && isStdCubeShape; i++)
	{
		if (!stdCubeFaces[i]) isStdCubeShape = false;
	}
	triangleCount = index / 3;

	// hash 是否已被缓存
	pShape->m_hash = isStdCubeShape ? CPhysicsBlockShape::GetStdCubeHash() : StringHelper::md5(source);
	for (int i = 0; i < shapeList->size(); i++)
	{
		if ((*shapeList)[i]->m_hash == pShape->m_hash)
		{
			shapeIndexMap->insert(std::make_pair(key, i));
			delete[] pVertices;
			delete[] pIndices;
			return (*shapeList)[i];
		}
	}
	// 新建shape
	if (isStdCubeShape)
	{
		ParaPhysicsSimpleShapeDesc desc;
		desc.m_shape = "box";
		desc.m_halfWidth = BlockConfig::g_half_blockSize;
		desc.m_halfHeight = BlockConfig::g_half_blockSize;
		desc.m_halfLength = BlockConfig::g_half_blockSize;
		pShape->m_shape = world->CreateSimpleShape(desc);
	}
	else
	{
		ParaPhysicsTriangleMeshDesc trimeshDesc;
		trimeshDesc.m_numVertices = pointCount;
		trimeshDesc.m_numTriangles = triangleCount;
		trimeshDesc.m_pointStrideBytes = pointStrideBytes;
		trimeshDesc.m_triangleStrideBytes = 3 * sizeof(uint16_t);
		trimeshDesc.m_points = pVertices;
		trimeshDesc.m_triangles = pIndices;
		trimeshDesc.m_flags = 0;
		pShape->m_shape = world->CreateTriangleMeshShape(trimeshDesc);
	}
	shapeIndexMap->insert(std::make_pair(key, (uint16_t)shapeList->size()));
	shapeList->push_back(pShape);
	delete[] pIndices;
	delete[] pVertices;
	return pShape;
}

void CPhysicsBlock::Load(BlockModel& model, IParaPhysics* world)
{
	if (m_actor) return;
	auto pShape = GetShape(model, world);
	if (!pShape->m_shape) return;

	ParaPhysicsActorDesc ActorDesc;
	ActorDesc.m_group = IParaPhysicsGroup::BLOCK;              // 2 字节 地块占用最高为分组
	ActorDesc.m_mask = -1 ^ (1 << ActorDesc.m_group);
	ActorDesc.m_mass = 0.0f;
	ActorDesc.m_pShape = pShape->m_shape;

	uint16_t bx, by, bz;
	float offset_y = BlockWorldClient::GetInstance()->GetVerticalOffset();
	UnPackID(GetID(), bx, by, bz);
	if (pShape->IsStdCube())
	{
		ActorDesc.m_origin = PARAVECTOR3((bx + 0.5f) * BlockConfig::g_dBlockSize, (by + 0.5f) * BlockConfig::g_dBlockSize + offset_y, (bz + 0.5f) * BlockConfig::g_dBlockSize);
	}
	else
	{
		ActorDesc.m_origin = PARAVECTOR3(bx * BlockConfig::g_dBlockSize, by * BlockConfig::g_dBlockSize + offset_y, bz * BlockConfig::g_dBlockSize);
	}
	Quaternion quat;
	quat.ToRotationMatrix((Matrix3&)ActorDesc.m_rotation);
	m_actor = world->CreateActor(ActorDesc);
	m_world = world;
}

void CPhysicsBlock::Unload()
{
	if (m_actor && m_world)
	{
		m_world->ReleaseActor(m_actor);
		m_world = nullptr;
		m_actor = nullptr;
	}
}

CPhysicsWorld::CPhysicsWorld()
	: m_pPhysicsWorld(NULL), m_bRunDynamicSimulation(true)
{
}

CPhysicsWorld::~CPhysicsWorld(void)
{
	ExitPhysics();

	SAFE_RELEASE(m_pPhysicsWorld);
}

void CPhysicsWorld::SetActorPhysicsProperty(IParaPhysicsActor* actor, const char* property)
{
	if (actor == nullptr) return ;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(property, (int)strlen(property));
	if (msg.GetType() == NPL::NPLObjectBase::NPLObjectType_Table) 
	{
		if (msg["Mass"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetMass((float)(double)msg["Mass"]);
		if (msg["LocalInertiaX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetLocalInertia(PARAVECTOR3((float)(double)msg["LocalInertiaX"], (float)(double)msg["LocalInertiaY"], (float)(double)msg["LocalInertiaZ"]));
		if (msg["GravityX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetGravity(PARAVECTOR3((float)(double)msg["GravityX"], (float)(double)msg["GravityY"], (float)(double)msg["GravityZ"]));
		if (msg["LinearDamping"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetLinearDamping((float)(double)msg["LinearDamping"]);
		if (msg["AngularDamping"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetAngularDamping((float)(double)msg["AngularDamping"]);
		if (msg["LinearFactorX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetLinearFactor(PARAVECTOR3((float)(double)msg["LinearFactorX"], (float)(double)msg["LinearFactorY"], (float)(double)msg["LinearFactorZ"]));
		if (msg["AngularFactorX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetAngularFactor(PARAVECTOR3((float)(double)msg["AngularFactorX"], (float)(double)msg["AngularFactorY"], (float)(double)msg["AngularFactorZ"]));
		if (msg["LinearVelocityX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetLinearVelocity(PARAVECTOR3((float)(double)msg["LinearVelocityX"], (float)(double)msg["LinearVelocityY"], (float)(double)msg["LinearVelocityZ"]));
		if (msg["AngularVelocityX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetAngularVelocity(PARAVECTOR3((float)(double)msg["AngularVelocityX"], (float)(double)msg["AngularVelocityY"], (float)(double)msg["AngularVelocityZ"]));
		if (msg["Flags"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetFlags((int)(double)msg["Flags"]);
		if (msg["ActivationState"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetActivationState((int)(double)msg["ActivationState"]);
		if (msg["DeactivationTime"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetDeactivationTime((float)(double)msg["DeactivationTime"]);
		if (msg["Restitution"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetRestitution((float)(double)msg["Restitution"]);
		if (msg["Friction"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetFriction((float)(double)msg["Friction"]);
		if (msg["RollingFriction"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetRollingFriction((float)(double)msg["RollingFriction"]);
		if (msg["SpinningFriction"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetSpinningFriction((float)(double)msg["SpinningFriction"]);
		if (msg["ContactStiffness"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetContactStiffness((float)(double)msg["ContactStiffness"]);
		if (msg["ContactDamping"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetContactDamping((float)(double)msg["ContactDamping"]);
		if (msg["IslandTag"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetIslandTag((int)(double)msg["IslandTag"]);
		if (msg["CompanionId"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetCompanionId((int)(double)msg["CompanionId"]);
		if (msg["HitFraction"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetHitFraction((float)(double)msg["HitFraction"]);
		if (msg["CollisionFlags"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetCollisionFlags((int)(double)msg["CollisionFlags"]);
		if (msg["CcdSweptSphereRadius"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetCcdSweptSphereRadius((float)(double)msg["CcdSweptSphereRadius"]);
		if (msg["CcdMotionThreshold"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) actor->SetCcdMotionThreshold((float)(double)msg["CcdMotionThreshold"]);
	}
}

const char* CPhysicsWorld::GetActorPhysicsProperty(IParaPhysicsActor* actor)
{
	if (actor == nullptr) return "";

	static std::string sCode;
	NPL::NPLObjectProxy msg;
	PARAVECTOR3 vec3;
	msg["Mass"] = actor->GetMass();
	vec3 = actor->GetLocalInertia();
	msg["LocalInertiaX"] = vec3.x;
	msg["LocalInertiaY"] = vec3.y;
	msg["LocalInertiaZ"] = vec3.z;
	vec3 = actor->GetGravity();
	msg["GravityX"] = vec3.x;
	msg["GravityY"] = vec3.y;
	msg["GravityZ"] = vec3.z;
	msg["LinearDamping"] = actor->GetLinearDamping();
	msg["AngularDamping"] = actor->GetAngularDamping();
	vec3 = actor->GetLinearFactor();
	msg["LinearFactorX"] = vec3.x;
	msg["LinearFactorY"] = vec3.y;
	msg["LinearFactorZ"] = vec3.z;
	vec3 = actor->GetAngularFactor();
	msg["AngularFactorX"] = vec3.x;
	msg["AngularFactorY"] = vec3.y;
	msg["AngularFactorZ"] = vec3.z;
	vec3 = actor->GetLinearVelocity();
	msg["LinearVelocityX"] = vec3.x;
	msg["LinearVelocityY"] = vec3.y;
	msg["LinearVelocityZ"] = vec3.z;
	vec3 = actor->GetAngularVelocity();
	msg["AngularVelocityX"] = vec3.x;
	msg["AngularVelocityY"] = vec3.y;
	msg["AngularVelocityZ"] = vec3.z;
	msg["Flags"] = (double)actor->GetFlags();
	msg["ActivationState"] = (double)actor->GetActivationState();
	msg["DeactivationTime"] = actor->GetDeactivationTime();
	msg["Restitution"] = actor->GetRestitution();
	msg["Friction"] = actor->GetFriction();
	msg["RollingFriction"] = actor->GetRollingFriction();
	msg["SpinningFriction"] = actor->GetSpinningFriction();
	msg["ContactStiffness"] = actor->GetContactStiffness();
	msg["ContactDamping"] = actor->GetContactDamping();
	msg["IslandTag"] = (double)actor->GetIslandTag();
	msg["CompanionId"] = (double)actor->GetCompanionId();
	msg["HitFraction"] = actor->GetHitFraction();
	msg["CollisionFlags"] = (double)actor->GetCollisionFlags();
	msg["CcdSweptSphereRadius"] = actor->GetCcdSweptSphereRadius();
	msg["CcdMotionThreshold"] = actor->GetCcdMotionThreshold();
	NPL::NPLHelper::NPLTableToString(NULL, msg, sCode);
	return sCode.c_str();
}

void CPhysicsWorld::InitPhysics()
{
	IParaPhysics* pPhysics = GetPhysicsInterface();
	if (pPhysics == 0)
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

	for (itCurCP = m_listMeshShapes.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		delete* itCurCP;
	}
	m_listMeshShapes.clear();
	m_mapDynamicActors.clear();
	// 清空块
	m_mapPhysicsBlocks.clear();
	CPhysicsBlock::GetShapeIndexMap()->clear();
	CPhysicsBlock::GetShapeList()->clear();
}

void CPhysicsWorld::ResetPhysics()
{
	ExitPhysics();
	InitPhysics();
}

void CPhysicsWorld::StepSimulation(double dTime)
{
#ifdef WIN32
    Matrix4 matrix;
#else
    alignas(16) Matrix4 matrix;
#endif
	CShapeAABB aabb;
	static int16_t s_block_frame_id = 0;
	s_block_frame_id++;
	if (IsDynamicsSimulationEnabled())
	{
		IParaPhysicsActor_Map_Type::iterator itCurCP = m_mapDynamicActors.begin();
		IParaPhysicsActor_Map_Type::iterator itEndCP = m_mapDynamicActors.end();

		// 加载地形
		BlockWorldClient* pWorld = BlockWorldClient::GetInstance();
		bool isAutoPhysicsBlock = pWorld->IsAutoPhysics();
		if (isAutoPhysicsBlock)
		{
			for (itCurCP = m_mapDynamicActors.begin(); itCurCP != itEndCP; itCurCP++)
			{
				IParaPhysicsActor* actor = *itCurCP;
				CBaseObject* obj = (CBaseObject*)(actor->GetUserData());
				obj->GetAABB(&aabb); // 已经包含中心点
				LoadPhysicsBlock(&aabb, s_block_frame_id);
			}
		}

		if (m_pPhysicsWorld)
		{
			PERF1("Dynamic Physics");
			m_pPhysicsWorld->StepSimulation((float)dTime);
		}

		// TODO 多线程是否需要加锁
		for (itCurCP = m_mapDynamicActors.begin(); itCurCP != itEndCP; itCurCP++)
		{
			IParaPhysicsActor* actor = *itCurCP;
			CBaseObject* obj = (CBaseObject*)(actor->GetUserData());
			if (actor->IsStaticOrKinematicObject())
			{
				// 属性设置为 CollisionFlags=2, ActivationState=4 可右玩家控制位置同步至物理世界 
				auto pAsset = obj->GetPrimaryAsset();
				CParaXModel* pModel = ((ParaXEntity*)pAsset)->GetModel();
				float halfHeight = pModel->GetHeader().maxExtent.y * 0.5f;
				Vector3 vCenter(0, halfHeight, 0);
				obj->GetLocalTransform(&matrix);
				vCenter = vCenter * matrix;
				Vector3 vPos = ((Vector3)(obj->GetPosition())) + vCenter;
				Vector3 vScale, vTrans;
				Quaternion quat;
				ParaMatrixDecompose(&vScale, &quat, &vTrans, &matrix);
				quat.ToRotationMatrix(matrix, vPos);
				actor->SetWorldTransform((PARAMATRIX*)&matrix);
			}
			else
			{
				actor->GetWorldTransform((PARAMATRIX*)&matrix);
				Vector3 pos = matrix.getTrans();
				float fCenterHeight = obj->GetAssetHeight() * 0.5f;

				// make this rotation matrix
				matrix.setTrans(Vector3(0, 0, 0));
				obj->SetPosition(DVector3(pos.x, pos.y - fCenterHeight, pos.z));

				Matrix4 matOffset;
				fCenterHeight = fCenterHeight / obj->GetScaling();
				matOffset.makeTrans(Vector3(0, -fCenterHeight, 0));
				matOffset = matOffset * matrix;
				matOffset.offsetTrans(Vector3(0, fCenterHeight, 0));

				obj->SetLocalTransform(matOffset);
				obj->SetYaw(0);
				obj->SetRoll(0);
				obj->SetPitch(0);
			}
		}

		// 移除无效方块
		if (isAutoPhysicsBlock)
		{
			auto it = m_mapPhysicsBlocks.begin();
			while (it != m_mapPhysicsBlocks.end())
			{
				auto curIt = it++;
				if (std::abs(s_block_frame_id - curIt->second->GetFrameId()) > 2)
				{
					m_mapPhysicsBlocks.erase(curIt);
				}
			}
		}
	}
}

IParaPhysicsActor* ParaEngine::CPhysicsWorld::CreateDynamicMesh(CBaseObject* obj)
{
	ParaPhysicsSimpleShapeDesc desc;
	desc.m_shape = obj->GetPhysicsShape();

	bool bHasModel = false;
	auto pAsset = obj->GetPrimaryAsset();
	if (pAsset && pAsset->GetType() == AssetEntity::parax)
	{
		CParaXModel* pModel = ((ParaXEntity*)pAsset)->GetModel();
		if (pModel != 0)
		{
			float fScale = obj->GetScaling();
			Vector3 vMin = pModel->GetHeader().minExtent;
			Vector3 vMax = pModel->GetHeader().maxExtent;
			desc.m_halfWidth = max(abs(vMax.x), abs(vMin.x)) * fScale;
			desc.m_halfHeight = vMax.y * 0.5f * fScale;
			desc.m_halfLength = max(abs(vMax.z), abs(vMin.z)) * fScale;
			bHasModel = true;
		}
	}
	if (!bHasModel)
		return NULL; // model is not ready, such as not loaded from disk. 

	IParaPhysicsShape* pShape = m_pPhysicsWorld->CreateSimpleShape(desc);

	ParaPhysicsActorDesc ActorDesc;
	ActorDesc.m_group = obj->GetPhysicsGroup();
	ActorDesc.m_mask = -1;
	ActorDesc.m_mass = 1.0f;
	ActorDesc.m_pShape = pShape;

	// set world position
	Matrix4 localMat;
	Vector3 vCenter(0, desc.m_halfHeight / obj->GetScaling(), 0);
	obj->GetLocalTransform(&localMat);
	vCenter = vCenter * localMat;
	auto vPos = obj->GetPosition();
	ActorDesc.m_origin = PARAVECTOR3((float)(vPos.x + vCenter.x), (float)(vPos.y + vCenter.y), (float)(vPos.z + vCenter.z));

	// set world local rotation matrix
	Vector3 vScale, vTrans;
	Quaternion quat;
	ParaMatrixDecompose(&vScale, &quat, &vTrans, &localMat);
	quat.ToRotationMatrix((Matrix3&)ActorDesc.m_rotation);

	IParaPhysicsActor* pActor = m_pPhysicsWorld->CreateActor(ActorDesc);
	pActor->SetUserData(obj);
	m_mapDynamicActors.insert(pActor);
	return pActor;
}

void ParaEngine::CPhysicsWorld::LoadPhysicsBlock(CShapeAABB* aabb, int16_t frameId, float extend)
{
	Vector3 min, max;
	aabb->GetMin(min);
	aabb->GetMax(max);
	min -= extend;  // extend 为下一帧可能的距离
	max += extend;
	float offset_y = BlockWorldClient::GetInstance()->GetVerticalOffset();
	// min, max 为世界坐标使用  BlockConfig::g_dBlockSize 局部坐标使用 BlockConfig::g_blockSize
	int16_t min_x = (int16_t)std::floor(min.x / BlockConfig::g_dBlockSize);
	int16_t min_y = (int16_t)std::floor((min.y - offset_y) / BlockConfig::g_dBlockSize);
	int16_t min_z = (int16_t)std::floor(min.z / BlockConfig::g_dBlockSize);
	int16_t max_x = (int16_t)std::floor(max.x / BlockConfig::g_dBlockSize);
	int16_t max_y = (int16_t)std::floor((max.y - offset_y) / BlockConfig::g_dBlockSize);
	int16_t max_z = (int16_t)std::floor(max.z / BlockConfig::g_dBlockSize);
	if (min_x < 0 || min_y < 0 || min_z < 0) return;

	for (int16_t bx = min_x; bx <= max_x; bx++)
	{
		for (int16_t by = min_y; by <= max_y; by++)
		{
			for (int16_t bz = min_z; bz <= max_z; bz++)
			{
				std::shared_ptr<CPhysicsBlock> pBlock = LoadPhysicsBlock(bx, by, bz);
				if (pBlock != nullptr)
				{
					pBlock->SetFrameId(frameId);
				}
			}
		}
	}
}

std::shared_ptr<CPhysicsBlock> ParaEngine::CPhysicsWorld::LoadPhysicsBlock(uint16_t bx, uint16_t by, uint16_t bz)
{
	CBlockWorld* pWorld = BlockWorldClient::GetInstance();
	BlockTemplate* pTemplate = pWorld->GetBlockTemplate(bx, by, bz);
	Block* pBlock = pWorld->GetBlock(bx, by, bz);
	uint64_t id = CPhysicsBlock::PackID(bx, by, bz);
	if (!pTemplate || !pBlock)
	{
		m_mapPhysicsBlocks.erase(id);
		return nullptr;
	}

	uint16_t blockData = pBlock->GetUserData();
	uint16_t tplId = pTemplate->GetID();
	uint32_t key = (blockData << 16) + tplId;
	BlockModel& model = pTemplate->GetBlockModel(pWorld, bx, by, bz, blockData);

	auto it = m_mapPhysicsBlocks.find(id);
	if (it != m_mapPhysicsBlocks.end())
	{
		std::shared_ptr<CPhysicsBlock> pBlock = it->second;
		if (pBlock->GetKey() == key)
		{
			// 不存在加载失败情况, 可以屏蔽此行
			if (!pBlock->IsLoaded()) 
			{
				pBlock->Load(model, m_pPhysicsWorld);
				SetActorPhysicsProperty(pBlock->GetActor(), pTemplate->GetPhysicsProperty().c_str());
			}
			return pBlock;
		}
		// 发生改变删除
		m_mapPhysicsBlocks.erase(it);
	}

	// 非实体方块不做物理映射
	if (!pWorld->IsObstructionBlock(bx, by, bz))
	{
		return nullptr;
	}

	// 加载physics block
	std::shared_ptr<CPhysicsBlock> newBlock = std::make_shared<CPhysicsBlock>(id, key);
	newBlock->Load(model, m_pPhysicsWorld);
	SetActorPhysicsProperty(newBlock->GetActor(), pTemplate->GetPhysicsProperty().c_str());
	m_mapPhysicsBlocks.insert(std::make_pair(id, newBlock));

	return newBlock;
}

/**
* please note that multiple mesh actor may be created.
*/
IParaPhysicsActor* ParaEngine::CPhysicsWorld::CreateStaticMesh(MeshEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup /*= 0*/, vector<IParaPhysicsActor*>* pOutputPhysicsActor /*= NULL*/, void* pUserData/*=NULL*/)
{
	if (!ppMesh->IsValid() || ppMesh->GetMesh() == 0 || m_pPhysicsWorld == 0)
		return NULL;
	TriangleMeshShape* MeshShape = NULL;

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
		if ((*itCurCP)->m_pMeshEntity == ppMesh)
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
		Vector3* verts = NULL;
		DWORD dwNumFaces = 0;
		WORD* indices = NULL;
		if (ppMesh->GetMesh() == 0)
			return NULL;
		int nMeshGroup = -1;

		while ((nMeshGroup = ppMesh->GetMesh()->GetNextPhysicsGroupID(nMeshGroup)) >= 0)
		{
			SAFE_DELETE_ARRAY(indices);
			if (!SUCCEEDED(ppMesh->GetMesh()->ClonePhysicsMesh((verts == NULL) ? &dwNumVx : NULL, (verts == NULL) ? (&verts) : NULL, &dwNumFaces, &indices, &nMeshGroup))
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
				MeshShape->m_pMeshEntity = ppMesh;
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

			MeshShape->m_pShapes.push_back(SubMeshPhysicsShape(m_pPhysicsWorld->CreateTriangleMeshShape(trimeshDesc), nMeshGroup));
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
		for (int i = 0; i < nMeshCount; ++i)
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
			for (int i = 0; i < 3; ++i)
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

IParaPhysicsActor* ParaEngine::CPhysicsWorld::CreateStaticMesh(ParaXEntity* ppMesh, const Matrix4& globalMat, uint32 nShapeGroup /*= 0*/, vector<IParaPhysicsActor*>* pOutputPhysicsActor /*= NULL*/, void* pUserData /*= NULL*/)
{
	if (!ppMesh->IsValid() || ppMesh->GetModel() == 0 || m_pPhysicsWorld == 0)
		return NULL;
	TriangleMeshShape* MeshShape = NULL;

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
		Vector3* verts = NULL;
		DWORD dwNumFaces = 0;
		DWORD* indices = NULL;
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
			trimeshDesc.m_triangleStrideBytes = 3 * sizeof(int32);
			trimeshDesc.m_points = verts;
			trimeshDesc.m_triangles = indices;
			trimeshDesc.m_flags = 0;

			// Cooking from memory

			MeshShape->m_pShapes.push_back(SubMeshPhysicsShape(m_pPhysicsWorld->CreateTriangleMeshShape(trimeshDesc), nMeshGroup));
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
		for (int i = 0; i < nMeshCount; ++i)
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
			for (int i = 0; i < 3; ++i)
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
	if (m_pPhysicsWorld)
	{
		m_mapDynamicActors.erase(pActor);
		m_pPhysicsWorld->ReleaseActor(pActor);
		if (CGlobals::WillGenReport())
		{
			CGlobals::GetReport()->SetValue("physics counts", CGlobals::GetReport()->GetValue("physics counts") - 1);
		}
}
}

// iOS does not support dynamically loaded dll, hence we will use statically linked plugin. 
#if defined(PHYSICS_STATICLIB)
extern ClassDescriptor* PhysicsBT_GetClassDesc();
#endif

IParaPhysics* CPhysicsWorld::GetPhysicsInterface()
{
	if (m_pPhysicsWorld)
		return m_pPhysicsWorld;

#ifdef PHYSICS_STATICLIB
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
		pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", PHYSICS_DLL_FILE_PATH);
	}

	if (pPluginEntity != 0)
	{
		for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
		{
			ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

			if (pClassDesc && (strcmp(pClassDesc->ClassName(), "IParaPhysics") == 0))
			{
				m_pPhysicsWorld = (IParaPhysics*)pClassDesc->Create();
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

void ParaEngine::CPhysicsWorld::SetDynamicsSimulationEnabled(bool bEnable)
{
	m_bRunDynamicSimulation = bEnable;
}

bool ParaEngine::CPhysicsWorld::IsDynamicsSimulationEnabled()
{
	return m_bRunDynamicSimulation;
}

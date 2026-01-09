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
#include "BipedObject.h"

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

	Vector3* pVertices = new Vector3[pointCount];
	for (int i = 0; i < pointCount; i++)
	{
		pVertices[i].x = vertices[i].position[0];
		pVertices[i].y = vertices[i].position[1];
		pVertices[i].z = vertices[i].position[2];
	}

	uint16_t index = 0;
	uint16_t* pIndices = new uint16_t[triangleCount * 3];
	bool stdCubeFaces[6] = { false, false, false, false, false, false }; // x 0, 1, y 0, 1 z 0, 1
	bool isStdCubeShape = faceCount == 6;
	
	for (int i = 0; i < faceCount; i++)
	{
		uint16_t indexOfs = 4 * i;
		const Vector3& pt0 = pVertices[indexOfs + 0];
		const Vector3& pt1 = pVertices[indexOfs + 1];
		const Vector3& pt2 = pVertices[indexOfs + 2];
		const Vector3& pt3 = pVertices[indexOfs + 3];
		
		if (pt0 != pt1 && pt0 != pt3 && pt1 != pt3)
		{
			pIndices[index++] = indexOfs + 0;
			pIndices[index++] = indexOfs + 1;
			pIndices[index++] = indexOfs + 3;
		}
		if (pt1 != pt2 && pt1 != pt3 && pt2 != pt3)
		{
			pIndices[index++] = indexOfs + 1;
			pIndices[index++] = indexOfs + 2;
			pIndices[index++] = indexOfs + 3;
		}
		
		// Check for standard cube shape
		if (isStdCubeShape)
		{
			// Verify all four vertices are unique
			isStdCubeShape = (pt0 != pt1) && (pt0 != pt2) && (pt0 != pt3) && 
			                 (pt1 != pt2) && (pt1 != pt3) && (pt2 != pt3);
			
			if (isStdCubeShape)
			{
				// Check which cube face this represents (optimized with early exit)
				int stdCubeFace = -1;
				if (pt0.x == 0 && pt1.x == 0 && pt2.x == 0 && pt3.x == 0) stdCubeFace = 0;
				else if (pt0.x == 1 && pt1.x == 1 && pt2.x == 1 && pt3.x == 1) stdCubeFace = 1;
				else if (pt0.y == 0 && pt1.y == 0 && pt2.y == 0 && pt3.y == 0) stdCubeFace = 2;
				else if (pt0.y == 1 && pt1.y == 1 && pt2.y == 1 && pt3.y == 1) stdCubeFace = 3;
				else if (pt0.z == 0 && pt1.z == 0 && pt2.z == 0 && pt3.z == 0) stdCubeFace = 4;
				else if (pt0.z == 1 && pt1.z == 1 && pt2.z == 1 && pt3.z == 1) stdCubeFace = 5;
				
				if (stdCubeFace >= 0 && !stdCubeFaces[stdCubeFace])
				{
					stdCubeFaces[stdCubeFace] = true;
				}
				else
				{
					isStdCubeShape = false;
				}
			}
		}
	}
	
	// Verify all 6 faces present for standard cube
	for (int i = 0; i < 6 && isStdCubeShape; i++)
	{
		if (!stdCubeFaces[i]) isStdCubeShape = false;
	}
	triangleCount = index / 3;

	// Compute hash efficiently
	if (isStdCubeShape)
	{
		pShape->m_hash = CPhysicsBlockShape::GetStdCubeHash();
	}
	else
	{
		// Build string for hash only if needed
		std::string source;
		source.reserve(pointCount * pointStrideBytes);
		const char* vertexData = reinterpret_cast<const char*>(pVertices);
		source.assign(vertexData, pointCount * pointStrideBytes);
		pShape->m_hash = StringHelper::md5(source);
	}
	
	// Check for existing shape with same hash
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
	
	// Create new shape
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
	if (actor == nullptr) return;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(property, (int)strlen(property));
	if (msg.GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
	{
		// Check for method calls (high-level API)
		if (msg["method"].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
		{
			std::string method = msg["method"];
			if (method == "AddWheel")
			{
				IParaPhysicsVehicle* pVehicle = GetVehicleByActor(actor);
				if (pVehicle) AddWheelToVehicle(pVehicle, property);
			}
			else if (method == "CreateVehicle")
			{
				CreateVehicle(actor);
			}
			else if (method == "ResetSuspension")
			{
				IParaPhysicsVehicle* pVehicle = GetVehicleByActor(actor);
				if (pVehicle) pVehicle->ResetSuspension();
			}
			else if (method == "SetVehicleControl")
			{
				IParaPhysicsVehicle* pVehicle = GetVehicleByActor(actor);
				if (pVehicle)
				{
					int wheelCount = pVehicle->GetNumWheels();

					// Process individual wheel controls
					for (int i = 0; i < wheelCount; ++i)
					{
						char keyBuf[32];
						snprintf(keyBuf, sizeof(keyBuf), "Steering%d", i);
						if (msg[(const char*)keyBuf].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
							pVehicle->SetSteeringValue((float)(double)msg[(const char*)keyBuf], i);

						snprintf(keyBuf, sizeof(keyBuf), "EngineForce%d", i);
						if (msg[(const char*)keyBuf].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
							pVehicle->ApplyEngineForce((float)(double)msg[(const char*)keyBuf], i);

						snprintf(keyBuf, sizeof(keyBuf), "Brake%d", i);
						if (msg[(const char*)keyBuf].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
							pVehicle->SetBrake((float)(double)msg[(const char*)keyBuf], i);
					}
					// Process shorthand controls
					if (msg["SteeringAll"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
					{
						float val = (float)(double)msg["SteeringAll"];
						for (int i = 0; i < wheelCount; ++i) pVehicle->SetSteeringValue(val, i);
					}
					if (msg["SteeringFront"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
					{
						float val = (float)(double)msg["SteeringFront"];
						int frontCount = wheelCount / 2;
						for (int i = 0; i < frontCount; ++i) pVehicle->SetSteeringValue(val, i);
					}
				}
			}
			else if (method == "ConstraintProperty")
			{
				// Delegate to existing logic via a temporary biped wrapper or reimplement
				// Since we have the actor, we can try to find the biped or work on actor directly
				// Reimplementing logic from SetConstraintPropertyForBiped/CreateJointForBipedStr to work with actor
				
				if (msg["Type"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
				{
					// Create Joint
					int constraintType = (int)(double)msg["Type"];
					IParaPhysicsActor* pOtherActor = nullptr;
					
					if (msg["OtherObject"].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
					{
						const std::string& sName = msg["OtherObject"];
						CBaseObject* pObj = CGlobals::GetScene()->GetGlobalObject(sName);
						if (pObj) pOtherActor = pObj->GetDynamicPhysicsActor();
					}

					Vector3 pivotA((float)(double)msg["PivotAX"], (float)(double)msg["PivotAY"], (float)(double)msg["PivotAZ"]);
					Vector3 pivotB((float)(double)msg["PivotBX"], (float)(double)msg["PivotBY"], (float)(double)msg["PivotBZ"]);
					Vector3 axisA((float)(double)msg["AxisAX"], (float)(double)msg["AxisAY"], (float)(double)msg["AxisAZ"]);
					Vector3 axisB((float)(double)msg["AxisBX"], (float)(double)msg["AxisBY"], (float)(double)msg["AxisBZ"]);

					// Defaults
					if (msg["AxisAY"].GetType() != NPL::NPLObjectBase::NPLObjectType_Number) axisA = Vector3(0, 1, 0);
					if (msg["AxisBY"].GetType() != NPL::NPLObjectBase::NPLObjectType_Number) axisB = Vector3(0, 1, 0);

					CreateConstraint(constraintType, actor, pOtherActor, pivotA, axisA, pivotB, axisB);
				}
				else if (msg["Index"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
				{
					// Set Property
					int index = (int)(double)msg["Index"];
					IParaPhysicsConstraint* pConstraint = GetConstraintByActor(actor, index);
					if (pConstraint)
					{
						SetConstraintProperty(pConstraint, property);
					}
				}
			}
			else if (method == "ReleaseAllJoints")
			{
				ReleaseConstraintsByActor(actor);
			}
			return; // Handled method
		}

		// Existing property handling
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

const char* CPhysicsWorld::GetActorPhysicsProperty(IParaPhysicsActor* actor, const char* inputTable)
{
	if (actor == nullptr) return "";

	static std::string sCode;
	NPL::NPLObjectProxy msg;

	if (inputTable && inputTable[0])
	{
		NPL::NPLObjectProxy inputMsg = NPL::NPLHelper::StringToNPLTable(inputTable, (int)strlen(inputTable));
		if (inputMsg.GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
		{
			if (inputMsg["method"].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
			{
				std::string method = inputMsg["method"];
				if (method == "ConstraintProperty")
				{
					if (inputMsg["Index"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
					{
						int index = (int)(double)inputMsg["Index"];
						IParaPhysicsConstraint* pConstraint = GetConstraintByActor(actor, index);
						if (pConstraint) return GetConstraintProperty(pConstraint);
					}
				}
				else if (method == "VehicleState")
				{
					IParaPhysicsVehicle* pVehicle = GetVehicleByActor(actor);
					msg["HasVehicle"] = (pVehicle != nullptr);
					msg["WheelCount"] = (double)(pVehicle ? pVehicle->GetNumWheels() : 0);
					msg["Speed"] = (double)(pVehicle ? pVehicle->GetCurrentSpeedKmHour() : 0.0f);
					if (pVehicle)
					{
						PARAVECTOR3 fwd = pVehicle->GetForwardVector();
						msg["ForwardX"] = (double)fwd.x;
						msg["ForwardY"] = (double)fwd.y;
						msg["ForwardZ"] = (double)fwd.z;
					}
					NPL::NPLHelper::NPLTableToString(NULL, msg, sCode);
					return sCode.c_str();
				}
			}
		}
	}

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
		// Release vehicles first (they depend on actors)
		m_vehicles.clear();
		
		// Release constraints (they depend on actors)
		m_constraints.clear();

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
	alignas(16) Matrix4 matrix;
	static int16_t s_block_frame_id = 0;
	s_block_frame_id++;
	
	if (IsDynamicsSimulationEnabled())
	{
		IParaPhysicsActor_Map_Type::iterator itCurCP = m_mapDynamicActors.begin();
		IParaPhysicsActor_Map_Type::iterator itEndCP = m_mapDynamicActors.end();

		// check load terrain physics blocks near all dynamic actors
		BlockWorldClient* pWorld = BlockWorldClient::GetInstance();
		bool isAutoPhysicsBlock = pWorld->IsAutoPhysics();
		
		if (isAutoPhysicsBlock)
		{
			CShapeAABB aabb;
			for (itCurCP = m_mapDynamicActors.begin(); itCurCP != itEndCP; itCurCP++)
			{
				IParaPhysicsActor* actor = *itCurCP;
				CBaseObject* obj = (CBaseObject*)(actor->GetUserData());
				
				if (!actor->IsStaticOrKinematicObject())
				{
					obj->GetAABB(&aabb); 
					LoadPhysicsBlock(&aabb, s_block_frame_id);
				}
			}
		}

		if (m_pPhysicsWorld)
		{
			PERF1("Dynamic Physics");
			m_pPhysicsWorld->StepSimulation((float)dTime);
		}

		// Update transforms for active dynamic actors only
		for (itCurCP = m_mapDynamicActors.begin(); itCurCP != itEndCP; itCurCP++)
		{
			IParaPhysicsActor* actor = *itCurCP;
			
			if (!actor->IsStaticOrKinematicObject())
			{
				int activationState = actor->GetActivationState();
				if (activationState != 2) // Skip ISLAND_SLEEPING actors
				{
					CBaseObject* obj = (CBaseObject*)(actor->GetUserData());
					actor->GetWorldTransform((PARAMATRIX*)&matrix);
					Vector3 pos = matrix.getTrans();
					float fCenterHeight = obj->GetAssetHeight() * 0.5f;

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
		}

		// Remove stale physics blocks
		if (isAutoPhysicsBlock)
		{
			auto it = m_mapPhysicsBlocks.begin();
			while (it != m_mapPhysicsBlocks.end())
			{
				if (std::abs(s_block_frame_id - it->second->GetFrameId()) > 2)
				{
					it = m_mapPhysicsBlocks.erase(it);
				}
				else
				{
					++it;
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
	if (!pShape)
		return NULL;
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
	if (!pActor)
		return NULL;
	pActor->SetUserData(obj);
	m_mapDynamicActors.insert(pActor);
	return pActor;
}

IParaPhysicsActor* ParaEngine::CPhysicsWorld::CreateDynamicShape(CBaseObject* obj)
{
	ParaPhysicsSimpleShapeDesc desc;
	desc.m_shape = obj->GetPhysicsShape();

	float fRadius = obj->GetPhysicsRadius();
	float fHeight = obj->GetPhysicsHeight();
	
	if (fRadius <= 0.f || fHeight <= 0.f)
		return NULL; // invalid dimensions
	
	float fScale = obj->GetScaling();
	desc.m_halfWidth = fRadius; // without scaling
	desc.m_halfHeight = fHeight * 0.5f;
	desc.m_halfLength = fRadius;

	IParaPhysicsShape* pShape = m_pPhysicsWorld->CreateSimpleShape(desc);
	if (!pShape)
		return NULL;
	
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
	if (!pActor)
		return NULL;
	pActor->SetUserData(obj);
	m_mapDynamicActors.insert(pActor);
	return pActor;
}

void ParaEngine::CPhysicsWorld::LoadPhysicsBlock(CShapeAABB* aabb, int16_t frameId, float extend)
{
	Vector3 min, max;
	aabb->GetMin(min);
	aabb->GetMax(max);
	min -= extend;  // extend for next frame's possible distance
	max += extend;
	
	float offset_y = BlockWorldClient::GetInstance()->GetVerticalOffset();
	float inv_blockSize = (float) BlockConfig::g_dBlockSizeInverse;
	
	// Use inverse multiplication instead of division for better performance
	int16_t min_x = (int16_t)std::floor(min.x * inv_blockSize);
	int16_t min_y = (int16_t)std::floor((min.y - offset_y) * inv_blockSize);
	int16_t min_z = (int16_t)std::floor(min.z * inv_blockSize);
	int16_t max_x = (int16_t)std::floor(max.x * inv_blockSize);
	int16_t max_y = (int16_t)std::floor((max.y - offset_y) * inv_blockSize);
	int16_t max_z = (int16_t)std::floor(max.z * inv_blockSize);
	
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
	
	Block* pBlock = pWorld->GetBlock(bx, by, bz);
	
	if (!pBlock)
	{
		uint64_t id = CPhysicsBlock::PackID(bx, by, bz);
		m_mapPhysicsBlocks.erase(id);
		return nullptr;
	}
	BlockTemplate* pTemplate = pBlock->GetTemplate();

	// Early rejection: check if block template allows physics before expensive lookups
	if (!pTemplate || !pTemplate->IsMatchAttribute(BlockTemplate::batt_obstruction))
	{
		return nullptr;
	}
	

	uint16_t blockData = pBlock->GetUserData();
	uint16_t tplId = pTemplate->GetID();
	uint32_t key = (blockData << 16) + tplId;
	uint64_t id = CPhysicsBlock::PackID(bx, by, bz);

	auto it = m_mapPhysicsBlocks.find(id);
	if (it != m_mapPhysicsBlocks.end())
	{
		std::shared_ptr<CPhysicsBlock>& pBlock = it->second;
		if (pBlock->GetKey() == key)
		{
			// Block exists and hasn't changed
			if (!pBlock->IsLoaded())
			{
				BlockModel& model = pTemplate->GetBlockModel(pWorld, bx, by, bz, blockData);
				pBlock->Load(model, m_pPhysicsWorld);
				SetActorPhysicsProperty(pBlock->GetActor(), pTemplate->GetPhysicsProperty().c_str());
			}
			return pBlock;
		}
		// Block changed, remove old one
		m_mapPhysicsBlocks.erase(it);
	}

	// Create new physics block
	BlockModel& model = pTemplate->GetBlockModel(pWorld, bx, by, bz, blockData);
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
		ReleaseConstraintsByActor(pActor);
		ReleaseVehicleByActor(pActor);
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

//////////////////////////////////////////////////////////////////////////
// Constraint/Joint Implementation
//////////////////////////////////////////////////////////////////////////

IParaPhysicsConstraint* ParaEngine::CPhysicsWorld::CreateConstraint(int constraintType, IParaPhysicsActor* pActorA, IParaPhysicsActor* pActorB,
	const Vector3& pivotInA, const Vector3& axisInA, const Vector3& pivotInB, const Vector3& axisInB, bool disableCollision)
{
	IParaPhysics* pPhysics = GetPhysicsInterface();
	if (!pPhysics || !pActorA)
		return nullptr;

	ParaPhysicsConstraintDesc desc;
	desc.m_type = (ParaPhysicsConstraintType)constraintType;
	desc.m_pActorA = pActorA;
	desc.m_pActorB = pActorB;
	desc.m_pivotInA = PARAVECTOR3(pivotInA.x, pivotInA.y, pivotInA.z);
	desc.m_axisInA = PARAVECTOR3(axisInA.x, axisInA.y, axisInA.z);
	desc.m_pivotInB = PARAVECTOR3(pivotInB.x, pivotInB.y, pivotInB.z);
	desc.m_axisInB = PARAVECTOR3(axisInB.x, axisInB.y, axisInB.z);
	desc.m_disableCollisionsBetweenBodies = disableCollision;

	IParaPhysicsConstraint* pConstraint = pPhysics->CreateConstraint(desc);
	if (pConstraint)
	{
		m_constraints.insert(pConstraint);
	}
	return pConstraint;
}

void ParaEngine::CPhysicsWorld::ReleaseConstraint(IParaPhysicsConstraint* pConstraint)
{
	if (!pConstraint)
		return;

	auto it = m_constraints.find(pConstraint);
	if (it != m_constraints.end())
	{
		m_constraints.erase(it);
		if (m_pPhysicsWorld)
		{
			m_pPhysicsWorld->ReleaseConstraint(pConstraint);
		}
	}
}

void ParaEngine::CPhysicsWorld::SetConstraintProperty(IParaPhysicsConstraint* pConstraint, const char* property)
{
	if (!pConstraint || !property)
		return;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(property, (int)strlen(property));
	if (msg.GetType() != NPL::NPLObjectBase::NPLObjectType_Table)
		return;

	if (msg["Enabled"].GetType() == NPL::NPLObjectBase::NPLObjectType_Bool)
		pConstraint->SetEnabled((bool)msg["Enabled"]);
	if (msg["BreakingThreshold"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		pConstraint->SetBreakingThreshold((float)(double)msg["BreakingThreshold"]);

	// Hinge-specific properties
	if (pConstraint->GetType() == ConstraintType_Hinge)
	{
		if (msg["LowLimit"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		{
			float low = (float)(double)msg["LowLimit"];
			float high = msg["HighLimit"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["HighLimit"] : low;
			float softness = msg["Softness"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["Softness"] : 0.9f;
			float biasFactor = msg["BiasFactor"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["BiasFactor"] : 0.3f;
			float relaxationFactor = msg["RelaxationFactor"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["RelaxationFactor"] : 1.0f;
			pConstraint->SetHingeLimit(low, high, softness, biasFactor, relaxationFactor);
		}
		if (msg["EnableMotor"].GetType() == NPL::NPLObjectBase::NPLObjectType_Bool)
		{
			bool enable = (bool)msg["EnableMotor"];
			float targetVelocity = msg["MotorTargetVelocity"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["MotorTargetVelocity"] : 0.0f;
			float maxImpulse = msg["MaxMotorImpulse"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["MaxMotorImpulse"] : 1.0f;
			pConstraint->EnableHingeMotor(enable, targetVelocity, maxImpulse);
		}
	}

	// Slider-specific properties
	if (pConstraint->GetType() == ConstraintType_Slider)
	{
		if (msg["LowerLinLimit"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		{
			float lower = (float)(double)msg["LowerLinLimit"];
			float upper = msg["UpperLinLimit"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["UpperLinLimit"] : lower;
			pConstraint->SetSliderLimit(lower, upper);
		}
		if (msg["EnableLinMotor"].GetType() == NPL::NPLObjectBase::NPLObjectType_Bool)
		{
			bool enable = (bool)msg["EnableLinMotor"];
			float targetVelocity = msg["TargetLinMotorVelocity"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["TargetLinMotorVelocity"] : 0.0f;
			float maxForce = msg["MaxLinMotorForce"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number ? (float)(double)msg["MaxLinMotorForce"] : 1.0f;
			pConstraint->EnableSliderMotor(enable, targetVelocity, maxForce);
		}
	}
}

const char* ParaEngine::CPhysicsWorld::GetConstraintProperty(IParaPhysicsConstraint* pConstraint)
{
	if (!pConstraint)
		return "";

	static std::string sCode;
	NPL::NPLObjectProxy msg;
	msg["Type"] = (double)pConstraint->GetType();
	msg["Enabled"] = pConstraint->IsEnabled();
	msg["BreakingThreshold"] = (double)pConstraint->GetBreakingThreshold();

	if (pConstraint->GetType() == ConstraintType_Hinge)
	{
		msg["HingeAngle"] = (double)pConstraint->GetHingeAngle();
	}
	else if (pConstraint->GetType() == ConstraintType_Slider)
	{
		msg["SliderPosition"] = (double)pConstraint->GetSliderPosition();
	}

	NPL::NPLHelper::NPLTableToString(NULL, msg, sCode);
	return sCode.c_str();
}

//////////////////////////////////////////////////////////////////////////
// Vehicle/Wheel Implementation
//////////////////////////////////////////////////////////////////////////

IParaPhysicsVehicle* ParaEngine::CPhysicsWorld::CreateVehicle(IParaPhysicsActor* pChassisActor)
{
	IParaPhysics* pPhysics = GetPhysicsInterface();
	if (!pPhysics || !pChassisActor)
		return nullptr;

	IParaPhysicsVehicle* pVehicle = pPhysics->CreateVehicle(pChassisActor);
	if (pVehicle)
	{
		m_vehicles.insert(pVehicle);
	}
	return pVehicle;
}

void ParaEngine::CPhysicsWorld::ReleaseVehicle(IParaPhysicsVehicle* pVehicle)
{
	if (!pVehicle)
		return;

	auto it = m_vehicles.find(pVehicle);
	if (it != m_vehicles.end())
	{
		m_vehicles.erase(it);
		if (m_pPhysicsWorld)
		{
			m_pPhysicsWorld->ReleaseVehicle(pVehicle);
		}
	}
}

int ParaEngine::CPhysicsWorld::AddWheelToVehicle(IParaPhysicsVehicle* pVehicle, const char* wheelConfig)
{
	if (!pVehicle || !wheelConfig)
		return -1;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(wheelConfig, (int)strlen(wheelConfig));
	if (msg.GetType() != NPL::NPLObjectBase::NPLObjectType_Table)
		return -1;

	ParaPhysicsWheelDesc desc;

	// Connection point
	if (msg["ConnectionPointX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		desc.m_connectionPoint.x = (float)(double)msg["ConnectionPointX"];
		desc.m_connectionPoint.y = (float)(double)msg["ConnectionPointY"];
		desc.m_connectionPoint.z = (float)(double)msg["ConnectionPointZ"];
	}

	// Wheel direction (usually down)
	if (msg["WheelDirectionX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		desc.m_wheelDirection.x = (float)(double)msg["WheelDirectionX"];
		desc.m_wheelDirection.y = (float)(double)msg["WheelDirectionY"];
		desc.m_wheelDirection.z = (float)(double)msg["WheelDirectionZ"];
	}
	else
	{
		desc.m_wheelDirection = PARAVECTOR3(0, -1, 0); // Default down
	}

	// Wheel axle (usually left or right)
	if (msg["WheelAxleX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		desc.m_wheelAxle.x = (float)(double)msg["WheelAxleX"];
		desc.m_wheelAxle.y = (float)(double)msg["WheelAxleY"];
		desc.m_wheelAxle.z = (float)(double)msg["WheelAxleZ"];
	}
	else
	{
		desc.m_wheelAxle = PARAVECTOR3(-1, 0, 0); // Default left
	}

	// Other properties with defaults
	if (msg["WheelRadius"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_wheelRadius = (float)(double)msg["WheelRadius"];
	if (msg["WheelWidth"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_wheelWidth = (float)(double)msg["WheelWidth"];
	if (msg["SuspensionRestLength"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_suspensionRestLength = (float)(double)msg["SuspensionRestLength"];
	if (msg["SuspensionStiffness"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_suspensionStiffness = (float)(double)msg["SuspensionStiffness"];
	if (msg["WheelsDampingCompression"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_wheelsDampingCompression = (float)(double)msg["WheelsDampingCompression"];
	if (msg["WheelsDampingRelaxation"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_wheelsDampingRelaxation = (float)(double)msg["WheelsDampingRelaxation"];
	if (msg["FrictionSlip"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_frictionSlip = (float)(double)msg["FrictionSlip"];
	if (msg["RollInfluence"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		desc.m_rollInfluence = (float)(double)msg["RollInfluence"];
	if (msg["IsFrontWheel"].GetType() == NPL::NPLObjectBase::NPLObjectType_Bool)
		desc.m_isFrontWheel = (bool)msg["IsFrontWheel"];

	return pVehicle->AddWheel(desc);
}

//////////////////////////////////////////////////////////////////////////
// Actor-based Lookup Implementation
//////////////////////////////////////////////////////////////////////////

int ParaEngine::CPhysicsWorld::GetConstraintsByActor(IParaPhysicsActor* pActor, std::vector<IParaPhysicsConstraint*>& outConstraints)
{
	outConstraints.clear();
	if (!pActor)
		return 0;

	for (auto pConstraint : m_constraints)
	{
		if (pConstraint->GetActorA() == pActor || pConstraint->GetActorB() == pActor)
		{
			outConstraints.push_back(pConstraint);
		}
	}
	return (int)outConstraints.size();
}

IParaPhysicsConstraint* ParaEngine::CPhysicsWorld::GetConstraintByActor(IParaPhysicsActor* pActor, int index)
{
	if (!pActor || index < 0)
		return nullptr;

	int count = 0;
	for (auto pConstraint : m_constraints)
	{
		if (pConstraint->GetActorA() == pActor || pConstraint->GetActorB() == pActor)
		{
			if (count == index)
				return pConstraint;
			++count;
		}
	}
	return nullptr;
}

int ParaEngine::CPhysicsWorld::GetConstraintCountByActor(IParaPhysicsActor* pActor)
{
	if (!pActor)
		return 0;

	int count = 0;
	for (auto pConstraint : m_constraints)
	{
		if (pConstraint->GetActorA() == pActor || pConstraint->GetActorB() == pActor)
		{
			++count;
		}
	}
	return count;
}

void ParaEngine::CPhysicsWorld::ReleaseConstraintsByActor(IParaPhysicsActor* pActor)
{
	if (!pActor)
		return;

	// Collect constraints to remove (can't modify set while iterating)
	std::vector<IParaPhysicsConstraint*> toRemove;
	for (auto pConstraint : m_constraints)
	{
		if (pConstraint->GetActorA() == pActor || pConstraint->GetActorB() == pActor)
		{
			toRemove.push_back(pConstraint);
		}
	}

	// Release collected constraints
	for (auto pConstraint : toRemove)
	{
		m_constraints.erase(pConstraint);
		if (m_pPhysicsWorld)
		{
			m_pPhysicsWorld->ReleaseConstraint(pConstraint);
		}
	}
}

IParaPhysicsVehicle* ParaEngine::CPhysicsWorld::GetVehicleByActor(IParaPhysicsActor* pChassisActor)
{
	if (!pChassisActor)
		return nullptr;

	for (auto pVehicle : m_vehicles)
	{
		if (pVehicle->GetChassisActor() == pChassisActor)
		{
			return pVehicle;
		}
	}
	return nullptr;
}

void ParaEngine::CPhysicsWorld::ReleaseVehicleByActor(IParaPhysicsActor* pChassisActor)
{
	if (!pChassisActor)
		return;

	IParaPhysicsVehicle* pVehicle = GetVehicleByActor(pChassisActor);
	if (pVehicle)
	{
		ReleaseVehicle(pVehicle);
	}
}
//////////////////////////////////////////////////////////////////////////
// Biped-oriented Constraint/Vehicle Implementation
//////////////////////////////////////////////////////////////////////////

IParaPhysicsConstraint* ParaEngine::CPhysicsWorld::CreateJointForBiped(CBipedObject* pBiped, int constraintType, CBipedObject* pOtherBiped,
	const Vector3& pivotInA, const Vector3& pivotInB,
	const Vector3& axisInA, const Vector3& axisInB)
{
	if (!pBiped)
		return nullptr;

	// Ensure this biped has a dynamic physics actor
	IParaPhysicsActor* pActorA = pBiped->GetDynamicPhysicsActor();
	if (!pActorA)
	{
		pBiped->LoadDynamicPhysics();
		pActorA = pBiped->GetDynamicPhysicsActor();
		if (!pActorA)
			return nullptr;
	}

	// Get the other actor if specified
	IParaPhysicsActor* pActorB = nullptr;
	if (pOtherBiped)
	{
		pActorB = pOtherBiped->GetDynamicPhysicsActor();
		if (!pActorB)
		{
			pOtherBiped->LoadDynamicPhysics();
			pActorB = pOtherBiped->GetDynamicPhysicsActor();
		}
	}

	IParaPhysicsConstraint* pConstraint = CreateConstraint(
		constraintType, pActorA, pActorB,
		pivotInA, axisInA, pivotInB, axisInB, true);

	if (pConstraint)
	{
		pConstraint->SetUserData(pBiped);
	}

	return pConstraint;
}

void ParaEngine::CPhysicsWorld::CreateJointForBipedStr(CBipedObject* pBiped, const char* config)
{
	if (!pBiped || !config)
		return;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(config, (int)strlen(config));
	if (msg.GetType() != NPL::NPLObjectBase::NPLObjectType_Table)
		return;

	int constraintType = 1; // Default to hinge
	if (msg["Type"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		constraintType = (int)(double)msg["Type"];

	Vector3 pivotInA(0, 0, 0), pivotInB(0, 0, 0);
	Vector3 axisInA(0, 1, 0), axisInB(0, 1, 0);

	if (msg["PivotAX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		pivotInA.x = (float)(double)msg["PivotAX"];
		pivotInA.y = (float)(double)msg["PivotAY"];
		pivotInA.z = (float)(double)msg["PivotAZ"];
	}
	if (msg["PivotBX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		pivotInB.x = (float)(double)msg["PivotBX"];
		pivotInB.y = (float)(double)msg["PivotBY"];
		pivotInB.z = (float)(double)msg["PivotBZ"];
	}
	if (msg["AxisAX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		axisInA.x = (float)(double)msg["AxisAX"];
		axisInA.y = (float)(double)msg["AxisAY"];
		axisInA.z = (float)(double)msg["AxisAZ"];
	}
	if (msg["AxisBX"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		axisInB.x = (float)(double)msg["AxisBX"];
		axisInB.y = (float)(double)msg["AxisBY"];
		axisInB.z = (float)(double)msg["AxisBZ"];
	}

	// Find other object by name if specified
	CBipedObject* pOtherBiped = nullptr;
	if (msg["OtherObject"].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
	{
		std::string otherName = (const std::string&)msg["OtherObject"];
		if (!otherName.empty())
		{
			CBaseObject* pObj = CGlobals::GetScene()->GetGlobalObject(otherName);
			if (pObj && pObj->GetType() == CBaseObject::BipedObject)
			{
				pOtherBiped = static_cast<CBipedObject*>(pObj);
			}
		}
	}

	IParaPhysicsConstraint* pConstraint = CreateJointForBiped(pBiped, constraintType, pOtherBiped, pivotInA, pivotInB, axisInA, axisInB);
	
	// Apply additional properties from the same config
	if (pConstraint)
	{
		SetConstraintProperty(pConstraint, config);
	}
}

void ParaEngine::CPhysicsWorld::SetJointPropertyByIndexForBiped(CBipedObject* pBiped, const char* config)
{
	if (!pBiped || !config)
		return;

	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	if (!pActor)
		return;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(config, (int)strlen(config));
	if (msg.GetType() != NPL::NPLObjectBase::NPLObjectType_Table)
		return;

	int index = 0;
	if (msg["Index"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
		index = (int)(double)msg["Index"];

	IParaPhysicsConstraint* pConstraint = GetConstraintByActor(pActor, index);
	if (pConstraint)
	{
		SetConstraintProperty(pConstraint, config);
	}
}

const char* ParaEngine::CPhysicsWorld::GetJointPropertyByIndexForBiped(CBipedObject* pBiped, int nSelectedIndex)
{
	if (!pBiped)
		return "";

	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	if (!pActor)
		return "";

	IParaPhysicsConstraint* pConstraint = GetConstraintByActor(pActor, nSelectedIndex);
	if (pConstraint)
	{
		return GetConstraintProperty(pConstraint);
	}
	return "";
}

bool ParaEngine::CPhysicsWorld::CreateVehicleForBiped(CBipedObject* pBiped)
{
	if (!pBiped)
		return false;

	// Check if we already have a vehicle
	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	if (pActor && GetVehicleByActor(pActor))
		return true;

	// Ensure this biped has a dynamic physics actor
	if (!pActor)
	{
		pBiped->LoadDynamicPhysics();
		pActor = pBiped->GetDynamicPhysicsActor();
		if (!pActor)
			return false;
	}

	IParaPhysicsVehicle* pVehicle = CreateVehicle(pActor);
	if (pVehicle)
	{
		pVehicle->SetUserData(pBiped);
	}
	return pVehicle != nullptr;
}

void ParaEngine::CPhysicsWorld::SetVehicleControlForBipedStr(CBipedObject* pBiped, const char* config)
{
	if (!pBiped || !config)
		return;

	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	IParaPhysicsVehicle* pVehicle = pActor ? GetVehicleByActor(pActor) : nullptr;
	if (!pVehicle)
		return;

	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(config, (int)strlen(config));
	if (msg.GetType() != NPL::NPLObjectBase::NPLObjectType_Table)
		return;

	int wheelCount = pVehicle->GetNumWheels();

	// Process individual wheel controls: Steering0, Steering1, EngineForce0, Brake0, etc.
	for (int i = 0; i < wheelCount; ++i)
	{
		char keyBuf[32];

		snprintf(keyBuf, sizeof(keyBuf), "Steering%d", i);
		if (msg[(const char*)keyBuf].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
			pVehicle->SetSteeringValue((float)(double)msg[(const char*)keyBuf], i);

		snprintf(keyBuf, sizeof(keyBuf), "EngineForce%d", i);
		if (msg[(const char*)keyBuf].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
			pVehicle->ApplyEngineForce((float)(double)msg[(const char*)keyBuf], i);

		snprintf(keyBuf, sizeof(keyBuf), "Brake%d", i);
		if (msg[(const char*)keyBuf].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
			pVehicle->SetBrake((float)(double)msg[(const char*)keyBuf], i);
	}

	// Process shorthand controls: SteeringFront, SteeringAll, EngineForceRear, etc.
	if (msg["SteeringAll"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["SteeringAll"];
		for (int i = 0; i < wheelCount; ++i)
			pVehicle->SetSteeringValue(val, i);
	}
	if (msg["SteeringFront"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["SteeringFront"];
		// Assume first half are front wheels
		int frontCount = wheelCount / 2;
		for (int i = 0; i < frontCount; ++i)
			pVehicle->SetSteeringValue(val, i);
	}

	if (msg["EngineForceAll"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["EngineForceAll"];
		for (int i = 0; i < wheelCount; ++i)
			pVehicle->ApplyEngineForce(val, i);
	}
	if (msg["EngineForceRear"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["EngineForceRear"];
		// Assume last half are rear wheels
		int frontCount = wheelCount / 2;
		for (int i = frontCount; i < wheelCount; ++i)
			pVehicle->ApplyEngineForce(val, i);
	}
	if (msg["EngineForceFront"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["EngineForceFront"];
		int frontCount = wheelCount / 2;
		for (int i = 0; i < frontCount; ++i)
			pVehicle->ApplyEngineForce(val, i);
	}

	if (msg["BrakeAll"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["BrakeAll"];
		for (int i = 0; i < wheelCount; ++i)
			pVehicle->SetBrake(val, i);
	}
	if (msg["BrakeRear"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["BrakeRear"];
		int frontCount = wheelCount / 2;
		for (int i = frontCount; i < wheelCount; ++i)
			pVehicle->SetBrake(val, i);
	}
	if (msg["BrakeFront"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		float val = (float)(double)msg["BrakeFront"];
		int frontCount = wheelCount / 2;
		for (int i = 0; i < frontCount; ++i)
			pVehicle->SetBrake(val, i);
	}
}

const char* ParaEngine::CPhysicsWorld::GetVehicleStateForBiped(CBipedObject* pBiped)
{
	static std::string sCode;
	NPL::NPLObjectProxy msg;

	if (!pBiped)
	{
		msg["HasVehicle"] = false;
		msg["WheelCount"] = 0.0;
		msg["Speed"] = 0.0;
		NPL::NPLHelper::NPLTableToString(NULL, msg, sCode);
		return sCode.c_str();
	}

	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	IParaPhysicsVehicle* pVehicle = pActor ? GetVehicleByActor(pActor) : nullptr;
	
	msg["HasVehicle"] = (pVehicle != nullptr);
	msg["WheelCount"] = (double)(pVehicle ? pVehicle->GetNumWheels() : 0);
	msg["Speed"] = (double)(pVehicle ? pVehicle->GetCurrentSpeedKmHour() : 0.0f);

	if (pVehicle)
	{
		PARAVECTOR3 fwd = pVehicle->GetForwardVector();
		msg["ForwardX"] = (double)fwd.x;
		msg["ForwardY"] = (double)fwd.y;
		msg["ForwardZ"] = (double)fwd.z;
	}

	NPL::NPLHelper::NPLTableToString(NULL, msg, sCode);
	return sCode.c_str();
}

void ParaEngine::CPhysicsWorld::SetConstraintPropertyForBiped(CBipedObject* pBiped, const char* property)
{
	if (!pBiped || !property)
		return;
	
	// Parse to check if this is a joint creation or property modification
	NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(property, (int)strlen(property));
	if (msg.GetType() != NPL::NPLObjectBase::NPLObjectType_Table)
		return;

	// If Type is specified, this is a joint creation request
	if (msg["Type"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number)
	{
		CreateJointForBipedStr(pBiped, property);
	}
	else
	{
		// Otherwise it's a property modification
		SetJointPropertyByIndexForBiped(pBiped, property);
	}
}

const char* ParaEngine::CPhysicsWorld::GetConstraintPropertyForBiped(CBipedObject* pBiped, int nIndex)
{
	if (!pBiped)
		return "";
	return GetJointPropertyByIndexForBiped(pBiped, nIndex);
}

bool ParaEngine::CPhysicsWorld::HasVehicleForBiped(CBipedObject* pBiped)
{
	if (!pBiped)
		return false;
	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	return pActor && GetVehicleByActor(pActor) != nullptr;
}

int ParaEngine::CPhysicsWorld::AddWheelForBiped(CBipedObject* pBiped, const char* wheelConfig)
{
	if (!pBiped || !wheelConfig)
		return -1;
	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	IParaPhysicsVehicle* pVehicle = pActor ? GetVehicleByActor(pActor) : nullptr;
	if (!pVehicle)
		return -1;
	return AddWheelToVehicle(pVehicle, wheelConfig);
}

int ParaEngine::CPhysicsWorld::GetWheelCountForBiped(CBipedObject* pBiped)
{
	if (!pBiped)
		return 0;
	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	IParaPhysicsVehicle* pVehicle = pActor ? GetVehicleByActor(pActor) : nullptr;
	return pVehicle ? pVehicle->GetNumWheels() : 0;
}

float ParaEngine::CPhysicsWorld::GetVehicleSpeedForBiped(CBipedObject* pBiped)
{
	if (!pBiped)
		return 0.0f;
	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	IParaPhysicsVehicle* pVehicle = pActor ? GetVehicleByActor(pActor) : nullptr;
	return pVehicle ? pVehicle->GetCurrentSpeedKmHour() : 0.0f;
}

void ParaEngine::CPhysicsWorld::ResetVehicleSuspensionForBiped(CBipedObject* pBiped)
{
	if (!pBiped)
		return;
	IParaPhysicsActor* pActor = pBiped->GetDynamicPhysicsActor();
	IParaPhysicsVehicle* pVehicle = pActor ? GetVehicleByActor(pActor) : nullptr;
	if (pVehicle)
	{
		pVehicle->ResetSuspension();
	}
}